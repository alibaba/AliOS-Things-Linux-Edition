#ifndef __WTEST_H__
#define __WTEST_H__

#define TEST_DUMP_SIMPLE    0x1
#define TEST_DUMP_PAYLOAD   0x2

#define TEST_FILTER_MT_ACT  0x1
#define TEST_FILTER_BEACON  0x2
#define TEST_FILTER_MGT     0x4
#define TEST_FILTER_CTRL    0x8
#define TEST_FILTER_DATA    0x10
#define TEST_FILTER_DATA_BC 0x20

typedef struct {
	volatile u32 start;
	volatile u32 indicate_stop;
	s32 tx_repeat;
	u32 tx_burst_num;
	u32 tx_sleep_time;
	u32 cmd_sleep_time;
	u32 tx_len;

	u16 rate_code;
	u8 tx_ba_win_size;
	u8 tx_ba_max_len;

	u8 *tx_addr;
	u8 *my_addr;

	u8 key[32];
	u8 gkey[32];

	u8 tx_short_preamble;
	u8 tx_green_field;
	u8 tx_sgi;
	u8 tx_frag;

	u8 tx_rts;
	u8 res;
	u16 ampdu_tx_mask;

	u8 rx_drop;
	u8 rx_dump;
	u32 iteration;

	u32 rx_filter;
	u32 rx_sleep_time;

	u8 cipher_type;
	u8 channel;
	u8 secondary_channel;
	u8 bss_desc;

	u8 tx_qos;
	u8 tx_whdr;
	u8 tx_amsdu;
	u8 tx_rate;

	u8 tx_power;
	u8 rx_echo;
	u8 tx_echo;
	u8 role;

	u8 wds;
	u8 tx_pkt;
	u8 bcap;
	u8 tx_noack;

	u8 rx_ampdu;
	u8 rx_ampdu_tid;
	u8 tx_ampdu;
	u8 tx_ampdu_tid;

	u32 tx_count;
	u32 rx_count;
	u32 expect_count;
	u32 fail_count;
	u32 wb1_field;
	u32 wb2_field;
	u32 wakeup_time;
	struct wm_bss *bss;
	struct sta_ctx *sta;
#if 0 // panther part
#if defined(CONFIG_FREERTOS)
	QueueHandle_t rxq;
#elif defined(CONFIG_OS)
	os_msgq *rxq;
#else
	struct pt pt;
#endif
#endif
	u32 alloc_count;
	u32 free_count;

	/* freq offset */
	u8 en_freq_offset;
	s8 self_cfo;
	u16 freq_offset_count;
	s32 freq_offset_sum;
	s32 rssi_sum;

	s32 rssi_offset;
	s8  rssi_lna_tbl[4];
} wla_test_cfg;;

extern wla_test_cfg *acfg_p;

#endif
