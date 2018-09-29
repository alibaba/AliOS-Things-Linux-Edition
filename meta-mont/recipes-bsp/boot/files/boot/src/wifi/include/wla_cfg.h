/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wla_cfg.h
*   \brief  WLAN configuration
*   \author Montage
*/

#ifndef _WLAN_CFG_H_
#define _WLAN_CFG_H_

#define NEW_IE_PARSER

#define ENABLE_POWER_SAVING
#define NEW_BB
//#define DEBUG_MEM_CORRUPT

#define MAX_BSSIDS                  4
#define BEACON_TX_DESCRIPTOR_COUNT  MAX_BSSIDS
#define MAX_DS_NUM      16
#define MAX_STA_NUM     32
#define DEF_BUF_SIZE        2048    /* could be overwrite by global config */
#define MAX_AP_COUNT                4              /* max entries in DS table */
#define BUFFER_HEADER_POOL_SIZE                 192
#define MAX_STA_CAP_TBL_COUNT		8		/* total elements of station capability tables */

#ifdef CONFIG_LARGE_DESCRIPTOR
#define TX_DESCRIPTOR_COUNT			64		/* numbers of continuous allocated TX descriptors */ /* TX's number is at least tiwce of RX's to handle AMSDU */
#define RX_DESCRIPTOR_COUNT			64		/* numbers of continuous allocated RX descriptors */
#else
#define TX_DESCRIPTOR_COUNT			32		/* numbers of continuous allocated TX descriptors */ /* TX's number is at least tiwce of RX's to handle AMSDU */
#define RX_DESCRIPTOR_COUNT			32		/* numbers of continuous allocated RX descriptors */
#endif
#define MMAC_RX_DESCRIPTOR_COUNT	0

#define PSBAQ_DESCRIPTOR_COUNT		0

#define DEFAULT_QOS_RX_MPDU_WINDOW			( 32 )
#define DEFAULT_LEGACY_RX_MPDU_WINDOW		( 16 )

#define DEF_FASTPATH_BUF_SIZE		0

#define MAX_ADDR_TABLE_ENTRIES					8
#define MAX_DS_TABLE_ENTRIES					2
#define MAC_MAX_BSSIDS							3
#define MAX_PEER_AP								4

#define MAC_MAX_EDCAQ							4
#define MAC_MAX_TXQ								16
#define MAC_MAX_PSBAQ							MAC_MAX_TXQ - MAC_MAX_BSSIDS - 1 /* 1: reserved txq */

#define MAX_SW_BUFFER_HEADER_POOL_SIZE			300

#define BEACON_Q_BUFFER_HEADER_POOL_SIZE		MAC_MAX_BSSIDS

#define DEF_SW_PATH_BUF_SIZE					512

#define HT_LONG_RETRY_DEFAULT					8
#define HT_SHORT_RETRY_DEFAULT					8
#define LEGACY_LONG_RETRY_DEFAULT				8
#define LEGACY_SHORT_RETRY_DEFAULT				8

#define NBUF_LEN								1536

#define PSBAQ_BCAST_TX_QUEUE_LEN				255

/* HW characterics; DO NOT CHANGE IT! */

#define MAC_STA_CAP_TABLE_ENTRIES_PER_BADDR		64

#define MAC_ADDR_LOOKUP_STA_TABLE_ENTRIES		256

#define MAC_ADDR_LOOKUP_DS_TABLE_ENTRIES		16


#define MAX_RATE_TBL_COUNT						(MAC_MAX_BSSIDS)

#define MAX_QOS_TIDS							8

#define QOS_TX_STATUS_TBLS						MAX_QOS_TIDS
#define QOS_RX_STATUS_TBLS						( MAX_QOS_TIDS + 1)

#define MAX_PS_QUEUE_LENGTH						64 //(64 * 3)	/* PS queue should not too big to consume all buffers */
#define MAX_AMPDU_QUEUE_LENGTH					170 			/* To pass WIFI 2.4.30 case, large queue length to buffer 4 IP fragment traffics simultaneously */

#define EDCA_LEGACY_FRAME_TID					0

#define EDCA_HIGHTEST_PRIORITY_TID				7

#define MAX_TX_AMPDU_IDLE_TIME					10 // seconds

#define ACQ_MIN_SIZE							10
#define ACQ_TOTAL_SIZE							256

#define BEACON_BITRATE							CCK_1M

#define MAX_CONSECUTIVE_AMPDU_TX_FAILURE		15

#define WLAN_BCAST_THRESHOLD					100

#define WLA_HWADDR_AGE							30000 // 5 min

#define WRX_TBUF_SIZE							128  /* temp buffer size */

#define MLME_STORED_FRAME_SIZE					1000
#ifndef EXTRA_BUF_SIZE
#define EXTRA_BUF_SIZE							(8*1024)
#endif
#define EXTRA_PAGE_SIZE							(4096)

#define MBSS_MAX_NUM					2
#define WDS_MAX_NUM						4
#define WLAN_DEFAULT_BEACON_INTERVAL	5

#define MAX_AID_SIZE	127

#define MAX_NEIGHBOR_BSS_NUM			32

#define WLAN_ADDBA_MAX_RETRY_NUM		3

#define PSQ_AGING_TIME					2000 /* msec */

#if defined(CONFIG_FPGA)
/* adjust the pre-tbtt to be 1/3 of beacon interval */
#define PRE_TBTT_TIME(beacon_interval)  (((beacon_interval) * 1) / 3)
#else
/* minimize pretbtt time since ASIC is much faster */
#define PRE_TBTT_TIME(beacon_interval)  (((beacon_interval) * 1) / 8)
#endif

#define WLA_DUMP_BUF			hexdump

#endif /* _WLAN_CFG_H_ */
