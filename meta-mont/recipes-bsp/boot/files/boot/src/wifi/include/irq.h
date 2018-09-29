/*=============================================================================+
|                                                                              |
| Copyright 2017                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file irq.h
*   \brief interrupt routine API
*   \author Montage
*/
#ifndef __IRQ_H__
#define __IRQ_H__

#define INTR_NUM_BASE       0x6
#define WIFIMAC_INTR_NUM    (INTR_NUM_BASE + 14)
#define PCM_INTR_NUM        (INTR_NUM_BASE + 15)
#define HWNAT_INTR_NUM      (INTR_NUM_BASE + 20)

#define MAX_INT_HANDLERS        (6 +32)

int int_add(unsigned long vect, void (* handler)(void));
void int_del(unsigned long vect);

#endif /* __IRQ_H__ */
