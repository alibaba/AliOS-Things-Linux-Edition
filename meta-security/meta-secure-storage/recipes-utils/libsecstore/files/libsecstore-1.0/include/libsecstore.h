/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef SECSTORE_LIB_H
#define SECSTORE_LIB_H

#include <stddef.h>

#define SECSTORE_ERROR_INPUT       -1 /* Incorrect input format */
#define SECSTORE_ERROR_FAILED      -2 /* Failed to read or write the file */
#define SECSTORE_ERROR_ISDIR       -3 /* Filename refers to a directory */
#define SECSTORE_ERROR_UNAVAILABLE -4 /* Secure storage is unavailable */
#define SECSTORE_ERROR_OTHER       -5 /* Other error occurs */

#ifndef SECSTORE_DATA_MAX_LENGTH
#define SECSTORE_DATA_MAX_LENGTH 65536 /* Max length of Data Read/Write */
#endif

#ifdef _cpluscplus
extern "c" {
#endif

/**
 * @brief  read from the specified file in secure storage
 *
 * @note   the length of buffer need to be multiple of aes block size
 *
 * @param  filename  relative Path of the file in secure storage
 * @param  data      fuffer to store the data
 * @param  size      number of bytes to read, which is limited to DATA_MAX_LENTH
 * @param  shared    write to the shared or private secure storage
 *                   1 stands for shared, 0 stands for private
 *
 * @return  the number of bytes actually read on success,
 *          zero while reading end of file,
 *          negative error integer on error
 */
int secure_store_read(const char *filename, void *data, size_t size, int shared);

/**
 * @brief  write to the specified file in secure storage
 *
 * @param  filename  relative Path of the file in secure storage
 * @param  data      Buffer which stores the data to write
 * @param  size      Number of bytes to write, which is limited to DATA_MAX_LENTH
 * @param  shared    Write to the shared or private secure storage
 *                   1 stands for shared, 0 stands for private
 *
 * @return  the number of bytes written on success,
 *          zero while writting notheing,
 *          negative error integer on error
 */
int secure_store_write(const char *filename, void *data, size_t size, int shared);

#ifdef __cplusplus
}
#endif

#endif /* SECSTORE_LIB_H */
