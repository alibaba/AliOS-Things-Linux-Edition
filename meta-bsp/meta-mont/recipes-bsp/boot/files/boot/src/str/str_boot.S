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

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

#define STACK_ADDRESS_IN_SRAM 0x90003E00
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
    .globl  boot
    .globl  flush_cache
    .globl  resume_asm
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
    .set    noreorder
    .set    noat

    addiu   sp, sp, -88

sw $1, 0x00(sp)
move $1, sp

la sp, STACK_ADDRESS_IN_SRAM

sw $1,0x00(sp)
sw $2,0x04(sp)
sw $3,0x08(sp)
sw $4,0x0C(sp)
sw $5,0x10(sp)
sw $6,0x14(sp)
sw $7,0x18(sp)
sw $16,0x1C(sp)
sw $17,0x20(sp)
sw $18,0x24(sp)
sw $19,0x28(sp)
sw $20,0x2C(sp)
sw $21,0x30(sp)
sw $22,0x34(sp)
sw $23,0x38(sp)
sw $26,0x3C(sp)
sw $27,0x40(sp)
sw $28,0x44(sp)
sw $1,0x48(sp)
sw $30,0x4C(sp)
sw $31,0x50(sp)

lw $1, 0x00($1)
nop
sw $1, 0x00(sp)

#define CP0_STATUS $12
#define CP0_WIRED  $6
#define CP0_CONFIG0 $16, 0
#define CP0_CONFIG1 $16, 1
#define CP0_CONFIG2 $16, 2
#define CP0_CONFIG3 $16, 3
#define CP0_EBASE   $15, 1
#define CP0_PAGEMASK $5, 0
#define CP0_CONTEXT $4, 0
#define CP0_COMPARE $11, 0
#define CP0_COUNT   $9, 0

mfc0 k0, CP0_STATUS /*Move from coprocessor 0, CP0_STATUS to k0*/
sw k0, 0x54(sp) /*Store word k0 to offset 0x54 in memory*/
mfc0 k0, CP0_CONFIG0 /*Move from coprocessor 0, CP0_CONFIG0 to k0*/
sw k0, 0x58(sp) /*Store word k0 to offset 0x58 in memory*/
mfc0 k0, CP0_CONFIG1 /*Move from coprocessor 0, CP0_CONFIG1 to k0*/
sw k0, 0x5C(sp) /*Store word k0 to offset 0x5C in memory*/
mfc0 k0, CP0_CONFIG2 /*Move from coprocessor 0, CP0_CONFIG2 to k0*/
sw k0, 0x60(sp) /*Store word k0 to offset 0x60 in memory*/
mfc0 k0, CP0_CONFIG3 /*Move from coprocessor 0, CP0_CONFIG3 to k0*/
sw k0, 0x64(sp) /*Store word k0 to offset 0x64 in memory*/
mfc0 k0, CP0_EBASE /*Move from coprocessor 0, CP0_EBASE to k0*/
sw k0, 0x68(sp) /*Store word k0 to offset 0x68 in memory*/
mfc0 k0, CP0_PAGEMASK /*Move from coprocessor 0, CP0_PAGEMASK to k0*/
sw k0, 0x6C(sp) /*Store word k0 to offset 0x6C in memory*/
mfc0 k0, CP0_CONTEXT /*Move from coprocessor 0, CP0_CONTEXT to k0*/
sw k0, 0x70(sp) /*Store word k0 to offset 0x70 in memory*/
mfc0 k0, CP0_WIRED /*Move from coprocessor 0, CP0_WIRED to k0*/
sw k0, 0x74(sp) /*Store word k0 to offset 0x74 in memory*/
mfc0 k0, CP0_COMPARE
sw k0, 0x78(sp)
mfc0 k0, CP0_COUNT
sw k0, 0x7C(sp)


    .set    at

#if 1
    la      a0, _fbss
    la      v1, _end

1:
    sw      zero, 0(a0)
    sw      zero, 4(a0)
    sw      zero, 8(a0)
    sw      zero, 12(a0)
    addu    a0, 16
    bltu    a0, v1, 1b
    nop
    la      gp, _gp
    //la      sp, STACK_ADDRESS_IN_SRAM

    jal     suspend
    nop
#endif

#if 1
resume_asm:
    .align  4
    la      sp, STACK_ADDRESS_IN_SRAM

    la      a0, _fbss
    la      v1, _end

2:
    sw      zero, 0(a0)
    sw      zero, 4(a0)
    sw      zero, 8(a0)
    sw      zero, 12(a0)
    addu    a0, 16
    bltu    a0, v1, 2b
    nop
    la      gp, _gp
    //la      sp, STACK_ADDRESS_IN_SRAM

    jal     resume
    nop
#endif

    .set    noat

lw k0, 0x7C(sp)
mtc0 k0, CP0_COUNT
lw k0, 0x78(sp)
mtc0 k0, CP0_COMPARE
lw k0, 0x74(sp) /*Load word k0 from offset 0x74 in memory*/
mtc0 k0, CP0_WIRED /*Move to coprocessor 0, CP0_WIRED from k0*/
lw k0, 0x70(sp) /*Load word k0 from offset 0x70 in memory*/
mtc0 k0, CP0_CONTEXT /*Move to coprocessor 0, CP0_CONTEXT from k0*/
lw k0, 0x6C(sp) /*Load word k0 from offset 0x6C in memory*/
mtc0 k0, CP0_PAGEMASK /*Move to coprocessor 0, CP0_PAGEMASK from k0*/
lw k0, 0x68(sp) /*Load word k0 from offset 0x68 in memory*/
mtc0 k0, CP0_EBASE /*Move to coprocessor 0, CP0_EBASE from k0*/
lw k0, 0x64(sp) /*Load word k0 from offset 0x64 in memory*/
mfc0 k0, CP0_CONFIG3 /*Move to coprocessor 0, CP0_CONFIG3 from k0*/
lw k0, 0x60(sp) /*Load word k0 from offset 0x60 in memory*/
mtc0 k0, CP0_CONFIG2 /*Move to coprocessor 0, CP0_CONFIG2 from k0*/
lw k0, 0x5C(sp) /*Load word k0 from offset 0x5C in memory*/
mtc0 k0, CP0_CONFIG1 /*Move to coprocessor 0, CP0_CONFIG1 from k0*/
lw k0, 0x58(sp) /*Load word k0 from offset 0x58 in memory*/
mtc0 k0, CP0_CONFIG0 /*Move to coprocessor 0, CP0_CONFIG0 from k0*/
lw k0, 0x54(sp) /*Load word k0 from offset 0x54 in memory*/
mtc0 k0, CP0_STATUS /*Move to coprocessor 0, CP0_STATUS from k0*/

lw $31,0x50(sp)
lw $30,0x4C(sp)
//lw $29,0x48(sp)
lw $28,0x44(sp)
lw $27,0x40(sp)
lw $26,0x3C(sp)
lw $23,0x38(sp)
lw $22,0x34(sp)
lw $21,0x30(sp)
lw $20,0x2C(sp)
lw $19,0x28(sp)
lw $18,0x24(sp)
lw $17,0x20(sp)
lw $16,0x1C(sp)
lw $7,0x18(sp)
lw $6,0x14(sp)
lw $5,0x10(sp)
lw $4,0x0C(sp)
lw $3,0x08(sp)
lw $2,0x04(sp)
lw $1,0x00(sp)
lw $29,0x48(sp)
    addiu   sp,sp,88

    jr      ra
    nop

#if 0
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
    nop
#endif
    
    .end    boot
