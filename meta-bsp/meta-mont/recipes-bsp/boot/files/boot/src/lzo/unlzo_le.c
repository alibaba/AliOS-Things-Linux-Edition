# 1 "arch/mips/boot/compressed/decompress.c"
# 1 "/workplace3/panther_le.asic/panther/linux-4.4//"
# 1 "<command-line>"
# 1 "././include/linux/kconfig.h" 1



# 1 "include/generated/autoconf.h" 1
# 5 "././include/linux/kconfig.h" 2
# 1 "<command-line>" 2
# 1 "arch/mips/boot/compressed/decompress.c"
# 14 "arch/mips/boot/compressed/decompress.c"
# 1 "include/linux/types.h" 1




# 1 "include/uapi/linux/types.h" 1



# 1 "./arch/mips/include/asm/types.h" 1
# 14 "./arch/mips/include/asm/types.h"
# 1 "include/asm-generic/int-ll64.h" 1
# 10 "include/asm-generic/int-ll64.h"
# 1 "include/uapi/asm-generic/int-ll64.h" 1
# 11 "include/uapi/asm-generic/int-ll64.h"
# 1 "./arch/mips/include/uapi/asm/bitsperlong.h" 1





# 1 "include/asm-generic/bitsperlong.h" 1



# 1 "include/uapi/asm-generic/bitsperlong.h" 1
# 5 "include/asm-generic/bitsperlong.h" 2
# 7 "./arch/mips/include/uapi/asm/bitsperlong.h" 2
# 12 "include/uapi/asm-generic/int-ll64.h" 2







typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;


__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
# 11 "include/asm-generic/int-ll64.h" 2




typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;
# 15 "./arch/mips/include/asm/types.h" 2
# 1 "./arch/mips/include/uapi/asm/types.h" 1
# 16 "./arch/mips/include/asm/types.h" 2
# 5 "include/uapi/linux/types.h" 2
# 13 "include/uapi/linux/types.h"
# 1 "./include/uapi/linux/posix_types.h" 1



# 1 "include/linux/stddef.h" 1



# 1 "include/uapi/linux/stddef.h" 1
# 1 "include/linux/compiler.h" 1
# 56 "include/linux/compiler.h"
# 1 "include/linux/compiler-gcc.h" 1
# 57 "include/linux/compiler.h" 2
# 85 "include/linux/compiler.h"
struct ftrace_branch_data {
 const char *func;
 const char *file;
 unsigned line;
 union {
  struct {
   unsigned long correct;
   unsigned long incorrect;
  };
  struct {
   unsigned long miss;
   unsigned long hit;
  };
  unsigned long miss_hit[2];
 };
};
# 199 "include/linux/compiler.h"
# 1 "include/uapi/linux/types.h" 1
# 200 "include/linux/compiler.h" 2
# 215 "include/linux/compiler.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((always_inline))
void __read_once_size(const volatile void *p, void *res, int size)
{
 ({ switch (size) { case 1: *(__u8 *)res = *(volatile __u8 *)p; break; case 2: *(__u16 *)res = *(volatile __u16 *)p; break; case 4: *(__u32 *)res = *(volatile __u32 *)p; break; case 8: *(__u64 *)res = *(volatile __u64 *)p; break; default: __asm__ __volatile__("": : :"memory"); __builtin_memcpy((void *)res, (const void *)p, size); __asm__ __volatile__("": : :"memory"); } });
}
# 234 "include/linux/compiler.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((always_inline))
void __read_once_size_nocheck(const volatile void *p, void *res, int size)
{
 ({ switch (size) { case 1: *(__u8 *)res = *(volatile __u8 *)p; break; case 2: *(__u16 *)res = *(volatile __u16 *)p; break; case 4: *(__u32 *)res = *(volatile __u32 *)p; break; case 8: *(__u64 *)res = *(volatile __u64 *)p; break; default: __asm__ __volatile__("": : :"memory"); __builtin_memcpy((void *)res, (const void *)p, size); __asm__ __volatile__("": : :"memory"); } });
}


static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((always_inline)) void __write_once_size(volatile void *p, void *res, int size)
{
 switch (size) {
 case 1: *(volatile __u8 *)p = *(__u8 *)res; break;
 case 2: *(volatile __u16 *)p = *(__u16 *)res; break;
 case 4: *(volatile __u32 *)p = *(__u32 *)res; break;
 case 8: *(volatile __u64 *)p = *(__u64 *)res; break;
 default:
  __asm__ __volatile__("": : :"memory");
  __builtin_memcpy((void *)p, (const void *)res, size);
  __asm__ __volatile__("": : :"memory");
 }
}
# 1 "include/uapi/linux/stddef.h" 2
# 5 "include/linux/stddef.h" 2




enum {
 false = 0,
 true = 1
};
# 5 "./include/uapi/linux/posix_types.h" 2
# 24 "./include/uapi/linux/posix_types.h"
typedef struct {
 unsigned long fds_bits[1024 / (8 * sizeof(long))];
} __kernel_fd_set;


typedef void (*__kernel_sighandler_t)(int);


typedef int __kernel_key_t;
typedef int __kernel_mqd_t;

# 1 "./arch/mips/include/uapi/asm/posix_types.h" 1
# 12 "./arch/mips/include/uapi/asm/posix_types.h"
# 1 "./arch/mips/include/uapi/asm/sgidefs.h" 1
# 13 "./arch/mips/include/uapi/asm/posix_types.h" 2







typedef long __kernel_daddr_t;



typedef struct {
 long val[2];
} __kernel_fsid_t;



# 1 "./include/uapi/asm-generic/posix_types.h" 1
# 14 "./include/uapi/asm-generic/posix_types.h"
typedef long __kernel_long_t;
typedef unsigned long __kernel_ulong_t;



typedef __kernel_ulong_t __kernel_ino_t;



typedef unsigned int __kernel_mode_t;



typedef int __kernel_pid_t;



typedef int __kernel_ipc_pid_t;



typedef unsigned int __kernel_uid_t;
typedef unsigned int __kernel_gid_t;



typedef __kernel_long_t __kernel_suseconds_t;







typedef unsigned int __kernel_uid32_t;
typedef unsigned int __kernel_gid32_t;



typedef __kernel_uid_t __kernel_old_uid_t;
typedef __kernel_gid_t __kernel_old_gid_t;



typedef unsigned int __kernel_old_dev_t;
# 67 "./include/uapi/asm-generic/posix_types.h"
typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
typedef int __kernel_ptrdiff_t;
# 86 "./include/uapi/asm-generic/posix_types.h"
typedef __kernel_long_t __kernel_off_t;
typedef long long __kernel_loff_t;
typedef __kernel_long_t __kernel_time_t;
typedef __kernel_long_t __kernel_clock_t;
typedef int __kernel_timer_t;
typedef int __kernel_clockid_t;
typedef char * __kernel_caddr_t;
typedef unsigned short __kernel_uid16_t;
typedef unsigned short __kernel_gid16_t;
# 31 "./arch/mips/include/uapi/asm/posix_types.h" 2
# 36 "./include/uapi/linux/posix_types.h" 2
# 14 "include/uapi/linux/types.h" 2
# 32 "include/uapi/linux/types.h"
typedef __u16 __le16;
typedef __u16 __be16;
typedef __u32 __le32;
typedef __u32 __be32;
typedef __u64 __le64;
typedef __u64 __be64;

typedef __u16 __sum16;
typedef __u32 __wsum;
# 6 "include/linux/types.h" 2






typedef __u32 __kernel_dev_t;

typedef __kernel_fd_set fd_set;
typedef __kernel_dev_t dev_t;
typedef __kernel_ino_t ino_t;
typedef __kernel_mode_t mode_t;
typedef unsigned short umode_t;
typedef __u32 nlink_t;
typedef __kernel_off_t off_t;
typedef __kernel_pid_t pid_t;
typedef __kernel_daddr_t daddr_t;
typedef __kernel_key_t key_t;
typedef __kernel_suseconds_t suseconds_t;
typedef __kernel_timer_t timer_t;
typedef __kernel_clockid_t clockid_t;
typedef __kernel_mqd_t mqd_t;

typedef _Bool bool;

typedef __kernel_uid32_t uid_t;
typedef __kernel_gid32_t gid_t;
typedef __kernel_uid16_t uid16_t;
typedef __kernel_gid16_t gid16_t;

typedef unsigned long uintptr_t;
# 45 "include/linux/types.h"
typedef __kernel_loff_t loff_t;
# 54 "include/linux/types.h"
typedef __kernel_size_t size_t;




typedef __kernel_ssize_t ssize_t;




typedef __kernel_ptrdiff_t ptrdiff_t;




typedef __kernel_time_t time_t;




typedef __kernel_clock_t clock_t;




typedef __kernel_caddr_t caddr_t;



typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;


typedef unsigned char unchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;




typedef __u8 u_int8_t;
typedef __s8 int8_t;
typedef __u16 u_int16_t;
typedef __s16 int16_t;
typedef __u32 u_int32_t;
typedef __s32 int32_t;



typedef __u8 uint8_t;
typedef __u16 uint16_t;
typedef __u32 uint32_t;


typedef __u64 uint64_t;
typedef __u64 u_int64_t;
typedef __s64 int64_t;
# 133 "include/linux/types.h"
typedef unsigned long sector_t;
typedef unsigned long blkcnt_t;
# 154 "include/linux/types.h"
typedef u32 dma_addr_t;


typedef unsigned gfp_t;
typedef unsigned fmode_t;
typedef unsigned oom_flags_t;




typedef u32 phys_addr_t;


typedef phys_addr_t resource_size_t;





typedef unsigned long irq_hw_number_t;

typedef struct {
 int counter;
} atomic_t;







struct list_head {
 struct list_head *next, *prev;
};

struct hlist_head {
 struct hlist_node *first;
};

struct hlist_node {
 struct hlist_node *next, **pprev;
};

struct ustat {
 __kernel_daddr_t f_tfree;
 __kernel_ino_t f_tinode;
 char f_fname[6];
 char f_fpack[6];
};
# 223 "include/linux/types.h"
struct callback_head {
 struct callback_head *next;
 void (*func)(struct callback_head *head);
} __attribute__((aligned(sizeof(void *))));


typedef void (*rcu_callback_t)(struct callback_head *head);
typedef void (*call_rcu_func_t)(struct callback_head *head, rcu_callback_t func);


typedef u64 cycle_t;
# 15 "arch/mips/boot/compressed/decompress.c" 2
# 1 "include/linux/kernel.h" 1




# 1 "/opt/montage.panther/toolchain-mipsel_interaptiv_gcc-4.8-linaro_uClibc-0.9.33.2/lib/gcc/mipsel-openwrt-linux-uclibc/4.8.3/include/stdarg.h" 1 3 4
# 40 "/opt/montage.panther/toolchain-mipsel_interaptiv_gcc-4.8-linaro_uClibc-0.9.33.2/lib/gcc/mipsel-openwrt-linux-uclibc/4.8.3/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 98 "/opt/montage.panther/toolchain-mipsel_interaptiv_gcc-4.8-linaro_uClibc-0.9.33.2/lib/gcc/mipsel-openwrt-linux-uclibc/4.8.3/include/stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 6 "include/linux/kernel.h" 2
# 1 "include/linux/linkage.h" 1




# 1 "include/linux/stringify.h" 1
# 6 "include/linux/linkage.h" 2
# 1 "include/linux/export.h" 1
# 26 "include/linux/export.h"
struct kernel_symbol
{
 unsigned long value;
 const char *name;
};
# 7 "include/linux/linkage.h" 2
# 1 "./arch/mips/include/asm/linkage.h" 1
# 8 "include/linux/linkage.h" 2
# 7 "include/linux/kernel.h" 2



# 1 "include/linux/bitops.h" 1
# 27 "include/linux/bitops.h"
extern unsigned int __sw_hweight8(unsigned int w);
extern unsigned int __sw_hweight16(unsigned int w);
extern unsigned int __sw_hweight32(unsigned int w);
extern unsigned long __sw_hweight64(__u64 w);





# 1 "./arch/mips/include/asm/bitops.h" 1
# 18 "./arch/mips/include/asm/bitops.h"
# 1 "./arch/mips/include/asm/barrier.h" 1
# 11 "./arch/mips/include/asm/barrier.h"
# 1 "./arch/mips/include/asm/addrspace.h" 1
# 13 "./arch/mips/include/asm/addrspace.h"
# 1 "./arch/mips/include/asm/mach-generic/spaces.h" 1
# 13 "./arch/mips/include/asm/mach-generic/spaces.h"
# 1 "./include/uapi/linux/const.h" 1
# 14 "./arch/mips/include/asm/mach-generic/spaces.h" 2
# 14 "./arch/mips/include/asm/addrspace.h" 2
# 12 "./arch/mips/include/asm/barrier.h" 2
# 19 "./arch/mips/include/asm/bitops.h" 2
# 1 "./arch/mips/include/uapi/asm/byteorder.h" 1
# 14 "./arch/mips/include/uapi/asm/byteorder.h"
# 1 "include/linux/byteorder/little_endian.h" 1



# 1 "include/uapi/linux/byteorder/little_endian.h" 1
# 12 "include/uapi/linux/byteorder/little_endian.h"
# 1 "include/linux/swab.h" 1



# 1 "include/uapi/linux/swab.h" 1





# 1 "./arch/mips/include/uapi/asm/swab.h" 1
# 20 "./arch/mips/include/uapi/asm/swab.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u16 __arch_swab16(__u16 x)
{
 __asm__(
 "	.set	push			\n"
 "	.set	arch=mips32r2		\n"
 "	wsbh	%0, %1			\n"
 "	.set	pop			\n"
 : "=r" (x)
 : "r" (x));

 return x;
}


static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u32 __arch_swab32(__u32 x)
{
 __asm__(
 "	.set	push			\n"
 "	.set	arch=mips32r2		\n"
 "	wsbh	%0, %1			\n"
 "	rotr	%0, %0, 16		\n"
 "	.set	pop			\n"
 : "=r" (x)
 : "r" (x));

 return x;
}
# 7 "include/uapi/linux/swab.h" 2
# 46 "include/uapi/linux/swab.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u16 __fswab16(__u16 val)
{

 return __builtin_bswap16(val);





}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u32 __fswab32(__u32 val)
{

 return __builtin_bswap32(val);





}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u64 __fswab64(__u64 val)
{

 return __builtin_bswap64(val);
# 81 "include/uapi/linux/swab.h"
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u32 __fswahw32(__u32 val)
{



 return ((__u32)( (((__u32)(val) & (__u32)0x0000ffffUL) << 16) | (((__u32)(val) & (__u32)0xffff0000UL) >> 16)));

}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((__const__)) __u32 __fswahb32(__u32 val)
{



 return ((__u32)( (((__u32)(val) & (__u32)0x00ff00ffUL) << 8) | (((__u32)(val) & (__u32)0xff00ff00UL) >> 8)));

}
# 154 "include/uapi/linux/swab.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u16 __swab16p(const __u16 *p)
{



 return (__builtin_constant_p((__u16)(*p)) ? ((__u16)( (((__u16)(*p) & (__u16)0x00ffU) << 8) | (((__u16)(*p) & (__u16)0xff00U) >> 8))) : __fswab16(*p));

}





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 __swab32p(const __u32 *p)
{



 return (__builtin_constant_p((__u32)(*p)) ? ((__u32)( (((__u32)(*p) & (__u32)0x000000ffUL) << 24) | (((__u32)(*p) & (__u32)0x0000ff00UL) << 8) | (((__u32)(*p) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(*p) & (__u32)0xff000000UL) >> 24))) : __fswab32(*p));

}





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u64 __swab64p(const __u64 *p)
{



 return (__builtin_constant_p((__u64)(*p)) ? ((__u64)( (((__u64)(*p) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(*p) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(*p) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(*p) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(*p) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(*p) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(*p) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(*p) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(*p));

}







static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 __swahw32p(const __u32 *p)
{



 return (__builtin_constant_p((__u32)(*p)) ? ((__u32)( (((__u32)(*p) & (__u32)0x0000ffffUL) << 16) | (((__u32)(*p) & (__u32)0xffff0000UL) >> 16))) : __fswahw32(*p));

}







static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 __swahb32p(const __u32 *p)
{



 return (__builtin_constant_p((__u32)(*p)) ? ((__u32)( (((__u32)(*p) & (__u32)0x00ff00ffUL) << 8) | (((__u32)(*p) & (__u32)0xff00ff00UL) >> 8))) : __fswahb32(*p));

}





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __swab16s(__u16 *p)
{



 *p = __swab16p(p);

}




static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __swab32s(__u32 *p)
{



 *p = __swab32p(p);

}





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __swab64s(__u64 *p)
{



 *p = __swab64p(p);

}







static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __swahw32s(__u32 *p)
{



 *p = __swahw32p(p);

}







static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __swahb32s(__u32 *p)
{



 *p = __swahb32p(p);

}
# 5 "include/linux/swab.h" 2
# 13 "include/uapi/linux/byteorder/little_endian.h" 2
# 43 "include/uapi/linux/byteorder/little_endian.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __le64 __cpu_to_le64p(const __u64 *p)
{
 return ( __le64)*p;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u64 __le64_to_cpup(const __le64 *p)
{
 return ( __u64)*p;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __le32 __cpu_to_le32p(const __u32 *p)
{
 return ( __le32)*p;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 __le32_to_cpup(const __le32 *p)
{
 return ( __u32)*p;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __le16 __cpu_to_le16p(const __u16 *p)
{
 return ( __le16)*p;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u16 __le16_to_cpup(const __le16 *p)
{
 return ( __u16)*p;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __be64 __cpu_to_be64p(const __u64 *p)
{
 return ( __be64)__swab64p(p);
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u64 __be64_to_cpup(const __be64 *p)
{
 return __swab64p((__u64 *)p);
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __be32 __cpu_to_be32p(const __u32 *p)
{
 return ( __be32)__swab32p(p);
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 __be32_to_cpup(const __be32 *p)
{
 return __swab32p((__u32 *)p);
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __be16 __cpu_to_be16p(const __u16 *p)
{
 return ( __be16)__swab16p(p);
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u16 __be16_to_cpup(const __be16 *p)
{
 return __swab16p((__u16 *)p);
}
# 5 "include/linux/byteorder/little_endian.h" 2

# 1 "include/linux/byteorder/generic.h" 1
# 143 "include/linux/byteorder/generic.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void le16_add_cpu(__le16 *var, u16 val)
{
 *var = (( __le16)(__u16)((( __u16)(__le16)(*var)) + val));
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void le32_add_cpu(__le32 *var, u32 val)
{
 *var = (( __le32)(__u32)((( __u32)(__le32)(*var)) + val));
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void le64_add_cpu(__le64 *var, u64 val)
{
 *var = (( __le64)(__u64)((( __u64)(__le64)(*var)) + val));
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void be16_add_cpu(__be16 *var, u16 val)
{
 *var = (( __be16)(__builtin_constant_p((__u16)(((__builtin_constant_p((__u16)(( __u16)(__be16)(*var))) ? ((__u16)( (((__u16)(( __u16)(__be16)(*var)) & (__u16)0x00ffU) << 8) | (((__u16)(( __u16)(__be16)(*var)) & (__u16)0xff00U) >> 8))) : __fswab16(( __u16)(__be16)(*var))) + val))) ? ((__u16)( (((__u16)(((__builtin_constant_p((__u16)(( __u16)(__be16)(*var))) ? ((__u16)( (((__u16)(( __u16)(__be16)(*var)) & (__u16)0x00ffU) << 8) | (((__u16)(( __u16)(__be16)(*var)) & (__u16)0xff00U) >> 8))) : __fswab16(( __u16)(__be16)(*var))) + val)) & (__u16)0x00ffU) << 8) | (((__u16)(((__builtin_constant_p((__u16)(( __u16)(__be16)(*var))) ? ((__u16)( (((__u16)(( __u16)(__be16)(*var)) & (__u16)0x00ffU) << 8) | (((__u16)(( __u16)(__be16)(*var)) & (__u16)0xff00U) >> 8))) : __fswab16(( __u16)(__be16)(*var))) + val)) & (__u16)0xff00U) >> 8))) : __fswab16(((__builtin_constant_p((__u16)(( __u16)(__be16)(*var))) ? ((__u16)( (((__u16)(( __u16)(__be16)(*var)) & (__u16)0x00ffU) << 8) | (((__u16)(( __u16)(__be16)(*var)) & (__u16)0xff00U) >> 8))) : __fswab16(( __u16)(__be16)(*var))) + val))));
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void be32_add_cpu(__be32 *var, u32 val)
{
 *var = (( __be32)(__builtin_constant_p((__u32)(((__builtin_constant_p((__u32)(( __u32)(__be32)(*var))) ? ((__u32)( (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x000000ffUL) << 24) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0xff000000UL) >> 24))) : __fswab32(( __u32)(__be32)(*var))) + val))) ? ((__u32)( (((__u32)(((__builtin_constant_p((__u32)(( __u32)(__be32)(*var))) ? ((__u32)( (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x000000ffUL) << 24) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0xff000000UL) >> 24))) : __fswab32(( __u32)(__be32)(*var))) + val)) & (__u32)0x000000ffUL) << 24) | (((__u32)(((__builtin_constant_p((__u32)(( __u32)(__be32)(*var))) ? ((__u32)( (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x000000ffUL) << 24) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0xff000000UL) >> 24))) : __fswab32(( __u32)(__be32)(*var))) + val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(((__builtin_constant_p((__u32)(( __u32)(__be32)(*var))) ? ((__u32)( (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x000000ffUL) << 24) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0xff000000UL) >> 24))) : __fswab32(( __u32)(__be32)(*var))) + val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(((__builtin_constant_p((__u32)(( __u32)(__be32)(*var))) ? ((__u32)( (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x000000ffUL) << 24) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0xff000000UL) >> 24))) : __fswab32(( __u32)(__be32)(*var))) + val)) & (__u32)0xff000000UL) >> 24))) : __fswab32(((__builtin_constant_p((__u32)(( __u32)(__be32)(*var))) ? ((__u32)( (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x000000ffUL) << 24) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x0000ff00UL) << 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)(( __u32)(__be32)(*var)) & (__u32)0xff000000UL) >> 24))) : __fswab32(( __u32)(__be32)(*var))) + val))));
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void be64_add_cpu(__be64 *var, u64 val)
{
 *var = (( __be64)(__builtin_constant_p((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val))) ? ((__u64)( (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(((__builtin_constant_p((__u64)(( __u64)(__be64)(*var))) ? ((__u64)( (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)(( __u64)(__be64)(*var)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64(( __u64)(__be64)(*var))) + val))));
}
# 7 "include/linux/byteorder/little_endian.h" 2
# 15 "./arch/mips/include/uapi/asm/byteorder.h" 2
# 20 "./arch/mips/include/asm/bitops.h" 2
# 1 "./arch/mips/include/asm/compiler.h" 1
# 21 "./arch/mips/include/asm/bitops.h" 2
# 1 "./arch/mips/include/asm/cpu-features.h" 1
# 12 "./arch/mips/include/asm/cpu-features.h"
# 1 "./arch/mips/include/asm/cpu.h" 1
# 270 "./arch/mips/include/asm/cpu.h"
enum cpu_type_enum {
 CPU_UNKNOWN,




 CPU_R2000, CPU_R3000, CPU_R3000A, CPU_R3041, CPU_R3051, CPU_R3052,
 CPU_R3081, CPU_R3081E,




 CPU_R6000, CPU_R6000A,




 CPU_R4000PC, CPU_R4000SC, CPU_R4000MC, CPU_R4200, CPU_R4300, CPU_R4310,
 CPU_R4400PC, CPU_R4400SC, CPU_R4400MC, CPU_R4600, CPU_R4640, CPU_R4650,
 CPU_R4700, CPU_R5000, CPU_R5500, CPU_NEVADA, CPU_R5432, CPU_R10000,
 CPU_R12000, CPU_R14000, CPU_R16000, CPU_VR41XX, CPU_VR4111, CPU_VR4121,
 CPU_VR4122, CPU_VR4131, CPU_VR4133, CPU_VR4181, CPU_VR4181A, CPU_RM7000,
 CPU_SR71000, CPU_TX49XX,




 CPU_R8000,




 CPU_TX3912, CPU_TX3922, CPU_TX3927,




 CPU_4KC, CPU_4KEC, CPU_4KSC, CPU_24K, CPU_34K, CPU_1004K, CPU_74K,
 CPU_ALCHEMY, CPU_PR4450, CPU_BMIPS32, CPU_BMIPS3300, CPU_BMIPS4350,
 CPU_BMIPS4380, CPU_BMIPS5000, CPU_JZRISC, CPU_LOONGSON1, CPU_M14KC,
 CPU_M14KEC, CPU_INTERAPTIV, CPU_P5600, CPU_PROAPTIV, CPU_1074K, CPU_M5150,
 CPU_I6400,




 CPU_5KC, CPU_5KE, CPU_20KC, CPU_25KF, CPU_SB1, CPU_SB1A, CPU_LOONGSON2,
 CPU_LOONGSON3, CPU_CAVIUM_OCTEON, CPU_CAVIUM_OCTEON_PLUS,
 CPU_CAVIUM_OCTEON2, CPU_CAVIUM_OCTEON3, CPU_XLR, CPU_XLP,

 CPU_QEMU_GENERIC,

 CPU_LAST
};
# 13 "./arch/mips/include/asm/cpu-features.h" 2
# 1 "./arch/mips/include/asm/cpu-info.h" 1
# 17 "./arch/mips/include/asm/cpu-info.h"
# 1 "./arch/mips/include/asm/cache.h" 1
# 12 "./arch/mips/include/asm/cache.h"
# 1 "./arch/mips/include/asm/mach-panther/kmalloc.h" 1
# 13 "./arch/mips/include/asm/cache.h" 2
# 18 "./arch/mips/include/asm/cpu-info.h" 2




struct cache_desc {
 unsigned int waysize;
 unsigned short sets;
 unsigned char ways;
 unsigned char linesz;
 unsigned char waybit;
 unsigned char flags;
};
# 41 "./arch/mips/include/asm/cpu-info.h"
struct cpuinfo_mips {
 unsigned long asid_cache;




 unsigned long ases;
 unsigned long long options;
 unsigned int udelay_val;
 unsigned int processor_id;
 unsigned int fpu_id;
 unsigned int fpu_csr31;
 unsigned int fpu_msk31;
 unsigned int msa_id;
 unsigned int cputype;
 int isa_level;
 int tlbsize;
 int tlbsizevtlb;
 int tlbsizeftlbsets;
 int tlbsizeftlbways;
 struct cache_desc icache;
 struct cache_desc dcache;
 struct cache_desc scache;
 struct cache_desc tcache;
 int srsets;
 int package;
 int core;
# 76 "./arch/mips/include/asm/cpu-info.h"
 int vpe_id;

 void *data;
 unsigned int watch_reg_count;
 unsigned int watch_reg_use_cnt;

 u16 watch_reg_masks[4];
 unsigned int kscratch_mask;




 unsigned int writecombine;




 unsigned int htw_seq;
} __attribute__((aligned((1 << 5))));

extern struct cpuinfo_mips cpu_data[];




extern void cpu_probe(void);
extern void cpu_report(void);

extern const char *__cpu_name[];


struct seq_file;
struct notifier_block;

extern int register_proc_cpuinfo_notifier(struct notifier_block *nb);
extern int proc_cpuinfo_notifier_call_chain(unsigned long val, void *v);
# 123 "./arch/mips/include/asm/cpu-info.h"
struct proc_cpuinfo_notifier_args {
 struct seq_file *m;
 unsigned long n;
};
# 14 "./arch/mips/include/asm/cpu-features.h" 2
# 1 "./arch/mips/include/asm/mach-panther/cpu-feature-overrides.h" 1
# 15 "./arch/mips/include/asm/cpu-features.h" 2
# 22 "./arch/mips/include/asm/bitops.h" 2

# 1 "./arch/mips/include/asm/war.h" 1
# 12 "./arch/mips/include/asm/war.h"
# 1 "./arch/mips/include/asm/mach-panther/war.h" 1
# 13 "./arch/mips/include/asm/war.h" 2
# 24 "./arch/mips/include/asm/bitops.h" 2
# 45 "./arch/mips/include/asm/bitops.h"
void __mips_set_bit(unsigned long nr, volatile unsigned long *addr);
void __mips_clear_bit(unsigned long nr, volatile unsigned long *addr);
void __mips_change_bit(unsigned long nr, volatile unsigned long *addr);
int __mips_test_and_set_bit(unsigned long nr,
       volatile unsigned long *addr);
int __mips_test_and_set_bit_lock(unsigned long nr,
     volatile unsigned long *addr);
int __mips_test_and_clear_bit(unsigned long nr,
         volatile unsigned long *addr);
int __mips_test_and_change_bit(unsigned long nr,
          volatile unsigned long *addr);
# 68 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void set_bit(unsigned long nr, volatile unsigned long *addr)
{
 unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
 int bit = nr & 31UL;
 unsigned long temp;

 if (1 && 0) {
  __asm__ __volatile__(
  "	.set	arch=r4000				\n"
  "1:	" "ll	" "%0, %1			# set_bit	\n"
  "	or	%0, %2					\n"
  "	" "sc	" "%0, %1					\n"
  "	beqzl	%0, 1b					\n"
  "	.set	mips0					\n"
  : "=&r" (temp), "=" "R" (*m)
  : "ir" (1UL << bit), "R" (*m));

 } else if (1 && __builtin_constant_p(bit)) {
  do {
   __asm__ __volatile__(
   "	" "ll	" "%0, %1		# set_bit	\n"
   "	" "ins	" "%0, %3, %2, 1			\n"
   "	" "sc	" "%0, %1				\n"
   : "=&r" (temp), "+" "R" (*m)
   : "ir" (bit), "r" (~0));
  } while (__builtin_expect(!!(!temp), 0));

 } else if (1) {
  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1		# set_bit	\n"
   "	or	%0, %2				\n"
   "	" "sc	" "%0, %1				\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m)
   : "ir" (1UL << bit));
  } while (__builtin_expect(!!(!temp), 0));
 } else
  __mips_set_bit(nr, addr);
}
# 120 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void clear_bit(unsigned long nr, volatile unsigned long *addr)
{
 unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
 int bit = nr & 31UL;
 unsigned long temp;

 if (1 && 0) {
  __asm__ __volatile__(
  "	.set	arch=r4000				\n"
  "1:	" "ll	" "%0, %1			# clear_bit	\n"
  "	and	%0, %2					\n"
  "	" "sc	" "%0, %1					\n"
  "	beqzl	%0, 1b					\n"
  "	.set	mips0					\n"
  : "=&r" (temp), "+" "R" (*m)
  : "ir" (~(1UL << bit)));

 } else if (1 && __builtin_constant_p(bit)) {
  do {
   __asm__ __volatile__(
   "	" "ll	" "%0, %1		# clear_bit	\n"
   "	" "ins	" "%0, $0, %2, 1			\n"
   "	" "sc	" "%0, %1				\n"
   : "=&r" (temp), "+" "R" (*m)
   : "ir" (bit));
  } while (__builtin_expect(!!(!temp), 0));

 } else if (1) {
  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1		# clear_bit	\n"
   "	and	%0, %2				\n"
   "	" "sc	" "%0, %1				\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m)
   : "ir" (~(1UL << bit)));
  } while (__builtin_expect(!!(!temp), 0));
 } else
  __mips_clear_bit(nr, addr);
}
# 170 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void clear_bit_unlock(unsigned long nr, volatile unsigned long *addr)
{
 __asm__ __volatile__("		\n" : : :"memory");
 clear_bit(nr, addr);
}
# 185 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void change_bit(unsigned long nr, volatile unsigned long *addr)
{
 int bit = nr & 31UL;

 if (1 && 0) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  __asm__ __volatile__(
  "	.set	arch=r4000			\n"
  "1:	" "ll	" "%0, %1		# change_bit	\n"
  "	xor	%0, %2				\n"
  "	" "sc	" "%0, %1				\n"
  "	beqzl	%0, 1b				\n"
  "	.set	mips0				\n"
  : "=&r" (temp), "+" "R" (*m)
  : "ir" (1UL << bit));
 } else if (1) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1		# change_bit	\n"
   "	xor	%0, %2				\n"
   "	" "sc	" "%0, %1				\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m)
   : "ir" (1UL << bit));
  } while (__builtin_expect(!!(!temp), 0));
 } else
  __mips_change_bit(nr, addr);
}
# 228 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_and_set_bit(unsigned long nr,
 volatile unsigned long *addr)
{
 int bit = nr & 31UL;
 unsigned long res;

 __asm__ __volatile__("		\n" : : :"memory");

 if (1 && 0) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  __asm__ __volatile__(
  "	.set	arch=r4000				\n"
  "1:	" "ll	" "%0, %1		# test_and_set_bit	\n"
  "	or	%2, %0, %3				\n"
  "	" "sc	" "%2, %1					\n"
  "	beqzl	%2, 1b					\n"
  "	and	%2, %0, %3				\n"
  "	.set	mips0					\n"
  : "=&r" (temp), "+" "R" (*m), "=&r" (res)
  : "r" (1UL << bit)
  : "memory");
 } else if (1) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1	# test_and_set_bit	\n"
   "	or	%2, %0, %3			\n"
   "	" "sc	" "%2, %1				\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m), "=&r" (res)
   : "r" (1UL << bit)
   : "memory");
  } while (__builtin_expect(!!(!res), 0));

  res = temp & (1UL << bit);
 } else
  res = __mips_test_and_set_bit(nr, addr);

 __asm__ __volatile__("		\n" : : :"memory");

 return res != 0;
}
# 284 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_and_set_bit_lock(unsigned long nr,
 volatile unsigned long *addr)
{
 int bit = nr & 31UL;
 unsigned long res;

 if (1 && 0) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  __asm__ __volatile__(
  "	.set	arch=r4000				\n"
  "1:	" "ll	" "%0, %1		# test_and_set_bit	\n"
  "	or	%2, %0, %3				\n"
  "	" "sc	" "%2, %1					\n"
  "	beqzl	%2, 1b					\n"
  "	and	%2, %0, %3				\n"
  "	.set	mips0					\n"
  : "=&r" (temp), "+m" (*m), "=&r" (res)
  : "r" (1UL << bit)
  : "memory");
 } else if (1) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1	# test_and_set_bit	\n"
   "	or	%2, %0, %3			\n"
   "	" "sc	" "%2, %1				\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m), "=&r" (res)
   : "r" (1UL << bit)
   : "memory");
  } while (__builtin_expect(!!(!res), 0));

  res = temp & (1UL << bit);
 } else
  res = __mips_test_and_set_bit_lock(nr, addr);

 __asm__ __volatile__("		\n" : : :"memory");

 return res != 0;
}
# 337 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_and_clear_bit(unsigned long nr,
 volatile unsigned long *addr)
{
 int bit = nr & 31UL;
 unsigned long res;

 __asm__ __volatile__("		\n" : : :"memory");

 if (1 && 0) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  __asm__ __volatile__(
  "	.set	arch=r4000				\n"
  "1:	" "ll	" "%0, %1		# test_and_clear_bit	\n"
  "	or	%2, %0, %3				\n"
  "	xor	%2, %3					\n"
  "	" "sc	" "%2, %1					\n"
  "	beqzl	%2, 1b					\n"
  "	and	%2, %0, %3				\n"
  "	.set	mips0					\n"
  : "=&r" (temp), "+" "R" (*m), "=&r" (res)
  : "r" (1UL << bit)
  : "memory");

 } else if (1 && __builtin_constant_p(nr)) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  do {
   __asm__ __volatile__(
   "	" "ll	" "%0, %1 # test_and_clear_bit	\n"
   "	" "ext	" "%2, %0, %3, 1			\n"
   "	" "ins	" "%0, $0, %3, 1			\n"
   "	" "sc	" "%0, %1				\n"
   : "=&r" (temp), "+" "R" (*m), "=&r" (res)
   : "ir" (bit)
   : "memory");
  } while (__builtin_expect(!!(!temp), 0));

 } else if (1) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1 # test_and_clear_bit	\n"
   "	or	%2, %0, %3			\n"
   "	xor	%2, %3				\n"
   "	" "sc	" "%2, %1				\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m), "=&r" (res)
   : "r" (1UL << bit)
   : "memory");
  } while (__builtin_expect(!!(!res), 0));

  res = temp & (1UL << bit);
 } else
  res = __mips_test_and_clear_bit(nr, addr);

 __asm__ __volatile__("		\n" : : :"memory");

 return res != 0;
}
# 411 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_and_change_bit(unsigned long nr,
 volatile unsigned long *addr)
{
 int bit = nr & 31UL;
 unsigned long res;

 __asm__ __volatile__("		\n" : : :"memory");

 if (1 && 0) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  __asm__ __volatile__(
  "	.set	arch=r4000				\n"
  "1:	" "ll	" "%0, %1		# test_and_change_bit	\n"
  "	xor	%2, %0, %3				\n"
  "	" "sc	" "%2, %1					\n"
  "	beqzl	%2, 1b					\n"
  "	and	%2, %0, %3				\n"
  "	.set	mips0					\n"
  : "=&r" (temp), "+" "R" (*m), "=&r" (res)
  : "r" (1UL << bit)
  : "memory");
 } else if (1) {
  unsigned long *m = ((unsigned long *) addr) + (nr >> 5);
  unsigned long temp;

  do {
   __asm__ __volatile__(
   "	.set	""arch=r4000""		\n"
   "	" "ll	" "%0, %1 # test_and_change_bit	\n"
   "	xor	%2, %0, %3			\n"
   "	" "sc	" "\t%2, %1			\n"
   "	.set	mips0				\n"
   : "=&r" (temp), "+" "R" (*m), "=&r" (res)
   : "r" (1UL << bit)
   : "memory");
  } while (__builtin_expect(!!(!res), 0));

  res = temp & (1UL << bit);
 } else
  res = __mips_test_and_change_bit(nr, addr);

 __asm__ __volatile__("		\n" : : :"memory");

 return res != 0;
}

# 1 "include/asm-generic/bitops/non-atomic.h" 1
# 15 "include/asm-generic/bitops/non-atomic.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __set_bit(int nr, volatile unsigned long *addr)
{
 unsigned long mask = (1UL << ((nr) % 32));
 unsigned long *p = ((unsigned long *)addr) + ((nr) / 32);

 *p |= mask;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __clear_bit(int nr, volatile unsigned long *addr)
{
 unsigned long mask = (1UL << ((nr) % 32));
 unsigned long *p = ((unsigned long *)addr) + ((nr) / 32);

 *p &= ~mask;
}
# 40 "include/asm-generic/bitops/non-atomic.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __change_bit(int nr, volatile unsigned long *addr)
{
 unsigned long mask = (1UL << ((nr) % 32));
 unsigned long *p = ((unsigned long *)addr) + ((nr) / 32);

 *p ^= mask;
}
# 57 "include/asm-generic/bitops/non-atomic.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __test_and_set_bit(int nr, volatile unsigned long *addr)
{
 unsigned long mask = (1UL << ((nr) % 32));
 unsigned long *p = ((unsigned long *)addr) + ((nr) / 32);
 unsigned long old = *p;

 *p = old | mask;
 return (old & mask) != 0;
}
# 76 "include/asm-generic/bitops/non-atomic.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __test_and_clear_bit(int nr, volatile unsigned long *addr)
{
 unsigned long mask = (1UL << ((nr) % 32));
 unsigned long *p = ((unsigned long *)addr) + ((nr) / 32);
 unsigned long old = *p;

 *p = old & ~mask;
 return (old & mask) != 0;
}


static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __test_and_change_bit(int nr,
         volatile unsigned long *addr)
{
 unsigned long mask = (1UL << ((nr) % 32));
 unsigned long *p = ((unsigned long *)addr) + ((nr) / 32);
 unsigned long old = *p;

 *p = old ^ mask;
 return (old & mask) != 0;
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_bit(int nr, const volatile unsigned long *addr)
{
 return 1UL & (addr[((nr) / 32)] >> (nr & (32 -1)));
}
# 460 "./arch/mips/include/asm/bitops.h" 2
# 470 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __clear_bit_unlock(unsigned long nr, volatile unsigned long *addr)
{
 __asm__ __volatile__("		\n" : : :"memory");
 __clear_bit(nr, addr);
}





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long __fls(unsigned long word)
{
 int num;

 if (32 == 32 && !__builtin_constant_p(word) &&
     __builtin_constant_p(((cpu_data[0].isa_level & 0x00000010) | (cpu_data[0].isa_level & 0x00000020) | (cpu_data[0].isa_level & 0x00000100) | (cpu_data[0].isa_level & 0x00000040) | (cpu_data[0].isa_level & 0x00000080) | (cpu_data[0].isa_level & 0x00000200))) && ((cpu_data[0].isa_level & 0x00000010) | (cpu_data[0].isa_level & 0x00000020) | (cpu_data[0].isa_level & 0x00000100) | (cpu_data[0].isa_level & 0x00000040) | (cpu_data[0].isa_level & 0x00000080) | (cpu_data[0].isa_level & 0x00000200))) {
  __asm__(
  "	.set	push					\n"
  "	.set	""mips64r2""			\n"
  "	clz	%0, %1					\n"
  "	.set	pop					\n"
  : "=r" (num)
  : "r" (word));

  return 31 - num;
 }

 if (32 == 64 && !__builtin_constant_p(word) &&
     __builtin_constant_p(((cpu_data[0].isa_level & 0x00000040) | (cpu_data[0].isa_level & 0x00000080) | (cpu_data[0].isa_level & 0x00000200))) && ((cpu_data[0].isa_level & 0x00000040) | (cpu_data[0].isa_level & 0x00000080) | (cpu_data[0].isa_level & 0x00000200))) {
  __asm__(
  "	.set	push					\n"
  "	.set	""mips64r2""			\n"
  "	dclz	%0, %1					\n"
  "	.set	pop					\n"
  : "=r" (num)
  : "r" (word));

  return 63 - num;
 }

 num = 32 - 1;







 if (!(word & (~0ul << (32 -16)))) {
  num -= 16;
  word <<= 16;
 }
 if (!(word & (~0ul << (32 -8)))) {
  num -= 8;
  word <<= 8;
 }
 if (!(word & (~0ul << (32 -4)))) {
  num -= 4;
  word <<= 4;
 }
 if (!(word & (~0ul << (32 -2)))) {
  num -= 2;
  word <<= 2;
 }
 if (!(word & (~0ul << (32 -1))))
  num -= 1;
 return num;
}
# 546 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long __ffs(unsigned long word)
{
 return __fls(word & -word);
}
# 558 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fls(int x)
{
 int r;

 if (!__builtin_constant_p(x) &&
     __builtin_constant_p(((cpu_data[0].isa_level & 0x00000010) | (cpu_data[0].isa_level & 0x00000020) | (cpu_data[0].isa_level & 0x00000100) | (cpu_data[0].isa_level & 0x00000040) | (cpu_data[0].isa_level & 0x00000080) | (cpu_data[0].isa_level & 0x00000200))) && ((cpu_data[0].isa_level & 0x00000010) | (cpu_data[0].isa_level & 0x00000020) | (cpu_data[0].isa_level & 0x00000100) | (cpu_data[0].isa_level & 0x00000040) | (cpu_data[0].isa_level & 0x00000080) | (cpu_data[0].isa_level & 0x00000200))) {
  __asm__(
  "	.set	push					\n"
  "	.set	""mips64r2""			\n"
  "	clz	%0, %1					\n"
  "	.set	pop					\n"
  : "=r" (x)
  : "r" (x));

  return 32 - x;
 }

 r = 32;
 if (!x)
  return 0;
 if (!(x & 0xffff0000u)) {
  x <<= 16;
  r -= 16;
 }
 if (!(x & 0xff000000u)) {
  x <<= 8;
  r -= 8;
 }
 if (!(x & 0xf0000000u)) {
  x <<= 4;
  r -= 4;
 }
 if (!(x & 0xc0000000u)) {
  x <<= 2;
  r -= 2;
 }
 if (!(x & 0x80000000u)) {
  x <<= 1;
  r -= 1;
 }
 return r;
}

# 1 "include/asm-generic/bitops/fls64.h" 1
# 18 "include/asm-generic/bitops/fls64.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((always_inline)) int fls64(__u64 x)
{
 __u32 h = x >> 32;
 if (h)
  return fls(h) + 32;
 return fls(x);
}
# 602 "./arch/mips/include/asm/bitops.h" 2
# 611 "./arch/mips/include/asm/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int ffs(int word)
{
 if (!word)
  return 0;

 return fls(word & -word);
}

# 1 "include/asm-generic/bitops/ffz.h" 1
# 620 "./arch/mips/include/asm/bitops.h" 2
# 1 "include/asm-generic/bitops/find.h" 1
# 14 "include/asm-generic/bitops/find.h"
extern unsigned long find_next_bit(const unsigned long *addr, unsigned long
  size, unsigned long offset);
# 28 "include/asm-generic/bitops/find.h"
extern unsigned long find_next_zero_bit(const unsigned long *addr, unsigned
  long size, unsigned long offset);
# 621 "./arch/mips/include/asm/bitops.h" 2



# 1 "include/asm-generic/bitops/sched.h" 1
# 12 "include/asm-generic/bitops/sched.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int sched_find_first_bit(const unsigned long *b)
{





 if (b[0])
  return __ffs(b[0]);
 if (b[1])
  return __ffs(b[1]) + 32;
 if (b[2])
  return __ffs(b[2]) + 64;
 return __ffs(b[3]) + 96;



}
# 625 "./arch/mips/include/asm/bitops.h" 2

# 1 "./arch/mips/include/asm/arch_hweight.h" 1
# 35 "./arch/mips/include/asm/arch_hweight.h"
# 1 "include/asm-generic/bitops/arch_hweight.h" 1





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned int __arch_hweight32(unsigned int w)
{
 return __sw_hweight32(w);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned int __arch_hweight16(unsigned int w)
{
 return __sw_hweight16(w);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned int __arch_hweight8(unsigned int w)
{
 return __sw_hweight8(w);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long __arch_hweight64(__u64 w)
{
 return __sw_hweight64(w);
}
# 36 "./arch/mips/include/asm/arch_hweight.h" 2
# 627 "./arch/mips/include/asm/bitops.h" 2
# 1 "include/asm-generic/bitops/const_hweight.h" 1
# 628 "./arch/mips/include/asm/bitops.h" 2

# 1 "include/asm-generic/bitops/le.h" 1
# 11 "include/asm-generic/bitops/le.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long find_next_zero_bit_le(const void *addr,
  unsigned long size, unsigned long offset)
{
 return find_next_zero_bit(addr, size, offset);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long find_next_bit_le(const void *addr,
  unsigned long size, unsigned long offset)
{
 return find_next_bit(addr, size, offset);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long find_first_zero_bit_le(const void *addr,
  unsigned long size)
{
 return find_next_zero_bit((addr), (size), 0);
}
# 52 "include/asm-generic/bitops/le.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_bit_le(int nr, const void *addr)
{
 return test_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void set_bit_le(int nr, void *addr)
{
 set_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void clear_bit_le(int nr, void *addr)
{
 clear_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __set_bit_le(int nr, void *addr)
{
 __set_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __clear_bit_le(int nr, void *addr)
{
 __clear_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_and_set_bit_le(int nr, void *addr)
{
 return test_and_set_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int test_and_clear_bit_le(int nr, void *addr)
{
 return test_and_clear_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __test_and_set_bit_le(int nr, void *addr)
{
 return __test_and_set_bit(nr ^ 0, addr);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __test_and_clear_bit_le(int nr, void *addr)
{
 return __test_and_clear_bit(nr ^ 0, addr);
}
# 630 "./arch/mips/include/asm/bitops.h" 2
# 1 "include/asm-generic/bitops/ext2-atomic.h" 1
# 631 "./arch/mips/include/asm/bitops.h" 2
# 37 "include/linux/bitops.h" 2
# 60 "include/linux/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int get_bitmask_order(unsigned int count)
{
 int order;

 order = fls(count);
 return order;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int get_count_order(unsigned int count)
{
 int order;

 order = fls(count) - 1;
 if (count & (count - 1))
  order++;
 return order;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((always_inline)) unsigned long hweight_long(unsigned long w)
{
 return sizeof(w) == 4 ? (__builtin_constant_p(w) ? ((((unsigned int) ((!!((w) & (1ULL << 0))) + (!!((w) & (1ULL << 1))) + (!!((w) & (1ULL << 2))) + (!!((w) & (1ULL << 3))) + (!!((w) & (1ULL << 4))) + (!!((w) & (1ULL << 5))) + (!!((w) & (1ULL << 6))) + (!!((w) & (1ULL << 7))))) + ((unsigned int) ((!!(((w) >> 8) & (1ULL << 0))) + (!!(((w) >> 8) & (1ULL << 1))) + (!!(((w) >> 8) & (1ULL << 2))) + (!!(((w) >> 8) & (1ULL << 3))) + (!!(((w) >> 8) & (1ULL << 4))) + (!!(((w) >> 8) & (1ULL << 5))) + (!!(((w) >> 8) & (1ULL << 6))) + (!!(((w) >> 8) & (1ULL << 7)))))) + (((unsigned int) ((!!(((w) >> 16) & (1ULL << 0))) + (!!(((w) >> 16) & (1ULL << 1))) + (!!(((w) >> 16) & (1ULL << 2))) + (!!(((w) >> 16) & (1ULL << 3))) + (!!(((w) >> 16) & (1ULL << 4))) + (!!(((w) >> 16) & (1ULL << 5))) + (!!(((w) >> 16) & (1ULL << 6))) + (!!(((w) >> 16) & (1ULL << 7))))) + ((unsigned int) ((!!((((w) >> 16) >> 8) & (1ULL << 0))) + (!!((((w) >> 16) >> 8) & (1ULL << 1))) + (!!((((w) >> 16) >> 8) & (1ULL << 2))) + (!!((((w) >> 16) >> 8) & (1ULL << 3))) + (!!((((w) >> 16) >> 8) & (1ULL << 4))) + (!!((((w) >> 16) >> 8) & (1ULL << 5))) + (!!((((w) >> 16) >> 8) & (1ULL << 6))) + (!!((((w) >> 16) >> 8) & (1ULL << 7))))))) : __arch_hweight32(w)) : (__builtin_constant_p(w) ? (((((unsigned int) ((!!((w) & (1ULL << 0))) + (!!((w) & (1ULL << 1))) + (!!((w) & (1ULL << 2))) + (!!((w) & (1ULL << 3))) + (!!((w) & (1ULL << 4))) + (!!((w) & (1ULL << 5))) + (!!((w) & (1ULL << 6))) + (!!((w) & (1ULL << 7))))) + ((unsigned int) ((!!(((w) >> 8) & (1ULL << 0))) + (!!(((w) >> 8) & (1ULL << 1))) + (!!(((w) >> 8) & (1ULL << 2))) + (!!(((w) >> 8) & (1ULL << 3))) + (!!(((w) >> 8) & (1ULL << 4))) + (!!(((w) >> 8) & (1ULL << 5))) + (!!(((w) >> 8) & (1ULL << 6))) + (!!(((w) >> 8) & (1ULL << 7)))))) + (((unsigned int) ((!!(((w) >> 16) & (1ULL << 0))) + (!!(((w) >> 16) & (1ULL << 1))) + (!!(((w) >> 16) & (1ULL << 2))) + (!!(((w) >> 16) & (1ULL << 3))) + (!!(((w) >> 16) & (1ULL << 4))) + (!!(((w) >> 16) & (1ULL << 5))) + (!!(((w) >> 16) & (1ULL << 6))) + (!!(((w) >> 16) & (1ULL << 7))))) + ((unsigned int) ((!!((((w) >> 16) >> 8) & (1ULL << 0))) + (!!((((w) >> 16) >> 8) & (1ULL << 1))) + (!!((((w) >> 16) >> 8) & (1ULL << 2))) + (!!((((w) >> 16) >> 8) & (1ULL << 3))) + (!!((((w) >> 16) >> 8) & (1ULL << 4))) + (!!((((w) >> 16) >> 8) & (1ULL << 5))) + (!!((((w) >> 16) >> 8) & (1ULL << 6))) + (!!((((w) >> 16) >> 8) & (1ULL << 7))))))) + ((((unsigned int) ((!!(((w) >> 32) & (1ULL << 0))) + (!!(((w) >> 32) & (1ULL << 1))) + (!!(((w) >> 32) & (1ULL << 2))) + (!!(((w) >> 32) & (1ULL << 3))) + (!!(((w) >> 32) & (1ULL << 4))) + (!!(((w) >> 32) & (1ULL << 5))) + (!!(((w) >> 32) & (1ULL << 6))) + (!!(((w) >> 32) & (1ULL << 7))))) + ((unsigned int) ((!!((((w) >> 32) >> 8) & (1ULL << 0))) + (!!((((w) >> 32) >> 8) & (1ULL << 1))) + (!!((((w) >> 32) >> 8) & (1ULL << 2))) + (!!((((w) >> 32) >> 8) & (1ULL << 3))) + (!!((((w) >> 32) >> 8) & (1ULL << 4))) + (!!((((w) >> 32) >> 8) & (1ULL << 5))) + (!!((((w) >> 32) >> 8) & (1ULL << 6))) + (!!((((w) >> 32) >> 8) & (1ULL << 7)))))) + (((unsigned int) ((!!((((w) >> 32) >> 16) & (1ULL << 0))) + (!!((((w) >> 32) >> 16) & (1ULL << 1))) + (!!((((w) >> 32) >> 16) & (1ULL << 2))) + (!!((((w) >> 32) >> 16) & (1ULL << 3))) + (!!((((w) >> 32) >> 16) & (1ULL << 4))) + (!!((((w) >> 32) >> 16) & (1ULL << 5))) + (!!((((w) >> 32) >> 16) & (1ULL << 6))) + (!!((((w) >> 32) >> 16) & (1ULL << 7))))) + ((unsigned int) ((!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 0))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 1))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 2))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 3))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 4))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 5))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 6))) + (!!(((((w) >> 32) >> 16) >> 8) & (1ULL << 7)))))))) : __arch_hweight64(w));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u64 rol64(__u64 word, unsigned int shift)
{
 return (word << shift) | (word >> (64 - shift));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u64 ror64(__u64 word, unsigned int shift)
{
 return (word >> shift) | (word << (64 - shift));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 rol32(__u32 word, unsigned int shift)
{
 return (word << shift) | (word >> ((-shift) & 31));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u32 ror32(__u32 word, unsigned int shift)
{
 return (word >> shift) | (word << (32 - shift));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u16 rol16(__u16 word, unsigned int shift)
{
 return (word << shift) | (word >> (16 - shift));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u16 ror16(__u16 word, unsigned int shift)
{
 return (word >> shift) | (word << (16 - shift));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u8 rol8(__u8 word, unsigned int shift)
{
 return (word << shift) | (word >> (8 - shift));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __u8 ror8(__u8 word, unsigned int shift)
{
 return (word >> shift) | (word << (8 - shift));
}
# 170 "include/linux/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __s32 sign_extend32(__u32 value, int index)
{
 __u8 shift = 31 - index;
 return (__s32)(value << shift) >> shift;
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __s64 sign_extend64(__u64 value, int index)
{
 __u8 shift = 63 - index;
 return (__s64)(value << shift) >> shift;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned fls_long(unsigned long l)
{
 if (sizeof(l) == 4)
  return fls(l);
 return fls64(l);
}
# 202 "include/linux/bitops.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) unsigned long __ffs64(u64 word)
{

 if (((u32)word) == 0UL)
  return __ffs((u32)(word >> 32)) + 32;



 return __ffs((unsigned long)word);
}
# 238 "include/linux/bitops.h"
extern unsigned long find_last_bit(const unsigned long *addr,
       unsigned long size);
# 11 "include/linux/kernel.h" 2
# 1 "include/linux/log2.h" 1
# 25 "include/linux/log2.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((const))
int __ilog2_u32(u32 n)
{
 return fls(n) - 1;
}



static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((const))
int __ilog2_u64(u64 n)
{
 return fls64(n) - 1;
}







static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((const))
bool is_power_of_2(unsigned long n)
{
 return (n != 0 && ((n & (n - 1)) == 0));
}




static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((const))
unsigned long __roundup_pow_of_two(unsigned long n)
{
 return 1UL << fls_long(n - 1);
}




static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((const))
unsigned long __rounddown_pow_of_two(unsigned long n)
{
 return 1UL << (fls_long(n) - 1);
}
# 12 "include/linux/kernel.h" 2
# 1 "include/linux/typecheck.h" 1
# 13 "include/linux/kernel.h" 2
# 1 "include/linux/printk.h" 1




# 1 "include/linux/init.h" 1
# 124 "include/linux/init.h"
typedef int (*initcall_t)(void);
typedef void (*exitcall_t)(void);

extern initcall_t __con_initcall_start[], __con_initcall_end[];
extern initcall_t __security_initcall_start[], __security_initcall_end[];


typedef void (*ctor_fn_t)(void);


extern int do_one_initcall(initcall_t fn);
extern char __attribute__ ((__section__(".init.data"))) boot_command_line[];
extern char *saved_command_line;
extern unsigned int reset_devices;


void setup_arch(char **);
void prepare_namespace(void);
void __attribute__ ((__section__(".init.text"))) __attribute__((__cold__)) __attribute__((no_instrument_function)) load_default_modules(void);
int __attribute__ ((__section__(".init.text"))) __attribute__((__cold__)) __attribute__((no_instrument_function)) init_rootfs(void);

extern void (*late_time_init)(void);

extern bool initcall_debug;
# 232 "include/linux/init.h"
struct obs_kernel_param {
 const char *str;
 int (*setup_func)(char *);
 int early;
};
# 281 "include/linux/init.h"
void __attribute__ ((__section__(".init.text"))) __attribute__((__cold__)) __attribute__((no_instrument_function)) parse_early_param(void);
void __attribute__ ((__section__(".init.text"))) __attribute__((__cold__)) __attribute__((no_instrument_function)) parse_early_options(char *cmdline);
# 6 "include/linux/printk.h" 2
# 1 "include/linux/kern_levels.h" 1
# 7 "include/linux/printk.h" 2

# 1 "include/linux/cache.h" 1



# 1 "include/uapi/linux/kernel.h" 1




# 1 "./include/uapi/linux/sysinfo.h" 1






struct sysinfo {
 __kernel_long_t uptime;
 __kernel_ulong_t loads[3];
 __kernel_ulong_t totalram;
 __kernel_ulong_t freeram;
 __kernel_ulong_t sharedram;
 __kernel_ulong_t bufferram;
 __kernel_ulong_t totalswap;
 __kernel_ulong_t freeswap;
 __u16 procs;
 __u16 pad;
 __kernel_ulong_t totalhigh;
 __kernel_ulong_t freehigh;
 __u32 mem_unit;
 char _f[20-2*sizeof(__kernel_ulong_t)-sizeof(__u32)];
};
# 6 "include/uapi/linux/kernel.h" 2
# 5 "include/linux/cache.h" 2
# 9 "include/linux/printk.h" 2

extern const char linux_banner[];
extern const char linux_proc_banner[];

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int printk_get_level(const char *buffer)
{
 if (buffer[0] == '\001' && buffer[1]) {
  switch (buffer[1]) {
  case '0' ... '7':
  case 'd':
   return buffer[1];
  }
 }
 return 0;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) const char *printk_skip_level(const char *buffer)
{
 if (printk_get_level(buffer))
  return buffer + 2;

 return buffer;
}
# 46 "include/linux/printk.h"
extern int console_printk[];






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void console_silent(void)
{
 (console_printk[0]) = 0;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void console_verbose(void)
{
 if ((console_printk[0]))
  (console_printk[0]) = 15;
}

struct va_format {
 const char *fmt;
 va_list *va;
};
# 111 "include/linux/printk.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((format(printf, 1, 2)))
int no_printk(const char *fmt, ...)
{
 return 0;
}


extern __attribute__((format(printf, 1, 2)))
void early_printk(const char *fmt, ...);





typedef __attribute__((format(printf, 1, 0))) int (*printk_func_t)(const char *fmt, va_list args);


 __attribute__((format(printf, 5, 0)))
int vprintk_emit(int facility, int level,
   const char *dict, size_t dictlen,
   const char *fmt, va_list args);

 __attribute__((format(printf, 1, 0)))
int vprintk(const char *fmt, va_list args);

 __attribute__((format(printf, 5, 6))) __attribute__((__cold__))
int printk_emit(int facility, int level,
  const char *dict, size_t dictlen,
  const char *fmt, ...);

 __attribute__((format(printf, 1, 2))) __attribute__((__cold__))
int printk(const char *fmt, ...);




__attribute__((format(printf, 1, 2))) __attribute__((__cold__)) int printk_deferred(const char *fmt, ...);






extern int __printk_ratelimit(const char *func);

extern bool printk_timed_ratelimit(unsigned long *caller_jiffies,
       unsigned int interval_msec);

extern int printk_delay_msec;
extern int dmesg_restrict;
extern int kptr_restrict;

extern void wake_up_klogd(void);

char *log_buf_addr_get(void);
u32 log_buf_len_get(void);
void log_buf_kexec_setup(void);
void __attribute__ ((__section__(".init.text"))) __attribute__((__cold__)) __attribute__((no_instrument_function)) setup_log_buf(int early);
__attribute__((format(printf, 1, 2))) void dump_stack_set_arch_desc(const char *fmt, ...);
void dump_stack_print_info(const char *log_lvl);
void show_regs_print_info(const char *log_lvl);
# 233 "include/linux/printk.h"
extern void dump_stack(void) __attribute__((__cold__));
# 277 "include/linux/printk.h"
# 1 "include/linux/dynamic_debug.h" 1
# 9 "include/linux/dynamic_debug.h"
struct _ddebug {




 const char *modname;
 const char *function;
 const char *filename;
 const char *format;
 unsigned int lineno:18;
# 35 "include/linux/dynamic_debug.h"
 unsigned int flags:8;
} __attribute__((aligned(8)));


int ddebug_add_module(struct _ddebug *tab, unsigned int n,
    const char *modname);
# 111 "include/linux/dynamic_debug.h"
# 1 "include/linux/string.h" 1
# 9 "include/linux/string.h"
# 1 "include/uapi/linux/string.h" 1
# 10 "include/linux/string.h" 2

extern char *strndup_user(const char *, long);
extern void *memdup_user(const void *, size_t);




# 1 "./arch/mips/include/asm/string.h" 1
# 23 "./arch/mips/include/asm/string.h"
static __inline__ __attribute__((always_inline)) __attribute__((no_instrument_function)) char *strcpy(char *__dest, __const__ char *__src)
{
  char *__xdest = __dest;

  __asm__ __volatile__(
 ".set\tnoreorder\n\t"
 ".set\tnoat\n"
 "1:\tlbu\t$1,(%1)\n\t"
 "addiu\t%1,1\n\t"
 "sb\t$1,(%0)\n\t"
 "bnez\t$1,1b\n\t"
 "addiu\t%0,1\n\t"
 ".set\tat\n\t"
 ".set\treorder"
 : "=r" (__dest), "=r" (__src)
 : "0" (__dest), "1" (__src)
 : "memory");

  return __xdest;
}


static __inline__ __attribute__((always_inline)) __attribute__((no_instrument_function)) char *strncpy(char *__dest, __const__ char *__src, size_t __n)
{
  char *__xdest = __dest;

  if (__n == 0)
    return __xdest;

  __asm__ __volatile__(
 ".set\tnoreorder\n\t"
 ".set\tnoat\n"
 "1:\tlbu\t$1,(%1)\n\t"
 "subu\t%2,1\n\t"
 "sb\t$1,(%0)\n\t"
 "beqz\t$1,2f\n\t"
 "addiu\t%0,1\n\t"
 "bnez\t%2,1b\n\t"
 "addiu\t%1,1\n"
 "2:\n\t"
 ".set\tat\n\t"
 ".set\treorder"
 : "=r" (__dest), "=r" (__src), "=r" (__n)
 : "0" (__dest), "1" (__src), "2" (__n)
 : "memory");

  return __xdest;
}


static __inline__ __attribute__((always_inline)) __attribute__((no_instrument_function)) int strcmp(__const__ char *__cs, __const__ char *__ct)
{
  int __res;

  __asm__ __volatile__(
 ".set\tnoreorder\n\t"
 ".set\tnoat\n\t"
 "lbu\t%2,(%0)\n"
 "1:\tlbu\t$1,(%1)\n\t"
 "addiu\t%0,1\n\t"
 "bne\t$1,%2,2f\n\t"
 "addiu\t%1,1\n\t"
 "bnez\t%2,1b\n\t"
 "lbu\t%2,(%0)\n\t"



 "move\t%2,$1\n"
 "2:\tsubu\t%2,$1\n"
 "3:\t.set\tat\n\t"
 ".set\treorder"
 : "=r" (__cs), "=r" (__ct), "=r" (__res)
 : "0" (__cs), "1" (__ct));

  return __res;
}




static __inline__ __attribute__((always_inline)) __attribute__((no_instrument_function)) int
strncmp(__const__ char *__cs, __const__ char *__ct, size_t __count)
{
 int __res;

 __asm__ __volatile__(
 ".set\tnoreorder\n\t"
 ".set\tnoat\n"
 "1:\tlbu\t%3,(%0)\n\t"
 "beqz\t%2,2f\n\t"
 "lbu\t$1,(%1)\n\t"
 "subu\t%2,1\n\t"
 "bne\t$1,%3,3f\n\t"
 "addiu\t%0,1\n\t"
 "bnez\t%3,1b\n\t"
 "addiu\t%1,1\n"
 "2:\n\t"



 "move\t%3,$1\n"
 "3:\tsubu\t%3,$1\n\t"
 ".set\tat\n\t"
 ".set\treorder"
 : "=r" (__cs), "=r" (__ct), "=r" (__count), "=r" (__res)
 : "0" (__cs), "1" (__ct), "2" (__count));

 return __res;
}



extern void *memset(void *__s, int __c, size_t __count);
# 148 "./arch/mips/include/asm/string.h"
extern void *memcpy(void *__to, __const__ void *__from, size_t __n);
# 161 "./arch/mips/include/asm/string.h"
extern void *memmove(void *__dest, __const__ void *__src, size_t __n);
# 18 "include/linux/string.h" 2
# 26 "include/linux/string.h"
size_t strlcpy(char *, const char *, size_t);


ssize_t __attribute__((warn_unused_result)) strscpy(char *, const char *, size_t);


extern char * strcat(char *, const char *);


extern char * strncat(char *, const char *, __kernel_size_t);


extern size_t strlcat(char *, const char *, __kernel_size_t);
# 47 "include/linux/string.h"
extern int strcasecmp(const char *s1, const char *s2);


extern int strncasecmp(const char *s1, const char *s2, size_t n);


extern char * strchr(const char *,int);


extern char * strchrnul(const char *,int);


extern char * strnchr(const char *, size_t, int);


extern char * strrchr(const char *,int);

extern char * __attribute__((warn_unused_result)) skip_spaces(const char *);

extern char *strim(char *);

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((warn_unused_result)) char *strstrip(char *str)
{
 return strim(str);
}


extern char * strstr(const char *, const char *);


extern char * strnstr(const char *, const char *, size_t);


extern __kernel_size_t strlen(const char *);


extern __kernel_size_t strnlen(const char *,__kernel_size_t);


extern char * strpbrk(const char *,const char *);


extern char * strsep(char **,const char *);


extern __kernel_size_t strspn(const char *,const char *);


extern __kernel_size_t strcspn(const char *,const char *);
# 108 "include/linux/string.h"
extern void * memscan(void *,int,__kernel_size_t);





extern void * memchr(const void *,int,__kernel_size_t);

void *memchr_inv(const void *s, int c, size_t n);
char *strreplace(char *s, char old, char new);

extern void kfree_const(const void *x);

extern char *kstrdup(const char *s, gfp_t gfp);
extern const char *kstrdup_const(const char *s, gfp_t gfp);
extern char *kstrndup(const char *s, size_t len, gfp_t gfp);
extern void *kmemdup(const void *src, size_t len, gfp_t gfp);

extern char **argv_split(gfp_t gfp, const char *str, int *argcp);
extern void argv_free(char **argv);

extern bool sysfs_streq(const char *s1, const char *s2);
extern int kstrtobool(const char *s, bool *res);
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int strtobool(const char *s, bool *res)
{
 return kstrtobool(s, res);
}







extern ssize_t memory_read_from_buffer(void *to, size_t count, loff_t *ppos,
           const void *from, size_t available);






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) bool strstarts(const char *str, const char *prefix)
{
 return strncmp(str, prefix, strlen(prefix)) == 0;
}

size_t memweight(const void *ptr, size_t bytes);
void memzero_explicit(void *s, size_t count);






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) const char *kbasename(const char *path)
{
 const char *tail = strrchr(path, '/');
 return tail ? tail + 1 : path;
}
# 112 "include/linux/dynamic_debug.h" 2
# 1 "include/linux/errno.h" 1



# 1 "include/uapi/linux/errno.h" 1
# 1 "./arch/mips/include/asm/errno.h" 1
# 11 "./arch/mips/include/asm/errno.h"
# 1 "./arch/mips/include/uapi/asm/errno.h" 1
# 15 "./arch/mips/include/uapi/asm/errno.h"
# 1 "./include/uapi/asm-generic/errno-base.h" 1
# 16 "./arch/mips/include/uapi/asm/errno.h" 2
# 12 "./arch/mips/include/asm/errno.h" 2
# 1 "include/uapi/linux/errno.h" 2
# 5 "include/linux/errno.h" 2
# 113 "include/linux/dynamic_debug.h" 2

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int ddebug_remove_module(const char *mod)
{
 return 0;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int ddebug_dyndbg_module_param_cb(char *param, char *val,
      const char *modname)
{
 if (strstr(param, "dyndbg")) {

  printk("\001" "4" "dyndbg param is supported only in "
   "CONFIG_DYNAMIC_DEBUG builds\n");
  return 0;
 }
 return -22;
}
# 278 "include/linux/printk.h" 2
# 420 "include/linux/printk.h"
extern const struct file_operations kmsg_fops;

enum {
 DUMP_PREFIX_NONE,
 DUMP_PREFIX_ADDRESS,
 DUMP_PREFIX_OFFSET
};
extern int hex_dump_to_buffer(const void *buf, size_t len, int rowsize,
         int groupsize, char *linebuf, size_t linebuflen,
         bool ascii);

extern void print_hex_dump(const char *level, const char *prefix_str,
      int prefix_type, int rowsize, int groupsize,
      const void *buf, size_t len, bool ascii);




extern void print_hex_dump_bytes(const char *prefix_str, int prefix_type,
     const void *buf, size_t len);
# 465 "include/linux/printk.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void print_hex_dump_debug(const char *prefix_str, int prefix_type,
     int rowsize, int groupsize,
     const void *buf, size_t len, bool ascii)
{
}
# 14 "include/linux/kernel.h" 2
# 165 "include/linux/kernel.h"
struct completion;
struct pt_regs;
struct user;


extern int _cond_resched(void);
# 193 "include/linux/kernel.h"
  static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void ___might_sleep(const char *file, int line,
       int preempt_offset) { }
  static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __might_sleep(const char *file, int line,
       int preempt_offset) { }
# 240 "include/linux/kernel.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u32 reciprocal_scale(u32 val, u32 ep_ro)
{
 return (u32)(((u64) val * ep_ro) >> 32);
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void might_fault(void) { }


extern struct atomic_notifier_head panic_notifier_list;
extern long (*panic_blink)(int state);
__attribute__((format(printf, 1, 2)))
void panic(const char *fmt, ...)
 __attribute__((noreturn)) __attribute__((__cold__));
extern void oops_enter(void);
extern void oops_exit(void);
void print_oops_end_marker(void);
extern int oops_may_print(void);
void do_exit(long error_code)
 __attribute__((noreturn));
void complete_and_exit(struct completion *, long)
 __attribute__((noreturn));


int __attribute__((warn_unused_result)) _kstrtoul(const char *s, unsigned int base, unsigned long *res);
int __attribute__((warn_unused_result)) _kstrtol(const char *s, unsigned int base, long *res);

int __attribute__((warn_unused_result)) kstrtoull(const char *s, unsigned int base, unsigned long long *res);
int __attribute__((warn_unused_result)) kstrtoll(const char *s, unsigned int base, long long *res);
# 290 "include/linux/kernel.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtoul(const char *s, unsigned int base, unsigned long *res)
{




 if (sizeof(unsigned long) == sizeof(unsigned long long) &&
     __alignof__(unsigned long) == __alignof__(unsigned long long))
  return kstrtoull(s, base, (unsigned long long *)res);
 else
  return _kstrtoul(s, base, res);
}
# 319 "include/linux/kernel.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtol(const char *s, unsigned int base, long *res)
{




 if (sizeof(long) == sizeof(long long) &&
     __alignof__(long) == __alignof__(long long))
  return kstrtoll(s, base, (long long *)res);
 else
  return _kstrtol(s, base, res);
}

int __attribute__((warn_unused_result)) kstrtouint(const char *s, unsigned int base, unsigned int *res);
int __attribute__((warn_unused_result)) kstrtoint(const char *s, unsigned int base, int *res);

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtou64(const char *s, unsigned int base, u64 *res)
{
 return kstrtoull(s, base, res);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtos64(const char *s, unsigned int base, s64 *res)
{
 return kstrtoll(s, base, res);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtou32(const char *s, unsigned int base, u32 *res)
{
 return kstrtouint(s, base, res);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtos32(const char *s, unsigned int base, s32 *res)
{
 return kstrtoint(s, base, res);
}

int __attribute__((warn_unused_result)) kstrtou16(const char *s, unsigned int base, u16 *res);
int __attribute__((warn_unused_result)) kstrtos16(const char *s, unsigned int base, s16 *res);
int __attribute__((warn_unused_result)) kstrtou8(const char *s, unsigned int base, u8 *res);
int __attribute__((warn_unused_result)) kstrtos8(const char *s, unsigned int base, s8 *res);
int __attribute__((warn_unused_result)) kstrtobool(const char *s, bool *res);

int __attribute__((warn_unused_result)) kstrtoull_from_user(const char *s, size_t count, unsigned int base, unsigned long long *res);
int __attribute__((warn_unused_result)) kstrtoll_from_user(const char *s, size_t count, unsigned int base, long long *res);
int __attribute__((warn_unused_result)) kstrtoul_from_user(const char *s, size_t count, unsigned int base, unsigned long *res);
int __attribute__((warn_unused_result)) kstrtol_from_user(const char *s, size_t count, unsigned int base, long *res);
int __attribute__((warn_unused_result)) kstrtouint_from_user(const char *s, size_t count, unsigned int base, unsigned int *res);
int __attribute__((warn_unused_result)) kstrtoint_from_user(const char *s, size_t count, unsigned int base, int *res);
int __attribute__((warn_unused_result)) kstrtou16_from_user(const char *s, size_t count, unsigned int base, u16 *res);
int __attribute__((warn_unused_result)) kstrtos16_from_user(const char *s, size_t count, unsigned int base, s16 *res);
int __attribute__((warn_unused_result)) kstrtou8_from_user(const char *s, size_t count, unsigned int base, u8 *res);
int __attribute__((warn_unused_result)) kstrtos8_from_user(const char *s, size_t count, unsigned int base, s8 *res);
int __attribute__((warn_unused_result)) kstrtobool_from_user(const char *s, size_t count, bool *res);

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtou64_from_user(const char *s, size_t count, unsigned int base, u64 *res)
{
 return kstrtoull_from_user(s, count, base, res);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtos64_from_user(const char *s, size_t count, unsigned int base, s64 *res)
{
 return kstrtoll_from_user(s, count, base, res);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtou32_from_user(const char *s, size_t count, unsigned int base, u32 *res)
{
 return kstrtouint_from_user(s, count, base, res);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int __attribute__((warn_unused_result)) kstrtos32_from_user(const char *s, size_t count, unsigned int base, s32 *res)
{
 return kstrtoint_from_user(s, count, base, res);
}



extern unsigned long simple_strtoul(const char *,char **,unsigned int);
extern long simple_strtol(const char *,char **,unsigned int);
extern unsigned long long simple_strtoull(const char *,char **,unsigned int);
extern long long simple_strtoll(const char *,char **,unsigned int);

extern int num_to_str(char *buf, int size, unsigned long long num);



extern __attribute__((format(printf, 2, 3))) int sprintf(char *buf, const char * fmt, ...);
extern __attribute__((format(printf, 2, 0))) int vsprintf(char *buf, const char *, va_list);
extern __attribute__((format(printf, 3, 4)))
int snprintf(char *buf, size_t size, const char *fmt, ...);
extern __attribute__((format(printf, 3, 0)))
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern __attribute__((format(printf, 3, 4)))
int scnprintf(char *buf, size_t size, const char *fmt, ...);
extern __attribute__((format(printf, 3, 0)))
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern __attribute__((format(printf, 2, 3)))
char *kasprintf(gfp_t gfp, const char *fmt, ...);
extern __attribute__((format(printf, 2, 0)))
char *kvasprintf(gfp_t gfp, const char *fmt, va_list args);
extern __attribute__((format(printf, 2, 0)))
const char *kvasprintf_const(gfp_t gfp, const char *fmt, va_list args);

extern __attribute__((format(scanf, 2, 3)))
int sscanf(const char *, const char *, ...);
extern __attribute__((format(scanf, 2, 0)))
int vsscanf(const char *, const char *, va_list);

extern int get_option(char **str, int *pint);
extern char *get_options(const char *str, int nints, int *ints);
extern unsigned long long memparse(const char *ptr, char **retptr);
extern bool parse_option_str(const char *str, const char *option);

extern int core_kernel_text(unsigned long addr);
extern int core_kernel_data(unsigned long addr);
extern int __kernel_text_address(unsigned long addr);
extern int kernel_text_address(unsigned long addr);
extern int func_ptr_is_kernel_text(void *ptr);

unsigned long int_sqrt(unsigned long);

extern void bust_spinlocks(int yes);
extern int oops_in_progress;
extern int panic_timeout;
extern int panic_on_oops;
extern int panic_on_unrecovered_nmi;
extern int panic_on_io_nmi;
extern int panic_on_warn;
extern int sysctl_panic_on_stackoverflow;

extern bool crash_kexec_post_notifiers;





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void set_arch_panic_timeout(int timeout, int arch_default_timeout)
{
 if (panic_timeout == arch_default_timeout)
  panic_timeout = timeout;
}
extern const char *print_tainted(void);
enum lockdep_ok {
 LOCKDEP_STILL_OK,
 LOCKDEP_NOW_UNRELIABLE
};
extern void add_taint(unsigned flag, enum lockdep_ok);
extern int test_taint(unsigned flag);
extern unsigned long get_taint(void);
extern int root_mountflags;

extern bool early_boot_irqs_disabled;


extern enum system_states {
 SYSTEM_BOOTING,
 SYSTEM_RUNNING,
 SYSTEM_HALT,
 SYSTEM_POWER_OFF,
 SYSTEM_RESTART,
} system_state;
# 497 "include/linux/kernel.h"
extern const char hex_asc[];



static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) char *hex_byte_pack(char *buf, u8 byte)
{
 *buf++ = hex_asc[((byte) & 0xf0) >> 4];
 *buf++ = hex_asc[((byte) & 0x0f)];
 return buf;
}

extern const char hex_asc_upper[];



static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) char *hex_byte_pack_upper(char *buf, u8 byte)
{
 *buf++ = hex_asc_upper[((byte) & 0xf0) >> 4];
 *buf++ = hex_asc_upper[((byte) & 0x0f)];
 return buf;
}

extern int hex_to_bin(char ch);
extern int __attribute__((warn_unused_result)) hex2bin(u8 *dst, const char *src, size_t count);
extern char *bin2hex(char *dst, const void *src, size_t count);

bool mac_pton(const char *s, u8 *mac);
# 545 "include/linux/kernel.h"
enum ftrace_dump_mode {
 DUMP_NONE,
 DUMP_ALL,
 DUMP_ORIG,
};
# 695 "include/linux/kernel.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void tracing_start(void) { }
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void tracing_stop(void) { }
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void trace_dump_stack(int skip) { }

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void tracing_on(void) { }
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void tracing_off(void) { }
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int tracing_is_on(void) { return 0; }
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void tracing_snapshot(void) { }
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void tracing_snapshot_alloc(void) { }

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) __attribute__((format(printf, 1, 2)))
int trace_printk(const char *fmt, ...)
{
 return 0;
}
static __attribute__((format(printf, 1, 0))) inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int
ftrace_vprintk(const char *fmt, va_list ap)
{
 return 0;
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void ftrace_dump(enum ftrace_dump_mode oops_dump_mode) { }
# 16 "arch/mips/boot/compressed/decompress.c" 2

# 1 "include/linux/libfdt.h" 1



# 1 "include/linux/libfdt_env.h" 1







typedef __be16 fdt16_t;
typedef __be32 fdt32_t;
typedef __be64 fdt64_t;
# 5 "include/linux/libfdt.h" 2
# 1 "include/linux/../../scripts/dtc/libfdt/fdt.h" 1
# 57 "include/linux/../../scripts/dtc/libfdt/fdt.h"
struct fdt_header {
 fdt32_t magic;
 fdt32_t totalsize;
 fdt32_t off_dt_struct;
 fdt32_t off_dt_strings;
 fdt32_t off_mem_rsvmap;
 fdt32_t version;
 fdt32_t last_comp_version;


 fdt32_t boot_cpuid_phys;


 fdt32_t size_dt_strings;


 fdt32_t size_dt_struct;
};

struct fdt_reserve_entry {
 fdt64_t address;
 fdt64_t size;
};

struct fdt_node_header {
 fdt32_t tag;
 char name[0];
};

struct fdt_property {
 fdt32_t tag;
 fdt32_t len;
 fdt32_t nameoff;
 char data[0];
};
# 6 "include/linux/libfdt.h" 2
# 1 "include/linux/../../scripts/dtc/libfdt/libfdt.h" 1
# 54 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
# 1 "include/linux/../../scripts/dtc/libfdt/libfdt_env.h" 1
# 55 "include/linux/../../scripts/dtc/libfdt/libfdt.h" 2
# 1 "include/linux/../../scripts/dtc/libfdt/fdt.h" 1
# 56 "include/linux/../../scripts/dtc/libfdt/libfdt.h" 2
# 130 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const void *fdt_offset_ptr(const void *fdt, int offset, unsigned int checklen);
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void *fdt_offset_ptr_w(void *fdt, int offset, int checklen)
{
 return (void *)(uintptr_t)fdt_offset_ptr(fdt, offset, checklen);
}

uint32_t fdt_next_tag(const void *fdt, int offset, int *nextoffset);





int fdt_next_node(const void *fdt, int offset, int *depth);
# 151 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_first_subnode(const void *fdt, int offset);
# 164 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_next_subnode(const void *fdt, int offset);
# 189 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_magic(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->magic = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_totalsize(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->totalsize = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_off_dt_struct(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->off_dt_struct = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_off_dt_strings(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->off_dt_strings = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_off_mem_rsvmap(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->off_mem_rsvmap = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_version(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->version = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_last_comp_version(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->last_comp_version = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_boot_cpuid_phys(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->boot_cpuid_phys = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_size_dt_strings(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->size_dt_strings = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void fdt_set_size_dt_struct(void *fdt, uint32_t val) { struct fdt_header *fdth = (struct fdt_header*)fdt; fdth->size_dt_struct = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val)))); };
# 215 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_check_header(const void *fdt);
# 236 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_move(const void *fdt, void *buf, int bufsize);
# 254 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const char *fdt_string(const void *fdt, int stroffset);
# 267 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_num_mem_rsv(const void *fdt);
# 284 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_get_mem_rsv(const void *fdt, int n, uint64_t *address, uint64_t *size);
# 298 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_subnode_offset_namelen(const void *fdt, int parentoffset,
          const char *name, int namelen);
# 323 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_subnode_offset(const void *fdt, int parentoffset, const char *name);
# 334 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_path_offset_namelen(const void *fdt, const char *path, int namelen);
# 358 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_path_offset(const void *fdt, const char *path);
# 381 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const char *fdt_get_name(const void *fdt, int nodeoffset, int *lenp);
# 401 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_first_property_offset(const void *fdt, int nodeoffset);
# 422 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_next_property_offset(const void *fdt, int offset);
# 448 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const struct fdt_property *fdt_get_property_by_offset(const void *fdt,
            int offset,
            int *lenp);
# 463 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const struct fdt_property *fdt_get_property_namelen(const void *fdt,
          int nodeoffset,
          const char *name,
          int namelen, int *lenp);
# 495 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const struct fdt_property *fdt_get_property(const void *fdt, int nodeoffset,
         const char *name, int *lenp);
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) struct fdt_property *fdt_get_property_w(void *fdt, int nodeoffset,
            const char *name,
            int *lenp)
{
 return (struct fdt_property *)(uintptr_t)
  fdt_get_property(fdt, nodeoffset, name, lenp);
}
# 536 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const void *fdt_getprop_by_offset(const void *fdt, int offset,
      const char **namep, int *lenp);
# 550 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const void *fdt_getprop_namelen(const void *fdt, int nodeoffset,
    const char *name, int namelen, int *lenp);
# 580 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const void *fdt_getprop(const void *fdt, int nodeoffset,
   const char *name, int *lenp);
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void *fdt_getprop_w(void *fdt, int nodeoffset,
      const char *name, int *lenp)
{
 return (void *)(uintptr_t)fdt_getprop(fdt, nodeoffset, name, lenp);
}
# 600 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
uint32_t fdt_get_phandle(const void *fdt, int nodeoffset);
# 611 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const char *fdt_get_alias_namelen(const void *fdt,
      const char *name, int namelen);
# 626 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
const char *fdt_get_alias(const void *fdt, const char *name);
# 653 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_get_path(const void *fdt, int nodeoffset, char *buf, int buflen);
# 685 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_supernode_atdepth_offset(const void *fdt, int nodeoffset,
     int supernodedepth, int *nodedepth);
# 707 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_node_depth(const void *fdt, int nodeoffset);
# 730 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_parent_offset(const void *fdt, int nodeoffset);
# 770 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_node_offset_by_prop_value(const void *fdt, int startoffset,
      const char *propname,
      const void *propval, int proplen);
# 793 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_node_offset_by_phandle(const void *fdt, uint32_t phandle);
# 817 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_node_check_compatible(const void *fdt, int nodeoffset,
         const char *compatible);
# 854 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_node_offset_by_compatible(const void *fdt, int startoffset,
      const char *compatible);
# 869 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_stringlist_contains(const char *strlist, int listlen, const char *str);
# 903 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_address_cells(const void *fdt, int nodeoffset);
# 923 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_size_cells(const void *fdt, int nodeoffset);
# 958 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_setprop_inplace(void *fdt, int nodeoffset, const char *name,
   const void *val, int len);
# 989 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_setprop_inplace_u32(void *fdt, int nodeoffset,
       const char *name, uint32_t val)
{
 fdt32_t tmp = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val))));
 return fdt_setprop_inplace(fdt, nodeoffset, name, &tmp, sizeof(tmp));
}
# 1024 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_setprop_inplace_u64(void *fdt, int nodeoffset,
       const char *name, uint64_t val)
{
 fdt64_t tmp = (( __be64)(__builtin_constant_p((__u64)((val))) ? ((__u64)( (((__u64)((val)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)((val)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)((val)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)((val)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)((val)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)((val)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)((val)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)((val)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64((val))));
 return fdt_setprop_inplace(fdt, nodeoffset, name, &tmp, sizeof(tmp));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_setprop_inplace_cell(void *fdt, int nodeoffset,
        const char *name, uint32_t val)
{
 return fdt_setprop_inplace_u32(fdt, nodeoffset, name, val);
}
# 1066 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_nop_property(void *fdt, int nodeoffset, const char *name);
# 1090 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_nop_node(void *fdt, int nodeoffset);





int fdt_create(void *buf, int bufsize);
int fdt_resize(void *fdt, void *buf, int bufsize);
int fdt_add_reservemap_entry(void *fdt, uint64_t addr, uint64_t size);
int fdt_finish_reservemap(void *fdt);
int fdt_begin_node(void *fdt, const char *name);
int fdt_property(void *fdt, const char *name, const void *val, int len);
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_property_u32(void *fdt, const char *name, uint32_t val)
{
 fdt32_t tmp = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val))));
 return fdt_property(fdt, name, &tmp, sizeof(tmp));
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_property_u64(void *fdt, const char *name, uint64_t val)
{
 fdt64_t tmp = (( __be64)(__builtin_constant_p((__u64)((val))) ? ((__u64)( (((__u64)((val)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)((val)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)((val)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)((val)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)((val)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)((val)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)((val)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)((val)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64((val))));
 return fdt_property(fdt, name, &tmp, sizeof(tmp));
}
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_property_cell(void *fdt, const char *name, uint32_t val)
{
 return fdt_property_u32(fdt, name, val);
}


int fdt_end_node(void *fdt);
int fdt_finish(void *fdt);





int fdt_create_empty_tree(void *buf, int bufsize);
int fdt_open_into(const void *fdt, void *buf, int bufsize);
int fdt_pack(void *fdt);
# 1151 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_add_mem_rsv(void *fdt, uint64_t address, uint64_t size);
# 1175 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_del_mem_rsv(void *fdt, int n);
# 1201 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_set_name(void *fdt, int nodeoffset, const char *name);
# 1231 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_setprop(void *fdt, int nodeoffset, const char *name,
  const void *val, int len);
# 1262 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_setprop_u32(void *fdt, int nodeoffset, const char *name,
      uint32_t val)
{
 fdt32_t tmp = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val))));
 return fdt_setprop(fdt, nodeoffset, name, &tmp, sizeof(tmp));
}
# 1297 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_setprop_u64(void *fdt, int nodeoffset, const char *name,
      uint64_t val)
{
 fdt64_t tmp = (( __be64)(__builtin_constant_p((__u64)((val))) ? ((__u64)( (((__u64)((val)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)((val)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)((val)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)((val)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)((val)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)((val)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)((val)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)((val)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64((val))));
 return fdt_setprop(fdt, nodeoffset, name, &tmp, sizeof(tmp));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_setprop_cell(void *fdt, int nodeoffset, const char *name,
       uint32_t val)
{
 return fdt_setprop_u32(fdt, nodeoffset, name, val);
}
# 1373 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_appendprop(void *fdt, int nodeoffset, const char *name,
     const void *val, int len);
# 1404 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_appendprop_u32(void *fdt, int nodeoffset,
         const char *name, uint32_t val)
{
 fdt32_t tmp = (( __be32)(__builtin_constant_p((__u32)((val))) ? ((__u32)( (((__u32)((val)) & (__u32)0x000000ffUL) << 24) | (((__u32)((val)) & (__u32)0x0000ff00UL) << 8) | (((__u32)((val)) & (__u32)0x00ff0000UL) >> 8) | (((__u32)((val)) & (__u32)0xff000000UL) >> 24))) : __fswab32((val))));
 return fdt_appendprop(fdt, nodeoffset, name, &tmp, sizeof(tmp));
}
# 1439 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_appendprop_u64(void *fdt, int nodeoffset,
         const char *name, uint64_t val)
{
 fdt64_t tmp = (( __be64)(__builtin_constant_p((__u64)((val))) ? ((__u64)( (((__u64)((val)) & (__u64)0x00000000000000ffULL) << 56) | (((__u64)((val)) & (__u64)0x000000000000ff00ULL) << 40) | (((__u64)((val)) & (__u64)0x0000000000ff0000ULL) << 24) | (((__u64)((val)) & (__u64)0x00000000ff000000ULL) << 8) | (((__u64)((val)) & (__u64)0x000000ff00000000ULL) >> 8) | (((__u64)((val)) & (__u64)0x0000ff0000000000ULL) >> 24) | (((__u64)((val)) & (__u64)0x00ff000000000000ULL) >> 40) | (((__u64)((val)) & (__u64)0xff00000000000000ULL) >> 56))) : __fswab64((val))));
 return fdt_appendprop(fdt, nodeoffset, name, &tmp, sizeof(tmp));
}






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) int fdt_appendprop_cell(void *fdt, int nodeoffset,
          const char *name, uint32_t val)
{
 return fdt_appendprop_u32(fdt, nodeoffset, name, val);
}
# 1509 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_delprop(void *fdt, int nodeoffset, const char *name);
# 1523 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_add_subnode_namelen(void *fdt, int parentoffset,
       const char *name, int namelen);
# 1555 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_add_subnode(void *fdt, int parentoffset, const char *name);
# 1578 "include/linux/../../scripts/dtc/libfdt/libfdt.h"
int fdt_del_node(void *fdt, int nodeoffset);





const char *fdt_strerror(int errval);
# 7 "include/linux/libfdt.h" 2
# 18 "arch/mips/boot/compressed/decompress.c" 2







unsigned long free_mem_ptr;
unsigned long free_mem_end_ptr;


extern unsigned char __image_begin, __image_end;



extern void puts(const char *s);
extern void puthex(unsigned long long val);





extern char __appended_dtb[];

void error(char *x)
{
#if 0
 puts("\n\n");
 puts(x);
 puts("\n\n -- System halted");
#endif
 while (1)
  ;
}
# 72 "arch/mips/boot/compressed/decompress.c"
# 1 "arch/mips/boot/compressed/../../../../lib/decompress_unlzo.c" 1
# 35 "arch/mips/boot/compressed/../../../../lib/decompress_unlzo.c"
# 1 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c" 1
# 18 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c"
# 1 "./arch/mips/include/asm/unaligned.h" 1
# 18 "./arch/mips/include/asm/unaligned.h"
# 1 "include/linux/unaligned/le_struct.h" 1



# 1 "include/linux/unaligned/packed_struct.h" 1





struct __una_u16 { u16 x; } __attribute__((packed));
struct __una_u32 { u32 x; } __attribute__((packed));
struct __una_u64 { u64 x; } __attribute__((packed));

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u16 __get_unaligned_cpu16(const void *p)
{
 const struct __una_u16 *ptr = (const struct __una_u16 *)p;
 return ptr->x;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u32 __get_unaligned_cpu32(const void *p)
{
 const struct __una_u32 *ptr = (const struct __una_u32 *)p;
 return ptr->x;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u64 __get_unaligned_cpu64(const void *p)
{
 const struct __una_u64 *ptr = (const struct __una_u64 *)p;
 return ptr->x;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __put_unaligned_cpu16(u16 val, void *p)
{
 struct __una_u16 *ptr = (struct __una_u16 *)p;
 ptr->x = val;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __put_unaligned_cpu32(u32 val, void *p)
{
 struct __una_u32 *ptr = (struct __una_u32 *)p;
 ptr->x = val;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __put_unaligned_cpu64(u64 val, void *p)
{
 struct __una_u64 *ptr = (struct __una_u64 *)p;
 ptr->x = val;
}
# 5 "include/linux/unaligned/le_struct.h" 2

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u16 get_unaligned_le16(const void *p)
{
 return __get_unaligned_cpu16((const u8 *)p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u32 get_unaligned_le32(const void *p)
{
 return __get_unaligned_cpu32((const u8 *)p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u64 get_unaligned_le64(const void *p)
{
 return __get_unaligned_cpu64((const u8 *)p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void put_unaligned_le16(u16 val, void *p)
{
 __put_unaligned_cpu16(val, p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void put_unaligned_le32(u32 val, void *p)
{
 __put_unaligned_cpu32(val, p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void put_unaligned_le64(u64 val, void *p)
{
 __put_unaligned_cpu64(val, p);
}
# 19 "./arch/mips/include/asm/unaligned.h" 2
# 1 "include/linux/unaligned/be_byteshift.h" 1





static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u16 __get_unaligned_be16(const u8 *p)
{
 return p[0] << 8 | p[1];
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u32 __get_unaligned_be32(const u8 *p)
{
 return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u64 __get_unaligned_be64(const u8 *p)
{
 return (u64)__get_unaligned_be32(p) << 32 |
        __get_unaligned_be32(p + 4);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __put_unaligned_be16(u16 val, u8 *p)
{
 *p++ = val >> 8;
 *p++ = val;
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __put_unaligned_be32(u32 val, u8 *p)
{
 __put_unaligned_be16(val >> 16, p);
 __put_unaligned_be16(val, p + 2);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void __put_unaligned_be64(u64 val, u8 *p)
{
 __put_unaligned_be32(val >> 32, p);
 __put_unaligned_be32(val, p + 4);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u16 get_unaligned_be16(const void *p)
{
 return __get_unaligned_be16((const u8 *)p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u32 get_unaligned_be32(const void *p)
{
 return __get_unaligned_be32((const u8 *)p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) u64 get_unaligned_be64(const void *p)
{
 return __get_unaligned_be64((const u8 *)p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void put_unaligned_be16(u16 val, void *p)
{
 __put_unaligned_be16(val, p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void put_unaligned_be32(u32 val, void *p)
{
 __put_unaligned_be32(val, p);
}

static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) void put_unaligned_be64(u64 val, void *p)
{
 __put_unaligned_be64(val, p);
}
# 20 "./arch/mips/include/asm/unaligned.h" 2






# 1 "include/linux/unaligned/generic.h" 1







extern void __bad_unaligned_access_size(void);
# 27 "./arch/mips/include/asm/unaligned.h" 2
# 19 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c" 2
# 1 "include/linux/lzo.h" 1
# 23 "include/linux/lzo.h"
int lzo1x_1_compress(const unsigned char *src, size_t src_len,
       unsigned char *dst, size_t *dst_len, void *wrkmem);


int lzo1x_decompress_safe(const unsigned char *src, size_t src_len,
     unsigned char *dst, size_t *dst_len);
# 20 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c" 2
# 1 "arch/mips/boot/compressed/../../../../lib/lzo/lzodefs.h" 1
# 21 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c" 2
# 38 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c"
int lzo1x_decompress_safe(const unsigned char *in, size_t in_len,
     unsigned char *out, size_t *out_len)
{
 unsigned char *op;
 const unsigned char *ip;
 size_t t, next;
 size_t state = 0;
 const unsigned char *m_pos;
 const unsigned char * const ip_end = in + in_len;
 unsigned char * const op_end = out + *out_len;

 op = out;
 ip = in;

 if (__builtin_expect(!!(in_len < 3), 0))
  goto input_overrun;
 if (*ip > 17) {
  t = *ip++ - 17;
  if (t < 4) {
   next = t;
   goto match_next;
  }
  goto copy_literal_run;
 }

 for (;;) {
  t = *ip++;
  if (t < 16) {
   if (__builtin_expect(!!(state == 0), 1)) {
    if (__builtin_expect(!!(t == 0), 0)) {
     size_t offset;
     const unsigned char *ip_last = ip;

     while (__builtin_expect(!!(*ip == 0), 0)) {
      ip++;
      if (!((size_t)(ip_end - ip) >= (size_t)(1))) goto input_overrun;
     }
     offset = ip - ip_last;
     if (__builtin_expect(!!(offset > ((((size_t)~0) / 255) - 2)), 0))
      return (-1);

     offset = (offset << 8) - offset;
     t += offset + 15 + *ip++;
    }
    t += 3;
copy_literal_run:
# 100 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c"
    {
     if (!((size_t)(op_end - op) >= (size_t)(t))) goto output_overrun;
     if (!((size_t)(ip_end - ip) >= (size_t)(t + 3))) goto input_overrun;
     do {
      *op++ = *ip++;
     } while (--t > 0);
    }
    state = 4;
    continue;
   } else if (state != 4) {
    next = t & 3;
    m_pos = op - 1;
    m_pos -= t >> 2;
    m_pos -= *ip++ << 2;
    if ((m_pos) < out) goto lookbehind_overrun;
    if (!((size_t)(op_end - op) >= (size_t)(2))) goto output_overrun;
    op[0] = m_pos[0];
    op[1] = m_pos[1];
    op += 2;
    goto match_next;
   } else {
    next = t & 3;
    m_pos = op - (1 + 0x0800);
    m_pos -= t >> 2;
    m_pos -= *ip++ << 2;
    t = 3;
   }
  } else if (t >= 64) {
   next = t & 3;
   m_pos = op - 1;
   m_pos -= (t >> 2) & 7;
   m_pos -= *ip++ << 3;
   t = (t >> 5) - 1 + (3 - 1);
  } else if (t >= 32) {
   t = (t & 31) + (3 - 1);
   if (__builtin_expect(!!(t == 2), 0)) {
    size_t offset;
    const unsigned char *ip_last = ip;

    while (__builtin_expect(!!(*ip == 0), 0)) {
     ip++;
     if (!((size_t)(ip_end - ip) >= (size_t)(1))) goto input_overrun;
    }
    offset = ip - ip_last;
    if (__builtin_expect(!!(offset > ((((size_t)~0) / 255) - 2)), 0))
     return (-1);

    offset = (offset << 8) - offset;
    t += offset + 31 + *ip++;
    if (!((size_t)(ip_end - ip) >= (size_t)(2))) goto input_overrun;
   }
   m_pos = op - 1;
   next = get_unaligned_le16(ip);
   ip += 2;
   m_pos -= next >> 2;
   next &= 3;
  } else {
   m_pos = op;
   m_pos -= (t & 8) << 11;
   t = (t & 7) + (3 - 1);
   if (__builtin_expect(!!(t == 2), 0)) {
    size_t offset;
    const unsigned char *ip_last = ip;

    while (__builtin_expect(!!(*ip == 0), 0)) {
     ip++;
     if (!((size_t)(ip_end - ip) >= (size_t)(1))) goto input_overrun;
    }
    offset = ip - ip_last;
    if (__builtin_expect(!!(offset > ((((size_t)~0) / 255) - 2)), 0))
     return (-1);

    offset = (offset << 8) - offset;
    t += offset + 7 + *ip++;
    if (!((size_t)(ip_end - ip) >= (size_t)(2))) goto input_overrun;
   }
   next = get_unaligned_le16(ip);
   ip += 2;
   m_pos -= next >> 2;
   next &= 3;
   if (m_pos == op)
    goto eof_found;
   m_pos -= 0x4000;
  }
  if ((m_pos) < out) goto lookbehind_overrun;
# 213 "arch/mips/boot/compressed/../../../../lib/lzo/lzo1x_decompress_safe.c"
  {
   unsigned char *oe = op + t;
   if (!((size_t)(op_end - op) >= (size_t)(t))) goto output_overrun;
   op[0] = m_pos[0];
   op[1] = m_pos[1];
   op += 2;
   m_pos += 2;
   do {
    *op++ = *m_pos++;
   } while (op < oe);
  }
match_next:
  state = next;
  t = next;







  {
   if (!((size_t)(ip_end - ip) >= (size_t)(t + 3))) goto input_overrun;
   if (!((size_t)(op_end - op) >= (size_t)(t))) goto output_overrun;
   while (t > 0) {
    *op++ = *ip++;
    t--;
   }
  }
 }

eof_found:
 *out_len = op - out;
 return (t != 3 ? (-1) :
  ip == ip_end ? 0 :
  ip < ip_end ? (-8) : (-4));

input_overrun:
 *out_len = op - out;
 return (-4);

output_overrun:
 *out_len = op - out;
 return (-5);

lookbehind_overrun:
 *out_len = op - out;
 return (-6);
}
# 36 "arch/mips/boot/compressed/../../../../lib/decompress_unlzo.c" 2






# 1 "include/linux/decompress/mm.h" 1
# 30 "include/linux/decompress/mm.h"
static unsigned long malloc_ptr;
static int malloc_count;

static void *malloc(int size)
{
 void *p;

 if (size < 0)
  return ((void *)0);
 if (!malloc_ptr)
  malloc_ptr = free_mem_ptr;

 malloc_ptr = (malloc_ptr + 3) & ~3;

 p = (void *)malloc_ptr;
 malloc_ptr += size;

 if (free_mem_end_ptr && malloc_ptr >= free_mem_end_ptr)
  return ((void *)0);

 malloc_count++;
 return p;
}

static void free(void *where)
{
 malloc_count--;
 if (!malloc_count)
  malloc_ptr = free_mem_ptr;
}
# 43 "arch/mips/boot/compressed/../../../../lib/decompress_unlzo.c" 2




static const unsigned char lzop_magic[] = {
 0x89, 0x4c, 0x5a, 0x4f, 0x00, 0x0d, 0x0a, 0x1a, 0x0a };






static inline __attribute__((always_inline)) __attribute__((no_instrument_function)) long parse_header(u8 *input, long *skip, long in_len)
{
 int l;
 u8 *parse = input;
 u8 *end = input + in_len;
 //u8 level = 0;
 u16 version;






 if (in_len < (9 + 7 + 4 + 8 + 1 + 4))
  return 0;


 for (l = 0; l < 9; l++) {
  if (*parse++ != lzop_magic[l])
   return 0;
 }



 version = get_unaligned_be16(parse);
 parse += 7;
 //if (version >= 0x0940)
  parse++;
 if (get_unaligned_be32(parse) & 0x00000800L)
  parse += 8;
 else
  parse += 4;







 if (end - parse < 8 + 1 + 4)
  return 0;


 parse += 8;
 if (version >= 0x0940)
  parse += 4;

 l = *parse++;

 if (end - parse < l + 4)
  return 0;
 parse += l + 4;

 *skip = parse - input;
 return 1;
}

static int unlzo(u8 *input, long in_len,
    long (*fill)(void *, unsigned long),
    long (*flush)(void *, unsigned long),
    u8 *output, long *posp,
    void (*error) (char *x))
{
 u8 r = 0;
 long skip = 0;
 u32 src_len, dst_len;
 size_t tmp;
 u8 *in_buf, *in_buf_save, *out_buf;
 int ret = -1;

 if (output) {
  out_buf = output;
 } else if (!flush) {
  if (error)
   error("NULL output pointer and no flush function provided");
  goto exit;
 } else {
  out_buf = malloc((256*1024l));
  if (!out_buf) {
   if (error)
    error("Could not allocate output buffer");
   goto exit;
  }
 }

 if (input && fill) {
  if (error)
   error("Both input pointer and fill function provided, don't know what to do");
  goto exit_1;
 } else if (input) {
  in_buf = input;
 } else if (!fill) {
  if (error)
   error("NULL input pointer and missing fill function");
  goto exit_1;
 } else {
  in_buf = malloc((((256*1024l)) + (((256*1024l)) / 16) + 64 + 3));
  if (!in_buf) {
   if (error)
    error("Could not allocate input buffer");
   goto exit_1;
  }
 }
 in_buf_save = in_buf;

 if (posp)
  *posp = 0;

 if (fill) {






  in_buf += (9 + 7 + 1 + 8 + 8 + 4 + 1 + 255 + 4);
  in_len = fill(in_buf, (9 + 7 + 1 + 8 + 8 + 4 + 1 + 255 + 4));
 }

 if (!parse_header(in_buf, &skip, in_len)) {
  if (error)
   error("invalid header");
  goto exit_2;
 }
 in_buf += skip;
 in_len -= skip;

 if (fill) {

  ({ size_t __len = (in_len); void *__ret; if (__builtin_constant_p(in_len) && __len >= 64) __ret = memcpy((in_buf_save), (in_buf), __len); else __ret = __builtin_memcpy((in_buf_save), (in_buf), __len); __ret; });
  in_buf = in_buf_save;
 }

 if (posp)
  *posp = skip;

 for (;;) {

  if (fill && in_len < 4) {
   skip = fill(in_buf + in_len, 4 - in_len);
   if (skip > 0)
    in_len += skip;
  }
  if (in_len < 4) {
   if (error)
    error("file corrupted");
   goto exit_2;
  }
  dst_len = get_unaligned_be32(in_buf);
  in_buf += 4;
  in_len -= 4;


  if (dst_len == 0) {
   if (posp)
    *posp += 4;
   break;
  }

  if (dst_len > (256*1024l)) {
   if (error)
    error("dest len longer than block size");
   goto exit_2;
  }


  if (fill && in_len < 8) {
   skip = fill(in_buf + in_len, 8 - in_len);
   if (skip > 0)
    in_len += skip;
  }
  if (in_len < 8) {
   if (error)
    error("file corrupted");
   goto exit_2;
  }
  src_len = get_unaligned_be32(in_buf);
  in_buf += 8;
  in_len -= 8;

  if (src_len <= 0 || src_len > dst_len) {
   if (error)
    error("file corrupted");
   goto exit_2;
  }


  if (fill && in_len < src_len) {
   skip = fill(in_buf + in_len, src_len - in_len);
   if (skip > 0)
    in_len += skip;
  }
  if (in_len < src_len) {
   if (error)
    error("file corrupted");
   goto exit_2;
  }
  tmp = dst_len;




  if (__builtin_expect(!!(dst_len == src_len), 0))
   ({ size_t __len = (src_len); void *__ret; if (__builtin_constant_p(src_len) && __len >= 64) __ret = memcpy((out_buf), (in_buf), __len); else __ret = __builtin_memcpy((out_buf), (in_buf), __len); __ret; });
  else {
   r = lzo1x_decompress_safe((u8 *) in_buf, src_len,
      out_buf, &tmp);

   if (r != 0 || dst_len != tmp) {
    if (error)
     error("Compressed data violation");
    goto exit_2;
   }
  }

  if (flush && flush(out_buf, dst_len) != dst_len)
   goto exit_2;
  if (output)
   out_buf += dst_len;
  if (posp)
   *posp += src_len + 12;

  in_buf += src_len;
  in_len -= src_len;
  if (fill) {





   if (in_len > 0)
    for (skip = 0; skip < in_len; ++skip)
     in_buf_save[skip] = in_buf[skip];
   in_buf = in_buf_save;
  }
 }

 ret = 0;
exit_2:
 if (!input)
  free(in_buf_save);
exit_1:
 if (!output)
  free(out_buf);
exit:
 return ret;
}
