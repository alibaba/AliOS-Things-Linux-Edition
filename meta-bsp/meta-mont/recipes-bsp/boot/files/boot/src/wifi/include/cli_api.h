/*
	cli_api.h
*/
#ifndef	CLI_API_H
#define CLI_API_H

typedef struct _cmdt
{
	char *name;
	int (*func)(int argc, char *argv[]); 
	char *usage;
} cmdt;

typedef struct _chain
{
    char buffer[257];
    struct _chain *prev;
    struct _chain *next;
} str_chain;

enum
{
	CLI_FLAG_HIDDEN=(1<<0),		// hidden
	CLI_FLAG_WIDE=(1<<3),		// wildcard
};

enum
{
	CLI_OK = 0,
	CLI_SHOW_USAGE = 1,
	CLI_ERROR = -1,
	CLI_ERR_PARM = -2,
	CLI_ERR_ADDR = -3,
	CLI_ERR_UNALIGN = -4,
};

#define CMD_DECL(x)   int x(int argc, char *argv[])

#define CLI_MAX_BUF_SIZE 255
#define	CLI_NUM_OF_CMDS(cmd)	(sizeof(cmd)/sizeof(cmdt))
#define	CLI_CMD(name, func, usage) \
	cmdt cmdt_##name __attribute__ ((section("cmdt"))) = {#name, func, usage}

int cli_handle(cmdt * start_cmds, char *cmd_buf);

#endif	//CLI_API_H

