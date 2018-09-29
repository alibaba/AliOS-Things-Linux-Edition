/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file common.h
*   \brief Common API
*   \author Montage
*/

#ifndef COMMON_H
#define COMMON_H

enum
{
    ERR_OK = 0,
    ERR_HELP = 1,
    ERR_PARM = -1,
    ERR_ALIGN = -2,
    ERR_ADDR = -3,
    ERR_FILE = -4,
    ERR_TIMEOUT = -5,
    ERR_ETHER = -6,
    ERR_MEM = -7,
    ERR_GO_RECOVERY = -8,
    ERR_GO_SECOND  = -9,
    ERR_LAST = -10,
};

enum
{
    BOOT_MODE_CMD = 0,
    BOOT_MODE_FLASH = 1,
    BOOT_MODE_TFTP = 2,
    BOOT_MODE_BOOT3 = 3,
    BOOT_MODE_BUF = 4,
    BOOT_MODE_RECOVERY = 5,
    BOOT_MODE_MPTOOL = 6,
    BOOT_MODE_SECOND = 7,
    MAX_BOOT_MODE,
};

enum
{
    RECOVERY_NONE = 0,
    RECOVERY_ONE = 1,
    RECOVERY_TWO = 2,
};

enum
{
    RECOVER_SUBMODE = 1,
    MPTOOL_SUBMODE = 2,
    END_SUBMODE = -1,
};

#define VECTOR_JUMP_SIZE 0x10
#define ALIGN_TO(a,b) (((unsigned int)a + (b-1)) & ~(b-1))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))
#define ALIGN(x,a)      __ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ARCH_DMA_MINALIGN   HAL_DCACHE_LINE_SIZE
typedef unsigned long int uintptr_t;

#define PAD_COUNT(s, pad) (((s) - 1) / (pad) + 1)
#define PAD_SIZE(s, pad) (PAD_COUNT(s, pad) * pad)
#define ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, pad)		\
	char __##name[ROUND(PAD_SIZE((size) * sizeof(type), pad), align)  \
		      + (align - 1)];					\
									\
	type *name = (type *) ALIGN((uintptr_t)__##name, align)
#define ALLOC_ALIGN_BUFFER(type, name, size, align)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, align, 1)
#define ALLOC_CACHE_ALIGN_BUFFER_PAD(type, name, size, pad)		\
	ALLOC_ALIGN_BUFFER_PAD(type, name, size, ARCH_DMA_MINALIGN, pad)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)			\
	ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

void clock_init();
unsigned int clock_get(void);
void mdelay(unsigned int time);
void udelay(unsigned int time);
unsigned int how_long(unsigned int from);
extern int get_args(const char *string, char *argv[]);
extern char _ftext[], _fbss[], _end[];
extern int (*io_redirect) (char *);
extern int (*telnetd_input) (unsigned char *);
extern int (*telnetd_output) (char *);
extern int (*telnetd_poll) (void);
extern unsigned long byte_count;
extern unsigned long buf_address;
void dcache_flush(void);
void icache_inv(void);
void dcache_flush_range(void *addr, int len);
void dcache_inv_range(void *addr, int len);
extern int flash_init(int boot_type);
int cmd_proc(int argc, char **argv);
int otp_parse_config(int shift_val);
extern int xmodem_rx(unsigned char *dest, int destsz);

void pmu_reset_wifi_mac(void);
void dratini_start(void);

extern int chip_revision;
void chip_revision_detection(void);
void reset_devices(void);

#define printk printf
int printf(char *fmt, ...);
int printf_no_redirect(char *fmt, ...);
#if !defined(BOOT_MODE_IPL) && !defined(BOOT_MODE_BOOT1) && !defined(DEBRICK)
extern int dbg_log_level;
#define LOG_FATAL    (1)
#define LOG_ERR      (2)
#define LOG_WARN     (3)
#define LOG_INFO     (4)
#define LOG_DBG      (5)
#define LOG_VERBOSE  (6)
#ifdef VERBOSE_DEBUG
#define dbg_log(level, fmt, args...)  	printf(fmt, ##args)
#else
#define dbg_log(level, fmt, args...)	if(dbg_log_level >= level) printf(fmt, ##args)
#endif
#else
#define dbg_log(level, fmt, args...)
#endif

int check_stop_condition(void);
void idelay(unsigned int count);

#include    <cmd.h>

#define virt_to_phy(a) (((unsigned int)a)&0x1fffffff)
#define phy_to_virt(a) (((unsigned int)a)|0xa0000000)
//#define phy_to_virt(a) (((unsigned int)a)|0x80000000)
#define uncached_addr(a) (((unsigned int)a)|0xa0000000)
#define KUSEG02KSEG0(addr)   (((addr)&0x1FFFFFFF)|0x80000000)
#define ADDR_IN_ROM(a) (((unsigned int)a)&0x10000000)
#endif
