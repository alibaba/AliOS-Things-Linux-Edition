#ifndef __MAC_H__
#define __MAC_H__

#if defined(__KERNEL__)
#include <linux/spinlock.h>
#endif

#include "mac_common_sim.h"
#include "mac_tables.h"
#include "mac_regs.h"

typedef struct {
    int sta_tbl_count;
    //sta_cap_tbl* sta_cap_tbls;                      /* STA station capability tables */

    //int rate_tbl_count;
    //sta_cap_tbl* rate_tbls;                       /* STA station capability tables next page for rate selection */

    cipher_key* group_keys;                         /* pointer to group/def keys array */
    cipher_key* private_keys;                       /* pointer to private/pair keys array */

    int buffer_hdr_count;                  /* total number of buffer headers in SW pool */
    int rx_buffer_hdr_count;               /* total number of buffer headers in SW pool used for RX linklist */
    volatile buf_header *buf_headers;       /* array of allocated buffer headers; Software path pool (set base address to hardware)*/

    volatile int rx_freelist_head_index;                     /* index of free list head of buf_header for RX usage */
    volatile int rx_freelist_tail_index;
    volatile int sw_tx_bhdr_head;                     /* index of free list head of buf_header for TX usage */
    volatile int sw_tx_bhdr_tail;

    int beacon_q_buffer_hdr_count;                /* static headers for HW beacon queue */
    buf_header *beacon_q_buf_headers;
    int beacon_tx_descr_count; 

    int rx_descr_count;                             /* static allocated rx_descriptor count */
    volatile rx_descriptor *rx_descriptors;                  /* array of static allocated rx_descriptors */

    int tx_descr_index;                             /* next/first free tx descriptor index */
    int rx_descr_index;                             /* next unhandled rx descriptor index */

#define ACQ_NUM   6
#define CMD_NUM   2

#define ACQ_POOL_SIZE 32

    acq* def_acq;
    u32 acq_hw_requested[ACQ_NUM][CMD_NUM];
    volatile acq* acq_free_list;

    int fastpath_bufsize;                           /* frame buffer size on fastpath */
    int sw_path_bufsize;
} MAC_INFO;

#define TXDESCR_SIZE  sizeof(tx_descriptor)
extern MAC_INFO mac_info;
extern MAC_INFO *info;

int set_mac_info_parameters(MAC_INFO* info);
int init_station_cap_tables(MAC_INFO* info);
int init_key_tables(MAC_INFO* info);

int init_buf_headers(MAC_INFO* info);
int mac_set_pairwise_key(int sta_id, cipher_key *key);
int mac_set_group_key(int bss_idx, cipher_key *key);
void mac_invalidate_ampdu_scoreboard_cache(int sta_index, int tid);

buf_header* alloc_buf_headers(int count);

#define MAC_MALLOC_CACHED       0x00000000
#define MAC_MALLOC_UNCACHED     0x00000001
#define MAC_MALLOC_BZERO        0x00000002      /* bzero the allocated memory */
#define MAC_MALLOC_ATOMIC       0x00000004      /* use atomic memory pool     */

void *mac_alloc_skb_buffer(MAC_INFO* info, int size, int flags, u32* pskb);
void *buf_alloc(void);
void mac_free(void *ptr);
int init_transmit_descriptors(MAC_INFO* info);

MAC_INFO* arthur_mac_init(void);
int program_mac_registers(MAC_INFO* info);
int mac_program_bss_sta_cfg(MAC_INFO* info);
int mac_program_bssids(MAC_INFO* info);
int arthur_mac_start(MAC_INFO* info);
int arthur_mac_stop(MAC_INFO* info);

buf_header* mac_tx_freelist_get_first(MAC_INFO* info, int *index, int blocking);
int mac_tx_freelist_insert_tail(MAC_INFO* info, int index);
tx_descriptor* get_free_tx_descr(MAC_INFO* info, int blocking);
tx_descriptor* get_free_eth_tx_descr(MAC_INFO* info, int blocking);
int poll_eth_tx_return_descr(MAC_INFO* info, tx_descriptor* eth_tx_r_descr);
int mac_trigger_tx(void);
int mac_eth_trigger_tx(void);
int mac_ssq_trigger_tx(void);

int enable_mac_interrupts(void);
int disable_mac_interrupts(void);

#endif // __MAC_H__

