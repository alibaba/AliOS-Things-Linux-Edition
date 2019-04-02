/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdio.h>
#include <sys/user.h>

#include "libsecstore.h"

#include "test.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

int test_read_write_private1(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("testdir1/testdir2/testdir3/file4", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("testdir1/testdir2/testdir3/file4", buffer_out, count, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_read_write_private2(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("testfile1", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

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


int test_read_write_dot_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("./testfile2", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("./testfile2", buffer_out, count, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_read_write_dir_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("./testdir/testfile3", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("./testdir/testfile3", buffer_out, count, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_read_write_abs_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "World! Hello";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("/testfile4", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("/testfile4", buffer_out, count, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_read_write_wired1(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "World! Hello";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("/testdir1//testfile5", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("/testdir1//testfile5", buffer_out, count, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_read_write_wired2(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "World! Hello";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("//testdir2//testfile6", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("//testdir2//testfile6", buffer_out, count, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_read_no_file(void)
{
    int ret = 0;

    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    ret = secure_store_read("/file_no_exist", buffer_out, 10, 0);

    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int test_read_too_much(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read("testfile1", buffer_out, count + 1000, 0);

    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_null_buf(void)
{
    int    ret   = 0;
    size_t count = 0;

    char  buffer_in[TEST_DEFAULT_BUF_SIZE] = "Hello world!";
    char *buffer_out;

    count = strlen(buffer_in);

    ret = secure_store_read("testfile1", buffer_out, strlen(buffer_in), 0);

    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    ret = secure_store_write("testfile1", buffer_out, strlen(buffer_in), 0);

    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    return 0;
}

int test_over_write(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Overwritten!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("testfile1", buffer_in, count, 0);

    if (ret != count) {
        return -1;
    }

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

int test_write_nothing(void)
{
    int ret = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE] = "\0";

    ret = secure_store_write("testfile1", buffer_in, 0, 0);

    if (ret != 0) {
        return -1;
    }

    return 0;
}

int test_read_nothing(void)
{
    int ret = 0;

    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    ret = secure_store_read("testfile1", buffer_out, 0, 0);

    if (ret != 0) {
        return -1;
    }

    return 0;
}

int test_long_path(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer[TEST_MAX_BUF_SIZE];
    char buffer_out[TEST_DEFAULT_BUF_SIZE]  = "test";

    for (int i = 0; i < TEST_MAX_BUF_SIZE - 1; i++) {
        buffer[i] = 'd';
    }
    buffer[TEST_MAX_BUF_SIZE] = "\0";
    count = strlen(buffer);

    ret = secure_store_read(buffer, buffer_out, count, 0);

    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    ret = secure_store_write(buffer, buffer_out, count, 0);

    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    return 0;
}

int test_write_read_dir1(void)
{
    size_t count = 0;
    int    ret   = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "input";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read("testdir1", buffer_in, count, 0);

    if (ret != SECSTORE_ERROR_ISDIR) {
        return -1;
    }

    ret = secure_store_write("testdir1", buffer_out, count, 0);

    if (ret != SECSTORE_ERROR_ISDIR) {
        return -1;
    }

    return 0;
}

int test_write_read_dir2(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "input";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read(".", buffer_in, count, 0);

    if (ret != SECSTORE_ERROR_ISDIR) {
        return -1;
    }


    ret = secure_store_write(".", buffer_out, count, 0);

    if (ret != SECSTORE_ERROR_ISDIR) {
        return -1;
    }

    return 0;
}

int test_write_read_dir3(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "input";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read("..", buffer_in, count, 0);

    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_write("..", buffer_out, count, 0);

    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int test_write_read_dir4(void)
{
    size_t count = 0;
    int    ret   = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "input";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read("../..", buffer_in, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_write("../..", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int test_write_read_dir5(void)
{
    size_t count = 0;
    int    ret   = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "input";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_read("testdir/../..", buffer_in, count, 0);

    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_write("testdir/../..", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int test_write_big_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char *buffer_in = malloc(PAGE_SIZE * 4);

    memset(buffer_in, 'a', PAGE_SIZE * 4);
    buffer_in[PAGE_SIZE * 4 - 1] = '\0';
    count = strlen(buffer_in) + 1;

    ret = secure_store_write("bigfile", buffer_in, count, 0);
    if (ret != count) {
        return -1;
    }

    free(buffer_in);

    return 0;
}

int test_read_big_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char *buffer_in  = malloc(PAGE_SIZE * 4);
    char *buffer_out = malloc(PAGE_SIZE * 4);

    memset(buffer_in, 'a', PAGE_SIZE * 4);
    buffer_in[PAGE_SIZE * 4 - 1] = '\0';
    count = strlen(buffer_in) + 1;

    ret = secure_store_read("bigfile", buffer_out, count, 0);
    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    free(buffer_in);
    free(buffer_out);

    return 0;
}

int test_write_too_big_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char *buffer_in  = malloc(SECSTORE_DATA_MAX_LENGTH * 2);
    char *buffer_out = malloc(SECSTORE_DATA_MAX_LENGTH * 2);

    memset(buffer_in, 'a', SECSTORE_DATA_MAX_LENGTH * 2);
    buffer_in[SECSTORE_DATA_MAX_LENGTH * 2 - 1] = '\0';
    count = strlen(buffer_in) + 1;

    ret = secure_store_read("toobigfile", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    free(buffer_in);
    free(buffer_out);

    return 0;
}

int test_read_too_big_file(void)
{
    int    ret   = 0;
    size_t count = 0;

    char *buffer_in  = malloc(SECSTORE_DATA_MAX_LENGTH * 2);
    char *buffer_out = malloc(SECSTORE_DATA_MAX_LENGTH * 2);

    memset(buffer_in, 'a', SECSTORE_DATA_MAX_LENGTH * 2);
    buffer_in[SECSTORE_DATA_MAX_LENGTH * 2 - 1] = '\0';

    count = strlen(buffer_in);

    ret = secure_store_read("toobigfile", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    free(buffer_in);
    free(buffer_out);

    return 0;
}

int test_write_read_outside(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("../outfile", buffer_in, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_read("../outfile", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int test_write_read_outside_in(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("../../root/secret/infile", buffer_in, count, 0);
    if (ret != count) {
        return -1;
    }

    ret = secure_store_read("../../root/secret/infile", buffer_out, count, 0);
    if (ret != count) {
        return -1;
    }

    buffer_out[count] = 0;

    if (strcmp(buffer_in, buffer_out) != 0) {
        return -1;
    }

    return 0;
}

int test_write_read_like_dir1(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("likedir/", buffer_in, count, 0);
    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    ret = secure_store_read("likedir/", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_INPUT) {
        return -1;
    }

    return 0;
}

int test_write_read_like_dir2(void)
{
    int    ret   = 0;
    size_t count = 0;

    char buffer_in[TEST_DEFAULT_BUF_SIZE]  = "Hello world!";
    char buffer_out[TEST_DEFAULT_BUF_SIZE] = "\0";

    count = strlen(buffer_in);

    ret = secure_store_write("/..", buffer_in, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    ret = secure_store_read("/..", buffer_out, count, 0);
    if (ret != SECSTORE_ERROR_FAILED) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    TEST(test_read_write_private1);
    TEST(test_read_write_private2);
    TEST(test_read_write_dot_file);
    TEST(test_read_write_dir_file);
    TEST(test_read_write_abs_file);
    TEST(test_read_write_wired1);
    TEST(test_read_write_wired2);
    TEST(test_read_no_file);
    TEST(test_read_too_much);
    TEST(test_over_write);
    TEST(test_write_nothing);
    TEST(test_read_nothing);
    TEST(test_long_path);
    TEST(test_write_read_dir1);
    TEST(test_write_read_dir2);
    TEST(test_write_read_dir3);
    TEST(test_write_read_dir4);
    TEST(test_write_read_dir5);
    TEST(test_write_big_file);
    TEST(test_read_big_file);
    TEST(test_read_too_big_file);
    TEST(test_write_too_big_file);
    TEST(test_write_read_outside);
    TEST(test_write_read_outside_in);
    TEST(test_write_read_like_dir1);
    TEST(test_write_read_like_dir2);
}
