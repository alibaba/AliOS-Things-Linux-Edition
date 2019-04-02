/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef SECSTORE_MANAGER_H
#define SECSTORE_MANAGER_H

#include <stddef.h>

#include "libsecstore-internal.h"

/* metadata of secure store */
struct secstore_metadata {
    unsigned char key[SECSTORE_KEY_BYTES];
    unsigned char sig[SECSTORE_SIG_BYTES];
    unsigned char salt[SECSTORE_SALT_BYTES];
};

/* metadata of openssl aes */
struct aes_metadata {
    unsigned char key[SECSTORE_KEK_BYTES];
    unsigned char iv[SECSTORE_IV_BYTES];
};

/* paths of metadata in secure storage */
struct secstore_paths {
    char aes_blob_path[SECSTORE_PATH_MAX];
    char fek_blob_path[SECSTORE_PATH_MAX];
    char fnek_blob_path[SECSTORE_PATH_MAX];
    char origin_path[SECSTORE_PATH_MAX];
    char secret_path[SECSTORE_PATH_MAX];
    char meta_path[SECSTORE_PATH_MAX];
    char user_path[SECSTORE_PATH_MAX];
};

#endif /* SECSTORE_MANAGER_H */
