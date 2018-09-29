/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file mac_ctrl.h
*   \brief
*   \author Montage
*/

#ifndef __MAC_INFO_H__
#define __MAC_INFO_H__

#if 0
#include <mac_common.h>
#include <panther_dev.h>
#include <sta.h>
#include <mt_types.h>
#include <lib_autoconf.h>
#define WLAN_RC_BTS
//#define WLAN_RC_AMRR
#define HW_SYNC_TIME			10 /* 100ms to check hardware status per STA */
#define TX_DESC_HEADROOM 	0	/* no head romm require (in sram : headroom + wtb + frame) */

/* definition of flags */
enum {
	WBUF_FIXED_SIZE				= BIT(0),
	WBUF_RX_FRAME				= BIT(1),		/* indicate the buffer is received from wlan */

	WBUF_TX_REQ_STATUS			= BIT(2),
	WBUF_TX_NO_ACK				= BIT(3),		/* this frame does not need ACK */
	WBUF_TX_NO_SECU				= BIT(4),		/* indicate the frame does not encrypt */
	WBUF_TX_SPEC_RATE			= BIT(5),		/* force to send out in specified rate */
	WBUF_TX_BASIC_RATE			= BIT(6),		/* force to send out in basic rate */
	WBUF_TX_CHECK_PS			= BIT(7),
	WBUF_TX_INSERT_TS			= BIT(8),		/* inform HW insert Timestamp field */

	WBUF_TX_PS_NULL				= BIT(9),		/* indicate send the NULL frame with PM bit */

	WBUF_TX_REPORT_STATUS		= BIT(10),		/* indicate the wbuf is TX status */
	WBUF_TX_SUCCESS				= BIT(11),		/* report transmition is success */
	
	WBUF_MGMT_FRAME				= BIT(12),		/* management frame */
	WBUF_CHAIN					= BIT(13),		/* wbuf link list */
	WBUF_EXT					= BIT(14),		
	WBUF_TX_DIS_DURATION        = BIT(15),      /* disable hw tx duration config */
	WBUF_TX_ASSOC				= BIT(16),

	WBUF_TX_NULL				= BIT(17),		/* indicate send the NULL frame */
	WBUF_TX_EAPOL				= BIT(18),		/* indicate send the eapol */
	WBUF_TX_UAPSD_PS_NULL		= BIT(19),		/* indicate U-APSD QOS NULL frame */
};

enum {
	WBUF_RX_DA_HIT_ADDR			= BIT(0),
	WBUF_RX_DA_HIT_BSSID		= BIT(1),
	WBUF_RX_SA_HIT_ADDR			= BIT(2),
	WBUF_RX_TA_HIT_DS			= BIT(3),
};

enum {	
	WBUF_RX_ICV_ERR				= BIT(0),
	WBUF_RX_MIC_ERR				= BIT(1),
	WBUF_RX_KEY_NOT_VALID		= BIT(2),
	WBUF_RX_NON_KEY				= BIT(3),
};

enum {
	WBUF_TYPE_DATA					= BIT(0),
	WBUF_TYPE_BEACON_PROBE_RESP		= BIT(1),
	WBUF_TYPE_MGMT_PUBLIC_ACTIN     = BIT(2),
};

#define WLAN_RX_LLC_OFFSET					80

#define MAC_ACK_POLICY_NORMAL_ACK			0
#define MAC_ACK_POLICY_NOACK				1

#define MAC_IFS_DIFS						2

//#define TKIP_COUNTERMEASURE	1

///#define BHDR_LOCK()				///arthur_lock()
///#define BHDR_UNLOCK()			///arthur_unlock()

#if 0
#define BW20_ONLY	0
#define HT40_PLUS	1
#define HT40_MINUS	2
#endif
enum {
	BW40MHZ_SCN = 0,	/* no secondary channel is present */
	BW40MHZ_SCA = 1,	/* secondary channel is above the primary channel */
	BW40MHZ_SCB = 3,	/* secondary channel is below the primary channel */
	BW40MHZ_AUTO = 4,	/* auto select secondary channel */
};

/* channel offset */
#define CH_OFFSET_20		0
#define CH_OFFSET_40		1
#define CH_OFFSET_20U		2
#define CH_OFFSET_20L		3

/* rate format encoding */
#define FMT_NO_HT			0
#define FMT_HT				1
#define FMT_HT_GF			2
#define FMT_11B				3

#define CCK_1M_CODE			0x0
#define CCK_2M_CODE			0x1
#define CCK_5_5M_CODE		0x2
#define CCK_11M_CODE		0x3
#define FMT_11B_SHORT_PREAMBLE 0x4

#define OFDM_6M_CODE		0xb
#define OFDM_9M_CODE		0xf
#define OFDM_12M_CODE		0xa
#define OFDM_18M_CODE		0xe
#define OFDM_24M_CODE		0x9
#define OFDM_36M_CODE		0xd
#define OFDM_48M_CODE		0x8
#define OFDM_54M_CODE		0xc

#define MCS_0_CODE			0
#define MCS_1_CODE			1
#define MCS_2_CODE			2
#define MCS_3_CODE			3
#define MCS_4_CODE			4
#define MCS_5_CODE			5
#define MCS_6_CODE			6
#define MCS_7_CODE			7
#define MCS_8_CODE			8
#define MCS_9_CODE			9
#define MCS_10_CODE			10
#define MCS_11_CODE			11
#define MCS_12_CODE			12
#define MCS_13_CODE			13
#define MCS_14_CODE			14
#define MCS_15_CODE			15

#define MCS_32_CODE			32

#define B_RATE				0x1
#define G_RATE				0x2
#define HT_RATE				0x4

#define MACADDR_TABLE_MAX	4

#define PHYSICAL_MACADDR	0x1
#define AP_MACADDR			0x2
#define STA_MACADDR			0x4
#define P2P_MACADDR			0x8
#define IBSS_MACADDR		0x10
#define WIF_USED			0x80

#define WMAC_WAIT_FOREVER(con, msg) \
	do { \
		unsigned int cnt = 0; \
		while ((con)) { \
			cnt++; \
			if (cnt == 0xffffffff) { \
				break; \
			} else if(cnt == 0xffff) { \
				WLAN_DBG(msg"?????????????System maybe enter Busy Loop????????????\n"); \
			} \
		} \
		if (cnt >= 0xffff) WLAN_DBG("Loop stop, cnt=%d\n", cnt); \
	} while(0)

struct wb_q {
	struct wbuf *head;
	struct wbuf *tail;
	unsigned int len;
};

struct beacon_desc {
	ssq_tx_descr desc;
	u32 total_len; /* software store beacon original total length */
};

#if (CONFIG_ROM_VER == 1)
#define IDX_TO_ADDR_TBL(_i)		&(((sta_addr_entry *)info->ext_sta_tbls)[_i])
#define IDX_TO_DS_TBL(_i)		&(((sta_addr_entry *)info->ext_ds_tbls)[_i])
#define IDX_TO_PRIVATE_KEY(_i)	&(((cipher_key *)info->private_keys)[_i])
#define IDX_TO_GROUP_KEY(_i)	&(((cipher_key *)info->group_keys)[_i])
#define IDX_TO_TXQ(_i)			&(((struct tx_q *)info->wl_txq)[_i])
#define IDX_TO_BCMCQ(_i)		&(((struct tx_q *)info->wl_txq)[MAC_MAX_PSBAQ + _i])
#else
#define IDX_TO_ADDR_TBL(_i)		&(((sta_addr_entry *)lapp->ext_sta_tbls)[_i])
#define IDX_TO_DS_TBL(_i)		&(((sta_addr_entry *)lapp->ext_ds_tbls)[_i])
#define IDX_TO_PRIVATE_KEY(_i)	&(((cipher_key *)lapp->private_keys)[_i])
#define IDX_TO_GROUP_KEY(_i)	&(((cipher_key *)lapp->group_keys)[_i])
#define IDX_TO_TXQ(_i)			&(((struct tx_q *)lapp->wl_txq)[_i])
#define IDX_TO_BCMCQ(_i)		&(((struct tx_q *)lapp->wl_txq)[MAC_MAX_PSBAQ + _i])
#endif

#define IDX_TO_BSS(_i)			&(((struct wm_bss *)lapp->bss_ctxs)[_i])
#define IDX_TO_STA(_i)			&(((struct sta_ctx *)lapp->sta_ctxs)[_i])
#define IDX_TO_NBSS(_i)			&(((struct neighbor_bss *)lapp->nbss_list)[_i])
#define IDX_TO_RXBA_SESS(_i)	&(((struct rx_ba_session *)lapp->rx_ba_tbls)[_i])

#if (CONFIG_ROM_VER == 1)
#define TXQ_TO_IDX(_t)			((struct tx_q *)_t - ((struct tx_q *)(info->wl_txq)))
#else
#define TXQ_TO_IDX(_t)			((struct tx_q *)_t - ((struct tx_q *)(lapp->wl_txq)))
#endif
#define BSS_TO_IDX(_t)			((struct wm_bss *)_t - ((struct wm_bss *)(lapp->bss_ctxs)))
#define STA_TO_IDX(_t)			((struct sta_ctx *)_t - ((struct sta_ctx *)(lapp->sta_ctxs)))
#define NBSS_TO_IDX(_t)			((struct neighbor_bss *)_t - ((struct neighbor_bss *)(lapp->nbss_list)))
#define RXBA_SESS_TO_IDX(_t)	((struct rx_ba_session *)_t - ((struct rx_ba_session *)(lapp->rx_ba_tbls)))

/* address lookup options */
#define BY_ADDR_IDX			0x1
#define IN_DS_TBL			0x2
/* address lookup error code */
#define LOOKUP_ERR			(-1)
#define LOOKUP_WRONG_TBL	(-2)

#define COUNTER_DELTA(delta, prev_hw_counter, current_hw_counter) \
	do { \
		u16 current_val = (current_hw_counter); \
		int hw_counter_width = sizeof(current_hw_counter) * 8; \
		if((current_val) < (prev_hw_counter)) \
			(delta) = ((0x01UL << hw_counter_width) - ((prev_hw_counter) - (current_val))); \
		else \
			(delta) = ((current_val) - (prev_hw_counter)); \
		(prev_hw_counter) = (current_val); \
	} while(0);


//register
struct regs {
	u32 reg;
	u32 val;
};

struct rf_regs_tbls_t {
	struct regs power_down;
	struct regs enable;
};

struct wmac_regs_tbls_t {
	struct regs rtscts_set;
	struct regs inr_trg_tick_num;
	struct regs rx_block_size;
	struct regs buff_pool_cntl;
	struct regs mac_llc_offset;
	struct regs bssid_cons_chk_en;
	struct regs rx_ack_policy;
	struct regs rxdma_ctrl;
	struct regs seceng_ctrl;
	struct regs lsig_rate;
	struct regs ofdm_defer_set;
	struct regs phydly_set;
	struct regs err_en;
	struct regs fc_acq_threshold;
	struct regs fc_public_threshold;
	struct regs da_miss_tocpu;
	struct regs rx_mpdu_maxsize;
	struct regs rx_filter;
	struct regs cw_set;
	struct regs aifs_set;
	struct regs txop_limit;
	struct regs acq_en;
	struct regs acq_intrm;
	struct regs acq_intrm2;
	struct regs sta_ds_table_cfg;
	struct regs lu_table_max;
	struct regs basic_set;			/* channel */
	struct regs bssid0_info;
	struct regs bssid0_info2;
	struct regs bssid1_info;
	struct regs bssid1_info2;
	struct regs bssid2_info;
	struct regs bssid2_info2;

	/* all configuration should done before LMAC start */
	struct regs umac_tx_cntl;
	struct regs uprx_en; 
	struct regs lmac_cntl;
	struct regs mac_int_mask;

	struct regs pre_tbtt;
	struct regs ts0_be;
	struct regs ts1_be;
	struct regs ts2_be;
};

extern struct rf_regs_tbls_t *rf_regs_tbls_p;
extern struct wmac_regs_tbls_t *wmac_regs_tbls_p;

#define WMAC_SET_SLOTTIME(_n) \
			MACREG_UPDATE32(OFDM_DEFER_SET, (_n << 24), 0xff000000) 

#define WMAC_SET_RTS_THRESHOLD(_n) \
			MACREG_UPDATE32(RTSCTS_SET, (_n<<16), LMAC_RTS_CTS_THRESHOLD)

#define WMAC_SET_POWER_SAVING_MODE(_n) \
		MACREG_WRITE32(PS_FUNC_CTRL, _n)

#define WMAC_SET_RX_LINKINFO_CACHE_SYNC() \
		MACREG_WRITE32(RX_LINK_INFO_CACHE_CTRL, RX_LINK_INFO_CACHE_OUT)

#define WMAC_REKEY_DONE() \
		MACREG_WRITE32(SEC_KEY_CTRL, 0)

#define WMAC_GET_DTIM_COUNTER() \
		(unsigned char)((MACREG_READ32(DTIM_INTERVAL_REG) & DTIM_INVERVAL_REG_DTIM_COUNTER) >> 8)

#define WMAC_GET_BEACON_COUNTER(_n) \
		(unsigned char)((MACREG_READ32(TS0_BE_LOST_COUNT + (_n<<3)) & TS0_BEACON_RX_COUNT) >> 16)

#define WMAC_INVALIDATE_AMPDU_SCOREBOARD_CACHE(_m, _n) \
		MACREG_WRITE32(AMPDU_BMG_CTRL, (BMG_CLEAR | (_n << 8) | (_m)));

#define TXQ_SZ_ENCODE(s)		(ffs(s) - 4)	/* 8 -> 0, 16 -> 1, 32 -> 2, 64 -> 3 */
#if 0
#define DATA_READ(data,mask,shift)			((data & mask) >> shift)
#define DATA_WRITE(data,mask,shift)			((data << shift) & mask)
#define DATA_UPDATE(data,field,mask,shift) \
		do { \
			data &= ~(mask); \
			data |= (((u32)field << shift) & mask); \
		} while(0)
#endif

#define RDESC_TAIL(_d)				((*(u32 *)_d & RX_BHDRTAIL) >> RX_SHIFT_BHDRTAIL)
#define RDESC_HEAD(_d)				((*(u32 *)_d & RX_BHDRHEAD) >> RX_SHIFT_BHDRHEAD)

#define TDESC_OWN(_d)				((*(u32 *)_d & TX_OWN) >> TX_SHIFT_OWN)
#define TDESC_BMAP(_d)				((*(u32 *)_d & TX_BMAP) >> TX_SHIFT_BMAP)
#define TDESC_PS(_d)				((*(u32 *)_d & TX_PS) >> TX_SHIFT_PS)
#define TDESC_MPDU(_d)				((*(u32 *)_d & TX_MPDU) >> TX_SHIFT_MPDU)
#define TDESC_TRYCNT(_d)			((*(u32 *)_d & TX_TRY_CNT) >> TX_SHIFT_TRY_CNT)
#define TDESC_NOA(_d)				((*(u32 *)_d & TX_NOA) >> TX_SHIFT_NOA)
#define TDESC_MB(_d)				((*(u32 *)_d & TX_MB) >> TX_SHIFT_MB)
#define TDESC_RATE(_d)				((*(u32 *)_d & TX_RATE) >> TX_SHIFT_RATE)
#define TDESC_TID(_d)				((*(u32 *)_d & TX_TID) >> TX_SHIFT_TID)
#define TDESC_ADDRIDX(_d)			((*(u32 *)_d & TX_AIDX) >> TX_SHIFT_AIDX)
#define TDESC_PKTLEN(_d)			((*(u32 *)_d & TX_PKTLEN) >> TX_SHIFT_PKTLEN)
#define TDESC_HEAD(_d)				((*(u32 *)_d & TX_BHDRHEAD) >> TX_SHIFT_BHDRHEAD)

#define OWN_TO_TDESC(_v)			((_v << TX_SHIFT_OWN) & TX_OWN)
#define BMAP_TO_TDESC(_v)			((_v << TX_SHIFT_BMAP) & TX_BMAP)
#define MPDU_TO_TDESC(_v)			((_v << TX_SHIFT_MPDU) & TX_MPDU)
#define TID_TO_TDESC(_v)			((_v << TX_SHIFT_TID) & TX_TID)
#define ADDRIDX_TO_TDESC(_v)		((_v << TX_SHIFT_AIDX) & TX_AIDX)
#define PKTLEN_TO_TDESC(_v)			((_v << TX_SHIFT_PKTLEN) & TX_PKTLEN)
#define HEAD_TO_TDESC(_v)			((_v << TX_SHIFT_BHDRHEAD) & TX_BHDRHEAD)

#define TDESC_OWN_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_OWN); \
			*(u32 *)_d |= OWN_TO_TDESC(_v); \
		} while(0)

#define TDESC_BMAP_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_BMAP); \
			*(u32 *)_d |= BMAP_TO_TDESC(_v); \
		} while(0)

#define TDESC_MPDU_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_MPDU); \
			*(u32 *)_d |= MPDU_TO_TDESC(_v); \
		} while(0)

#define TDESC_TID_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_TID); \
			*(u32 *)_d |= TID_TO_TDESC(_v); \
		} while(0)

#define TDESC_ADDRIDX_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_AIDX); \
			*(u32 *)_d |= ADDRIDX_TO_TDESC(_v); \
		} while(0)

#define TDESC_PKTLEN_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_PKTLEN); \
			*(u32 *)_d |= PKTLEN_TO_TDESC(_v); \
		} while(0)

#define TDESC_BHDRHEAD_UPDATE(_d,_v) \
		do { \
			*(u32 *)_d &= ~(TX_BHDRHEAD); \
			*(u32 *)_d |= HEAD_TO_TDESC(_v); \
		} while(0)

#define BCAP_VALID(_b)		(((sta_basic_info *)_b)->val & BCAP_VLD)
#define BCAP_AMPDU(_b)		((((sta_basic_info *)_b)->val & BCAP_RX_AMPDU) >> BCAP_SHIFT_RX_AMPDU)
#define BCAP_BSSIDX(_b)		((((sta_basic_info *)_b)->val & BCAP_BSSID) >> BCAP_SHIFT_BSSID)
#define BCAP_TBLIDX(_b)		((((sta_basic_info *)_b)->val & BCAP_CAPTBL_IDX) >> BCAP_SHIFT_CAPTBL_IDX)

#define BCAP_WEP_DEFKEY_CLR(_b) \
		do { \
			((sta_basic_info *)_b)->val &= ~BCAP_WEP_DEFKEY; \
		} while(0)

#define BCAP_AMPDU_SET(_b,_v) \
		do { \
			((sta_basic_info *)_b)->val |= (_v << BCAP_SHIFT_RX_AMPDU); \
		} while(0)

#define BCAP_AMPDU_CLR(_b,_v) \
		do { \
			((sta_basic_info *)_b)->val &= ~(_v << BCAP_SHIFT_RX_AMPDU); \
		} while(0)

struct wlan_rate {
	unsigned int flags;
	unsigned int perfect_tp;		/* throughput of 100% successful transmission */
	unsigned short bitrate;
	unsigned short hw_value;
};

#define RATE_FLAGS_HT_SGI20			0x1
#define RATE_FLAGS_HT_SGI40			0x2
#define RATE_FLAGS_HT_40M			0x4
#define RATE_FLAGS_HT_GF			0x8
#define RATE_FLAGS_SHORT_PREAMBLE 	0x10
#define RATE_FLAGS_INIT				0x80

#define TXQ_FLG_AMPDU				0x0001
#define TXQ_FLG_AMPDU_FORCETO_MPDU	0x0002
#define TXQ_FLG_LOCK				0x0010
#define TXQ_FLG_FREE				0x0020
#define TXQ_FLG_USED				0x0040
#define TXQ_FLG_SCHEDULE			0x0080
#define TXQ_FLG_BLOCK_DTIM			0x0100
#define TXQ_FLG_RECYCLE				0x0200
#define TXQ_FLG_RECYCLE_DONE		0x0400
#define TXQ_FLG_PSTXQ				0x0800

struct rx_ba_session {
	u16 win_start;
	u8 win_size;
	u8 stored_num;
	struct wbuf *reorder_q_head;
	struct wbuf *reorder_q_tail;
};

/* BA's command */
enum{
	BA_SESSION_INITIATION,
	BA_SESSION_TEARDOWN,
};

struct rate_stats{
	u32 attempts;
	u32 success;
};

#define TS_STATE_INIT		BIT(0)
#define TS_STATE_SYNCED		BIT(1)


#define APSD_AC_BE_TRIGGER		0x01
#define APSD_AC_BK_TRIGGER		0x02
#define APSD_AC_VI_TRIGGER		0x04
#define APSD_AC_VO_TRIGGER		0x08
#define APSD_AC_BE_DELIVERY		0x10
#define APSD_AC_BK_DELIVERY		0x20
#define APSD_AC_VI_DELIVERY		0x40
#define APSD_AC_VO_DELIVERY		0x80

#define APSD_AC_ALL_DELIVERY	(APSD_AC_BE_DELIVERY|APSD_AC_BK_DELIVERY|APSD_AC_VI_DELIVERY|APSD_AC_VO_DELIVERY)


#define diag_dump_buf_32bit(a,b)	reg_dump((unsigned char *)a, (int)b)
#define diag_dump_buf_16bit(a,b)	reg_dump((unsigned char *)a, (int)b)

#define IS_TXQ_CAN_SEND(q)			(q->widx != q->ssn)
#define IS_TXQ_EMPTY(q)				(q->widx == q->ridx)
#define IS_TXQ_FULL(q)				((q->widx >= q->ridx) ? \
									((q->widx - q->ridx) == TX_DESCRIPTOR_COUNT) : \
									((0x1000 - q->ridx + q->widx) == TX_DESCRIPTOR_COUNT))
#define TXQ_DESC(q, ptr)			(&q->desc[ptr & (TX_DESCRIPTOR_COUNT-1)])
#define TXQ_PRE_IDX(ptr)			((ptr != 0) ? (ptr - 1) : (0x1000 - 1))
#define TXQ_INC_ONE(idx)			(idx = ((idx + 1) & 0xfff))		/* 0-4095 */
#define TXQ_PKT_NUM(q)				(((q->widx - q->ssn) <= TX_DESCRIPTOR_COUNT) ? \
									(q->widx - q->ssn) : \
									(0x1000 - q->ssn + q->widx))

#define IS_PRE_BAQ(q)				(q->mode == TXQ_PRE_AMPDU)
#define IS_BAQ(q)					(q->mode == TXQ_AMPDU)
#define IS_PSQ(q)					((q->mode == TXQ_PS) || (q->mode == TXQ_PS_DRAIN))
#define IS_WORKQ(q)					((q->mode == TXQ_AMPDU) || (q->mode == TXQ_MPDU))
#define IS_IDLEQ(q)					(q->mode == TXQ_IDLE)
#define IS_EDCAQ(idx)				(idx < MAC_MAX_EDCAQ)
#define IS_BCMCQ(idx)				(idx >= MAC_MAX_PSBAQ)
#define IS_GREATER_THAN(a, b)		(((a > b) && ((a - b) <= TX_DESCRIPTOR_COUNT)) || \
									((b > a) && ((0x1000 - b + a) <= TX_DESCRIPTOR_COUNT)))
#define IS_FREEQ(q)					(q->flag & TXQ_FLG_FREE)
#define IS_LOCKQ(q)					(q->flag & TXQ_FLG_LOCK)

#define WACL_ENABLE					1
#define WLAN_CHAN_DISABLED			0x1

#define AP_CAP_11B					BIT(0)
#define AP_CAP_11G					BIT(1)
#define AP_CAP_11N					BIT(2)
#define AP_CAP_11A					BIT(3)

#define CONFIG_DISCONNECT_DBG 1
#ifdef CONFIG_DISCONNECT_DBG
struct disconnect_dbg
{
	unsigned int interface_del;
	unsigned int host_disconnect;
	unsigned int ap_disconnect;
	unsigned int keep_alive;
};

extern struct disconnect_dbg *connect_dbg;
#endif

struct ie_entry
{
	u16	offset;
	u16	length;
};

void wmac_set_monitor(u32 mon_bit);
void wmac_if_ctrl(int idx, int ctrl);
int wmac_mac_start(void);
int wmac_mac_stop(void);
int wmac_addr_lookup_engine_find(u8 *addr, int addr_index, int *basic_cap, char flag);
int wmac_addr_lookup_engine_update(u32 addr, u32 basic_cap, u8 flag);
int wmac_addr_lookup_engine_flush(void);
void find_all_addrs_and_update(u8 captbl_idx, u32 basic_cap);
void wmac_reset(void);
void wmac_free_buffer_hdr(u32 bhdr_h, u32 bhdr_t);

void wmac_set_key(cipher_key *hwkey, char *key, char cipher_type, char keyidx, char is_txkey);
void wmac_rekey_start(int index, int key_type, char cipher_type);

struct sta_ctx *wmac_update_ps_state(int is_pspoll);

void wmac_set_protection(u8 type);
void wmac_set_channel(u8 ch, u8 bw);
void wmac_start_tsync(struct wm_bss *bss, u32 timeout);
void wmac_set_beacon_interrupt(struct wm_bss *bss, u32 en);

u32 wmac_get_cca(void);
char ds_table(struct sta_ctx *sta);
void wmac_cipher_key(struct sta_ctx *sta, unsigned char cipher_type, unsigned char key_type,
						char *key, char keyidx, char is_txkey);

void wmac_tx_power(u32 level);
ehdr *wframe_format(void);

/* rate adaption */
unsigned char get_hw_bitrate_format(int tx_rate_idx);
inline unsigned char get_hw_bitrate_code(int tx_rate_idx);
unsigned int get_hw_bitrate(int tx_rate_idx);
unsigned char primary_ch_offset(u8 bw_type);
short setup_rate_encode(int tx_rate_idx, unsigned char retry, unsigned char flags);
void wla_rc_init(struct sta_ctx *sta);
void wla_rc_statistics(struct sta_ctx *sta, volatile u32 *txd, unsigned char *bts_rates);
void wla_rc_new_divisor(struct sta_ctx *sta);
void wla_rc_update(struct sta_ctx *sta);
#if defined(WLAN_RC_BTS)
void dump_bts(int sta_idx);
#endif

/* QoS */
void wmac_ac_paramters(void);

/* beacon */
void wla_beacon_setup(struct wm_bss *bss);
void wla_beacon_start(struct wm_bss *bss);
void wmac_beacon_stop(struct wm_bss *bss);
void wmac_beacon_stop_handler(void);
u32 wmac_read_beacon_rx_count(u32 bss_idx);
void wla_beacon(void);
void wla_update_next_tbtt(u32 intr_mask);
u32 wmac_busy_timestamp(u32 bss_desc, u32 timestamp, u32 act);
void client_tbtt_handle(u32 bss_desc);
u32 wmac_get_peer_ps_sm(struct sta_ctx *sta);
void wmac_set_peer_ps_sm(struct sta_ctx *sta, int flag);
u8 wmac_get_my_ps_sm(u8 bss_desc);
int wmac_set_my_ps_sm(u8 bss_desc, u8 state);

void wmac_set_txq_sm(struct tx_q *q, u8 state, u8 txq_is_sta);
void wmac_software_intr(int *parm);
int wmac_release_sta(struct sta_ctx *sta);
char wmac_setup_sta(struct sta_ctx *sta);
struct tx_q *wmac_new_psbaq(void);
void wmac_free_psbaq(void);
void wmac_txq_init(void);
struct wm_bss *wmac_tsfidx_to_bss(unsigned char tsf_idx);
void wmac_bss_to_tsfidx(struct wm_bss *bss);
void wmac_cfg_wif(u32 idx);
void wmac_acq_ctrl(int val, int open);
void wmac_update_slottime(void);
#ifdef CONFIG_WLA_DEBUG_SWBUF
void wmac_swbuf_check_nonmatch(int i, char c);
void wmac_swbuf_check_match(int i, char c);
void wmac_swbuf_check_rx_range(int i);
void wmac_swbuf_check_tx_range(int i);
void wmac_swbuf_update(int i, char c);
void wmac_swbuf_ra_update(int i, int ra);
void wmac_bhdr_get_handler(int idx);
void wmac_bhdr_insert_handler(int idx, int ret_addr);
#endif

/* debug */
void reg_dump(unsigned char *addr,int size);
void dump_list(short head, short tail, short low, short high);
void dump_bcap(int index, char is_ds);
void dump_sta(int sta_idx);
void dump_bss(int bss_idx);
void dump_ba(int sta_idx);
void dump_bc_qinfo(void);
#ifdef TKIP_COUNTERMEASURE
void dump_ptk(int cmd);
#endif
void inc_ps_counter(unsigned int i);

extern u8 temp_buf[];
#else
#include <panther_dev.h>
#include <lib_autoconf.h>

enum {
	BW40MHZ_SCN = 0,	/* no secondary channel is present */
	BW40MHZ_SCA = 1,	/* secondary channel is above the primary channel */
	BW40MHZ_SCB = 3,	/* secondary channel is below the primary channel */
	BW40MHZ_AUTO = 4,	/* auto select secondary channel */
};

/* rate format encoding */
#define FMT_NO_HT  0
#define FMT_HT     1
#define FMT_HT_GF  2
#define FMT_11B    3

#define CCK_1M_CODE            0x0
#define CCK_2M_CODE            0x1
#define CCK_5_5M_CODE          0x2
#define CCK_11M_CODE           0x3
#define FMT_11B_SHORT_PREAMBLE 0x4

#define OFDM_6M_CODE    0xb
#define OFDM_9M_CODE    0xf
#define OFDM_12M_CODE   0xa
#define OFDM_18M_CODE   0xe
#define OFDM_24M_CODE   0x9
#define OFDM_36M_CODE   0xd
#define OFDM_48M_CODE   0x8
#define OFDM_54M_CODE   0xc

#define MCS_0_CODE  0
#define MCS_1_CODE  1
#define MCS_2_CODE  2
#define MCS_3_CODE  3
#define MCS_4_CODE  4
#define MCS_5_CODE  5
#define MCS_6_CODE  6
#define MCS_7_CODE  7

#define B_RATE  0x1
#define G_RATE  0x2
#define HT_RATE 0x4
struct wlan_rate {
	unsigned int flags;
	unsigned short hw_value;
};

#define WACL_ENABLE					1
#define WLAN_CHAN_DISABLED			0x1

/* definition of flags */
enum {
	WBUF_FIXED_SIZE				= BIT(0),
	WBUF_RX_FRAME				= BIT(1),		/* indicate the buffer is received from wlan */

	WBUF_TX_REQ_STATUS			= BIT(2),
	WBUF_TX_NO_ACK				= BIT(3),		/* this frame does not need ACK */
	WBUF_TX_NO_SECU				= BIT(4),		/* indicate the frame does not encrypt */
	WBUF_TX_SPEC_RATE			= BIT(5),		/* force to send out in specified rate */
	WBUF_TX_BASIC_RATE			= BIT(6),		/* force to send out in basic rate */
	WBUF_TX_CHECK_PS			= BIT(7),
	WBUF_TX_INSERT_TS			= BIT(8),		/* inform HW insert Timestamp field */

	WBUF_TX_PS_NULL				= BIT(9),		/* indicate send the NULL frame with PM bit */

	WBUF_TX_REPORT_STATUS		= BIT(10),		/* indicate the wbuf is TX status */
	WBUF_TX_SUCCESS				= BIT(11),		/* report transmition is success */

	WBUF_MGMT_FRAME				= BIT(12),		/* management frame */
	WBUF_CHAIN					= BIT(13),		/* wbuf link list */
	WBUF_EXT					= BIT(14),
	WBUF_TX_DIS_DURATION        = BIT(15),      /* disable hw tx duration config */
	WBUF_TX_ASSOC				= BIT(16),

	WBUF_TX_NULL				= BIT(17),		/* indicate send the NULL frame */
	WBUF_TX_EAPOL				= BIT(18),		/* indicate send the eapol */
	WBUF_TX_UAPSD_PS_NULL		= BIT(19),		/* indicate U-APSD QOS NULL frame */
	WBUF_TX_ADDBA				= BIT(20),		/* ADDBA */
	WBUF_TX_DELBA				= BIT(21),		/* DELBA */
	WBUF_TX_CTRL_BAR			= BIT(22),		/* BAR */
	WBUF_TX_CTRL_PSPOLL			= BIT(23),		/* PSPOLL */
	WBUF_TX_RESEND_COUNT		= BIT(27) | BIT(26) | BIT(25) | BIT(24),
};

#define RATE_FLAGS_HT_SGI20			0x1
#define RATE_FLAGS_HT_SGI40			0x2
#define RATE_FLAGS_HT_40M			0x4
#define RATE_FLAGS_HT_GF			0x8
#define RATE_FLAGS_SHORT_PREAMBLE 	0x10
#define RATE_FLAGS_INIT				0x80
#endif
 
#endif // __MAC_INFO_H__

