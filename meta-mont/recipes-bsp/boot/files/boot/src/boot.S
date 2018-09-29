/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file boot.S
*   \brief cheetah bootstrap loader before C main()
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/cpu.h>
#include <arch/chip.h>

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

#define STACK_ADDRESS_IN_SRAM 0x90007E00
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
    .globl  boot
    .globl  putchar
    .globl  flush_cache
.text

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

/*!-----------------------------------------------------------------------------
 * function: boot
 *
 *  \brief
 *  \param
 *  \return
 +----------------------------------------------------------------------------*/
boot:
    .ent    boot

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
    la      sp, STACK_ADDRESS_IN_SRAM

start:
    jal     bmain
    nop
    j       start

/*!-----------------------------------------------------------------------------
 * function: flush_cache
 *
 *  \brief For loading instruction with cache address,
           flush dcahe and invalidate icache in advance.
 *  \param none
 *  \return none
 +----------------------------------------------------------------------------*/
flush_cache:
    /*dcache flush index*/
    la      v1, KSEG0_CACHED_BASE
    la      v0, (KSEG0_CACHED_BASE+HAL_DCACHE_SIZE)
1:
    cache   HAL_CACHE_OP(DCACHE, HAL_INDEX_INVALIDATE), 0(v1)
    addiu   v1, v1, HAL_DCACHE_LINE_SIZE
    bne     v0, v1, 1b

    /*icache invalidate index*/
    la      v1, KSEG0_CACHED_BASE
    la      v0, (KSEG0_CACHED_BASE+HAL_ICACHE_SIZE)
2:
    cache   HAL_CACHE_OP(ICACHE, HAL_INDEX_INVALIDATE), 0(v1)
    addiu   v1, v1, HAL_ICACHE_LINE_SIZE
    bne     v0, v1, 2b

    jr      ra

/*!-----------------------------------------------------------------------------
 * function: putchar
 *
 *  \brief  put char
 *  \param  a0: char
 *  \return none
 +----------------------------------------------------------------------------*/
putchar:
#ifdef CONFIG_SHOW_PROMPT_CHARS_IN_BOOT1
    la      v0, UR_BASE
    sll     a0, a0, URBR_DTSHFT
    sw      a0, URBR(v0)
    jr      ra
#else
    jr      ra
#endif

    .end    boot
