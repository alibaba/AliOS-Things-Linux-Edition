/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#ifndef SECSTORE_TEST_H
#define SECSTORE_TEST_H

#define TEST_DEFAULT_BUF_SIZE 64
#define TEST_MAX_BUF_SIZE     4096

#define \
    TEST(x) \
    ({ \
        if (x() != 0) \
        printf( "Test %s failed\n", #x); \
        else \
        printf( "Test %s success\n", #x); \
    })

#endif /* SECSTORE_TEST_H */
