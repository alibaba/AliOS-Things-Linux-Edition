/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file start.S
*   \brief cheetah bootstrap loader before C main()
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/cpu.h>
#include <arch/chip.h>

.comm   sp_base,  CONFIG_STACK_SZ

        .extern irq_tables
        .extern ex_tables
        .globl  start
        .globl  vec00_jump
        .globl  vec80_jump

.text

/*!-----------------------------------------------------------------------------
 * function: start
 *
 *  \brief  entry point
 *  \param
 *  \return none
 +----------------------------------------------------------------------------*/
start:
    .ent    start
#ifdef CONFIG_CPU_COUNT
    la      v0, 0x7f00
#else
    la      v0, 0xff00
#endif
    mtc0    v0, C0_STATUS
    mtc0    zero, C0_CAUSE

    la      a0, _fbss
    la      v1, _end
1:
    sw      zero, 0(a0)
    sw      zero, 4(a0)
    sw      zero, 8(a0)
    sw      zero, 12(a0)
    addu    a0, 16
    bltu    a0, v1, 1b
    la      gp, _gp
    la      sp, sp_base+CONFIG_STACK_SZ-0x80
1:
    jal     cmain
    nop
    jal 1b
    nop
    .end    start

/*!-----------------------------------------------------------------------------
 * function: generic exception vector
 *
 *  \brief  generic exception vector
 *  \param
 *  \return none
 +----------------------------------------------------------------------------*/
generic_exc_handle:

    SAVE_CONTEXT

    mfc0    k0, C0_CAUSE
    sll     t8, k0, 25
    beqz    t8, do_interrupt
    la      k1, ex_tables
    srl     t8, t8, 24
    addu    k1, k1, t8
    lw      t8, 0(k1)
    lw      a0, 4(k1)
    move    a1, sp
    jal     t8
    nop
    j       1f
    nop

do_interrupt:
	la      v0, IC_BASE
	lw      v1, (IMSK)(v0)
	lw      v0, (ISTS)(v0)
	nor     v1, v1, zero # inverse bits
	and     v0, v0, v1
	beqz    v0, 1f
	move    t3, zero # initialze t3
2:
	andi    v1, v0, 1
	bnez	v1, 4f
	srl		v0, 1
	addi    t3, 8 # sizeof(irq_handler) for each entry
	slti	t1, t3, 256			# < 8*32 = 256
	bnez    t1, 2b
4:
    la      k1, irq_tables
    addu    k1, k1, t3
    lw      t8, 0(k1) #irq handler
    lw      a0, 4(k1) #irq data

    beq     t8, a0, 5f
    nop

    jal     t8
    nop
    j       1f
    nop

5:
    move    a0, sp
    jal     t8
    nop
    move    sp, v0

1:
    RESTORE_CONTEXT

    .set noreorder
	eret
    nop
    .set reorder

/*!-----------------------------------------------------------------------------
 * function: tlb exception hanlder
 *
 *  \brief  call from tlbvec
 *  \param
 *  \return none
 +----------------------------------------------------------------------------*/
tlb_exc_handle:

    SAVE_CONTEXT

    mfc0    k0, C0_CAUSE
    sll     k0, k0, 25
    la      k1, ex_tables
    srl     k0, k0, 24
    addu    k1, k1, k0
    lw      k0, 0(k1)
    lw      a0, 4(k1)
    move    a1, sp
    jal     k0
    nop

    RESTORE_CONTEXT

    .set noreorder
    mfc0    k0, C0_EPC
    nop
    jr  k0
    rfe
    nop
    .set reorder

/*!-----------------------------------------------------------------------------
 * function: jump to tlb exception handler
 *
 *  \brief
 *  \param none
 *  \return none
 +----------------------------------------------------------------------------*/
vec00_jump:
    .ent    vec00_jump
    la      k0, tlb_exc_handle
    jr      k0
    .end    vec00_jump

/*!-----------------------------------------------------------------------------
 * function: jump to generic exception handler
 *
 *  \brief
 *  \param none
 *  \return none
 +----------------------------------------------------------------------------*/
vec80_jump:
    .ent    vec80_jump
    la      k0, generic_exc_handle
    jr      k0
    .end    vec80_jump

