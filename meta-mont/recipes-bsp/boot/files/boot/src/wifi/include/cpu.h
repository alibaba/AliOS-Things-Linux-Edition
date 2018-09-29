/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cpu.h
*   \brief CPU Related Macro Definition
*   \author Montage
*/

#ifndef CPU_H
#define CPU_H

/* System Control Coprocessor (CP0) exception processing registers */
#define C0_INDEX        $0
#define C0_ENTRYLO0     $2
#define C0_ENTRYLO1     $3
#define C0_CONTEXT      $4      /* Context */
#define C0_PAGEMASK     $5
#define C0_WIRED        $6
#define C0_BADVADDR     $8      /* Bad Virtual Address */
#define C0_COUNT        $9      /* Count */
#define C0_ENTRYHI      $10
#define C0_COMPARE      $11     /* Compare */
#define C0_STATUS       $12     /* Processor Status */
#define C0_CAUSE        $13     /* Exception Cause */
#define C0_EPC          $14     /* Exception PC */
#define C0_EBASE        $15,1   /* Exception Base Address */
#define C0_CONFIG       $16     /* Config Register 0 */
#define C0_WATCHLO      $18     /* Watchpoint Low */
#define C0_WATCHHI      $19     /* Watchpoint High */
#define C0_ERRCTL       $26     /* Error Control */
#define C0_CACHEERR     $27     /* Cache Error */
#define C0_ITAGLO       $28     /* ICACHE Tag Low */
#define C0_IDATALO      $28,1   /* ICACHE Data Low */
#define C0_DTAGLO       $28,2   /* DCACHE Tag Low */
#define C0_DDATALO      $28,3   /* DCACHE Data Low */
#define C0_ERROREPC     $30     /* Error EPC */

#define zero    $0
#define AT      $at
#define v0      $2
#define v1      $3
#define a0      $4
#define a1      $5
#define a2      $6
#define a3      $7
#define t0      $8
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12
#define t5      $13
#define t6      $14
#define t7      $15
#define s0      $16
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23
#define t8      $24
#define t9      $25
#define k0      $26
#define k1      $27
#define gp      $28
#define sp      $29
#define s8      $30
#define fp      $30
#define ra      $31

#define EF_ZERO    0*4
#define EF_AT      1*4
#define EF_V0      2*4
#define EF_V1      3*4
#define EF_A0      4*4
#define EF_A1      5*4
#define EF_A2      6*4
#define EF_A3      7*4
#define EF_T0      8*4
#define EF_T1      9*4
#define EF_T2      10*4
#define EF_T3      11*4
#define EF_T4      12*4
#define EF_T5      13*4
#define EF_T6      14*4
#define EF_T7      15*4
#define EF_S0      16*4
#define EF_S1      17*4
#define EF_S2      18*4
#define EF_S3      19*4
#define EF_S4      20*4
#define EF_S5      21*4
#define EF_S6      22*4
#define EF_S7      23*4
#define EF_T8      24*4
#define EF_T9      25*4
#define EF_K0      26*4
#define EF_K1      27*4
#define EF_GP      28*4
#define EF_SP      29*4
#define EF_S8      30*4
#define EF_RA      31*4
#define EF_SR      32*4
#define EF_LO      33*4
#define EF_HI      34*4
#define EF_BADVAR  35*4
#define EF_CAUSE   36*4
#define EF_EPC     37*4
#define EF_SIZE    38*4

#define SAVE_CONTEXT         \
        .set noreorder          ;\
        move    k1, sp          ;\
        subu    sp, EF_SIZE     ;\
        sw      k1, EF_SP(sp)   ;\
        sw      gp, EF_GP(sp)   ;\
        mfc0    k1, C0_EPC      ;\
        sw      ra, EF_RA(sp)   ;\
        sw      k1, EF_EPC(sp)  ;\
        .set noat               ;\
        sw  AT, EF_AT(sp)       ;\
        .set at                 ;\
        mfc0    k1, C0_STATUS   ;\
        sw  v0, EF_V0(sp)       ;\
        sw      k1, EF_SR(sp)   ;\
        sw  v1, EF_V1(sp)       ;\
        sw  a0, EF_A0(sp)       ;\
        mfc0    k1, C0_CAUSE    ;\
        sw  a1, EF_A1(sp)       ;\
        sw      k1, EF_CAUSE(sp);\
        sw  a2, EF_A2(sp)       ;\
        mfc0    k1, C0_BADVADDR ;\
        sw  a3, EF_A3(sp)       ;\
        sw      k1, EF_BADVAR(sp);\
        sw  s0, EF_S0(sp)       ;\
        sw  s1, EF_S1(sp)       ;\
        sw  s2, EF_S2(sp)       ;\
        sw  s3, EF_S3(sp)       ;\
        sw  t0, EF_T0(sp)       ;\
        sw  t1, EF_T1(sp)       ;\
        sw  t2, EF_T2(sp)       ;\
        sw  t3, EF_T3(sp)       ;\
        sw  t8, EF_T8(sp)       ;\
        mfhi    t0              ;\
        mflo    t1              ;\
        sw  t0, EF_HI(sp)       ;\
        sw  t1, EF_LO(sp)       ;\
        sw  s4, EF_S4(sp)       ;\
        sw  s5, EF_S5(sp)       ;\
        sw  s6, EF_S6(sp)       ;\
        sw  s7, EF_S7(sp)       ;\
        sw  s8, EF_S8(sp)       ;\
        sw  t4, EF_T4(sp)       ;\
        sw  t5, EF_T5(sp)       ;\
        sw  t6, EF_T6(sp)       ;\
        sw  t7, EF_T7(sp)       ;\
        sw  t9, EF_T9(sp)       ;\
        sw  k0, EF_K0(sp)       ;\
        sw  k1, EF_K1(sp)       ;\
        sw  zero, EF_ZERO(sp)   ;\
        .set reorder

#define RESTORE_CONTEXT      \
        .set noreorder        ; \
        lw      t0, EF_EPC(sp); \
        lw      t1, EF_HI(sp);  \
        lw      t2, EF_LO(sp);  \
        mtc0    t0, C0_EPC;     \
        mthi    t1;             \
        mtlo    t2;             \
        lw      v0, EF_V0(sp);  \
        lw      v1, EF_V1(sp);  \
        lw      a0, EF_A0(sp);  \
        lw      a1, EF_A1(sp);  \
        lw      a2, EF_A2(sp);  \
        lw      a3, EF_A3(sp);  \
        lw      s0, EF_S0(sp);  \
        lw      s1, EF_S1(sp);  \
        lw      s2, EF_S2(sp);  \
        lw      s3, EF_S3(sp);  \
        lw      t0, EF_T0(sp);  \
        lw      t1, EF_T1(sp);  \
        lw      t2, EF_T2(sp);  \
        lw      t3, EF_T3(sp);  \
        lw      t8, EF_T8(sp);  \
        lw      s4, EF_S4(sp);  \
        lw      s5, EF_S5(sp);  \
        lw      s6, EF_S6(sp);  \
        lw      s7, EF_S7(sp);  \
        lw      s8, EF_S8(sp);  \
        lw      t4, EF_T4(sp);  \
        lw      t5, EF_T5(sp);  \
        lw      t6, EF_T6(sp);  \
        lw      t7, EF_T7(sp);  \
        lw      t9, EF_T9(sp);  \
        lw      k0, EF_K0(sp);  \
        lw      k1, EF_K1(sp);  \
        lw      ra, EF_RA(sp);  \
        lw      gp, EF_GP(sp);  \
        .set noat            ;  \
        lw      AT, EF_AT(sp);  \
        nop                  ;  \
        .set at              ;  \
        lw      sp, EF_SP(sp);  \
        .set reorder

#ifdef  LANGUAGE_C

struct saved_frame
{
    unsigned int zero;
    unsigned int at;
    unsigned int v0;
    unsigned int v1;
    unsigned int a0;
    unsigned int a1;
    unsigned int a2;
    unsigned int a3;
    unsigned int t0;
    unsigned int t1;
    unsigned int t2;
    unsigned int t3;
    unsigned int t4;
    unsigned int t5;
    unsigned int t6;
    unsigned int t7;
    unsigned int s0;
    unsigned int s1;
    unsigned int s2;
    unsigned int s3;
    unsigned int s4;
    unsigned int s5;
    unsigned int s6;
    unsigned int s7;
    unsigned int t8;
    unsigned int t9;
    unsigned int k0;
    unsigned int k1;
    unsigned int gp;
    unsigned int sp;
    unsigned int s8;
    unsigned int ra;
    unsigned int c0_sr;
    unsigned int lo;
    unsigned int hi;
    unsigned int c0_bad;
    unsigned int c0_cause;
    unsigned int c0_epc;
};

#ifndef __STR
#define __STR(x) #x
#endif

#define MFC0(reg)   \
({ int __ret;               \
        __asm__ __volatile__(    \
    ".set\tpush\n\t"        \
    ".set\treorder\n\t"     \
    "mfc0\t%0,"__STR(reg)"\n\t" \
    ".set\tpop"             \
        : "=r" (__ret));    \
        __ret;              \
})

#define MTC0(reg, val)      \
    __asm__ __volatile__(   \
    "mtc0\t%0,"__STR(reg)"\n\t" \
    "nop"                   \
    : : "r" (val));
#endif

#define MACRO_START do{
#define MACRO_END   }while(0)
#define HAL_CACHE_OP(which, op)             (which | (op << 2))

#define KSEG0_CACHED_BASE               (0x80000000)
#define RESET_VECOTR                    (0xbfc00000)

/* cache operation */
#define ICACHE                  0x0
#define DCACHE                  0x1

#define HAL_INDEX_INVALIDATE    0x0
#define INDEX_LOAD_TAG          0x1
#define INDEX_STORE_TAG         0x2
#define HAL_HIT_INVALIDATE      0x4
#define HAL_HIT_FLUSH           0x5
#define PREFETCH_LOCKING        0x7

// Data cache
#define HAL_DCACHE_SIZE             32768       // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE        32  // size of DCACHE line
#define HAL_DCACHE_WAYS             4   // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE             65536       // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE        32  // size of ICACHE line
#define HAL_ICACHE_WAYS             4   // Associativity of the cache

#define HAL_DCACHE_START_ADDRESS(_addr_) \
(((unsigned int)(_addr_)) & ~(HAL_DCACHE_LINE_SIZE-1))
#define HAL_DCACHE_END_ADDRESS(_addr_, _asize_) \
(((unsigned int)(_addr_) + (_asize_)))
#define HAL_ICACHE_START_ADDRESS(_addr_) \
(((unsigned int)(_addr_)) & ~(HAL_ICACHE_LINE_SIZE-1))
#define HAL_ICACHE_END_ADDRESS(_addr_, _asize_) \
(((unsigned int)(_addr_) + (_asize_)))

#define HAL_DCACHE_FLUSH( _start_ , _asize_ )                                           \
    MACRO_START                                                                         \
    register unsigned int _addr_  = HAL_DCACHE_START_ADDRESS(_start_);                  \
    register unsigned int _eaddr_ = HAL_DCACHE_END_ADDRESS(_start_, _asize_);           \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE ){                          \
      asm volatile (" cache %0, 0(%1)"                                                  \
                    :                                                                   \
                    : "i" (HAL_CACHE_OP(DCACHE, HAL_HIT_FLUSH)),                        \
                      "r"(_addr_));}                                                    \
    MACRO_END

#define HAL_DCACHE_INVALIDATE( _start_ , _asize_ )                                      \
    MACRO_START                                                                         \
    register unsigned int _addr_  = HAL_DCACHE_START_ADDRESS(_start_);                  \
    register unsigned int _eaddr_ = HAL_DCACHE_END_ADDRESS(_start_, _asize_);           \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE ){                          \
      asm volatile (" cache %0, 0(%1)"                                                  \
                    :                                                                   \
                    : "i" (HAL_CACHE_OP(DCACHE, HAL_HIT_INVALIDATE)),                   \
                      "r"(_addr_));}                                                    \
    MACRO_END

#define HAL_ICACHE_INVALIDATE( _start_ , _asize_ )                                      \
    MACRO_START                                                                         \
    register unsigned int _addr_  = HAL_ICACHE_START_ADDRESS(_start_);                  \
    register unsigned int _eaddr_ = HAL_ICACHE_END_ADDRESS(_start_, _asize_);           \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_ICACHE_LINE_SIZE ){                          \
      asm volatile (" cache %0, 0(%1)"                                                  \
                    :                                                                   \
                    : "i" (HAL_CACHE_OP(ICACHE, HAL_HIT_INVALIDATE)),                   \
                      "r"(_addr_));}                                                    \
    MACRO_END

#define HAL_DCACHE_LOCKING( _start_ , _asize_ )                                         \
    MACRO_START                                                                         \
    register unsigned int _addr_  = HAL_DCACHE_START_ADDRESS(_start_);                  \
    register unsigned int _eaddr_ = HAL_DCACHE_END_ADDRESS(_start_, _asize_);           \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_DCACHE_LINE_SIZE ){                          \
      asm volatile (" cache %0, 0(%1)"                                                  \
                    :                                                                   \
                    : "i" (HAL_CACHE_OP(DCACHE, PREFETCH_LOCKING)),                     \
                      "r"(_addr_));}                                                    \
    MACRO_END

#define HAL_ICACHE_LOCKING( _start_ , _asize_ )                                         \
    MACRO_START                                                                         \
    register unsigned int _addr_  = HAL_ICACHE_START_ADDRESS(_start_);                  \
    register unsigned int _eaddr_ = HAL_ICACHE_END_ADDRESS(_start_, _asize_);           \
    for( ; _addr_ < _eaddr_; _addr_ += HAL_ICACHE_LINE_SIZE ){                          \
      asm volatile (" cache %0, 0(%1)"                                                  \
                    :                                                                   \
                    : "i" (HAL_CACHE_OP(ICACHE, PREFETCH_LOCKING)),                     \
                      "r"(_addr_));}                                                    \
    MACRO_END

#define HAL_DCACHE_FLUSH_ALL()                                                          \
    MACRO_START                                                                         \
    register unsigned int _baddr_ = (unsigned int)(KSEG0_CACHED_BASE);                  \
    register unsigned int _addr_  = (unsigned int)(KSEG0_CACHED_BASE);                  \
    register unsigned int _size_  = (unsigned int)HAL_DCACHE_SIZE;                      \
    for ( ; _addr_ < _baddr_+_size_; _addr_ += HAL_DCACHE_LINE_SIZE){                   \
      asm volatile ("cache %0, 0(%1)\n\t"                                               \
                    :                                                                   \
                    : "i" (HAL_CACHE_OP(DCACHE, HAL_INDEX_INVALIDATE)),                 \
                      "r"(_addr_)); }                                                   \
    MACRO_END

#define HAL_ICACHE_INVALIDATE_ALL()                                                     \
    MACRO_START                                                                         \
    register unsigned int _baddr_ = (unsigned int)(KSEG0_CACHED_BASE);                  \
    register unsigned int _addr_  = (unsigned int)(KSEG0_CACHED_BASE);                  \
    register unsigned int _size_  = (unsigned int)HAL_ICACHE_SIZE;                      \
    for ( ; _addr_ < _baddr_+_size_; _addr_ += HAL_ICACHE_LINE_SIZE){                   \
      asm volatile (" cache %0, 0(%1)\n\t"                                              \
                      :                                                                 \
                      : "i" (HAL_CACHE_OP(ICACHE, HAL_INDEX_INVALIDATE)),               \
                        "r"(_addr_));}                                                  \
    MACRO_END
#endif
