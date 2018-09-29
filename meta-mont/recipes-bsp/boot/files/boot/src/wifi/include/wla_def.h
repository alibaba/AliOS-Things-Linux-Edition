/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wla_def.h
*   \brief  WLAN protocol definitions.
*   \author Montage
*/

#ifndef _WLAN_DEF_H_
#define _WLAN_DEF_H_
#include <mt_types.h>
#include <byteorder.h>
#include "panther_debug.h"

#define RETURN_ERR	(-1)
#define RETURN_OK	(0)
#define RETURN_SKIP	(1)

#define RX_MON_GC					0x00000003	// management and data
#define RX_MON_GC_ALL				0x00000001
#define RX_MON_GC_TA				0x00000002
#define RX_MON_UC					0x0000003C	// management and data
#define RX_MON_UC_ALL				0x00000004
#define RX_MON_UC_RA				0x00000008
#define RX_MON_UC_TA				0x00000010
#define RX_MON_UC_RA_TA				0x00000018
#define RX_MON_BEACON				0x000000C0	// beacon and probe response
#define RX_MON_BEACON_ALL			0x00000040
#define RX_MON_BEACON_TA			0x00000080
#define RX_MON_PROBE_REQ			0x00000300
#define RX_MON_PROBE_REQ_ALL		0x00000100
#define RX_MON_PROBE_REQ_TA			0x00000200

#define RX_MON_MGMT					0x00010000	// monitor management frame except beacon and probe response
#define RX_MON_CTRL					0x00020000	// monitor control frame
#define RX_MON_DATA					0x00040000	// monitor mismatch data frame
#define RX_MON_MIMO					0x00080000	// monitor MIMO packet
#define RX_MON_MODE					0x00100000	// monitor all
#define RX_MON_MGMT_BEACON			0x01000000	// beacon
#define RX_MON_MGMT_PROBE_REQ		0x02000000	// probe request
#define RX_MON_MGMT_PROBE_RES		0x04000000	// probe response
#define RX_MON_MGMT_ACTION			0x08000000	// probe response
#define RX_MON_DATA_UNICAST			0x20000000	// unicase data
#define RX_MON_DATA_GROUPCAST		0x40000000	// groupcast data

enum{
	NO_PROTECTION = 0,
	GF_PROTECTION = 1,
	OFDM_PROTECTION,
};

enum {
	SLOTTIME_9US = 9,
	SLOTTIME_20US = 20,
};

enum {
	KEY_TYPE_PAIRWISE_KEY = 0,
	KEY_TYPE_STA_PAIRWISE_KEY,
	KEY_TYPE_GLOBAL_KEY,
};

#ifndef WIF_NONE
#define	WIF_NONE				0
#define WIF_STA_ROLE			0x01
#define WIF_AP_ROLE				0x02
#define WIF_IBSS_ROLE			0x03
#define WIF_WDS_ROLE			0x04
#define WIF_P2P_CLIENT_ROLE		0x9
#define WIF_P2P_GO_ROLE			0xa
//#define WIF_MAT 				0x100
#endif

#define PMK_LEN				32
#define PMKID_LEN			16

#define	WLAN_ADDR_LEN			6
#define	WLAN_IS_MULTICAST(a)	(*(a) & 0x01)
#define WLAN_FORTY_TIMEOUT		1500	/* 1500 = 300sec*5 */

#define FRAME_CONTROL_FIELD \
	u16		subtype:4;		\
	u16		type:2;			\
	u16      ver:2; \
	u16		order:1;	\
	u16		protected:1;	\
	u16		moredata:1;		\
	u16		pwr_mgt:1;		\
	u16		retry:1;		\
	u16		morefrag:1;		\
	u16		from_ds:1;		\
	u16		to_ds:1;		\

#define QOS_CONTROL_FIELD 	\
	u8		amsdu:1; 		\
	u8		ack_policy:2; 	\
	u8		eosp:1; 		\
	u8		tid:4;			\
	u8		txop_dur;

struct wlan_hdr {
	u16		fc;
	u16		dur;
	u8		addr1[WLAN_ADDR_LEN];
	u8		addr2[WLAN_ADDR_LEN];
	u8		addr3[WLAN_ADDR_LEN];
	u16		seq_frag;
} __attribute__ ((packed));

struct wlan_qoshdr {
	u16		fc;
	u16		dur;
	u8		addr1[WLAN_ADDR_LEN];
	u8		addr2[WLAN_ADDR_LEN];
	u8		addr3[WLAN_ADDR_LEN];
	u16		seq_frag;
	u16		qos;
} __attribute__ ((packed));

struct wlan_hdr_addr4 {
	u16		fc;
	u16		dur;
	u8		addr1[WLAN_ADDR_LEN];
	u8		addr2[WLAN_ADDR_LEN];
	u8		addr3[WLAN_ADDR_LEN];
	u16		seq_frag;
	u8		addr4[WLAN_ADDR_LEN];
} __attribute__ ((packed));

struct wlan_qoshdr_addr4 {
	u16		fc;
	u16		dur;
	u8		addr1[WLAN_ADDR_LEN];
	u8		addr2[WLAN_ADDR_LEN];
	u8		addr3[WLAN_ADDR_LEN];
	u16		seq_frag;
	u8		addr4[WLAN_ADDR_LEN];
	u16		qos;
} __attribute__ ((packed));

struct wlan_ctrl_hdr {
	u16		fc;
	u16		dur;
	u8		addr1[WLAN_ADDR_LEN];
	u8		addr2[WLAN_ADDR_LEN];
} __attribute__ ((packed));

struct wlan_ctrl_pspoll_hdr {
	u16	fc;
	u16 aid;
	u8 	bssid[WLAN_ADDR_LEN];
	u8 	ta[WLAN_ADDR_LEN];
} __attribute__ ((packed));

/* WLAN FRAME CONTROL FIELD */
#define WLAN_FC_STYPE		0xf000
#define WLAN_FC_FTYPE		0x0c00
#define WLAN_FC_VERS		0x0300
#define WLAN_FC_ORDER		0x0080
#define WLAN_FC_PROTECTED	0x0040
#define WLAN_FC_MOREDATA	0x0020
#define WLAN_FC_PM			0x0010
#define WLAN_FC_RETRY		0x0008
#define WLAN_FC_MOREFRAGS	0x0004
#define WLAN_FC_FROMDS		0x0002
#define WLAN_FC_TODS		0x0001

/* WLAN QoS CONTROL FIELD */
#define WLAN_QOS_CF_AMSDU		0x8000
#define WLAN_QOS_CF_ACK_POLICY	0x6000
#define WLAN_QOS_CF_EOSP		0x1000
#define WLAN_QOS_CF_TID			0x0f00
#define WLAN_QOS_CF_TXOP_DUR	0x00ff

#define	WLAN_FC_VERSION_0			0x0000	// 0x00
#define	WLAN_FC_TYPE_MGT			0x0000	// 0x00
#define	WLAN_FC_TYPE_CTRL			0x0400	// 0x01
#define	WLAN_FC_TYPE_DATA			0x0800	// 0x02

/* WLAN_FC_TYPE_MGT */
#define	WLAN_FC_SUBTYPE_ASSOC_REQ		0x0000	//	0x0
#define	WLAN_FC_SUBTYPE_ASSOC_RESP		0x1000	//	0x1
#define	WLAN_FC_SUBTYPE_REASSOC_REQ		0x2000	//	0x2
#define	WLAN_FC_SUBTYPE_REASSOC_RESP	0x3000	//	0x3
#define	WLAN_FC_SUBTYPE_PROBE_REQ		0x4000	//	0x4
#define	WLAN_FC_SUBTYPE_PROBE_RESP		0x5000	//	0x5
#define	WLAN_FC_SUBTYPE_BEACON			0x8000	//	0x8
#define	WLAN_FC_SUBTYPE_ATIM			0x9000	//	0x9
#define	WLAN_FC_SUBTYPE_DISASSOC		0xa000	//	0xa
#define	WLAN_FC_SUBTYPE_AUTH			0xb000	//	0xb
#define	WLAN_FC_SUBTYPE_DEAUTH			0xc000	//	0xc
#define	WLAN_FC_SUBTYPE_ACTION			0xd000	//	0xd
/* WLAN_FC_TYPE_CTRL */
#define	WLAN_FC_SUBTYPE_BAR				0x8000	//	0x8
#define	WLAN_FC_SUBTYPE_BA				0x9000	//	0x9
#define	WLAN_FC_SUBTYPE_PS_POLL			0xa000	//	0xa
#define	WLAN_FC_SUBTYPE_RTS				0xb000	//	0xb
#define	WLAN_FC_SUBTYPE_CTS				0xc000	//	0xc
#define	WLAN_FC_SUBTYPE_ACK				0xd000	//	0xd
#define	WLAN_FC_SUBTYPE_CF_END			0xe000	//	0xe
#define	WLAN_FC_SUBTYPE_CF_END_ACK		0xf000	//	0xf
/* WLAN_FC_TYPE_DATA */
#define	WLAN_FC_SUBTYPE_DATA			0x0000	//	0x0
#define	WLAN_FC_SUBTYPE_CF_ACK			0x1000	//	0x1
#define	WLAN_FC_SUBTYPE_CF_POLL			0x2000	//	0x2
#define	WLAN_FC_SUBTYPE_CF_ACPL			0x3000	//	0x3
#define	WLAN_FC_SUBTYPE_NULL			0x4000	//	0x4
#define	WLAN_FC_SUBTYPE_CFACK			0x5000	//	0x5
#define	WLAN_FC_SUBTYPE_CFPOLL			0x6000	//	0x6
#define	WLAN_FC_SUBTYPE_CF_ACK_CF_ACK	0x7000	//	0x7
#define	WLAN_FC_SUBTYPE_QOS				0x8000	//	0x8
#define	WLAN_FC_SUBTYPE_QOS_NULL		0xc000	//	0xc

#define	WLAN_SEQ	0xfff0			/* seqnum */
#define	WLAN_SEQ_S	4
#define	WLAN_SEQ_FRAG	0x000f		/* fragment number */

#define	WLAN_SEQ_RANGE					4096
#define	WLAN_SEQ_BA_RANGE				2048	/* 2^11 */
#define WLAN_SEQ_RESET					0xffff

#define	WLAN_SEQ_ADD(seq, incr) 	(((seq) + (incr)) & (WLAN_SEQ_RANGE-1))
#define	WLAN_SEQ_INC(seq)			WLAN_SEQ_ADD(seq,1)
#define	WLAN_SEQ_SUB(a, b) 			(((a) - (b)) & (WLAN_SEQ_RANGE-1))
#define WLAN_SEQ_LESS(seq1, seq2)	(((seq1) - (seq2)) & WLAN_SEQ_BA_RANGE)

#define WLAN_QOS_ACKPOLICY_ACK			0x0000	//	0x0
#define WLAN_QOS_ACKPOLICY_PSMPACK		0x2000	//	0x1
#define	WLAN_QOS_ACKPOLICY_NOACK		0x4000	//	0x2
#define	WLAN_QOS_ACKPOLICY_BA			0x6000	//	0x3

/* diassociation response frame fixed length format */
struct wlan_disassoc_frame {
        struct wlan_hdr  hdr;
        u16    reasoncode;
}__attribute__ ((packed));

/* association request frame fixed length format */
struct wlan_assoc_req_frame {
	struct wlan_hdr  hdr;
	u16 		capability;
	u16 		listen_interval;
} __attribute__ ((packed));

/* association response frame fixed length format */
struct wlan_assoc_resp_frame {
	struct wlan_hdr  hdr;
	u16		capability;
	u16		statuscode;
	u16		aid;
} __attribute__ ((packed));

/* reassociation response frame fixed length format */
struct wlan_resassoc_frame {
        struct wlan_hdr  hdr;
		u16		capability;
		u16 		listen_interval;
		u8		current_ap[WLAN_ADDR_LEN];
}__attribute__ ((packed));

/* probe request frame fixed length format */
struct wlan_probe_req_frame {
	struct wlan_hdr  hdr;
} __attribute__ ((packed));

/* probe response frame fixed length format */
struct wlan_probe_resp_frame {
	struct wlan_hdr  hdr;
	u32		timestamp[2];
	u16		beacon_interval;
	u16 		capability;
} __attribute__ ((packed));

/* authentication frame fixed length format */
struct wlan_auth_frame {
	struct wlan_hdr hdr;
	u16		alg;			/* algorithm */ 
	u16 		transaction;
	u16 		statuscode;
} __attribute__ ((packed));

/* deauthentication frame fixed length format */
struct wlan_deauth_frame {
	struct wlan_hdr hdr;
	u16    reasoncode;
} __attribute__ ((packed));

#ifndef CONFIG_LINUX_WLA
#define WLAN_CAPABILITY_ESS 			BIT(0)
#define WLAN_CAPABILITY_IBSS 			BIT(1)
#define WLAN_CAPABILITY_CF_POLLABLE 	BIT(2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST BIT(3)
#define WLAN_CAPABILITY_PRIVACY 		BIT(4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE 	BIT(5)
#define WLAN_CAPABILITY_PBCC 			BIT(6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY BIT(7)
#define WLAN_CAPABILITY_SPECTRUM_MGMT 	BIT(8)
#define WLAN_CAPABILITY_QOS 			BIT(9)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME BIT(10)
#define WLAN_CAPABILITY_APSD 			BIT(11)
#define WLAN_CAPABILITY_DSSS_OFDM 		BIT(13)
#define WLAN_CAPABILITY_DELAYED_BLOCK_ACK 	BIT(14)
#define WLAN_CAPABILITY_IMMEDIATE_BLOCK_ACK BIT(15)
#endif

#define	WLAN_AUTH_ALG_OPEN		0
#define	WLAN_AUTH_ALG_SHARED	1
#define WLAN_AUTH_ALG_FT		2
#define	WLAN_AUTH_ALG_LEAP		128

#define	WLAN_WEP_40BIT_KEYLEN		5
#define WLAN_WEP_104BIT_KEYLEN 		13
#define	WLAN_WEP_IVLEN		3	/* 24bit */
#define	WLAN_WEP_KIDLEN		1	/* 1 octet */
#define	WLAN_WEP_CRCLEN		4	/* CRC-32 */
#define	WLAN_WEP_TOTLEN		(WLAN_WEP_IVLEN + WLAN_WEP_KIDLEN + WLAN_WEP_CRCLEN)
#define	WLAN_WEP_NKID		4	/* number of key ids */
#define WLAN_WEP_MAX_KEYLEN	16	/* 128bit */

#define WLAN_PMKID_LEN		16
#define WLAN_PMK_LEN		32

struct eapol {
	u8 		snap[6];
	u16 		ethertype;
	u8 		version;
	u8 		type;
	u16 		len;
} __attribute__ ((packed));

/* action frame fixed length format */
struct wlan_action_frame {
	struct wlan_hdr hdr;
	u8		category;
	u8		action;
} __attribute__ ((packed));

#define	WLAN_ACTION_SPEC_MGT	0	/* Spectrum Management */
#define	WLAN_ACTION_QOS			1	/* QoS */
#define	WLAN_ACTION_DLS			2	/* DLS */
#define	WLAN_ACTION_BA			3	/* BA */
#define	WLAN_ACTION_PUBLIC		4	/* Public */
#define	WLAN_ACTION_HT			7	/* HT */
#define	WLAN_ACTION_VENDOR		127	/* Vendor Specific */


/* Block Ack actions */
#define WLAN_ACTION_BA_ADDBA_REQUEST		0   /* ADDBA request */
#define WLAN_ACTION_BA_ADDBA_RESPONSE		1   /* ADDBA response */
#define WLAN_ACTION_BA_DELBA	        	2   /* DELBA */

/* Public actions */
#define WLAN_ACTION_PUBLIC_COEXISTANCE		0	/* 20/40 MHZ Coexistance Managment */
#define WLAN_ACTION_VENDOR_SPECIFIC			9	/* IEEE 802.11 vendor specific usage.*/
#define WLAN_ACTION_GAS_INITIAL_REQ			10	/*P2P use GAS (802.11u) to do service discovery */
#define WLAN_ACTION_GAS_INITIAL_RESP		11
#define WLAN_ACTION_GAS_COMEBACK_REQ		12
#define WLAN_ACTION_GAS_COMEBACK_RESP		13


/* Block Ack Parameter Set */
#define	WLAN_BAPS_BUFSIZ	0xffc0		/* buffer size */
#define	WLAN_BAPS_BUFSIZ_S	6
#define	WLAN_BAPS_TID	0x003c		/* TID */
#define	WLAN_BAPS_TID_S	2
#define	WLAN_BAPS_POLICY	0x0002		/* block ack policy */
#define	WLAN_BAPS_POLICY_S	1

#define	WLAN_BAPS_POLICY_DELAYED	(0<<WLAN_BAPS_POLICY_S)
#define	WLAN_BAPS_POLICY_IMMEDIATE	(1<<WLAN_BAPS_POLICY_S)

#define WLAN_BAPS_BUFSIZ_MIN	8
#define WLAN_BAPS_BUFSIZ_MAX	64
 
/* Block Ack Sequence Control */
#define	WLAN_BASEQ_START	0xfff0		/* starting seqnum */
#define	WLAN_BASEQ_START_S	4
#define	WLAN_BASEQ_FRAG	0x000f		/* fragment number */
#define	WLAN_BASEQ_FRAG_S	0

/* Delayed Block Ack Parameter Set */
#define	WLAN_DELBAPS_TID	0xf000		/* TID */
#define	WLAN_DELBAPS_TID_S	12
#define	WLAN_DELBAPS_INIT	0x0800		/* initiator */
#define	WLAN_DELBAPS_INIT_S 11

#define WLAN_BA_RECIPIENT	0
#define WLAN_BA_INITIATOR	1

/* BA - ADDBA request */
struct wlan_action_ba_addba_req {
	struct wlan_action_frame hdr;
	u8		dialogtoken;
	u16		baparamset;
	u16		batimeout;
	u16		baseqctl;
} __attribute__ ((packed));

/* BA - ADDBA response */
struct wlan_action_ba_addba_resp {
	struct wlan_action_frame hdr;
	u8		dialogtoken;
	u16		statuscode;
	u16		baparamset; 
	u16		batimeout;
} __attribute__ ((packed));

/* BA - DELBA */
struct wlan_action_ba_delba {
	struct wlan_action_frame hdr;
	u16		baparamset;
	u16		reasoncode;
} __attribute__ ((packed));

/* BAR Control */
#define	WLAN_BAR_TID	0xf000		/* TID */
#define	WLAN_BAR_TID_S	12
#define	WLAN_BAR_COMP	0x0004		/* Compressed Bitmap */
#define	WLAN_BAR_MTID	0x0002		/* Multi-TID */
#define	WLAN_BAR_NOACK	0x0001		/* No-Ack policy */

/* BAR Starting Sequence Control */
#define	WLAN_BAR_SEQ_START	0xfff0		/* starting seqnum */
#define	WLAN_BAR_SEQ_START_S	4

#ifndef CONFIG_LINUX_WLA
#define swap16(x)	\
	(u16)(((u16)(x) & 0xff) << 8 | ((u16)(x) & 0xff00) >> 8)

#define swap32(x)							\
    (u32)(((u32)(x) & 0xff) << 24 |				\
    ((u32)(x) & 0xff00) << 8 | ((u32)(x) & 0xff0000) >> 8 |	\
    ((u32)(x) & 0xff000000) >> 24)

#define htole16 swap16
#define htole32 swap32
#define letoh16 swap16
#define letoh32 swap32

#define htobe16(x) (x)
#define htobe32(x) (x)
#define betoh16(x) (x)
#define betoh32(x) (x)

#define htows htole16
#define htowl htole32
#define wtohs letoh16
#define wtohl letoh32



#define htows htole16
#define htowl htole32
#define wtohs letoh16
#define wtohl letoh32
#endif

struct wlan_ba_req {
	struct wlan_ctrl_hdr hdr;
	u16		barctl;
	u16		barseqctl;
} __attribute__ ((packed));

#define AUTH_CAP_OPEN					BIT(0)
#define AUTH_CAP_AUTO_AUTH_ALG			BIT(1)
#define AUTH_CAP_WEP					BIT(2)
#define AUTH_CAP_WPA					BIT(3)
#define AUTH_CAP_WPA2					BIT(4)
#define AUTH_CAP_WPS					BIT(5)
#define AUTH_CAP_LEAP					BIT(6)
#define AUTH_CAP_WAPI					BIT(7)

enum {
	AUTH_CIPHER_NONE = 0,
	AUTH_CIPHER_WEP40,
	AUTH_CIPHER_WEP104,
	AUTH_CIPHER_TKIP,
	AUTH_CIPHER_CCMP,
	AUTH_CIPHER_SMS4,
	AUTH_CIPHER_AES_128_CMAC,
};

#define AUTH_CAP_CIPHER_MASK			0xff00
#define AUTH_CAP_CIPHER_SHIFT			8

#define AUTH_CAP_CIPHER(_a)		((1 << _a) << AUTH_CAP_CIPHER_SHIFT)
#define AUTH_CAP_CIPHER_NONE			AUTH_CAP_CIPHER(AUTH_CIPHER_NONE)
#define AUTH_CAP_CIPHER_WEP40			AUTH_CAP_CIPHER(AUTH_CIPHER_WEP40)
#define AUTH_CAP_CIPHER_WEP104			AUTH_CAP_CIPHER(AUTH_CIPHER_WEP104)
#define AUTH_CAP_CIPHER_TKIP			AUTH_CAP_CIPHER(AUTH_CIPHER_TKIP)
#define AUTH_CAP_CIPHER_CCMP			AUTH_CAP_CIPHER(AUTH_CIPHER_CCMP)
#define AUTH_CAP_CIPHER_SMS4			AUTH_CAP_CIPHER(AUTH_CIPHER_SMS4)
#define AUTH_CAP_CIPHER_AES_128_CMAC	AUTH_CAP_CIPHER(AUTH_CIPHER_AES_128_CMAC)

#define ALL_CIPHER (AUTH_CAP_CIPHER_WEP40|AUTH_CAP_CIPHER_WEP104|AUTH_CAP_CIPHER_TKIP| \
					AUTH_CAP_CIPHER_CCMP|AUTH_CAP_CIPHER_SMS4|AUTH_CAP_CIPHER_AES_128_CMAC)

#define AUTH_CAP_TO_WEP_TYPE(_a)		(_a & AUTH_CAP_CIPHER_WEP40 ? AUTH_CIPHER_WEP40 : AUTH_CIPHER_WEP104)
#define AUTH_CAP_TO_CIPHER_TYPE(_a)		((AUTH_CAP_CIPHER_MASK & _a) >> AUTH_CAP_CIPHER_SHIFT)

enum {
	AUTH_KEY_MGT_PSK = 0,
	AUTH_KEY_MGT_1X,
	AUTH_KEY_MGT_PSK_SHA256,
	AUTH_KEY_MGT_1X_SHA256,
	AUTH_KEY_MGT_FT_PSK,
	AUTH_KEY_MGT_FT_1X,	
};

#define AUTH_CAP_KEY_MGT_MASK			0xffff0000
#define AUTH_CAP_KEY_MGT_SHIFT			16

#define AUTH_CAP_KEY_MGT(_a)	((1 << _a) << AUTH_CAP_KEY_MGT_SHIFT)
#define AUTH_CAP_KEY_MGT_PSK			AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_PSK)
#define AUTH_CAP_KEY_MGT_1X				AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_1X)
#define AUTH_CAP_KEY_MGT_PSK_SHA256 	AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_PSK_SHA256) /* 802.11w */
#define AUTH_CAP_KEY_MGT_1X_SHA256 		AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_1X_SHA256)
#define AUTH_CAP_KEY_MGT_FT_PSK 		AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_FT_PSK)
#define AUTH_CAP_KEY_MGT_FT_1X 			AUTH_CAP_KEY_MGT(AUTH_KEY_MGT_FT_1X)

#define AUTH_CAP_TO_MGT_TYPE(_a)		((AUTH_CAP_KEY_MGT_MASK & _a) >> AUTH_CAP_KEY_MGT_SHIFT)

#define AUTH_CAP_KEY_MGT_ALL_1X			1
#define AUTH_CAP_KEY_MGT_ALL_PSK		2

#define WLAN_MAX_LISTEN_INTERVAL	65525
	

#define MIN_LENGTH_VERIFY(_len, _min, _act) \
		do { \
			if ((_len) < (_min)) \
			{					\
				/*WLAN_DBG("%s(%d), too short payload, (%d<%d)\n", __func__, __LINE__, _len, _min);	*/\
				_act; \
			}						\
		}while(0)

#define MAX_LENGTH_VERIFY(_len, _max, _act) \
		do { \
			if ((_len) > (_max)) \
			{					\
				WLAN_DBG("%s(%d), too long payload, (%d>%d)\n",	__func__, __LINE__, _len, _max);	\
				_act; \
			}						\
		}while(0)

enum WLAN_state {
	WLAN_STATE_INIT		= 0,	/* default state */
	WLAN_STATE_SCAN		= 1,	/* scanning */
	WLAN_STATE_AUTH		= 2,	/* try to authenticate */
	WLAN_STATE_ASSOC	= 3,	/* try to assoc */
	WLAN_STATE_CHCK		= 4,	/* doing channel availability check */
	WLAN_STATE_RUN		= 5,	/* operational (e.g. associated) */
	WLAN_STATE_CHSW		= 6,	/* channel switch announce pending */
	WLAN_STATE_PS		= 7,	/* power save */
};

enum{
	CCK_1M = 1,
	CCK_2M,
	CCK_5_5M,
	CCK_11M,
/* OFDM */
	OFDM_6M,
	OFDM_9M,
	OFDM_12M,
	OFDM_18M,
	OFDM_24M,
	OFDM_36M,
	OFDM_48M,
	OFDM_54M,
/* HT */
	MCS_0 = 13,
	MCS_1,
	MCS_2,
	MCS_3,
	MCS_4,
	MCS_5,
	MCS_6,
	MCS_7 = 20,
	MCS_8 = 21,
	MCS_9,
	MCS_10,
	MCS_11,
	MCS_12,
	MCS_13,
	MCS_14,
	MCS_15 = 28,

	MCS_32 = 29,
	TEST_RC = 31,
	DIS_RC = 32,
};

#define R_BIT(s) (1 << (s - 1))

#define HIGHEST_BITRATE				MCS_7
#define LOWEST_BITRATE				CCK_1M
#define HT_LOWEST_BITRATE			MCS_0
#define OFDM_LOWEST_BITRATE			OFDM_6M
#define MCS_RATES					(R_BIT(MCS_0)|R_BIT(MCS_1)|R_BIT(MCS_2)|R_BIT(MCS_3)| \
							R_BIT(MCS_4)|R_BIT(MCS_4)|R_BIT(MCS_5)|R_BIT(MCS_6)|R_BIT(MCS_7))
#define OFDM_RATES					(R_BIT(OFDM_6M)|R_BIT(OFDM_9M)|R_BIT(OFDM_12M)|R_BIT(OFDM_18M)| \
							R_BIT(OFDM_24M)|R_BIT(OFDM_36M)|R_BIT(OFDM_48M)|R_BIT(OFDM_54M))	
#define B_RATES						(R_BIT(CCK_1M)|R_BIT(CCK_2M)|R_BIT(CCK_5_5M)|R_BIT(CCK_11M))

#define ALL_SUPPORTED_RATES			(B_RATES|OFDM_RATES|MCS_RATES)
#define ALL_BASIC_RATES 			(R_BIT(CCK_1M)|R_BIT(CCK_2M)|R_BIT(CCK_5_5M)|R_BIT(CCK_11M)| \
									R_BIT(OFDM_6M)|R_BIT(OFDM_12M)|R_BIT(OFDM_24M))

#ifndef CONFIG_LINUX_WLA
/* Reason codes */
enum {
	WLAN_REASON_UNSPECIFIED = 1,
	WLAN_REASON_PREV_AUTH_NOT_VALID = 2,
	WLAN_REASON_DEAUTH_LEAVING = 3,
	WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY = 4,
	WLAN_REASON_DISASSOC_AP_BUSY = 5,
	WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA = 6,
	WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA = 7,
	WLAN_REASON_DISASSOC_STA_HAS_LEFT = 8,
	WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH = 9,
	WLAN_REASON_PWR_CAPABILITY_NOT_VALID = 10,
	WLAN_REASON_SUPPORTED_CHANNEL_NOT_VALID = 11,
	WLAN_REASON_INVALID_IE = 13,
	WLAN_REASON_MIC_FAILURE = 14,
	WLAN_REASON_4WAY_HANDSHAKE_TIMEOUT = 15,
	WLAN_REASON_GROUP_KEY_UPDATE_TIMEOUT = 16,
	WLAN_REASON_IE_IN_4WAY_DIFFERS = 17,
	WLAN_REASON_GROUP_CIPHER_NOT_VALID = 18,
	WLAN_REASON_PAIRWISE_CIPHER_NOT_VALID = 19,
	WLAN_REASON_AKMP_NOT_VALID = 20,
	WLAN_REASON_UNSUPPORTED_RSN_IE_VERSION = 21,
	WLAN_REASON_INVALID_RSN_IE_CAPAB = 22,
	WLAN_REASON_IEEE_802_1X_AUTH_FAILED = 23,
	WLAN_REASON_CIPHER_SUITE_REJECTED = 24,
};

/* Status codes */
enum {
	WLAN_STATUS_SUCCESS = 0,
	WLAN_STATUS_UNSPECIFIED_FAILURE = 1,
	WLAN_STATUS_CAPS_UNSUPPORTED = 10,
	WLAN_STATUS_REASSOC_NO_ASSOC = 11,
	WLAN_STATUS_ASSOC_DENIED_UNSPEC = 12,
	WLAN_STATUS_UNSUPPORTED_AUTH_ALG = 13,
	WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION = 14,
	WLAN_STATUS_CHALLENGE_FAIL = 15,
	WLAN_STATUS_AUTH_TIMEOUT = 16,
	WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA = 17,
	WLAN_STATUS_ASSOC_DENIED_RATES = 18,
	WLAN_STATUS_ASSOC_DENIED_NOSHORT = 19,
	WLAN_STATUS_ASSOC_DENIED_NOPBCC = 20,
	WLAN_STATUS_ASSOC_DENIED_NOAGILITY = 21,
	WLAN_STATUS_SPEC_MGMT_REQUIRED = 22,
	WLAN_STATUS_PWR_CAPABILITY_NOT_VALID = 23,
	WLAN_STATUS_SUPPORTED_CHANNEL_NOT_VALID = 24,
	WLAN_STATUS_ASSOC_DENIED_NO_SHORT_SLOT_TIME = 25,
	WLAN_STATUS_ASSOC_DENIED_NO_ER_PBCC = 26,
	WLAN_STATUS_ASSOC_DENIED_NO_DSSS_OFDM = 27,
	WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY = 30,
	WLAN_STATUS_ROBUST_MGMT_FRAME_POLICY_VIOLATION = 31,
	WLAN_STATUS_REQUEST_DECLINED = 37,
	WLAN_STATUS_INVALID_QOS_PARAM = 38,
	WLAN_STATUS_INVALID_IE = 40,
	WLAN_STATUS_GROUP_CIPHER_NOT_VALID = 41,
	WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID = 42,
	WLAN_STATUS_AKMP_NOT_VALID = 43,
	WLAN_STATUS_UNSUPPORTED_RSN_IE_VERSION = 44,
	WLAN_STATUS_INVALID_RSN_IE_CAPAB = 45,
	WLAN_STATUS_CIPHER_REJECTED_PER_POLICY = 46,
	WLAN_STATUS_TS_NOT_CREATED = 47,
	WLAN_STATUS_DIRECT_LINK_NOT_ALLOWED = 48,
	WLAN_STATUS_DEST_STA_NOT_PRESENT = 49,
	WLAN_STATUS_DEST_STA_NOT_QOS_STA = 50,
	WLAN_STATUS_ASSOC_DENIED_LISTEN_INT_TOO_LARGE = 51,
	WLAN_STATUS_INVALID_FT_ACTION_FRAME_COUNT = 52,
	WLAN_STATUS_INVALID_PMKID = 53,
	WLAN_STATUS_INVALID_MDIE = 54,
	WLAN_STATUS_INVALID_FTIE = 55,
};
#endif

#define MACSTR	"%2x:%2x:%2x:%2x:%2x:%2x"

#define WLAN_RET_OK			0
#define WLAN_RET_ERROR		(-1)

#define WPA_REPLAY_COUNTER_LEN		8
#define WPA_NONCE_LEN 		32
#define WPA_KEY_RSC_LEN		8

#define WLAN_MAX_TIM_LEN	251

#define IBSS_BACKOFF_TIMER	30	/* (CWmin*2 = 15*2 = 30 ) */

#ifndef CONFIG_LINUX_WLA
extern const unsigned char broadcast_addr[6];
extern const unsigned char rfc1042_header[6];
extern const unsigned char bridge_tunnel_header[6];

/*
 * Management information element payloads.
 */
enum {
	WLAN_ELEMID_SSID			= 0,
	WLAN_ELEMID_SUPP_RATES		= 1,
	WLAN_ELEMID_FH_PARMS		= 2,
	WLAN_ELEMID_DS_PARMS		= 3,
	WLAN_ELEMID_CF_PARMS		= 4,
	WLAN_ELEMID_TIM				= 5,
	WLAN_ELEMID_IBSS_PARMS		= 6,
	WLAN_ELEMID_COUNTRY			= 7,
	WLAN_ELEMID_EDCA_PARMS		= 12,
	WLAN_ELEMID_TSPEC			= 13,
	WLAN_ELEMID_TCLAS			= 14,
	WALN_ELEMID_SCHEDULE		= 15,
	WLAN_ELEMID_CHALLENGE		= 16,
	/* 17-31 reserved */
	WLAN_ELEMID_PWR_CNSTR		= 32,		/* power constraint */
	WLAN_ELEMID_PWR_CAP			= 33,		/* power capability */
	WLAN_ELEMID_TPC_REQ			= 34,
	WLAN_ELEMID_TPC_REPORT		= 35,
	WLAN_ELEMID_SUPP_CHAN		= 36,
	WLAN_ELEMID_CSA				= 37,
	WLAN_ELEMID_MEAS_REQ		= 38,
	WLAN_ELEMID_MEAS_REPORT		= 39,
	WLAN_ELEMID_QUIET			= 40,
	WLAN_ELEMID_IBSSDFS			= 41,
	WLAN_ELEMID_ERP_INFO		= 42,
	WLAN_ELEMID_TS_DELAY		= 43,
	WLAN_ELEMID_TCLAS_PRO		= 44,
	WLAN_ELEMID_HT_CAP			= 45,
	WLAN_ELEMID_QOS_CAP			= 46,
	WLAN_ELEMID_RSN				= 48,
	WLAN_ELEMID_EXT_SUPP_RATES	= 50,
	WLAN_ELEMID_MOBILITY_DOMAIN	= 54,
	WLAN_ELEMID_FAST_BSS_TRANS	= 55,
	WLAN_ELEMID_TIMEOUT_INTVAL	= 56,
	WLAN_ELEMID_HT_INFO			= 61,
	WLAN_ELEMID_SEC_CH_OFFSET	= 62,
	WLAN_ELEMID_WAPI			= 68,
	WLAN_ELEMID_2040_BSS_COE	= 72,
	WLAN_ELEMID_2040_BSS_INT	= 73,
	WLAN_ELEMID_OBSS_SCAN		= 74,	
	WLAN_ELEMID_EXT_CAP			= 127,	
	WLAN_ELEMID_TPC				= 150,
	WLAN_ELEMID_CCKM			= 156,
	WLAN_ELEMID_VENDOR_SPEC		= 221,	/* vendor private */

	/*
	 * 802.11s IEs based on D3.0 spec and were not assigned by
	 * ANA. Beware changing them because some of them are being
	 * kept compatible with Linux.
	 */
	WLAN_ELEMID_MESHCONF	= 51,
	WLAN_ELEMID_MESHID		= 52,
	WLAN_ELEMID_MESHLINK	= 35,
	WLAN_ELEMID_MESHCNGST	= 36,
	WLAN_ELEMID_MESHPEER	= 55,
	WLAN_ELEMID_MESHCSA	= 38,
	WLAN_ELEMID_MESHTIM	= 39,
	WLAN_ELEMID_MESHAWAKEW	= 40,
	WLAN_ELEMID_MESHBEACONT	= 41,
	WLAN_ELEMID_MESHPANN	= 48,
	WLAN_ELEMID_MESHRANN	= 49,
	WLAN_ELEMID_MESHPREQ	= 68,
	WLAN_ELEMID_MESHPREP	= 69,
	WLAN_ELEMID_MESHPERR	= 70,
	WLAN_ELEMID_MESHPU		= 53,
	WLAN_ELEMID_MESHPUC	= 54,
	WLAN_ELEMID_MESHAH		= 60, /* Abbreviated Handshake */
	WLAN_ELEMID_MESHPEERVER	= 80, /* Peering Protocol Version */
};

struct wlan_ie_hdr {
	u8 id;
	u8 len;
} __attribute__ ((packed));

struct wlan_ie_generic {
	u8 id;
	u8 len;
	u8 data[0];
} __attribute__ ((packed));


/*
 * WME/802.11e information element.
 */
struct wlan_ie_wme_info {
	u8		id;			
	u8		len;	
	u8		oui[3];	/* 0x00, 0x50, 0xf2 */
	u8		type;
	u8		subtype;
	u8		version;	
	u8		qosinfo;	
} __attribute__ ((packed));

/*
 * WME Parameter Element
 */
struct wme_acparams {
	u8 		reserved:1;
	u8		aci:2;
	u8 		acm:1;
	u8 		aifsn:4;
	u8 		ecwmax:4;
	u8 		ecwmin:4;
	u16		txop;
} __attribute__ ((packed));

struct wme_sta_qosinfo {
	u8 		:1;
	u8 		max_sp_len:2;
	u8 		:1;
	u8 		uapsd_ac_be:1;
	u8 		uapsd_ac_bk:1;
	u8 		uapsd_ac_vi:1;
	u8 		uapsd_ac_vo:1;
}__attribute__ ((packed));

#define WME_QOSINFO_MAX_SP_LEN		(BIT(6)|BIT(5))
#define WME_QOSINFO_UAPSD_AC_BE		BIT(3)
#define WME_QOSINFO_UAPSD_AC_BK		BIT(2)
#define WME_QOSINFO_UAPSD_AC_VI		BIT(1)
#define WME_QOSINFO_UAPSD_AC_VO		BIT(0)

struct wlan_ie_wme_param {
	u8		id;
	u8		len;
	u8		oui[3];
	u8		type;
	u8		subtype;
	u8		version;
	u8		qosinfo;
#define	WME_QOSINFO_COUNT	0x0f	/* Mask for param count field */
#define WME_QOSINFO_APSD	0x80
	u8		reserved;
#define WME_NUM_AC		4	/* 4 AC categories */
	struct wme_acparams	ac_params[WME_NUM_AC];
} __attribute__ ((packed));


/*
 * BEACON management packets
 *
 *	octet timestamp[8]
 *	octet beacon interval[2]
 *	octet capability information[2]
 *	information element
 *		octet elemid
 *		octet length
 *		octet information[length]
 */
#define	IEEE80211_BEACON_INTERVAL(beacon) \
	((beacon)[8] | ((beacon)[9] << 8))
#define	IEEE80211_BEACON_CAPABILITY(beacon) \
	((beacon)[10] | ((beacon)[11] << 8))

#define	IEEE80211_CAPINFO_ESS				0x0001
#define	IEEE80211_CAPINFO_IBSS				0x0002
#define	IEEE80211_CAPINFO_CF_POLLABLE		0x0004
#define	IEEE80211_CAPINFO_CF_POLLREQ		0x0008
#define	IEEE80211_CAPINFO_PRIVACY			0x0010
#define	IEEE80211_CAPINFO_SHORT_PREAMBLE	0x0020
#define	IEEE80211_CAPINFO_PBCC				0x0040
#define	IEEE80211_CAPINFO_CHNL_AGILITY		0x0080
#define	IEEE80211_CAPINFO_SPECTRUM_MGMT		0x0100
/* bit 9 is reserved */
#define	IEEE80211_CAPINFO_SHORT_SLOTTIME	0x0400
#define	IEEE80211_CAPINFO_RSN				0x0800
/* bit 12 is reserved */
#define	IEEE80211_CAPINFO_DSSSOFDM			0x2000
/* bits 14-15 are reserved */

#define	IEEE80211_CAPINFO_BITS \
	"\20\1ESS\2IBSS\3CF_POLLABLE\4CF_POLLREQ\5PRIVACY\6SHORT_PREAMBLE" \
	"\7PBCC\10CHNL_AGILITY\11SPECTRUM_MGMT\13SHORT_SLOTTIME\14RSN" \
	"\16DSSOFDM"


/*
 * 802.11i/WPA information element (minmum size).
 */
struct wlan_ie_wpa {
	u8		id;		/* IEEE80211_ELEMID_VENDOR */
	u8		len;	/* length in bytes */
	u8		oui[3];	/* 0x00, 0x50, 0xf2 */
	u8		oui_type;	/* OUI type */
	u16		version;	/* spec revision */
	u32		group_cipher[1];	/* multicast/group key cipher */
} __attribute__ ((packed));

/* 802.11n HT Capability IE */
struct ht_capability {
	u16	capabilities_info;
	u8 	ampdu_params;
	u8 	supported_mcs_set[16];
	u16 ht_extended_capability;
	u32	tx_bf_capability;
	u8	asel_capabilities;
}__attribute__ ((packed));

struct wlan_ie_ht_capability {
	u8 id;
	u8 len;
	struct ht_capability ht_cap;
} __attribute__ ((packed));


/* HT capability flags (ht_cap) */
#define	WLAN_HTCAP_LDPC				0x0001	/* LDPC supported */
#define	WLAN_HTCAP_CHWIDTH			0x0002	/* 20/40 supported */
#define	WLAN_HTCAP_SMPS				0x000c	/* SM Power Save mode */
#define	WLAN_HTCAP_GREENFIELD		0x0010	/* Greenfield supported */
#define	WLAN_HTCAP_SHORTGI20		0x0020	/* Short GI in 20MHz */
#define	WLAN_HTCAP_SHORTGI40		0x0040	/* Short GI in 40MHz */
#define	WLAN_HTCAP_TXSTBC			0x0080	/* STBC tx ok */
#define	WLAN_HTCAP_RXSTBC			0x0300  /* STBC rx support */
#define	WLAN_HTCAP_RXSTBC_S			8
#define	WLAN_HTCAP_RXSTBC_1STREAM	0x0100  /* 1 spatial stream */
#define	WLAN_HTCAP_RXSTBC_2STREAM	0x0200  /* 1-2 spatial streams*/
#define	WLAN_HTCAP_RXSTBC_3STREAM	0x0300  /* 1-3 spatial streams*/
#define	WLAN_HTCAP_DELBA			0x0400	/* HT DELBA supported */
#define	WLAN_HTCAP_MAXAMSDU			0x0800	/* max A-MSDU length */
#define	WLAN_HTCAP_MAXAMSDU_7935	0x0800	/* 7935 octets */
#define	WLAN_HTCAP_MAXAMSDU_3839	0x0000	/* 3839 octets */
#define	WLAN_HTCAP_DSSSCCK40		0x1000  /* DSSS/CCK in 40MHz */
#define	WLAN_HTCAP_PSMP				0x2000  /* PSMP supported */
#define	WLAN_HTCAP_40INTOLERANT		0x4000  /* 40MHz intolerant */
#define	WLAN_HTCAP_LSIGTXOPPROT		0x8000  /* L-SIG TXOP prot */

#define	IEEE80211_HTCAP_BITS \
	"\20\1LDPC\2CHWIDTH40\5GREENFIELD\6SHORTGI20\7SHORTGI40\10TXSTBC" \
	"\13DELBA\14AMSDU(7935)\15DSSSCCK40\16PSMP\1740INTOLERANT" \
	"\20LSIGTXOPPROT"

/* HT parameters (hc_param) */
#define	WLAN_HTCAP_MAXRXAMPDU		0x03	/* max rx A-MPDU factor */
#define	WLAN_HTCAP_MAXRXAMPDU_S		0
#define	WLAN_HTCAP_MAXRXAMPDU_8K	0
#define	WLAN_HTCAP_MAXRXAMPDU_16K	1
#define	WLAN_HTCAP_MAXRXAMPDU_32K	2
#define	WLAN_HTCAP_MAXRXAMPDU_64K	3
#define	WLAN_HTCAP_MPDUDENSITY		0x1c	/* min MPDU start spacing */
#define	WLAN_HTCAP_MPDUDENSITY_S	2
#define	WLAN_HTCAP_MPDUDENSITY_NA	0	/* no time restriction */
#define	WLAN_HTCAP_MPDUDENSITY_025	1	/* 1/4 us */
#define	WLAN_HTCAP_MPDUDENSITY_05	2	/* 1/2 us */
#define	WLAN_HTCAP_MPDUDENSITY_1	3	/* 1 us */
#define	WLAN_HTCAP_MPDUDENSITY_2	4	/* 2 us */
#define	WLAN_HTCAP_MPDUDENSITY_4	5	/* 4 us */
#define	WLAN_HTCAP_MPDUDENSITY_8	6	/* 8 us */
#define	WLAN_HTCAP_MPDUDENSITY_16	7	/* 16 us */

/* HT extended capabilities (hc_extcap) */
#define	WLAN_HTCAP_PCO			0x0001	/* PCO capable */
#define	WLAN_HTCAP_PCOTRANS		0x0006	/* PCO transition time */
#define	WLAN_HTCAP_PCOTRANS_S	1
#define	WLAN_HTCAP_PCOTRANS_04	0x0002	/* 400 us */
#define	WLAN_HTCAP_PCOTRANS_15	0x0004	/* 1.5 ms */
#define	WLAN_HTCAP_PCOTRANS_5	0x0006	/* 5 ms */
/* bits 3-7 reserved */
#define	WLAN_HTCAP_MCSFBACK			0x0300	/* MCS feedback */
#define	WLAN_HTCAP_MCSFBACK_S		8
#define	WLAN_HTCAP_MCSFBACK_NONE	0x0000	/* nothing provided */
#define	WLAN_HTCAP_MCSFBACK_UNSOL	0x0200	/* unsolicited feedback */
#define	WLAN_HTCAP_MCSFBACK_MRQ		0x0300	/* " "+respond to MRQ */
#define	WLAN_HTCAP_HTC				0x0400	/* +HTC support */
#define	WLAN_HTCAP_RDR				0x0800	/* reverse direction responder*/
/* bits 12-15 reserved */

/*
 * 802.11n HT Information IE
 */
struct wlan_ie_ht_info {
	u8		id;			/* element ID */
	u8		len;			/* length in bytes */
	u8		primary_channel;		/* primary channel */
	u8 		param;
	u16 	operation_mode;
	u16 	stbc_param;
	u8 		basicmcsset[16]; 	/* basic MCS set */
} __attribute__ ((packed));

/* byte1 */
#define	WLAN_HTINFO_SECCHAN			0x03	/* secondary/ext chan offset */
#define	WLAN_HTINFO_SECCHAN_S		0
#define	WLAN_HTINFO_SECCHAN_NONE	0x00	/* no secondary/ext channel */
#define	WLAN_HTINFO_SECCHAN_ABOVE	0x01	/* above private channel */
/* NB: 2 is reserved */
#define	WLAN_HTINFO_2NDCHAN_BELOW	0x03	/* below primary channel */ 
#define	WLAN_HTINFO_TXWIDTH			0x04	/* tx channel width */
#define	WLAN_HTINFO_TXWIDTH_20		0x00	/* 20MHz width */
#define	WLAN_HTINFO_TXWIDTH_2040	0x04	/* any supported width */
#define	WLAN_HTINFO_RIFSMODE		0x08	/* Reduced IFS (RIFS) use */
#define	WLAN_HTINFO_RIFSMODE_PROH	0x00	/* RIFS use prohibited */
#define	WLAN_HTINFO_RIFSMODE_PERM	0x08	/* RIFS use permitted */
#define	WLAN_HTINFO_PMSPONLY		0x10	/* PSMP required to associate */
#define	WLAN_HTINFO_SIGRAN			0xe0	/* shortest Service Interval */
#define	WLAN_HTINFO_SIGRAN_S		5
#define	WLAN_HTINFO_SIGRAN_5		0x00	/* 5 ms */
/* XXX add rest */

/* bytes 2+3 */
#define	WLAN_HTINFO_OPMODE			0x03	/* operating mode */
#define	WLAN_HTINFO_OPMODE_S		0
#define	WLAN_HTINFO_OPMODE_PURE		0x00	/* no protection */
#define	WLAN_HTINFO_OPMODE_PROTOPT	0x01	/* protection optional */
#define	WLAN_HTINFO_OPMODE_HT20PR	0x02	/* protection for HT20 sta's */
#define	WLAN_HTINFO_OPMODE_MIXED	0x03	/* protection for legacy sta's*/
#define	WLAN_HTINFO_NONGF_PRESENT	0x04	/* non-GF sta's present */
#define	WLAN_HTINFO_TXBL			0x08	/* transmit burst limit */
#define	WLAN_HTINFO_NONHT_PRESENT	0x10	/* non-HT sta's present */
/* bits 5-15 reserved */

/* bytes 4+5 */
#define	WLAN_HTINFO_2NDARYBEACON	0x01
#define	WLAN_HTINFO_LSIGTXOPPROT	0x02
#define	WLAN_HTINFO_PCO_ACTIVE		0x04
#define	WLAN_HTINFO_40MHZPHASE		0x08

/* byte5 */
#define	WLAN_HTINFO_BASIC_STBCMCS	0x7f
#define	WLAN_HTINFO_BASIC_STBCMCS_S 0
#define	WLAN_HTINFO_DUALPROTECTED	0x80

/* 802.11n 20/40 BSS Coexistance element : data */
#define WLAN_20_40_BSS_COEX_INFO_REQ            BIT(0)
#define WLAN_20_40_BSS_COEX_40MHZ_INTOL         BIT(1)
#define WLAN_20_40_BSS_COEX_20MHZ_WIDTH_REQ     BIT(2)
#define WLAN_20_40_BSS_COEX_OBSS_EXEMPT_REQ     BIT(3)
#define WLAN_20_40_BSS_COEX_OBSS_EXEMPT_GRNT    BIT(4)

/* 802.11n 20/40 BSS Coexistance element */
struct wlan_ie_2040_coe{
	u8 id;
	u8 len;
	u8 data;
} __attribute__ ((packed));

/* 802.11n 20/40 BSS Intolerant Channel Report element */
struct wlan_ie_2040_int{
	u8 id;
	u8 len;
	u8 regulatory_class;	/* regulatory class, see IEEE 802.11n Annex.J */
	u8 channel_list[0];	/* variable-length channel list */
} __attribute__ ((packed)); 

/* challenge length for shared key auth */
#define WLAN_AUTH_CHALLENGE_LEN		128

struct wlan_ie_tim{
	u8		id;	
	u8		len;
	u8		count;		/* DTIM count */
	u8		period;		/* DTIM period */
	u8		bitctl;		/* bitmap control */
	u8		bitmap[1];		/* variable-length bitmap */
} __attribute__ ((packed));


struct wlan_country_str{
	u8	first;			/* First channel number */
	u8	num;			/* Number of channels */
	u8	max_txpower;	/* Maximum transmit power */ 
};

#define	IEEE80211_COUNTRY_MAX_BANDS	84	/* max possible bands */
#define	IEEE80211_COUNTRY_MAX_SIZE \
	(sizeof(struct ieee80211_country_ie) + 3*(IEEE80211_COUNTRY_MAX_BANDS-1))

/*
 * Note the min acceptable CSA count is used to guard against
 * malicious CSA injection in station mode.  Defining this value
 * as other than 0 violates the 11h spec.
 */
#define	IEEE80211_CSA_COUNT_MIN	2
#define	IEEE80211_CSA_COUNT_MAX	255

/* rate set entries are in .5 Mb/s units, and potentially marked as basic */
#define	IEEE80211_RATE_BASIC		0x80
#define	IEEE80211_RATE_VAL			0x7f

/* EPR information element flags */
#define	WLAN_ERP_NON_ERP_PRESENT	0x01
#define	WLAN_ERP_USE_PROTECTION		0x02
#define	WLAN_ERP_LONG_PREAMBLE		0x04

#define	IEEE80211_ERP_BITS \
	"\20\1NON_ERP_PRESENT\2USE_PROTECTION\3LONG_PREAMBLE"

/* OUI */
#define	ATHEROS_OUI				0x00037f	/* Atheros OUI */
#define	ATHEROS_OUI_TYPE		0x01
#define	TDMA_OUI_TYPE			0x02

#define	BROADCOM_OUI			0x00904c	/* Broadcom OUI */
#define	BROADCOM_OUI_HTCAP		0x33
#define	BROADCOM_OUI_HTINFO		0x34

#define	MICROSOFT_OUI			0x0050f2 	/* microsoft OUI */
#define	WPA_OUI_TYPE			0x01
#define	WME_OUI_TYPE			0x02
#define	WPS_OUI_TYPE			0x04
#define	WME_INFO_OUI_SUBTYPE	0x00
#define	WME_PARAM_OUI_SUBTYPE	0x01
#define	WME_TSPEC_OUI_SUBTYPE	0x02


#define	RSN_OUI					0xac0f00


#define	WPA_VERSION				1
#define	RSN_VERSION				1
#define	WME_VERSION				1
#define	WAPI_VERSION			1

/* WME stream classes */
#define	WME_AC_BE	0		/* best effort */
#define	WME_AC_BK	1		/* background */
#define	WME_AC_VI	2		/* video */
#define	WME_AC_VO	3		/* voice */

#define WAI_BKID_LEN		16
#define SELECTOR_LEN		4

#define OUI_SUITETYPE(a, b, c, d) \
	((((u32) (a)) << 24) | (((u32) (b)) << 16) | (((u32) (c)) << 8) | \
	 (u32) (d))

#define WPA_AKM_SUITE_NONE 					OUI_SUITETYPE(0x00, 0x50, 0xf2, 0)
#define WPA_AKM_SUITE_1X					OUI_SUITETYPE(0x00, 0x50, 0xf2, 1)
#define WPA_AKM_SUITE_PSK					OUI_SUITETYPE(0x00, 0x50, 0xf2, 2)

#define WPA_CIPHER_SUITE_NONE 				OUI_SUITETYPE(0x00, 0x50, 0xf2, 0)
#define WPA_CIPHER_SUITE_WEP40				OUI_SUITETYPE(0x00, 0x50, 0xf2, 1)
#define WPA_CIPHER_SUITE_TKIP				OUI_SUITETYPE(0x00, 0x50, 0xf2, 2)
#define WPA_CIPHER_SUITE_CCMP				OUI_SUITETYPE(0x00, 0x50, 0xf2, 4)
#define WPA_CIPHER_SUITE_WEP104				OUI_SUITETYPE(0x00, 0x50, 0xf2, 5)


#define WAI_CIPHER_SUITE_NONE 				OUI_SUITETYPE(0x00, 0x14, 0x72, 0)
#define WAI_CIPHER_SUITE_1X					OUI_SUITETYPE(0x00, 0x14, 0x72, 1)
#define WAI_CIPHER_SUITE_PSK				OUI_SUITETYPE(0x00, 0x14, 0x72, 2)

#define WPI_CIPHER_SUITE_NONE 				OUI_SUITETYPE(0x00, 0x14, 0x72, 0)
#define WPI_CIPHER_SUITE_SMS4				OUI_SUITETYPE(0x00, 0x14, 0x72, 1)

#define RSN_AKM_SUITE_1X					OUI_SUITETYPE(0x00, 0x0f, 0xac, 1)
#define RSN_AKM_SUITE_PSK					OUI_SUITETYPE(0x00, 0x0f, 0xac, 2)
#define RSN_AKM_SUITE_FT_1X					OUI_SUITETYPE(0x00, 0x0f, 0xac, 3)
#define RSN_AKM_SUITE_FT_PSK				OUI_SUITETYPE(0x00, 0x0f, 0xac, 4)
#define RSN_AKM_SUITE_1X_SHA256				OUI_SUITETYPE(0x00, 0x0f, 0xac, 5)
#define RSN_AKM_SUITE_PSK_SHA256			OUI_SUITETYPE(0x00, 0x0f, 0xac, 6)

#define RSN_CIPHER_SUITE_NONE				OUI_SUITETYPE(0x00, 0x0f, 0xac, 0)
#define RSN_CIPHER_SUITE_WEP40				OUI_SUITETYPE(0x00, 0x0f, 0xac, 1)
#define RSN_CIPHER_SUITE_TKIP				OUI_SUITETYPE(0x00, 0x0f, 0xac, 2)
#define RSN_CIPHER_SUITE_CCMP 				OUI_SUITETYPE(0x00, 0x0f, 0xac, 4)
#define RSN_CIPHER_SUITE_WEP104 			OUI_SUITETYPE(0x00, 0x0f, 0xac, 5)
#define RSN_CIPHER_SUITE_AES_128_CMAC 		OUI_SUITETYPE(0x00, 0x0f, 0xac, 6)

#define IE_SSID_BIT				0x1
#define IE_SUPP_RATES_BIT		0x2
#define IE_DS_PARMS_BIT			0x4
#define IE_TIM_BIT				0x8
#define IE_IBSS_BIT				0x10
#define IE_COUNTRY_BIT			0x20
#define IE_CHALLENGE_BIT		0x40
#define IE_PWR_CAP_BIT			0x80
#define IE_SUPPORT_CHAN_BIT		0x100
#define IE_ERP_INFO_BIT			0x200
#define IE_HT_CAP_BIT			0x400 
#define IE_RSN_BIT				0x800
#define IE_EXT_SUPP_RATES_BIT	0x1000
#define IE_MD_BIT				0x2000
#define IE_FT_BIT				0x4000
#define IE_TIME_OUT_BIT			0x8000
#define IE_HT_INFO_BIT			0x10000
#define IE_WAPI_BIT				0x20000
#define IE_EXT_CAP_BIT			0x40000
#define IE_WPA_BIT				0x80000
#define IE_WMM_BIT				0x100000
#define IE_WPS_BIT				0x200000
#define IE_P2P_BIT				0x400000

#ifdef   CONFIG_HOST_WPS
#define IE_WPS_BEACON_BIT			BIT(23)
#define IE_WPS_PROBE_RESP_BIT		BIT(24)
#define IE_WPS_ASSOC_RESP_BIT		BIT(25)
#define IE_WPS_PROBE_REQ_BIT		BIT(26)
#define IE_WPS_ASSOC_REQ_BIT		BIT(27)
#define IE_END_BIT					BIT(28)	/* last bit */
#else
#define IE_END_BIT					BIT(23)	/* last bit */
#endif /*CONFIG_HOST_WPS*/

struct wlan_ie{
	u8 *ssid;				/* 0 */
	u8 *supp_rates;			/* 1 */
	u8 *ds_params;			/* 3 */
	u8 *tim;				/* 5 */
	u8 *ibss_ie;			/* 6 */
	u8 *country_ie;			/* 7 */
	u8 *challenge;			/* 16 */
	u8 *power_cap;			/* 33 */
	u8 *supp_channels;		/* 36 */
	u8 *erp_info;			/* 42 */
	u8 *ht_capabilities;	/* 45 */

	u8 *rsn_ie;				/* 48 */
	u8 *ext_supp_rates;		/* 50 */
	u8 *mdie;				/* 54 */
	u8 *ftie;				/* 55 */
	u8 *timeout_int;		/* 56 */
	u8 *ht_operation;		/* 61 */
	u8 *wapi_ie;			/* 68 */
	u8 *ext_cap_ie;			/* 127 */

	/* below items uses vendor IE */ 
	u8 *wpa_ie;
	u8 *wme;
	u8 *wps_ie;
	u8 *p2p_ie;

};

struct wlan_ie_ibss {
	u8 	id;
	u8 	len;
	u16 atim_window;
} __attribute__ ((packed));

struct wpa_ie_data{
	u32 proto;
	u32 pairwise_cipher;
	u32 group_cipher;
	u32 key_mgmt;
	u32 capabilities;
	u32 num_pmkid;
	u8 *pmkid;
	u32 mgmt_group_cipher;
};

struct wlan_wme_ac_params {
	short qidx;
	short cwmin;
	short cwmax;
	short aifs;
	u16 txoplimit; /* in units of 32us */
	u16 acm; // admission_control_mandatory; only need 1 bits
};

struct wme_ie_data {
	struct wlan_wme_ac_params wme_ac_params[4];
	u32 wme_acm;
};

enum {
	PS_STATE_AWAKE,
	PS_STATE_QUERY,
	PS_STATE_DOZE,
};

/* Power Saving Policy*/
#define PS_POLICY_UAPSD_VO	BIT(0)
#define PS_POLICY_UAPSD_VI	BIT(1)
#define PS_POLICY_UAPSD_BK	BIT(2)
#define PS_POLICY_UAPSD_BE	BIT(3)
#define PS_POLICY_PSPOLL	BIT(4)
#define PS_POLICY_PSnonPOLL	BIT(5)
#define PS_POLICY_LEGACY	(PS_POLICY_PSPOLL | PS_POLICY_PSnonPOLL)
#define PS_POLICY_UAPSD		(PS_POLICY_UAPSD_BE | PS_POLICY_UAPSD_BK | PS_POLICY_UAPSD_VI | PS_POLICY_UAPSD_VO)

/* P2P OUI */
#define WFA_SPECIFIC_OUI		0x506F9A
#define WFA_P2P_OUI_SUBTYPE		0x9

/* P2P Attribute ID */
#define	P2P_ATTR_STATUS 					0
#define	P2P_ATTR_MINOR_REASON_CODE			1
#define	P2P_ATTR_CAPABILITY 				2
#define	P2P_ATTR_DEVICE_ID 					3
#define	P2P_ATTR_GROUP_OWNER_INTENT 		4
#define	P2P_ATTR_CONFIGURATION_TIMEOUT 		5
#define	P2P_ATTR_LISTEN_CHANNEL 			6
#define	P2P_ATTR_GROUP_BSSID 				7
#define	P2P_ATTR_EXT_LISTEN_TIMING 			8
#define P2P_ATTR_INTENDED_INTERFACE_ADDR 	9
#define	P2P_ATTR_MANAGEABILITY 				10
#define	P2P_ATTR_CHANNEL_LIST 				11
#define	P2P_ATTR_NOTICE_OF_ABSENCE 			12
#define	P2P_ATTR_DEVICE_INFO  				13
#define	P2P_ATTR_GROUP_INFO  				14
#define	P2P_ATTR_GROUP_ID  					15
#define	P2P_ATTR_INTERFACE  				16
#define	P2P_ATTR_OPERATING_CHANNEL 			17
#define	P2P_ATTR_INVITATION_FLAGS  			18
#define	P2P_ATTR_VENDOR_SPECIFIC 			221

/* P2P Status Code */
#define P2P_SC_SUCCESS 							0
#define	P2P_SC_FAIL_INFO_CURRENTLY_UNAVAILABLE 	1
#define	P2P_SC_FAIL_INCOMPATIBLE_PARAMS 		2
#define	P2P_SC_FAIL_LIMIT_REACHED 				3
#define	P2P_SC_FAIL_INVALID_PARAMS 				4
#define	P2P_SC_FAIL_UNABLE_TO_ACCOMMODATE 		5
#define	P2P_SC_FAIL_PREV_PROTOCOL_ERROR 		6
#define	P2P_SC_FAIL_NO_COMMON_CHANNELS 			7
#define	P2P_SC_FAIL_UNKNOWN_GROUP 				8
#define	P2P_SC_FAIL_BOTH_GO_INTENT_15 			9
#define	P2P_SC_FAIL_INCOMPATIBLE_PROV_METHOD 	10
#define	P2P_SC_FAIL_REJECTED_BY_USER 			11

struct wlan_p2p_attr {
	u8	id;			
	u16	len;	
	u8	body[0];
} __attribute__ ((packed));

struct wlan_ie_p2p {
	u8	id;			
	u8	len;	
	u8	oui[3];
	u8	oui_type;
	/* the folowing is attributes (struct wlan_p2p_attr) */
} __attribute__ ((packed));
#endif

enum {
	CTS_PROTECTION_AUTOMATIC = 0,
	CTS_PROTECTION_FORCE_ENABLED = 1,
	CTS_PROTECTION_FORCE_DISABLED = 2,
	CTS_PROTECTION_AUTOMATIC_NO_OLBC = 3,
};

#define READ_BE16		read_be16
#define READ_BE24		read_be24
#define READ_BE32		read_be32
#define READ_BE64		read_be64

#define WRITE_BE16 		write_be16
#define WRITE_BE24 		write_be24
#define WRITE_BE32 		write_be32
#define WRITE_BE64		write_be64

#define READ_LE16		read_le16
#define READ_LE24		read_le24
#define READ_LE32		read_le32
#define READ_LE64		read_le64

#define WRITE_LE16 		write_le16
#define WRITE_LE24 		write_le24
#define WRITE_LE32 		write_le32
#define WRITE_LE64		write_le64

#define IEEE80211_FCTL_TODS		0x0100
#define IEEE80211_FCTL_FROMDS	0x0200

#define ETH_P_AARP	0x80F3		/* Appletalk AARP		*/
#define ETH_P_IPX	0x8137		/* IPX over DIX			*/

#define ETH_P_802_3_MIN	0x0600		/* If the value in the ethernet type is less than this value
					 * then the frame is Ethernet II. Else it is 802.3 */

/**
 * ieee80211_has_tods - check if IEEE80211_FCTL_TODS is set
 * @fc: frame control bytes in little-endian byteorder
 */
static inline int ieee80211_has_tods(u16 fc)
{
	return ((fc & cpu_to_le16(IEEE80211_FCTL_TODS)) != 0) ? 1 : 0;
}

/**
 * ieee80211_has_fromds - check if IEEE80211_FCTL_FROMDS is set
 * @fc: frame control bytes in little-endian byteorder
 */
static inline int ieee80211_has_fromds(u16 fc)
{
	return ((fc & cpu_to_le16(IEEE80211_FCTL_FROMDS)) != 0) ? 1 : 0;
}

/**
 * ether_addr_equal - Compare two Ethernet addresses
 * @addr1: Pointer to a six-byte array containing the Ethernet address
 * @addr2: Pointer other six-byte array containing the Ethernet address
 *
 * Compare two Ethernet addresses, returns true if equal
 *
 * Please note: addr1 & addr2 must both be aligned to u16.
 */
static inline int ether_addr_equal(const u8 *addr1, const u8 *addr2)
{
	const u16 *a = (const u16 *)addr1;
	const u16 *b = (const u16 *)addr2;

	return (((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) == 0) ? 1 : 0;
}

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

struct ethhdr {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	u16		h_proto;		/* packet type ID field	*/
} __attribute__((packed));

/* Frame control field constants */
#define IEEE80211_FCTL_VERS		0x0002
#define IEEE80211_FCTL_FTYPE		0x000c
#define IEEE80211_FCTL_STYPE		0x00f0
#define IEEE80211_FCTL_TODS		0x0100
#define IEEE80211_FCTL_FROMDS		0x0200
#define IEEE80211_FCTL_MOREFRAGS	0x0400
#define IEEE80211_FCTL_RETRY		0x0800
#define IEEE80211_FCTL_PM		0x1000
#define IEEE80211_FCTL_MOREDATA	0x2000
#define IEEE80211_FCTL_WEP		0x4000
#define IEEE80211_FCTL_ORDER		0x8000

#define IEEE80211_FTYPE_MGMT		0x0000
#define IEEE80211_FTYPE_CTL		0x0004
#define IEEE80211_FTYPE_DATA		0x0008

/* management */
#define IEEE80211_STYPE_ASSOC_REQ	0x0000
#define IEEE80211_STYPE_ASSOC_RESP	0x0010
#define IEEE80211_STYPE_REASSOC_REQ	0x0020
#define IEEE80211_STYPE_REASSOC_RESP	0x0030
#define IEEE80211_STYPE_PROBE_REQ	0x0040
#define IEEE80211_STYPE_PROBE_RESP	0x0050
#define IEEE80211_STYPE_BEACON		0x0080
#define IEEE80211_STYPE_ATIM		0x0090
#define IEEE80211_STYPE_DISASSOC	0x00A0
#define IEEE80211_STYPE_AUTH		0x00B0
#define IEEE80211_STYPE_DEAUTH		0x00C0

/* control */
#define IEEE80211_STYPE_PSPOLL		0x00A0
#define IEEE80211_STYPE_RTS		0x00B0
#define IEEE80211_STYPE_CTS		0x00C0
#define IEEE80211_STYPE_ACK		0x00D0
#define IEEE80211_STYPE_CFEND		0x00E0
#define IEEE80211_STYPE_CFENDACK	0x00F0

/* data */
#define IEEE80211_STYPE_DATA		0x0000
#define IEEE80211_STYPE_DATA_CFACK	0x0010
#define IEEE80211_STYPE_DATA_CFPOLL	0x0020
#define IEEE80211_STYPE_DATA_CFACKPOLL	0x0030
#define IEEE80211_STYPE_NULLFUNC	0x0040
#define IEEE80211_STYPE_CFACK		0x0050
#define IEEE80211_STYPE_CFPOLL		0x0060
#define IEEE80211_STYPE_CFACKPOLL	0x0070
#define IEEE80211_QOS_DATAGRP		0x0080
#define IEEE80211_QoS_DATAGRP		IEEE80211_QOS_DATAGRP

#define IEEE80211_SCTL_FRAG		0x000F
#define IEEE80211_SCTL_SEQ		0xFFF0

#endif /* _WLAN_DEF_H_ */
