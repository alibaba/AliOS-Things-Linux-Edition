#ifndef __PANTHER_APP_H__
#define __PANTHER_APP_H__

#include <wla_def.h>
#include <wla_cfg.h>
#include <mac_common.h>

#define DEV_DRV_LWIP			0x00000001
#define DEV_TXBA_SIZE			0x00000002
#define DEV_HT40				0x00000004
#define DEV_RC_VO				0x00000008
#define DEV_RC_LGI				0x00000010
#define DEV_RC_LPBL				0x00000020
#define DEV_RECONNECT			0x00000040
#define DEV_RECOVER				0x00000080
#define DEV_MAC_WD_MSG			0x00000100
#define DEV_RX_DROP				0x00000400
#define DEV_TX_DROP				0x00000800
#define DEV_RXBA_DROP			0x00001000
#define DEV_TXBA_DROP			0x00002000
#define DEV_TXOPLIMIT_AP		0x00004000
#define DEV_KEEPALIVE			0x00008000
#define DEV_PS_BY_RSSI			0x00010000
#define DEV_PS_BY_RC			0x00020000
#define DEV_DYNAMIC_VGA			0x00040000

struct panther_app {
	struct statistic {
		u8 wrx_buf_full;
		u8 wrx_desc_full;
		u8 wrx_fifo_full;
		u8 wrxq_drop_count;
		u32 wrxq_count;

		u32 wretq_count;
		u32 wtxq_count;
		u8 wtxq_drop_count;
		u8 wtxq_full_count;
		u8 ts_err;
		u8 res1;

		u16 wtxq_fail_psba_mpdu;
		u16 wtxq_fail_edca_mpdu;
		u16 wtxq_fail_psba_ampdu;
		u8 res2[2];

		u8 host_detach;
		u8 host_deauth;
		u8 ap_deauth;
		u8 sta_deauth;

		u32 pre_tbtt_int;
		u32 beacon_tx_miss;
		u32 ts0_int;
		u32 tsx_int;

		/* for checking the environment & device*/
		u8	sta_static_new;
		u8	sta_static_old;
		u8	obss_static_new;
		u8	obss_static_old;

		u8 wrx_recovery_cnt;
		u8 wtx_recovery_cnt;
		u8 wrx_wd_cnt;
		u8 wtx_wd_cnt;

		u16 beacon_sw_rx_cnt;
		u16 beacon_hw_rx_cnt;
		u16 beacon_hw_crc_cnt;
		u16 beacon_hw_lost_cnt;
	} stat;

	struct wrx_extbuf {
		u32 tbl;
		u32 tbl_end;
		u32 node;
		u32 free;
		u8 waround;
		u8 res[3];
	} extbuf;

	struct wrx_frag {
		struct wbuf *head;
		struct wbuf *tail;
		u32 len;

		u32 init_time;
		u32 seq;
		u32 tid;
		u32 last_frag;
		u32 total_len;
	} frag;

	/* callback function */
	int (*wla_drv_rx_done)(void);
	void (*wla_scan_done)(void);
	void (*wla_link_up)(void *, void *);
	void (*wla_link_down)(void *);
	void (*wla_link_err)(int, int);
	void (*wla_link_to)(void);
	void (*notify_wifi_scan_completed)(void *);
	void (*notify_wifi_status_changed)(int);
	void (*notify_wifi_para_changed)(void *, char *, int);
	void (*notify_wifi_connect_failed)(int);
	void (*monitor_cb)(u8*, int);
	int (*notify_wifi_led_timeout)(void);

	/* beacon ts */
	u32 ts_l_us[MAC_MAX_BSSIDS];
	u32 ts_h_us[MAC_MAX_BSSIDS];
	u32 ntbtt_l_us[MAC_MAX_BSSIDS];
	u32 ntbtt_h_us[MAC_MAX_BSSIDS];
	u32 dtim[MAC_MAX_BSSIDS];

	u32 beacon_timestamp;
	/* Configuration from host */
	u8 phy_cap;
	/* FIXME: ldev->slottime is the final result of all bss and set to HW */
	u8 slottime;
	u16 capability;

	/* channel */
	u8 channel;
	u8 bandwidth;
	u8 active_channel;
	u8 active_bandwidth;

	u8 channel_num;
	u8 cts_protection_type;
	u8 ibss_exist;
	u8 ap_exist;

	/* ampdu */
	u8 tx_ba_to;
	u8 ampdu_params;
	u8 rx_ba_win_size;
	u8 tx_ba_win_size;
	u8 rc_rts;
	u8 rc_rtsr;
	u16 rts_threshold;

	/* wmm */
	u32 txop;

	u8 modem_ps;
	u8 modem_ps_policy;
	u8 modem_ps_next_tbtt;
	u8 modem_ps_pending;

	u8 modem_ps_lock;
	u8 modem_ps_bc_pending;
	u8 wmac_is_poweroff;
	u8 light_ps;

	u8  min_mpdu_space;
	u8  min_rate_idx;
	u16 listen_interval_max; /* listen interval maximum value */

	struct wlan_wme_ac_params my_ac_parms[4];
	struct wlan_wme_ac_params peer_ac_parms[4];
	u32 wrx_mon_bmap;
	u32 wrx_mon_bmap_old;
	u32 current_rx_rates;
	u32 current_tx_rates;
	u32 basic_rates;
	u8 country[4];			/* country string */

	/* HT capabilities */
	u16 ht_op_mode;
	/* WMM */
	__s8 parameter_set_count;
	/* FIXME: need to modify the scan progress */
	u8 scan_state;
	u32 scan_channel_map;
	u32 user_channel_map;	/* 6800K */
	u8 res;
	u8 dtim_wakeup;
	u16 scan_interval;	/* schedule scan, unit: sec */
	u16 remain_on_channel_time;	/* unit: msec */
	u16 next_remain_on_ch;	/* 0: disable, 1~13 : channel number */
	struct wm_bss *scan_bss;
	/* FIXME: should support acl in firmware ? */
	struct wacl_rule *ap_acl;

	enum wlan_acl_result {
		WLAN_ACL_PASS = 0,
		WLAN_ACL_REJECT,
	} acl_default_policy;

	u8 bss_total_num;
	u8 sta_total_num;
	u8 nbss_total_num;
	u8 bw40mhz_intolerant;

	void *bss_ctxs;
	void *sta_ctxs;
	void *nbss_list;
	void *aplist;
	void *rx_ba_tbls;

#if (CONFIG_ROM_VER > 1)
	u8 sta_tbl_count;
	u8 ds_tbl_count;
	u8 wrxq_max;
	u8 wrxq_idx;
	u8 wtxq_max;
	u8 wmac_is_ready;
	u8 beacon_rate_code;
	u8 beacon_change;

	u32 int_mask;
	u32 ts_status;
	u32 beacon_tx_bitmap;

	/*	Do not move order of below member.
		We need to configure register by this order */
	void *ext_sta_tbls;
	void *ext_ds_tbls;
	void *group_keys;
	void *private_keys;

	void *wl_rxq;
	void *wl_txq;
	void *sw_buf_trace;
	void *sw_buf_trace_ra;
	void *extra_bufs;
#endif
	/* debug control */
	char wrx_dump;
	char wtx_dump;
	u8 rssi_max;
	u8 rssi_min;

	u8 resend_mode;
	u8 resend_max_cnt;
	u8 resend_min_rssi;
	u8 mac_wd_to;

	u32 op_flag;
	u8 fast_rc_cnt;

	/* iot */
	u8 op_bss_bmap;
	u16 op_mon_bmap;
	unsigned int ps_cnt[8];
	unsigned int tsfidx[MAC_MAX_BSSIDS];

	unsigned int buflen;
	char buf[255];
	u8 otp_fofs;
	u8 otp_bb_txpwr[16];

	/* monitor RX */
	unsigned int old_wrx_cnt;
	unsigned int old_wrx_drop_cnt;

	/* monitor TX */
	unsigned int old_wtxq_count;
	unsigned int old_wretq_count;

	/* ampdu */
    u16 ampdu_tx_mask;
};

extern struct panther_app *lapp;
extern struct wlan_wme_ac_params sta_wme_ac_parms[4];
extern struct wlan_wme_ac_params ap_non_wme_ac_parms[4];
extern struct wlan_wme_ac_params ap_wme_ac_parms[4];

#endif
