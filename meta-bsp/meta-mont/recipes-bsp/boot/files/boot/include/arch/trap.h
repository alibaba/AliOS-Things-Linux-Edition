/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file trap.h
*   \brief Trap API
*   \author Montage
*/

#ifndef TRAP_H
#define TRAP_H

#include <arch/cpu.h>

#define EXP_MAX 16
typedef struct irq_handler
{
    unsigned int hnd;
    unsigned int id;
} irq_handler;

extern irq_handler ex_tables[EXP_MAX];

void set_ex_handler(unsigned int exp, void (*handler) (), void *id);
void trap_init();

#endif
