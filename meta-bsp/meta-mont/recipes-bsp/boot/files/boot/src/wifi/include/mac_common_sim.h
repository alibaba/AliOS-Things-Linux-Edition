#ifndef __MAC_COMMON_H__
#define __MAC_COMMON_H__

#if !defined(__KERNEL__)
#include "mac_sim.h"
//#include "mac_sim_config.h"
#else
#include <asm/types.h>
#include "mac_options.h"
#include "mac_config.h"
#endif
#include <mt_types.h>
/* HW characterics; DO NOT CHANGE IT! */
#define MAC_TX_STATUS_TBL_COUNT     17          /* total elements of HT mode TX status tables */
#define MAC_RX_STATUS_TBL_COUNT     17          /* total elements of HT mode RX status tables */

#define MAC_TOTAL_TID_COUNT         9           /* 8 tid + 1 legacy */

#define MAC_TX_TEMP_DRAM_BUFFER_SIZE            ( 64 * 8 )

#define MAC_HT_TX_RETRY_BUFFER_STRUCT_SIZE      ( 64 * 16 )
#define MAC_HT_RX_BUFFER_STRUCT_SIZE            ( 8 )   /* times the MPDU_window ( e.g. 8 x 64 ) */
#define MAC_LEGACY_RX_BUFFER_STRUCT_SIZE        ( 8 )   /* times the MPDU_window ( e.g. 8 x 64 ) */

#define MAC_AMPDU_RX_REORDER_BUFFER_SIZE        ( 128 * 16 )

int reset_mac_registers(void);
void bb_init(void);


#define BY_ADDR_IDX 0x1
#define IN_DS_TBL   0x2
int mac_addr_lookup_engine_find(u8 *addr, int addr_index, u32 *basic_cap, char flag);
int mac_addr_lookup_engine_update(u8 *addr, u32 basic_cap, u8 flag);

#define BASIC_CAP_WEP_DEF_KEY 0x10000000UL
#define BASIC_CAP_CIPHER      0x20000000UL
int mac_basic_cap_update(int sta_id, u32 new_val, u32 mask, char flag);

#if defined(LYNX)
#define IN_QRAM  __attribute__((section(".sim_qram")))
#define IN_SRAM  __attribute__((section(".sim_sram")))
#elif defined(REAL_BB)
#define IN_QRAM
#define IN_SRAM
#else
#define IN_QRAM  __attribute__((section(".phyram")))
#define IN_SRAM  __attribute__((section(".phyram")))
//#define IN_QRAM  __attribute__((section(".sram")))
//#define IN_SRAM  __attribute__((section(".sram")))
#endif

int apply_network_configuration(void);
void verification_main(void);

#endif // __MAC_COMMON_H__

