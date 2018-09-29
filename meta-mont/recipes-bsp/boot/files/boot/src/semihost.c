/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file semihost.c
*   \brief IO function in semihost mode
*   \author Montage
*/
#include "config.h"

#ifdef  CONFIG_SEMI_HOST

#define MAXLN   0x100

void gets(char *buf)
{
    unsigned long conf[3] = { 1, buf, MAXLN };

    __asm__ __volatile__("mov r0,#6 \n"
                         "mov r1,%[input]\n"
                         "swi #0x123456"::[input] "r"(conf));
    return;
}

int getchar(void)
{
    char c;

    __asm__ __volatile__("mov r0,#7 \n"
                         "mov r1,#0\n"
                         "swi #0x123456\n"
                         "mov %[output],r0":[output] "=r"(c):);

    return (c);
}

void puts(char *s)
{
    __asm__ __volatile__("mov r0,#4 \n" "mov r1,%0\n" "swi #0x123456"::"r"(s));

}

void putchar(int c)
{
    __asm__ __volatile__("mov r0,#3 \n"
                         "mov r1,%0\n" "swi #0x123456\n"::"r"(c));
    return;
}

void serial_init()
{
}

#endif                          //CONFIG_SEMI_HOST
