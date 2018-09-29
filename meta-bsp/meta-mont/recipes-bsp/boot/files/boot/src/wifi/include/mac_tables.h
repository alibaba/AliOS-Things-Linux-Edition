#ifndef __MAC_TABLES_H__
#define __MAC_TABLES_H__

#include "mac_common_sim.h"

#undef __packed
#define __packed

typedef union {
    struct {
#if defined(BIG_ENDIAN)
        u32     :8;

        u32 vld:1;
        u32 tosw:1;
        u32 cipher:1;
        u32 wep_defkey:1;       

        u32 under_ap:1;
        u32 p2p_go:1;
        u32 eth_header:1;
        u32 reserved:6;

        u32 rx_ampd7:1;        
        u32 rx_ampd6:1;
        u32 rx_ampd5:1;
        u32 rx_ampd4:1;
        u32 rx_ampd3:1;
        u32 rx_ampd2:1;
        u32 rx_ampd1:1;
        u32 rx_ampd0:1;

        u32 bssid:3;
#else
        u32 bssid:3;

        u32 rx_ampd0:1;        
        u32 rx_ampd1:1;
        u32 rx_ampd2:1;
        u32 rx_ampd3:1;
        u32 rx_ampd4:1;
        u32 rx_ampd5:1;
        u32 rx_ampd6:1;
        u32 rx_ampd7:1;

        u32 reserved:6;
        u32 eth_header:1;
        u32 p2p_go:1;
        u32 under_ap:1;

        u32 wep_defkey:1;
        u32 cipher:1;
        u32 tosw:1;
        u32 vld:1;

        u32     :8;
#endif
    } bit;
    u32 val;
} __packed sta_basic_info;

#define NEW_BUFH

#if defined(NEW_BUFH)
typedef struct {
#if defined(BIG_ENDIAN)
    u32     offset:8;
    u32     next_index:11;
    u32     len:13;
    u32     ep:1;
    u32      :2;
    u32     dptr:29;
#else
    u32     len:13;
    u32     next_index:11;
    u32     offset:8;
    u32     dptr:29;
    u32      :2;
    u32     ep:1;
#endif
} __packed buf_header;
#else
typedef struct {
#if defined(BIG_ENDIAN)
    u32     next_index:16;
    u32     ep:1;
    u32     offset_h:2;             /* offset: high bits */
    u32     len:13;
    u32     offset:6;               /* offset: low bits */
    u32     dptr:26;
#else
    u32     len:13;
    u32     offset_h:2;             /* offset: high bits */
    u32     ep:1;
    u32     next_index:16;
    u32     dptr:26;
    u32     offset:6;               /* offset: low bits */
#endif
} __packed buf_header;
#endif

typedef struct {
#if defined(BIG_ENDIAN)
    u32      :6;
    u32     bhr_t:10;
    u32     bcq_eop:1;
    u32      :5;
    u32     bhr_h:10;

    u32     wh:1;
    u32     bc:1;
    u32     ts:1;         
    u32     pkt_length:13;

    u32     sn:12;
    u32     frag_num:4;

    u32      :1;
    u32     rts:1;
    u32     hdr_len:6;
    u32     dis_dur:1;
    u32     llc:1;
    u32     qos:1;
    u32     no_mic:1;
    u32     cipher_mode:4;

    u32     order:1;
    u32     sec:1;
    u32     md:1;
    u32     pm:1;
    u32      :1;
    u32     m_frag:1;
    u32     frds:1;
    u32     tods:1;
    u32     amsdu:1;
    u32     ack:2;
    u32     eosp:1;
    u32     tid:4;

    u32     wep_def:1;
    u32     rekey_id:1;
    u32     key_idx:6;
    u32     ds_idx:5;
    u32     bssid:3;
    u32     tx_pwr:4;
    u32     sp:1;
    u32     smt:1;
    u32     ness:2;
    u32     nhtm:2;
    u32     ant:2;
    u32     fec:1;
    u32     nsd:1;
    u32     stbc:2;

    u16     tx_rate[4];

    u64     tsc[2];

    u16     tx_status[4];
#else
    u32     bhr_h:10;
    u32      :5;
    u32     bcq_eop:1;
    u32     bhr_t:10;
    u32      :6;

    u32     frag_num:4;
    u32     sn:12;

    u32     pkt_length:13;
    u32     ts:1;  
    u32     bc:1;
    u32     wh:1;

    u32     tid:4;
    u32     eosp:1;
    u32     ack:2;
    u32     amsdu:1;
    u32     tods:1;
    u32     frds:1;
    u32     m_frag:1;
    u32      :1;
    u32     pm:1;
    u32     md:1;
    u32     sec:1;
    u32     order:1;

    u32     cipher_mode:4;
    u32     no_mic:1;
    u32     qos:1;
    u32     llc:1;
    u32     dis_dur:1;
    u32     hdr_len:6;
    u32     rts:1;
    u32      :1;

    u32     stbc:2;
    u32     nsd:1;
    u32     fec:1;
    u32     ant:2;
    u32     nhtm:2;
    u32     ness:2;
    u32     smt:1;
    u32     sp:1;
    u32     tx_pwr:4;
    u32     bssid:3;
    u32     ds_idx:5;
    u32     key_idx:6;
    u32     rekey_id:1;
    u32     wep_def:1;

    u16     tx_rate[4]; /* XXX */

    u64     tsc[2]; /* XXX */

    u16     tx_status[4];
#endif
} __packed tx_descriptor;

typedef struct {
#if defined(BIG_ENDIAN)
    u32     own:1;          /* owner bit, 0:free , 1: upper MAC occupy */
    u32     eor:1;          /* end of ring */
    u32         :1;
    u32     bc:1;           
    u32     da_is_ap:1;
    u32     df:1;           /* direct forward */
    u32     swb:1;
    u32     np:1;           /* use sta cap tables next page */

    u32     secoff:1;
    u32         :2;
    u32     dis_agg:1;
    u32     tid:4;

    u32     swq_status:2;
    u32     header_length:6;

    u32     addr_index:8;

    u32     essid:4;
    u32     nd_rts:1;
    u32     dis_sn:1;
    u32     dis_duration:1;
    u32     ins_ts:1;

    u32     fack:1;
    u32     md:1;
    u32     ins_gsn:1;
    u32     ack_policy:2; // nd_ack:2;
    u32     bssid:3;

    u32     pkt_length:13;
    u32     ba_no_agg:1;
    u32     AMSDU:1;
    u32     ps:1;            /* when np=1 and ps=1, the TX packet will check for PS using addr_index value */

    u32     frame_header_tail_index:16;
    u32     frame_header_head_index:16;

    union
    {
            struct
            {
                  u32         :16;
                  u32     np_addr_index:8;
                  u32         :3;
                  u32     tx_failed:1;
                  u32     retry_cnt:4;
            } tx;
            struct
            {
                  u32         :22;
                  u32     ifs:2;
                  u32         :1;
                  u32     antennas:1;
                  u32     ch_offset:2;
                  u32     non_ht_mode:2;
                  u32     format:2;
            } beacon;
    } u;
#else
    u32     addr_index:8;

    u32     header_length:6;
    u32     swq_status:2;

    u32     tid:4;
    u32     dis_agg:1;
    u32         :2;
    u32     secoff:1;

    u32     np:1;
    u32     swb:1;
    u32     df:1;           /* direct forward */
    u32     da_is_ap:1;
    u32     bc:1; 
    u32         :1;
    u32     eor:1;          /* end of ring */
    u32     own:1;          /* owner bit, 0:free , 1: upper MAC occupy */

    u32     ps:1;
    u32     AMSDU:1;
    u32     ba_no_agg:1;
    u32     pkt_length:13;

    u32     bssid:3;
    u32     ack_policy:2; // nd_ack:2;
    u32     ins_gsn:1;
    u32     md:1;
    u32     fack:1;

    u32     ins_ts:1;
    u32     dis_duration:1;
    u32     dis_sn:1;
    u32     nd_rts:1;
    u32     essid:4;

    u32     frame_header_head_index:16;
    u32     frame_header_tail_index:16;

    union
    {
            struct
            {
                  u32     retry_cnt:4;
                  u32     tx_failed:1;
                  u32         :3;
                  u32     np_addr_index:8;
                  u32         :16;
            } tx;
            struct
            {
                  u32     format:2;  
                  u32     non_ht_mode:2;
                  u32     ch_offset:2;
                  u32     antennas:1;
                  u32         :1;
                  u32     ifs:2;
                  u32         :22;
            } beacon;
    } u;
#endif
} __packed old_tx_descriptor;

typedef struct 
{
    old_tx_descriptor tx_descr;
#if defined(BIG_ENDIAN)
    u32 np:1;
    u32 b2b:1;
    u32    :4;
    u32 next_pointer:26;
    u32 ifs:2;
    u32 is_atim:1;
    u32    :3;
    u32 schedule_time:26;
#else
    u32 next_pointer:26;
    u32    :4;
    u32 b2b:1;
    u32 np:1;
    u32 schedule_time:26;
    u32    :3;
    u32 is_atim:1;
    u32 ifs:2;
#endif
} __packed ssq_tx_descr;

typedef struct {
#if defined(BIG_ENDIAN)
    u32     own:1;          /* owner bit, 0:free , 1: upper MAC occupy */
    u32     eor:1;          /* end of ring */
    u32     fsc_err_sw:1;
    u32     icv_err_sw:1;
    u32     ra_mismatch:1;
    u32      :1;

    u32     frame_header_tail_index:10;

    u32      :6;
    u32     frame_header_head_index:10;
#else
    u32     frame_header_head_index:10;
    u32      :6;

    u32     frame_header_tail_index:10;

    u32      :1;
    u32     ra_mismatch:1;
    u32     icv_err_sw:1;
    u32     fsc_err_sw:1;
    u32     eor:1;          /* end of ring */
    u32     own:1;          /* owner bit, 0:free , 1: upper MAC occupy */
#endif
} __packed rx_descriptor;

typedef struct {
#if defined(BIG_ENDIAN)
    u32      :1;
    u32     wh:1;
    u32     llc:1;
    u32     ipcsok:1;
    u32     tcpcsok:1;
    u32     ba:1;
    u32     from_ds:1;
    u32     tods:1;

    u32     amsdu:1;
    u32     bc:1;
    u32     mc:1;
    u32     qos:1;
    u32     retry:1;
    u32     ps:1;
    u32     eosp:1;
    u32     more_data:1;

    u32     ra_index:3;
    u32     mfrag:1;
    u32     tid:4;
    u32     sa_index:8;

    u32      :2;
    u32     packet_len:14;

    u32     sn:12;
    u32     frag_num:4; 

    u32     sec_status:4;

#define SEC_STATUS_NO_KEY       0x08
#define SEC_STATUS_SEC_MISMATCH 0x04
#define SEC_STATUS_TKIP_MIC_ERR 0x02
#define SEC_STATUS_ICV_ERR      0x01

    u32     hit:4;

#define HIT_DS              0x08
#define HIT_STA             0x04
#define HIT_AP_MODE_BSSID   0x02

    u32     hd_buff_ofs:8;
    u32     rssi:8;
    u32     rate:8;

    u32     rsc[4];

    u32     snr:8;
    u32      :3;
    u32     sgi:1;
    u32     cbw:1;
    u32     b_sp:1;
    u32     format:2;
    u32      :16;

    u32     snr_p1:8;
    u32      :24;

    u32     adc_swing_idx:8;
    u32     sfo_ppm:8;
    u32     cfo_ppm:8;
    u32     snr_p2:8;

    u32     bhr_t:16;
    u32     bhr_h:16;
#else
    u32     sa_index:8;
    u32     tid:4;
    u32     mfrag:1;
    u32     ra_index:3;

    u32     more_data:1;
    u32     eosp:1;
    u32     ps:1;
    u32     retry:1;
    u32     qos:1;
    u32     mc:1;
    u32     bc:1;
    u32     amsdu:1;

    u32     tods:1;
    u32     from_ds:1;
    u32     ba:1;
    u32     tcpcsok:1;
    u32     ipcsok:1;
    u32     llc:1;
    u32     wh:1;
    u32      :1;

    u32     frag_num:4; 
    u32     sn:12;

    u32     packet_len:14;
    u32      :2;

    u32     rate:8;
    u32     rssi:8;
    u32     hd_buff_ofs:8;

    u32     hit:4;

#define HIT_DS              0x08
#define HIT_STA             0x04
#define HIT_AP_MODE_BSSID   0x02

    u32     sec_status:4;

#define SEC_STATUS_NO_KEY       0x08
#define SEC_STATUS_SEC_MISMATCH 0x04
#define SEC_STATUS_TKIP_MIC_ERR 0x02
#define SEC_STATUS_ICV_ERR      0x01

    u32     rsc[4];

    u32      :16;
    u32     format:2;
    u32     b_sp:1;
    u32     cbw:1;
    u32     sgi:1;
    u32      :3;
    u32     snr:8;

    u32      :24;
    u32     snr_p1:8;

    u32     snr_p2:8;
    u32     cfo_ppm:8;
    u32     sfo_ppm:8;
    u32     adc_swing_idx:8;

    u32     bhr_h:16;
    u32     bhr_t:16;
#endif
} __packed rx_packet_descriptor;

typedef struct {
#if defined(BIG_ENDIAN)
    u32     sequence_num:12;
    u32        :2;
    u32     bank_sel:2;
    u32     next_index:16;
    u32     descr[4];
    u32     tsc[4];
#else
    u32     next_index:16;
    u32     bank_sel:2;
    u32        :2;
    u32     sequence_num:12;
    u32     descr[4];
    u32     tsc[4];
#endif
} __packed mmac_rx_descriptor;

#define CIPHER_TYPE_NONE    0
#define CIPHER_TYPE_WEP40   1
#define CIPHER_TYPE_WEP104  2
#define CIPHER_TYPE_TKIP    3
#define CIPHER_TYPE_CCMP    4
#define CIPHER_TYPE_WAPI    5

#define CIPHYER_TYPE_INVALID    CIPHER_TYPE_NONE

#define WAPI_PN_MSB         0x5c365c36UL
#define WAPI_TX_INC_ONE     0
#define WAPI_TX_INC_TWO     1
#define WAPI_RX_CHECK_INC_EVEN    0
#define WAPI_RX_CHECK_INC_ODD     1
#define WAPI_RX_CHECK_INC         2

typedef union {
#if defined(BIG_ENDIAN)
    struct {
        u32     cipher_type:4;
        u32     cipher_type1:4;
        u32     cipher_type2:4;
        u32     cipher_type3:4;
        u32         :14;
        u32     txkeyid:2; 

        u8      key[13];
        u8      key1[13];
        u8      key2[13];
        u8      key3[13];

        u32     __dummy;

        u32     iv:24;
        u32         :8;
    } __packed wep_def_key;
    struct {
        u32     cipher_type:4;
        u32     cipher_type1:4;
        u32     cipher_type2:4;
        u32     cipher_type3:4;
        u32         :14;
        u32     txkeyid:2; 

        u8      key[13];
        u8      key1[13];
        u8      key2[13];
        u8      key3[13];

        u32     __dummy;

        u32     iv:24;
        u32         :8;
    } __packed wep_km_key;
    struct {
        u32     cipher_type:4;
        u32         :26;
        u32     txkeyid:2;
    
        u32     txmickey[2];

        u32     rxmickey[2];

        u8      key[16];

        u32     txtsc_lo;
    
        u32     txtsc_hi:16;
        u32         :16;

        u32     __dummy[5];
    } __packed tkip_group_key;
    struct {
        u32     cipher_type:4;
        u32         :26;
        u32     txkeyid:2;
    
        u32     txmickey[2];

        u32     rxmickey[2];

        u8      key[16];

        u32     txtsc_lo;
    
        u32     txtsc_hi:16;
        u32         :16;
    
        u32     rxmic[2];
    
        u32     rxdata[3];
    } __packed tkip_pair_key;
    struct {
        u32     cipher_type:4;
        u32         :26;
        u32     txkeyid:2;
    
        u32     __dummy[4];

        u8      key[16];

        u32     txpn_lo;

        u32     txpn_hi:16;
        u32         :16;
    
        u32     __dummy2[5];
    } __packed ccmp_group_key;
    struct {
        u32     cipher_type:4;
        u32         :26;
        u32     txkeyid:2;
    
        u32     __dummy[4];

        u8      key[16];

        u32     txpn_lo;

        u32     txpn_hi:16;
        u32         :16;
    
        u32     __dummy2[5];
    } __packed ccmp_pair_key;
    struct {
        u32     cipher_type:4;
        u32         :26;
        u32     txkeyid:2;
    
        u8      mickey[16];

        u8      key[16];

        u32     txpn[4];

        u32     __dummy[3];
    } __packed wapi_group_key;
    struct {
        u32     cipher_type:4;
        u32         :26;
        u32     txkeyid:2;
    
        u8      mickey[16];

        u8      key[16];

        u32     txpn[4];

        u32     __dummy[3];
    } __packed wapi_pair_key;
#else
    struct {
        u32     txkeyid:2;
        u32         :14;
        u32     cipher_type3:4;
        u32     cipher_type2:4;
        u32     cipher_type1:4;
        u32     cipher_type:4;

        u8      key[13];
        u8      key1[13];
        u8      key2[13];
        u8      key3[13];

        u32     __dummy;

        u32         :8;
        u32     iv:24;
    } __packed wep_def_key;
    struct {
        u32     txkeyid:2;
        u32         :14;
        u32     cipher_type3:4;
        u32     cipher_type2:4;
        u32     cipher_type1:4;
        u32     cipher_type:4;

        u8      key[13];
        u8      key1[13];
        u8      key2[13];
        u8      key3[13];

        u32     __dummy;

        u32         :8;
        u32     iv:24;
    } __packed wep_km_key;
    struct {
        u32     txkeyid:2;
        u32         :26;
        u32     cipher_type:4;
    
        u32     txmickey[2];

        u32     rxmickey[2];

        u8      key[16];

        u32     txtsc_lo;

        u32         :16;
        u32     txtsc_hi:16;

        u32     __dummy[5];
    } __packed tkip_group_key;
    struct {
        u32     txkeyid:2;
        u32         :26;
        u32     cipher_type:4;
    
        u32     txmickey[2];

        u32     rxmickey[2];

        u8      key[16];

        u32     txtsc_lo;

        u32         :16;
        u32     txtsc_hi:16;
    
        u32     rxmic[2];
    
        u32     rxdata[3];
    } __packed tkip_pair_key;
    struct {
        u32     txkeyid:2;
        u32         :26;
        u32     cipher_type:4;
    
        u32     __dummy[4];

        u8      key[16];

        u32     txpn_lo;

        u32         :16;
        u32     txpn_hi:16;
    
        u32     __dummy2[5];
    } __packed ccmp_group_key;
    struct {
        u32     txkeyid:2;
        u32         :26;
        u32     cipher_type:4;
    
        u32     __dummy[4];

        u8      key[16];

        u32     txpn_lo;

        u32         :16;
        u32     txpn_hi:16;
    
        u32     __dummy2[5];
    } __packed ccmp_pair_key;
    struct {
        u32     txkeyid:2;
        u32         :26;
        u32     cipher_type:4;
    
        u8      mickey[16];

        u8      key[16];

        u32     txpn[4];

        u32     __dummy[3];
    } __packed wapi_group_key;
    struct {
        u32     txkeyid:2;
        u32         :26;
        u32     cipher_type:4;
    
        u8      mickey[16];

        u8      key[16];

        u32     txpn[4];

        u32     __dummy[3];
    } __packed wapi_pair_key;
#endif
} cipher_key;

typedef struct {
#if defined(BIG_ENDIAN)
    u32         :23;
    u32     tx_queue_len:9;

    u32         :29;
    u32     qmode:3;

    u32         :32;
    u32         :32;

    u32     tx_tail_ptr:16;
    u32     tx_head_ptr:16;

    u32         :23;
    u32     pkt_cnt:9;

    u32         :32;
    u32         :32;
#else
    u32     tx_queue_len:9;
    u32         :23;

    u32     qmode:3;
    u32         :29;

    u32         :32;
    u32         :32;

    u32     tx_head_ptr:16;
    u32     tx_tail_ptr:16;

    u32     pkt_cnt:9;
    u32         :23;

    u32         :32;
    u32         :32;
#endif
} __packed bcast_qinfo;

typedef struct {
#if defined(BIG_ENDIAN)
    u32     timestamp:16;
    u32     next_descr:16;
    u32     tsc[4];
    u32     mpdu_desp[4];
#else
    u32     next_descr:16;
    u32     timestamp:16;
    u32     tsc[4];
    u32     mpdu_desp[4];
#endif
} __packed psbaq_descriptor;

typedef struct {
#if defined(BIG_ENDIAN)
    u32     vld:1;
    u32     qos:1;
    u32        :4;
    u32     qinfo_addr:26;

    u32        :23;
    u32     ap:1;
    u32     sta_idx:8;
#else
    u32     qinfo_addr:26;
    u32        :4;
    u32     qos:1;
    u32     vld:1;

    u32     sta_idx:8;
    u32     ap:1;
    u32        :23;
#endif
} __packed sta_qinfo_tbl;

typedef union {
#if defined(BIG_ENDIAN)
    struct {

        u32     own:1;
        u32     bmap:1;
        u32     ps:1;
        u32     mpdu:1;

        u32     try_cnt:4;
        u32     noa:1;
        u32     mb:1;

        u32      :2;
        u32     aidx:6;

        u32     tid:4;
        u32     bhr_h:10;
    } m;
    struct {

        u32     own:1;
        u32     bmap:1;
        u32     ps:1;
        u32     mpdu:1;

        u32     try_cnt:4;
        u32     noa:1;

        u32     pktlen:13;
        u32     bhr_h:10;
    } a;
#else
    struct {
        u32     bhr_h:10;
        u32     tid:4;

        u32     aidx:6;
        u32      :2;

        u32     mb:1;
        u32     noa:1;
        u32     try_cnt:4;

        u32     mpdu:1;
        u32     ps:1;
        u32     bmap:1;
        u32     own:1;
    } m;
    struct {
        u32     bhr_h:10;
        u32     pktlen:13;

        u32     noa:1;
        u32     try_cnt:4;

        u32     mpdu:1;
        u32     ps:1;
        u32     bmap:1;
        u32     own:1;
    } a;
#endif
} __packed acqe;

#define ACQ_MAX_BUFSIZE 64
struct __tag_acq;
typedef struct __tag_acq acq;
struct __tag_acq 
{
   volatile acq  *next;
   u8   win_size;
   u8   queue_size;
   u8   queue_size_code;
   volatile u8    flags;
   volatile u32    :20;
   volatile u32   rptr:12;
   volatile u32    :20;
   volatile u32   wptr:12;
   u16  qid;
   volatile u16  esn;
   volatile u32  qinfo;
   volatile acqe acqe[ACQ_MAX_BUFSIZE];
};

typedef struct {
    sta_basic_info  basic_info;
#define ADDR    RA_ADDR
    u8      RA_ADDR[6];
    u8      index;
} addr_tbl;


#define KEYTYPE_NONE    0
#define KEYTYPE_WEP     1
#define KEYTYPE_WEP104  2
#define KEYTYPE_TKIP    3
#define KEYTYPE_CCMP    4
#define KEYTYPE_WAPI    5

typedef struct {
	u8          keytype;

	u8          keylen;	        /* key length in bytes */
	u8          key[32];

	u64         keyrsc[17];     /* key receive sequence counter */
	u64	        keytsc;         /* key transmit sequence counter */

	u8          keylen1;	    /* WEP only */
	u8          key1[16];

	u8          keylen2;	    /* WEP only */
	u8          key2[16];

	u8          keylen3;	    /* WEP only */
	u8          key3[16];

    u32         iv;             /* WEP only */
    u8          txkeyid;
} crypto_key;


typedef struct {
    u32     valid:1;
    u32     ess_idx:4;
    u32     bssid:3;
    u32     qos:1;
    u32     ap:1;
    u32     parent_ap:1;
    u32     cipher_mode:4;
    u32     wep_defkey:1;
    u32     rx_check_inc:2;
    u32     mpdu_spacing:3;
    u8      eth:1;
    u8      tx_ampd7:1;
    u8      tx_ampd6:1;
    u8      tx_ampd5:1;
    u8      tx_ampd4:1;
    u8      tx_ampd3:1;
    u8      tx_ampd2:1;
    u8      tx_ampd1:1;
    u8      tx_ampd0:1;

    u8      sta_id;

    /* we still need to reduce memory footprint even it is not a HW structure */
    /* the configuration data will be compiled into simulation program */
    sta_basic_info  basic_info;
    u8      RA_ADDR[6];

    u8      addr_index;
    u8      foreign_sta;

    u8      ps_stat;

    u16     tx_rate[4];
    u16     rx_rate;

    u16     sequence_num_in;
    u16     tid_sequence_num_in[16];
    u16     sequence_num_out;
    u16     tid_sequence_num_out[16];

    crypto_key     key;

    u8 max_ampdu_len;
    u16 u_apsd_bitmap;

    u8 ap_idx;
} sta_cfg;

#undef __packed

#endif // __MAC_TABLES_H__

