/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file trap.c
*   \brief Trap Initialization
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <arch/trap.h>
#include <arch/irq.h>
#include <lib.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
//#define printk
#define printf printk
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
const char *exception_names[EXP_MAX] = {
    "INT", "TLBM", "TLBL", "TLBS", "ADEL", "ADES", "IBE", "DBE", "SYSCALL",
    "BP", "RI", "CPU", "OV", "TRAP", "REV", "FP"
};

irq_handler ex_tables[EXP_MAX];
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern void *(vec00_jump) ();
extern void *(vec80_jump) ();
extern void memcpy(void *dst, void *src, int len);
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

/*!
 * function: dump_regs
 *
 *  \brief  dump register frame
 *  \param p
 *  \return
 */
void dump_regs(struct saved_frame *p)
{
    unsigned int *r = &p->zero;
    printf("regiters:\n");

    printf(" 0 -a3: %08x %08x %08x %08x %08x %08x %08x %08x\n",
           *r, *(r + 1), *(r + 2), *(r + 3), *(r + 4), *(r + 5), *(r + 6),
           *(r + 7));
    r += 8;
    printf(" t0-t7: %08x %08x %08x %08x %08x %08x %08x %08x\n",
           *r, *(r + 1), *(r + 2), *(r + 3), *(r + 4), *(r + 5), *(r + 6),
           *(r + 7));
    r += 8;
    printf(" s0-s7: %08x %08x %08x %08x %08x %08x %08x %08x\n",
           *r, *(r + 1), *(r + 2), *(r + 3), *(r + 4), *(r + 5), *(r + 6),
           *(r + 7));
    r += 8;
    /* use k0, k1 to save context, so k0, k1 are not valid in here */
    printf(" t8 t9 gp sp s8 ra: %08x %08x %08x %08x %08x %08x\n",
           *r, *(r + 1), *(r + 4), *(r + 5), *(r + 6), *(r + 7));
    printf(" epc:%08x sr:%08x cause:%08x badvaddr:%08x\n",
           p->c0_epc, p->c0_sr, p->c0_cause, p->c0_bad);
}

/*!
 * function: set_ex_handler
 *
 *  \brief  set_ex_handler
 *  \param exp
 *  \param handler
 *  \param id
 *  \return
 */
void set_ex_handler(unsigned int exp, void (*handler) (), void *id)
{
    unsigned int old = irq_save();      /* diable interrupt */

    ex_tables[exp].hnd = (unsigned int) handler;
    ex_tables[exp].id = (unsigned int) id;

    irq_restore(old);           /* Restore interrupt */
}

/*!
 * function: null exception handler
 *
 *  \brief
 *  \param id
 *  \param reg_frame
 *  \return
 */
void null_ex_handler(void *id, struct saved_frame *reg_frame)
{
    register unsigned int c0_cause = MFC0(C0_CAUSE);
    register unsigned int exc_code = (c0_cause & 0x7c) >> 2;

    if (c0_cause == 0)
        return;

    printf("\nException %s not handled\n",
           (exc_code < EXP_MAX) ? exception_names[exc_code] : "unknown");

    dump_regs(reg_frame);
#ifdef  CONFIG_GDB
    handle_exception((unsigned int) (reg_frame) - 24);  /* 24 = struct gdb_regs -> pad0[6] */
#endif
#ifdef  CONFIG_CMD
    cmd_loop();
#endif
}

/*!
 * function: exception trap initialization
 *
 *  \brief
 *  \return
 */
void trap_init()
{
    int i;
    memcpy((void *) uncached_addr(CONFIG_TLB_EXCEPTION_VECTOR), vec00_jump,
           VECTOR_JUMP_SIZE);
    memcpy((void *) uncached_addr(CONFIG_GENERAL_EXCEPTION_VECTOR), vec80_jump,
           VECTOR_JUMP_SIZE);

    for (i = 0; i < EXP_MAX; i++)
    {
        ex_tables[i].hnd = (unsigned int) null_ex_handler;
        ex_tables[i].id = 0;
    }
}
