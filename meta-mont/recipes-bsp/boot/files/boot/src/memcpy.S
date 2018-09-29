#ifdef CONFIG_OPTIMIZED_MEMCPY

#include <arch/cpu.h>

#define ENTRY(name) \
  .globl name;      \
  .align 2;         \
  .ent name,0;      \
  name/* use a comment rather than ## to workaround bug in gcc-3.4.x */:

#undef END
#define END(function) \
  .end    function;   \
  .size   function,.-function

#define L(label) .L ## label

/* void *memcpy(void *dst, const void *src, size_t len);  */

ENTRY (memcpy)
    .set    noreorder

    slti    t0, a2, 8       # Less than 8?
    bne t0, zero, L(last8)
    move    v0, a0          # Setup exit value before too late

    xor t0, a1, a0      # Find a0/a1 displacement
    andi    t0, 0x3
#if 1
    bne t0, zero, L(last8)  # copy byte by byte
    nop
#else
    bne t0, zero, L(shift)  # Go handle the unaligned case
#endif
    subu    t1, zero, a1
    andi    t1, 0x3         # a0/a1 are aligned, but are we
    beq t1, zero, L(chk8w)  #  starting in the middle of a word?
#if 1
    nop
    lb  t0, 0(a1)
    addiu   t1, -1
    sb  t0, 0(a0)
    beq t1, zero, L(ucpydone)

    nop
    lb  t0, 1(a1)
    addiu   t1, -1
    sb  t0, 1(a0)
    beq t1, zero, L(ucpydone)

    nop
    lb  t0, 2(a1)
    nop
    nop
    sb  t0, 2(a0)

L(ucpydone):
    subu    t1, zero, a1        # restore t1
    andi    t1, 0x3

    subu    a2, t1
    addu    a1, t1
    addu    a0, t1
#else
    subu    a2, t1
    LWHI    t0, 0(a1)       # Yes we are... take care of that
    addu    a1, t1
    SWHI    t0, 0(a0)
    addu    a0, t1
#endif

L(chk8w):
    andi    t0, a2, 0x1f        # 32 or more bytes left?
    beq t0, a2, L(chk1w)
    subu    a3, a2, t0      # Yes
    addu    a3, a1          # a3 = end address of loop
    move    a2, t0          # a2 = what will be left after loop
L(lop8w):
    lw  t0,  0(a1)      # Loop taking 8 words at a time
    lw  t1,  4(a1)
    lw  t2,  8(a1)
    lw  t3, 12(a1)
    lw  t4, 16(a1)
    lw  t5, 20(a1)
    lw  t6, 24(a1)
    lw  t7, 28(a1)
    addiu   a0, 32
    addiu   a1, 32
    sw  t0, -32(a0)
    sw  t1, -28(a0)
    sw  t2, -24(a0)
    sw  t3, -20(a0)
    sw  t4, -16(a0)
    sw  t5, -12(a0)
    sw  t6,  -8(a0)
    bne a1, a3, L(lop8w)
    sw  t7,  -4(a0)

L(chk1w):
    andi    t0, a2, 0x3     # 4 or more bytes left?
    beq t0, a2, L(last8)
    subu    a3, a2, t0      # Yes, handle them one word at a time
    addu    a3, a1          # a3 again end address
    move    a2, t0
L(lop1w):
    lw  t0, 0(a1)
    addiu   a0, 4
    addiu   a1, 4
    bne a1, a3, L(lop1w)
    sw  t0, -4(a0)

L(last8):
    blez    a2, L(lst8e)        # Handle last 8 bytes, one at a time
    addu    a3, a2, a1
L(lst8l):
    lb  t0, 0(a1)
    addiu   a0, 1
    addiu   a1, 1
    bne a1, a3, L(lst8l)
    sb  t0, -1(a0)
L(lst8e):
    jr  ra          # Bye, bye
    nop

    .set    reorder
END (memcpy)

#endif
