/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file irq.h
*   \brief Interrupt API
*   \author Montage
*/

#ifndef IRQ_H
#define IRQ_H

#include <arch/cpu.h>
#include <arch/trap.h>

#define IRQ_MAX     32

void irq_init();
void irq_enable();
void request_irq(unsigned int irq, void (*handler) (), void *id);
// disable and save the orginal status
unsigned int irq_save();
// restore cpu's status
void irq_restore(unsigned int org);
void enable_irq(int irq);   // unmask interrupt
void disable_irq(int irq);  // mask interrupt
void free_irq(unsigned int intr, void *id);

#endif                          //IRQ_H
