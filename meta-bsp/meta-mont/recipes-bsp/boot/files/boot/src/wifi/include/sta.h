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

#ifndef _WM_STA_H_
#define _WM_STA_H_

//#include <radius_register.h>
#include <mac_common.h>
#include <wpa.h>
#include <wapi.h>

#define WLAN_STA_VALID				BIT(0)
#define WLAN_STA_AUTH				BIT(1)
#define WLAN_STA_ASSOC				BIT(2)
#define WLAN_STA_AUTHORIZED			BIT(3)
#define WLAN_STA_PS					BIT(4)
#define WLAN_STA_QOS				BIT(5)
#define WLAN_STA_HT					BIT(6)
#define WLAN_STA_ERP				BIT(7)
#define WLAN_STA_SHORT_SLOT			BIT(8)
#define WLAN_STA_SHORT_PREAMBLE		BIT(9)
#define WLAN_STA_HT_40M				BIT(10)
#define WLAN_STA_HT_GF				BIT(11)
#define WLAN_STA_WPS				BIT(12)
#define WLAN_STA_MAYBE_WPS			BIT(13)
#define WLAN_STA_AUTH_SHARED		BIT(14)
#define WLAN_STA_FORTY_INTOLERANT	BIT(15)		// dis-allow 20/40Mhz or not
#define WLAN_STA_ACTIVITY			BIT(16)
#define WLAN_STA_LINKED_AP			BIT(17)
#define WLAN_STA_WDS_LINKED_AP		BIT(18)
#define WLAN_STA_IBSS_STA			BIT(19)
#define WLAN_STA_NOA				BIT(20)

#define WLAN_TIME_UNIT				(1000)	/* timer unit : 1 ms */
#define STA_MAX_IDLE_TIME			(5*60*WLAN_TIME_UNIT)
#define STA_NULL_CHECK_TIME			(5*WLAN_TIME_UNIT)
#define STA_KEEP_ALIVE_TIME			(1*WLAN_TIME_UNIT)
#define STA_START_KEEP_ALIVE_TIME	(15*WLAN_TIME_UNIT)
#define STA_TRY_ASSOC_TIME			(5*WLAN_TIME_UNIT)
#define STA_HW_SYNC_TIME			(HW_SYNC_TIME)
#define STA_RSNA_TIMEOUT			(60*WLAN_TIME_UNIT)	// dot11RSNAConfigSATimeout
#define STA_PMKSA_TIMEOUT			(3600)				// 3600 = 1 hr (dot11RSNAConfigOMKLifetime = 43200)
#define STA_DYNAMIC_PS_TIMEOUT		(2*WLAN_TIME_UNIT)	
#define STA_LISTEN_BEACON_TIME		(3*WLAN_TIME_UNIT)
#define DYNAMIC_PS_TIME				(5*WLAN_TIME_UNIT)
#define DYNAMIC_ACT_TIME			(500)

#define STA_INITIATOR_ADDBA_TIME 	(WLAN_TIME_UNIT)
	
#define DEV_SITESURVEY_TIME			(200)	//	site suvery each channel with 500ms
#define DEV_ALIVE_TIME				(1*WLAN_TIME_UNIT)
#define DEV_SCAN_RESULT_HOLD_TIME	(3*WLAN_TIME_UNIT)

#define QUICK_SCAN_TIME				(100)	// 100ms
#define ON_OPERATION_TIME			(500)	// 500ms
#define SCAN_INTERVAL_FOR_2040_COEX	(30)	// 30 sec

#define BA_HOUSEKEEP_TIME			(10)

/* FIXME: not use anymore. remove it */
#if 0
#define STA_RESET_FLAGS		0	//STA_RELEASE_WITHOUT_FREE_MEM
#define STA_FREE_DELAYED	1	//STA_RELEASE_WITH_FREE_MEM_DELAYED
#define STA_FREE_IMMED		2	//STA_RELEASE_WITH_FREE_MEM
#endif

#define BROADCAST_STA		(-1)


#define BSS_FLG_ENABLE					BIT(0)
#define BSS_FLG_HIDDEN_SSID				BIT(1)
#define BSS_FLG_WMM						BIT(2)
#define BSS_FLG_NO_PROBE_RESP			BIT(3)
#define BSS_FLG_TKIP_COUNTERMEASURE		BIT(4)
#define BSS_FLG_DISABLE_BEACON			BIT(5)
#define BSS_FLG_MAT_CLIENT_MODE			BIT(6)
#define BSS_FLG_MATCH_BSSID				BIT(7)
#define BSS_FLG_RUN						BIT(8)
#define BSS_FLG_STATIC_IP				BIT(9)
#define BSS_FLG_DISABLE_DHCP			BIT(10)
#define BSS_FLG_LINKUP					BIT(11)
#define BSS_FLG_RECONNECT				BIT(12)

#define WMBSS_START_SECTOR	tsf_idx
#define WMBSS_END_SECTOR	sta_list
struct wm_bss {
	/*--- Host Setup Section : Start ---*/
	u8	tsf_idx;
	u8	bss_desc;				/* indicate Driver's VIF number */

	/* for TSF sync */
	u16	beacon_interval;
	u8	dtim_period;

	u8	phy_cap;	/* (panther dev cap) & (parent AP's cap) */
	u16 tx_rate_code;

	/* WMM */
	unsigned int wme_acm;
	/* FIXME: bss->slottime records the connection's ability & to effect the ldev->slottime */
	u16	slottime; 			/* 0: auto, 1: short(9us), 2: long(20us) */ 
	/* HT Capability */
	u16	ht_capability;
	u8	support_mcs_set[16];	/* FIXME: No Need ? */

	u8  myaddr[WLAN_ADDR_LEN]; 	/* My MAC address */
	u8	bssid[WLAN_ADDR_LEN];	/* BSSID in the bss group (ex: ibss group BSSID) */

	u8	ssid[MAX_SSID_LEN];
	u8	ssid_len;

	u16	role;				/* ex: WIF_AP_ROLE ... */
	u32	flag;
	u32	auth_capability;
	u8	ampdu_params;

	/* IBSS */
	u8 	ibss_state;
	u16 atim_window;

	/*--- Host Setup Section : End ---*/

	/***** Below is that can't been overwrite when host update bss data *****/
	/* sta_list must in the head of which can't been overwrited. */
	struct sta_ctx *sta_list; /* STA info list head */
	/* ie pool pointer */
	struct wlan_ie ie_pool;

	u32 wb1_field;
	u32 wb2_field;
	u32 tsc[4];
	u32 rsc[4];

	struct tx_q *bcq;
	struct beacon_desc *beacon_desc;
	struct beacon_desc *new_beacon_desc;

	u8	sta_num; 			/* current number of sta */
	u8	ps_sta_num; 		/* current number of power saving sta */
	u16	last_aid;
	u32 last_mic_failure;
	u16 mic_failure_count;
	u16 tx_seq_num;			/* global sequence number */
	
	u8	dialogtoken; /* global dialogtoken for ADDBA use */

	u8	key_idx;
	u8	wep_type;
	u8	auth_mode;
	u8	psk[PMK_LEN];
	u8 	wpa_counter[WPA_NONCE_LEN];
	u8	wep[WLAN_WEP_NKID][WLAN_WEP_MAX_KEYLEN];

#ifdef CONFIG_WPS
	struct 	wps_data *wps;	//one per bss
	u8 	*wps_ie;
	u8	*wps_ie2;
	u16	wps_ie_len;
	u16	wps_ie2_len;
#endif

	struct pmksa_cache_entry pmksa; /* pmksa cache entry */

	u32	ptk_rekey_time;
	u32	gtk_rekey_time;

	union {
#ifdef CONFIG_WPA
		struct wpa_group_key wpa_group;
		struct wapi_group_key wapi_group;
#endif
	} group_key;

	unsigned char ps_state;
	unsigned char ps_tx_null;
	unsigned char ps_pending;
	unsigned char ps_policy;
	unsigned int busy_timestamp;		/* unit: 10ms, os_current_time() */

	u8 *rsn_ie;
	u8 *wpa_ie;

#ifdef CONFIG_IBSS_ATIM
	struct beacon_desc *atim_bc_desc;
#endif
	/* iot */
	u32 ipaddr;
	u32 netmask;
	u32 gateway;
	u32 dns_svr;
	u16 retry_interval;					/* time unit is millisecond */
	u8 op_state;
	u8 dhcp;

	unsigned char password[65];

#ifdef CONFIG_HOST_WPS
	u8  beacon_ie_len;
	u8  probe_req_ie_len;
	u8  probe_resp_ie_len;
	u8  assoc_req_ie_len;
	u8  assoc_resp_ie_len;
	u8	padding[2];	// FIXME: for alignment
	/* HOST WPS P2P IE */
	u8  *beacon_ie;     /*26~51 bytes for WPS Host mode */
	u8  *probe_req_ie;  /*135 bytes for WPS Client mode */
	u8  *probe_resp_ie; /*94~119 bytes for WPS Host mode */
	u8  *assoc_req_ie;  /*not use cause host not implement.TODO: check p2p if need */
	u8  *assoc_resp_ie; /*26 bytes for WPS Host mode*/
#endif /*CONFIG_HOST_WPS*/
} __attribute__ ((packed, aligned(4)));		/* FIXME: is "packed" needed for attach cmd to directy copy? */

#define STA_STATIC_NO_ERP				BIT(0)
#define STA_STATIC_NO_HT				BIT(1)
#define	STA_STATIC_HT_NO_GF				BIT(2)
#define	STA_STATIC_HT_20MHZ				BIT(3)
#define	STA_STATIC_HT_40MHZ_PROHIBIT	BIT(4)
#define STA_STATIC_NO_SHORT_PREAMBLE	BIT(5)
#define STA_STATIC_NO_SHORT_SLOTTIME	BIT(6)
#define STA_STATIC_PROTECT_MASK			STA_STATIC_NO_ERP|STA_STATIC_NO_HT|STA_STATIC_HT_NO_GF|STA_STATIC_HT_20MHZ

#define OBSS_STATIC_NON_ERP				BIT(0)
#define	OBSS_STATIC_NON_HT				BIT(1)
#define	OBSS_NO_40_PERMITTED			BIT(2)
#define OBSS_STATIC_PROTECT_MASK		OBSS_STATIC_NON_ERP|OBSS_STATIC_NON_HT

struct tx_rate_info {
	u8 rate_idx;
	u8 bts_rates;
};

/* station context */
struct sta_ctx {
	struct sta_ctx *next;
	u32		sync_time;
	u32		timestamp;

	u8		addr[WLAN_ADDR_LEN];			/* STA's mac address */
	u8 		previous_ap[WLAN_ADDR_LEN];

	u16		aid; 				/* STA's unique AID (1 .. 2007) or 0 if not yet assigned */
	u8 		addr_idx;			/* indicator index of bcap table */
	u8		bss_desc;

	u8		ampdu_params;
	u8		max_sp_len;
	u16		listen_interval;

	u32		flags;

	/* fill in txd */
	u32		wb1_field;
	u32		wb2_field;
	u16		tx_rate[4];
	u32		tsc[4];

#define BTS_RATE_MAX_NUM 20
	struct bts_cb {
		struct bts_rate {
			u32 timestamp;			/* Timestamp of transmited using this rate */
			u32 tp;					/* throughput */

			u16 avg_prob;			/* average probability of successful transmission */
			u8 res;
			u8 rate_idx;

			u16 success;
			u16 attempts;
			u16 hw_success;
			u16 hw_attempts;
			u16 pre_success;
			u16 pre_attempts;
		} rates[BTS_RATE_MAX_NUM];

		struct tx_rate_info aux_tx_rate[4];			/* auxiliary rate set to probe at background */
		u32 sample_timestamp;
		u8 cur_bts_rate[4];							/* indicates bts_rates without decode */

		u8 aux;
		u8 rates_len;
		u16 probation;
	} bts;
	struct tx_rate_info cur_tx_rate[4];

	u8		rate_flags;
	u8		pre_trycnt;								/* previous trycnt for start of aggregation */
	u8		timeout_lock;
	u8		sample_cnt;

	u8		pre_rate_idx[4];						/* previous rate index for start of aggregation */

	u16		current_cnt;
	u16		current_cnt_div;

	u32		supported_rates;
	u32		rsc[9][4];

	u32		rx_bytes;
	u32		rx_cnt;
	u32		rx_last_cnt;							/* for keep alive feature */

	u32		tx_bytes;
	u32		tx_cnt;

	u16		beacon_cnt;								/* for keep alive feature */
	u16		beacon_last_cnt;						/* for keep alive feature */

	u8		res;
	u8		tx_null_fail_cnt; 						/* for ibss mode keep alive */
	u8		ps_null_fail_cnt;						/* debug client mode power saving */
	u8		ps_pm_null_fail_cnt;					/* debug client mode power saving */

	u8		ps_policy;
	u8		apsd_trigger_deliver;
	u8		rssi;
	u8		snr;

	u8		rc_allow_ampdu;
	u8		tx_ampdu_inhibit;
	u8		tx_ampdu_bitmap;
	u8		chip_vendor;

	struct tx_t{
		u16	tx_seq_num;
		u8	qidx;
		u8	req_num;					/* record how many times to send ADDBA or fail to send AMPDU */
		u8	dialogtoken;
		u8	res[3];
	} tx[8];
	struct tx_q	psq;
	u8		ba_rx_idx[MAX_QOS_TIDS];

	u16 	rx_seq_num[9];
	u8		ts_synced;					/* for TBTT_EXCEPTION_HANDLE & de-auth by beacon loss */
	u8		ibss_ds_idx;

	u8 		*ibss_bssid;

	union {
		struct wpa_ctx wpa;
		struct wapi_ctx wapi;
	} wpa_wapi_ctx;
	struct eap_ctx *eap_ctx;

	/* record the bhdr head & tail to delay send the wci evt (ex: ap role connect evt) */
	u16		bhdr_head_idx;
	u16		bhdr_tail_idx;

#ifdef CONFIG_IBSS_ATIM
	struct beacon_desc *atim_uc_desc;
#endif
} __attribute__ ((aligned(4)));

#endif /* _WM_STA_H_ */
