/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file panther_debug.h
*   \brief  debug function.
*   \author Montage
*/

#ifndef _PANTHER_DEBUG_H_
#define _PANTHER_DEBUG_H_

#include <os_compat.h>

#ifdef __KERNEL__
#include <linux/linkage.h>
#include <linux/version.h>
#include <linux/printk.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#include <linux/kern_levels.h>
#endif
#endif

#include "lib_autoconf.h"

#define DRAGONITE_RFC_DEBUG 1

#ifdef __KERNEL__

#define DRAGONITE_DEBUG_TX_FLAG                0x00000001
#define DRAGONITE_DEBUG_TX_RATE_FLAG           0x00000002
#define DRAGONITE_DEBUG_RX_FLAG                0x00000100
#define DRAGONITE_DEBUG_PS_FLAG                0x00010000
#define DRAGONITE_DEBUG_ENCRPT_FLAG            0x00020000
#define DRAGONITE_DEBUG_NETWORK_INFO_FLAG      0x10000000
#define DRAGONITE_DEBUG_DRIVER_WARN_FLAG       0x20000000
#define DRAGONITE_DEBUG_SYSTEM_FLAG            0x80000000

extern u32 dragonite_debug_flag;

#define TSF_WARN(_vp, _fmt, ...) do { \
        if(!_vp->ts_err_mute) \
                    printk(KERN_WARNING "DRAGONITE: (%s:%d): " _fmt, \
                        __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define DRG_DBG(_fmt, ...) do { \
                    printk(KERN_DEBUG "DRAGONITE: (%s:%d): " _fmt, \
                        __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define DRG_NOTICE_DUMP(_flag, _fmt, _buf, _len) do { \
        if(_flag & dragonite_debug_flag) \
                    { \
                        printk(KERN_NOTICE "DRAGONITE: (%s:%d): ", \
                            __func__, __LINE__); \
                        print_hex_dump(KERN_NOTICE, _fmt, \
                            DUMP_PREFIX_NONE, 16, 1, _buf, _len, true); \
                    } \
        } while (0)

#define DRG_NOTICE(_flag, _fmt, ...) do { \
        if(_flag & dragonite_debug_flag) \
                    printk(KERN_NOTICE "DRAGONITE: (%s:%d): " _fmt, \
                        __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define DRG_WARN(_flag, _fmt, ...) do { \
        if(_flag & dragonite_debug_flag) \
                    printk(KERN_WARNING "DRAGONITE: (%s:%d): " _fmt, \
                        __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)

#define DRG_ERROR(_flag, _fmt, ...) do { \
        if(_flag & dragonite_debug_flag) \
                    printk(KERN_ERR "DRAGONITE: (%s:%d): " _fmt, \
                        __func__, __LINE__, ##__VA_ARGS__); \
        } while (0)


#endif //__KERNEL__



//#define DBG(...)                                printk(__VA_ARGS__)
#define DBG_PRINTF(_lv, ...) \
	do {                           \
			printk( __VA_ARGS__);     \
	} while(0)

#ifdef DRAGONITE_RFC_DEBUG
#define serial_printf(x, args...) \
        printk(x, ##args);
#else
#define serial_printf(x, args...);
#endif

#define WLAN_DBG        serial_printf  

#ifdef CONFIG_WLA_DEBUG_MSG
#define WLA_DBG(_mode, ...)     DBG(__VA_ARGS__)
#else
#define WLA_DBG(_l, _str...)
#endif


#endif //_PANTHER_DEBUG_H_

