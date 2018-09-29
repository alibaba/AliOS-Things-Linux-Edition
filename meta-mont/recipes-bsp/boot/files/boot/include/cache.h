#ifndef __CACHE_H__
#define __CACHE_H__

typedef unsigned char CYG_BYTE;
#define CYGARC_KSEG_CACHED_BASE         (0x80000000)

#define CYG_MACRO_START do {
#define CYG_MACRO_END   } while (0)

// Data cache
#define HAL_DCACHE_SIZE                 32768   // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            32      // Size of a data cache line
#define HAL_DCACHE_WAYS                 4       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 65536   // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            32      // Size of a cache line
#define HAL_ICACHE_WAYS                 4       // Associativity of the cache


#define HAL_CLEAR_TAGLO()  asm volatile (" mtc0 $0, $28;" \
                                             " nop;"      \
                                             " nop;"      \
                                             " nop;")
#define HAL_CLEAR_TAGHI()  asm volatile (" mtc0 $0, $29;" \
                                             " nop;"      \
                                             " nop;"      \
                                             " nop;")


/* Cache instruction opcodes */
#define HAL_CACHE_OP(which, op)             (which | (op << 2))

#define HAL_WHICH_ICACHE                    0x0
#define HAL_WHICH_DCACHE                    0x1

#define HAL_INDEX_INVALIDATE                0x0
#define HAL_INDEX_LOAD_TAG                  0x1
#define HAL_INDEX_STORE_TAG                 0x2
#define HAL_HIT_INVALIDATE                  0x4
#define HAL_ICACHE_FILL                     0x5
#define HAL_DCACHE_HIT_INVALIDATE           0x5
#define HAL_DCACHE_HIT_WRITEBACK            0x6
#define HAL_FETCH_AND_LOCK                  0x7


#define HAL_DCACHE_WB_INVALIDATE_ALL()                                                  \
    CYG_MACRO_START                                                                     \
    register volatile CYG_BYTE *addr;                                                   \
    for (addr = (CYG_BYTE *)CYGARC_KSEG_CACHED_BASE;                                    \
         addr < (CYG_BYTE *)(CYGARC_KSEG_CACHED_BASE + HAL_DCACHE_SIZE);                \
         addr += HAL_DCACHE_LINE_SIZE )                                                 \
    {                                                                                   \
        asm volatile (" cache %0, 0(%1)"                                                \
                      :                                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_DCACHE, HAL_INDEX_INVALIDATE)),     \
                        "r"(addr));                                                     \
    }                                                                                   \
    CYG_MACRO_END

#define HAL_ICACHE_INVALIDATE_ALL()                                                     \
    CYG_MACRO_START                                                                     \
    register volatile CYG_BYTE *addr;                                                   \
    for (addr = (CYG_BYTE *)CYGARC_KSEG_CACHED_BASE;                                    \
         addr < (CYG_BYTE *)(CYGARC_KSEG_CACHED_BASE + HAL_ICACHE_SIZE);                \
         addr += HAL_ICACHE_LINE_SIZE )                                                 \
    {                                                                                   \
        asm volatile (" cache %0, 0(%1)"                                                \
                      :                                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_ICACHE, HAL_INDEX_INVALIDATE)),     \
                        "r"(addr));                                                     \
    }                                                                                   \
    CYG_MACRO_END


#endif // __CACHE_H__
