/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef SECSTORE_LIB_INTERNAL_H
#define SECSTORE_LIB_INTERNAL_H

#define SECSTORE_DEFAULT_USERNAME "shared"

#define SECSTORE_AES_BLOCK_BYTES 16
#define SECSTORE_KEY_BYTES       32
#define SECSTORE_KEK_BYTES       16
#define SECSTORE_SIG_BYTES       8
#define SECSTORE_SIG_BYTES_HEX   16
#define SECSTORE_SALT_BYTES      8
#define SECSTORE_IV_BYTES        16

#define SECSTORE_SECRET_NAME_LEN 8
#define SECSTORE_BASE_PATH_LEN   10
#define SECSTORE_UNAME_MAX       32
#define SECSTORE_PATH_MAX        128

#define SECSTORE_CIPHER "aes"
#define SECSTORE_FSTYPE "ecryptfs"

#define SECSTORE_BASE_PATH       "/secstore/"
#define SECSTORE_DIR_NAME        "/secstore"
#define SECSTORE_FEK_BLOB_NAME   "/fekblob"
#define SECSTORE_FNEK_BLOB_NAME  "/fnekblob"
#define SECSTORE_AES_BLOB_NAME   "/aesblob"
#define SECSTORE_META_NAME       "/keyvault"
#define SECSTORE_ORIGIN_NAME     "/.secret"
#define SECSTORE_SECRET_NAME     "/secret"
#define SECSTORE_SECRET_DIR_BASE "/secret/"

/**
 * @brief  check if a user's secret directory is mounted
 *
 * @param  dir  name of the user directory
 *
 * @return  1 if mounted, 0 if not mounted
 *
 */
int check_is_mounted(char *dir);

#endif /* SECSTORE_LIB_INTERNAL_H */
