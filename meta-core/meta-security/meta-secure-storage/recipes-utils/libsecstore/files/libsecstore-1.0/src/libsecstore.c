/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include "libsecstore.h"
#include "libsecstore-internal.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include <mntent.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <unistd.h>

#include <linux/limits.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/* create Dirs recursively for the full path */
static int create_dirs(char *full_path, char *secstore_path, int shared);

/* get user name of current program */
static int get_user_name(char *uname);

/* check if a file is in secure storage. */
static int check_in_secstore(char *path_name, char *secstore_path);

extern int secure_store_read(const char *pathname, void *data, size_t count, int shared)
{
    int i;
    int fd;
    int ret;

    char *full_path;
    char  secstore_path[SECSTORE_PATH_MAX];
    char  uname[SECSTORE_UNAME_MAX + 1];
    int   read_count;

    size_t path_length;
    size_t temp_length;
    size_t full_length;

    struct stat dir_st;

    memset((void *)secstore_path, 0, sizeof(secstore_path));
    memset((void *)uname, 0, sizeof(uname));

    if ((pathname == NULL) || (data == NULL) || (count > SECSTORE_DATA_MAX_LENGTH)) {
        return SECSTORE_ERROR_INPUT;
    }

    path_length = strlen(pathname);

    if (path_length > PATH_MAX - SECSTORE_SECRET_NAME_LEN -
        SECSTORE_BASE_PATH_LEN - SECSTORE_UNAME_MAX) {
        return SECSTORE_ERROR_INPUT;
    }

    if (get_user_name(uname) != 0) {
        return SECSTORE_ERROR_OTHER;
    }

    if (pathname[path_length - 1] == '/') {
        return SECSTORE_ERROR_INPUT;
    }

    if (shared == 0) {
        /* using user's private secure storage */
        strncpy(secstore_path, SECSTORE_BASE_PATH, SECSTORE_PATH_MAX - 1);
        temp_length = strlen(secstore_path);
        strncat(secstore_path, uname, SECSTORE_PATH_MAX - temp_length - 1);
    } else if (shared == 1) {
        /* using shared secure storage */
        strncpy(secstore_path, SECSTORE_BASE_PATH, SECSTORE_PATH_MAX - 1);
        temp_length = strlen(secstore_path);
        strncat(secstore_path, SECSTORE_DEFAULT_USERNAME, SECSTORE_PATH_MAX - temp_length - 1);
    } else {
        return SECSTORE_ERROR_INPUT;
    }

    if ((stat(secstore_path, &dir_st) == -1) || (!S_ISDIR(dir_st.st_mode))) {
        return SECSTORE_ERROR_UNAVAILABLE;
    }

    if (check_is_mounted(secstore_path) == 0) {
        return SECSTORE_ERROR_UNAVAILABLE;
    }

    temp_length = strlen(secstore_path);
    strncat(secstore_path, SECSTORE_SECRET_DIR_BASE, SECSTORE_PATH_MAX - temp_length - 1);

    full_length = strlen(secstore_path) + path_length + 1;
    full_path = malloc(full_length);
    strncpy(full_path, secstore_path, full_length);

    temp_length = strlen(full_path);
    strncat(full_path, pathname, full_length - temp_length - 1);

    /* ensure the file is in secret dir */
    if (check_in_secstore(full_path, secstore_path) != 1) {
        free(full_path);
        return SECSTORE_ERROR_FAILED;
    }

    fd = open(full_path, O_RDONLY);

    free(full_path);

    if (fd == -1) {
        if (errno == EISDIR) {
            return SECSTORE_ERROR_ISDIR;
        }
        return SECSTORE_ERROR_FAILED;
    }

    /* read in sliced small part */
    read_count = 0;

    for (i = 0 ; i < count / PAGE_SIZE; ++i) {
        ret = read(fd, data + i * PAGE_SIZE, PAGE_SIZE);
        if (ret == -1) {
            close(fd);
            if (errno == EISDIR) {
                return SECSTORE_ERROR_ISDIR;
            }
            return SECSTORE_ERROR_OTHER;
        } else {
            read_count += ret;
        }
    }

    ret = read(fd, data + i * PAGE_SIZE, count % PAGE_SIZE);
    if (ret == -1) {
        close(fd);
        if (errno == EISDIR) {
            return SECSTORE_ERROR_ISDIR;
        }
        return SECSTORE_ERROR_OTHER;
    } else {
        read_count += ret;
    }

    close(fd);

    return read_count;
}

extern int secure_store_write(const char *pathname, void *data, size_t count, int shared)
{
    int i;
    int fd;
    int ret;

    char *full_path;
    char  secstore_path[SECSTORE_PATH_MAX];
    char  uname[SECSTORE_UNAME_MAX + 1];
    int   write_count;

    size_t path_length;
    size_t temp_length;
    size_t full_length;

    struct stat dir_st;

    memset((void *)secstore_path, 0, sizeof(secstore_path));
    memset((void *)uname, 0, sizeof(uname));

    if ((pathname == NULL) || (data == NULL) || (count > SECSTORE_DATA_MAX_LENGTH)) {
        return SECSTORE_ERROR_INPUT;
    }

    path_length = strlen(pathname);

    if (path_length > PATH_MAX - SECSTORE_SECRET_NAME_LEN -
        SECSTORE_BASE_PATH_LEN - SECSTORE_UNAME_MAX) {
        return SECSTORE_ERROR_INPUT;
    }

    if (get_user_name(uname) != 0) {
        return SECSTORE_ERROR_OTHER;
    }

    if (pathname[path_length - 1] == '/') {
        return SECSTORE_ERROR_INPUT;
    }

    if (shared == 0) {
        /* using user's private secure storage */
        strncpy(secstore_path, SECSTORE_BASE_PATH, SECSTORE_PATH_MAX - 1);
        temp_length = strlen(secstore_path);
        strncat(secstore_path, uname, SECSTORE_PATH_MAX - temp_length - 1);
    } else if (shared == 1) {
        /* using shared secure storage */
        strncpy(secstore_path, SECSTORE_BASE_PATH, SECSTORE_PATH_MAX - 1);
        temp_length = strlen(secstore_path);
        strncat(secstore_path, SECSTORE_DEFAULT_USERNAME, SECSTORE_PATH_MAX - temp_length - 1);
    } else {
        return SECSTORE_ERROR_INPUT;
    }

    if (check_is_mounted(secstore_path) == 0) {
        return SECSTORE_ERROR_UNAVAILABLE;
    }

    temp_length = strlen(secstore_path);
    strncat(secstore_path, SECSTORE_SECRET_DIR_BASE, SECSTORE_PATH_MAX - temp_length - 1);

    full_length = strlen(secstore_path) + path_length + 1;
    full_path = malloc(full_length);
    strncpy(full_path, secstore_path, full_length);

    temp_length = strlen(full_path);
    strncat(full_path, pathname, full_length - temp_length - 1);

    /* ensure the file is in secret dir */
    if (check_in_secstore(full_path, secstore_path) == -1) {
        free(full_path);
        return SECSTORE_ERROR_FAILED;
    }

    fd = open(full_path, O_WRONLY | O_TRUNC, 0600);

    /* try to create a new file */
    if (fd == -1) {
        if (errno == EISDIR) {
            free(full_path);
            return SECSTORE_ERROR_ISDIR;
        }

        fd = open(full_path, O_WRONLY | O_CREAT, 0600);
        if ((fd != -1) && (shared == 1)) {
            if (fchmod(fd, 0660) != 0) {
                free(full_path);
                close(fd);
                return SECSTORE_ERROR_OTHER;
            }
        }
    }

    /* create dirs and a new file */
    if (fd == -1) {
        if (errno == ENOENT) {
            if (create_dirs(full_path, secstore_path, shared) == -1) {
                free(full_path);
                return SECSTORE_ERROR_FAILED;
            }
        }

        /* ensure the file is in secret dir */
        if (check_in_secstore(full_path, secstore_path) != 1) {
            free(full_path);
            return SECSTORE_ERROR_FAILED;
        }

        fd = open(full_path, O_CREAT | O_WRONLY, 0600);
        if ((fd != -1) && (shared == 1)) {
            if (fchmod(fd, 0660) != 0) {
                close(fd);
                free(full_path);
                return SECSTORE_ERROR_OTHER;
            }
        }

        if (fd == -1) {
            free(full_path);
            return SECSTORE_ERROR_FAILED;
        }
    }

    free(full_path);

    /* write in sliced small part */
    write_count = 0;

    for (i = 0 ; i < count / PAGE_SIZE; ++i) {
        ret = write(fd, data + i * PAGE_SIZE, PAGE_SIZE);
        if (ret == -1) {
            close(fd);
            return SECSTORE_ERROR_OTHER;
        } else {
            write_count += ret;
        }
    }

    ret = write(fd, data + i * PAGE_SIZE, count % PAGE_SIZE);
    if (ret == -1) {
        close(fd);
        return SECSTORE_ERROR_OTHER;
    } else {
        write_count += ret;
    }
    close(fd);

    return write_count;
}

static int get_user_name(char *uname)
{
    int   fd;
    uid_t uid;

    struct passwd *pws;

    uid = geteuid();
    pws = getpwuid(uid);

    if (pws == NULL) {
        goto err;
    }

    if (pws->pw_name == NULL) {
        goto err;
    }

    if (strncpy(uname, pws->pw_name, SECSTORE_UNAME_MAX) != uname) {
        goto err;
    }

    return 0;
err:
    return -1;
}

static int create_dirs(char *full_path, char *secstore_path, int shared)
{
    char  *path_tmp;
    char  *p = NULL;
    char   tmp[PATH_MAX];
    size_t path_len;

    struct stat dir_st;

    memset((void *)tmp, 0, sizeof(tmp));

    strncpy(tmp, full_path, PATH_MAX - 1);
    path_tmp = dirname(tmp);

    if (path_tmp == NULL) {
        return -1;
    }

    path_len = strlen(path_tmp);
    if (path_tmp[path_len - 1] == '/') {
            path_tmp[path_len - 1] = 0;
    }

    for (p = path_tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (stat(path_tmp, &dir_st) == -1) {
                /* ensure the file is in secret dir */
                if (check_in_secstore(path_tmp, secstore_path) != 1) {
                    return -1;
                }

                if (mkdir(path_tmp, S_IRWXU) != 0) {
                    return -1;
                }

                if (shared == 1) {
                    if (chmod(path_tmp, S_IRWXU | S_IRWXG) != 0) {
                        return -1;
                    }
                }
            } else if (!S_ISDIR(dir_st.st_mode)) {
                *p = '/';
                return -1;
            }
            *p = '/';
        }
    }

    if (stat(path_tmp, &dir_st) == -1) {
        if (mkdir(path_tmp,  S_IRWXU) != 0) {
            return -1;
        }

        if (shared == 1) {
            if (chmod(path_tmp, S_IRWXU | S_IRWXG) != 0) {
                return -1;
            }
        }
    } else if (!S_ISDIR(dir_st.st_mode)) {
        return -1;
    }

    return 0;
}

int check_is_mounted(char *dir)
{
    char   src[PATH_MAX];
    char   dest[PATH_MAX];
    FILE  *fp        = NULL;
    struct mntent *m = NULL;

    size_t temp_length;

    memset((void *)src, 0, sizeof(src));
    memset((void *)dest, 0, sizeof(dest));

    strncpy(src , dir, PATH_MAX - 1);
    temp_length = strlen(src);
    strncat(src, SECSTORE_ORIGIN_NAME, PATH_MAX - temp_length - 1);

    strncpy(dest, dir, PATH_MAX - 1);
    temp_length = strlen(dest);
    strncat(dest, SECSTORE_SECRET_NAME, PATH_MAX - temp_length - 1);

    fp = setmntent("/proc/mounts", "r");

    if (fp == NULL) {
        return 0;
    }

    flockfile(fp);

    while ((m = getmntent(fp)) != NULL) {
        if (strcmp(m->mnt_type, "ecryptfs") != 0) {
            continue;
        } else if ((strcmp(m->mnt_fsname, src) == 0) &&
                   (strcmp(m->mnt_dir, dest) == 0)) {
            return 1;
        }
    }

    endmntent(fp);

    return 0;
}

static int check_in_secstore(char *path_name, char *secstore_path) {
    char *path_temp;
    char  dir_real_path[PATH_MAX];

    size_t path_length;
    size_t dir_length;

    path_length = strlen(path_name) + 1;
    path_temp = malloc(path_length);
    strncpy(path_temp, path_name, path_length);

    /* check if the file is in secret dir */
    if (realpath(dirname(path_temp), dir_real_path) == dir_real_path) {
        dir_length = strlen(dir_real_path);
        strncat(dir_real_path, "/", PATH_MAX - dir_length - 1);
        if (strstr(dir_real_path, secstore_path) == NULL) {
            free(path_temp);
            return -1;
        }
        strncpy(path_temp, path_name, path_length);
        /* dir .. is out of secure storage */
        if (strcmp(basename(path_temp), "..") == 0) {
            free(path_temp);
            return -1;
        }
    } else {
        free(path_temp);
        return 0;
    }

    free(path_temp);

    return 1;
}
