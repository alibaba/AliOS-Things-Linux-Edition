/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "secstore-manager.h"

#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <mntent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/fs.h>
#include <linux/limits.h>

#include <keyutils.h>
#include <ecryptfs.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

/* decrypt data with aes-128-cbc */
static int aes_decrypt_data(unsigned char *out_buf, unsigned char *in_buf,
                            size_t count, struct aes_metadata *aes_meta);

/* encrypt data with aes-128-cbc */
static int aes_encrypt_data(unsigned char *out_buf, unsigned char *in_buf,
                            size_t count, struct aes_metadata *aes_meta);

/* check if signatures are valid */
static int check_sigs(unsigned char *fek_sig, unsigned char *fnek_sig);

/* get the necessary metadata to build the auth tok for specified uname */
static int get_metadata(struct secstore_metadata *fek_meta,
                        struct secstore_metadata *fnek_meta,
                        struct secstore_paths *ss_paths);

/* initialization phrase when boot up for the first time */
static int secstore_init(struct secstore_metadata *fek_meta,
                         struct secstore_metadata *fnek_meta,
                         struct secstore_paths *ss_paths, const char *uname);

/* generate auth token keys with metadata and add them to keyring */
static int generate_and_add_key_to_keyring(struct secstore_metadata *fek_meta,
                                           struct secstore_metadata *fnek_meta);

/* mount all secret directories in secure storage */
static int mount_secret_dirs(void);

/* mount the secret directory with specified uname */
static int mount_with_uname(const char *uname);

/* mount a secret directory */
static int mount_secret_dir(const char *uname, const char *fek_sig,
                            const char *fnek_sig, struct secstore_paths *ss_paths);

/* unmount all secret directorys in secure storage */
static int umount_secret_dirs(void);

/* unmount the secret directory with specified username. */
static int umount_with_uname(const char *uname);

int main(int argc, char *argv[])
{
    /* Check the argument */
    if (argc > 1) {
        if (strcmp(argv[1], "stop") == 0) {
            /* Unmount all secret dirs */
            if (umount_secret_dirs() != 0) {
                fprintf(stderr, "Failed to umount secret dirs\n");
            }
            return 0;
        } else if (strcmp(argv[1], "start") == 0) {
            /* Mount all secret dirs */
            if (mount_secret_dirs() != 0) {
                fprintf(stderr, "Failed to mount secret dirs\n");
            }
            return 0;
        } else {
            printf("Usage: secstore-manager start/stop\n");
        }
    } else {
        printf("Usage: secstore-manager start/stop\n");
    }

    return 0;
}

static int umount_secret_dirs(void)
{
    char buf[256] = "\0";
    int  finish   = 0;
    DIR *dir;

    struct dirent *de;

    dir = opendir(SECSTORE_DIR_NAME);

    if (dir == NULL) {
        fprintf(stderr, "Could not open secure store directory\n");
        goto err;
    }

    /* Iterate the directory and umount secure storage */
    while ((de = readdir(dir)) != NULL) {
        if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)) {
            if (umount_with_uname(de->d_name) != 0) {
                fprintf(stderr, "umounting %s's secure store failed\n", buf);
            }
        }
    }

    closedir(dir);

    return 0;
err:
    return -1;
}

static int umount_with_uname(const char *uname)
{
    char user_path[SECSTORE_PATH_MAX];

    size_t temp_length;

    memset((void *)user_path, 0, sizeof(user_path));

    strncpy(user_path, SECSTORE_BASE_PATH, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(user_path);
    strncat(user_path, uname, SECSTORE_PATH_MAX - temp_length - 1);

    if (check_is_mounted(user_path) == 0) {
        fprintf(stderr, "Secret dir of %s is already umounted\n", uname);
        return 0;
    }

    temp_length = strlen(user_path);
    strncat(user_path, SECSTORE_SECRET_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    /* No lazy unmount with MNT_DETACH */
    if (umount(user_path) != 0) {
        fprintf(stderr, "Failed to unmount uname:%s's secstore dir, the errno is %d\n", uname, errno);
        return -1;
    }

    return 0;
}

static int mount_secret_dirs(void)
{
    char buf[256] = "\0";
    int  finish   = 0;
    DIR *dir;

    struct dirent *de;

    ecryptfs_validate_keyring();

    dir = opendir(SECSTORE_DIR_NAME);

    if (dir == NULL) {
        fprintf(stderr, "Could not open secure store directory\n");
        goto err;
    }

    /* Iterate the directory and mount secure storage */
    while ((de = readdir(dir)) != NULL) {
        if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)) {
            if (mount_with_uname(de->d_name) != 0) {
                fprintf(stderr, "mounting %s's secure store failed\n", de->d_name);
            }
        }
    }

    closedir(dir);

    return 0;
err:
    return -1;
}

static int mount_with_uname(const char *uname)
{
    int ret;

    struct secstore_metadata fek_meta;
    struct secstore_metadata fnek_meta;
    struct secstore_paths    ss_paths;

    size_t temp_length;

    memset((void *)&fek_meta, 0, sizeof(struct secstore_metadata));
    memset((void *)&fnek_meta, 0, sizeof(struct secstore_metadata));
    memset((void *)&ss_paths, 0, sizeof(struct secstore_paths));

    /* calculate the absolute paths in secure storage */
    strncpy(ss_paths.user_path, SECSTORE_BASE_PATH, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.user_path);
    strncat(ss_paths.user_path, uname, SECSTORE_PATH_MAX - temp_length - 1);

    strncpy(ss_paths.meta_path, ss_paths.user_path, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.meta_path);
    strncat(ss_paths.meta_path, SECSTORE_META_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    strncpy(ss_paths.fek_blob_path, ss_paths.meta_path, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.fek_blob_path);
    strncat(ss_paths.fek_blob_path, SECSTORE_FEK_BLOB_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    strncpy(ss_paths.fnek_blob_path, ss_paths.meta_path, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.fnek_blob_path);
    strncat(ss_paths.fnek_blob_path, SECSTORE_FNEK_BLOB_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    strncpy(ss_paths.aes_blob_path, ss_paths.meta_path, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.aes_blob_path);
    strncat(ss_paths.aes_blob_path, SECSTORE_AES_BLOB_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    strncpy(ss_paths.origin_path, ss_paths.user_path, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.origin_path);
    strncat(ss_paths.origin_path, SECSTORE_ORIGIN_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    strncpy(ss_paths.secret_path, ss_paths.user_path, SECSTORE_PATH_MAX - 1);
    temp_length = strlen(ss_paths.secret_path);
    strncat(ss_paths.secret_path, SECSTORE_SECRET_NAME, SECSTORE_PATH_MAX - temp_length - 1);

    /* check if initialization is needed */
    if ((access(ss_paths.fek_blob_path, F_OK) == -1) ||
       (access(ss_paths.fnek_blob_path, F_OK) == -1) ||
       (access(ss_paths.aes_blob_path, F_OK) == -1)) {
        /* boot up for the first time */
        ret = secstore_init(&fek_meta, &fnek_meta, &ss_paths, uname);
    } else {
        /* get meta data directly */
        ret = get_metadata(&fek_meta, &fnek_meta, &ss_paths);
    }

    if (ret != 0) {
        goto err;
    }

    /* generate keys and add them to keyring */
    ret = generate_and_add_key_to_keyring(&fek_meta, &fnek_meta);

    if (ret != 0) {
        goto err;
    }

    /* mount the corresponding secret directory */
    ret = mount_secret_dir(uname, fek_meta.sig, fnek_meta.sig, &ss_paths);
    if (ret != 0) {
        goto err;
    }

    /* clean the meta data */
    memset((void *)&fek_meta, 0, sizeof(struct secstore_metadata));
    memset((void *)&fnek_meta, 0, sizeof(struct secstore_metadata));

    return 0;

err:
    fprintf(stderr, "Failed to mount uname:%s's secret dir\n", uname);

    return -1;
}

static int get_metadata(struct secstore_metadata *fek_meta,
                        struct secstore_metadata *fnek_meta,
                        struct secstore_paths *ss_paths)
{
    FILE *fd_fek_blob;
    FILE *fd_fnek_blob;
    FILE *fd_aes_blob;

    unsigned char fek_encrypt[SECSTORE_KEY_BYTES];
    unsigned char fnek_encrypt[SECSTORE_KEY_BYTES];

    struct passwd           *pws;
    struct stat              dir_st;
    struct aes_metadata      aes_meta;
    struct secstore_metadata encrypted_fek_meta;
    struct secstore_metadata encrypted_fnek_meta;

    fd_fek_blob  = fopen(ss_paths->fek_blob_path, "rb");
    fd_fnek_blob = fopen(ss_paths->fnek_blob_path, "rb");
    fd_aes_blob  = fopen(ss_paths->aes_blob_path, "rb");

    if ((fd_fek_blob == NULL) || (fd_fnek_blob == NULL) || (fd_aes_blob == NULL)) {
        goto error;
    }

    if (fread(&encrypted_fek_meta, sizeof(char), sizeof(struct secstore_metadata),
        fd_fek_blob) != sizeof(struct secstore_metadata)) {
        goto error;
    }

    if (fread(&encrypted_fnek_meta, sizeof(char), sizeof(struct secstore_metadata),
        fd_fnek_blob) != sizeof(struct secstore_metadata)) {
        goto error;
    }

    if (fread(&aes_meta, sizeof(char), sizeof(struct aes_metadata),
        fd_aes_blob) != sizeof(struct aes_metadata)) {
        goto error;
    }

    /* decrypt the fek and fnek */
    if (aes_decrypt_data((unsigned char *)(fek_meta), (unsigned char *)&encrypted_fek_meta,
                         sizeof(struct secstore_metadata), &aes_meta) != 0) {
        goto error;
    }

    if (aes_decrypt_data((unsigned char *)(fnek_meta), (unsigned char *)&encrypted_fnek_meta,
                         sizeof(struct secstore_metadata), &aes_meta) != 0) {
        goto error;
    }

    /* clean the aes metadata */
    memset((void *)&aes_meta, 0, sizeof(struct aes_metadata));

    fclose(fd_fek_blob);
    fclose(fd_fnek_blob);
    fclose(fd_aes_blob);

    return 0;

error:
    fprintf(stderr, "Failed to get meta data\n");

    if (fd_fek_blob) {
        fclose(fd_fek_blob);
    }

    if (fd_fnek_blob) {
        fclose(fd_fnek_blob);
    }

    if (fd_aes_blob) {
        fclose(fd_aes_blob);
    }

    return -1;
}

static int generate_and_add_key_to_keyring(struct secstore_metadata *fek_meta,
                                           struct secstore_metadata *fnek_meta)
{
    key_serial_t fek_tmp;
    key_serial_t fnek_tmp;

    char   fek_sig_hex[SECSTORE_SIG_BYTES_HEX + 1]  = "\0";
    char   fnek_sig_hex[SECSTORE_SIG_BYTES_HEX + 1] = "\0";
    struct ecryptfs_auth_tok fek_auth_tok;
    struct ecryptfs_auth_tok fnek_auth_tok;

    to_hex(fek_sig_hex, fek_meta->sig, SECSTORE_SIG_BYTES);
    to_hex(fnek_sig_hex, fnek_meta->sig, SECSTORE_SIG_BYTES);

    generate_payload(&fek_auth_tok, fek_sig_hex, fek_meta->salt, fek_meta->key);
    generate_payload(&fnek_auth_tok, fnek_sig_hex, fnek_meta->salt, fnek_meta->key);

    ecryptfs_add_auth_tok_to_keyring(&fek_auth_tok, fek_sig_hex);
    ecryptfs_add_auth_tok_to_keyring(&fnek_auth_tok, fnek_sig_hex);

    /* the keys should never be modified by user space apps, set permissions */
    fek_tmp  = keyctl_search(KEY_SPEC_USER_KEYRING, "user", fek_sig_hex, 0);
    fnek_tmp = keyctl_search(KEY_SPEC_USER_KEYRING, "user", fnek_sig_hex, 0);

    if (fek_tmp == -1 || fnek_tmp == -1) {
        fprintf(stderr, "Failed to search the added auth tok\n");
        return -1;
    }

    return 0;
}

static int mount_secret_dir(const char *uname, const char *fek_sig,
                            const char *fnek_sig, struct secstore_paths *ss_paths)
{
    int   result;
    char *options = NULL;

    char fek_sig_hex[SECSTORE_SIG_BYTES_HEX + 1]  = "\0";
    char fnek_sig_hex[SECSTORE_SIG_BYTES_HEX + 1] = "\0";

    key_serial_t fek_tmp;
    key_serial_t fnek_tmp;

    if (check_is_mounted(ss_paths->user_path) == 1) {
        fprintf(stderr, "Secret dir %s is already mounted\n", uname);
        return 0;
    }

    /* calculate the fek_sig_hex and fnse_sig_hex */
    to_hex(fek_sig_hex, fek_sig, SECSTORE_SIG_BYTES);
    to_hex(fnek_sig_hex, fnek_sig, SECSTORE_SIG_BYTES);

    if ((asprintf(&options, "ecryptfs_cipher=%s,ecryptfs_key_bytes=%d,ecryptfs_unlink_sigs,ecryptfs_sig=%s,ecryptfs_fnek_sig=%s",
                  SECSTORE_CIPHER, SECSTORE_KEY_BYTES, fek_sig_hex, fnek_sig_hex) < 0) || (options == NULL)) {
        fprintf(stderr, "Failed to build mount options\n");
        goto err;
    }

    if (mount(ss_paths->origin_path, ss_paths->secret_path, SECSTORE_FSTYPE, MS_NOSUID | MS_NODEV, options) != 0) {
        fprintf(stderr, "Failed to mount secret folder %s, the errno is %d\n",
                ss_paths->secret_path, errno);
        goto err;
    }

    free(options);

    return 0;
err:
    return -1;
}

static int aes_encrypt_data(unsigned char *out_buf, unsigned char *in_buf,
                            size_t count, struct aes_metadata *aes_meta)
{
    int encrypt_size  = 0;
    int encrypt_fsize = 0;

    EVP_CIPHER_CTX encryption_context;

    /* The length of input need to be multiple of aes block size */
    if (count % AES_BLOCK_SIZE != 0) {
        fprintf(stderr, "error: not padding\n");
        goto err;
    }

    EVP_CIPHER_CTX_init(&encryption_context);

    if (!EVP_EncryptInit_ex(&encryption_context, EVP_aes_128_cbc(),
                            NULL, aes_meta->key, aes_meta->iv)) {
        goto err;
    }

    EVP_CIPHER_CTX_set_padding(&encryption_context, 0);

    if (!EVP_EncryptUpdate(&encryption_context, out_buf, &encrypt_size, in_buf, count)) {
        goto err;
    }

    if (!EVP_EncryptFinal_ex(&encryption_context, out_buf + encrypt_size, &encrypt_fsize)) {
        goto err;
    }

    EVP_CIPHER_CTX_cleanup(&encryption_context);

    return 0;
err:
    EVP_CIPHER_CTX_cleanup(&encryption_context);

    return -1;
}

static int aes_decrypt_data(unsigned char *out_buf, unsigned char *in_buf,
                            size_t count, struct aes_metadata *aes_meta)
{
    int decrypt_size  = 0;
    int decrypt_fsize = 0;

    EVP_CIPHER_CTX decryption_context;

    /* The length of input need to be multiple of aes block size */
    if (count % AES_BLOCK_SIZE != 0) {
        fprintf(stderr, "error: not padding\n");
        return -1;
    }

    EVP_CIPHER_CTX_init(&decryption_context);

    if (!EVP_DecryptInit_ex(&decryption_context, EVP_aes_128_cbc(),
                            NULL, aes_meta->key, aes_meta->iv)) {
        goto err;
    }

    EVP_CIPHER_CTX_set_padding(&decryption_context, 0);

    if (!EVP_DecryptUpdate(&decryption_context, out_buf, &decrypt_size, in_buf, count)) {
        goto err;
    }

    if (!EVP_DecryptFinal_ex(&decryption_context, out_buf + decrypt_size, &decrypt_fsize)) {
        goto err;
    }

    EVP_CIPHER_CTX_cleanup(&decryption_context);

    return 0;
err:
    EVP_CIPHER_CTX_cleanup(&decryption_context);

    return -1;
}

static int secstore_init(struct secstore_metadata *fek_meta,
                         struct secstore_metadata *fnek_meta,
                         struct secstore_paths *ss_paths, const char *uname)
{
    int fd;
    int ret;

    FILE *fd_fek_blob  = NULL;
    FILE *fd_fnek_blob = NULL;
    FILE *fd_aes_blob  = NULL;

    unsigned char fek_encrypt[SECSTORE_KEY_BYTES];
    unsigned char fnek_encrypt[SECSTORE_KEY_BYTES];

    struct passwd           *pws;
    struct stat              dir_st;
    struct aes_metadata      aes_meta;
    struct secstore_metadata encrypted_fek_meta, encrypted_fnek_meta;

    /* set the owner of secure store. we do it here to make devtool happy */
    if ((strcmp(uname, "root") != 0) &&
        (strcmp(uname, SECSTORE_DEFAULT_USERNAME) != 0)) {
        pws = getpwnam(uname);
        if (pws == NULL) {
            goto error;
        }

        if (chown(ss_paths->user_path, pws->pw_uid, pws->pw_gid) != 0) {
            goto error;
        }

        if (chown(ss_paths->secret_path, pws->pw_uid, pws->pw_gid) != 0) {
            goto error;
        }

        if (chown(ss_paths->origin_path, pws->pw_uid, pws->pw_gid) != 0) {
            goto error;
        }
    }

    /* if the meta directory does not exist, create it */
    if (stat(ss_paths->meta_path, &dir_st) == -1) {
        if (mkdir(ss_paths->meta_path, 0700) != 0) {
            goto error;
        }
    } else if (!S_ISDIR(dir_st.st_mode)) {
        goto error;
    }

    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        goto error;
    }

    /* generate keys, signatures and salts with urandom */
    ret = read(fd, fek_meta, sizeof(struct secstore_metadata));
    if (ret != sizeof(struct secstore_metadata)) {
        close(fd);
        goto error;
    }

    ret = read(fd, fnek_meta, sizeof(struct secstore_metadata));
    if (ret != sizeof(struct secstore_metadata)) {
        close(fd);
        goto error;
    }

    ret = read(fd, &aes_meta, sizeof(struct aes_metadata));
    if (ret != sizeof(struct aes_metadata)) {
        close(fd);
        goto error;
    }

    /* The signature need to be unique */
    while (check_sigs(fek_meta->sig, fnek_meta->sig) != 0) {
        ret = read(fd, fek_meta, sizeof(struct secstore_metadata));
        if (ret != sizeof(struct secstore_metadata)) {
            close(fd);
            goto error;
        }

        ret = read(fd, fnek_meta, sizeof(struct secstore_metadata));
        if (ret != sizeof(struct secstore_metadata)) {
            close(fd);
            goto error;
        }
    }

    close(fd);

    /* remove the files if they already exist */
    if (access(ss_paths->fek_blob_path, F_OK) != -1) {
        if (remove(ss_paths->fek_blob_path) != 0) {
            goto error;
        }
    }

    if (access(ss_paths->fnek_blob_path, F_OK) != -1) {
        if (remove(ss_paths->fnek_blob_path) != 0) {
            goto error;
        }
    }

    if (access(ss_paths->aes_blob_path, F_OK) != -1) {
        if (remove(ss_paths->aes_blob_path) != 0) {
            goto error;
        }
    }

    /* save the key and the salt */
    fd_fek_blob  = fopen(ss_paths->fek_blob_path, "wb+");
    fd_fnek_blob = fopen(ss_paths->fnek_blob_path, "wb+");
    fd_aes_blob  = fopen(ss_paths->aes_blob_path, "wb+");

    if ((fd_fek_blob == NULL) || (fd_fnek_blob == NULL) || (fd_aes_blob == NULL)) {
        goto error;
    }

    /* encrypt the fek and fnek */
    if (aes_encrypt_data((unsigned char *)(&encrypted_fek_meta), (unsigned char *)fek_meta,
                         sizeof(struct secstore_metadata), &aes_meta) != 0) {
        goto error;
    }

    if (aes_encrypt_data((unsigned char *)(&encrypted_fnek_meta), (unsigned char *)fnek_meta,
                         sizeof(struct secstore_metadata), &aes_meta) != 0) {
        goto error;
    }

    if (fwrite(&encrypted_fek_meta, sizeof(char), sizeof(struct secstore_metadata), fd_fek_blob)
        != sizeof(struct secstore_metadata)) {
        goto error;
    }

    if (fwrite(&encrypted_fnek_meta, sizeof(char), sizeof(struct secstore_metadata), fd_fnek_blob)
        != sizeof(struct secstore_metadata)) {
        goto error;
    }

    if (fwrite(&aes_meta, sizeof(char), sizeof(struct aes_metadata), fd_aes_blob)
        != sizeof(struct aes_metadata)) {
        goto error;
    }

    /* clean the aes metadata */
    memset((void *)&aes_meta, 0, sizeof(struct aes_metadata));

    fclose(fd_fek_blob);
    fclose(fd_fnek_blob);
    fclose(fd_aes_blob);

    return 0;

error:
    fprintf(stderr, "Failed to initialize secure storage\n");

    if (fd_fek_blob) {
        fclose(fd_fek_blob);
    }

    if (fd_fnek_blob) {
        fclose(fd_fnek_blob);
    }

    if (fd_aes_blob) {
        fclose(fd_aes_blob);
    }

    return -1;
}

static int check_sigs(unsigned char *fek_sig, unsigned char *fnek_sig)
{
    char fek_sig_hex_tmp[SECSTORE_SIG_BYTES_HEX + 1]  = "\0";
    char fnek_sig_hex_tmp[SECSTORE_SIG_BYTES_HEX + 1] = "\0";

    key_serial_t fek_tmp;
    key_serial_t fnek_tmp;

    /* calculate the fek_sig_hex and fnse_sig_hex */
    to_hex(fek_sig_hex_tmp, fek_sig, SECSTORE_SIG_BYTES);
    to_hex(fnek_sig_hex_tmp, fnek_sig, SECSTORE_SIG_BYTES);

    /* the signatures should not be the same */
    if (strcmp(fek_sig_hex_tmp, fnek_sig_hex_tmp) == 0) {
        return -1;
    }

    fek_tmp = keyctl_search(KEY_SPEC_USER_KEYRING, "user", fek_sig_hex_tmp, 0);
    if (fek_tmp != -1) {
        return -1;
    }

    fnek_tmp = keyctl_search(KEY_SPEC_USER_KEYRING, "user", fnek_sig_hex_tmp, 0);

    if (fnek_tmp != -1) {
        return -1;
    }

    return 0;
}
