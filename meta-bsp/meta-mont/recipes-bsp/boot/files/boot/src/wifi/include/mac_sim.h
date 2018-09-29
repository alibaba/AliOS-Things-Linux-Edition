#ifndef __MAC_SIM_H__
#define __MAC_SIM_H__

int diag_printf( const char *fmt, ... );
//int sprintf(char *str, const char *format, ...);
//#define printf diag_printf

//#define GPIO_BASE       (MI_BASE+0x5000)

//#define GPVAL 0
//#define GPSET 1
//#define GPCLR 2
//#define GPDIR 3
//#define GPSEL 4

//#define GPREG(reg)              ((volatile unsigned long*)(GPIO_BASE))[reg]
/*
typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef u64 __le64;
typedef u32 __le32;
typedef u16 __le16;
typedef u16 __be16;
typedef u8 __u8;
typedef signed char s8;
*/
//#define bool int
//#define true 1
//#define false 0

#define unlikely(x)  (x)

#if defined(SIM)
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned long u_int32_t;
typedef unsigned long long u_int64_t;
#endif

//#include <stdlib.h>
//#include <string.h>

#if defined(LYNX)
#define UNCACHED_ADDR(x)   ( ((u32) (x) & 0x000fffffUL) | 0xBEB00000UL )
#define PHYSICAL_ADDR(x)   ((u32) (x) & 0x000fffffUL)
#define VIRTUAL_ADDR(x)    ((u32) (x) | 0xBEB00000UL)
#else
#define UNCACHED_ADDR(x)   ( ((u32) (x) & 0x1fffffffUL) | 0xA0000000UL )
#define PHYSICAL_ADDR(x)   ((u32) (x) & 0x1fffffffUL)
#define VIRTUAL_ADDR(x)    ((u32) (x) | 0x80000000L)
#endif

#define nonca_addr(x) 	((u32) (x))
#define virtophy	PHYSICAL_ADDR
#define phytovirt	VIRTUAL_ADDR

//#define DEBUG_WIFI
#ifdef DEBUG_WIFI
#define MACDBG  printf
#else
#define MACDBG(...)
#endif

#if 0
#define ASSERT(cond,message)   \
if(!(cond)) { \
    printf((message));\
    abort();\
}
#else
#define ASSERT(cond,message)
#endif

#define SIMDBG(...)  do {} while(0)

#define ___memcpy   memcpy

#define ___constant_swab16(x) ((u16)(				\
	(((u16)(x) & (u16)0x00ffU) << 8) |			\
	(((u16)(x) & (u16)0xff00U) >> 8)))

#define ___constant_swab32(x) ((u32)(				\
	(((u32)(x) & (u32)0x000000ffUL) << 24) |		\
	(((u32)(x) & (u32)0x0000ff00UL) <<  8) |		\
	(((u32)(x) & (u32)0x00ff0000UL) >>  8) |		\
	(((u32)(x) & (u32)0xff000000UL) >> 24)))

#ifndef NULL
#define NULL    0
#endif

//#if defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)
//
//#error both big and little endian defined
//
//#elif defined(LITTLE_ENDIAN)
//#define cpu_to_be32(x)      ___constant_swab32(x)
//#define be32_to_cpu(x)      ___constant_swab32(x)
//#define cpu_to_be16(x)      ___constant_swab16(x)
//#define be16_to_cpu(x)      ___constant_swab16(x)
//
//#define cpu_to_le32(x)      (x)
//#define le32_to_cpu(x)      (x)
//#define cpu_to_le16(x)      (x)
//#define le16_to_cpu(x)      (x)
//#elif defined(BIG_ENDIAN)
//#define cpu_to_be32(x)      (x)
//#define be32_to_cpu(x)      (x)
//#define cpu_to_be16(x)      (x)
//#define be16_to_cpu(x)      (x)
//
//#define cpu_to_le32(x)      ___constant_swab32(x)
//#define le32_to_cpu(x)      ___constant_swab32(x)
//#define cpu_to_le16(x)      ___constant_swab16(x)
//#define le16_to_cpu(x)      ___constant_swab16(x)
//#else
//
//#error unknown processor endian
//
//#endif

#endif // __MAC_SIM_H__
