/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cmd.h
*   \brief Command API
*   \author Montage
*/

#ifndef _CMD_H_
#define _CMD_H_

#define MAX_ARGV    8
#define CMD_BUF_SZ  256
#define CLI_HISTORY_MAX 5

typedef struct
{
    const char *cmd;
    int (*func) (int argc, char *argv[]);
    const char *msg;
} cmdt;

typedef struct _chain
{
    char buffer[CMD_BUF_SZ + 1];
    struct _chain *prev;
    struct _chain *next;
} str_chain;

extern short cmdt_sz;
extern cmdt *cmd_table;

#endif                          // _CMD_H_
