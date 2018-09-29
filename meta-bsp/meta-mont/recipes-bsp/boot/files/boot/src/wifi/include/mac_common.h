/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                           |
|                                                                              |
+=============================================================================*/
/*! 
*   \file mac_common.c
*   \brief  wla MAC's data structure.
*   \author Montage
*/

#ifndef __MAC_COMMON_H__
#define __MAC_COMMON_H__

#include <wla_cfg.h>
#include <mt_types.h>

#define ACQ_NUM						6
#define CMD_NUM						2
#define MAX_PENDING_BEACON			10
#define MAX_CHANNEL_NUM				32
#define MAX_SSID_LEN				33
#define MAX_CFGSTR_LEN				255

#define ACQ_INTR_BIT(qid, cmdid)	(0x01UL << ((qid * 2) + cmdid))

struct acq {
	struct tx_q *shead;				/* head of schedule */
	struct tx_q *stail;				/* tail of schedule */
	struct tx_q *rhead;				/* head of recycle */
	struct tx_q *rtail;				/* tail of recycle */
	unsigned int cmd[CMD_NUM];
};

#define BCAP_VLD					0x80000000UL
#define BCAP_TOSW					0x40000000UL
#define BCAP_PS						0x20000000UL
#define BCAP_WEP_DEFKEY				0x10000000UL
#define BCAP_NO_AP_ISO				0x08000000UL
#define BCAP_P2P_GO					0x04000000UL
#define BCAP_ETH_HDR				0x02000000UL
#define BCAP_RX_AMPDU				0x0007F800UL
#define BCAP_BSSID					0x00000700UL
#define BCAP_CAPTBL_IDX				0x000000FFUL

#define BCAP_SHIFT_VLD				31
#define BCAP_SHIFT_TOSW				30
#define BCAP_SHIFT_PS				29
#define BCAP_SHIFT_WEP_DEFKEY		28
#define BCAP_SHIFT_NO_AP_ISO		27
#define BCAP_SHIFT_P2P_GO			26
#define BCAP_SHIFT_ETH_HDR			25
#define BCAP_SHIFT_RX_AMPDU			11
#define BCAP_SHIFT_BSSID			8
#define BCAP_SHIFT_CAPTBL_IDX		0

#define RATE_CH_OFFSET				0x0000C000UL
#define RATE_TXCNT					0x00003800UL
#define RATE_SGI					0x00000400UL
#define RATE_FORMAT					0x00000300UL
#define RATE_HT						0x00000080UL
#define RATE_TX_RATE				0x0000007FUL

#define RATE_SHIFT_CH_OFFSET		14
#define RATE_SHIFT_TXCNT			11
#define RATE_SHIFT_SGI				10
#define RATE_SHIFT_FORMAT			8
#define RATE_SHIFT_HT				7
#define RATE_SHIFT_TX_RATE			0

/* Lynx to cheetah wrapper */
typedef struct {
	u32 tdesc[4];
} ctx_descriptor;

#define ctx_w0						tdesc[0]
#define ctx_w1						tdesc[1]
#define ctx_w2						tdesc[2]
#define ctx_w3						tdesc[3]

#define CTX_OWN						0x80000000UL
#define CTX_EOR						0x40000000UL
#define CTX_BC						0x10000000UL
#define CTX_DA_IS_AP				0x08000000UL
#define CTX_DF						0x04000000UL
#define CTX_SWB						0x02000000UL
#define CTX_NP						0x01000000UL
#define CTX_SECOFF					0x00800000UL
#define CTX_DIS_AGG					0x00100000UL
#define CTX_TID						0x000F0000UL
#define CTX_FORCE_HW_FREE			0x00008000UL
#define CTX_HDRLEN					0x00003F00UL
#define CTX_AIDX					0x000000FFUL

#define CTX_SHIFT_OWN				31
#define CTX_SHIFT_EOR				30
#define CTX_SHIFT_BC				28
#define CTX_SHIFT_DA_IS_AP			27
#define CTX_SHIFT_DF				26
#define CTX_SHIFT_SWB				25
#define CTX_SHIFT_NP				24
#define CTX_SHIFT_SECOFF			23
#define CTX_SHIFT_DIS_AGG			20
#define CTX_SHIFT_TID				16
#define CTX_SHIFT_FORCE_HW_FREE		15
#define CTX_SHIFT_HDRLEN			8
#define CTX_SHIFT_AIDX				0

#define CTX_LLC						0x20000000UL
#define CTX_BA						0x10000000UL
#define CTX_ND_RTS					0x08000000UL
#define CTX_DIS_SN					0x04000000UL
#define CTX_DIS_DURATION			0x02000000UL
#define CTX_INS_TS					0x01000000UL
#define CTX_FACK					0x00800000UL
#define CTX_MD						0x00400000UL
#define CTX_INS_GSN					0x00200000UL
#define CTX_ACK_POLICY				0x00180000UL
#define CTX_BSSID					0x00070000UL
#define CTX_PKTLEN					0x0000FFF8UL
#define CTX_BA_NO_AGG				0x00000004UL
#define CTX_AMSDU					0x00000002UL
#define CTX_CHK_PS					0x00000001UL

#define CTX_SHIFT_LLC				29
#define CTX_SHIFT_BA				28
#define CTX_SHIFT_ND_RTS			27
#define CTX_SHIFT_DIS_SN			26
#define CTX_SHIFT_DIS_DURATION		25
#define CTX_SHIFT_INS_TS			24
#define CTX_SHIFT_FACK				23
#define CTX_SHIFT_MD				22
#define CTX_SHIFT_INS_GSN			21
#define CTX_SHIFT_ACK_POLICY		19
#define CTX_SHIFT_BSSID				16
#define CTX_SHIFT_PKTLEN			3
#define CTX_SHIFT_BA_NO_AGG			2
#define CTX_SHIFT_AMSDU				1
#define CTX_SHIFT_CHK_PS			0

#define CTX_BHDRTAIL				0xFFFF0000UL
#define CTX_BHDRHEAD				0x0000FFFFUL

#define CTX_SHIFT_BHDRTAIL			16
#define CTX_SHIFT_BHDRHEAD			0

#define CTX_RATE_IDX				0x0000FF00UL
#define CTX_TX_FAILED				0x00000010UL
#define CTX_RETRY_CNT				0x0000000FUL

#define CTX_SHIFT_RATE_IDX			8
#define CTX_SHIFT_TX_FAILED			4
#define CTX_SHIFT_RETRY_CNT			0

#define CTX_BACKOFF_0_8				0x0007FC00UL
#define CTX_IFS						0x00000300UL
#define CTX_BACKOFF_9				0x00000080UL
#define CTX_ANTENNAS				0x00000040UL
#define CTX_CH_OFFSET				0x00000030UL
#define CTX_NON_HT_MODE				0x0000000CUL
#define CTX_FORMAT					0x00000003UL

#define CTX_SHIFT_BACKOFF_0_8		10
#define CTX_SHIFT_IFS				8
#define CTX_SHIFT_BACKOFF_9			7
#define CTX_SHIFT_ANTENNAS			6
#define CTX_SHIFT_CH_OFFSET			4
#define CTX_SHIFT_NON_HT_MODE		2
#define CTX_SHIFT_FORMAT			0

#define TX_OWN						0x80000000UL
#define TX_BMAP						0x40000000UL
#define TX_PS						0x20000000UL
#define TX_MPDU						0x10000000UL
#define TX_TRY_CNT					0x0F000000UL
#define TX_NOA						0x00800000UL
#define TX_MB						0x00400000UL
#define TX_RATE						0x00780000UL
#define TX_PKTLEN					0x007FFC00UL
#define TX_AIDX						0x003FC000UL
#define TX_TID						0x00003C00UL
#define TX_BHDRHEAD					0x000003FFUL

#define TX_SHIFT_OWN				31
#define TX_SHIFT_BMAP				30
#define TX_SHIFT_PS					29
#define TX_SHIFT_MPDU				28
#define TX_SHIFT_TRY_CNT			24
#define TX_SHIFT_NOA				23
#define TX_SHIFT_MB					22
#define TX_SHIFT_RATE				19
#define TX_SHIFT_PKTLEN				10
#define TX_SHIFT_AIDX				14
#define TX_SHIFT_TID				10
#define TX_SHIFT_BHDRHEAD			0

#define DA_HIT_ADDR		0x1
#define DA_HIT_BSSID	0x2
#define SA_HIT_ADDR		0x4
#define TA_HIT_DS		0x8

#define RX_OWN						0x80000000UL
#define RX_EOR						0x40000000UL
#define RX_CRCE						0x20000000UL
#define RX_SECE						0x10000000UL
#define RX_RAE						0x08000000UL
#define RX_PSPOLL					0x04000000UL
#define RX_BHDRTAIL					0x03FF0000UL
#define RX_BHDRHEAD					0x000003FFUL

#define RX_SHIFT_OWN				31
#define RX_SHIFT_EOR				30
#define RX_SHIFT_CRCE				29
#define RX_SHIFT_SECE				28
#define RX_SHIFT_BSSE				27
#define RX_SHIFT_PSPOLL				26
#define RX_SHIFT_BHDRTAIL			16
#define RX_SHIFT_BHDRHEAD			0

#define SA_IDX_FROM_DESC(_x) ((_x->sa_idx_7 << 7) | (_x->sa_idx_26 << 2) | _x->sa_idx_01)


/* RX descriptor sec_status bitfields */
#define SS_TOCPU				0x100
#define SS_BYPASS				0x040
#define SS_CIPHER_KEY_NONE		0x020
#define SS_CIPHER_KEY_INVALID	0x010
#define SS_REKEY_DROP			0x008
#define SS_DISASSOC_DROP		0x004
#define SS_TKIP_MIC_ERR			0x002
#define SS_ICV_ERR				0x001
#define SS_DROP					( SS_REKEY_DROP | SS_DISASSOC_DROP | SS_TKIP_MIC_ERR | SS_ICV_ERR )


#define CIPHER_TYPE_NONE    0
#define CIPHER_TYPE_WEP40   1
#define CIPHER_TYPE_WEP104  2
#define CIPHER_TYPE_TKIP    3
#define CIPHER_TYPE_CCMP    4
#define CIPHER_TYPE_SMS4	5

#define WEP_DEF_TXKEY_ID	0
#define CIPHYER_TYPE_INVALID    CIPHER_TYPE_NONE

#define WEP_40_KEY_LEN		5
#define WEP_104_KEY_LEN		13
#define TKIP_KEY_LEN		16
#define TKIP_MICKEY_LEN		8
#define CCMP_KEY_LEN		16

#define WAPI_PN_DWORD		0x5c365c36UL
#define WAPI_PN_64BITS		0x5c365c365c365c36ULL	
#define WAPI_TX_INC_ONE     0
#define WAPI_TX_INC_TWO     1
#define WAPI_RX_CHECK_INC_EVEN    0
#define WAPI_RX_CHECK_INC_ODD     1
#define WAPI_RX_CHECK_INC         2

#define WAPI_CIPHER_KEY_LEN     16
#define WAPI_MICKEY_LEN         16

typedef struct{
	u8 da[6];
	u8 sa[6];
	u16 len;
	u8 llc[6];
	u16 type;
	u8 data[0];	
} __attribute__((__packed__)) amsdu_sf;

typedef struct{
	u8 da[6];
	u8 sa[6];
	u16 type;
	u8 data[0];
} __attribute__((__packed__)) ehdr;

#define IS_GROUPCAST(_da)	(((u8 *)_da)[0] & 1)
#define IS_MULTICAST(_da)	((((u8 *)_da)[0] & 0xff) != 0xff)
#define IS_MCAST_MAC(_da)	((_da[0]==0x01) && (_da[1]==0x00) && (_da[2]==0x5e))

#define ETHER_HDR_LEN		14
#define DA_SA_LEN			12

struct tx_q {
	struct tx_q *snext;						/* next of schedule list */
	struct tx_q *rnext;						/* next of recycle list */
	unsigned char cmdid;
	unsigned char sp_len;
	unsigned char pattern;
	unsigned char mode;						/* mode */
	unsigned char tid;
	unsigned char sta_idx;
	unsigned char ba_sz;
	unsigned char ba_no;
	unsigned short flag;
	unsigned short ridx;					/* For global TXQ: hw read index; for independent TXQ: hw read ssn index */
	unsigned short widx;					/* For global TXQ: sw write index; for independent TXQ: sw write ssn index */
	unsigned short ssn;
	unsigned int timestamp;
	unsigned int desc[TX_DESCRIPTOR_COUNT];/* array of u32s */
};

#define TXQ_PUT_TAIL(acq, q, head, tail, next, f) { \
	if ((q->flag & f) == 0) { \
		if (acq->head == 0) { \
			acq->head = acq->tail = q; \
		} else { \
			acq->tail->next = q; \
			acq->tail = q; \
		} \
		q->flag |= f; \
		q->next = 0; \
	} \
}

#define TXQ_GET_HEAD(acq, q, head, tail, next, f) { \
	if (q = acq->head) { \
		if (acq->head == acq->tail) \
			acq->head = acq->tail = 0; \
		else \
			acq->head = q->next; \
		q->next = 0; \
		q->flag &= ~f; \
	} \
}

#define TXQ_DEL(acq, q, head, tail, next, f) { \
	struct tx_q *prev; \
	if (q == acq->head) { \
		acq->head = acq->head->next; \
	} else { \
		prev = acq->head; \
		while (prev) { \
			if (q != prev->next) \
				prev = prev->next; \
			else \
				break; \
		} \
		if (prev) { \
			if (q == acq->tail) { \
				acq->tail = prev; \
				prev->next = 0; \
			} else { \
				prev->next = q->next; \
				q->next = 0; \
			} \
		} \
	} \
	q->flag &= ~f; \
}

enum {
	TXQ_IDLE,
	TXQ_DEAD,
	TXQ_MPDU,
	TXQ_BA,
	TXQ_PS,
	TXQ_PS_DRAIN,
};

#endif // __MAC_COMMON_H__

