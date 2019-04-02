/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdio.h>
#include <sys/user.h>

#include "libsecstore.h"

#include "test.h"

int test_read_shared_fail(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Test String!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("shareddir/testsharedfile", buffer_in, count, 1);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_read("shareddir/testsharedfile", buffer_out, count, 1);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int test_write_shared_fail(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Test String!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("shareddir/testsharedfile1", buffer_in, count, 1);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_read("shareddir/testsharedfile1", buffer_out, count, 1);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    TEST(test_read_shared_fail);
    TEST(test_write_shared_fail);
}
