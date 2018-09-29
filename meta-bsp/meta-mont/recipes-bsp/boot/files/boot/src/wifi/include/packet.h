#ifndef __PACKET_H__
#define __PACKET_H__

#include <mac_tables.h>

struct __tag_mpdu;

int int_disable(unsigned long vect);
int int_enable(unsigned long vect);
#define PKT_LOCK()      sched_lock()
#define PKT_UNLOCK()    sched_unlock()

#define PKT_PANIC(message)

#define BB_HEADER_SIZE        16
//#define DESCR_SIZE            32
#define DESCR_SIZE            sizeof(rx_packet_descriptor)

//#define MAX_STA_NUM           256
#define MAX_RAW_MPDU_SIZE        8100   // A-MSDU + BB header + SECURITY + CRC
#define MAX_RAW_PACKET_SIZE      68000

#define PACKET_POOL_MEMORY_BASE 0x08000000
#define SYSTEM_MEMORY_SIZE      0x10000000

#ifndef DIRECTION_IN
#define DIRECTION_IN        0
#endif
#ifndef DIRECTION_OUT
#define DIRECTION_OUT       1
#endif

#ifndef ETH_ALEN
#define ETH_ALEN    6
#endif

#ifndef MAX_WIFI_HEADER_LEN
#define MAX_WIFI_HEADER_LEN     ( 2+2+6+6+6+2+6+2+4 )
#endif

struct __tag_mpdu;
typedef int (*packet_matched_callback)(struct __tag_mpdu *, struct __tag_mpdu *, struct __tag_mpdu *);
typedef int (*packet_unmatched_callback)(struct __tag_mpdu *, struct __tag_mpdu *);
typedef int (*packet_tx_done_callback)(u32 descr);
extern packet_unmatched_callback tx_unmatched_cb;
extern packet_unmatched_callback rx_unmatched_cb;

#define DOMAIN_WIFI        0
#define DOMAIN_ETHERNET    1

typedef struct __tag_mpdu
{
   unsigned char direction;   // OUT:TX,  IN:RX
   unsigned char qos;
   signed char tid;
   unsigned char domain;
   unsigned char ethernet_format;
   int llc_length;
   unsigned char llc[16];
   int ethernet_payload_length;
   unsigned short type;
   unsigned short subtype;
   unsigned short power_management;
   unsigned short seq_num;
   unsigned short total_fragments;
   unsigned short frag_num;
   unsigned short duration;
   int sta_id;      // TX: DA station id,  IN: SA station id
   int sta_num;
   int bss_idx;
   int plaintext_length;
   int encrypted_length;
   int header_length;         // this variable always indicates wifi header length, never stores ETH header length
   int payload_length;

   unsigned char bb_header[BB_HEADER_SIZE];
   unsigned char wifi_hdr[36];
   //unsigned char *payload;
   unsigned char eth_hdr[14];

   unsigned char plaintext_data[MAX_RAW_MPDU_SIZE];
   unsigned char encrypted_data[MAX_RAW_MPDU_SIZE];

   unsigned long descr[(DESCR_SIZE + 4)/4];
   unsigned char descr_valid;
   
   int length;
   int crc_ok;
   int ipcsok;
   int tcpcsok;
   int from_ds;
   int to_ds;
   int ta_idx;
   int ra_idx;
   int retry;
   int more_frag;

   int in_aggregation;
   int ampdu_len;
   int max_ampdu_len;
   unsigned char *data;
   unsigned char ignore;

   int ch_offset;
   int format;
   int sgi;
   int rate;
   int _11b_sp;

   u64 tsc[2];
   u64 packet_start;
   int broadcast;
   int multicast;
   int more_data;
   int eosp;
   int amsdu;
   int ps_returned;
   int noa_returned;
   int retry_failed;

   unsigned long flags;
   unsigned char retry_crc[4];

   packet_matched_callback matched_cb;    /* callback function invoked when the packet was matched, return 0 to free the packet */

   u8 rssi; /* set rssi in mpdu pkt*/
   u8 snr; /* set snr in mpdu pkt*/

   struct __tag_mpdu *prev, *next;
} mpdu;

/* 
   FLAGS for Model TX control  (DUT RX injection) 
   PKT_FLAG_NEED_RESPONSE  (default on): expected to have a response packet,  e.g. MPDU->ACK, A-MPDU->BACK, RTS->CTS
   PKT_FLAG_NORESP_RETRY   (default on): if a response packet is expected, but no recevied, redo the transmission
   PKT_FLAG_TOGGLE_RETRY_BIT (default on): toggle the retry-bit in the wifi header in the retransmission
 
   PKT_FLAG_AT_ABSOLUTE_TIME (default off): try to transmit/inject the packet at the specific simulation time
                                            if the flag is not set, the timestamp value is used to set the time interval before TXs
   PKT_FLAG_ALLOW_COLLISION  (default off): do not adopt the mechanism to avoid collision, turn on it if you liek to have collisions
   PKT_FLAG_ZERO_IFS_BACKOFF (default off): do not do not add any additional delay before TX, e.g. useful when you try to test RTS->CTS <SIFS> MPDU->ACK sequence
 */
#define PKT_FLAG_NEED_RESPONSE      0x00000001UL 
#define PKT_FLAG_NORESP_RETRY       0x00000002UL
#define PKT_FLAG_TOGGLE_RETRY_BIT   0x00000004UL  /* toggle retry bit on retry */

#define PKT_FLAG_AT_ABSOLUTE_TIME   0x00010000UL  /* XXX: the bit must be identical to the define in cosim.h */
#define PKT_FLAG_ALLOW_COLLISION    0x00020000UL  /* do not apply collision avoidance mechanism */
#define PKT_FLAG_ZERO_IFS_BACKOFF   0x00040000UL  /* do not add any additional delay before TX */

#define PKT_FLAG_FORCE_AGGREGATION_WRONG_LENGTH       0x01000000UL
#define PKT_FLAG_FORCE_AGGREGATION_CORRUPT_DELIMETER  0x02000000UL

typedef struct __tag_raw_packet
{
   unsigned char data[MAX_RAW_PACKET_SIZE + 4];
   unsigned char *bb_header;
   unsigned char *payload;
   unsigned char domain;
   int length;
   int direction;
   u32 duration;
   u64 timestamp;
   u64 airtime;
   u64 packet_start;

   int ch_offset;
   int format;
   int sgi;
   int rate;

   unsigned long flags;
} raw_packet;

#define __RAW_PACKET_DEFINED

void packet_queue_init(void);
mpdu *packet_alloc(void);
void packet_free(mpdu *pkt);
void packet_enqueue(mpdu *pkt);
mpdu *packet_match(mpdu *pkt);
void packet_queue_dump(int direction);
int packet_queue_check(void);
int packet_parse(mpdu *pkt);
int packet_encrypt(mpdu *pkt);
int packet_decrypt(mpdu *pkt);
void packet_append_fcs(mpdu *pkt);
void packet_link_tail(mpdu *pkt1, mpdu *pkt2);

void packets_to_raw_packet(mpdu *pkt, raw_packet *rawpkt, int agg_count);
int raw_packet_to_packets(mpdu **pkt, raw_packet *rawpkt);
int packet_check_fcs(mpdu *pkt);

void packet_dump(mpdu *pkt);
void packet_dump_single(mpdu *pkt);
void raw_packet_dump(raw_packet *rawpkt);

struct bb_header
{
#if defined(BIG_ENDIAN)
   u8    :1;
   u8   antennas:1;
   u8   ch_offset:2;
   u8    :1;
   u8   _11b_sp:1;
   u8   format:2;

   u8    :4;
   u8   rate:4;
   
   u8   mpdu_len_l:8;
   u8    :4;
   u8   mpdu_len_h:4;

   u8   cbw:1;
   u8   mcs:7;

   u8   ampdu_len_l:8;
   u8   ampdu_len_h:8;

   u8   sgi:1;
   u8   fec:1;
   u8   stbc:2;
   u8   aggregation:1;
   u8    :1;
   u8   nsounding:1;
   u8   smoothing:1;

   u8   __reserved1;
   u8   rssi;
   u8   snr;
   u8   __reserved[5];
#else
   u8   format:2;
   u8   _11b_sp:1;
   u8    :1;
   u8   ch_offset:2;
   u8   antennas:1;
   u8    :1;

   u8   rate:4;
   u8    :4;

   u8   mpdu_len_l:8;

   u8   mpdu_len_h:4;
   u8    :4;

   u8   mcs:7;
   u8   cbw:1;

   u8   ampdu_len_l:8;
   u8   ampdu_len_h:8;

   u8   smoothing:1;
   u8   nsounding:1;
   u8    :1;
   u8   aggregation:1;
   u8   stbc:2;
   u8   fec:1;
   u8   sgi:1;

   u8   __reserved1;
   u8   rssi;
   u8   snr;
   u8   __reserved[5];
#endif  
} __attribute__((__packed__));

struct ampdu_delimiter 
{
#if defined(BIG_ENDIAN)
   u8    mpdu_length_l:4;
   u8    __reserved:4;

   u8    mpdu_length_h;

   u8    crc;

   u8    signature;
#else
   u8    __reserved:4;
   u8    mpdu_length_l:4;

   u8    mpdu_length_h;

   u8    crc;

   u8    signature;
#endif
} __attribute__((__packed__));

#define AMPDU_DELIMITER_SIGNATURE   0x4E
#define BB_HEADER_LENGTH   (sizeof(struct bb_header))

#define FMT_NONE_HT  0
#define FMT_HT_MIXED 1
#define FMT_HT_GF    2
#define FMT_11B      3

#define CH_OFFSET_20            0
#define CH_OFFSET_40            1
#define CH_OFFSET_20_U          2
#define CH_OFFSET_20_L          3


#endif //__PACKET_H__
