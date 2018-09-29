/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cmd_proc.c
*   \brief Command Parser
*   \author Montage
*/

#include    <common.h>
#include    <lib.h>

#define     PROMPT      CONFIG_PROMPT
extern char __stop_cmdt[];
extern char __start_cmdt[];
short cmdt_sz;
cmdt *cmd_table = (cmdt *) & __start_cmdt[0];

extern int cli_gets(char *buf, str_chain * p_cmd, char peek);

char *err_str[] = {
    "argument",
    "unalign",
    "address",
    "file",
    "timeout",
    "ethernet",
    "memory",
};

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int cmd_proc(int argc, char **argv)
{
    cmdt *cp, *match;
    int i, j, match_len, rc;

    cp = &cmd_table[0];
    for (match = 0, match_len = i = 0; i < cmdt_sz; i++, cp++)
    {
        if (!strcmp(cp->cmd, argv[0]))
        {
            match = cp;
            goto match;
        }
        for (j = 0; j < 8; j++)
            if (argv[0][j] != cp->cmd[j])
                break;
        if (argv[0][j] && cp->cmd[j])   /* don't count match_len for a wrong command */
            continue;
        if (match_len < j)
        {
            match_len = j;      /* most likely to be match */
            match = cp;
        }
    }

    if (!match)
        printf("Command not found!\n\r");
    else
    {
      match:
        rc = match->func(argc - 1, argv + 1);
        if (rc == ERR_HELP)
            printf("%s", match->msg);
        else if (rc == ERR_OK)
            ;
            //printf("OK");
        else if (0 > rc)
        {
            i = 0 - rc - 1;
            printf("ERR: ");
            if (i < (0 - ERR_LAST))
                printf("%s\n", err_str[i]);
        }
        //printf("\n\r");
        return rc;
    }
    return -1;
}

#ifdef  CONFIG_CLI_HISTORY
/*!
 * function:
 *
 *  \brief
 *  \param history
 *  \return
 */
static void history_init(str_chain history[])
{
    int i;

    for (i = 0; i < (CLI_HISTORY_MAX - 1); i++)
    {
        history[i].next = &history[i + 1];
        history[i + 1].prev = &history[i];
        history[i].buffer[0] = 0x00;
    }
    history[CLI_HISTORY_MAX - 1].next = &history[0];
    history[0].prev = &history[CLI_HISTORY_MAX - 1];
    history[CLI_HISTORY_MAX - 1].buffer[0] = 0x00;

}
#endif

/*!
 * function:
 *
 *  \brief
 *  \return void
 */
void cmd_loop(void)
{
    char *argv[10], buffer[CMD_BUF_SZ + 1];
    int argc;
    str_chain *phistory = 0x00;
#ifdef  CONFIG_CLI_HISTORY
    str_chain history[CLI_HISTORY_MAX];
    history_init(history);
    phistory = &history[0];
#endif

    memset(buffer, 0, CMD_BUF_SZ);

    for (;;)
    {
        printf(PROMPT);
        cli_gets(buffer, phistory, 0);
//copy buf to history_buf
#ifdef  CONFIG_CLI_HISTORY
        if (buffer[0])          // & strcmp(buffer,phistory->buffer))
        {
            strcpy(phistory->buffer, buffer);
            phistory = phistory->next;
        }
#endif

#if 1
        putchar('\n');
        putchar('\r');
#else
        printf("\n");
#endif

        if (1 > (argc = get_args(&buffer[0], argv)))
            continue;
        cmd_proc(argc, argv);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void cmd_init()
{
    cmdt_sz = (&__stop_cmdt[0] - &__start_cmdt[0]) / sizeof (cmdt);
    //printf("%s %d cmds at %x\n", __func__, (int) cmdt_sz, &cmd_table[0]);
}
