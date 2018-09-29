/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file irq.c
*   \brief Interrupt setting and handler
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <arch/irq.h>
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
irq_handler irq_tables[IRQ_MAX];
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void null_irq_handler(void *id);
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

/*!
 * function:    request_irq
 *
 *  \brief
 *  \param intr
 *  \param handler
 *  \param id
 *  \return
 */
void request_irq(unsigned int intr, void (*handler) (), void *id)
{
    unsigned int old = irq_save();

    irq_tables[intr].hnd = (unsigned int) handler;
    irq_tables[intr].id = (unsigned int) id;
    enable_irq(intr);

    irq_restore(old);
}

/*!
 * function:    free_irq
 *
 *  \brief
 *  \param intr
 *  \param id
 *  \return
 */
void free_irq(unsigned int intr, void *id)
{
    /* diable interrupt */
    unsigned int old = irq_save();

    disable_irq(intr);
    irq_tables[intr].hnd = (unsigned int) null_irq_handler;
    irq_tables[intr].id = 0;

    /* Restore interrupt */
    irq_restore(old);
}

/*!
 * function:    irq_init
 *
 *  \brief
 *  \return
 */
void irq_init()
{
    int i;

    for (i = 0; i < IRQ_MAX; i++)
    {
        irq_tables[i].hnd = (unsigned int) null_irq_handler;
        irq_tables[i].id = 0;
    }
}

/*!
 * function:
 *
 *  \brief      enable interrupts
 *  \return
 */
void irq_enable()
{
    register unsigned int c0_sr = MFC0(C0_STATUS);

    c0_sr |= 1;

    MTC0(C0_STATUS, c0_sr);
}

/*!
 * function:    irq_save
 *
 *  \brief
 *  \return
 */
unsigned int irq_save()
{

    register unsigned int old = MFC0(C0_STATUS);
    register unsigned int set;

    set = old & (~1);
    MTC0(C0_STATUS, set);
    return old;

}

/*!
 * function: irq_restore
 *
 *  \brief
 *  \param org
 *  \return
 */

void irq_restore(unsigned int org)
{
    register unsigned int sr = org;
    MTC0(C0_STATUS, sr);
}

/*!
 * function: enable_irq, unmask interrupt
 *
 *  \brief
 *  \param intr
 *  \return
 */

void enable_irq(int intr)
{
    if (intr >= IRQ_MAX)
        return;
    ICREG(IMSK) &= ~(1 << (intr));
}

/*!
 * function: disable_irq, mask interrupt
 *
 *  \brief
 *  \param intr
 *  \return
 */

void disable_irq(int intr)
{
    if (intr >= IRQ_MAX)
        return;
    ICREG(IMSK) |= (1 << (intr));
}

/*
    default interrupt handler
*/

/*!
 * function:    null_irq_handler
 *
 *  \brief
 *  \param id
 *  \return
 */

void null_irq_handler(void *id)
{
//  register unsigned int c0_cause = MFC0(C0_CAUSE);
//  register unsigned int ip_code = (c0_cause << 16) >> 26;

//  return;
}
