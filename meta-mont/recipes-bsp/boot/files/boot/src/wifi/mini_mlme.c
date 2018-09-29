/*=============================================================================+
|                                                                              |
| Copyright 2017                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file   mini_mlme.c
*   \brief  mini version of MLME of station
*   \author Montage Inc.
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
//#include <lynx_dev.h>
#include <lib.h>
#include <wla_cfg.h>
#include <wla_def.h>
#include <common.h>
//#include <stdio.h>
#include <sta.h>
//#include <mlme.h>
//#include <wla_ie.h>
#include <wbuf.h>
//#include <wla_api.h>
//#include <eapol.h>
//#include <scan.h>
//#include <ibss.h>

//#include <stdlib.h>
#include <mac_ctrl.h>
#include <mac_regs.h>
//#include <event.h>
//#include <txrx.h>

#include <mac_sim.h>
#include <mt_types.h>
#include <panther_app.h>
#include <netprot.h>
#include <netdev.h>

/*=============================================================================+
| Define
+=============================================================================*/
#define NEW_IE_PARSER

#define MAX_PROBERESP_LEN 768

#define ETH_HLEN    14
#define ETHER_ADDR_LEN  6

u8 challenge[WLAN_AUTH_CHALLENGE_LEN]; 	/* for shared Key Auth, only allow one progress */

#define WLAN_IE_CONTEXT_TO_LEN(_context)  (*(char *)(_context - 1))

extern unsigned char broadcast[6];
extern unsigned char zeros[6];

#define BC_ADDR	  broadcast

/* See IEEE 802.1H for LLC/SNAP encapsulation/decapsulation */
/* Ethernet-II snap header (RFC1042 for most EtherTypes) */
const unsigned char rfc1042_header[] __attribute__ ((aligned (2))) = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00 };

/* Bridge-Tunnel header (for EtherTypes ETH_P_AARP and ETH_P_IPX) */
const unsigned char bridge_tunnel_header[] __attribute__ ((aligned (2))) = { 0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8 };

typedef void (*FUNCPTR)(void *);

#if defined(NEW_IE_PARSER)
u8 ie_mapping_table[] =
{
    WLAN_ELEMID_SSID,
    WLAN_ELEMID_SUPP_RATES,
    WLAN_ELEMID_DS_PARMS,
    WLAN_ELEMID_TIM,
    WLAN_ELEMID_IBSS_PARMS,
    WLAN_ELEMID_COUNTRY,
    WLAN_ELEMID_CHALLENGE,
    WLAN_ELEMID_PWR_CAP,
    WLAN_ELEMID_SUPP_CHAN,
    WLAN_ELEMID_ERP_INFO,
    WLAN_ELEMID_HT_CAP,
    WLAN_ELEMID_RSN,
    WLAN_ELEMID_EXT_SUPP_RATES,
    WLAN_ELEMID_MOBILITY_DOMAIN,
    WLAN_ELEMID_FAST_BSS_TRANS,
    WLAN_ELEMID_TIMEOUT_INTVAL,
    WLAN_ELEMID_HT_INFO,
    WLAN_ELEMID_EXT_CAP,
    WLAN_ELEMID_WAPI,

    WLAN_ELEMID_VENDOR_SPEC,
};

u32 oui_mapping_table[] =
{
    (MICROSOFT_OUI << 8)|WPA_OUI_TYPE,
    (MICROSOFT_OUI << 8)|WME_OUI_TYPE,
    (MICROSOFT_OUI << 8)|WPS_OUI_TYPE,
};
#endif

struct wm_bss bss_ctxs[MAC_MAX_BSSIDS] __attribute__((aligned(4))) = {
    /* BSS0 */
    {
        .bss_desc = 0,
        .tsf_idx = 1,
        .beacon_interval = 100,
        .myaddr = {0x10, 0x05, 0x06, 0x07, 0x08, 0x01},
        .bssid = {0x10, 0x05, 0x06, 0x07, 0x08, 0x01},
        .role = WIF_AP_ROLE,
        .tx_rate_code = 0x1b00, /* B mode */
        .ssid = {'r','e','c','o','v','e','r', 'y', 0x0},
        .ssid_len = 8,
        .retry_interval = 100,
        .auth_capability = AUTH_CAP_OPEN,
    },
//  {
//  	.bss_desc = 0,
//  	.tsf_idx = 1,
//  	.myaddr = {0x00, 0x32, 0x81, 0xf5, 0xad, 0x01},
//  	.bssid = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},
//  	.role = WIF_STA_ROLE,
//  	.tx_rate_code = 0x1b00, /* B mode */
//  	.ssid = {'D','e','m','o','_','A','P',0x0},
//  	.ssid_len = 7,
//  	.retry_interval = 100,
//  	.password = {'1','2','3','4','5','6','7','8',0x0},
//  },
    /* BSS1 */
    {
        .bss_desc = 1,
        .tsf_idx = 0,
        .myaddr = {0x00, 0x32, 0x81, 0xf5, 0xad, 0x02},
        .bssid = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},
        .role = WIF_AP_ROLE,
        .tx_rate_code = 0x1b00, /* B mode */
        .ssid = {'D','e','m','o','_','A','P','1',0x0},
        .ssid_len = 8,
        .retry_interval = 100,
        .password = {'1','2','3','4','5','6','7','8',0x0},
    }
};

#define AP_CAP_11B					BIT(0)
#define AP_CAP_11G					BIT(1)
#define AP_CAP_11N					BIT(2)
#define AP_CAP_11A					BIT(3)

struct panther_app app_entity = {
#ifdef CONFIG_WLA
    .phy_cap = AP_CAP_11B | AP_CAP_11G, // | AP_CAP_11N,
    .slottime = SLOTTIME_9US,
    .capability = WLAN_CAPABILITY_ESS | WLAN_CAPABILITY_SHORT_SLOT_TIME |
                  WLAN_CAPABILITY_SHORT_PREAMBLE, /* WLAN_CAPABILITY_APSD */
    .channel = 7,                                   /* default channel */
    .bandwidth = BW40MHZ_SCN,                       /* 40MHz */
    .ampdu_params = 0x2,                            /* Maximum AMPDU RX length is 32K */
#if (WRX_BUF_SIZE > 64)
    .rx_ba_win_size = 16,
#else
    .rx_ba_win_size = 8,
#endif
    /* Once tx window size is 32, Broadcom station can not response BA under throughput test. */
    .tx_ba_win_size = ((TX_DESCRIPTOR_COUNT > 8) ? 8 : TX_DESCRIPTOR_COUNT),
                      .rts_threshold = (2346 - 24),
                                       .modem_ps = 0,
    .modem_ps_policy = PS_POLICY_PSPOLL,
    .tx_ba_to = 0,
    .listen_interval_max = 100,
    .my_ac_parms = {
#if !defined(CONFIG_RATEADAPTION2) // MP
        { .qidx = 1, .cwmin = 15, .cwmax = 511, .aifs = 3, .txoplimit = 0,},/* AC_BE */
        { .qidx = 0, .cwmin = 15, .cwmax = 511, .aifs = 7, .txoplimit = 0,},/* AC_BK */
        { .qidx = 2, .cwmin = 7,  .cwmax = 15,  .aifs = 2, .txoplimit = 94,},/* AC_VI */
        { .qidx = 3, .cwmin = 3,  .cwmax = 7,   .aifs = 2, .txoplimit = 47,} /* AC_VO */
#elif defined(CONFIG_FPGA)
        { .qidx = 1, .cwmin = 15, .cwmax = 1023, .aifs = 3, .txoplimit = 0,}, /* AC_BE */
        { .qidx = 0, .cwmin = 15, .cwmax = 1023, .aifs = 7, .txoplimit = 0,}, /* AC_BK */
        { .qidx = 2, .cwmin = 7,  .cwmax = 15,   .aifs = 2, .txoplimit = 94,}, /* AC_VI */
        { .qidx = 3, .cwmin = 3,  .cwmax = 7,    .aifs = 2, .txoplimit = 47,} /* AC_VO */
#elif defined(CONFIG_TXOP_PARAM)
        /* FIXME TXOP + power-saving lets MAC tx hang, disable TXOP in AP mode at A1 */
        { .qidx = 1, .cwmin = 63, .cwmax = 127, .aifs = 3, .txoplimit = 0,}, /* AC_BE */
        { .qidx = 0, .cwmin = 63, .cwmax = 127, .aifs = 7, .txoplimit = 0,}, /* AC_BK */
        { .qidx = 2, .cwmin = 7,  .cwmax = 15,  .aifs = 2, .txoplimit = 0,}, /* AC_VI */
        { .qidx = 3, .cwmin = 3,  .cwmax = 7,   .aifs = 2, .txoplimit = 0,} /* AC_VO */    
#else
        /* FIXME TXOP + power-saving lets MAC tx hang, disable TXOP in AP mode at A1 */
        { .qidx = 1, .cwmin = 15, .cwmax = 127, .aifs = 3, .txoplimit = 0,}, /* AC_BE */
        { .qidx = 0, .cwmin = 15, .cwmax = 127, .aifs = 7, .txoplimit = 0,}, /* AC_BK */
        { .qidx = 2, .cwmin = 7,  .cwmax = 15,  .aifs = 2, .txoplimit = 0,}, /* AC_VI */
        { .qidx = 3, .cwmin = 3,  .cwmax = 7,   .aifs = 2, .txoplimit = 0,} /* AC_VO */
#endif
    },
    .wrx_mon_bmap = RXF_GC_MGT_TA_HIT|RXF_GC_DAT_TA_HIT|RXF_UC_MGT_RA_HIT|RXF_UC_DAT_RA_HIT|RXF_BEACON_TA|RXF_PROBE_REQ_ALL,
    .wrx_mon_bmap_old  = RXF_GC_MGT_TA_HIT|RXF_GC_DAT_TA_HIT|RXF_UC_MGT_RA_HIT|RXF_UC_DAT_RA_HIT|RXF_BEACON_TA|RXF_PROBE_REQ_ALL,
    .current_rx_rates = B_RATES|OFDM_RATES, //ALL_SUPPORTED_RATES,
    .current_tx_rates = B_RATES|OFDM_RATES, //ALL_SUPPORTED_RATES,
    .basic_rates = ALL_BASIC_RATES,
#if defined(CONFIG_CH14)
    .country = {'J','P',0x0},
#else
    .country = {'C','N',0x0},
#endif
    .channel_num = MAX_CHANNEL_NUM,
    .bss_total_num = 3,
    .sta_total_num = 8,
    .nbss_total_num = 32,
    .rc_rts = 0,        // disable RTS
    .rc_rtsr = 0,       // 24M RTS
    .op_flag = DEV_KEEPALIVE,

    .resend_mode = 1,   // 0:disable, 1:always, 2:by RSSI
    .resend_max_cnt = 3,
    .resend_min_rssi = 75,
#ifdef	CONFIG_WIFI_TEST_PARAM
    .mac_wd_to = 0,     // for WIFI VERIFIED
#else
    .mac_wd_to = 2,
#endif  /*CONFIG_WIFI_TEST_PARAM*/

    .op_bss_bmap = 0,
    .buflen = 0,
    .min_mpdu_space = 2,
    .otp_bb_txpwr = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
#endif
};
struct panther_app *p_app = &app_entity;

u16 rates_in_100kbps[] = {
    10,
    20,
    55,
    110,
    60,
    90,
    120,
    180,
    240,
    360,
    480,
    540,
};

struct wlan_channel_info {
	//u16 freq; /* frequency in MHz */
	u8 max_txpower; /* maximum transmit power in dBm */
	u8 flag;
	u32 cca_busy;
	u8 rssi;
	u8 bss_num;

	/* for auto channel selection */
	u8 noise_floor;
	u32 active_time;
	u32 busy_time;
};

/* cap_info flags */
#define CAP_INFO_VALID 				BIT(0)
#define CAP_INFO_NON_ERP 			BIT(1)
#define CAP_INFO_NON_HT 			BIT(2)
#define CAP_INFO_WMM_SUPPORT 		BIT(3)
#define CAP_INFO_NO_40_PERMITTED	BIT(4)

#define SMRT_SSID_LEN 32
struct cap_info {
	u32 flags;
	u8	ssid[SMRT_SSID_LEN+1];
	u8	ssid_len;
	u8	phy_cap;
	u16 pairwise_auth_cap;	/* including security type (WPA/WPA2/...) */
	u16	group_auth_cap;
	u16 key_mgmt;
	u8	channel;
	u8	bandwidth;
	u32	supp_rates;
	/* for TSF sync */
	u8	dtim_period;
	/* HT Capability */
	u16	ht_capability;
	u8	ampdu_params;
	u8	ht_op_mode;	/* FIXME: is the op_mode is need for all the case? */
	/* WME */
	u8 qinfo;
	struct wme_ie_data wme_info;
};

struct wlan_channel_info channels[MAX_CHANNEL_NUM];

struct sta_ctx m_sta;

u8 m_wpa_ie[64];
u8 m_rsn_ie[64];

u8 *ieee80211_data_to_8023(u8 *dptr, u32 *len);
u8 *ieee80211_data_from_8023(u8 *dptr, u32 *len);

/*=============================================================================+
| Utility Functions
+=============================================================================*/
u8 *buf_pull(u8 *buf, u32 *len, u32 offset)
{
    buf += offset;
	*len -= offset;

    return buf;
}

u8 *buf_push(u8 *buf, u32 *len, u32 offset)
{
    buf -= offset;
	*len += offset;

    return buf;
}

void mini_mlme_init(void)
{
    struct wm_bss *bss = &bss_ctxs[0];

    if (bootvars.mac0)
    {
        memcpy(bss->myaddr, bootvars.mac0, ETH_ALEN);
        memcpy(bss->bssid, bootvars.mac0, ETH_ALEN);
        sprintf((char *) bss->ssid, "Recovery_%02x%02x%02x", bootvars.mac0[3], bootvars.mac0[4], bootvars.mac0[5]);
        bss->ssid_len = strlen((char *) bss->ssid);
    }
}

void build_mgt_frame(struct wlan_hdr *hdr, u32 subtype, struct wm_bss *bss, struct sta_ctx *sta)
{
    u8 *ra = NULL;
    u8 *bssid = bss->bssid;

    if (subtype == WLAN_FC_SUBTYPE_PROBE_REQ)
    {           /* special case */
        if (sta)
            bssid = ra = (u8 *) sta;
        else
            bssid = ra = (u8 *)BC_ADDR;
    }
    else if (bss->role == WIF_STA_ROLE)
    {
        ra = sta->addr;
    }
    else
    {
        if (subtype == WLAN_FC_SUBTYPE_PROBE_RESP)
        {
            ra = (u8 *) sta;
        }
        else
        {
            if (sta)
            {
//              int idx = STA_TO_IDX(sta);
//
//              if ((idx >= 0) && (idx < MAX_STA_CAP_TBL_COUNT))
                    ra = sta->addr;
//              else
//                  ra = (u8 *)sta;	// invalid station is SA address.
            }
            else
            {
                ra = (u8 *)BC_ADDR;
            }
        }
    }

    /* FIXME: fc filed has other definition, which should be keep */
    hdr->fc |= htons(WLAN_FC_TYPE_MGT | subtype);
    memcpy(hdr->addr1, ra, WLAN_ADDR_LEN);
    memcpy(hdr->addr2, bss->myaddr, WLAN_ADDR_LEN);
    memcpy(hdr->addr3, bssid, WLAN_ADDR_LEN);
}

u16 ap_encode_capability(struct wm_bss *mybss)
{
    int capab = p_app->capability;
    int privacy = 0;

    if (p_app->stat.sta_static_new & STA_STATIC_NO_SHORT_PREAMBLE)
        capab &= ~WLAN_CAPABILITY_SHORT_PREAMBLE;

    if ((mybss->auth_capability & (AUTH_CAP_CIPHER_WEP40|AUTH_CAP_CIPHER_WEP104)))
        privacy = 1;

    //if((mybss->flag & BSS_FLG_WMM) == 0)
    //	capab &= ~WLAN_CAPABILITY_QOS;

    if ((p_app->slottime == SLOTTIME_20US) || ((p_app->slottime == 0) && 
                                               (!(p_app->phy_cap & (AP_CAP_11G|AP_CAP_11N)) ||
                                                (p_app->stat.sta_static_new & STA_STATIC_NO_SHORT_SLOTTIME))))
        capab &= ~WLAN_CAPABILITY_SHORT_SLOT_TIME;

    if (p_app->modem_ps_policy & PS_POLICY_UAPSD)
        capab |= WLAN_CAPABILITY_APSD;

    if (mybss->auth_capability & (AUTH_CAP_WPA|AUTH_CAP_WPA2))
        privacy = 1;

    if (mybss->auth_capability & AUTH_CAP_WAPI)
        privacy = 1;

    if (privacy)
        capab |= WLAN_CAPABILITY_PRIVACY;

    return capab;
}

u32 support_rate_to_bits(u8 *data, u8 len)
{
	u8 *pos, *end;
	u32 rates;

	rates = 0;
	pos = data;
	end = data + len;
	while(pos < end)
	{
		switch(*pos & 0x7f)
		{
			case 2:	
				rates |= R_BIT(CCK_1M);
				break;
			case 4:	
				rates |= R_BIT(CCK_2M);
				break;
			case 11:	
				rates |= R_BIT(CCK_5_5M);
				break;
			case 22:	
				rates |= R_BIT(CCK_11M);
				break;
			case 12:	
				rates |= R_BIT(OFDM_6M);
				break;
			case 18:	
				rates |= R_BIT(OFDM_9M);
				break;
			case 24:	
				rates |= R_BIT(OFDM_12M);
				break;
			case 36:	
				rates |= R_BIT(OFDM_18M);
				break;
			case 48:	
				rates |= R_BIT(OFDM_24M);
				break;
			case 72:	
				rates |= R_BIT(OFDM_36M);
				break;
			case 96:	
				rates |= R_BIT(OFDM_48M);
				break;
			case 108:	
				rates |= R_BIT(OFDM_54M);
				break;
			default:
				break;
		}
		pos++;
	}
	return rates;
}

u32 wlan_parse_ie(u8 *pos, int left, struct wlan_ie *ie, struct cap_info *info)
{
#if defined(NEW_IE_PARSER)
    //struct ht_capability *ht_cap;
    //struct wpa_ie_data wpa_ie;
    struct wlan_ie_generic *one = NULL;
    int i, *ptr, max_i;
    //int j, max_j;
    u8 id;
#endif	// NEW_IE_PARSER

    /* FIXME: should check no issue that ie set to "0" here */
    memset((void *) ie, 0, sizeof(struct wlan_ie));
    max_i = sizeof(ie_mapping_table)/sizeof(ie_mapping_table[0]);

    while (left >= 2)
    {
        one = (struct wlan_ie_generic *)pos;
        id = one->id;

        MIN_LENGTH_VERIFY(left, one->len, return WLAN_RET_ERROR);

        /* sizeof(struct wlan_ie)-4) : 4 = (u32 offset_base) */
        for (i = 0; i < max_i; i++)
        {
            if (id != ie_mapping_table[i])
                continue;
            ptr = ((int *)ie + i);

            if (info == NULL)
            {
				/* don't parse the detail */
            }
            else if (id == WLAN_ELEMID_SSID)
            {
				if (one->len > 32)
					info->ssid_len = 32;
				else
					info->ssid_len = one->len;
				memcpy(info->ssid, one->data, info->ssid_len);
			}
            else if (id == WLAN_ELEMID_DS_PARMS)
            {
				info->channel = *one->data;
			}
            else if (id == WLAN_ELEMID_TIM)
            {
				info->dtim_period = ((struct wlan_ie_tim *)one)->period;
			}
            else if (id == WLAN_ELEMID_ERP_INFO)
            {
				/* 1. (ie.supported_rates < R_BIT(OFDM_6M) == 1 :
						only has 1, 2, 5.5, 11 Mbps rates
				   2. (ie.erp_info && (ie.erp_info & 0x1)) == 1 :
						NonERP_Present bit is set to 1 in the beacon
				 */
                info->phy_cap |= AP_CAP_11G;
				
				if ((*(one->data) & WLAN_ERP_NON_ERP_PRESENT))
					info->flags |= CAP_INFO_NON_ERP;
			}
            else if (id == WLAN_ELEMID_SUPP_RATES)
            {
				MAX_LENGTH_VERIFY(one->len, 32, return WLAN_RET_ERROR);

				/* FIXME: should init supp_rates with a non-zero value? */
				info->supp_rates |= support_rate_to_bits(one->data, one->len);

				if (info->supp_rates & R_BIT(CCK_5_5M))
					info->phy_cap |= AP_CAP_11B;
			}
            else if (id == WLAN_ELEMID_EXT_SUPP_RATES)
            {
				MAX_LENGTH_VERIFY(one->len, 32, return WLAN_RET_ERROR);
				info->supp_rates |= support_rate_to_bits(one->data, one->len);
			}

            /* record the pointer of the ie entry in the ie pool */
            *ptr = (int) one->data;
        }
//skip:
        left -= one->len + 2;
        pos += one->len + 2;
    }

    if (left)
        return WLAN_RET_ERROR;

    return WLAN_RET_OK;
}

u16 ap_encode_wpa_ie(struct wm_bss *bss, u8 *start, u8 is_rsn)
{
	u32 num_suites, val;
	u8 *pos, *count;
	int sta = (bss->role == WIF_STA_ROLE);

	pos = start;
	if (!is_rsn) {
		WRITE_BE32(pos, WPA_AKM_SUITE_1X);
		pos += 4;
	}

	/* rsn version is same as wpa version */
	WRITE_LE16(pos, WPA_VERSION);
	pos += 2;
	if (bss->auth_capability & AUTH_CAP_CIPHER_TKIP) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_TKIP;
		else
			val = RSN_CIPHER_SUITE_TKIP;
		WRITE_BE32(pos, val);
	} else if (bss->auth_capability & AUTH_CAP_CIPHER_CCMP) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_CCMP;
		else
			val = RSN_CIPHER_SUITE_CCMP;
		WRITE_BE32(pos, val);
	} else if(bss->auth_capability & AUTH_CAP_CIPHER_WEP104) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_WEP104;
		else
			val = RSN_CIPHER_SUITE_WEP104;
		WRITE_BE32(pos, val);
	} else if(bss->auth_capability & AUTH_CAP_CIPHER_WEP40) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_WEP40;
		else
			val = RSN_CIPHER_SUITE_WEP40;
		WRITE_BE32(pos, val);
	} else {
		return 0;
	}
	pos += SELECTOR_LEN;

	num_suites = 0;
	count = pos;
	pos += 2;

	/* fill in pairwise cipher */
	if ((!sta || !num_suites) &&
		(bss->auth_capability & AUTH_CAP_CIPHER_CCMP)) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_CCMP;
		else
			val = RSN_CIPHER_SUITE_CCMP;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}
	if ((!sta || !num_suites) &&
		(bss->auth_capability & AUTH_CAP_CIPHER_TKIP)) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_TKIP;
		else
			val = RSN_CIPHER_SUITE_TKIP;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}

	/* ??? */
	if ((bss->auth_capability & ALL_CIPHER) == 0) {
		if (!is_rsn)
			val = WPA_CIPHER_SUITE_NONE;
		else
			val = RSN_CIPHER_SUITE_NONE;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}
	if (num_suites == 0)
		return 0;
	WRITE_LE16(count, num_suites);

	num_suites = 0;
	count = pos;
	pos += 2;

	/* fill in key management */
	if ((!sta || !num_suites) &&
		(bss->auth_capability & AUTH_CAP_KEY_MGT_1X)) {
		if (!is_rsn)
			val = WPA_AKM_SUITE_1X;
		else
			val = RSN_AKM_SUITE_1X;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}
	if ((!sta || !num_suites) &&
		(bss->auth_capability & AUTH_CAP_KEY_MGT_PSK)) {
		if (!is_rsn)
			val = WPA_AKM_SUITE_PSK;
		else
			val = RSN_AKM_SUITE_PSK;
		WRITE_BE32(pos, val);

		pos += SELECTOR_LEN;
		num_suites++;
	}

	if (num_suites == 0)
		return 0;
	WRITE_LE16(count, num_suites);

	if (is_rsn) {
		/* RSN capability */
		//WRITE_LE16(pos, 0);
		/* FIXME: the rsn ie in the probe resp/assoc req & that in the eapol must be the same. */
		/* It seems that hostapd set this as 0x000c and wpa_supplicant set as 0 */
		if (bss->role == WIF_AP_ROLE)
			WRITE_LE16(pos, 0x000c);
		else
			WRITE_LE16(pos, 0);
		pos += 2;

		/* PMKID */
	}

	return (pos - start);
}

char cw2ecw(short cw)
{
	char i;
	for (i = 0; i < 10; i++) {
		if (cw < (1 << i))
			break;
	}
	return i;
}

u32 wlan_ap_encode_ie_func(struct wm_bss *bss, u8 *start, u32 encode_bits)
{
    u32 total_len, ie_len, mask;
    u32 i, which;
    u8 *pos = start;
    u32 supp_rates = 0;
    u8 host_multies_flag = 0;
    total_len = 0;
    mask = 1;

    while (1)
    {
        ie_len = 0;
        if (mask == IE_END_BIT)
            break;
        which = (encode_bits & mask);
        switch (which)
        {
            case IE_SSID_BIT:
                {
                    ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_SSID;
                    memcpy(((struct wlan_ie_generic *)pos)->data, bss->ssid, bss->ssid_len);
                    ie_len = 2 + bss->ssid_len;
                    break; 
                }
            case IE_SUPP_RATES_BIT:
            case IE_EXT_SUPP_RATES_BIT:
                {
                    u32 val, num = 0;

                    if ((bss->role == WIF_STA_ROLE) && 
                        (bss->sta_list && bss->sta_list->supported_rates))
                    {
                        supp_rates = bss->sta_list->supported_rates;
                    }
                    else
                    {
                        supp_rates = p_app->current_rx_rates;
                    }

                    ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_SUPP_RATES;
                    ie_len = 2;

                    for (i = LOWEST_BITRATE; i < HT_LOWEST_BITRATE; i++)
                    {
                        val = R_BIT(i);
                        if ((supp_rates & val) == 0)
                            continue;
                        num++;
                        if (num > 8)
                        {
                            if (which == IE_SUPP_RATES_BIT)
                                break;
                        }
                        else
                        {
                            if (which == IE_EXT_SUPP_RATES_BIT)
                                continue;
                        }

                        pos[ie_len] = rates_in_100kbps[i-1]/5;

                        if (p_app->basic_rates & val)
                            pos[ie_len] |= 0x80;

                        ie_len++;
                    }

                    if (which == IE_EXT_SUPP_RATES_BIT)
                    {
                        if (num <= 8)
                            ie_len = 0;
                        else
                            ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_EXT_SUPP_RATES;
                    }

                    break;
                }
            case IE_DS_PARMS_BIT:
                {
                    ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_DS_PARMS;
                    ((struct wlan_ie_generic *)pos)->data[0] =  p_app->channel;
                    ie_len = 3;
                    break;
                }
            case IE_COUNTRY_BIT:
                {
                    u32 num;
                    struct wlan_channel_info *curr, *prev;
                    struct wlan_country_str *cstr;

                    ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_COUNTRY;
                    /* country string length always have 3 bytes */
                    memcpy(((struct wlan_ie_generic *)pos)->data, p_app->country,3);
                    ie_len = 5;

                    cstr = (struct wlan_country_str *)(pos+ie_len);
                    num = 0;
                    prev = NULL;
                    for (i = 0, curr = &channels[i]; i < p_app->channel_num; curr = &channels[++i])
                    {
                        if (!prev)
                        {
                            cstr->first = i+1;
                        }
                        else if (prev && ((curr->flag & WLAN_CHAN_DISABLED) ||
                                          (curr->max_txpower != prev->max_txpower)))
                        {
                            cstr->num = num;
                            cstr->max_txpower = prev->max_txpower;
                            ie_len += 3;

                            /* next country string */
                            cstr = (struct wlan_country_str *)(pos+ie_len);
                            cstr->first = i+1;
                            num = 0;
                        }
                        prev = curr;
                        num++;
                    }

                    if (prev)
                    {
                        cstr->num = num;
                        cstr->max_txpower = prev->max_txpower;
                        ie_len += 3;
                    }

                    break;
                }
            case IE_ERP_INFO_BIT:
                {
                    u8 erp = 0;
                    if ((bss->phy_cap & (AP_CAP_11N|AP_CAP_11G)) == 0)
                        break;

                    ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_ERP_INFO;

                    switch (p_app->cts_protection_type)
                    {
                        case CTS_PROTECTION_FORCE_ENABLED:
                            erp |= WLAN_ERP_NON_ERP_PRESENT | WLAN_ERP_USE_PROTECTION;
                            break;
                        case CTS_PROTECTION_FORCE_DISABLED:
                            erp = 0;
                            break;
                        case CTS_PROTECTION_AUTOMATIC:
                            /* FIXME: obss_non_erp? & num_sta_non_erp? */
#if 0
                            if (my_wlan_dev->obss_non_erp[0])
                                erp |= WLAN_ERP_USE_PROTECTION;
                            if (my_wlan_dev->num_sta_non_erp[0] > 0)
                            {
                                erp |= WLAN_ERP_NON_ERP_PRESENT |
                                       WLAN_ERP_USE_PROTECTION;
                            }
#endif
                            break;
                        default :
                            break;
                    }

                    /* FIXME: num_sta_no_short_preamble? */
#if 0
                    if (my_wlan_dev->num_sta_no_short_preamble[0] > 0)
                        erp |= WLAN_ERP_LONG_PREAMBLE;
#endif
                    ((struct wlan_ie_generic *)pos)->data[0] = erp;
                    ie_len = 3;

                    break;
                }
            case IE_WPA_BIT:
                {
                    if (!(bss->auth_capability & AUTH_CAP_WPA))
                        break;

                    if (bss->wpa_ie)
                    {
                        ie_len = ((struct wlan_ie_generic *)bss->wpa_ie)->len + 2;
                        memcpy(pos, bss->wpa_ie, ie_len);
                    }
                    else
                    {
                        ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_VENDOR_SPEC;
                        ie_len = ap_encode_wpa_ie(bss, pos + 2, 0);   /* 2 = ie id & len */
                        ie_len += 2;

//                      if ((bss->wpa_ie = (u8 *)malloc(ie_len)) != NULL)
                        if ((bss->wpa_ie = (u8 *)m_wpa_ie) != NULL)
                        {
                            ((struct wlan_ie_generic *)pos)->len = ie_len - 2;
                            memcpy(bss->wpa_ie, pos, ie_len);
                        }
                    }
                    break;
                }
            case IE_RSN_BIT:
                {
                    if (!(bss->auth_capability & AUTH_CAP_WPA2))
                        break;

                    if (bss->rsn_ie)
                    {
                        ie_len = ((struct wlan_ie_generic *)bss->rsn_ie)->len + 2;
                        memcpy(pos, bss->rsn_ie, ie_len);
                    }
                    else
                    {
                        ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_RSN;
                        ie_len = ap_encode_wpa_ie(bss, pos + 2, 1);   /* 2 = ie id & len */
                        ie_len += 2;

//                      if ((bss->rsn_ie = (u8 *)malloc(ie_len)) != NULL)
                        if ((bss->rsn_ie = (u8 *)m_rsn_ie) != NULL)
                        {
                            ((struct wlan_ie_generic *)pos)->len = ie_len - 2;
                            memcpy(bss->rsn_ie, pos, ie_len);
                        }
                    }
                    break;
                }
            case IE_HT_CAP_BIT:
                {
                    struct wlan_ie_ht_capability *cap = (struct wlan_ie_ht_capability *)pos;

                    if (!(bss->phy_cap & AP_CAP_11N))    /* FIXME: p_app->phy_cap? */
                        break;

                    cap->id = WLAN_ELEMID_HT_CAP;
                    cap->ht_cap.capabilities_info = htows(bss->ht_capability);
                    cap->ht_cap.ampdu_params = p_app->ampdu_params;
                    cap->ht_cap.supported_mcs_set[0] = 0xff & (p_app->current_rx_rates >> 12);
                    cap->ht_cap.supported_mcs_set[4] = 0x1; /* support MCS 32 */

                    ie_len = sizeof(*cap);
                    break;
                }
            case IE_HT_INFO_BIT:
                {
                    struct wlan_ie_ht_info *info = (struct wlan_ie_ht_info *)pos;

                    if (!(bss->phy_cap & AP_CAP_11N))    /* FIXME: p_app->phy_cap? */
                        break;
                    info->id = WLAN_ELEMID_HT_INFO;

                    info->primary_channel = p_app->channel;
                    info->operation_mode = htows(p_app->ht_op_mode);
                    info->param = WLAN_HTINFO_RIFSMODE_PERM;
                    if (p_app->active_bandwidth)
                        info->param |= (p_app->active_bandwidth|WLAN_HTINFO_TXWIDTH);
                    ie_len = sizeof(*info);

                    break;
                }
            case IE_WMM_BIT:
                {
                    struct wlan_ie_wme_param  *wme = (struct wlan_ie_wme_param *)pos;
                    struct wme_acparams *ac;
                    struct wlan_wme_ac_params *wme_ac_params=NULL;
                    struct wlan_wme_ac_params *acp;
                    u32 i;

                    if (!(bss->flag & BSS_FLG_WMM))
                        break;
                    /* for station mode*/
                    if ((bss->role == WIF_STA_ROLE) || (bss->role == WIF_IBSS_ROLE))
                    {
                        /* max sp length = 0 */
                        if (p_app->modem_ps_policy & PS_POLICY_UAPSD)
                            wme->qosinfo = p_app->modem_ps_policy & PS_POLICY_UAPSD;
                        /* more data ack = 1 */
                        wme->qosinfo |= 0x80;
                        wme->subtype = WME_INFO_OUI_SUBTYPE;
                        ie_len  = sizeof(struct wlan_ie_wme_info);
                    }
                    else
                    {
                        wme_ac_params = &p_app->peer_ac_parms[0];
                        wme->subtype = WME_PARAM_OUI_SUBTYPE;
                        wme->qosinfo = (p_app->parameter_set_count & 0xf) | WME_QOSINFO_APSD;

                        /* fill in a parameter set record for each AC */
                        for (i = 0; i < 4; i++)
                        {
                            ac = &wme->ac_params[i];
                            acp = &wme_ac_params[i];

                            ac->aifsn = acp->aifs;
                            ac->acm = acp->acm;
                            ac->aci = i;
                            ac->reserved = 0;
                            ac->ecwmin = cw2ecw(acp->cwmin);
                            ac->ecwmax = cw2ecw(acp->cwmax);
                            ac->txop = htows(acp->txoplimit);
                        }

                        ie_len  = sizeof(struct wlan_ie_wme_param);
                    }

                    wme->id = WLAN_ELEMID_VENDOR_SPEC;
                    wme->oui[0] = 0x00;
                    wme->oui[1] = 0x50;
                    wme->oui[2] = 0xf2;
                    wme->type = WME_OUI_TYPE;
                    wme->version = WME_VERSION;

                    break;
                }
            case IE_EXT_CAP_BIT:
                {
                    ((struct wlan_ie_generic *)pos)->id = WLAN_ELEMID_EXT_CAP;
                    /* 20/40 BSS coexistence management support*/
                    if ((p_app->bandwidth != BW40MHZ_SCN))
                        ((struct wlan_ie_generic *)pos)->data[0] |= 0x1;
                    ie_len = 3;
                    break;
                }
            default:
                break;
        }

        if (ie_len)
        {
            if (host_multies_flag != 1)
                ((struct wlan_ie_generic *)pos)->len = ie_len - 2;
            host_multies_flag = 0;

            pos += ie_len;
            total_len += ie_len;
        }
        mask <<= 1;
    }

    return total_len;
}

extern int TX_packet(u8 *buf, u32 payload_length, u32 subtype);
void send_mgt_frame(struct wm_bss *bss, u8 *data, int len, u32 w0, u32 flags, u32 subtype)
{
    TX_packet(data, len, subtype);
}

void send_deauth(struct wm_bss *bss, struct sta_ctx *sta, u16 reason)
{
    u8 buf[sizeof(struct wlan_deauth_frame)];
    struct wlan_deauth_frame *resp = (struct wlan_deauth_frame *)buf;

    /* FIXME: we asume that sta_ctx must not been NULL */

    /* FIXME: may use disassoc for some case? ex: disconnect */

    memset((void *) buf, 0, sizeof(buf));
    build_mgt_frame(&resp->hdr, WLAN_FC_SUBTYPE_DEAUTH, bss, sta);
    resp->reasoncode = htows(reason);

    send_mgt_frame(bss, buf, sizeof(struct wlan_deauth_frame), 0, WBUF_MGMT_FRAME, WLAN_FC_SUBTYPE_DEAUTH);
}

/*=============================================================================+
| Main Handle Functions
+=============================================================================*/
void send_probe_resp(struct wm_bss *bss, char *addr2)
{
    u8 resp_buf[MAX_PROBERESP_LEN];
    struct wlan_probe_resp_frame *resp = (struct wlan_probe_resp_frame *)resp_buf;
    //u8 *bssid = NULL;
    u8 *pos;
    u32 encode_bits, len;

    /* TODO: verify that supp_rates contains at least one matching rate
     * with AP configuration */

    /* build response frame to transmit */
    memset((void *) resp_buf, 0, sizeof(resp_buf));
    //bssid = bss->bssid;
    build_mgt_frame(&resp->hdr, WLAN_FC_SUBTYPE_PROBE_RESP, bss, (struct sta_ctx *)addr2);

    /* hardware should fill in timestamp */
    resp->beacon_interval = cpu_to_le16(bss->beacon_interval);
    resp->capability = cpu_to_le16(ap_encode_capability(bss));
    pos = (unsigned char *)&resp[1];

    encode_bits = IE_SSID_BIT | IE_SUPP_RATES_BIT | IE_DS_PARMS_BIT |
                  IE_COUNTRY_BIT | IE_ERP_INFO_BIT | IE_EXT_SUPP_RATES_BIT | IE_RSN_BIT | IE_WMM_BIT |
                  IE_HT_CAP_BIT | IE_HT_INFO_BIT | IE_WPA_BIT|IE_WAPI_BIT | IE_WPS_BIT;


    if ((bss->auth_capability & AUTH_CAP_WEP)
        || (!(bss->auth_capability & AUTH_CAP_CIPHER_CCMP)
            && (bss->auth_capability & AUTH_CAP_CIPHER_TKIP)))
    {
        encode_bits &= ~(IE_HT_CAP_BIT|IE_HT_INFO_BIT);
    }

    len = wlan_ap_encode_ie_func(bss, pos, encode_bits);

    send_mgt_frame(bss, resp_buf, len + sizeof(struct wlan_probe_resp_frame),
                    WTB_W0_TS, WBUF_MGMT_FRAME, WLAN_FC_SUBTYPE_PROBE_RESP);
}

int ap_handle_probe_req(u8 *dptr, u32 len)
{
    int ret = WLAN_RET_OK;
    u32 left = 0;
    struct wlan_probe_req_frame *fm = (struct wlan_probe_req_frame *)dptr;
    struct wm_bss *bss;
    u8 *pos;
    //u8 *ssid = NULL;
    //u8 ssid_len = 0;
    struct wlan_ie ie;

    pos = (unsigned char *)&fm[1];
    left = len - sizeof(struct wlan_hdr);

    if (wlan_parse_ie(pos, left, &ie, NULL) < 0)
        return ret;

    if (ie.supp_rates == NULL)
    {
        return ret;
    }
    if (ie.ssid == NULL)
    {
        return ret;
    }
    else
    {
        //ssid = ie.ssid;
        //ssid_len = WLAN_IE_CONTEXT_TO_LEN(ssid);
    }

#if 1
    bss = &bss_ctxs[0];
    send_probe_resp(bss, (char *) fm->hdr.addr2);
#else
    for (i=0; i < MBSS_MAX_NUM; i++)
    {
        bss = IDX_TO_BSS(i);

        if (!(bss->flag & BSS_FLG_ENABLE) ||
            (bss->flag & BSS_FLG_NO_PROBE_RESP))
            continue;
        if ((bss->role < WIF_AP_ROLE) ||
            (bss->role == WIF_P2P_CLIENT_ROLE) ||
            (bss->role == WIF_WDS_ROLE))
            continue;

        /* verify broadcast ssid or my ssid */
        if (ssid_len == 0)
        {
            if (bss->flag & BSS_FLG_HIDDEN_SSID)
                continue;
        }
        else
        {
            if ((ssid_len != bss->ssid_len) || memcmp(ssid, bss->ssid, bss->ssid_len))
                continue;
        }

        send_probe_resp(bss, fm->hdr.addr2);
    }
#endif

    return ret;
}

// only handle without encryption
void mlme_handle_auth(u8 *dptr, u32 len)
{
    u8 req_buf[512];
    struct wlan_auth_frame *fm = (struct wlan_auth_frame *)dptr;
    struct wlan_hdr *hdr = (struct wlan_hdr *)req_buf;  /* for response */
    /* for ap role or WLAN_AUTH_ALG_SHARED response	*/
    struct wlan_auth_frame *resp = (struct wlan_auth_frame *)req_buf;
    struct wm_bss *bss = &bss_ctxs[0];
    struct sta_ctx *sta = &m_sta;
    int req_len = sizeof(struct wlan_auth_frame);
    u16 alg = 0, transaction = 0;
    u16 ret = WLAN_STATUS_SUCCESS;

    memset((void *) req_buf, 0, sizeof(req_buf));

    alg = le16_to_cpu(fm->alg);
    transaction = le16_to_cpu(fm->transaction);

    if (bss->role == WIF_AP_ROLE)
    {
        if (!((alg == WLAN_AUTH_ALG_OPEN) && (bss->auth_capability & AUTH_CAP_OPEN)))
        {
            printf("Unsupported authentication algorithm (%d)\n", alg);
            ret = WLAN_STATUS_UNSUPPORTED_AUTH_ALG;
            goto fail;
        }

        if ((transaction & 0x1) != 1)
        {
            printf("Unknown authentication transaction number (%d)\n", transaction);
            ret = WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION;
            goto fail;
        }
    }

    if (alg == WLAN_AUTH_ALG_OPEN)
    {
        sta->flags |= WLAN_STA_AUTH;
		sta->flags &= ~WLAN_STA_AUTH_SHARED;
        printf("authentication OK (open system)\n");
        printf("alg = %d, bss->auth_capability = 0x%x\n", alg, bss->auth_capability);
        /* Wait for ack to transaction 2 frame */
    }

fail:
    memcpy(sta->addr, fm->hdr.addr2, WLAN_ADDR_LEN);
    
    /* sta role's assoc req frame is sent by sta_send_assoc() */
    build_mgt_frame(hdr, WLAN_FC_SUBTYPE_AUTH, bss, sta);

    if ((transaction != 2))
    {
        resp->alg = cpu_to_le16(alg);
        resp->transaction = cpu_to_le16(transaction + 1);
        resp->statuscode = cpu_to_le16(ret);
    }

    /* send frame */
    send_mgt_frame(bss, req_buf, req_len, 0, WBUF_MGMT_FRAME, WLAN_FC_SUBTYPE_AUTH);
}

static u32 cur_aid = 1;
// only handle without encryption
void ap_handle_assoc(u8 *dptr, u32 len)
{
    struct wlan_assoc_req_frame *fm = (struct wlan_assoc_req_frame *)dptr;
    struct wm_bss *mybss = &bss_ctxs[0];
    struct sta_ctx *sta = &m_sta;
    u16 capability, listen_interval;
    u16 ret = WLAN_STATUS_UNSPECIFIED_FAILURE;
    u8 *pos;
    int deauth = 0, resp_len, left;
    u32 encode_bits;
    u8 buf[512];
    struct wlan_assoc_resp_frame *resp = (struct wlan_assoc_resp_frame *)buf;
    int reass = !((ntohs(fm->hdr.fc) & WLAN_FC_STYPE) == WLAN_FC_SUBTYPE_ASSOC_REQ);
    struct wlan_ie ie;
    struct cap_info info;
    u8 *sta_static = &p_app->stat.sta_static_new;


    MIN_LENGTH_VERIFY(len, (reass ? sizeof(struct wlan_resassoc_frame):
                            sizeof(struct wlan_assoc_req_frame)), return);

    if (sta == NULL || ((sta->flags & WLAN_STA_AUTH) == 0))
    {
        deauth = 1;
        goto fail;
    }

    capability = le16_to_cpu(fm->capability);
    listen_interval = le16_to_cpu(fm->listen_interval);

    /* The listen_interval_max is adjusted by beacon interval to ensure STAs
        does not sleep over PSQ_AGING_TIME.*/
    if (listen_interval > p_app->listen_interval_max)
    {
        printf("Too large Listen Interval (%d)", listen_interval);
    }

    /* avoid retry auth */
    sta->rx_seq_num[8] = le16_to_cpu(fm->hdr.seq_frag) >> 4;

    sta->supported_rates = 0;
    sta->rate_flags = 0;

    sta->listen_interval = listen_interval;
    if (reass)
    {
        left = len - sizeof(struct wlan_resassoc_frame);
        pos = (unsigned char *)&((struct wlan_resassoc_frame *)fm)[1];
    }
    else
    {
        left = len - sizeof(struct wlan_assoc_req_frame);
        pos = (unsigned char *)&fm[1];
    }

    if ((wlan_parse_ie(pos, left, &ie, &info) < 0))
    {
        goto fail;
    }

    /* check ssid */
    if ((info.ssid_len != mybss->ssid_len) ||
        (memcmp(info.ssid, mybss->ssid, info.ssid_len) != 0))
        goto fail;

    sta->flags &= (WLAN_STA_VALID|WLAN_STA_AUTH|WLAN_STA_AUTH_SHARED|
                   WLAN_STA_LINKED_AP|WLAN_STA_WDS_LINKED_AP|WLAN_STA_IBSS_STA);
    sta->apsd_trigger_deliver = 0;
    sta->ps_policy = 0;

    /* support rates is MUST */
    if ((info.supp_rates == 0))
    {
        /* FIXME: info.supp_rates contains ext_supp_rates, is that ok? */
        WLAN_DBG("No supported rates element in AssocReq");
        goto fail;
    }

    if ((ie.ext_supp_rates != NULL) &&
        ((p_app->phy_cap & AP_CAP_11G) == 0))
    {
        goto fail;
    }

    /* collect sta's supported rates */
    sta->supported_rates |= (p_app->current_tx_rates & info.supp_rates);

    *sta_static |= STA_STATIC_NO_HT;

    if (sta->supported_rates >= R_BIT(OFDM_6M))
        sta->flags |= WLAN_STA_ERP;
    else
        *sta_static |= STA_STATIC_NO_ERP;

    if (capability & WLAN_CAPABILITY_SHORT_SLOT_TIME)
        sta->flags |= WLAN_STA_SHORT_SLOT;
    else
        *sta_static |= STA_STATIC_NO_SHORT_SLOTTIME;

    if (capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
    {
        sta->flags |= WLAN_STA_SHORT_PREAMBLE;
        sta->rate_flags |= RATE_FLAGS_SHORT_PREAMBLE;
    }
    else
    {
        *sta_static |= STA_STATIC_NO_SHORT_PREAMBLE;
    }

    if (reass)
    {
        memcpy(sta->previous_ap,
               ((struct wlan_resassoc_frame *)fm)->current_ap,
               WLAN_ADDR_LEN);
    }

    /* assoc success */
    ret = WLAN_STATUS_SUCCESS;

fail:
    if (deauth)
    {
        if (sta == NULL)
            sta = (struct sta_ctx *)fm->hdr.addr2;
        send_deauth(mybss, sta, ret);
    }
    else
    {
        /* build response frame to transmit */
        memset((void *) buf, 0, sizeof(buf));
        build_mgt_frame(&resp->hdr, WLAN_FC_SUBTYPE_ASSOC_RESP, mybss, sta);
        pos = (unsigned char *)&resp[1];
        resp_len = 0;
        encode_bits = (IE_SUPP_RATES_BIT);

        if (sta->flags & WLAN_STA_ERP)
            encode_bits |= (IE_EXT_SUPP_RATES_BIT);

        resp_len += wlan_ap_encode_ie_func(mybss, pos, encode_bits);

        if (reass)
        {
            resp->hdr.fc &= htons(~WLAN_FC_STYPE);
            resp->hdr.fc |= htons(WLAN_FC_SUBTYPE_REASSOC_RESP);
        }

        resp->capability = cpu_to_le16(ap_encode_capability(mybss));
        resp->statuscode = cpu_to_le16(ret);
        resp->aid = cpu_to_le16(cur_aid);

        cur_aid++;
        if (cur_aid > MAX_AID_SIZE)
        {
            cur_aid = 1;    // aid 0 is reserved to notify station
        }

        printf(">TX Assoc Resp, ret = %d, aid = %d\n", resp->statuscode, cur_aid);
        //hexdump("",resp, resp_len + sizeof(struct wlan_assoc_resp_frame));

        send_mgt_frame(mybss, buf, resp_len + sizeof(struct wlan_assoc_resp_frame), 0, WBUF_MGMT_FRAME, WLAN_FC_SUBTYPE_ASSOC_RESP);
    }
}

extern u32 server_ipaddr;
extern u32 *get_server_ipaddr(void);
int mlme_handle_arp_frame(u8 *pos, u8 *saddr)
{
    struct arprequest *arp_hdr = (struct arprequest *)pos;
    u16 opcode = arp_hdr->opcode;

    if (opcode != 1)
    {
        printf("We only handle ARP request frame, opcode = %d\n", opcode);
        return 1;
    }

//  unsigned char shwaddr[6];
//  unsigned char sipaddr[4];
//  unsigned char thwaddr[6];
//  unsigned char tipaddr[4];
    opcode = 2;
    arp_hdr->opcode = opcode;
    memcpy(arp_hdr->thwaddr, arp_hdr->shwaddr, 6);
    memcpy(arp_hdr->tipaddr, arp_hdr->sipaddr, 4);

    memcpy(arp_hdr->shwaddr, saddr, 6);
    memcpy(arp_hdr->sipaddr, (void *) get_server_ipaddr(), 4);

    return 0;
}

#if 0   // use to handle dhcp and arp request only
int mlme_handle_data_frame(u8 *dptr, u32 len)
{
    struct wlan_hdr *hdr = (struct wlan_hdr *)dptr;
	u8 *pos;
	u16	subtype = WLAN_FC_SUBTYPE_DATA | WLAN_FC_FROMDS;
	u8 *addr1;
    struct wm_bss *bss = &bss_ctxs[0];
    u32 ethertype;
    struct iphdr *ip;
    unsigned int src_addr, dest_addr;

    hdr->fc = WLAN_FC_TYPE_DATA | subtype;
    hdr->seq_frag = 0;
    pos = (u8 *)(hdr + 1);

    ethertype = (pos[6] << 8) | pos[7];
//  printf("ethertype = 0x%x\n", ethertype);

    pos += 8;   // seek position after protocol type
    switch (ethertype)
    {
        case IP:
            printf("case IP\n");
            dptr = (u8 *)ieee80211_data_to_8023(dptr, &len);

            ip = (struct iphdr *) &dptr[ETHER_HEAD_SZ];
            // handle packets about DHCP
            if (ip->protocol == IP_UDP)
            {
//              printf("ip->protocol == IP_UDP\n");
                src_addr = ntohl(ip->src.s_addr);
                dest_addr = ntohl(ip->dest.s_addr);

                // DHCP discovery packet
                if (!memcmp(&src_addr, zeros, 4)
                    && !memcmp(&dest_addr, broadcast, 4))
                {
                    udp_appcall(&dptr[ETHER_HEAD_SZ + IPHDR_SZ + UDPHDR_SZ],
                                      len - (ETHER_HEAD_SZ + IPHDR_SZ + UDPHDR_SZ));
                }
            }
            goto exit;
        case ARP:
            printf("case ARP\n");

            addr1 = hdr->addr2;
            memcpy(hdr->addr1, addr1, WLAN_ADDR_LEN);
            memcpy(hdr->addr2, bss->myaddr, WLAN_ADDR_LEN);
            memcpy(hdr->addr3, bss->bssid, WLAN_ADDR_LEN);

            if (mlme_handle_arp_frame(pos, bss->myaddr))
            {
                goto exit;
            }
            len -= 4;
            break;
        default:
            goto exit;
    }

    TX_packet(dptr, len, WLAN_FC_SUBTYPE_DATA);

exit:
      return 0;
}
#else
struct nbuf nbuf_pkt;
extern void net_poll(struct nbuf *nbuf);
int mlme_handle_data_frame(u8 *dptr, u32 len)
{
    struct nbuf *pkt = nbuf_get();

    if (pkt == NULL)
    {
        //printf("%s: get buf failed\n", __FUNCTION__);
        goto exit;
    }

    dptr = (u8 *)ieee80211_data_to_8023(dptr, &len);

    //printf("==> %x %x %d %x\n", pkt, (char *) pkt + NB_HEAD_SZ, len, dptr);
    if(((unsigned long) dptr & 0x3)==0x2)   /* need IP header alignment */
        pkt->data =  (unsigned char *) pkt + NB_HEAD_SZ + 2;
    else
        pkt->data =  (unsigned char *) pkt + NB_HEAD_SZ;
    if(len)
        memcpy(pkt->data, dptr, len);

    pkt->len = len;
    net_poll(pkt);

exit:
    return 0;
}

#define ETH_TO_WIFI_OFFSET 40
u8 tx_buf[2048 + ETH_TO_WIFI_OFFSET];
u8 *tx_data = &tx_buf[ETH_TO_WIFI_OFFSET];
void wifi_tx(const char *dest, unsigned int type, unsigned int size, const void *pkt)
{
    struct wlan_hdr *hdr;
    u8 *dptr;
    u32 len = size;
//  int i=0;

    if (!bootvars.mac0)
    {
        printf("No definition of bootvars.mac0\n");
        return;
    }

    // tansfrom data frame from type of ether to Wi-Fi
    memcpy(tx_data + ETH_HLEN, (void *) pkt, size);
    memcpy(tx_data, (void *) dest, ETHER_ADDR_LEN);
    memcpy(tx_data + ETHER_ADDR_LEN, bootvars.mac0, ETHER_ADDR_LEN);

    tx_data[12] = (type >> 8) & 0xff;
    tx_data[13] = (type & 0xff);
    len = size + ETH_HLEN;

    dptr = (u8 *)ieee80211_data_from_8023(tx_data, &len);
//  printf("len = %d\n", len);
//  for (i = 0; i < len; i ++) {
//      printf("%02x ", dptr[i]);
//  }
//  printf("\n\n");

    hdr = (struct wlan_hdr *) dptr;
    hdr->fc |= htons(WLAN_FC_MOREDATA);

    // transmit through wifi TX function
    if (dptr)
    {
        TX_packet(dptr, len, NULL);
    }
    else
    {
        printf("%s: Transform eth to wifi failed\n", __FUNCTION__);
    }
}
#endif

/**
 * ieee80211_get_SA - get pointer to SA
 * @hdr: the frame
 *
 * Given an 802.11 frame, this function returns the offset
 * to the source address (SA). It does not verify that the
 * header is long enough to contain the address, and the
 * header must be long enough to contain the frame control
 * field.
 */
static inline u8 *ieee80211_get_SA(struct wlan_hdr_addr4 *hdr)
{
//  if (ieee80211_has_a4(hdr->frame_control))
//  	return hdr->addr4;
	if (ieee80211_has_fromds(hdr->fc))
		return hdr->addr3;
	return hdr->addr2;
}

/**
 * ieee80211_get_DA - get pointer to DA
 * @hdr: the frame
 *
 * Given an 802.11 frame, this function returns the offset
 * to the destination address (DA). It does not verify that
 * the header is long enough to contain the address, and the
 * header must be long enough to contain the frame control
 * field.
 */
static inline u8 *ieee80211_get_DA(struct wlan_hdr_addr4 *hdr)
{
	if (ieee80211_has_tods(hdr->fc))
		return hdr->addr3;
	else
		return hdr->addr1;
}

//int ieee80211_data_to_8023(struct sk_buff *skb, const u8 *addr,
//               enum nl80211_iftype iftype)
u8 *ieee80211_data_to_8023(u8 *dptr, u32 *len)
{
    struct wlan_hdr_addr4 *hdr = (struct wlan_hdr_addr4 *)dptr;
    u16 hdrlen, ethertype;
    u8 *payload;
    u8 dst[ETH_ALEN];
    u8 src[ETH_ALEN] __attribute__ ((aligned (2)));

//  if (unlikely(!ieee80211_is_data_present(hdr->frame_control)))
//      return -1;

//  hdrlen = ieee80211_hdrlen(hdr->fc);
    hdrlen = 24;

    /* convert IEEE 802.11 header + possible LLC headers into Ethernet
     * header
     * IEEE 802.11 address fields:
     * ToDS FromDS Addr1 Addr2 Addr3 Addr4
     *   0     0   DA    SA    BSSID n/a
     *   0     1   DA    BSSID SA    n/a
     *   1     0   BSSID SA    DA    n/a
     *   1     1   RA    TA    DA    SA
     */
    memcpy(dst, ieee80211_get_DA(hdr), ETH_ALEN);
    memcpy(src, ieee80211_get_SA(hdr), ETH_ALEN);

//  switch (hdr->fc & cpu_to_le16(IEEE80211_FCTL_TODS | IEEE80211_FCTL_FROMDS))
//  {
//      case cpu_to_le16(IEEE80211_FCTL_TODS):
//          if (unlikely(iftype != NL80211_IFTYPE_AP &&
//                       iftype != NL80211_IFTYPE_AP_VLAN &&
//                       iftype != NL80211_IFTYPE_P2P_GO))
//              return -1;
//          break;
//      case cpu_to_le16(IEEE80211_FCTL_TODS | IEEE80211_FCTL_FROMDS):
//          if (unlikely(iftype != NL80211_IFTYPE_WDS &&
//                       iftype != NL80211_IFTYPE_MESH_POINT &&
//                       iftype != NL80211_IFTYPE_AP_VLAN &&
//                       iftype != NL80211_IFTYPE_STATION))
//              return -1;
//          if (iftype == NL80211_IFTYPE_MESH_POINT)
//          {
//              struct ieee80211s_hdr *meshdr =
//              (struct ieee80211s_hdr *) (skb->data + hdrlen);
//              /* make sure meshdr->flags is on the linear part */
//              if (!pskb_may_pull(skb, hdrlen + 1))
//                  return -1;
//              if (meshdr->flags & MESH_FLAGS_AE_A4)
//                  return -1;
//              if (meshdr->flags & MESH_FLAGS_AE_A5_A6)
//              {
//                  skb_copy_bits(skb, hdrlen +
//                                offsetof(struct ieee80211s_hdr, eaddr1),
//                                dst, ETH_ALEN);
//                  skb_copy_bits(skb, hdrlen +
//                                offsetof(struct ieee80211s_hdr, eaddr2),
//                                src, ETH_ALEN);
//              }
//              hdrlen += ieee80211_get_mesh_hdrlen(meshdr);
//          }
//          break;
//      case cpu_to_le16(IEEE80211_FCTL_FROMDS):
//          if ((iftype != NL80211_IFTYPE_STATION &&
//               iftype != NL80211_IFTYPE_P2P_CLIENT &&
//               iftype != NL80211_IFTYPE_MESH_POINT) ||
//              (is_multicast_ether_addr(dst) &&
//               ether_addr_equal(src, addr)))
//              return -1;
//          if (iftype == NL80211_IFTYPE_MESH_POINT)
//          {
//              struct ieee80211s_hdr *meshdr =
//              (struct ieee80211s_hdr *) (skb->data + hdrlen);
//              /* make sure meshdr->flags is on the linear part */
//              if (!pskb_may_pull(skb, hdrlen + 1))
//                  return -1;
//              if (meshdr->flags & MESH_FLAGS_AE_A5_A6)
//                  return -1;
//              if (meshdr->flags & MESH_FLAGS_AE_A4)
//                  skb_copy_bits(skb, hdrlen +
//                                offsetof(struct ieee80211s_hdr, eaddr1),
//                                src, ETH_ALEN);
//              hdrlen += ieee80211_get_mesh_hdrlen(meshdr);
//          }
//          break;
//      case cpu_to_le16(0):
//          if (iftype != NL80211_IFTYPE_ADHOC &&
//              iftype != NL80211_IFTYPE_STATION &&
//              iftype != NL80211_IFTYPE_OCB)
//              return -1;
//          break;
//  }

//  if (!pskb_may_pull(skb, hdrlen + 8))
//      return -1;

    payload = dptr + hdrlen;
    ethertype = (payload[6] << 8) | payload[7];

//  if ((ether_addr_equal(payload, rfc1042_header)
//       && ethertype != ETH_P_AARP && ethertype != ETH_P_IPX)
//       || ether_addr_equal(payload, bridge_tunnel_header))
    if ((ether_addr_equal(payload, rfc1042_header)
         && ethertype != ETH_P_AARP && ethertype != ETH_P_IPX))
    {
        /* remove RFC1042 or Bridge-Tunnel encapsulation and
         * replace EtherType */
        dptr = buf_pull(dptr, len, hdrlen + 6);
        dptr = buf_push(dptr, len, ETH_ALEN);
        memcpy(dptr, src, ETH_ALEN);
        dptr = buf_push(dptr, len, ETH_ALEN);
        memcpy(dptr, dst, ETH_ALEN);
    }
    else
    {
        struct ethhdr *ehdr;
        u16 eth_len;

        buf_pull(dptr, len, hdrlen);
        eth_len = htons(*len);
        ehdr = (struct ethhdr *)buf_push(dptr, len, sizeof(struct ethhdr));
        memcpy(ehdr->h_dest, dst, ETH_ALEN);
        memcpy(ehdr->h_source, src, ETH_ALEN);
        ehdr->h_proto = eth_len;
    }

    return dptr;
}

//int ieee80211_data_from_8023(struct sk_buff *skb, const u8 *addr,
//                 enum nl80211_iftype iftype,
//                 const u8 *bssid, bool qos)
u8 *ieee80211_data_from_8023(u8 *dptr, u32 *len)
{
    struct wlan_hdr_addr4 hdr;
    u16 hdrlen, ethertype;
    u16 fc;
    const u8 *encaps_data;
    int encaps_len, skip_header_bytes;
//  int nh_pos, h_pos;
    //int head_need;
    struct wm_bss *bss = &bss_ctxs[0];
    //u32 k;

    if (*len < ETH_HLEN)
    {
        return 0;
    }

//  nh_pos = skb_network_header(skb) - skb->data;
//  h_pos = skb_transport_header(skb) - skb->data;

    /* convert Ethernet header to proper 802.11 header (based on
     * operation mode) */
    ethertype = (dptr[12] << 8) | dptr[13];
    fc = cpu_to_le16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA);

#if 0
    switch (iftype)
    {
        case NL80211_IFTYPE_AP:
        case NL80211_IFTYPE_AP_VLAN:
        case NL80211_IFTYPE_P2P_GO:
            fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS);
            /* DA BSSID SA */
            memcpy(hdr.addr1, skb->data, ETH_ALEN);
            memcpy(hdr.addr2, addr, ETH_ALEN);
            memcpy(hdr.addr3, skb->data + ETH_ALEN, ETH_ALEN);
            hdrlen = 24;
            break;
        case NL80211_IFTYPE_STATION:
        case NL80211_IFTYPE_P2P_CLIENT:
            fc |= cpu_to_le16(IEEE80211_FCTL_TODS);
            /* BSSID SA DA */
            memcpy(hdr.addr1, bssid, ETH_ALEN);
            memcpy(hdr.addr2, skb->data + ETH_ALEN, ETH_ALEN);
            memcpy(hdr.addr3, skb->data, ETH_ALEN);
            hdrlen = 24;
            break;
        case NL80211_IFTYPE_OCB:
        case NL80211_IFTYPE_ADHOC:
            /* DA SA BSSID */
            memcpy(hdr.addr1, skb->data, ETH_ALEN);
            memcpy(hdr.addr2, skb->data + ETH_ALEN, ETH_ALEN);
            memcpy(hdr.addr3, bssid, ETH_ALEN);
            hdrlen = 24;
            break;
        default:
            return -EOPNOTSUPP;
    }
#else
    // case NL80211_IFTYPE_AP
    fc |= cpu_to_le16(IEEE80211_FCTL_FROMDS);

    /* DA BSSID SA */
    memcpy(hdr.addr1, dptr, ETH_ALEN);
    memcpy(hdr.addr2, bss->myaddr, ETH_ALEN);
    memcpy(hdr.addr3, dptr + ETH_ALEN, ETH_ALEN);
    hdrlen = 24;
#endif

//  if (qos) {
//      fc |= cpu_to_le16(IEEE80211_STYPE_QOS_DATA);
//      hdrlen += 2;
//  }

    hdr.fc = fc;
    hdr.dur = 0;
    hdr.seq_frag = 0;

    skip_header_bytes = ETH_HLEN;
    if (ethertype == ETH_P_AARP || ethertype == ETH_P_IPX)
    {
        encaps_data = bridge_tunnel_header;
        encaps_len = sizeof(bridge_tunnel_header);
        skip_header_bytes -= 2;
    }
    else if (ethertype >= ETH_P_802_3_MIN)
    {
        encaps_data = rfc1042_header;
        encaps_len = sizeof(rfc1042_header);
        skip_header_bytes -= 2;
    }
    else
    {
        encaps_data = NULL;
        encaps_len = 0;
    }

    dptr = buf_pull(dptr, len, skip_header_bytes);

//  nh_pos -= skip_header_bytes;
//  h_pos -= skip_header_bytes;

//  head_need = hdrlen + encaps_len - skb_headroom(skb);
//
//  if (head_need > 0 || skb_cloned(skb)) {
//      head_need = max(head_need, 0);
//      if (head_need)
//          skb_orphan(skb);
//
//      if (pskb_expand_head(skb, head_need, 0, GFP_ATOMIC))
//          return -ENOMEM;
//
//      skb->truesize += head_need;
//  }

    if (encaps_data)
    {
        dptr = buf_push(dptr, len, encaps_len);
        memcpy(dptr, (void *) encaps_data, encaps_len);
//      nh_pos += encaps_len;
//      h_pos += encaps_len;
    }

    dptr = buf_push(dptr, len, hdrlen);
    memcpy(dptr, &hdr, hdrlen);

//  nh_pos += hdrlen;
//  h_pos += hdrlen;
//
//  /* Update skb pointers to various headers since this modified frame
//   * is going to go through Linux networking code that may potentially
//   * need things like pointer to IP header. */
//  skb_set_mac_header(skb, 0);
//  skb_set_network_header(skb, nh_pos);
//  skb_set_transport_header(skb, h_pos);

    return dptr;
}
