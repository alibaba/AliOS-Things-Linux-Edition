#ifndef __GENPKTS_H__
#define __GENPKTS_H__
//#include <sys/types.h>
#if 0
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "mac.h"
#include "mac_internal.h"
#include "mac_common_sim.h"
#include "mac_tables.h"
#include "wla_cfg.h"
#include "common.h"
#include "crc32.h"

/*
    Crypto functions, move to other file later
 */

#include <stdarg.h>

#if defined(SIM)
#include <crypto/compat.h>
#include <crypto/ieee80211_crypto.h>
#include <crypto/rijndael/rijndael.h>
#endif

/*
    End of Crypto functions
 */

#define MAX_FRAGMENT        16
#define METADATA_FIX_OFFSET         44      /* for management & ctrl frame, we insert the meta in offset 32 from the very first byte */

#define SET(x,val) \
do          \
{           \
    (x##_set) = 1;\
    (x) = val;\
} while (0);

#define IS_SET(x)  (x##_set)

#define CHECK_AND_OVERLOAD(VAL, NEWVAL)  \
do          \
{           \
    if(NEWVAL##_set)    \
    {                   \
        VAL = NEWVAL;   \
    }                   \
} while (0);

typedef struct
{  
    unsigned char protocol;

    int direct_forward;     // for CPU injected packet config, set direct_forward to 1 to generate packet with wifi header
                            // for CPU injected packet config, set direct_forward to 0 to generate packet with ethernet header
                            
    int payload_length;     // MANDATORY for data packets : the length of payload
    int org_payload_length; // backup of payload_length, the payload_length will be modified on program execution
    int wifi_header_len;    // for CPU injected packet config, if this is specified, the wifi header will NOT be auto-generated

    int random_data;        // generate random data in the payload, generate data in sequence number if this is not set

    int target_station;     // target WiFi STA number , described in STA cap. table, if the target station is WiFi STA/AP
    int source_station;     // source WiFi STA number , described in STA cap. table, if the source station is WiFi STA/AP

    unsigned char target_mac[MACADDR_LEN];    // MAC address of target interface, if it is ETHERNET device
    unsigned char source_mac[MACADDR_LEN];    // MAC address of source interface, if it is ETHERNET device
    unsigned char nat_da[MACADDR_LEN];
    unsigned char nat_sa[MACADDR_LEN];

    unsigned char da_mac[MACADDR_LEN];
    unsigned char sa_mac[MACADDR_LEN];

    int ether_type;         // ether_type generate in data packet/frame payload, set to 0x800 (IP) if it is not specified
    unsigned long vlan;     // vlan tag in the ethernet header
    unsigned long vlan2;    // 2nd vlan tag in the ethernet header (double tagging)

    unsigned long src_ip;
    unsigned long dst_ip;
    unsigned short src_port;
    unsigned short dst_port;

    unsigned long nat_ip;
    unsigned short nat_port;
    int nat_dir;            // OUTBOUND-SNAT:0, INBOUND-DNAT:1

    unsigned char llc[8];
    int llc_length;

    unsigned char pppoe[32];        // PPPoE header, 1-16 bytes (inserted into payload)
    int pppoe_length;               // PPPoE header length

    unsigned char ip_header[32];     // IP header (inserted into payload)
    int ip_header_length;

    int qos;            // generate qos field in Wifi Header
    int ht;             // generate ht field in Wifi Header

    // variables for packet fragment control , you can config it as fixed (by max_framesize), random (by random_framesize), or customized (variable_framesize)
    int max_framesize;          // max frame buffer size to inject into TX
    int variable_framesize[FRAME_SIZE_ARRAY_SIZE];      // customize the frame size to inject into TX
    int random_framesize;       // randomize the frame size of the packet
    int data_offset;            // ofs field in TX frame header

    int fragment_threshold;     // change the fragment threshold prior to kick the packet if this is set

    #define custom_framesize variable_framesize_set
    // end of packet fragment control

    int cpu_tx_pkt;     // generate CPU inject packet
    int eth_tx_pkt;     // generate Ethernet inject packet
    unsigned int dut_role;

    int bssid;          // BSSID index of the generated packet: from 0 ~ 7 , default is 0

    int version;        // overload version in WiFI frame control, default is 0
    int type;           // overload type in WiFI frame control, default is data frame
    int subtype;        // overload subtype in WiFI frame control, default is 0
    int tods;           // overload tods in WiFI frame control, auto config by default
    int fromds;         // overload fromds in WiFI frame control, auto config by default
    int more_frag;      // overload more_frag in WiFI frame control, default is 0
    int retry;          // overload retry in WiFI frame control, default is 0
    int power_mgt;      // overload power_mgt in WiFI frame control, default is 0
    int more_data;      // overload more_data in WiFI frame control, default is 0
    int protected_frame;    // overload protected_frame in WiFI frame control, default is 0   
    int order;          // overload order in WiFI frame control, default is 0

    int eosp;

    int duration;       // overload duration field in Wifi header

    int tid;            // TID to be filled into TX descriptor, or TID to be filled into QoS control
    int force_tid;      // with force_tid, QoS control field will be added unconditionally for IN direction
    int ack_policy;     // ACK policy in QoS control field

    int amsdu;          // aggregate MSDU next N frames start from this frame

    /* following parameters are applicable for LMAC verification */

    unsigned long fcs;            // manually overwrite FCS
    int crc_error;                // force CRC to be wrong

    unsigned char header_data[50];

    int wifi_fragment_threshold;        // fragment threshold for Wifi packet
    int wifi_fragment_count;            // total numbers of fragment

    /* BAR frame parameters */
    int ssn;                    // start sequence control
    int bar_policy;             // 0 for HT Normal ACK, 1 for NOACK

    /* HTC */
    u32 htc;                    // HT control field

    u64 tsc;                    // override the cipher TSC/IV value

    // variable to do the tricks 
    int protocol_set;

    int direct_forward_set;
    int payload_length_set;
        // int org_payload_length_set; // not going to allow this to be set by users
    int wifi_header_len_set;
    int random_data_set;
    int target_station_set;
    int source_station_set;
    int target_mac_set;
    int source_mac_set;
    int nat_da_set;
    int nat_sa_set;

    int da_mac_set;
    int sa_mac_set;

    int ether_type_set;
    int vlan_set;
    int vlan2_set;

    int src_ip_set;
    int dst_ip_set;
    int src_port_set;
    int dst_port_set;

    int nat_ip_set;
    int nat_port_set;
    int nat_dir_set;

    int llc_set;
    int llc_length_set;

    int pppoe_set;
    int pppoe_length_set;

    int ip_header_set;
    int ip_header_length_set;

    int qos_set;
    int ht_set;
    int max_framesize_set;
    int variable_framesize_set;
    int random_framesize_set;
    int custom_framesize_set;
    int data_offset_set;
    int fragment_threshold_set;
    int cpu_tx_pkt_set;
    int eth_tx_pkt_set;
    int dut_role_set;

    int bssid_set;

    int version_set;
    int type_set;
    int subtype_set;
    int tods_set;
    int fromds_set;
    int more_frag_set;
    int retry_set;
    int power_mgt_set;
    int more_data_set;
    int protected_frame_set;
    int order_set;

    int eosp_set;

    int duration_set;

    int tid_set;
    int force_tid_set; 
    int ack_policy_set;

    int amsdu_set;

    int fcs_set;
    int crc_error_set;

    int header_data_set;

    int wifi_fragment_threshold_set;
    int wifi_fragment_count_set;

    /* BAR frame parameters */
    int ssn_set;
    int bar_policy_set;

    /* HTC */
    int htc_set;

    int tsc_set;
}  pkt_descr;

#ifndef MAX_WIFI_HEADER_LEN
#define MAX_WIFI_HEADER_LEN     ( 2+2+6+6+6+2+6+2+4 )
#endif
#define MAX_PAYLOAD_SIZE        (10240 + 32)

typedef struct __tag_pkt
{
    int direction;

    u8  wifi_hdr[MAX_WIFI_HEADER_LEN];
    int wifi_hdr_len;

    u8  org_wifi_hdr[MAX_WIFI_HEADER_LEN];
    int org_wifi_hdr_len;

#define HT_CONTROL_LENGTH       4
    u8  htc[HT_CONTROL_LENGTH];         // 4 bytes HT control
    int htc_len;                        // ht control length

    int zero_padding; // ethernet zero_padding

#define ETH_HEADER_LEN      12
    u8  org_eth_hdr[ETH_HEADER_LEN];
    u8  eth_hdr[ETH_HEADER_LEN];

    int direct_forward;

    pktgen_pktdescr pkt_descr;

    int payload_len;
    u8  payload[MAX_PAYLOAD_SIZE];

    int eth_payload_len;
    u8  *eth_payload;

    int llc_length;
    u8  llc[16];

    int plaintext_copied;
    int org_payload_len;
    u8  org_payload[MAX_PAYLOAD_SIZE];

    int from;
    int to;
    int type;
    int subtype;
    int power_management;
    int more_data;
    int eosp;
    int amsdu;

    int ipcsok;
    int tcpcsok;

    int crc_error;
    int drop;

    int frame_length;

    int fragment_count;

    int payload_fragment_len[MAX_FRAGMENT];
    u8  payload_fragment[MAX_FRAGMENT][MAX_PAYLOAD_SIZE];

    int org_payload_fragment_len[MAX_FRAGMENT];
    u8  org_payload_fragment[MAX_FRAGMENT][MAX_PAYLOAD_SIZE];

    u8  wifi_hdr_fragment[MAX_FRAGMENT][MAX_WIFI_HEADER_LEN];
    u8  org_wifi_hdr_fragment[MAX_FRAGMENT][MAX_WIFI_HEADER_LEN];

#if defined(SIM)
    struct mbuf mbuf;
#endif

    int cipher_mode;

    int qos;
    int tid;

    int sequence_num;

    u64 tsc[MAX_FRAGMENT][2];
    u64 rsc[2];

    int broadcast;
    int multicast;

    int from_ds;
    int to_ds;

    int ta_idx;
    int ra_idx;

    /* variables for decrypt program */
    int bssid;
} pkt;

#define __PKT_DEFINED

#define ETH_ALEN    6

#define DIRECTION_IN    0
#define DIRECTION_OUT   1

#define QOS_TID0    0
#define QOS_STA_DEFAULT_TID 3


#define PAYLOAD_FORMAT_WIFI         0
#define PAYLOAD_FORMAT_ETHERNET     1


#define ERROR_OUT_OF_MEM        (-1)
#define ERROR_DESCR_DST_STA     (-2)
#define ERROR_DESCR_SRC_STA     (-3)

#define ERROR_INVAL_SOURCE_STA      (-10)
#define ERROR_INVAL_TARGET_STA      (-11)
#define ERROR_IBSS_NOSUPP           (-12)

#define ERROR_PAYLOAD_SIZE          (-20)
#define ERROR_FRAGMENT_PROC         (-21)

#define ROLE_NONE   0
#define ROLE_AP     1
#define ROLE_STA    2
#define ROLE_IBSS   3
#define ROLE_P2PC   4
#define ROLE_P2PGO  5

struct __bss_info
{
    int timer_index;
    unsigned int dut_role;
    unsigned char BSSID[ETH_ALEN];
    unsigned char MAC_ADDR[ETH_ALEN];
    unsigned char WAN_MAC_ADDR[ETH_ALEN];
    unsigned long LAN_IP_ADDR;
    unsigned long WAN_IP_ADDR;
    int AP_STA_NUM;
    int ap_stacap_idx;
    int ap_dstable_idx;
    int cipher_mode;
    int ps_stat;
};
extern struct __bss_info bss_info[MAX_BSSIDS];


typedef struct  __tag_pkg_cfg
{
    unsigned char ip_header[20];
    unsigned char pppoe[8];
    unsigned char ether_type[2];
    unsigned char vlan2_tag[4];
    unsigned char vlan_tag[4];
    unsigned char llc[6];

    unsigned char __dummy;
    unsigned char protocol;
    unsigned char subtype;
    unsigned char wifi_type;

    int ip_header_len;
    int pppoe_len;
    int ether_type_len;
    int vlan2_len;
    int vlan_len;
    int llc_len;

    int fragment_num;
    int tid;
    int payload_len;
    int target_sta_num;
    int source_sta_num;
    int from_cpu;

    int more_data;
    int eosp;
    int amsdu;
}  __attribute__((__packed__)) PKT_CFG;

#define __PKT_CFG_DEFINED

extern int cfg_to_pktgen_descr(PKT_CFG *cfg, pkt_descr *descr, int pkt_type);
extern int generate_wifi_header(pkt_descr *descr, pkt *packet, int direction);
extern int generate_packet_payload(pkt_descr *descr, pkt *packet, int format, int direction, int force_encryption_off, crypto_key *key);

#endif //__GENPKTS_H__
