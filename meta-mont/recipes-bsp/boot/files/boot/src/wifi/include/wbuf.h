/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wbuf.h
*   \brief 
*   \author Montage
*/

#ifndef __WBUF_H__
#define __WBUF_H__

#include <mt_types.h>

#define DEF_SW_PATH_BUF_SIZE					512

/* Wi-Fi MAC TX buffer */
struct wbuf {
	union {
		u32 word[8];
		u8 byte[32];
	} cb;
	
	/* software used */
	struct wbuf *rb_next;	/* reorder buffer link list */
	u32 timestamp;
	u16 tail;
	u8	type;
	u8	channel;
	struct wbuf *wb_next;
};

#define rcb_w0						cb.word[0]
#define rcb_w1						cb.word[1]
#define rcb_w2						cb.word[2]
#define rcb_w3						cb.word[7]

#define WRB_W0_HOST					0x80000000UL
#define WRB_W0_WH					0x40000000UL
#define WRB_W0_LLC					0x20000000UL
#define WRB_W0_IPCSOK				0x10000000UL
#define WRB_W0_TCPCSOK				0x08000000UL
#define WRB_W0_BA					0x04000000UL
#define WRB_W0_FMDS					0x02000000UL
#define WRB_W0_TODS					0x01000000UL
#define WRB_W0_AMSDU				0x00800000UL
#define WRB_W0_BC					0x00400000UL
#define WRB_W0_MC					0x00200000UL
#define WRB_W0_QOS					0x00100000UL
#define WRB_W0_RETRY				0x00080000UL
#define WRB_W0_PS					0x00040000UL
#define WRB_W0_EOSP					0x00020000UL
#define WRB_W0_MDAT					0x00010000UL
#define WRB_W0_RAIDX				0x0000E000UL
#define WRB_W0_MFRG					0x00001000UL 		// more fragment
#define WRB_W0_TID					0x00000F00UL
#define WRB_W0_SAIDX				0x000000FFUL

#define WRB_W0_SHIFT_HOST			31
#define WRB_W0_SHIFT_WH				30
#define WRB_W0_SHIFT_LLC			29
#define WRB_W0_SHIFT_IPCSOK			28
#define WRB_W0_SHIFT_TCPCSOK		27
#define WRB_W0_SHIFT_BA				26
#define WRB_W0_SHIFT_FMDS			25
#define WRB_W0_SHIFT_TODS			24
#define WRB_W0_SHIFT_AMSDU			23
#define WRB_W0_SHIFT_BC				22
#define WRB_W0_SHIFT_MC				21
#define WRB_W0_SHIFT_QOS			20
#define WRB_W0_SHIFT_RETRY			19
#define WRB_W0_SHIFT_PS				18
#define WRB_W0_SHIFT_EOSP			17
#define WRB_W0_SHIFT_MDAT			16
#define WRB_W0_SHIFT_RAIDX			13
#define WRB_W0_SHIFT_MFRG			12
#define WRB_W0_SHIFT_TID			8
#define WRB_W0_SHIFT_SAIDX			0
	
#define WRB_W1_AMPDU				0x80000000UL
#define WRB_W1_RES_30				0x40000000UL
#define WRB_W1_PKTLEN				0x3FFF0000UL
#define WRB_W1_SEQNUM				0x0000FFF0UL
#define WRB_W1_FRGNUM				0x0000000FUL

#define WRB_W1_SHIFT_AMPDU			31
#define WRB_W1_SHIFT_RES_30			30
#define WRB_W1_SHIFT_PKTLEN			16
#define WRB_W1_SHIFT_SEQNUM			4
#define WRB_W1_SHIFT_FRGNUM			0

#define WRB_W2_SECST				0xF0000000UL
#define WRB_W2_HIT					0x0F000000UL
	#define WRB_W2_DA_HIT_ADDR		0x01000000UL
	#define WRB_W2_DA_HIT_BSSID		0x02000000UL
	#define WRB_W2_SA_HIT_ADDR		0x04000000UL
	#define WRB_W2_TA_HIT_DS		0x08000000UL
#define WRB_W2_DATAOFF				0x00FF0000UL
#define WRB_W2_RSSI					0x0000FF00UL
#define WRB_W2_RATE					0x000000FFUL

#define WRB_W2_SHIFT_SECST			28
#define WRB_W2_SHIFT_HIT			24
#define WRB_W2_SHIFT_DATAOFF		16
#define WRB_W2_SHIFT_RSSI			8
#define WRB_W2_SHIFT_RATE			0

#define WRB_W3_SNR					0xFF000000UL
#define WRB_W3_SGI					0x00100000UL
#define WRB_W3_CBW					0x00080000UL
#define WRB_W3_B_SP					0x00040000UL
#define WRB_W3_FORMAT				0x00030000UL

#define WRB_W3_SHIFT_SNR	 		24
#define WRB_W3_SHIFT_SGI	 		20
#define WRB_W3_SHIFT_CBW	 		19
#define WRB_W3_SHIFT_B_SP	 		18
#define WRB_W3_SHIFT_FORMAT			16

#define WBQ_PUT(q, wb, next) { \
	(wb)->next = 0; \
	if((q)->tail == 0) \
		(q)->head = (wb); \
	else \
		(q)->tail->next = (wb); \
	(q)->tail = wb; \
	(q)->len++; \
}

#define WBQ_GET(q, wb, next) { \
	(wb) = q->head; \
	if(wb) { \
		if(((q)->head = (wb)->next) == 0) \
			(q)->tail = 0; \
		(wb)->next = 0; \
		(q)->len--; \
	} \
}

#define WBQ_IS_EMPTY(q)		((q)->len == 0)

#define IS_WDS(_w)			((((struct wbuf *)_w)->rcb_w0 & (WRB_W0_FMDS|WRB_W0_TODS)) == (WRB_W0_FMDS|WRB_W0_TODS))

#define WB_BROADCAST(_w)	(((struct wbuf *)_w)->rcb_w0 & WRB_W0_BC)
#define WB_MULTICAST(_w)	(((struct wbuf *)_w)->rcb_w0 & WRB_W0_MC)
#define WB_GROUPCAST(_w)	(((struct wbuf *)_w)->rcb_w0 & (WRB_W0_MC|WRB_W0_BC))

#define WB_WH(_w)			(((struct wbuf *)_w)->rcb_w0 & WRB_W0_WH)
#define WB_AMSDU(_w)		(((struct wbuf *)_w)->rcb_w0 & WRB_W0_AMSDU)
#define WB_QOS(_w)			(((struct wbuf *)_w)->rcb_w0 & WRB_W0_QOS)
#define WB_PS(_w)			(((struct wbuf *)_w)->rcb_w0 & WRB_W0_PS)
#define WB_EOSP(_w)			(((struct wbuf *)_w)->rcb_w0 & WRB_W0_EOSP)
#define WB_MDAT(_w)			(((struct wbuf *)_w)->rcb_w0 & WRB_W0_MDAT)
#define WB_BSSDESC(_w)		((((struct wbuf *)_w)->rcb_w0 & WRB_W0_RAIDX) >> WRB_W0_SHIFT_RAIDX)
#define WB_MFRG(_w)			(((struct wbuf *)_w)->rcb_w0 & WRB_W0_MFRG)
#define WB_TID(_w)			((((struct wbuf *)_w)->rcb_w0 & WRB_W0_TID) >> WRB_W0_SHIFT_TID)
#define WB_SAIDX(_w)		((((struct wbuf *)_w)->rcb_w0 & WRB_W0_SAIDX) >> WRB_W0_SHIFT_SAIDX)
#define WB_AMPDU(_w)		(((struct wbuf *)_w)->rcb_w1 & WRB_W1_AMPDU)
#define WB_PKTLEN(_w)		((((struct wbuf *)_w)->rcb_w1 & WRB_W1_PKTLEN) >> WRB_W1_SHIFT_PKTLEN)
#define WB_SEQNUM(_w)		((((struct wbuf *)_w)->rcb_w1 & WRB_W1_SEQNUM) >> WRB_W1_SHIFT_SEQNUM)
#define WB_FRGNUM(_w)		((((struct wbuf *)_w)->rcb_w1 & WRB_W1_FRGNUM) >> WRB_W1_SHIFT_FRGNUM)
#define WB_SECST(_w)		((((struct wbuf *)_w)->rcb_w2 & WRB_W2_SECST) >> WRB_W2_SHIFT_SECST)
#define WB_HIT(_w)			((((struct wbuf *)_w)->rcb_w2 & WRB_W2_HIT) >> WRB_W2_SHIFT_HIT)
#define WB_HIT_ADDR(_w)		((((struct wbuf *)_w)->rcb_w2 & (WRB_W2_SA_HIT_ADDR|WRB_W2_TA_HIT_DS)))
#define WB_SA_HIT_ADDR(_w)	((((struct wbuf *)_w)->rcb_w2 & WRB_W2_SA_HIT_ADDR))
#define WB_TA_HIT_DS(_w)	((((struct wbuf *)_w)->rcb_w2 & WRB_W2_TA_HIT_DS))
#define WB_DATAOFF(_w)		((((struct wbuf *)_w)->rcb_w2 & WRB_W2_DATAOFF) >> WRB_W2_SHIFT_DATAOFF)
#define WB_RSSI(_w)			((((struct wbuf *)_w)->rcb_w2 & WRB_W2_RSSI) >> WRB_W2_SHIFT_RSSI)
#define WB_SNR(_w)			((((struct wbuf *)_w)->rcb_w3 & WRB_W3_SNR) >> WRB_W3_SHIFT_SNR)

#define WH_TO_WB(_v)		((_v << WRB_W0_SHIFT_WH) & WRB_W0_WH)
#define BSSDESC_TO_WB(_v)	((_v << WRB_W0_SHIFT_RAIDX) & WRB_W0_RAIDX)
#define PKTLEN_TO_WB(_v)	((_v << WRB_W1_SHIFT_PKTLEN) & WRB_W1_PKTLEN)
#define SEQNUM_TO_WB(_v)	((_v << WRB_W1_SHIFT_SEQNUM) & WRB_W1_SEQNUM)
#define DATAOFF_TO_WB(_v)	((_v << WRB_W2_SHIFT_DATAOFF) & WRB_W2_DATAOFF)

#define WB_WH_UPDATE(_w,_v) \
		do { \
			_w->rcb_w0 &= ~(WRB_W0_WH); \
			_w->rcb_w0 |= WH_TO_WB(_v); \
		} while(0)

#define WB_PKTLEN_UPDATE(_w,_v) \
		do { \
			_w->rcb_w1 &= ~(WRB_W1_PKTLEN); \
			_w->rcb_w1 |= PKTLEN_TO_WB(_v); \
		} while(0)

#define WB_SEQNUM_UPDATE(_w,_v) \
		do { \
			_w->rcb_w1 &= ~(WRB_W1_SEQNUM); \
			_w->rcb_w1 |= SEQNUM_TO_WB(_v); \
		} while(0)

#define WB_DATAOFF_UPDATE(_w,_v) \
		do { \
			_w->rcb_w2 &= ~(WRB_W2_DATAOFF); \
			_w->rcb_w2 |= DATAOFF_TO_WB(_v); \
		} while(0)

#define DPTR_TO_WB(_p)		((struct wbuf *)NONCACHED_ADDR(_p))
#define WB_TO_DPTR(_w)		((char *)PHYSICAL_ADDR((int)_w))
#define WB_TO_LLC(_w)		((char *)((int)_w + WLAN_RX_LLC_OFFSET))

#define WTB_SET_EOSP				BIT(4)	/* match wtb w2 bit location */
#define WTB_SET_ACK_NORMAL			0		/* ACK POLICY in wtb w2 BIT(5)|BIT(6) */
#define WTB_SET_ACK_NOACK			BIT(5)	/* match wtb w2 bit location */
#define WTB_SET_TODS				BIT(8)	/* match wtb w2 bit location */
#define WTB_SET_FRDS				BIT(9)	/* match wtb w2 bit location */
#define WTB_SET_PM					BIT(12)	/* match wtb w2 bit location */
#define WTB_SET_SEC					BIT(14)	/* match wtb w2 bit location */
#define WTB_SET_TS					BIT(18)	/* match wtb w2 bit location, insert timestamp */
#define WTB_SET_LLC					BIT(20)	/* match wtb w2 bit location */
#define WTB_SET_QOS					BIT(21)	/* match wtb w2 bit location */
#define WTB_SET_BCMC				BIT(30)	/* match wtb w2 bit location */

/* Wi-Fi MAC TX buffer */
struct wtbuf {
	/* word 0 */
	u16 bhdr_t;
	u16 bhdr_h;
	
	union {
		u32 word[3];
		u8 byte[12];
	} cb;

	u16	tx_rate[4];		/* word 4, 5 */
	u32	tsc_w[4];		/* word 6,7,8,9 */
	u8 bts_rates[4];
	u32 flags;
};

#define tcb_w0						cb.word[0]
#define tcb_w1						cb.word[1]
#define tcb_w2						cb.word[2]

#define WTB_VALID_RC				0x80
#define WTB_VALID_SAMPLE			0x40
#define WTB_BCQ_EOP					0x8000

#define WTB_W0_WH					0x80000000UL
#define WTB_W0_BCMC					0x40000000UL
#define WTB_W0_TS					0x20000000UL
#define WTB_W0_PKTLEN				0x1FFF0000UL
#define WTB_W0_SN					0x0000FFF0UL
#define WTB_W0_FRGNUM				0x0000000FUL

#define WTB_W0_SHIFT_WH				31
#define WTB_W0_SHIFT_BCMC			30
#define WTB_W0_SHIFT_TS				29
#define WTB_W0_SHIFT_PKTLEN			16
#define WTB_W0_SHIFT_SN				4
#define WTB_W0_SHIFT_FRGNUM			0

#define WTB_W1_GSN					0x80000000UL
#define WTB_W1_RTS					0x40000000UL
#define WTB_W1_HDR_LEN				0x3F000000UL
#define WTB_W1_NO_DUR				0x00800000UL
#define WTB_W1_LLC					0x00400000UL
#define WTB_W1_QOS					0x00200000UL
#define WTB_W1_NMIC					0x00100000UL
#define WTB_W1_CIPHER				0x000F0000UL
#define WTB_W1_ORDER				0x00008000UL
#define WTB_W1_SEC					0x00004000UL
#define WTB_W1_MD					0x00002000UL
#define WTB_W1_PM					0x00001000UL
#define WTB_W1_RETRY				0x00000800UL
#define WTB_W1_MFRG					0x00000400UL
#define WTB_W1_FRDS					0x00000200UL
#define WTB_W1_TODS					0x00000100UL
#define WTB_W1_AMSD					0x00000080UL
#define WTB_W1_ACK					0x00000060UL
#define WTB_W1_EOSP					0x00000010UL
#define WTB_W1_TID					0x0000000FUL

#define WTB_W1_SHIFT_GSN			31
#define WTB_W1_SHIFT_RTS			30
#define WTB_W1_SHIFT_HDR_LEN		24
#define WTB_W1_SHIFT_DUR			23
#define WTB_W1_SHIFT_LLC			22
#define WTB_W1_SHIFT_QOS			21
#define WTB_W1_SHIFT_NMIC			20
#define WTB_W1_SHIFT_CIPHER			16
#define WTB_W1_SHIFT_ORDER			15
#define WTB_W1_SHIFT_SEC			14
#define WTB_W1_SHIFT_MD				13
#define WTB_W1_SHIFT_PM				12
#define WTB_W1_SHIFT_RETRY			11
#define WTB_W1_SHIFT_MFRG			10
#define WTB_W1_SHIFT_FRDS			9
#define WTB_W1_SHIFT_TODS			8
#define WTB_W1_SHIFT_AMSD			7
#define WTB_W1_SHIFT_ACK			5
#define WTB_W1_SHIFT_EOSP			4
#define WTB_W1_SHIFT_TID			0

#define WTB_W2_WEP					0x80000000UL
#define WTB_W2_REKEY				0x40000000UL
#define WTB_W2_KEYIDX				0x3F000000UL
#define WTB_W2_DSIDX				0x00F80000UL
#define WTB_W2_BSSID				0x00070000UL
#define WTB_W2_TXPWR				0x0000F000UL
#define WTB_W2_SP					0x00000800UL
#define WTB_W2_SMT					0x00000400UL
#define WTB_W2_NESS					0x00000300UL
#define WTB_W2_NHTM					0x000000C0UL
#define WTB_W2_ANT					0x00000030UL
#define WTB_W2_FEC					0x00000008UL
#define WTB_W2_NSD					0x00000004UL
#define WTB_W2_STBC					0x00000003UL

#define WTB_W2_SHIFT_WEP			31
#define WTB_W2_SHIFT_REKEY			30
#define WTB_W2_SHIFT_KEYIDX			24
#define WTB_W2_SHIFT_DSIDX			19
#define WTB_W2_SHIFT_BSSID			16
#define WTB_W2_SHIFT_TXPWR			12
#define WTB_W2_SHIFT_SP				11
#define WTB_W2_SHIFT_SMT			10
#define WTB_W2_SHIFT_NESS			8
#define WTB_W2_SHIFT_NHTM			6
#define WTB_W2_SHIFT_ANT			4
#define WTB_W2_SHIFT_FEC			3
#define WTB_W2_SHIFT_NSD			2
#define WTB_W2_SHIFT_STBC			0

#define WTB_FLAG_RESEND				0x0F000000UL
#define WTB_FLAG_SHIFT_RESEND		24

#define WTB_TSN(_w)			((((struct wtbuf *)_w)->tcb_w0 & WTB_W0_SN) >> WTB_W0_SHIFT_SN)
#define WTB_WH(_w)			((((struct wtbuf *)_w)->tcb_w0 & WTB_W0_WH) >> WTB_W0_SHIFT_WH)
#define WTB_BCMC(_w)		((((struct wtbuf *)_w)->tcb_w0 & WTB_W0_BCMC) >> WTB_W0_SHIFT_BCMC)
#define WTB_PKTLEN(_w)		((((struct wtbuf *)_w)->tcb_w0 & WTB_W0_PKTLEN) >> WTB_W0_SHIFT_PKTLEN)
#define WTB_QOS(_w)			((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_QOS) >> WTB_W1_SHIFT_QOS)
#define WTB_LLC(_w)			((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_LLC) >> WTB_W1_SHIFT_LLC)
#define WTB_CIPHER(_w)		((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_CIPHER) >> WTB_W1_SHIFT_CIPHER)
#define WTB_SEC(_w)			((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_SEC) >> WTB_W1_SHIFT_SEC)
#define WTB_PM(_w)			((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_PM) >> WTB_W1_SHIFT_PM)
#define WTB_ACK(_w)			((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_ACK) >> WTB_W1_SHIFT_ACK)
#define WTB_TID(_w)			((((struct wtbuf *)_w)->tcb_w1 & WTB_W1_TID) >> WTB_W1_SHIFT_TID)
#define WTB_KEYIDX(_w)		((((struct wtbuf *)_w)->tcb_w2 & WTB_W2_KEYIDX) >> WTB_W2_SHIFT_KEYIDX)
#define WTB_DSIDX(_w)		((((struct wtbuf *)_w)->tcb_w2 & WTB_W2_DSIDX) >> WTB_W2_SHIFT_DSIDX)
#define WTB_BSSDESC(_w)		((((struct wtbuf *)_w)->tcb_w2 & WTB_W2_BSSID) >> WTB_W2_SHIFT_BSSID)

#define WTB_RESEND(_w)		((((struct wtbuf *)_w)->flags & WTB_FLAG_RESEND) >> WTB_FLAG_SHIFT_RESEND)

#define PKTLEN_TO_WTB(_v)	((_v << WTB_W0_SHIFT_PKTLEN) & WTB_W0_PKTLEN)
#define SN_TO_WTB(_v)		((_v << WTB_W0_SHIFT_SN) & WTB_W0_SN)
#define PM_TO_WTB(_v)		((_v << WTB_W1_SHIFT_PM) & WTB_W1_PM)
#define ACK_TO_WTB(_v)		((_v << WTB_W1_SHIFT_ACK) & WTB_W1_ACK)
#define TID_TO_WTB(_v)		((_v << WTB_W1_SHIFT_TID) & WTB_W1_TID)

#define RESEND_TO_WTB(_v)		((_v << WTB_FLAG_SHIFT_RESEND) & WTB_FLAG_RESEND)

#define WTB_PKTLEN_UPDATE(_w,_v) \
		do { \
			((struct wtbuf *)_w)->tcb_w0 &= ~WTB_W0_PKTLEN; \
			((struct wtbuf *)_w)->tcb_w0 |= PKTLEN_TO_WTB(_v); \
		} while(0)

#define WTB_SN_UPDATE(_w,_v) \
		do { \
			((struct wtbuf *)_w)->tcb_w0 &= ~WTB_W0_SN; \
			((struct wtbuf *)_w)->tcb_w0 |= SN_TO_WTB(_v); \
		} while(0)

#define WTB_LLC_UPDATE(_w,_v) \
		do { \
			((struct wtbuf *)_w)->tcb_w1 &= ~WTB_W1_LLC; \
			((struct wtbuf *)_w)->tcb_w1 |= _v; \
		} while(0)

#define WTB_ACK_UPDATE(_w,_v) \
		do { \
			((struct wtbuf *)_w)->tcb_w1 &= ~WTB_W1_ACK; \
			((struct wtbuf *)_w)->tcb_w1 |= ACK_TO_WTB(_v); \
		} while(0)

#define WTB_TID_UPDATE(_w,_v) \
		do { \
			((struct wtbuf *)_w)->tcb_w1 &= ~WTB_W1_TID; \
			((struct wtbuf *)_w)->tcb_w1 |= TID_TO_WTB(_v); \
		} while(0)

#define WTB_RESEND_UPDATE(_w,_v) \
		do { \
			((struct wtbuf *)_w)->flags &= ~WTB_FLAG_RESEND; \
			((struct wtbuf *)_w)->flags |= RESEND_TO_WTB(_v); \
		} while(0)

/* Wi-Fi MAC buffer header */
typedef struct {
	union {
		u32 word[2];
		u8 byte[8];
	} bh;
} buf_header;

#define bh_w0						bh.word[0]
#define bh_w1						bh.word[1]

#define BH_NEXTIDX					0xFFFF0000UL
#define BH_EP						0x00008000UL
#define BH_OFFSETH					0x00006000UL
#define BH_LEN						0x00001FFFUL

#define BH_SHIFT_NEXTIDX			16
#define BH_SHIFT_EP					15
#define BH_SHIFT_OFFSETH			13
#define BH_SHIFT_LEN				0

#define BH_OFFSET					0xFC000000UL
#define BH_DPTR						0x03FFFFFFUL

#define BH_SHIFT_OFFSET				26
#define BH_SHIFT_DPTR				0

#define BHDR_NEXTIDX(_b)	(_b->bh_w0 >> BH_SHIFT_NEXTIDX)
#define BHDR_EP(_b)			(_b->bh_w0 & BH_EP)
#define BHDR_OFFSET(_b)		(((_b->bh_w0 >> BH_SHIFT_OFFSETH) & 0x3) | (_b->bh_w1 >> BH_SHIFT_OFFSET))
#define BHDR_LEN(_b)		(_b->bh_w0 & BH_LEN)
#define BHDR_DPTR(_b)		(_b->bh_w1 & BH_DPTR)

#define NEXTIDX_TO_BHDR(_v)	((_v << BH_SHIFT_NEXTIDX) & BH_NEXTIDX)

#define BHDR_NEXTIDX_UPDATE(_b,_v) \
		do { \
			_b->bh_w0 &= ~(BH_NEXTIDX); \
			_b->bh_w0 |= NEXTIDX_TO_BHDR(_v); \
		} while(0)

#define BHDR_NEXTIDX_EP_UPDATE(_b,_v) \
		do { \
			_b->bh_w0 &= ~(BH_NEXTIDX|BH_EP); \
			_b->bh_w0 |= NEXTIDX_TO_BHDR(_v); \
		} while(0)

#define BHDR_EP_SET(_b) \
		do { \
			_b->bh_w0 |= BH_EP; \
		} while(0)

#define BHDR_EP_CLR(_b) \
		do { \
			_b->bh_w0 &= ~BH_EP; \
		} while(0)

#define BHDR_LEN_UPDATE(_b,_v) \
		do { \
			_b->bh_w0 &= ~(BH_LEN); \
			_b->bh_w0 |= (_v & BH_LEN); \
		} while(0)

#define BHDR_OFFSET_UPDATE(_b,_v) \
		do { \
			_b->bh_w0 = _b->bh_w0 & ~BH_OFFSETH | ((_v >> 6) << BH_SHIFT_OFFSETH); \
			_b->bh_w1 = ((_b->bh_w1 & ~BH_OFFSET)) | ((_v & 0x3f) << BH_SHIFT_OFFSET); \
		} while(0)

#define BHDR_DPTR_UPDATE(_b,_v) \
		do { \
			_b->bh_w1 &= ~(BH_DPTR); \
			_b->bh_w1 |= ((u32)_v & BH_DPTR); \
		} while(0)

int bhdr_to_idx(buf_header *bhdr);
int hw_sw_bhdr_to_idx(buf_header *bhdr, char is_hwbuf);
buf_header *idx_to_bhdr(int index);
buf_header *bhdr_find_tail(buf_header *bhdr);
void fill_bhdr(buf_header *bhdr, struct wbuf *wb, unsigned short index, unsigned short ep);
unsigned char get_bhdr_offset(buf_header *bhdr);
buf_header* bhdr_get_first(u32 extra);
void bhdr_insert_tail(int head);
struct wbuf *copy_bhdr_to_wb(unsigned short head, unsigned short tail);
int bhdr_dma_copy(u16 head, char *dst, u32 offset, u32 len);
void dma_copy(void *dst, void *src, unsigned short len, int ch);

#endif
