#ifndef __OS_COMPAT_H__
#define __OS_COMPAT_H__

#ifdef __KERNEL__
#include <asm/delay.h>
#include <asm/types.h>
#include <linux/string.h>
#include <linux/bitops.h>
#else   // ecos use now
#include <mt_types.h>
#include <clock.h>
//#include <commands.h>
//#include <string.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stddef.h>

#define CONFIG_PANTHER_CHIP_VERSION 1
#endif
//#include <linux/kernel.h>
//#include <asm/mach-panther/common.h>

//#include <asm/mach-panther/panther.h>
//#include <asm/mach-panther/gpio.h>
//#include <string.h>
//#include <stdlib.h>

#ifndef NULL
#define NULL                                    0
#endif


#ifndef BIT
#define BIT(s) (1 << (s))
#endif

#ifdef __KERNEL__

#ifndef printf
#define printf(...)                                printk(KERN_DEBUG __VA_ARGS__)
#endif

#else

#ifndef printk
#define printk printf
#define simple_strtol strtol
#endif

#endif

#define DMA_MALLOC_BZERO            MAC_MALLOC_BZERO
//os time
#define WLA_CURRENT_TIME            wla_current_time()

#define WLA_MFREE(a)                mac_free(a)
#define WLA_MALLOC(size, flags)     mac_malloc(NULL, size, flags)

extern unsigned int wla_current_time(void);

#endif
