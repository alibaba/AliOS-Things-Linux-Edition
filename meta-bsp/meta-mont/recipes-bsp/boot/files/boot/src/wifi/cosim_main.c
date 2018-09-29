#define REAL_BB 1
#include <lib.h>
#include <mac.h>
#include <mac_internal.h>
#include <mac_common_sim.h>
#include <genpkts.h>
#include <mt_types.h>
#include <mac_regs.h>
#include <packet.h>
#include <buffer.h>
#include <tx.h>
#include <bb.h>
#include "panther_app.h"
#include "performance.h"
#include "rfc_comm.h"
#include "mac_ctrl.h"
#include <math.h>
#include <wtest.h>
#include <arch/chip.h>
#include <wla_def.h>
#include <netprot.h>
#include <arch/irq.h>
#if defined(CONFIG_SCHED)
#include <sched.h>
#endif

int printf(char *fmt, ...);

void beacon_kick(void);
void mlme_handle_auth(u8 *dptr, u32 len);
int mlme_handle_data_frame(u8 *dptr, u32 len);
void ap_handle_assoc(u8 *dptr, u32 len);
void init_beacon_settings(void);
void beacon_descriptor_setup(int beacon_packet_length);
void arthur_beacon_setup(unsigned short beacon_interval, unsigned char dtim_period);
void arthur_beacon_start(unsigned long beacon_bitrate);
void wmac_set_channel(u8 ch, u8 bw);
u8 bb_rssi_decode(u8 val, int rssi_offset, u8 *lna_gain_tble);
int ap_handle_probe_req(u8 *dptr, u32 len);
void udelay(unsigned int time);
void mini_mlme_init(void);
void bb_set_20mhz_mode(int freq);
int lrf_ch2freq(int ch);
void rf_set_40mhz_channel(int primary_ch_freq, int channel_type);

u8 AP_MAC_ADDR[] = { 0x78, 0x00, 0x00, 0x00, 0x00, 0x42 };
u8 MY_MAC_ADDR[] = { 0x10, 0x05, 0x06, 0x07, 0x08, 0x01 };

const unsigned char broadcast[] = {[0 ... 5] = 0xff };
const unsigned char zeros[] = {[0 ... 5] = 0 };

extern char boot3_submode;
enum
{
    RECOVER_SUBMODE = 1,
    MPTOOL_SUBMODE = 2,
    END_SUBMODE = -1,
};

u16 tx_rates[4];
#if 1
    #ifdef LYNX
        #define IN_QRAM __attribute__((section(".sim_qram")))
        #define IN_SRAM __attribute__((section(".sim_sram")))
    #elif defined(REAL_BB)
        #define IN_QRAM
        #define IN_SRAM
    #else
        #define IN_QRAM __attribute__((section(".phyram")))
        #define IN_SRAM __attribute__((section(".phyram")))
    #endif

    #if defined(__IRQ_H__)
        #define INTR_NUM_BASE	0x6
        #define WIFIMAC_INTR_NUM	(INTR_NUM_BASE + 14)

        #define MAX_INT_HANDLERS	(6 +32)
    #endif

struct ihnd
{
    void (*handler)(void);
};

    #define INJECT_PKT_LEN		0xBEB00060
    #define INJECT_PKT_TIME_L	0xBEB00064
    #define INJECT_PKT_TIME_H	0xBEB00068
    #define INJECT_PKT_DURATION	0xBEB0006C
    #define INJECT_PKT_FLAGS	0xBEB00070
    #define INJECT_PKT_DATA		0xBEC00000

    #define COSIM_REG_READ32(x)  (*(volatile u32*)((x)))
    #define COSIM_REG_WRITE32(x,val) (*(volatile u32*)((x)) = (u32)(val))
    #define COSIM_REG_UPDATE32(x,val,mask) do {           \
    u32 newval;                                        \
    newval = *(volatile u32*) ((x));     \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));\
    *(volatile u32*)((x)) = newval;      \
} while(0)

#if !defined(CONFIG_SCHED)
void sched_lock(void);
void sched_unlock(void);
#endif

    #define COSIM_LOCK()       sched_lock()
    #define COSIM_UNLOCK()     sched_unlock()

    #define COSIM_CONTROL			0xBEB00000
    #define COSIM_CONTROL_PACKET_INJECT	0x00000001
    #define COSIM_STACFG_OPERATION		0xBEB00148
    #define COSIM_STACFG_OPERATION_ADD	0x01
    #define COSIM_STACFG_OPERATION_DELETE	0x02
    #define COSIM_STACFG_OPERATION_UPDATE	0x04

    #define COSIM_STA_CONFIG		0xBEB0014C
    #define COSIM_STA_CONFIG_STA_IDX	0x0000FFFF
    #define COSIM_STA_CONFIG_BSSIDX		0x00FF0000
    #define COSIM_STA_CONFIG_DUT		0x01000000
    #define COSIM_STA_CONFIG_QOS		0x02000000
    #define COSIM_STA_CONFIG_MODE		0xF0000000
    #define COSIM_STA_CONFIG_MODE_CLIENT	0x00000000
    #define COSIM_STA_CONFIG_MODE_LEGACY_AP	0x10000000
    #define COSIM_STA_CONFIG_MODE_P2PGO	0x20000000
    #define COSIM_STA_CONFIG_MODE_IBSS	0x40000000
    #define COSIM_STA_CONFIG2		0xBEB00150
    #define COSIM_STA_CONFIG2_AMPDU_TX	0x000000FF
    #define COSIM_STA_CONFIG2_AMPDU_RX	0x00FF0000
    #define COSIM_STA_CONFIG_MAC_ADDR0	0xBEB00154
    #define COSIM_STA_CONFIG_MAC_ADDR1	0xBEB00158

    #define COSIM_CPU_YIELD			0xBEB00034
    #define COSIM_INTR_CLEAR		0xBEB00038

    #define COSIM_SET_TEST_PASSED		0xBEB00140
    #define COSIM_FINISH			0xBEB0013C
    #define COSIM_WAIT_RELATE_TIME		0xBEB00028

    #define FILL_PER_STA_TX_RATE		0x00000001UL

#endif
#define UNCACHED_VIRTUAL_ADDR(x)    ((u32) (x) | 0xA0000000L)


#define simply 1
#define BSS0_AP_NUM     128
#define BSS0_BC_STA_NUM 255
#define BSS0_DUT_STA_NUM 129
#define BSS0_STA_NUM	64 
#define BSS4_AP_NUM           65    // with RX header conversion
#define MAX_BUFFER_PER_PACKET 9

#define QOS             0x00000001
#define AP              0x00000002
#define WEP40_DEF       0x00000004
#define WEP104_DEF      0x00000008
#define WEP40           0x00000010
#define WEP104          0x00000020
#define TKIP            0x00000040
#define CCMP            0x00000080
#define WAPI            0x00000100
#define ETH             0x00000200
#define TOSW            0x00000400
#define DUT             0x00000800
#define STATION         0x00001000
#define IBSS            0x00002000
#define P2PC            0x00004000
#define P2PGO           0x00008000

typedef struct
{
    int sta_id;
    int STA_NUM;
    int BSSID;
    unsigned char MAC_ADDR[6];
    unsigned char WAN_MAC_ADDR[6];
    unsigned char BSSID_ADDR[6];
    unsigned char KEY[32];
    unsigned char LAN_IP_ADDR[4];
    unsigned char WAN_IP_ADDR[4];
    unsigned int CAP;           /* Capabilities */
    unsigned int MODE;          /* Operation mode */
    int BSS;
    int ESS;
    unsigned char AMPDU_TX;     /* 8bits bitmap for AMPDU TX sessions (per TID) */
    unsigned char AMPDU_RX;     /* 8bits bitmap for AMPDU RX sessions (per TID) */
    unsigned long RATE0;
    unsigned long RATE1;
    unsigned long RATE2;
    unsigned long RATE3;
    unsigned char MPDU_SPACING;
    unsigned char AMPDU_LEN;
    unsigned short U_APSD;
    unsigned char WEP_KEYID;
} network_config;
#define DBG printf
static network_config net_cfg[] =
{
#include <network_config>
};
#define NET_CFG_ENTRIES     (sizeof(net_cfg)/sizeof(net_cfg[0]))

#define MIN(x,y)  ((x) > (y)) ? (y) : (x)

#define ACQ_LOCK()   sched_lock() //int_disable(WIFI_MAC_INTR_NUM)
#define ACQ_UNLOCK() sched_unlock() //int_enable(WIFI_MAC_INTR_NUM)

sta_cfg __sta_cfg[MAX_STA_NUM];

#define USE_FREE_LIST
#define PKT_FREELIST_SIZE     64

#if defined(USE_FREE_LIST)
mpdu* freelist_head;
#endif
static acq def_acq[ACQ_NUM] __attribute__ ((aligned (16)));
static acq acq_pool[ACQ_POOL_SIZE] __attribute__ ((aligned (16)));

u32 beacon_tx_count = 0;

MAC_INFO mac_info;
MAC_INFO *info;

// null data
static unsigned char null_tx_packet[] = {
    0x48, 0x11, 0x00, 0x00, 0xF8, 0xD1, 0x11, 0x5B, 0x0C, 0xC4, 0x10, 0x05, 0x06, 0x07, 0x08, 0x01,
    0xF8, 0xD1, 0x11, 0x5B, 0x0C, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char test_tx_packet[] = {
    0x88, 0x02, 0x00, 0x00, 0x10, 0x05, 0x06, 0x07, 0x08, 0x01, 0x78, 0x00, 0x00, 0x00, 0x00, 0x42, 
    0x78, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 
    0x08, 0x00, 0x45, 0x61, 0x00, 0x38, 0xa9, 0x73, 0x00, 0x00, 0xdc, 0x00, 0x90, 0xc6, 0xcb, 0x93, 
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
    0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 
    0x82, 0x4a, 0xb7, 0x31, 0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59,
    0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 0xb1, 0x5e, 0xdb, 0x61, 0xcb, 0x93,
};

static unsigned char test_rx_packet[] = {0x01, 0x00, 0x5c, 0x00, 0x07, 0x5c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x08, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x78, 0x00, 0x00, 0x00, 0x00, 0x41, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 
    0x00, 0x00, 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x08, 0x00, 0x45, 0x61, 0x00, 0x38, 0xa9, 0x73, 0x00, 0x00, 0xdc, 0x00, 0x90, 0xc6, 
    0xcb, 0x93, 0x58, 0x66, 0xb9, 0x27, 0xc7, 0x09, 0xc5, 0xa3, 0x06, 0x98, 0x72, 0x81, 0xd6, 0xf5, 0x03, 0xca, 0x82, 0x4a, 0xb7, 0x31, 
    0x92, 0x5c, 0xed, 0xeb, 0x4d, 0x7f, 0x03, 0x4e, 0x98, 0xa8, 0x27, 0x59, 0x9b, 0x29, 0x3c, 0xea, 0xfe, 0xd4, 0xc7, 0xed, 0xd1, 0x3b, 
    0xcb, 0x18, 0x20, 0xe6, 0x00, 0x00, 0x00, 0x00};

typedef struct rx_phy_info {
    unsigned char format;
    unsigned char snr_p1;
    unsigned char snr_p2;
    unsigned char rssi;
    unsigned char rate;
    unsigned char bandwidth;
    unsigned char offset;
} _rx_phy_info;

#define MAX_RX_PHY_INFO_STORAGE_SIZE 10
_rx_phy_info rx_phy_info_storage[MAX_RX_PHY_INFO_STORAGE_SIZE];
int rx_phy_info_storage_idx = 0;
void store_rx_phy_info(unsigned char format, unsigned char snr_p1, unsigned char snr_p2,
        unsigned char rssi, unsigned char rate, unsigned char bandwidth, unsigned char offset)
{
    rx_phy_info_storage[rx_phy_info_storage_idx].format = format;
    rx_phy_info_storage[rx_phy_info_storage_idx].snr_p1 = snr_p1;
    rx_phy_info_storage[rx_phy_info_storage_idx].snr_p2 = snr_p2;
    rx_phy_info_storage[rx_phy_info_storage_idx].rssi = rssi;
    rx_phy_info_storage[rx_phy_info_storage_idx].rate = rate;
    rx_phy_info_storage[rx_phy_info_storage_idx].bandwidth = bandwidth;
    rx_phy_info_storage[rx_phy_info_storage_idx].offset = offset;
    rx_phy_info_storage_idx++;
    rx_phy_info_storage_idx = rx_phy_info_storage_idx % MAX_RX_PHY_INFO_STORAGE_SIZE;
}

#define CCK_1M                 0x0
#define CCK_2M                 0x1
#define CCK_5_5M               0x2
#define CCK_11M                0x3
#define CCK_SHORT_PREAMBLE     0x04

/* channel offset */
#define CH_OFFSET_20		0
#define CH_OFFSET_40		1
#define CH_OFFSET_20U		2
#define CH_OFFSET_20L		3

void dump_rx_rate_simply(unsigned char format, unsigned char rate, unsigned char bandwidth, unsigned char offset)
{
    if(bandwidth == 0)
        printk("20");
    else
        printk("40");

    if(offset == CH_OFFSET_20)
        printk("  MHz");
    else if(offset == CH_OFFSET_20U)
        printk("+ MHz");
    else if(offset == CH_OFFSET_20L)
        printk("- MHz");
    else
        printk("  MHz");

    if(format == FMT_NONE_HT)
        printk("  G   mode");
    else if(format == FMT_HT_MIXED)
        printk(" HT  mixed");
    else if(format == FMT_HT_GF)
        printk(" HT gfield");
    else if(format == FMT_11B)
        printk("  B   mode");

    if(format == FMT_11B)
    {
        if(rate == CCK_1M)
            printk(" 1M\n");
        else if(rate == CCK_2M)
            printk(" 2M\n");
        else if(rate == CCK_5_5M)
            printk(" 5.5M\n");
        else if(rate == CCK_11M)
            printk(" 11M\n");
        else if(rate == CCK_SHORT_PREAMBLE)
            printk(" short preamble\n");
    }

    if(format == FMT_NONE_HT)
    {
        if(rate == OFDM_6M)
            printk(" 6M\n");
        else if(rate == OFDM_9M)
            printk(" 9M\n");
        else if(rate == OFDM_12M)
            printk(" 12M\n");
        else if(rate == OFDM_18M)
            printk(" 18M\n");
        else if(rate == OFDM_24M)
            printk(" 24M\n");
        else if(rate == OFDM_36M)
            printk(" 36M\n");
        else if(rate == OFDM_48M)
            printk(" 48M\n");
        else if(rate == OFDM_54M)
            printk(" 54M\n");
    }

    if(format == FMT_HT_MIXED || format == FMT_HT_GF)
    {
        if(rate == MCS_0)
            printk(" 6.5M\n");
        else if(rate == MCS_1)
            printk(" 13M\n");
        else if(rate == MCS_2)
            printk(" 19.5M\n");
        else if(rate == MCS_3)
            printk(" 26M\n");
        else if(rate == MCS_4)
            printk(" 39M\n");
        else if(rate == MCS_5)
            printk(" 52M\n");
        else if(rate == MCS_6)
            printk(" 58.5M\n");
        else if(rate == MCS_7)
            printk(" 65M\n");
    }
}
static unsigned char primary_ch_offset(u8 bw_type)
{
	if (bw_type == BW40MHZ_SCB)
		//return CH_OFFSET_20U;
		return CH_OFFSET_40;
	else if(bw_type == BW40MHZ_SCA)
		//return CH_OFFSET_20L;
		return CH_OFFSET_40;
	else
		return CH_OFFSET_20;
}
void dump_rx_phy_info(void)
{
    int idx;
    double snr_x, snr_y, snr_db;

    for(idx = 0; idx < rx_phy_info_storage_idx; idx++)
    {
        snr_x = (double)rx_phy_info_storage[idx].snr_p1 / 1024;
        snr_y = ((double)(((rx_phy_info_storage[idx].snr_p2 >> 5) & 0x07) + 8)/64) * pow(2, -1*(rx_phy_info_storage[idx].snr_p2 & 0x1F));
        snr_db = 10*log10(0.5 * (snr_x/snr_y - 1));

        dbg_double(RFC_DBG_TRUE, snr_db); printf(" / ");
        printf("%d / ", rx_phy_info_storage[idx].rssi);
        dump_rx_rate_simply(rx_phy_info_storage[idx].format, rx_phy_info_storage[idx].rate, rx_phy_info_storage[idx].bandwidth, rx_phy_info_storage[idx].offset);
    }
    /* init storage index */
    rx_phy_info_storage_idx = 0;
}

void cosim_stop(void)
{
#if defined(REAL_BB)
    return;
#endif

    COSIM_REG_WRITE32(COSIM_FINISH, 1);
}
void cosim_panic(char *message)
{
    printf("COSIM_PANIC: %s\n", message);

    cosim_stop();
}
void cosim_delay_ns(unsigned long delay_ns)
{
    //int i;
#if defined(REAL_BB)
    return;
#endif

    COSIM_REG_WRITE32(COSIM_WAIT_RELATE_TIME, delay_ns);

#if 0
    for (i=0;i<CPU_INST_EXEC_PER_NS / 2;i++)
    {
        __asm__ __volatile__ ("b.nop");
    }   
#endif
}
static void pkt_init(mpdu *pkt)
{
    pkt->sta_id = -1;     /* internal id, sequentially assigned from 0 */
    pkt->sta_num = -1;    /* mnemonic ID, the number in network_config */
    pkt->bss_idx = -1;
    pkt->domain = DOMAIN_WIFI;
    pkt->ethernet_format = 0;
    pkt->header_length = 0;
    pkt->rate = -1;
    pkt->sgi = 0;
    pkt->ch_offset = CH_OFFSET_20;
    pkt->plaintext_length = 0;
    pkt->encrypted_length = 0;
    pkt->length = 0;
    pkt->data = pkt->plaintext_data;
    pkt->crc_ok = 0;
    pkt->ipcsok = 0;
    pkt->tcpcsok = 0;
    pkt->from_ds = -1;
    pkt->to_ds = -1;
    pkt->ta_idx = -1;
    pkt->ra_idx = -1;
    pkt->retry = 0;
    pkt->more_frag = 0;
    pkt->frag_num = 0;
    pkt->flags = (PKT_FLAG_NEED_RESPONSE | PKT_FLAG_NORESP_RETRY | PKT_FLAG_TOGGLE_RETRY_BIT);
    pkt->matched_cb = NULL;
    pkt->in_aggregation = 0;
    pkt->max_ampdu_len = 0;
    pkt->ampdu_len = 0;
    pkt->descr_valid = 0;
    pkt->ignore = 0;
    pkt->packet_start = 0;
    pkt->broadcast = 0;
    pkt->multicast = 0;
    pkt->total_fragments = 0;
    pkt->more_data = 0;
    pkt->eosp = 0;
    pkt->amsdu = 0;
    pkt->ps_returned = 0;
    pkt->noa_returned = 0;
    pkt->retry_failed = 0;

    pkt->type = 0;
    pkt->subtype = 0;
    pkt->seq_num = 0;

    pkt->prev = NULL;
    pkt->next = NULL;
}
#define ISR_ONLY

#if !defined(CONFIG_SCHED)
void sched_lock(void)
{
//#if defined(ISR_ONLY)
//    cyg_interrupt_disable();
//#else
//    cyg_scheduler_lock();
//#endif
}

void sched_unlock(void)
{
//#if defined(ISR_ONLY)
//   cyg_interrupt_enable();
//#else
//   cyg_scheduler_unlock();
//#endif
}
#endif
void packet_free(mpdu *pkt)
{
    mpdu *pkt_next;

    PKT_LOCK();

#if defined(USE_FREE_LIST)
    if (pkt==NULL)
    {
        PKT_UNLOCK();
        return;
    }

    pkt_next = pkt;
    while (pkt_next->next)
    {
        pkt_next = pkt_next->next;
    }

    if (freelist_head)
        pkt_next->next = freelist_head;
    freelist_head = pkt;
#else
    while (pkt)
    {
        pkt_next = pkt->next;
        free(pkt);
        pkt = pkt_next;
    }
#endif

    PKT_UNLOCK();
}

mpdu m_pkt_list[PKT_FREELIST_SIZE];
void packet_queue_init(void)
{
    int i;
#if defined(USE_FREE_LIST)
    mpdu *pkt;
#endif

    PKT_LOCK();
#if defined(USE_FREE_LIST)
    for (i=0;i<PKT_FREELIST_SIZE;i++)
    {
        pkt = &m_pkt_list[i];//malloc(sizeof(mpdu));

        if (pkt==NULL)
        {
            cosim_panic("1packet_alloc() failed, out-of-memory");
        }

        pkt->next = NULL;

        if (freelist_head)
            pkt->next = freelist_head;
        freelist_head = pkt;
    }
#endif

    PKT_UNLOCK();
}

#define MAX_MPDU_BUF_SIZE   RX_DESCRIPTOR_COUNT
#define NEXT_PKT_IDX(_idx)  (_idx++ % MAX_MPDU_BUF_SIZE)
mpdu m_pkts[MAX_MPDU_BUF_SIZE];
int m_pkt_idx = 0;
mpdu *packet_alloc(void)
{
    mpdu *pkt;

#if defined(USE_FREE_LIST)
    PKT_LOCK();

    pkt = freelist_head;
    if (pkt)
    {
        freelist_head = pkt->next;
    }

    PKT_UNLOCK();
#else
    PKT_LOCK();

    pkt = &m_pkts[NEXT_PKT_IDX(m_pkt_idx)];

    PKT_UNLOCK();
#endif

    if (pkt==NULL)
    {
        cosim_panic("2packet_alloc() failed, out-of-memory");
    }

    if (pkt)
    {
        pkt_init(pkt);
    }

    return pkt;
}

static int tx_alloc_count;
buf_header* bhdr_get_first(MAC_INFO *info)
{
    volatile buf_header *bhdr;

    BHDR_LOCK();

    if (info->sw_tx_bhdr_head >= 0)
    {
        bhdr = idx_to_bhdr(info, info->sw_tx_bhdr_head);
        if (info->sw_tx_bhdr_head == info->sw_tx_bhdr_tail)
        {
            info->sw_tx_bhdr_head = -1;   /* the freelist is now empty */
            info->sw_tx_bhdr_tail = -1;
        }
        else
        {
            info->sw_tx_bhdr_head = bhdr->next_index;
        }

        bhdr->ep = 0;
        bhdr->next_index = 0;

        tx_alloc_count++;

        BHDR_UNLOCK();

        return(buf_header *)UNCACHED_ADDR(bhdr);
    }
    else
    {
        BHDR_UNLOCK();
        return NULL;
    }
}

void bhdr_insert_tail(MAC_INFO *info, int head, int tail)
{
    volatile buf_header *bhdr;
    int idx;

    BHDR_LOCK();

    if (info->sw_tx_bhdr_tail >= 0)
    {
        bhdr = idx_to_bhdr(info, info->sw_tx_bhdr_tail);
        bhdr->next_index = head;
        bhdr->ep = 0;
    }
    else
    {
        info->sw_tx_bhdr_head = head;
    }

    for (idx = head;; idx = bhdr->next_index)
    {
        bhdr = &info->buf_headers[idx];
        //bhdr->dptr = 0;

        tx_alloc_count--;

        if ((idx == tail) || bhdr->ep)
            break;
        bhdr->ep = 0;
    }

    bhdr->ep = 1;
    info->sw_tx_bhdr_tail = bhdr_to_idx(info, (buf_header *)bhdr);

    BHDR_UNLOCK();
}

int mpdu_copy_to_buffer(mpdu *pkt, int use_eth_hdr, int *bhr_h, int *bhr_t)
{
    buf_header *bhdr, *bhdr_prev;
    u8 *dptr;
    int bhdr_idx[MAX_BUFFER_PER_PACKET];
    int i;
    int cur_offset, copy_length;
    int alloc_failure;
    int buf_count;

    bhdr_prev = NULL;

    /* allocate buffers */
    buf_count = ((pkt->length + TXDESCR_SIZE + (DEF_BUF_SIZE - 1)) / DEF_BUF_SIZE);
    if (buf_count > MAX_BUFFER_PER_PACKET)
    {
        cosim_panic("Unexpected packet length");
    }

    for (i=0;i<buf_count;i++)
        bhdr_idx[i] = -1;

    alloc_failure = 0;
    for (i=0;i<buf_count;i++)
    {
        bhdr = bhdr_get_first(info);

        if (bhdr)
        {
            bhdr_idx[i] = bhdr_to_idx(info, bhdr);
        }
        else
        {
            alloc_failure = 1;
            break;
        }
    }

    if (alloc_failure)
    {
        for (i=0;i<buf_count;i++)
        {
            if (bhdr_idx[i] != -1)
                bhdr_insert_tail(info, bhdr_idx[i], bhdr_idx[i]);
        }

        return -1;
    }

    // fill TX data
    cur_offset = 0;
    for (i=0;i<buf_count;i++)
    {
        bhdr = idx_to_bhdr(info, bhdr_idx[i]);

        dptr = (u8 *) UNCACHED_VIRTUAL_ADDR(bhdr->dptr);

        if (i==0)
        {
            bhdr->offset = TXDESCR_SIZE;
#if !defined(NEW_BUFH)
            bhdr->offset_h = 0;
#endif
            dptr += TXDESCR_SIZE;

            copy_length = MIN(pkt->length - cur_offset, DEF_BUF_SIZE - TXDESCR_SIZE);
            memcpy((void *)dptr, &pkt->data[cur_offset], copy_length);
            bhdr->len = copy_length;
            cur_offset += copy_length;

            *bhr_h = bhdr_idx[i];
        }
        else
        {
            bhdr_prev->next_index = bhdr_idx[i];
            bhdr->offset = 0;
#if !defined(NEW_BUFH)
            bhdr->offset_h = 0;
#endif

            copy_length = MIN(pkt->length - cur_offset, DEF_BUF_SIZE);
            memcpy((void *)dptr, &pkt->data[cur_offset], copy_length);
            bhdr->len = copy_length;
            cur_offset += copy_length;
        }

        if (i==(buf_count-1))
        {
            *bhr_t = bhdr_idx[i];
            bhdr->ep = 1;
        }

        bhdr_prev = bhdr;
    }

    return 0;
}

sta_cfg* load_station_config(void)
{
    return __sta_cfg;
}

u16 default_tx_rates[4];
static int default_tx_rates_init = 0;
u64 tsc = 0xFFFFFFFFFFFFFF01ULL;
u64 tsc_hi = 0xFFFFFFFFFFFFFFDEULL;
struct __bss_info bss_info[MAX_BSSIDS];
u16 tx_rate_encoding(int format, int ch_offset, int retry_count, int sgi, int rate)
{
    union __tx_rate
    {
        struct
        {
#if defined(BIG_ENDIAN)
            u16 ch_offset:2;
            u16 txcnt:3;
            u16 sgi:1;
            u16 format:2;
            u16 ht:1;
            u16 rate:7;
#else
            u16 rate:7;
            u16 ht:1;
            u16 format:2;
            u16 sgi:1;
            u16 txcnt:3;
            u16 ch_offset:2;
#endif	 
        };
        struct
        {
            u16 encoded_rate;
        };

    } __attribute__((__packed__));

    union __tx_rate  txrate;

    txrate.encoded_rate = 0;

    txrate.ch_offset = ch_offset;
    txrate.txcnt = retry_count;
    txrate.sgi = sgi;
    txrate.format = format;
    txrate.rate = rate;

    if ((format==FMT_HT_MIXED)||(format==FMT_HT_GF))
        txrate.ht = 1;

    return txrate.encoded_rate;
}

void fill_out_tx_descriptor(mpdu *pkt, tx_descriptor *descr, u32 flags, u32 subtype)
{
    sta_cfg* sta_tbl = load_station_config();
    sta_cfg* sta = NULL;
    int cipher_mode = 0;

    memset((void *)descr, 0, sizeof(tx_descriptor));

    if (subtype == WLAN_FC_SUBTYPE_PROBE_RESP)
    {
        descr->ts = 1;
    }

    if ((pkt->sta_id >= 0) && (pkt->sta_id < MAX_STA_NUM))
    {
        sta = &sta_tbl[pkt->sta_id];
        cipher_mode = sta->cipher_mode;
        descr->wep_def = sta->wep_defkey;

        descr->ds_idx = sta->ap_idx;
    }

    if (pkt->multicast)
    {
        descr->bc = 1;
        descr->ack = 1;  // 2 bits,  0: normal ack/blockack, 1: no ack

        cipher_mode = bss_info[pkt->bss_idx].cipher_mode;
    }

    descr->bssid = pkt->bss_idx;
    descr->tid = pkt->tid;
    descr->pkt_length = pkt->length;
    descr->key_idx = pkt->sta_id;

    if ((cipher_mode != CIPHER_TYPE_NONE) && (pkt->type==FRAME_TYPE_DATA))
    {
        descr->sec = 1;  
        descr->cipher_mode = cipher_mode;
    }
    descr->sn = pkt->seq_num;

    if (pkt->total_fragments)
        descr->no_mic = 1;

    if (pkt->ethernet_format)
    {
        descr->wh = 0;

        descr->qos = pkt->qos;
        descr->frds = (pkt->wifi_hdr[1] & 0x2) ? 1 : 0;
        descr->tods = (pkt->wifi_hdr[1] & 0x1) ? 1 : 0;
        descr->md = pkt->more_data;
        descr->eosp = pkt->eosp;
        descr->amsdu = pkt->amsdu;

        descr->pm = pkt->power_management;

        if (pkt->llc_length)
            descr->llc = 1;

        descr->hdr_len = ETH_HEADER_LEN;
    }
    else
    {
        descr->wh = 1;

        descr->qos = pkt->qos;
        //descr->llc = 0;

        descr->hdr_len = pkt->header_length;
    }

    if ((cipher_mode==CIPHER_TYPE_WEP40) || (cipher_mode==CIPHER_TYPE_WEP104))
    {
        pkt->tsc[1] = 0;
        pkt->tsc[0] = (0x0000000000FFFFFFULL &  tsc);
        descr->tsc[0] = (0x0000000000FFFFFFULL &  tsc);
        tsc++;
    }
    else if (cipher_mode==CIPHER_TYPE_TKIP)
    {
        pkt->tsc[1] = 0;
        pkt->tsc[0] = (0x0000FFFFFFFFFFFFULL &  tsc);
        descr->tsc[0] = (0x0000FFFFFFFFFFFFULL &  tsc);
        tsc++;
    }
    else if (cipher_mode==CIPHER_TYPE_CCMP)
    {
        pkt->tsc[1] = 0;
        pkt->tsc[0] = (0x0000FFFFFFFFFFFFULL &  tsc);
        descr->tsc[0] = (0x0000FFFFFFFFFFFFULL &  tsc);
        tsc++;
    }
    else if (cipher_mode==CIPHER_TYPE_WAPI)
    {
        pkt->tsc[0] = tsc_hi;
        pkt->tsc[1] = tsc;
        descr->tsc[0] = tsc_hi++;
        descr->tsc[1] = tsc++;
    }

    if ((flags & FILL_PER_STA_TX_RATE) && sta)
    {
        descr->tx_rate[0] = sta->tx_rate[0];
        descr->tx_rate[1] = sta->tx_rate[1];
        descr->tx_rate[2] = sta->tx_rate[2];
        descr->tx_rate[3] = sta->tx_rate[3];
    }
    else
    {
        if (0 == default_tx_rates_init)
        {
            default_tx_rates[0] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_0);
            default_tx_rates[1] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_0);
            default_tx_rates[2] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_0);
            default_tx_rates[3] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_0);
            default_tx_rates_init = 1;
        }

        descr->tx_rate[0] = default_tx_rates[0];
        descr->tx_rate[1] = default_tx_rates[1];
        descr->tx_rate[2] = default_tx_rates[2];
        descr->tx_rate[3] = default_tx_rates[3];
    }
}

void tx_acq_kick(acq *q, u32 esn)
{
    int qid;
    int cmdid;

    qid = q->qid;

    ACQ_LOCK();

    q->flags |= ACQ_FLAG_ACTIVE;
    q->esn = (u16) esn;

    if (q->flags & ACQ_FLAG_CMD0_REQ)
    {
        MACREG_WRITE32(ACQ_INFO2(qid,0), esn);
        ACQ_UNLOCK();
        return;
    }
    else if (q->flags & ACQ_FLAG_CMD1_REQ)
    {
        MACREG_WRITE32(ACQ_INFO2(qid,1), esn);
        ACQ_UNLOCK();
        return;
    }

    cmdid = -1;
    if ((info->acq_hw_requested[qid][0])==0)
    {
        q->flags |= ACQ_FLAG_CMD0_REQ;
        cmdid = 0;
    }
    else if ((info->acq_hw_requested[qid][1])==0)
    {
        q->flags |= ACQ_FLAG_CMD1_REQ;
        cmdid = 1;
    }

    if (cmdid>=0)
    {
        info->acq_hw_requested[qid][cmdid] = (u32) q;

        MACREG_WRITE32(ACQ_INFO(qid, cmdid), q->qinfo);
        MACREG_WRITE32(ACQ_INFO2(qid,cmdid), esn);
        if (q->flags & ACQ_FLAG_LOCK)
            MACREG_WRITE32(ACQ_BADDR(qid, cmdid), (ACQ_REQ | ACQ_REQ_LOCK | ((q->queue_size_code) << 28) | (PHYSICAL_ADDR(&q->acqe[0])>>2)));
        else
            MACREG_WRITE32(ACQ_BADDR(qid, cmdid), (ACQ_REQ | ((q->queue_size_code) << 28) | (PHYSICAL_ADDR(&q->acqe[0])>>2)));
    }

    ACQ_UNLOCK();
}

extern void rand_init(void);
void tx_random_packet_generate(void)
{
    int idx, size;
    unsigned int num_t;

    size = sizeof(test_tx_packet);

    rand_init();

    for(idx = 0; idx < (size - 38); idx++)
    {
        num_t = (unsigned int) rand();
        test_tx_packet[38+idx] = num_t & 0xff;
    }
}

#define MAX_PAYLOAD (1518)
#define MIN_PAYLOAD (64)
pkt tx_packet;
int TX_packet_simply(int source, int target, int tid, int payload_length, u16 *rates)
{
    tx_descriptor *descr;
    mpdu *pkt;
    int bhr_h, bhr_t;
    buf_header *bhdr;
    volatile acqe* _acqe;
    acq* q;
//   int i;
    //printf("send TX_packet %x\n", rates[0]);
    pkt = packet_alloc();
    if (NULL==pkt)
    {
        printf("packet_alloc() failed\n");
        return 1;
        cosim_panic("packet_alloc() failed");
    }
    q = DEF_ACQ(0);
    Retry:
    if (ACQ_FULL(q))
    {
        packet_free(pkt);
        return 1;
        cosim_delay_ns(10000);
        goto Retry;
    }
#if 1
    if (MIN_PAYLOAD > payload_length)
    {
        printf("payload less than %d\n", MIN_PAYLOAD);
        payload_length = MIN_PAYLOAD;
    }
    else if (MAX_PAYLOAD < payload_length)
    {
        printf("payload bigger than %d\n", MAX_PAYLOAD);
        payload_length = MAX_PAYLOAD;
    }
    pkt->length = payload_length;
    memcpy(&pkt->data[0], &test_tx_packet[0], pkt->length);
    pkt->data[36] = ((payload_length - 38) >> 8) & 0xffUL;
    pkt->data[37] = (payload_length - 38) & 0xffUL;
    pkt->direction = DIRECTION_OUT;
    pkt->sta_id = -1;
    pkt->bss_idx = 0;
    pkt->tid = tid;
    pkt->total_fragments = 0;
    pkt->ethernet_format = 0;
    pkt->qos = 0x01;
    memcpy(&pkt->wifi_hdr[0], &test_tx_packet[0], 26);
    memset((void *) &pkt->wifi_hdr[26], 0, 10);
    pkt->more_data = 0;
    pkt->eosp = 0;
    pkt->amsdu = 0;
    pkt->power_management = 0;
    pkt->llc_length =6;
    pkt->header_length = 26;
    pkt->multicast = 1;
#endif
#if 0
    printf("direction = %02x, sta_id = %d, bss_idx = %d, total_fragments = %04x, ethernet_format = %02x\n"
           "qos = %02x, more_data = %d, eosp = %d, amsdu = %d, power_management = %04x, llc_length = %d\n"
           "header_length = %d, multicast = %d\n",
           pkt->direction, pkt->sta_id, pkt->bss_idx, pkt->total_fragments, pkt->ethernet_format, pkt->qos, 
           pkt->more_data, pkt->eosp, pkt->amsdu, pkt->power_management, pkt->llc_length, pkt->header_length, pkt->multicast);
    printf("wifi_hdr = ");
    for (i=0;i<36;i++)
    {
        printf("%02x ", pkt->wifi_hdr[i]);
        if (i==16)
        {
            printf("\n");
        }
    }
    printf("\n");
#endif
    if (0 > mpdu_copy_to_buffer(pkt, 0, &bhr_h, &bhr_t))
    {
        //printf("TX copy buffer failed\n");
        //cosim_panic("TX copy buffer failed");
        packet_free(pkt);
        return 1;
        cosim_delay_ns(10000);
        goto Retry;
    }
    packet_free(pkt);
    // fill TX descr
    bhdr = idx_to_bhdr(info, bhr_h);
    descr = (tx_descriptor *) UNCACHED_VIRTUAL_ADDR(bhdr->dptr);

    fill_out_tx_descriptor(pkt, descr, 0, NULL);

    descr->bhr_t = bhr_t;
    descr->bhr_h = bhr_h;

    if (rates)
    {
#if defined (WLA_TEST)
        if(boot3_submode == RECOVER_SUBMODE)
        {
            descr->tx_rate[0] = rates[0];
            descr->tx_rate[1] = rates[1];
            descr->tx_rate[2] = rates[2];
            descr->tx_rate[3] = rates[3];
        }
        else
        {
            descr->tx_rate[0] = rates[0];
            descr->tx_rate[1] = 0;
            descr->tx_rate[2] = 0;
            descr->tx_rate[3] = 0;
        }
#else
        descr->tx_rate[0] = rates[0];
        descr->tx_rate[1] = rates[1];
        descr->tx_rate[2] = rates[2];
        descr->tx_rate[3] = rates[3];
#endif
    }

    _acqe = (acqe*)UNCACHED_ADDR(ACQE(q, q->wptr));
    _acqe->m.own = 1;
    _acqe->m.bmap = 0;
    _acqe->m.try_cnt = 0;
    _acqe->m.mb = 0;
    _acqe->m.bhr_h = bhr_h;
    _acqe->m.aidx = pkt->sta_id;

    q->wptr++;
    tx_acq_kick(q, q->wptr);
    return 0;
}

#define NEXT_TX_SEQ_NUM(_seq)  (_seq++ & 0xfff)
static unsigned int tx_seq_num = 0;
int TX_packet(u8 *buf, u32 payload_length, u32 subtype)
{
    tx_descriptor *descr;
    mpdu *pkt;
    int bhr_h, bhr_t;
    buf_header *bhdr;
    volatile acqe* _acqe;
    acq* q;
    u32 seq_num;

    pkt = packet_alloc();
    if (NULL==pkt)
    {
        printf("packet_alloc() failed\n");
        return 1;
        cosim_panic("packet_alloc() failed");
    }
    q = DEF_ACQ(0);
    Retry:
    if (ACQ_FULL(q))
    {
        packet_free(pkt);
        return 1;
        cosim_delay_ns(10000);
        goto Retry;
    }

    if (MIN_PAYLOAD > payload_length)
    {
        printf("payload less than %d\n", MIN_PAYLOAD);
        payload_length = MIN_PAYLOAD;
    }
    else if (MAX_PAYLOAD < payload_length)
    {
        printf("payload bigger than %d\n", MAX_PAYLOAD);
        payload_length = MAX_PAYLOAD;
    }
    pkt->length = payload_length;

    memcpy(&pkt->data[0], buf, pkt->length);
    memcpy(&pkt->wifi_hdr[0], buf, 26);

#if defined (WLA_TEST)
    if(boot3_submode == RECOVER_SUBMODE)
    {
        seq_num = NEXT_TX_SEQ_NUM(tx_seq_num);
        pkt->data[22] = (seq_num & 0xf) << 4;
        pkt->data[23] = (seq_num >> 4) & 0xff;
    }
    else
    {
        pkt->data[36] = ((payload_length - 38) >> 8) & 0xffUL;
        pkt->data[37] = (payload_length - 38) & 0xffUL;
        memset((void *) &pkt->wifi_hdr[26], 0, 10);
    }
#else
    seq_num = NEXT_TX_SEQ_NUM(tx_seq_num);
    pkt->data[22] = (seq_num & 0xf) << 4;
    pkt->data[23] = (seq_num >> 4) & 0xff;
#endif
    pkt->direction = DIRECTION_OUT;
    pkt->sta_id = -1;
    pkt->bss_idx = 0;
    pkt->tid = 0;
    pkt->total_fragments = 0;
    pkt->ethernet_format = 0;
    pkt->qos = 0x01;
    pkt->more_data = 0;
    pkt->eosp = 0;
    pkt->amsdu = 0;
    pkt->power_management = 1;
    pkt->llc_length = 6;
    pkt->header_length = 26;
    pkt->multicast = 0;

#if 0
    printf("direction = %02x, sta_id = %d, bss_idx = %d, total_fragments = %04x, ethernet_format = %02x\n"
           "qos = %02x, more_data = %d, eosp = %d, amsdu = %d, power_management = %04x, llc_length = %d\n"
           "header_length = %d, multicast = %d\n",
           pkt->direction, pkt->sta_id, pkt->bss_idx, pkt->total_fragments, pkt->ethernet_format, pkt->qos, 
           pkt->more_data, pkt->eosp, pkt->amsdu, pkt->power_management, pkt->llc_length, pkt->header_length, pkt->multicast);
    printf("wifi_hdr = ");
    for (i=0;i<36;i++)
    {
        printf("%02x ", pkt->wifi_hdr[i]);
        if (i==16)
        {
            printf("\n");
        }
    }
    printf("\n");
#endif

    if (0 > mpdu_copy_to_buffer(pkt, 0, &bhr_h, &bhr_t))
    {
        //printf("TX copy buffer failed\n");
        //cosim_panic("TX copy buffer failed");
        packet_free(pkt);
        return 1;
        cosim_delay_ns(10000);
        goto Retry;
    }
    packet_free(pkt);
    // fill TX descr
    bhdr = idx_to_bhdr(info, bhr_h);
    descr = (tx_descriptor *) UNCACHED_VIRTUAL_ADDR(bhdr->dptr);

    fill_out_tx_descriptor(pkt, descr, 0, subtype);

    if(boot3_submode == RECOVER_SUBMODE)
        descr->md = 1;  // force trigger more data bit to keep station awake

    descr->bhr_t = bhr_t;
    descr->bhr_h = bhr_h;

    if (tx_rates[0])
    {
        descr->tx_rate[0] = tx_rates[0];
        descr->tx_rate[1] = tx_rates[1];
        descr->tx_rate[2] = tx_rates[2];
        descr->tx_rate[3] = tx_rates[3];
    }

    _acqe = (acqe*)UNCACHED_ADDR(ACQE(q, q->wptr));
    _acqe->m.own = 1;
    _acqe->m.bmap = 0;
    _acqe->m.try_cnt = 0;
    _acqe->m.mb = 0;
    _acqe->m.bhr_h = bhr_h;
    _acqe->m.aidx = pkt->sta_id;

    q->wptr++;
    tx_acq_kick(q, q->wptr);
    return 0;
}

void cosim_station_config(int operation, unsigned long config, unsigned long config2, unsigned char *macaddr)
{
#if defined(REAL_BB)
    return;
#endif

    COSIM_LOCK();

    COSIM_REG_WRITE32(COSIM_STA_CONFIG, config);
    COSIM_REG_WRITE32(COSIM_STA_CONFIG2, config2);
    COSIM_REG_WRITE32(COSIM_STA_CONFIG_MAC_ADDR0, (macaddr[0] << 8) | macaddr[1]);
    COSIM_REG_WRITE32(COSIM_STA_CONFIG_MAC_ADDR1, (macaddr[2] << 24) | (macaddr[3] << 16) | (macaddr[4] << 8) | macaddr[5]);
    COSIM_REG_WRITE32(COSIM_STACFG_OPERATION, operation);

    COSIM_UNLOCK();
}

unsigned long ip_byte_array_to_long(unsigned char *ip)
{
    unsigned long data;

    data = ((ip[0]<<24) | (ip[1]<<16) | (ip[2]<<8) | ip[3]);

    return cpu_to_be32(data);
}

/* structure to store AP information, including P2PGO/AP/WDS-AP */
struct __ap_info
{
    int BSS;            /* BSS identification num (>=MAX_BSSIDS) */
    int addr_index;     /* stacap index */
    int bssid;          /* peer/associate with our bssid */ 
};

struct __ap_info ap_info[MAX_AP_COUNT];
int ap_idx = 0;
int setup_network_configuration(void)
{
    sta_cfg *cfg;
    unsigned char RX_MAC_ADDR[] = {0x78, 0x00, 0x00, 0x00, 0x00, 0x41};
#if 0
    unsigned long station_config_0 = 0x03000080;
    unsigned long station_config1_0 = 0x00000000;
    unsigned long station_config_1 = 0x020000ff;
    unsigned long station_config1_1 = 0x00000000;
    unsigned long station_config_2 = 0x02000041;
    unsigned long station_config1_2 = 0x00ff0000;
#endif
/*
   bss_info[0].dut_role = ROLE_AP;
   memcpy(bss_info[0].MAC_ADDR, net_cfg[0].MAC_ADDR, ETH_ALEN);
   memcpy(bss_info[0].WAN_MAC_ADDR, net_cfg[0].WAN_MAC_ADDR, ETH_ALEN);
   bss_info[0].timer_index = 0;
   bss_info[1].timer_index = 0;
   bss_info[2].timer_index = 0;
   memcpy(bss_info[0].BSSID, net_cfg[0].MAC_ADDR, ETH_ALEN);
*/
    bss_info[0].dut_role = ROLE_STA;
    memcpy(bss_info[0].MAC_ADDR, net_cfg[0].MAC_ADDR, ETH_ALEN);
    memcpy(bss_info[0].WAN_MAC_ADDR, net_cfg[0].WAN_MAC_ADDR, ETH_ALEN);

    bss_info[0].timer_index = 0;
    bss_info[1].timer_index = 1;
    bss_info[2].timer_index = 2;
    net_cfg[0].sta_id = -1;
/*   
   cfg = &__sta_cfg[0];
   cfg->addr_index = 0;
   cfg->basic_info.bit.vld = 0;
   cfg->valid = 1;
   memcpy(cfg->RA_ADDR, net_cfg[1].MAC_ADDR, ETH_ALEN);
   cfg->qos = 1;
   cfg->bssid = 0;
   cfg->basic_info.bit.bssid = 0;
   memset(&cfg->key, 0, sizeof(crypto_key));
   memcpy(cfg->key.key, cfg->RA_ADDR, ETH_ALEN);
   cfg->u_apsd_bitmap = 0;
*/
    /*
    net_cfg[1].sta_id = 0;
    cfg = &__sta_cfg[0];
    cfg->addr_index = 0;
    cfg->basic_info.bit.vld = 1;
    cfg->valid = 1;
    memcpy(cfg->RA_ADDR, net_cfg[1].MAC_ADDR, ETH_ALEN);
    cfg->qos = 1;
    cfg->bssid = 0;
    cfg->basic_info.bit.bssid = 0;
    memset(&cfg->key, 0, sizeof(crypto_key));
    memcpy(cfg->key.key, cfg->RA_ADDR, ETH_ALEN);
    cfg->u_apsd_bitmap = 0;
 */
//  net_cfg[2].sta_id = 1;
    cfg = &__sta_cfg[1];
    cfg->addr_index = 1;
    cfg->basic_info.bit.vld = 1;
    cfg->valid = 1;
    memcpy(cfg->RA_ADDR, RX_MAC_ADDR, ETH_ALEN);
    cfg->qos = 1;
    cfg->bssid = 1;
    cfg->basic_info.bit.bssid = 1;
    memset((void *) &cfg->key, 0, sizeof(crypto_key));
    memcpy(cfg->key.key, cfg->RA_ADDR, ETH_ALEN);
    cfg->basic_info.bit.rx_ampd0 = 1;
    cfg->basic_info.bit.rx_ampd1 = 1;
    cfg->basic_info.bit.rx_ampd2 = 1;
    cfg->basic_info.bit.rx_ampd3 = 1;
    cfg->basic_info.bit.rx_ampd4 = 1;
    cfg->basic_info.bit.rx_ampd5 = 1;
    cfg->basic_info.bit.rx_ampd6 = 1;
    cfg->basic_info.bit.rx_ampd7 = 1;



#if !defined(REAL_BB)
    printf("COSIM_STACFG_OPERATION_ADD = %d, station_config = %08x, station_config1 = %08x\n", 
           COSIM_STACFG_OPERATION_ADD, station_config_0, station_config1_0);
    cosim_station_config(COSIM_STACFG_OPERATION_ADD, station_config_0, station_config1_0, net_cfg[0].MAC_ADDR);

    printf("COSIM_STACFG_OPERATION_ADD = %d, station_config = %08x, station_config1 = %08x\n", 
           COSIM_STACFG_OPERATION_ADD, station_config_1, station_config1_1);
    cosim_station_config(COSIM_STACFG_OPERATION_ADD, station_config_1, station_config1_1, net_cfg[1].MAC_ADDR);

    printf("COSIM_STACFG_OPERATION_ADD = %d, station_config = %08x, station_config1 = %08x\n", 
           COSIM_STACFG_OPERATION_ADD, station_config_2, station_config1_2);
    cosim_station_config(COSIM_STACFG_OPERATION_ADD, station_config_2, station_config1_2, net_cfg[1].MAC_ADDR);
#endif

    return 0;      
}

#define __QOS           0x1
#define __AP            0x2
#define __WEP40_DEF     0x4
#define __WEP104_DEF    0x8
#define __WEP40         0x10
#define __WEP104        0x20
#define __TKIP          0x40
#define __CCMP          0x80
#define __WAPI          0x100
#define __ETH           0x200
#define __TOSW          0x400
#define __DUT           0x800
#define __STA           0x1000
#define __IBSS          0x2000
#define __P2PC          0x4000
#define __P2PGO         0x8000

unsigned char DEF_BSSID_ADDR[] = { 0x74, 0x00, 0x00, 0xff, 0xff, 0xfe};
#define DEF_MAC_ADDR_MSB    0x78

static char *mac_addr_string(u8 *macaddr)
{
    static char mac_addr_string[32];

    sprintf(mac_addr_string, "%02x:%02x:%02x:%02x:%02x:%02x",
            macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);

    return mac_addr_string;
}

#if 1
int apply_network_configuration(void)
{
    int i,j;
    int sta_id;
    sta_cfg *cfg;
    unsigned long station_config, station_config1;
    unsigned char zero_mac_addr[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    int sta_tbl_index = 0;
    int bssidx;

    int timer_index = 0;

    ap_idx = 0;

    DBG("1st phase: parsing DUT role per-BSS\n");

    for (i=0;i<NET_CFG_ENTRIES;i++)
    {
        if (net_cfg[i].CAP & __DUT)
        {
            bssidx = net_cfg[i].BSSID;

            if ((bssidx>=MAX_BSSIDS)||(bssidx<0))
                continue;

            if (net_cfg[i].CAP & __STA)
                bss_info[bssidx].dut_role = ROLE_STA;
            else if (net_cfg[i].CAP & __AP)
                bss_info[bssidx].dut_role = ROLE_AP;
            else if (net_cfg[i].CAP & __IBSS)
                bss_info[bssidx].dut_role = ROLE_IBSS;
            else if (net_cfg[i].CAP & __P2PC)
                bss_info[bssidx].dut_role = ROLE_P2PC;
            else if (net_cfg[i].CAP & __P2PGO)
                bss_info[bssidx].dut_role = ROLE_P2PGO;
            else
                bss_info[bssidx].dut_role = ROLE_NONE;

            memcpy(bss_info[bssidx].MAC_ADDR, net_cfg[i].MAC_ADDR, ETH_ALEN);
            memcpy(bss_info[bssidx].WAN_MAC_ADDR, net_cfg[i].WAN_MAC_ADDR, ETH_ALEN);
#if defined(HNAT)
            bss_info[bssidx].LAN_IP_ADDR = ip_byte_array_to_long(net_cfg[i].LAN_IP_ADDR);
            bss_info[bssidx].WAN_IP_ADDR = ip_byte_array_to_long(net_cfg[i].WAN_IP_ADDR);
#endif
        }
    }

    DBG("1st phase A: assign TSF timer for each BSS\n");

    for (i=0;i<MAX_BSSIDS;i++)
    {
        bss_info[i].timer_index = 0;

        if ((bss_info[i].dut_role == ROLE_AP) || (bss_info[i].dut_role == ROLE_P2PGO) || (bss_info[i].dut_role == ROLE_IBSS))
        {
            printf("    BSS(%d) TSF timer index 0\n", i);
            if (timer_index == 0)
                timer_index++;
        }
    }
    for (i=0;i<MAX_BSSIDS;i++)
    {
        if ((bss_info[i].dut_role == ROLE_STA) || (bss_info[i].dut_role == ROLE_P2PC))
        {
            printf("    BSS(%d) TSF timer index %d\n", i, timer_index);
            bss_info[i].timer_index = timer_index;
            timer_index++;
        }
    }


    DBG("2nd phase\n");
    for (i=0;i<NET_CFG_ENTRIES;i++)
    {
        DBG("Configuring STA %d\n", net_cfg[i].STA_NUM);

        // set MAC address
        if (memcmp(zero_mac_addr, net_cfg[i].MAC_ADDR, ETH_ALEN))
        {
            DBG("    MAC addr: %s\n", mac_addr_string(net_cfg[i].MAC_ADDR));
        }
        else
        {
            net_cfg[i].MAC_ADDR[0] = DEF_MAC_ADDR_MSB;
            net_cfg[i].MAC_ADDR[1] = net_cfg[i].BSSID;
            net_cfg[i].MAC_ADDR[4] = (net_cfg[i].STA_NUM & 0xff) >> 8;
            net_cfg[i].MAC_ADDR[5] = net_cfg[i].STA_NUM;
            DBG("    MAC addr: %s    (auto)\n", mac_addr_string(net_cfg[i].MAC_ADDR));
        }

        if (net_cfg[i].CAP & __STA)
        {
            DBG("    role: STA\n");
        }
        else if (net_cfg[i].CAP & __AP)
        {
            DBG("    role: AP\n");

            if (net_cfg[i].CAP & __DUT)
            {
                bssidx = net_cfg[i].BSSID;
                if (memcmp(zero_mac_addr, net_cfg[i].BSSID_ADDR, ETH_ALEN))
                {
                    DBG("    AP BSSID %s\n", mac_addr_string(net_cfg[i].BSSID_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].BSSID_ADDR, ETH_ALEN);
                }
                else if ((bssidx>=0)&&(bssidx<MAX_BSSIDS))
                {
                    DBG("    AP BSSID %s\n", mac_addr_string(net_cfg[i].MAC_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].MAC_ADDR, ETH_ALEN);
                }
            }
            else
            {
                bssidx = net_cfg[i].BSSID;

                if (((bssidx>=0)&&(bssidx<MAX_BSSIDS)) && (net_cfg[i].BSS < MAX_BSSIDS) &&
                    (bss_info[bssidx].dut_role != ROLE_AP) && (bss_info[bssidx].dut_role != ROLE_P2PGO))
                {
                    DBG("    AP BSSID %s\n", mac_addr_string(net_cfg[i].MAC_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].MAC_ADDR, ETH_ALEN);
                }
            }
        }
#if 0
        else if (net_cfg[i].CAP & __IBSS)
        {
            DBG("    role: IBSS STA\n");

            if (net_cfg[i].CAP & __DUT)
            {
                bssidx = net_cfg[i].BSSID;
                if (memcmp(zero_mac_addr, net_cfg[i].BSSID_ADDR, ETH_ALEN))
                {
                    DBG("    IBSS BSSID %s\n", mac_addr_string(net_cfg[i].BSSID_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].BSSID_ADDR, ETH_ALEN);
                    bss_info[bssidx].ap_dstable_idx = ap_idx++;
                    //    cfg->ess_idx = bss_info[bssidx].ap_dstable_idx;
                }
                else
                {
                    DBG("    IBSS BSSID %s\n", mac_addr_string(net_cfg[i].MAC_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].MAC_ADDR, ETH_ALEN);
                    bss_info[bssidx].ap_dstable_idx = ap_idx++;
                    //    cfg->ess_idx = bss_info[bssidx].ap_dstable_idx;
                }
            }
        }
        else if (net_cfg[i].CAP & __P2PC)
        {
            DBG("    role: P2P Client\n");
        }
        else if (net_cfg[i].CAP & __P2PGO)
        {
            DBG("    role: P2P GO\n");

            if (net_cfg[i].CAP & __DUT)
            {
                bssidx = net_cfg[i].BSSID;
                if (memcmp(zero_mac_addr, net_cfg[i].BSSID_ADDR, ETH_ALEN))
                {
                    DBG("    GO BSSID %s\n", mac_addr_string(net_cfg[i].BSSID_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].BSSID_ADDR, ETH_ALEN);
                }
                else if ((bssidx>=0)&&(bssidx<MAX_BSSIDS))
                {
                    DBG("    GO BSSID %s\n", mac_addr_string(net_cfg[i].MAC_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].MAC_ADDR, ETH_ALEN);
                }
            }
            else
            {
                bssidx = net_cfg[i].BSSID;

                if (((bssidx>=0)&&(bssidx<MAX_BSSIDS)) && (net_cfg[i].BSS < MAX_BSSIDS) &&
                    (bss_info[bssidx].dut_role != ROLE_AP) && (bss_info[bssidx].dut_role != ROLE_P2PGO))
                {
                    DBG("    GO BSSID %s\n", mac_addr_string(net_cfg[i].MAC_ADDR));
                    memcpy(bss_info[bssidx].BSSID, net_cfg[i].MAC_ADDR, ETH_ALEN);
                }
            }
        }
#endif
        if (net_cfg[i].CAP & __DUT)
        {

            DBG("    DUT (BSSID %d)\n", net_cfg[i].BSSID);

            net_cfg[i].sta_id = -1;

            if (0 == (net_cfg[i].CAP & (__STA|__AP|__IBSS|__P2PC|__P2PGO)))
            {
                DBG("    Default to AP mode\n");
                net_cfg[i].CAP |= __AP;
            }

            continue;
        }

        if (net_cfg[i].ESS > 0)
        {
            /* not our ESS, not implement yet */
            DBG("    ESS-%d, skipped\n", net_cfg[i].ESS);
            net_cfg[i].sta_id = -1;
            continue;
        }
        else if ((net_cfg[i].BSSID < 0) || (net_cfg[i].BSSID >= MAX_BSSIDS))
        {
            net_cfg[i].sta_id = -1;
            continue;
        }
        else if (((net_cfg[i].BSSID >= 0) && (net_cfg[i].BSSID < MAX_BSSIDS)) && 
                 ( (bss_info[net_cfg[i].BSSID].dut_role == ROLE_STA)  || (bss_info[net_cfg[i].BSSID].dut_role == ROLE_P2PC))
                 && (net_cfg[i].CAP & (__STA | __P2PC)))
        {
            /* not generating stacap table when we are a STA and the network node is also a STA */
            net_cfg[i].sta_id = -1;
            continue;
        }
        else if (net_cfg[i].BSS >= MAX_BSSIDS)
        {
            DBG("    ESS-%d  BSS-%d\n", net_cfg[i].ESS, net_cfg[i].BSS);
            if (net_cfg[i].CAP & __AP)
            {
                if (ap_idx>=MAX_AP_COUNT)
                {
                    DBG("ERROR: Too many WDS peers\n");
                    net_cfg[i].sta_id = -1;
                    continue;
                }

                /* WDS-AP, handle it in first phase */
                DBG("    WDS-AP\n");
                sta_id = sta_tbl_index;
                net_cfg[i].sta_id = sta_id;
                sta_tbl_index++;
                DBG("    Assign sta_id/key_id %d for WDS-AP %d\n", sta_id, net_cfg[i].STA_NUM);
            }
            else
            {
                DBG("    WDS-STA, Going to process on 2nd phase\n");
                net_cfg[i].sta_id = -1;
                continue;
            }
        }
        else
        {
            sta_id = sta_tbl_index;
            net_cfg[i].sta_id = sta_id;
            sta_tbl_index++;
            DBG("    Assign sta_id/key_id %d for STA %d\n", sta_id, net_cfg[i].STA_NUM);
        }
        cfg = &__sta_cfg[sta_id];
        cfg->addr_index = sta_id;

        if ((net_cfg[i].CAP & __P2PGO) || (net_cfg[i].CAP & __AP))
        {
            bssidx = net_cfg[i].BSSID;

            if (((bssidx>=0)&&(bssidx<MAX_BSSIDS)) && (net_cfg[i].BSS < MAX_BSSIDS) &&
                (bss_info[bssidx].dut_role != ROLE_AP) && (bss_info[bssidx].dut_role != ROLE_P2PGO))
            {
                bss_info[bssidx].AP_STA_NUM = i;
                bss_info[bssidx].ap_stacap_idx = sta_id;
            }

            /* force packet send to CPU (instead of LAN) in when our STA receive packets from AP in simulation environment */
            cfg->basic_info.bit.tosw = 1;
        }

        if (net_cfg[i].CAP & __IBSS)
        {
            /* force packet send to CPU (instead of LAN) in when our IBSS-STA receive packets from peers in simulation environment */
            cfg->basic_info.bit.tosw = 1;
        }

        // toggle valid bits in address lookup table
        if (net_cfg[i].CAP == 0)
        {
            printf("    This STA shall be created for simulation purpose only\n");
            cfg->basic_info.bit.vld = 0;
            net_cfg[i].CAP |= __QOS;
            net_cfg[i].sta_id = -1;
        }
        else
        {
            cfg->basic_info.bit.vld = 1;
        }

        cfg->valid = 1;
        memcpy(cfg->RA_ADDR, net_cfg[i].MAC_ADDR, ETH_ALEN);

        // set legacy or QOS
        if (net_cfg[i].CAP & __QOS)
        {
            DBG("    QoS STA\n");
            cfg->qos = 1;
        }
        else
        {
            DBG("    Legacy STA\n");
        }

        // set AP or Station
        if (net_cfg[i].CAP & (__AP | __P2PGO))
        {
            bssidx = net_cfg[i].BSSID;

            if ((bssidx>=0) && (bssidx<MAX_BSSIDS) && (bss_info[bssidx].dut_role != ROLE_AP))
            {
                DBG("    AP-STA or P2PGO-STA\n");

                cfg->ap = 1;
                cfg->parent_ap = 1;
                cfg->ess_idx = ap_idx;

                cfg->ap_idx = ap_idx;
                ap_info[ap_idx].BSS = net_cfg[i].BSS;
                ap_info[ap_idx].addr_index = cfg->addr_index;
                ap_info[ap_idx].bssid = net_cfg[i].BSSID;

                ap_idx++;

                //assign_group_key(net_cfg[i].BSSID, net_cfg[i].CAP);
            }
            else
            {
                DBG("    WDS AP-STA\n");

                cfg->ap = 1;
                cfg->basic_info.bit.under_ap = 1;
                cfg->ess_idx = ap_idx;

                cfg->ap_idx = ap_idx;
                ap_info[ap_idx].BSS = net_cfg[i].BSS;
                ap_info[ap_idx].addr_index = cfg->addr_index;
                ap_info[ap_idx].bssid = net_cfg[i].BSSID;

                if (net_cfg[i].CAP & __WAPI)
                {
                    DBG("    adjust WAPI PN check to INC1\n");
                    cfg->rx_check_inc = WAPI_RX_CHECK_INC;
                }

                ap_idx++;

                /* XXX: GKEY? */
            }
        }
        else
        {
            DBG("    NonAP-STA\n");

            //assign_group_key(net_cfg[i].BSSID, net_cfg[i].CAP);
        }


        // associate BSSID
        DBG("    BSSID-%d\n", net_cfg[i].BSSID);
        cfg->bssid = net_cfg[i].BSSID;
        cfg->basic_info.bit.bssid = net_cfg[i].BSSID;            


        // set security mode & key
        memset((void *) &cfg->key, 0, sizeof(crypto_key));
        memcpy(cfg->key.key, cfg->RA_ADDR, ETH_ALEN);
#if 0
        if (net_cfg[i].CAP & __WEP40_DEF)
        {
            DBG("    WEP40 default key\n");

            cfg->key.keytype = KEYTYPE_WEP;
            cfg->key.keylen = 5;

            // XXX: todo
            cfg->basic_info.bit.wep_defkey = 1;
            cfg->basic_info.bit.cipher = 1;

            cfg->cipher_mode = CIPHER_TYPE_WEP40;
        }
        else if (net_cfg[i].CAP & __WEP104_DEF)
        {
            DBG("    WEP104 default key\n");

            cfg->key.keytype = KEYTYPE_WEP;
            cfg->key.keylen = 13;

            // XXX: todo
            cfg->basic_info.bit.wep_defkey = 1;
            cfg->basic_info.bit.cipher = 1;

            cfg->cipher_mode = CIPHER_TYPE_WEP104;
        }
        else if (net_cfg[i].CAP & __WEP40)
        {
            DBG("    WEP40 key-mapping key, ID %d\n", net_cfg[i].WEP_KEYID);

            cfg->basic_info.bit.cipher = 1;

            cfg->key.txkeyid = net_cfg[i].WEP_KEYID;

            cfg->key.keytype = KEYTYPE_WEP;
            cfg->key.keylen = 5;
            cfg->key.keylen1 = 5;
            cfg->key.keylen2 = 5;
            cfg->key.keylen3 = 5;

            memcpy(cfg->key.key, cfg->RA_ADDR, 5);
            memcpy(cfg->key.key1, cfg->RA_ADDR, 5);
            memcpy(cfg->key.key2, cfg->RA_ADDR, 5);
            memcpy(cfg->key.key3, cfg->RA_ADDR, 5);
            cfg->key.key[0] = 0x50;
            cfg->key.key1[0] = 0x51;
            cfg->key.key2[0] = 0x52;
            cfg->key.key3[0] = 0x53;

            cfg->cipher_mode = CIPHER_TYPE_WEP40;
        }
        else if (net_cfg[i].CAP & __WEP104)
        {
            DBG("    WEP104 key-mapping key, ID %d\n", net_cfg[i].WEP_KEYID);

            cfg->basic_info.bit.cipher = 1;

            cfg->key.txkeyid = net_cfg[i].WEP_KEYID;

            cfg->key.keytype = KEYTYPE_WEP;
            cfg->key.keylen = 13;
            cfg->key.keylen1 = 13;
            cfg->key.keylen2 = 13;
            cfg->key.keylen3 = 13;

            memcpy(cfg->key.key, cfg->RA_ADDR, ETH_ALEN);
            memcpy(cfg->key.key1, cfg->RA_ADDR, ETH_ALEN);
            memcpy(cfg->key.key2, cfg->RA_ADDR, ETH_ALEN);
            memcpy(cfg->key.key3, cfg->RA_ADDR, ETH_ALEN);
            cfg->key.key[0] = 0x40;
            cfg->key.key1[0] = 0x41;
            cfg->key.key2[0] = 0x42;
            cfg->key.key3[0] = 0x43;

            cfg->cipher_mode = CIPHER_TYPE_WEP104;
        }
        else if (net_cfg[i].CAP & __TKIP)
        {
            DBG("    TKIP\n");

            cfg->basic_info.bit.cipher = 1;

            cfg->key.keytype = KEYTYPE_TKIP;
            cfg->key.keylen = 16;
            cfg->key.keytsc = 1;

            cfg->cipher_mode = CIPHER_TYPE_TKIP;
        }
        else if (net_cfg[i].CAP & __CCMP)
        {
            DBG("    CCMP\n");

            cfg->basic_info.bit.cipher = 1;

            cfg->key.keytype = KEYTYPE_CCMP;
            cfg->key.keylen = 16;
            cfg->key.keytsc = 1;

            cfg->cipher_mode = CIPHER_TYPE_CCMP;
        }
        else if (net_cfg[i].CAP & __WAPI)
        {
            DBG("    WAPI\n");

            cfg->basic_info.bit.cipher = 1;

            cfg->key.keytype = KEYTYPE_WAPI;
            cfg->key.keylen = 16;

            cfg->cipher_mode = CIPHER_TYPE_WAPI;
        }
        else

#endif
        {
            DBG("    None security\n");
        }
#if 0
        if (net_cfg[i].CAP & __TOSW)
        {
            DBG("    Forced RX to CPU\n");
            cfg->basic_info.bit.tosw = 1;
        }

        if (net_cfg[i].CAP & __ETH)
        {
            DBG("    enable RX header conversion\n");
            cfg->basic_info.bit.eth_header = 1;
            cfg->eth = 1;
        }
#endif
        if (net_cfg[i].AMPDU_TX)
        {
            DBG("    AMPDU TX TID BITMAP %02X\n", net_cfg[i].AMPDU_TX);
            if (net_cfg[i].AMPDU_TX & 0x01)
                cfg->tx_ampd0 = 1;
            if (net_cfg[i].AMPDU_TX & 0x02)
                cfg->tx_ampd1 = 1;
            if (net_cfg[i].AMPDU_TX & 0x04)
                cfg->tx_ampd2 = 1;
            if (net_cfg[i].AMPDU_TX & 0x08)
                cfg->tx_ampd3 = 1;
            if (net_cfg[i].AMPDU_TX & 0x10)
                cfg->tx_ampd4 = 1;
            if (net_cfg[i].AMPDU_TX & 0x20)
                cfg->tx_ampd5 = 1;
            if (net_cfg[i].AMPDU_TX & 0x40)
                cfg->tx_ampd6 = 1;
            if (net_cfg[i].AMPDU_TX & 0x80)
                cfg->tx_ampd7 = 1;
        }

        if (net_cfg[i].AMPDU_RX)
        {
#if 0
            DBG("    Forced AMPDU RX to CPU\n");
            cfg->basic_info.bit.tosw = 1;
#endif            
            DBG("    AMPDU RX TID BITMAP %02X\n", net_cfg[i].AMPDU_RX);
            if (net_cfg[i].AMPDU_RX & 0x01)
                cfg->basic_info.bit.rx_ampd0 = 1;
            if (net_cfg[i].AMPDU_RX & 0x02)
                cfg->basic_info.bit.rx_ampd1 = 1;
            if (net_cfg[i].AMPDU_RX & 0x04)
                cfg->basic_info.bit.rx_ampd2 = 1;
            if (net_cfg[i].AMPDU_RX & 0x08)
                cfg->basic_info.bit.rx_ampd3 = 1;
            if (net_cfg[i].AMPDU_RX & 0x10)
                cfg->basic_info.bit.rx_ampd4 = 1;
            if (net_cfg[i].AMPDU_RX & 0x20)
                cfg->basic_info.bit.rx_ampd5 = 1;
            if (net_cfg[i].AMPDU_RX & 0x40)
                cfg->basic_info.bit.rx_ampd6 = 1;
            if (net_cfg[i].AMPDU_RX & 0x80)
                cfg->basic_info.bit.rx_ampd7 = 1;
        }

        if (net_cfg[i].MPDU_SPACING)
            cfg->mpdu_spacing = net_cfg[i].MPDU_SPACING;
/*
        switch(net_cfg[i].AMPDU_LEN)
        {
            case MAX_8191:
                cfg->max_ampdu_len = 0;
                break;
            case MAX_16383:
                cfg->max_ampdu_len = 1;
                break;
            case MAX_32767:
                cfg->max_ampdu_len = 2;
                break;
            case MAX_65535:
                cfg->max_ampdu_len = 3;
                break;                
            default:
                cfg->max_ampdu_len = 3;
                break;
        }
*/
        DBG("    U-APSD bitmap %x\n", net_cfg[i].U_APSD);   
        cfg->u_apsd_bitmap = net_cfg[i].U_APSD;
    }

    DBG("3rd phase\n");
    for (i=0;i<NET_CFG_ENTRIES;i++)
    {
        if ((net_cfg[i].BSS >= MAX_BSSIDS) && (net_cfg[i].ESS == 0))
        {
            if (net_cfg[i].CAP & __AP)
            {
                /* WDS-AP, already handled in first phase */
            }
            else
            {
                DBG("Configuring WDS-STA %d, sta. table index %d\n", net_cfg[i].STA_NUM, sta_tbl_index);
                DBG("    ESS-%d  BSS-%d\n", net_cfg[i].ESS, net_cfg[i].BSS);

                printf("    This STA shall be created for simulation purpose only\n");

                sta_id = sta_tbl_index;
                net_cfg[i].sta_id = sta_id;
                sta_tbl_index++;

                cfg = &__sta_cfg[sta_id];

                cfg->foreign_sta = 1;

                for (j=0;j<MAX_AP_COUNT;j++)
                {
                    if (ap_info[j].BSS==net_cfg[i].BSS)
                    {
                        DBG("    Under WDS peer %d, sta table index %d\n", j, ap_info[j].addr_index);
                        cfg->addr_index = ap_info[j].addr_index;
                        cfg->basic_info.bit.bssid = ap_info[j].bssid;

                        //cfg->basic_info.bit.vld = 1;
                        //cfg->basic_info.bit.under_ap = 1;
                        break;
                    }
                }

                memcpy(cfg->RA_ADDR, net_cfg[i].MAC_ADDR, ETH_ALEN);
            }
        }
    }

    for (i=0;i<NET_CFG_ENTRIES;i++)
    {
        station_config = ((net_cfg[i].BSSID << 16) | net_cfg[i].STA_NUM);

        if (net_cfg[i].CAP & __QOS)
            station_config |= COSIM_STA_CONFIG_QOS;

        if (net_cfg[i].CAP & __DUT)
            station_config |= COSIM_STA_CONFIG_DUT;

        if (net_cfg[i].CAP & __AP)
            station_config |= COSIM_STA_CONFIG_MODE_LEGACY_AP;
        if (net_cfg[i].CAP & __P2PGO)
            station_config |= COSIM_STA_CONFIG_MODE_P2PGO;
        if (net_cfg[i].CAP & __IBSS)
            station_config |= COSIM_STA_CONFIG_MODE_IBSS;
        else
            station_config |= COSIM_STA_CONFIG_MODE_CLIENT;

        station_config1 = ((net_cfg[i].AMPDU_RX << 16) | net_cfg[i].AMPDU_TX);
        printf("COSIM_STACFG_OPERATION_ADD = %d, station_config = %08x, station_config1 = %08x\n", 
               COSIM_STACFG_OPERATION_ADD, station_config, station_config1);
        cosim_station_config(COSIM_STACFG_OPERATION_ADD, station_config, station_config1, net_cfg[i].MAC_ADDR);
    }

    return 1;
}
#endif

int mac_addr_lookup_engine_flush(void)
{
    MACDBG("flush_mac_addr_lookup_engine\n");

    /* flush lookup entries */
    MACREG_WRITE32(LUT_CMD, LU_TRIGGER| LU_FLUSH_STA_TABLE);

    MACREG_POLL32(LUT_CMD, LU_DONE, LU_DONE);

    MACREG_WRITE32(LUT_CMD, LU_TRIGGER| LU_FLUSH_DS_TABLE);

    MACREG_POLL32(LUT_CMD, LU_DONE, LU_DONE);

    MACDBG("flush_mac_addr_lookup_engine...Done.\n");

    return 0;
}

int reset_mac_registers(void)
{
    mac_addr_lookup_engine_flush();

    MACREG_WRITE32(MAC_INT_MASK_REG, 0x0000ffff);       /* mask all interrupts */
    MACREG_WRITE32(MAC_INT_CLR, 0x0000ffff);       /* clear all interrupt status  */

    return 0;
}

int set_mac_info_parameters(MAC_INFO* info)
{
    info->sta_tbl_count = MAX_STA_NUM;

    //info->rate_tbl_count = MAX_STA_NUM;

    info->buffer_hdr_count = BUFFER_HEADER_POOL_SIZE;

    info->beacon_q_buffer_hdr_count = BEACON_Q_BUFFER_HEADER_POOL_SIZE;

    /* the swpool_rx_buffer_hdr_count should be less then swpool_buffer_hdr_count 
       it is the value to chain linklist in SW buffer header pool for RX path
     */
    info->rx_buffer_hdr_count = info->buffer_hdr_count / 2;  

    info->rx_descr_count = RX_DESCRIPTOR_COUNT;
    info->beacon_tx_descr_count = BEACON_TX_DESCRIPTOR_COUNT;

    info->tx_descr_index = 0;
    info->rx_descr_index = 0;

    info->fastpath_bufsize = DEF_BUF_SIZE;

    info->sw_path_bufsize = DEF_BUF_SIZE;

    return 0;
}

#define MAX_DATA_FRAME_HEADERS_COUNT  (BUFFER_HEADER_POOL_SIZE+BEACON_Q_BUFFER_HEADER_POOL_SIZE)
buf_header __buf_header[MAX_DATA_FRAME_HEADERS_COUNT]; 
buf_header* alloc_buf_headers(int count)
{
    static int i = 0;
    buf_header *alloc;

    if ((i+count) > MAX_DATA_FRAME_HEADERS_COUNT)
    {
        ASSERT(0, "alloc_buf_headers failed\n");
    }

    alloc = &__buf_header[i];

    i += count;

    return(buf_header *)UNCACHED_ADDR(alloc);
}

#if defined(PACKET_BUFFER_IN_QRAM)
u8 __packet_buffer[BUFFER_HEADER_POOL_SIZE][DEF_BUF_SIZE] IN_QRAM __attribute__((aligned(512)));
#else
u8 __packet_buffer[BUFFER_HEADER_POOL_SIZE][DEF_BUF_SIZE] IN_SRAM __attribute__((aligned(512)));
#endif

void *buf_alloc(void)
{
    void *ptr = NULL;
    static int alloc_idx = 0;

    if (alloc_idx >= BUFFER_HEADER_POOL_SIZE)
        cosim_panic("Out of memory");

    ptr = &__packet_buffer[alloc_idx];

    alloc_idx++;

    return ptr;
}

int setup_buf_headers(MAC_INFO* info)
{
    int i;

    /* 
        chain buffer headers from element[0] to element[info->hwpool_buffer_hdr_count-1]
        this linklist is used for HW as buffer to store & forward frame in fastpath         
     */
#if 0
    ASSERT(info->hwpool_buffer_hdr_count, "zero info->hwpool_buffer_hdr_count\n");

    for (i=0;i<info->hwpool_buffer_hdr_count;i++)
    {
        info->hwpool_buf_headers[i].next_index = i + 1;
        info->hwpool_buf_headers[i].offset = 0;
        info->hwpool_buf_headers[i].ep = 0;

        /* allocate buffer referenced by this buffer header */
        info->hwpool_buf_headers[i].dptr = (u32) PHYSICAL_ADDR(mac_malloc(info, info->fastpath_bufsize, MAC_MALLOC_UNCACHED));
        //MACDBG("hwpool buffer %d, data ptr %x\n", i, info->hwpool_buf_headers[i].dptr);
    }
    info->hwpool_buf_headers[i-1].next_index = 0;
    info->hwpool_buf_headers[i-1].ep = 1;
#endif

    /* 
        chain buffer headers from element[0] to element[info->rx_buffer_hdr_count-1]
        this linklist is used for HW to store receive frame (RX to software path)           
     */
    ASSERT(info->rx_buffer_hdr_count, "zero info->rx_buffer_hdr_count\n");
    for (i=0;i<info->rx_buffer_hdr_count;i++)
    {
        info->buf_headers[i].next_index = i + 1;
        info->buf_headers[i].offset = 0;
        info->buf_headers[i].ep = 0;

        /* allocate buffer referenced by this buffer header */
        info->buf_headers[i].dptr = (u32) PHYSICAL_ADDR(buf_alloc());
        //MACDBG("swpool buffer %d, data ptr %x\n", i, info->buf_headers[i].dptr);
    }
    info->buf_headers[i-1].next_index = 0;
    info->buf_headers[i-1].ep = 1;

    info->rx_freelist_head_index = 0;
    info->rx_freelist_tail_index = i-1;

    /* 
        chain buffer headers from element[info->rx_buffer_hdr_count] to  element[buffer_hdr_count-1]
        this linklist is used as freelist for SW to allocate buffer headers in TX path
     */
    ASSERT(info->buffer_hdr_count > info->rx_buffer_hdr_count, "no free buffer headers available for TX freelist\n");
    for (i=info->rx_buffer_hdr_count;i<info->buffer_hdr_count;i++)
    {
        info->buf_headers[i].next_index = i + 1;
        info->buf_headers[i].offset = 0;
        info->buf_headers[i].ep = 0;

        /* allocate buffer for simulation module to use */
        info->buf_headers[i].dptr = (u32) PHYSICAL_ADDR(buf_alloc());
        //MACDBG("swpool buffer %d, data ptr %x\n", i, info->buf_headers[i].dptr);
    }
    info->buf_headers[i-1].next_index = 0;
    info->buf_headers[i-1].ep = 1;

    info->sw_tx_bhdr_head = info->rx_buffer_hdr_count;
    info->sw_tx_bhdr_tail = i-1;

    return 0;
}

int init_buf_headers(MAC_INFO* info)
{
    info->buf_headers = alloc_buf_headers(info->buffer_hdr_count);
    info->beacon_q_buf_headers = alloc_buf_headers(info->beacon_q_buffer_hdr_count);
    //info->hwpool_buf_headers = alloc_buf_headers(info->hwpool_buffer_hdr_count);

    ASSERT(info->buf_headers, "NULL info->buf_headers\n");
    ASSERT(info->beacon_q_buf_headers, "NULL info->hwpool_buf_headers\n");
    //ASSERT(info->hwpool_buf_headers, "NULL info->hwpool_buf_headers\n");

    setup_buf_headers(info);

    return 0;
}

rx_descriptor __rx_descriptor[RX_DESCRIPTOR_COUNT] __attribute__ ((aligned (16)));
rx_descriptor* alloc_rx_descriptors(int count)
{
    rx_descriptor* descr;
    static int i = 0;

    if (i >= RX_DESCRIPTOR_COUNT)
        return NULL;

    descr = &__rx_descriptor[i];
    i += count;

    return(rx_descriptor*) UNCACHED_ADDR(descr);
}

/* alloc & init TX/RX descriptors */
int init_transmit_descriptors(MAC_INFO* info)
{
    int i;

    /* init rx descriptors pool */

    info->rx_descriptors = alloc_rx_descriptors(info->rx_descr_count);
    ASSERT(info->rx_descriptors, "alloc_rx_descriptors failed\n");

    /* assign ownership of all RX descriptors to hardware */
    for (i=0;i<info->rx_descr_count;i++)
    {
        info->rx_descriptors[i].own = 1;
        info->rx_descriptors[i].eor = 0;
    }
    info->rx_descriptors[i-1].eor = 1;      /* indicate the HW it is the last descriptors */

    return 0;
}


u8 __ext_sta_table[MAX_STA_NUM][10] __attribute__ ((aligned (16)));
u8 __ext_ds_table[MAX_DS_NUM][10] __attribute__ ((aligned (16)));
int mac_program_addr_lookup_engine(MAC_INFO* info)
{
    int i;
    int sta_idx = 0;
    printf("mac_program_addr_lookup_engine\n");
    sta_cfg* sta_tbl = load_station_config();

    if (0==MACREG_READ32(EXT_STA_TABLE_ADDR))
        MACREG_WRITE32(EXT_STA_TABLE_ADDR, PHYSICAL_ADDR(__ext_sta_table));

    if (0==MACREG_READ32(EXT_DS_TABLE_ADDR))
        MACREG_WRITE32(EXT_DS_TABLE_ADDR, PHYSICAL_ADDR(__ext_ds_table));

    MACREG_UPDATE32(STA_DS_TABLE_CFG, STA_DS_TABLE_CFG_DONE, STA_DS_TABLE_CFG_DONE);

    /* program the lookup table */
    for (i=0; i<info->sta_tbl_count; i++)
    {
        if (sta_tbl[i].basic_info.bit.vld)
        {
            if (!sta_tbl[i].ap)
            {
                MACDBG("Program station %d into lookup table\n", i);

                SIMDBG("ADDR ", (u32) &sta_tbl[i].basic_info.val);
                MACREG_WRITE32(LUBASIC_CAP, ((sta_tbl[i].basic_info.val << 8)) | sta_tbl[i].addr_index);       /* [31:8] is basic capability, [7:0] is lookup index */

                MACDBG("Program station %d LUBASIC_CAP %x\n", i, (((sta_tbl[i].basic_info.val << 8)) | i));

                MACREG_WRITE32(LUT_ADDR0, ((sta_tbl[i].ADDR[0] << 8) | (sta_tbl[i].ADDR[1])));
                MACREG_WRITE32(LUT_ADDR1, ((sta_tbl[i].ADDR[2] << 24) | (sta_tbl[i].ADDR[3]  << 16)
                                           | (sta_tbl[i].ADDR[4] << 8) | (sta_tbl[i].ADDR[5])));
                //orig code MACREG_WRITE32(LUT_CMD, LU_TRIGGER| LU_CMD_INSERT_INTO_STA_BY_ADDR);
                MACREG_WRITE32(LUT_CMD, LU_TRIGGER| LU_CMD_SEL_STA_TABLE | LU_CMD_INSERT_INTO_STA_BY_ADDR);
#if defined(LINUX_I386)
                MACREG_POLL32(LUT_CMD, LU_DONE, LU_DONE);
#else
                while (0 == (MACREG_READ32(LUT_CMD) & LU_DONE));
#endif

                sta_idx++;
            }
            else
            {
                MACDBG("Program DS %d into lookup table %d\n", i, sta_tbl[i].ap_idx);

                SIMDBG("ADDR ", (u32) &sta_tbl[i].basic_info.val);
                MACREG_WRITE32(LUBASIC_CAP, ((sta_tbl[i].basic_info.val << 8)) | i);       /* [31:8] is basic capability, [7:0] is lookup index */

                MACDBG("Program DS %d LUBASIC_CAP %x\n", i, (((sta_tbl[i].basic_info.val << 8)) | sta_tbl[i].addr_index));

                MACREG_WRITE32(LUT_ADDR0, ((sta_tbl[i].ADDR[0] << 8) | (sta_tbl[i].ADDR[1])));
                MACREG_WRITE32(LUT_ADDR1, ((sta_tbl[i].ADDR[2] << 24) | (sta_tbl[i].ADDR[3]  << 16)
                                           | (sta_tbl[i].ADDR[4] << 8) | (sta_tbl[i].ADDR[5])));

                MACREG_WRITE32(LUT_CMD, LU_TRIGGER| (sta_tbl[i].ap_idx << 5)| LU_CMD_UPDATE_DS_BY_INDEX);
#if defined(LINUX_I386)
                MACREG_POLL32(LUT_CMD, LU_DONE, LU_DONE);
#else
                while (0 == (MACREG_READ32(LUT_CMD) & LU_DONE));
#endif

                SIMDBG("LUR_INDX_ADDR_DS ", (u32) MACREG_READ32(LUR_INDX_ADDR));
            }
        }
    }

    /* set max sta/ds table max. search */
    MACREG_UPDATE32(STA_DS_TABLE_CFG, (ap_idx << 9) | (sta_idx), STA_DS_TABLE_MAX_DS_SEARCH | STA_DS_TABLE_MAX_STA_SEARCH);

    /* for Lynx IBSS support: program IBSS BSSID into DS table */
    for (i=0;i<MAX_BSSIDS;i++)
    {
        sta_basic_info basic_info;

        if (bss_info[i].dut_role == ROLE_IBSS)
        {
            basic_info.val = 0;
            basic_info.bit.vld = 1;
            basic_info.bit.bssid = i;

            MACREG_WRITE32(LUBASIC_CAP, ((basic_info.val << 8)));

            MACREG_WRITE32(LUT_ADDR0, ((bss_info[i].BSSID[0] << 8) | (bss_info[i].BSSID[1])));
            MACREG_WRITE32(LUT_ADDR1, ((bss_info[i].BSSID[2] << 24) | (bss_info[i].BSSID[3]  << 16)
                                       | (bss_info[i].BSSID[4] << 8) | (bss_info[i].BSSID[5])));

            MACREG_WRITE32(LUT_CMD, LU_TRIGGER| (bss_info[i].ap_dstable_idx << 5)| LU_CMD_UPDATE_DS_BY_INDEX);
#if defined(LINUX_I386)
            MACREG_POLL32(LUT_CMD, LU_DONE, LU_DONE);
#else
            while (0 == (MACREG_READ32(LUT_CMD) & LU_DONE));
#endif
        }
    }

    return 0;
}

void mac_program_bssid(u8 *macaddr, int bss_idx, int role, int timer_index)
{
    u32 reg_val = 0;
    u32 reg2_val;

    switch (role)
    {
        case ROLE_STA:
            reg_val = (BSSID_INFO_EN |  BSSID_INFO_MODE_STA);
            break;
        case ROLE_AP:
            reg_val = (BSSID_INFO_EN |  BSSID_INFO_MODE_AP);
            break;
        case ROLE_IBSS:
            reg_val = (BSSID_INFO_EN |  BSSID_INFO_MODE_IBSS);
            break;
        case ROLE_P2PC:
            reg_val = (BSSID_INFO_EN |  BSSID_INFO_MODE_STA | BSSID_INFO_P2P);
            break;
        case ROLE_P2PGO:
            reg_val = (BSSID_INFO_EN |  BSSID_INFO_MODE_AP | BSSID_INFO_P2P);
            break;
        default:
            printf("BSS(%d) not enabled\n", bss_idx);
            reg_val = 0;
            break;
    }

    reg_val |= (((timer_index << 24) & BSSID_INFO_TIMER_INDEX) | ((macaddr[0] << 8) | macaddr[1]));
    reg2_val = ((macaddr[2] << 24) | (macaddr[3] << 16) | (macaddr[4] << 8) | macaddr[5]);

    if (bss_idx==0)
    {
        MACREG_WRITE32(BSSID0_INFO2, reg2_val);
        MACREG_WRITE32(BSSID0_INFO, reg_val);
    }
    else if (bss_idx==1)
    {
        MACREG_WRITE32(BSSID1_INFO2, reg2_val);
        MACREG_WRITE32(BSSID1_INFO, reg_val);
    }
    else if (bss_idx==2)
    {
        MACREG_WRITE32(BSSID2_INFO2, reg2_val);
        MACREG_WRITE32(BSSID2_INFO, reg_val);
    }
}

int mac_program_bssids(MAC_INFO* info)
{
    int i;

    for (i=0;i<MAX_BSSIDS;i++)
    {
        mac_program_bssid(bss_info[i].MAC_ADDR, i, bss_info[i].dut_role, bss_info[i].timer_index);
    }

    return 0;
}

int program_mac_registers(MAC_INFO* info)
{
    MACREG_WRITE32(SEC_GPKEY_BA, PHYSICAL_ADDR(info->group_keys));

    MACREG_WRITE32(SEC_PVKEY_BA, PHYSICAL_ADDR(info->private_keys));

    MACREG_WRITE32(CAPT_SIZE, CAPT_SIZE_64_BYTES);

    MACREG_WRITE32(SLOTTIME_SET, 0x0205);

    // MACREG_WRITE32(TXDSC_BADDR, PHYSICAL_ADDR(info->tx_descriptors));
    MACREG_WRITE32(RXDSC_BADDR, PHYSICAL_ADDR(info->rx_descriptors));

    /* Set WIFI/Ethernet MAX Length to Maximum */
    MACREG_WRITE32(MAX_WIFI_LEN,0xffff);
    MACREG_WRITE32(MAX_ETHER_LEN,0xffff);

    MACREG_WRITE32(RX_MPDU_MAXSIZE, 4096);

    // MACREG_WRITE32(FRQ_BADDR, PHYSICAL_ADDR(info->fp_returned_tx_descriptors));
    // MACREG_WRITE32(SRQ_BADDR, PHYSICAL_ADDR(info->sw_returned_tx_descriptors));

    MACREG_WRITE32(SWBL_BADDR, PHYSICAL_ADDR(info->buf_headers));

    // MACREG_WRITE32(BEACON_SWBL_BADDR, PHYSICAL_ADDR(info->buf_headers));

    //MACREG_WRITE32(HWBL_BADDR, PHYSICAL_ADDR(info->hwpool_buf_headers));

    /* the link list head & tail index are simply ( 0  , total element of buffer headers - 1 ) */
    // MACREG_WRITE32(HWB_HT, (((u32) 0x0000 << 16)|((u16)info->hwpool_buffer_hdr_count - 1)));

    /* 
        the link list head & tail index for SW path are ( 0  , total element for RX buffer headers - 1 ) 
     */
    MACREG_WRITE32(SWB_HT, (((u32) 0x0000 << 16)|((u16)info->rx_buffer_hdr_count - 1)));


    /*
        Set the buffer sizes of hwpool & swpool
     */
    MACREG_WRITE32(RX_BLOCK_SIZE, (info->sw_path_bufsize << 16 ) | info->fastpath_bufsize);
    MACREG_WRITE32(BUFF_POOL_CNTL, 1);

    /* Set LLC offset to 80 bytes to prevent wifi header corruptted by packet SN, RSC */
    MACREG_WRITE32(MAC_LLC_OFFSET, 0x50);

    MACREG_WRITE32(RX_ACK_POLICY, RX_ACK_POLICY_AMPDU_BA|RX_PLCP_LEN_CHK|RX_AMPDU_REORDER_ENABLE);

    MACREG_WRITE32(ERR_EN, 0xf3);  // Enable WIFI RX fast path 

    MACREG_UPDATE32(BEACON_SETTING2, SW_CONTROL_SENDBAR_RATE , SW_CONTROL_SENDBAR_RATE); //Enable SW decide rate of BAR packet

    MACREG_UPDATE32(RTSCTS_SET, LMAC_RTS_CTS_THRESHOLD, LMAC_RTS_CTS_THRESHOLD);

    MACREG_UPDATE32(SECENG_CTRL, SECENG_CTRL_REKEY_ERR_DROP, SECENG_CTRL_GKEY_RSC_ERR_DROP|SECENG_CTRL_KEYID_ERR_DROP|SECENG_CTRL_REKEY_ERR_DROP);

    MACREG_UPDATE32(SW_DES_NO, PKT_DES_NO, PKT_DES_NO); // Switch rx pkt descr to length 40 for bb team debug

    panther_throughput_config();

    return 0;
}

void disable_sniffer_mode(void)
{
    //dragonite_mac_lock();

    MACREG_UPDATE32(RTSCTS_SET, 0, LMAC_FILTER_ALL_PASS);
    MACREG_UPDATE32(RX_ACK_POLICY, 0, RX_SNIFFER_MODE);

    /* restore ERR_EN to default value */
    MACREG_WRITE32(ERR_EN, FASTPATH_WIFI_TO_WIFI | ERR_EN_SEC_MISMATCH_TOCPU | ERR_EN_TID_ERROR_TOCPU | ERR_EN_BSSID_CONS_ERROR_TOCPU
                   | ERR_EN_MANGMENT_TA_MISS_TOCPU | ERR_EN_DATA_TA_MISS_TOCPU);
    /* restore MAC_RX_FILTER to default value */
    MACREG_WRITE32(MAC_RX_FILTER, RXF_GC_MGT_TA_HIT | RXF_GC_DAT_TA_HIT | RXF_UC_MGT_RA_HIT | RXF_UC_DAT_RA_HIT
                   | RXF_BEACON_ALL | RXF_PROBE_REQ_ALL);

    bb_register_write(2, 0x5, 0x3);

    //dragonite_mac_unlock();
}

void enable_sniffer_mode(void)
{
    //dragonite_mac_lock();

    MACREG_UPDATE32(RTSCTS_SET, LMAC_FILTER_ALL_PASS, LMAC_FILTER_ALL_PASS);
    MACREG_UPDATE32(RX_ACK_POLICY, RX_SNIFFER_MODE, RX_SNIFFER_MODE);

    MACREG_UPDATE32(ERR_EN, 0, ERR_EN_FCS_ERROR_TOCPU);
    MACREG_UPDATE32(ERR_EN, ERR_EN_BA_SESSION_MISMATCH, ERR_EN_BA_SESSION_MISMATCH);

    MACREG_UPDATE32(MAC_RX_FILTER, RXF_UC_DAT_ALL |RXF_GC_DAT_ALL, RXF_UC_DAT_TA_RA_HIT | RXF_GC_DAT_TA_HS_HIT);

#if defined(HANDLE_ANDROID_PROBE_REQUEST)
    MACREG_UPDATE32(MAC_RX_FILTER, RXF_PROBE_REQ_ALL, RXF_PROBE_REQ);
#endif

    bb_register_write(2, 0x5, 0xb);

    //dragonite_mac_unlock();
}

int mac_program_bss_sta_cfg(MAC_INFO* info)
{
    int i;
    u32 bss0_sta_bitmap = 0;
    u32 bss1_sta_bitmap = 0;
    u32 bss2_sta_bitmap = 0;
    sta_cfg* sta_tbl = load_station_config();

    for (i=0; i<info->sta_tbl_count; i++)
    {
        if (sta_tbl[i].basic_info.bit.vld)
        {
            switch ((sta_tbl[i].bssid & 0x03))
            {
                case 0:
                    bss0_sta_bitmap |= (0x01UL << i);
                    break;
                case 1:
                    bss1_sta_bitmap |= (0x01UL << i);
                    break;
                case 2:
                    bss2_sta_bitmap |= (0x01UL << i);
                    break;
                default:
                    break;
            }
        }
    }

    MACREG_WRITE32(BSS0_STA_BITMAP, bss0_sta_bitmap);
    MACREG_WRITE32(BSS1_STA_BITMAP, bss1_sta_bitmap);
    MACREG_WRITE32(BSS2_STA_BITMAP, bss2_sta_bitmap);

    return 0;
}

int enable_mac_interrupts(void)
{
    //MACREG_WRITE32(MAC_INT_MASK_REG, 0x00000001UL);            /* unmask all interrupts, except pin 0 */ 

#if defined(WLA_TEST)
    if(boot3_submode == RECOVER_SUBMODE)
    {
        MACREG_WRITE32(MAC_INT_MASK_REG, ~(MAC_INT_TSF|MAC_INT_SW_RX));
    }
    else
        /* default disable RX intr for wla test rx */
        MACREG_WRITE32(MAC_INT_MASK_REG, ~(MAC_INT_TSF));
#else
    /* only enable TSF/TX/RX intr */
    MACREG_WRITE32(MAC_INT_MASK_REG, ~(MAC_INT_TSF|MAC_INT_SW_RX));
#endif

    return 0;
}

int arthur_mac_start(MAC_INFO* info)
{
    /* XXX: set fragment threshold, move somewhere else later */
    // MACREG_UPDATE32(TXFRAG_CNTL, 100, 0x0000ffffUL);

#if 0 // defined(REAL_BB)
    /*
        enable BB
     */
#if 0
    // 20M Setting
    MACREG_WRITE32(0x1000, 0x1UL); 
    MACREG_WRITE32(0x1000, 0x00100000UL);   //TX
    MACREG_WRITE32(0x1000, 0x00110000UL);   //RX 
#else
    // 40M Setting
    MACREG_WRITE32(0x1000, 0x1UL); 
    MACREG_WRITE32(0x10f0, 0x40020000UL); 
    MACREG_WRITE32(0x1000, 0x00100200UL);   //TX
    MACREG_WRITE32(0x1000, 0x00110200UL);   //RX
#endif
#endif

    /*
        enable TX

     */
    MACREG_WRITE32(UMAC_TX_CNTL, 0x43); /* XXX  fix this */


    /* 
        enable RX
     */
    MACREG_WRITE32(UPRX_EN, 1);

    /*
        enable LMAC timer
     */
    MACREG_UPDATE32(LMAC_CNTL, 0x1, LMAC_CNTL_TSTART);

    enable_mac_interrupts();

    return 0;
}

MAC_INFO* arthur_mac_init(void)
{
    static int initialled = 0;

    if (initialled)
        return 0;

    initialled = 1;

    info = &mac_info;

    reset_mac_registers();

    set_mac_info_parameters(&mac_info);

    //init_station_cap_tables(&mac_info);

    //init_rate_tables(&mac_info);

    init_buf_headers(&mac_info);

    init_transmit_descriptors(&mac_info);

    //init_ampdu_reorder_buffer(&mac_info);

    //init_psbaq_qinfo(&mac_info);

    mac_program_addr_lookup_engine(&mac_info);

//  mac_program_bssids(&mac_info);

    if (bootvars.mac0)
    {
        memcpy(MY_MAC_ADDR, bootvars.mac0, 6);
    }
    mac_program_bssid(MY_MAC_ADDR, 0 /* BSS0 */, ROLE_AP, 0 /* TSF0 */);

    /* we need first bssid number to setup key tables per bssid */
    //init_key_tables(&mac_info);

    program_mac_registers(&mac_info);

    mac_program_bss_sta_cfg(&mac_info);

    //ba_session_setup(&mac_info);

    /* post init the registers according to the configuration in config/post_init_registers */
//    post_init_registers();

    arthur_mac_start(&mac_info);

    return &mac_info;
}

void cosim_irq_exit(void)
{
#if defined(REAL_BB)
    return;
#endif

    COSIM_REG_WRITE32(COSIM_CPU_YIELD, 1);

#if 0
    int i;

    for (i=0;i<CPU_INST_EXEC_PER_NS / 2;i++)
    {
        __asm__ __volatile__ ("b.nop");
    }
#endif
}

u32 tx_acq_size_encoding(int size)
{
    u32 size_code = 0;

    switch (size)
    {
        case 8:
            size_code = 0;
            break;
        case 16:
            size_code = 1;
            break;
        case 32:
            size_code = 2;
            break;
        case 64:
            size_code = 3;
            break;
        default:
            cosim_panic("Unsupport ACQ WIN/QUEUE SIZE");
            break;
    }

    return size_code;
}

void tx_mixed_acq_setup(acq *acq, int acqid, int queue_size)
{
    u32 queue_size_code;

    memset((void *) acq, 0, sizeof(struct __tag_acq));
    acq->qid = acqid;

    queue_size_code = tx_acq_size_encoding(queue_size);

    acq->queue_size = queue_size;
    acq->queue_size_code = queue_size_code;
}

void lynx_tx_init(void)
{
    int i;

    for (i=0;i<ACQ_POOL_SIZE;i++)
    {
        acq_pool[i].next = &acq_pool[i+1];
    }
    acq_pool[ACQ_POOL_SIZE-1].next = NULL;

    for (i=0;i<ACQ_NUM;i++)
    {
        info->acq_hw_requested[i][0] = 0;
        info->acq_hw_requested[i][1] = 0;
    }

    info->def_acq = (acq *) UNCACHED_ADDR(def_acq);
    info->acq_free_list = &acq_pool[0];

    for (i=0;i<ACQ_NUM;i++)
    {
        tx_mixed_acq_setup(&info->def_acq[i], i, 64);
        info->def_acq[i].flags |= ACQ_FLAG_DEFAULT_Q;
    }
    info->def_acq[BCQ_QID].flags |= ACQ_FLAG_LOCK;

    // enable all 6 ACQs
    MACREG_WRITE32(ACQ_EN, 0x03f);

    // enable CMD0/CMD1 switch interrupt
    MACREG_UPDATE32(ACQ_INTRM, 0x0, ACQ_INTRM_CMD_SWITCH);

    // enable ACQ done interrupt
    MACREG_UPDATE32(ACQ_INTRM, 0x0, ACQ_INTRM_TX_DONE);

    // enable retry fail interrupt
    MACREG_UPDATE32(ACQ_INTRM2, 0x0, ACQ_INTRM2_RETRY_FAIL);

    // unmask WifiMAC ACQ INTR
    MACREG_UPDATE32(MAC_INT_MASK_REG, 0x0, (MAC_INT_ACQ_TX_DONE | MAC_INT_ACQ_CMD_SWITCH | MAC_INT_ACQ_RETRY_FAIL));
}

/* Add interrupt handler */ 
int int_add(unsigned long vect, void (* handler)(void))
{
    request_irq(vect, handler, (void *) vect);
    return 0;
}
/* Delete interrupt handler */
void int_del(unsigned long vect)
{
    disable_irq(vect);
}

void lynx_acq_intr_handler()
{
    int acqid, cmdid;
    acq *q;
    //acq *nextq;
    //volatile acq *qtmp;
    volatile acqe* _acqe;
    u32 tx_acq_status, tx_acq_status2;
    //u32 acq_info;
    u32 ssn;

    tx_acq_status = MACREG_READ32(ACQ_INTR);
    MACREG_WRITE32(ACQ_INTR, tx_acq_status);
    tx_acq_status2 = MACREG_READ32(ACQ_INTR2);
    MACREG_WRITE32(ACQ_INTR2, tx_acq_status2);
    //printf("\n\n========> ACQSTATUS %08x %08x\n", tx_acq_status, tx_acq_status2);

    for (acqid=0;acqid<ACQ_NUM;acqid++)
    {
        for (cmdid=0;cmdid<CMD_NUM;cmdid++)
        {
            if (tx_acq_status & ACQ_INTR_BIT(acqid,cmdid))
            {
                q = (acq *) (info->acq_hw_requested[acqid][cmdid]);

                if (q->queue_size==0)
                {
                    printf(" <<<<tx_acq_status %x>>>>\n", tx_acq_status);
                    cosim_panic("Wrong ACQ_INTR status");
                }

                ssn = MACREG_READ32(ACQ_INFO(acqid,cmdid)) & 0x0fff;
                while (q->rptr != ssn)
                {
                    //printf("\n\n====> SSN %08x SSN PLUS %08x\n", MACREG_READ32(ACQ_INFO(acqid,cmdid)), MACREG_READ32(ACQ_INFO3(acqid,cmdid)));

                    _acqe = (volatile acqe*)UNCACHED_ADDR(ACQE(q, q->rptr));

                    if (_acqe->m.own == 1)
                        break;

#if 0
                    printf(" rptr %d, bmap:%d ps:%d noa:%d try_cnt:%d bhr_h:%d\n", q->rptr, _acqe->m.bmap, _acqe->m.ps, _acqe->m.noa, _acqe->m.try_cnt, _acqe->m.bhr_h);
#endif
                    if (_acqe->m.try_cnt || _acqe->m.ps || _acqe->m.noa || (_acqe->m.bmap == 0))
                    {
                        //printf(" bmap:%d ps:%d noa:%d try_cnt:%d bhr_h:%d\n", _acqe->m.bmap, _acqe->m.ps, _acqe->m.noa, _acqe->m.try_cnt, _acqe->m.bhr_h);
                    }

                    if (_acqe->m.bhr_h == EMPTY_PKT_BUFH)
                    {
                        /* ignore empty packet TX done */
                    }
                    else
                    {
/*
                  if(_acqe->m.bmap==0)
                  {
                     lynx__acq_bmap_ps_noa_handler(_acqe);
                  }
*/
                        bhdr_insert_tail(info, _acqe->m.bhr_h, -1);
                    }

                    q->rptr++;
                    //printf("\n\n RPTR %x\n", q->rptr);

                }
                if (ACQ_EMPTY(q))
                {
                    q->flags &= ~ACQ_FLAG_ACTIVE;
                }
            }
        }
    }
    //printf("\n\n====>INTR END\n\n");
}

#if defined(WLA_TEST)
char *mgt_subtype_ntoa(u8 subtype)
{
    char *subtype_name;

    switch (subtype & 0x0f)
    {
        case 0x08:
            subtype_name = "beacon";
            break;
        case 0x05:
            subtype_name = "prob resp";
            break;
        case 0x04:
            subtype_name = "prob req";
            break;
        default:
            subtype_name = "";
            break;
    }
    return subtype_name;
}
#endif

#ifdef WLA_TEST
extern char *ether_ntoa(unsigned char *m);
char rx_packet_descr_parser_mptool(rx_packet_descriptor * rxpkt_descr, u32 data_offset)
{
    wifi_common_header *hdr = (wifi_common_header *)((unsigned char *)rxpkt_descr + data_offset);
    char *type, *subtype, *saddr;

    if (!(acfg_p->rx_dump & TEST_DUMP_SIMPLE))
    {
        return 0;
    }

    type = "";
    subtype = "";
    saddr = "";

    if (rxpkt_descr->wh)
    {
        switch (hdr->frame_ctrl.type & 0x3)
        {
            case 0x0:
                type = "MGT";
                subtype = mgt_subtype_ntoa(hdr->frame_ctrl.subtype);
                if (0 == (acfg_p->rx_filter & TEST_FILTER_MGT)
                    || ((0 == (acfg_p->rx_filter & TEST_FILTER_BEACON))&&(!strcmp(subtype, "beacon"))))
                {
                    goto drop;
                }
                break;
            case 0x1:
                if (0 == (acfg_p->rx_filter & TEST_FILTER_CTRL))
                {
                    goto drop;
                }
                type = "CTRL";
                break;
            case 0x2:
                if (0 == (acfg_p->rx_filter & TEST_FILTER_DATA))
                {
                    goto drop;
                }
                type = "DATA";
                if (rxpkt_descr->bc | rxpkt_descr->mc)
                {
                    if (0 == (acfg_p->rx_filter & TEST_FILTER_DATA_BC))
                    {
                        goto drop;
                    }
                    subtype = "BCAST";
                }
                else
                {
                    subtype = "UCAST";
                }
                break;
        }
        saddr = ether_ntoa(hdr->addr2);
    }
    else
    {
        if (0 == (acfg_p->rx_filter & TEST_FILTER_DATA))
        {
            goto drop;
        }
        type = "DATA";
        if (rxpkt_descr->bc | rxpkt_descr->mc)
        {
            if (0 == (acfg_p->rx_filter & TEST_FILTER_DATA_BC))
            {
                goto drop;
            }
            subtype = "BCAST";
        }
        else
        {
            subtype = "UCAST";
        }
        saddr = ether_ntoa(acfg_p->tx_addr);
    }
    printf("---- RX %s(%s) From:%s    RSSI:%d ----\n",
           type, subtype, saddr, bb_rssi_decode(rxpkt_descr->rssi, 0, NULL));

    return 0;
    drop:
    return 1;
}
#endif

struct rx_record_info
{
    u8      is_record;
	u8		addr[WLAN_ADDR_LEN];
	u16		seq_num;
    u8      retry;
} __attribute__ ((packed));

#define MAX_RX_RECORD_SIZE 8

int empty_rec_info_idx = -1;    // use to facilitate register progress

/*
 *  TODO: must add/modify function to handle situation that rx_rec_info is full of used
 */

struct rx_record_info rx_rec_info[MAX_RX_RECORD_SIZE];

/*
 * return -1 means not find match info, otherwhile will return index
 */
int find_match_rx_rec_info(u8 *addr)
{
    int i;
    int index = -1;
    struct rx_record_info *info;

    for (i = 0; i < MAX_RX_RECORD_SIZE; i++)
    {
        info = &rx_rec_info[i];

        if (!memcmp(info->addr, addr, WLAN_ADDR_LEN))
        {
            index = i;
            break;
        }
    }

    return index;
}

void dump_rx_rec_info(int index)
{
    struct rx_record_info *info = &rx_rec_info[index];

    printf("===================================================\n");
    printf("index = %d, seq_num = %d, retry = %d\n", index, info->seq_num, info->retry);
    printf("mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
           info->addr[0], info->addr[1], info->addr[2], info->addr[3], info->addr[4], info->addr[5]);
}

void dump_all_rx_rec_info(void)
{
    int i;

    for (i = 0; i < MAX_RX_RECORD_SIZE; i++)
    {
        dump_rx_rec_info(i);
    }
}

int unregister_rec_info(void)
{
    int index = -1;
    //struct rx_record_info *info;

    return index;
}

int register_rec_info(u8 *addr, rx_packet_descriptor * rxpkt_descr)
{
    int i = 0;
    int index = -1;
    struct rx_record_info *info;

    // find empty slot to register
    for (i = 0; i < MAX_RX_RECORD_SIZE; i++)
    {
        info = &rx_rec_info[i];

        if (info->is_record != 1)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        // not found any empty position
        return index;
    }

    info = &rx_rec_info[index];

    info->is_record = 1;
    memcpy(info->addr, addr, WLAN_ADDR_LEN);
    info->seq_num = rxpkt_descr->sn;
    info->retry = rxpkt_descr->retry;

    return index;
}

int update_rec_info(int index, rx_packet_descriptor * rxpkt_descr)
{
    int is_drop = 0;
    u32 seq_num;
    //u32 retry;
    struct rx_record_info *info = &rx_rec_info[index];

    // update sequence number and retry bit information first
    seq_num = info->seq_num;
    //retry = info->retry;
    info->seq_num = rxpkt_descr->sn;
    info->retry = rxpkt_descr->retry;

    // compare information between last and current packet to decide whether to drop packet,
    // although retry bit is recorded, but not use to check state
    if (seq_num == rxpkt_descr->sn)
    {
        MACDBG("Have the same seq_num: %d, mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
                seq_num, info->addr[0], info->addr[1], info->addr[2], info->addr[3], info->addr[4], info->addr[5]);
        is_drop = 1;
    }

    return is_drop;
}

int rx_packet_descr_parser_recover(rx_packet_descriptor * rxpkt_descr, u32 data_offset)
{
    int index;
    u8 *pos;
#ifdef DEBUG_WIFI
    u32 seq_num;
#endif
    wifi_common_header *hdr = (wifi_common_header *)((unsigned char *)rxpkt_descr + data_offset);

//  printf("sn: %d, retry: %d, retry2: %d\n", rxpkt_descr->sn, rxpkt_descr->retry, hdr->frame_ctrl.retry);
//  printf("frame_ctrl.type = %02x, frame_ctrl.subtype = %02x\n", hdr->frame_ctrl.type, hdr->frame_ctrl.subtype);

    // skip broadcast packet
    if (!memcmp(hdr->addr1, (void *) broadcast, WLAN_ADDR_LEN))
    {
        goto leave;
    }

    if ((hdr->frame_ctrl.type == 0x02))
    {
        // skip null data frame
        if ((hdr->frame_ctrl.subtype & 0x04) != 0)
        {
            goto leave;
        }

        pos = (u8 *)(hdr + 1);
        // skip invalid data frame
        if (pos[0] != 0xaa || pos[1] != 0xaa)
        {
            goto leave;
        }
    }

    index = find_match_rx_rec_info(hdr->addr2);

#ifdef DEBUG_WIFI
    seq_num = ((hdr->sequence >> 12) & 0xf) | (hdr->sequence & 0xff) << 4;

    printf("index = %d, sn = %d\n", index, seq_num);
    int i;
    u8 *p;
    u32 l;

    l = MIN(rxpkt_descr->packet_len, 40);

    p = (u8 *)hdr;
    for (i = 0; i < l; i++)
    {
        printf("%02x ", p[i]);
    }
    printf("\n\n");
#endif

    if (index == -1)
    {
        index = register_rec_info(hdr->addr2, rxpkt_descr);
    }
    else
    {
        if (update_rec_info(index, rxpkt_descr))
        {
            goto drop;
        }
    }

leave:
    return 0;
drop:
    return 1;
}

int rx_packet_descr_parser(rx_packet_descriptor * rxpkt_descr, u32 data_offset)
{
#if defined (WLA_TEST)
    if(boot3_submode == RECOVER_SUBMODE)
        return rx_packet_descr_parser_recover(rxpkt_descr, data_offset);
    else
        return rx_packet_descr_parser_mptool(rxpkt_descr, data_offset);
#else
    return rx_packet_descr_parser_recover(rxpkt_descr, data_offset);
#endif
}

#define SHOWING_FRAME_TYPE  printf

u8 rx_buf[2048];
int rx_frame_handler(u8 *rx_frame, u32 len)
{
    u16 ftype, stype;
    struct wlan_qoshdr *fm;
    u8 *dptr = (u8 *)UNCACHED_ADDR(rx_buf);

    memcpy(dptr, (u8 *)UNCACHED_ADDR(rx_frame), len);

    fm = (struct wlan_qoshdr *)dptr;
    ftype = ntohs(fm->fc) & WLAN_FC_FTYPE;
    stype = ntohs(fm->fc) & WLAN_FC_STYPE;

    if (ftype == WLAN_FC_TYPE_MGT)
    {
        switch (stype)
        {
            case WLAN_FC_SUBTYPE_BEACON:
                //SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_BEACON\n");
                break;
            case WLAN_FC_SUBTYPE_PROBE_RESP:
                SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_PROBE_RESP\n");
                break;
            case WLAN_FC_SUBTYPE_PROBE_REQ:
                //SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_PROBE_REQ\n");
                if (ap_handle_probe_req(dptr, len))
                {
                    printf("Handle probe request got failed.\n");
                }
                break;
        }

        switch (stype) {
            case WLAN_FC_SUBTYPE_AUTH:
                SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_AUTH\n");
                mlme_handle_auth(dptr, len);
    			goto exit;
            case WLAN_FC_SUBTYPE_ASSOC_REQ:
            case WLAN_FC_SUBTYPE_REASSOC_REQ:
                SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_ASSOC_REQ\n");
                ap_handle_assoc(dptr, len);
                goto exit;
    		case WLAN_FC_SUBTYPE_ASSOC_RESP:
            case WLAN_FC_SUBTYPE_REASSOC_RESP:
                SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_ASSOC_RESP\n");
//  			sta_handle_assoc();
    			goto exit;
    		case WLAN_FC_SUBTYPE_DISASSOC:
            case WLAN_FC_SUBTYPE_DEAUTH:
                SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_DISASSOC\n");
                // TODO
//  			sta_handle_deauth();
    			goto exit;
            case WLAN_FC_SUBTYPE_ACTION:
                SHOWING_FRAME_TYPE("WLAN_FC_SUBTYPE_ACTION\n");
//  			res = mlme_handle_action();
//  			wmac_update_ps_state(0);
                goto exit;
    		default:
    			goto exit;
		}
    }
    else if (ftype == WLAN_FC_TYPE_CTRL)
    {
        SHOWING_FRAME_TYPE("ftype == WLAN_FC_TYPE_CTRL\n");
//        mon_bit = RX_MON_CTRL;
//        if (sta == 0)
//            goto exit;
//        if (stype == WLAN_FC_SUBTYPE_BAR) {
//            struct wlan_ba_req *bar = (struct wlan_ba_req *)cur_wrx->fmhdr;
//            struct rx_ba_session *ba;
//            int idx;
//
//            //WLA_DBG(INFO, "RX: BAR\n");
//
//            idx = wtohs(bar->barctl) >> WLAN_BAR_TID_S;
//            if (sta->ba_rx_idx[idx] == 0) {
//                WLA_DBG(INFO, "NO BA exist???\n");
//                send_delba_request(sta, idx, 0, WLAN_REASON_UNSPECIFIED);
//#if defined(CONFIG_WLA_RX_REORDER_BUF)
//            } else {
//                ba = IDX_TO_RXBA_SESS(sta->ba_rx_idx[idx]-1);
//                reorder_buf_release(ba, wtohs(bar->barseqctl) >> WLAN_BAR_SEQ_START_S, 0);
//#endif
//            }
//        } else if (stype == WLAN_FC_SUBTYPE_PS_POLL) {
//            WLA_DBG(INFO, "RX: PS-POLL\n");
//            if (WB_SA_HIT_ADDR(cur_wrx->wb) == 0) {
//                /* UMAC may lose SA_HIT even if STA is exist. Try to lookup again. */
//                if (wmac_addr_lookup_engine_find(fm->addr2, 0, 0, 0) < 0) {
//                    /* Receives illege frame, we should deauth him to avoid it attacks me */
//                    resp_deauth_frame(cur_wrx->wb, fm);
//                }
//            } else {
//                wmac_update_ps_state(1);
//            }
//        } else {
//            WLA_DBG(WARN, "RX: ctrl frame, length=%d\n", WB_PKTLEN(wb));
//        }
//        goto exit;
	}
    else if (ftype == WLAN_FC_TYPE_DATA)
    {
        //SHOWING_FRAME_TYPE("ftype == WLAN_FC_TYPE_DATA\n");

        if (stype == WLAN_FC_SUBTYPE_DATA)
        {
            mlme_handle_data_frame(dptr, len);
        }
	}

exit:
    return 0;
}

int rx_count = 0;
int rx_phy_info_record = 0;
#ifdef CONFIG_ATE
extern unsigned int ate_rssi;
extern unsigned int ate_cnt;
#endif
int mac_intr(MAC_INFO *info)
{
    u32 status;
    int i;
    //int j;
    u16 hdr_index;
    buf_header *hdr;
    unsigned char *dptr;
    int skb_offset;
    //int offset;
    u32 ts_status;

    //int_disable(WIFI_MAC_INTR_NUM);
#if !defined(WLA_TEST)
    MACDBG("I'm mac_intr\n");
#endif
    status = MACREG_READ32(MAC_INT_STATUS);
    MACDBG("mac_intr status %x\n", status);
    MACREG_WRITE32(MAC_INT_CLR, status);     /* write 1 clear */

    if (status & MAC_INT_TSF)
    {
        MACDBG("status & MAC_INT_TSF\n");
        if ((ts_status = MACREG_READ32(TS_INT_STATUS)))
        {
            MACDBG("ts_status = TS_INT_STATUS\n");
            MACREG_WRITE32(TS_INT_STATUS, ts_status);

#if defined (WLA_TEST)
            if((boot3_submode == RECOVER_SUBMODE))
                beacon_kick();
            else
                if (--beacon_tx_count > 0)
#endif
                {
                    beacon_kick();
                }

            MACDBG("MACREG_READ32(BEACON_TX_STATUS) = 0x%x\n", MACREG_READ32(BEACON_TX_STATUS));
        }
    }

    i = info->rx_descr_index;
    while (info->rx_descriptors[i].own == 0)
    {
#if !defined(WLA_TEST)
        MACDBG("RX(%d) ", rx_count+1);
#endif

        MACDBG(" DESCR: ");
        dptr = (unsigned char *)&info->rx_descriptors[i];
        MACDBG("%08x ", *((u32 *)dptr));
        MACDBG("\n");
        int first_buffer = 1;

        hdr_index = info->rx_descriptors[i].frame_header_head_index;
        do
        {
            hdr = (buf_header *)&info->buf_headers[hdr_index];
            skb_offset = hdr->offset;
            hdr_index = hdr->next_index;

            dptr = (unsigned char *) UNCACHED_VIRTUAL_ADDR(((unsigned long)hdr->dptr));
            if (first_buffer)
            {
                rx_packet_descriptor *rxpkt_descr = (rx_packet_descriptor *)dptr;

                if(rx_phy_info_record)
                    store_rx_phy_info(rxpkt_descr->format, rxpkt_descr->snr_p1, rxpkt_descr->snr_p2, rxpkt_descr->rssi, 
                                      rxpkt_descr->rate, rxpkt_descr->cbw, primary_ch_offset(lapp->bandwidth));

                #ifdef CONFIG_ATE
                ate_rssi += rxpkt_descr->rssi;
                ate_cnt += 1;
                #endif

                if (rx_packet_descr_parser(rxpkt_descr, hdr->offset))
                {
                    continue;
                }

                first_buffer = 0;
                dptr += skb_offset;

#if 0
#if defined(WLA_TEST)
                if (acfg_p->rx_dump & TEST_DUMP_PAYLOAD)
#endif
                //if ((dptr[0]!=0x80) && (dptr[0]!=0x40))
                //if (dptr[10] == 0xD8 && dptr[11] == 0x50 && (dptr[0]!=0x40))    // 1x1
                //if (dptr[10] == 0xF8 && dptr[11] == 0xD1)                       // Nexus7
                {
                    printf("frame:");
                    for (j=0;j<hdr->len;j++)
                    {
                        if (0==(j%16))
                        {
                            printf("\n%08x: ", dptr + j);
                        }
                        printf("%02x ", dptr[j]);
                    }
                    printf("\n");
                }
#endif
            }

            if(boot3_submode == RECOVER_SUBMODE)
                rx_frame_handler(dptr, hdr->len - 4);   // cut 4 bytes because FCS length is 4
        } while (hdr->ep!=1);

        while (MACREG_READ32(MAC_FREE_CMD) & MAC_FREE_CMD_BUSY) SIMDBG("MAC_FREE_CMD_BUSY", 1);

        MACREG_WRITE32(MAC_FREE_PTR,
                       ((info->rx_descriptors[i].frame_header_head_index << 16)
                        | info->rx_descriptors[i].frame_header_tail_index));

        MACREG_WRITE32(MAC_FREE_CMD,  MAC_FREE_CMD_SW_BUF | MAC_FREE_CMD_KICK);

        info->rx_descriptors[i].own = 1;
        i = (i + 1) % info->rx_descr_count;
        info->rx_descr_index = i;

        MACREG_WRITE32(CPU_RD_SWDES_IND, 1);     /* kick RX queue logic as it might be pending on get free RX descriptor to write */

        continue;
    }

    /* handle SW TX return Queue */
    if (status & (MAC_INT_ACQ_TX_DONE | MAC_INT_ACQ_CMD_SWITCH | MAC_INT_ACQ_RETRY_FAIL))
    {
#if !defined(WLA_TEST)
        MACDBG("finish tx\n");
#endif
        lynx_acq_intr_handler();
    }

    //int_enable(WIFI_MAC_INTR_NUM);
    cosim_irq_exit();

    return 0;
}

void wifi_intr_handler(void)
{
    mac_intr(&mac_info);
}

void cosim_init(void)
{
// printf("reg 38d8=0x%x, reg 38dc=0x%x\n", (*(volatile unsigned int*)(0xbf0038D8)), (*(volatile unsigned int*)(0xbf0038DC)));
// printf("reg 38d8=0x%x, reg 38dc=0x%x\n", (*(volatile unsigned int*)(0xbf0038D8)), (*(volatile unsigned int*)(0xbf0038DC)));
    /* int_add will enable the INTR automatically */
    int_add(IRQ_WIFI, wifi_intr_handler);

//   int_add(HNAT_INTR_NUM, hnat_intr_handler);
//   int_add(ETH_PKT_COLLECT_INTR_NUM, eth_pkt_collect_handler);
//   int_add(ISS_TIMER_INTR_NUM, iss_timer_intr_handler);

    packet_queue_init();

#if 1 // AGGRESSIVE TX parameters for simulation speedup
    MACREG_WRITE32(CW_SET, 0x21212121UL);
    MACREG_WRITE32(CW_SET_BG, 0x00002100UL);
    MACREG_WRITE32(AIFS_SET, 0x11110001UL);
#endif

#if 1
    // 40Mhz channel with 11n20 & Legacy packet set to 20Mhz upper
    MACREG_UPDATE32(BASIC_SET, 0x02 << 9, LMAC_CH_BW_CTRL_CH_OFFSET);

    MACREG_UPDATE32(BASIC_SET, 0x02 << 6, BASIC_CHANNEL_OFFSET);

    MACREG_UPDATE32(BASIC_SET, LMAC_DECIDE_CH_OFFSET | LMAC_CCA1_EN | LMAC_DECIDE_CTRL_FR, LMAC_CH_BW_CTRL_AUTO_MASK);
#endif   
    lynx_tx_init();
}

void cosim_packet_inject(raw_packet *rawpkt)
{
#if defined(REAL_BB)
    return;
#endif

    COSIM_LOCK();

    COSIM_REG_WRITE32(INJECT_PKT_LEN, rawpkt->length);
    COSIM_REG_WRITE32(INJECT_PKT_TIME_L, (u32) rawpkt->timestamp);
    COSIM_REG_WRITE32(INJECT_PKT_TIME_H, (u32) (rawpkt->timestamp >> 32));
    COSIM_REG_WRITE32(INJECT_PKT_FLAGS, rawpkt->flags);
    COSIM_REG_WRITE32(INJECT_PKT_DURATION, rawpkt->duration);

    memcpy((void *) INJECT_PKT_DATA, rawpkt->data, rawpkt->length + 4 /* retry_crc */);

    COSIM_REG_WRITE32(COSIM_CONTROL, COSIM_CONTROL_PACKET_INJECT);

    COSIM_UNLOCK();
}

raw_packet rawpkt;
void RX_packet_inject_simply(int source, int target)
{
    //int i;
    printf("inject RX_packet\n");

    rawpkt.length = 108;
    rawpkt.timestamp = 0;
    rawpkt.flags = 0;
    rawpkt.duration = 0;
    memcpy(&rawpkt.data[0], &test_rx_packet[0], rawpkt.length + 4 /* retry_crc */);
#if 0
    if (target == 255)
    {

        printf("rawpkt->length = %d, rawpkt->timestamp = %08x, rawpkt->timestamp = %08x, rawpkt->flags = %08x, rawpkt->duration = %08x\n",
               rawpkt.length, rawpkt.timestamp, (rawpkt.timestamp>>32), rawpkt.flags, rawpkt.duration);
        for (i=0;i<rawpkt.length+4;i++)
        {
            printf("%02x ", rawpkt.data[i]);
        }
        printf("\n");
    }
#endif
    cosim_packet_inject(&rawpkt);

}

void cosim_set_test_passed(void)
{
#if defined(REAL_BB)
    return;
#endif

    COSIM_REG_WRITE32(COSIM_SET_TEST_PASSED, 1);
}

extern unsigned char txvga_gain[14];
void panther_set_channel(u32 ch, u32 bw)
{
    char pri_channel = CH_OFFSET_20;
	int freq = 2412;
	//int central_freq;

	printf("set channel to Ch_%d, bw %d\n", ch, bw);
	rf_set_40mhz_channel(ch, bw);
	freq = lrf_ch2freq(ch);

	bb_set_20mhz_mode(freq);

	MACREG_UPDATE32(BASIC_SET, pri_channel << 9, LMAC_CH_BW_CTRL_CH_OFFSET);
	MACREG_UPDATE32(BASIC_SET, pri_channel << 6, BASIC_CHANNEL_OFFSET);
	MACREG_UPDATE32(BASIC_SET, LMAC_DECIDE_CH_OFFSET | LMAC_DECIDE_CTRL_FR, LMAC_CH_BW_CTRL_AUTO_MASK);

	if(ch >= 1 && ch <=14)
		bb_set_tx_gain(txvga_gain[ch - 1]);
}

void beacon_init(void)
{
    init_beacon_settings();

    beacon_descriptor_setup(83);
    arthur_beacon_setup(100, 1);    /* beacon interval 3, DTIM interval 2 */
    arthur_beacon_start(0x20);    /* 1M */
}

extern int fem_en;
void beacon_tx(u32 ch, u32 pkt_num)
{
    beacon_tx_count = pkt_num;

    // set channel

#if defined (WLA_TEST)
    wmac_set_channel(ch, 0);
#else
    panther_set_channel(ch, 0);
#endif
    if(fem_en)
        panther_fem_config(fem_en);

    panther_channel_config(ch, fem_en);
}

int send_null_data(u32 pkt_num)
{
    int i = 0;
    int ret = 0;

    for (i = 0; i < pkt_num; i++)
    {
        TX_packet(&null_tx_packet[0], 64, NULL);

        udelay(1000000);
    }

    return ret;
}

void unicast_tx(u32 ch, u32 pkt_num)
{
    // set channel

#if defined (WLA_TEST)
    wmac_set_channel(ch, 0);
#else
    panther_set_channel(ch, 0);
#endif
    if(fem_en)
        panther_fem_config(fem_en);

    panther_channel_config(ch, fem_en);

    send_null_data(pkt_num);
}

int wifi_init(void)
{
//  tx_rates[0] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_7);
//  tx_rates[1] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_7);
//  tx_rates[2] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_7);
//  tx_rates[3] = tx_rate_encoding(FMT_HT_MIXED, CH_OFFSET_20, 7, 0, MCS_7);

    // customized tx rate
    tx_rates[0] = tx_rate_encoding(FMT_NONE_HT, CH_OFFSET_20, 2, 0, OFDM_54M);
    tx_rates[1] = tx_rate_encoding(FMT_NONE_HT, CH_OFFSET_20, 2, 0, OFDM_24M);
    tx_rates[2] = tx_rate_encoding(FMT_NONE_HT, CH_OFFSET_20, 2, 0, OFDM_6M);
    tx_rates[3] = tx_rate_encoding(FMT_11B, CH_OFFSET_20, 1, 0, CCK_1M);

    //printf("Hello World Main()\n");

    setup_network_configuration();
//  apply_network_configuration();

    arthur_mac_init();
    cosim_init();

#if defined (WLA_TEST)
    if(boot3_submode == RECOVER_SUBMODE)
    {
        beacon_init();
        mini_mlme_init();
    }
    wmac_set_channel(7, 0);
#else
    if(boot3_submode == RECOVER_SUBMODE)
    {
        beacon_init();
        mini_mlme_init();
    }
    panther_set_channel(7, 0);
#endif

    if(fem_en)
        panther_fem_config(fem_en);

    panther_channel_config(7, fem_en);

    printf("wifi init finish\n");

//  beacon_tx(7, 100);

//  unicast_tx(7, 100);

    return 0;
}

