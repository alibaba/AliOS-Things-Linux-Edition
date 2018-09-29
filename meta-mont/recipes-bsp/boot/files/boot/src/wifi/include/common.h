#ifndef _COMMON_H_
#define _COMMON_H_

#define FRAME_TYPE_MANAGEMENT   0
#define FRAME_TYPE_CONTROL      1
#define FRAME_TYPE_DATA         2

/* SUB-TYPES of TYPE MANAGEMENT */
#define ASSOC_REQ       0
#define ASSOC_RESP      1
#define REASSOC_REQ     2
#define REASSOC_RESP    3
#define PROBE_REQ       4
#define PROBE_RESP      5
#define BEACON          8
#define ATIM            9
#define DISASSOC        10
#define AUTH            11
#define DEAUTH          12
#define ACTION          13
#define ACTION_NOACK    14

/* SUB-TYPES of TYPE DATA*/
#define DATA            0
#define DATA_CFACK      1
#define DATA_CF_POLL    2
#define DATA_CF_ACK_CF_POLL     3
#define DATA_NULL       4
#define CF_ACK          5
#define CF_POLL         6
#define CF_ACK_CF_POLL  7
#define QDATA            8
#define QDATA_CFACK      9
#define QDATA_CF_POLL    10
#define QDATA_CF_ACK_CF_POLL     11
#define QDATA_NULL       12
#define QCF_POLL         13
#define QCF_ACK_CF_POLL  15

/* SUB-TYPES of TYPE CONTROL */
#define CW          7
#define BAR         8
#define BA          9
#define PS_POLL     10
#define RTS         11
#define CTS         12
#define ACK         13
#define CFEND       14
#define CFEND_CFACK 15

#define UNKNOWN     16

#define FRAME_SUBTYPE_QOS_BIT   0x8
#define FRAME_SUBTYPE_NULL_DATA_BIT 0x04



#define SLOT_TIME   9
#define SIFS_TIME   16
#define PIFS_TIME   (SIFS_TIME + SLOT_TIME)
#define DIFS_TIME   (PIFS_TIME + SLOT_TIME)


// should be large enough to accommodate all kinds of wifi headers
typedef struct 
{
#if defined(BIG_ENDIAN)
    u8      subtype:4;
    u8      type:2;
    u8      version:2;
    
    u8      order:1;
    u8      protected_frame:1;
    u8      more_data:1;
    u8      power_mgt:1;
    u8      retry:1;
    u8      more_frag:1;
    u8      fromds:1;
    u8      tods:1;
#else
    u8      version:2;
    u8      type:2;
    u8      subtype:4;

    u8      tods:1;
    u8      fromds:1;
    u8      more_frag:1;
    u8      retry:1;
    u8      power_mgt:1;
    u8      more_data:1;
    u8      protected_frame:1;
    u8      order:1;
#endif
} __attribute__((__packed__)) frame_control;

typedef struct 
{
#if defined(BIG_ENDIAN)
    u8      amsdu:1;
    u8      ack_policy:2;
    u8      eosp:1;
    u8      tid:4;

    u8      txop:8;
#else
    u8      tid:4;
    u8      eosp:1;
    u8      ack_policy:2;
    u8      amsdu:1;

    u8      txop:8;
#endif
} __attribute__((__packed__)) qos_control;

typedef struct
{
    frame_control   frame_ctrl;
    u16     duration;
    u8      addr1[6];
    u8      addr2[6];
    u8      addr3[6];
    u16     sequence;
} __attribute__((__packed__)) wifi_common_header;

typedef struct
{
    frame_control   frame_ctrl;
    u16     duration;
    u8      addr1[6];
    u8      addr2[6];
    u8      addr3[6];
    u16     sequence;
} __attribute__((__packed__)) wifi_header_3addr;

typedef struct
{
    frame_control   frame_ctrl;
    u16     duration;
    u8      addr1[6];
    u8      addr2[6];
    u8      addr3[6];
    u16     sequence;
    qos_control     qos_ctrl;
} __attribute__((__packed__)) wifi_header_3addr_qos;

typedef struct
{
    frame_control   frame_ctrl;
    u16     duration;
    u8      addr1[6];
    u8      addr2[6];
    u8      addr3[6];
    u16     sequence;
    u8      addr4[6];
    qos_control     qos_ctrl;
} __attribute__((__packed__)) wifi_header_4addr_qos;

typedef struct
{
    frame_control   frame_ctrl;
    u16     duration;
    u8      addr1[6];
    u8      addr2[6];
    u8      addr3[6];
    u16     sequence;
    u8      addr4[6];
} __attribute__((__packed__)) wifi_header_4addr;

typedef struct
{
    frame_control   frame_ctrl;
    u16     duration;
    u8      addr1[6];
    frame_control   carried_frame_ctrl;
    u32     htc;
    u8      addr2[6];
    u8      __data[32];
} __attribute__((__packed__)) wifi_cw_header;

// ethernet header
typedef struct
{
    u8      DA[6];
    u8      SA[6];
    u16     Len;
    u16     padding;  /* let the size equal to ethernet_vlan_header */
    u32     padding1[2];
} __attribute__((__packed__)) ethernet_header;

// ethernet header with VLAN tag
typedef struct
{
    u8      DA[6];
    u8      SA[6];
    u32     vlan;
    u16     Len;
    u16     padding;
    u32     padding1;
} __attribute__((__packed__)) ethernet_vlan_header;

// ethernet header with double VLAN tag
typedef struct
{
    u8      DA[6];
    u8      SA[6];
    u32     vlan;
    u32     vlan2;
    u16     Len;
    u16     padding;
} __attribute__((__packed__)) ethernet_double_vlan_header;

#define MACADDR_LEN 6

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
    ERR_LAST = -8,
};

#endif // _COMMON_H_

