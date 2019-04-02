/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdio.h>
#include <sys/user.h>

#include "libsecstore.h"

#include "test.h"

int test_read_write_private(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Test String!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("testprivatefile", buffer_in, count, 0);
    if ( ret != count) {
        return -1;
    }

    ret = secure_store_read("testprivatefile", buffer_out, count, 0);
    if ( ret != count) {
        return -1;
    }

    buffer_out[count] = 0;
    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_over_write_private(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE] = "OverWrite Test String!";

    count = strlen(buffer_in);

    ret = secure_store_write("testfile1", buffer_in, count, 0);
    if (ret != count) {
        return -1;
    }

    return 0;
}

int test_read_over_write_private(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "OverWrite Test String!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read("testfile1", buffer_out, count, 0);
    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    TEST(test_read_write_private);
    TEST(test_over_write_private);
    TEST(test_read_over_write_private);
}
