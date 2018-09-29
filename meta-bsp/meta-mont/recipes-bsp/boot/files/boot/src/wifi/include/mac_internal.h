#ifndef __MAC_INTERNAL_H__
#define __MAC_INTERNAL_H__

#include "mac_common_sim.h"
#include "mac_tables.h"
#include "mac_regs.h"
//#include "mac_regs.h"
int wait_mac_intr(MAC_INFO* info);
sta_cfg* load_station_config(void);
cipher_key* alloc_group_key_tbls(void);
cipher_key* alloc_private_key_tbls(void);

buf_header* alloc_buf_headers(int count);
rx_descriptor* alloc_rx_descriptors(int count);
ssq_tx_descr* alloc_beacon_tx_descriptors(int count);

#if 0
void ___memcpy(u8 *dst, u8 *src, int length);
#endif





/* shared structure with pktgen */

#define FRAME_SIZE_ARRAY_SIZE   32

#define PKTGEN_PKTDESCR_TX_QUEUE_EDCA       0   /* default value */
#define PKTGEN_PKTDESCR_TX_QUEUE_BEACON     1
#define PKTGEN_PKTDESCR_TX_QUEUE_SOFTWARE_SCHEDULE  2   /* software schedule queue */

/* keep the size as multiples of 4 bytes, thus __attribute__((__packed__)) not used */
/* be sure it is always compiled with 32bit target GCC */
typedef struct
{
    tx_descriptor   tx_descr;
    u8      tx_descr_padding[16];   /* 8 bytes for beacon TX descr runtime padding */

    u16     pktgen_data_offset;


    u16     data_offset;        /* ofs field in buffer header */
    u16     fragment_threshold;
    u16     frame_size[FRAME_SIZE_ARRAY_SIZE];

    u16     tx_queue;           /* indicate which queue to be used for transmission */
    u16     frame_type;
    u8      padding_for_32bits_alignment[2];
}  pktgen_pktdescr;

#define MACADDR_LEN 6

typedef struct
{
    int max_framesize;          // frame data above this value will be seperated into several buffers
    int random_option;          // randomly pick the packet config entry for generating packets

    crypto_key bssid0_key;
    crypto_key bssid1_key;
    crypto_key bssid2_key;
} global_config;

extern global_config global_cfg;
/* enqueue direct forward frame into TX queue to transmission */

int soc_init(void);
int post_init_registers(void);

struct mactest_control
{
    int packet_num;
    int repeat;
    unsigned long test_mode;
};

#define TEST_MODE_ENABLE_TX         0x0001UL
#define TEST_MODE_ENABLE_RX         0x0002UL
#define TEST_MODE_RX_FORWARD        0x0004UL
#define TEST_MODE_RX_DUMP_BUFFER    0x0008UL
#define TEST_MODE_HNAT_ONLY         0x0010UL
#define TEST_MODE_RX_HANDLER_ONLY   0x0020UL

#endif // __MAC_INTERNAL_H__

