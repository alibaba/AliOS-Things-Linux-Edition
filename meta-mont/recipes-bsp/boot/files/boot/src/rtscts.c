/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file rtscts.c
*   \brief mptool cmd 
*   \author Montage
*/

#ifdef RTS_CTS_TEST
//#include <arch/chip.h>
#include <common.h>
#include <lib.h>

extern char rts_cts_test;

void rts_cts_start(void)
{
    int i, t;

    rts_cts_test = 1;
    for(i=0; i < 100; i++)
    {
        t = '0' + (i % 0x4f);
        serial_putc(t);
    }
}

int rtscts_cmd(int argc, char *argv[])
{
    if (0 < argc)
    {
        if (!strcmp(argv[0], "start"))
            rts_cts_start();
        else if (!strcmp(argv[0], "stop"))
            rts_cts_test = 0;
    }

    return ERR_OK;
}

cmdt cmdt_rtscts __attribute__ ((section("cmdt"))) =
{
"rtscts", rtscts_cmd, "rtscts [start/stop] -- mptool"};

#endif

