/*=============================================================================+
|                                                                              |
| Copyright 2015                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file wtest.c
*   \brief
*   \author Montage
*/
//#include <stdio.h>
//#include <stdlib.h>
#include <lib.h>
#include <wla_def.h>
#include <mac_ctrl.h>
#include <mac_regs.h>
//#include <wbuf.h>
//#include <wla_api.h>
//#include <common.h>
#include <ip301.h>
//#include <sta.h>
#include <bb.h>
#include <netprot.h>
#include "performance.h"
#if 0 // lynx part
//#include <netprot.h>
//#include <event.h>
#if defined(CONFIG_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#elif defined(CONFIG_OS)
#include <os_api.h>
#else
#include <pt.h>
#endif
#endif
#include <panther_dev.h>
#include <panther_app.h>

#if defined(CONFIG_ATE_DUT)
#include <gpio.h>
#include <cfg_api.h>
#endif
#include <rfc_comm.h>
//#include <commands.h>
#include <cli_api.h>
#include <mac.h>
#include <wtest.h>
#include <otp.h>
#include <rf.h>
#include <panther_rf.h>
#include <panther_rfac.h>
#if defined(CONFIG_SCHED)
#include <sched.h>
#endif

#if !defined(ERR_OK)
#define ERR_OK 0
#endif

int cli_gets(char *buf, str_chain * p_cmd, char peek);
int cmd_proc(int argc, char **argv);
int get_args(const char *string, char *argv[]);
int printf(char *fmt, ...);
u16 tx_rate_encoding(int format, int ch_offset, int retry_count, int sgi, int rate);
extern int TX_packet_simply(int source, int target, int tid, int payload_length, u16 *rates);
extern void dump_rx_phy_info(void);
extern int rx_phy_info_record;
extern int rx_phy_info_storage_idx;
extern void enable_sniffer_mode(void);

u8 bc_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
u8 mc_addr[6] = {0x01, 0x00, 0x5e, 0x01, 0x02, 0x03};
u8 tx_addr[6] = {0x00, 0x32, 0x81, 0xf5, 0xad, 0x66};
u8 my_addr[6] = {0x00, 0x32, 0x81, 0xf5, 0xad, 0x99};

char wtest_key[32] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66,
						0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00,0x11,0x22,0x33,0x44,0x55,0x66};

char wtest_gkey[32] = {0x55,0x44,0x33,0x22,0x11,0x00,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00,
						0x55,0x44,0x33,0x22,0x11,0x00,0x99,0x88, 0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00};

u8 buf[1564];

static struct wlan_rate wla_rates[] = {
		{ .hw_value = CCK_1M_CODE, .flags = B_RATE },
		{ .hw_value = CCK_2M_CODE, .flags = B_RATE },
		{ .hw_value = CCK_5_5M_CODE, .flags = B_RATE },
		{ .hw_value = CCK_11M_CODE, .flags = B_RATE },
		/* OFDM */
		{ .hw_value = OFDM_6M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_9M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_12M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_18M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_24M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_36M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_48M_CODE, .flags = G_RATE },
		{ .hw_value = OFDM_54M_CODE, .flags = G_RATE },
		/* HT */
#if defined(CONFIG_RATEADAPTION2)
		{ .hw_value = MCS_0_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_1_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_2_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_3_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_4_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_5_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_6_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_7_CODE, .flags = HT_RATE },
#else
		{ .hw_value = MCS_0_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_1_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_2_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_3_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_4_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_5_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_6_CODE, .flags = HT_RATE },
		{ .hw_value = MCS_7_CODE, .flags = HT_RATE },
#endif
};

/* default value */
wla_test_cfg acfg = {
	.start = 0,
	.indicate_stop = 0,
	.tx_repeat = 0,
	.tx_burst_num = 10,
	.tx_sleep_time = 10,//10 = 10ms
	.cmd_sleep_time = 10,
	.tx_len = 256,
	.tx_ba_win_size = 16,
	.tx_ba_max_len = 0x3,
	.tx_addr = tx_addr,
	.my_addr = my_addr,

	.cipher_type = CIPHER_TYPE_NONE, //CIPHER_TYPE_SMS4,
	.tx_short_preamble = 0,
	.tx_green_field = 0,
	.tx_sgi = 0,
	.tx_frag = 0,
	.tx_rts = 0,
	.ampdu_tx_mask = 0,
	.role = WIF_STA_ROLE,
	.wds = 0,
	.rx_dump = 0,
	.rx_echo = 0,
	.rx_filter = 0,
	.rx_sleep_time = 100,

	.channel = 1,
	.secondary_channel = 0,
	.tx_qos = 1,
	.tx_whdr = 0,
	.tx_amsdu = 0,
	.tx_rate = 5,
	.tx_power = 0,
	.tx_pkt = 0,
	.tx_noack = 1,
	.bcap = 1,
	.rx_ampdu = 0,
	.tx_ampdu = 0,
	.tx_ampdu_tid = 0,
#if 0 // lynx part
#if defined(CONFIG_FREERTOS)||defined(CONFIG_OS)
	.rxq = 0,
#else
	.pt = {0},
#endif
#endif
};

wla_test_cfg *acfg_p = &acfg;
extern MAC_INFO *info;
#define txvga_def 50
extern unsigned char txvga_gain[14];
extern unsigned char txvga_gain_save[14];
extern unsigned char fofs;
extern unsigned char fofs_save;
extern unsigned char bg_txp_diff;
extern unsigned char ng_txp_diff;
extern unsigned char bg_txp_gap;
extern int fem_en;
extern int decimal_vga_converter(unsigned char c);
extern unsigned char string_vga_converter(char c);
extern unsigned char default_mac_addr[6];
#if defined(CONFIG_ATE_DUT)
#define def -10000
int rx_result_num = 0;
int rx_result_save = 0;
int rx_result[10][3] = {{def,def,def}, {def,def,def}, {def,def,def}, {def,def,def}, {def,def,def},\
						{def,def,def}, {def,def,def}, {def,def,def}, {def,def,def}, {def,def,def}};
void save_wt_rx_result(void);
#endif
extern void wrxq_process(void);
extern void wtxq_done(void);

void wt_ez(void);
void wt_rez(void);

//void freq_offset_record(struct wbuf *wb);
void freq_offset_calc(int option);

char *wt_txsubcmd[] =
{
    "stop",
    "fofs",
    "txvga"
};

char *wt_rxsubcmd[] =
{
    "stop",
    "bbcnt"
};

#define GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
#pragma GCC optimize ("O0")
int idle_cycle = 0, pattern = 0;
int do_ddr_access_pattern(void)
{
	volatile unsigned long addr1 = 0;
	int cycle;
	switch(pattern)
	{
		case 1:
			addr1 = *(volatile unsigned long *)(0xA0000000UL);
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 2:
			addr1 = *(volatile unsigned long *)(0xAFFFFFFCUL);
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 3:
			addr1 = *(volatile unsigned long *)(0xAAAAAAA8UL);
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 4:
			addr1 = *(volatile unsigned long *)(0xA0000000UL);
			addr1 = *(volatile unsigned long *)(0xAFFFFFFCUL);
			addr1 = *(volatile unsigned long *)(0xAAAAAAA8UL);
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 5:
			*(volatile unsigned long *)(0xA0000000UL) = 0;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 6:
			*(volatile unsigned long *)(0xA0000000UL) = 0xFFFFFFFFUL;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 7:
			*(volatile unsigned long *)(0xA0000000UL) = 0xAAAAAAAAUL;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 8:
			*(volatile unsigned long *)(0xA0000000UL) = 0;
			*(volatile unsigned long *)(0xA0000000UL) = 0xFFFFFFFFUL;
			*(volatile unsigned long *)(0xA0000000UL) = 0xAAAAAAAAUL;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 9:
			*(volatile unsigned long *)(0xAFFFFFFCUL) = 0;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 10:
			*(volatile unsigned long *)(0xAAAAAAA8UL) = 0;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 11:
			*(volatile unsigned long *)(0xAAAAAAA8UL) = 0x55555555UL;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		case 12:
			*(volatile unsigned long *)(0xA0000000UL) = 0x55555555UL;
			*(volatile unsigned long *)(0xAFFFFFFCUL) = 0x55555555UL;
			*(volatile unsigned long *)(0xAAAAAAA8UL) = 0x55555555UL;
			for(cycle=0;cycle<idle_cycle;cycle++)
				addr1=cycle;
			break;
		default:
			break;
	}
	return (int) addr1;
}
#endif
int check_pkt_err_rate(char *str)
{
	unsigned int fcs_pass, legacy_pass, mf_pass, gf_pass, b_pass, total;
	unsigned int *counter[5], i, pkt_err_rate, len;

	counter[0] = &fcs_pass;
	counter[1] = &legacy_pass;
	counter[2] = &mf_pass;
	counter[3] = &gf_pass;
	counter[4] = &b_pass;

	/* disable counter */
	bb_register_write(0, 0x80, 0x0);
	for(i = 0; i < (sizeof(counter)/sizeof(unsigned int *)); i++)
	{
		*(counter[i]) = (bb_register_read(3, (i << 1) + 6) << 8);
		*(counter[i]) |= bb_register_read(3, (i << 1) + 7);
		//serial_printf("counter[%d] = %5d\n", i, *(counter[i]));
	}

	// clear counter
	bb_register_write(0, 0x80, 0xc0);
	// enable counter
	bb_register_write(0, 0x80, 0x80);

	total = (legacy_pass + mf_pass + gf_pass + b_pass);
	//serial_printf("total pkt counter: %d\n",  legacy_pass + mf_pass + gf_pass + b_pass);
	if(total)
		pkt_err_rate = (1000 - ((fcs_pass * 1000)/total));
	else
		pkt_err_rate = 1000;
	if(pkt_err_rate > 1000)
		pkt_err_rate = 1000;

	len = sprintf(str, "%d.%1d %%\r\n", (int)(pkt_err_rate / 10), (int)(pkt_err_rate % 10));
	return len;
}
void wla_test_rx(void)
{
#if !defined(CONFIG_SCHED)
	char buffer[256 + 1];
	str_chain *phistory = 0x00;
	char *argv_t[10];
	int ret, argc_t;
#endif
	MACREG_WRITE32(MAC_INT_MASK_REG, ~(MAC_INT_TSF|MAC_INT_SW_RX));

	while(1)
	{
		if (0 == acfg_p->start)
		{
			break;
		}
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
		do_ddr_access_pattern();
#endif
#if !defined(CONFIG_SCHED)
		ret=cli_gets(buffer, phistory, 1);
		if(ret == ERR_OK)
		{
			if (1 <= (argc_t = get_args(&buffer[0], argv_t)))
			{
				if(!strcmp("wt", argv_t[0]))
				{
					int i;
					for(i=0; i < sizeof(wt_rxsubcmd)/sizeof(wt_rxsubcmd[0]); i++)
					{
						if(!strcmp(wt_rxsubcmd[i], argv_t[1]))
						{
							cmd_proc(argc_t, argv_t);
							break;
						}
					}
				}
			}
			if(acfg_p->start)
				printf("\nCLI>");
		}
#endif
	}

	// stop RX intr
	MACREG_WRITE32(MAC_INT_MASK_REG, ~MAC_INT_TSF);
}

/* channel offset */
#define CH_OFFSET_20		0
#define CH_OFFSET_40		1
#define CH_OFFSET_20U		2
#define CH_OFFSET_20L		3

unsigned char primary_ch_offset(u8 bw_type)
{
	if (bw_type == BW40MHZ_SCB)
		//return CH_OFFSET_20U;
		return CH_OFFSET_40;
	else if(bw_type == BW40MHZ_SCA)
		//return CH_OFFSET_20L;
		return CH_OFFSET_40;
	else
		return CH_OFFSET_20;
}

void wmac_set_channel(u8 ch, u8 bw)
{
	char pri_channel;
	int freq;

	rf_set_40mhz_channel(ch, bw);
	freq = lrf_ch2freq(ch);

	if (bw == BW40MHZ_SCN)
		bb_set_20mhz_mode(freq);
	else
		bb_set_40mhz_mode(bw, freq);

	pri_channel = primary_ch_offset(bw);
	MACREG_UPDATE32(BASIC_SET, pri_channel << 9, LMAC_CH_BW_CTRL_CH_OFFSET);
	MACREG_UPDATE32(BASIC_SET, pri_channel << 6, BASIC_CHANNEL_OFFSET);

	/* backward compatible for some FPGA-HW without second CCA circuit */
	if(bw == BW40MHZ_SCN)
		MACREG_UPDATE32(BASIC_SET,
				LMAC_DECIDE_CH_OFFSET|LMAC_DECIDE_CTRL_FR,
				LMAC_CH_BW_CTRL_AUTO_MASK);
	else
		/* David asked LMAC look both CCA0 and CCA1 to transmit, like other vendors. */
		MACREG_UPDATE32(BASIC_SET,
				LMAC_DECIDE_CH_OFFSET|LMAC_CCA1_EN|LMAC_DECIDE_CTRL_FR,
				LMAC_CH_BW_CTRL_AUTO_MASK);
	if(ch >= 1 && ch <=14)
		bb_set_tx_gain(txvga_gain[ch - 1]);
}

#define HT_RATE 0x4

#define RATE_HT 0x00000080UL

#define RATE_SHIFT_CH_OFFSET    14
#define RATE_SHIFT_TXCNT        11
#define RATE_SHIFT_FORMAT       8
#define RATE_SHIFT_TX_RATE      0

short setup_rate_encode(int tx_rate_idx, unsigned char retry, unsigned char flags)
{
	u16 val;

	if (wla_rates[tx_rate_idx - 1].flags & HT_RATE) {
		if(flags & RATE_FLAGS_HT_SGI40)
			val = tx_rate_encoding(FMT_HT,primary_ch_offset(lapp->bandwidth),
					       retry,1,wla_rates[tx_rate_idx - 1].hw_value);
		else
			val = tx_rate_encoding(FMT_HT,primary_ch_offset(lapp->bandwidth),
					       retry,0,wla_rates[tx_rate_idx - 1].hw_value);
	} else {
		if (tx_rate_idx <= CCK_11M) {
			if(flags & RATE_FLAGS_SHORT_PREAMBLE)
				val = tx_rate_encoding(FMT_11B,primary_ch_offset(lapp->bandwidth),
						       retry,0,wla_rates[tx_rate_idx - 1].hw_value + FMT_11B_SHORT_PREAMBLE);
			else
				val = tx_rate_encoding(FMT_11B,primary_ch_offset(lapp->bandwidth),
						       retry,0,wla_rates[tx_rate_idx - 1].hw_value);
		}
		else
		{
			/* non-HT only run in BW 20Mhz. Let LMAC auto determine 20U or 20L according
			LMAC_CH_BW_CTRL_CH_OFFSET of BASIC_SET. */
			val = tx_rate_encoding(FMT_NO_HT,primary_ch_offset(lapp->bandwidth),
					       retry,0,wla_rates[tx_rate_idx - 1].hw_value);
		}
	}

	return val;
}

#if defined(BB_NOTCH_FILTER_TEST)
int bb_notch_filter_test_first = 1;
int bb_notch_filter_test_enable = 0;
u8 bb_notch_filter_test_backup14, bb_notch_filter_test_backup15, bb_notch_filter_test_backup16, bb_notch_filter_test_backup17, bb_notch_filter_test_backup18;
#endif
#if defined(TX_RANDOM_PACKET_GENERATE)
extern void tx_random_packet_generate(void);
#endif
void wla_test(void)
{
	int tx_enable = 0;
#if !defined(CONFIG_SCHED)
	char buffer[256 + 1];
	int ret;
	str_chain *phistory = 0x00;
	char *argv_t[10];
	int argc_t;
#endif

	rf_update(8,  fofs << 19, 0x00f80000);

	wmac_set_channel(lapp->channel, lapp->bandwidth);

	if(fem_en)
		panther_fem_config(fem_en);

	panther_channel_config(lapp->channel, fem_en);

#if defined(BB_NOTCH_FILTER_TEST)
	if(bb_notch_filter_test_first)
	{
		bb_notch_filter_test_backup14 = bb_register_read(2, 0x14);
		bb_notch_filter_test_backup15 = bb_register_read(2, 0x15);
		bb_notch_filter_test_backup16 = bb_register_read(2, 0x16);
		bb_notch_filter_test_backup17 = bb_register_read(2, 0x17);
		bb_notch_filter_test_backup18 = bb_register_read(2, 0x18);

		bb_notch_filter_test_first = 0;
	}
	if(bb_notch_filter_test_enable)
	{
		if((lapp->channel == 1) && (lapp->bandwidth == BW40MHZ_SCA))
		{
			bb_register_write(2, 0x14, 0x9e);
			bb_register_write(2, 0x15, 0xa0);
			bb_register_write(2, 0x16, 0x1f);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 2) && (lapp->bandwidth == BW40MHZ_SCA))
		{
			bb_register_write(2, 0x14, 0x19);
			bb_register_write(2, 0x15, 0x42);
			bb_register_write(2, 0x16, 0x1b);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 5) && (lapp->bandwidth == BW40MHZ_SCN))
		{
			bb_register_write(2, 0x14, 0x41);
			bb_register_write(2, 0x15, 0x61);
			bb_register_write(2, 0x16, 0x1e);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 5) && (lapp->bandwidth == BW40MHZ_SCB))
		{
			bb_register_write(2, 0x14, 0x9e);
			bb_register_write(2, 0x15, 0xa0);
			bb_register_write(2, 0x16, 0x1f);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 6) && (lapp->bandwidth == BW40MHZ_SCN))
		{
			bb_register_write(2, 0x14, 0x91);
			bb_register_write(2, 0x15, 0x7b);
			bb_register_write(2, 0x16, 0x0e);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 6) && (lapp->bandwidth == BW40MHZ_SCA))
		{
			bb_register_write(2, 0x14, 0x6a);
			bb_register_write(2, 0x15, 0x53);
			bb_register_write(2, 0x16, 0x2f);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 6) && (lapp->bandwidth == BW40MHZ_SCB))
		{
			bb_register_write(2, 0x14, 0x19);
			bb_register_write(2, 0x15, 0x42);
			bb_register_write(2, 0x16, 0x1b);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 7) && (lapp->bandwidth == BW40MHZ_SCN))
		{
			bb_register_write(2, 0x14, 0x0e);
			bb_register_write(2, 0x15, 0x72);
			bb_register_write(2, 0x16, 0x1b);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 8) && (lapp->bandwidth == BW40MHZ_SCN))
		{
			bb_register_write(2, 0x14, 0xc8);
			bb_register_write(2, 0x15, 0x59);
			bb_register_write(2, 0x16, 0x23);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 9) && (lapp->bandwidth == BW40MHZ_SCA))
		{
			bb_register_write(2, 0x14, 0x9b);
			bb_register_write(2, 0x15, 0xa0);
			bb_register_write(2, 0x16, 0x1f);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 10) && (lapp->bandwidth == BW40MHZ_SCB))
		{
			bb_register_write(2, 0x14, 0x6a);
			bb_register_write(2, 0x15, 0x53);
			bb_register_write(2, 0x16, 0x2f);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 13) && (lapp->bandwidth == BW40MHZ_SCN))
		{
			bb_register_write(2, 0x14, 0x3a);
			bb_register_write(2, 0x15, 0x79);
			bb_register_write(2, 0x16, 0x1d);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 13) && (lapp->bandwidth == BW40MHZ_SCB))
		{
			bb_register_write(2, 0x14, 0x9b);
			bb_register_write(2, 0x15, 0xa0);
			bb_register_write(2, 0x16, 0x1f);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else if((lapp->channel == 14) && (lapp->bandwidth == BW40MHZ_SCN))
		{
			bb_register_write(2, 0x14, 0x3f);
			bb_register_write(2, 0x15, 0x53);
			bb_register_write(2, 0x16, 0x2d);
			bb_register_write(2, 0x17, 0x04);

			bb_register_write(2, 0x18, 0);
		}
		else
		{
			bb_register_write(2, 0x14, bb_notch_filter_test_backup14);
			bb_register_write(2, 0x15, bb_notch_filter_test_backup15);
			bb_register_write(2, 0x16, bb_notch_filter_test_backup16);
			bb_register_write(2, 0x17, bb_notch_filter_test_backup17);

			bb_register_write(2, 0x18, bb_notch_filter_test_backup18);
		}
	}
	else
	{
		bb_register_write(2, 0x14, bb_notch_filter_test_backup14);
		bb_register_write(2, 0x15, bb_notch_filter_test_backup15);
		bb_register_write(2, 0x16, bb_notch_filter_test_backup16);
		bb_register_write(2, 0x17, bb_notch_filter_test_backup17);

		bb_register_write(2, 0x18, bb_notch_filter_test_backup18);

	}
#endif

	if(acfg_p->tx_sgi)
		acfg_p->rate_code = setup_rate_encode(acfg_p->tx_rate, 5, RATE_FLAGS_HT_SGI40);
	else
	{
		if(acfg_p->tx_short_preamble)
			acfg_p->rate_code = setup_rate_encode(acfg_p->tx_rate, 5, RATE_FLAGS_SHORT_PREAMBLE);
		else
			acfg_p->rate_code = setup_rate_encode(acfg_p->tx_rate, 5, 0);
	}


	acfg_p->rx_count = 0;
	acfg_p->fail_count = 0;
	if (acfg_p->tx_repeat)
		acfg_p->expect_count = 0;
	acfg_p->iteration = 0;

#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
        if(pattern)
                printf("start ddr_test idle_cycle: %d, access pattern: %d\n", idle_cycle, pattern);
#endif
	while ((acfg_p->tx_repeat == -1)||(acfg_p->iteration < acfg_p->tx_repeat)) {
		if(tx_enable == 0)
		{
			MACREG_WRITE32(MAC_INT_MASK_REG, ~(MAC_INT_ACQ_TX_DONE));
			tx_enable = 1;
		}
		if (acfg_p->start == 0)
			break;

#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
		do_ddr_access_pattern();
#endif
		if ((acfg_p->iteration % acfg_p->tx_burst_num) == 0) {
			udelay(acfg_p->tx_sleep_time*1000);
		}
		acfg_p->iteration++;

#if !defined(CONFIG_SCHED)
		ret=cli_gets(buffer, phistory, 1);
		if(ret == ERR_OK)
		{
			if (1 <= (argc_t = get_args(&buffer[0], argv_t)))
			{
				if(!strcmp("wt", argv_t[0]))
				{
					int i;
					for(i=0; i < sizeof(wt_txsubcmd)/sizeof(wt_txsubcmd[0]); i++)
					{
						if(!strcmp(wt_txsubcmd[i], argv_t[1]))
						{
							cmd_proc(argc_t, argv_t);
							break;
						}
					}
				}
			}
			if(acfg_p->start)
				printf("\nCLI>");
		}
#endif
#if defined(TX_RANDOM_PACKET_GENERATE)
		tx_random_packet_generate();
#endif

		TX_packet_simply(128, 255, 0, acfg_p->tx_len, &acfg_p->rate_code);

		acfg_p->expect_count++;
	}

	if (acfg_p->rx_echo || acfg_p->rx_drop)
                wla_test_rx();

	if (acfg_p->expect_count)
		acfg_p->fail_count = acfg_p->expect_count - acfg_p->rx_count;

	acfg_p->start = 0;

	acfg_p->indicate_stop = 0;

#if defined(CONFIG_SCHED)
	thread_exit();
#endif
}

int dut_pass(void)
{
	unsigned char txvga[OTP_LEN_TXVGA], txp_diff[OTP_LEN_TXP_DIFF];
	int ret, idx;
	if(0 != otp_load())
	{
		ret = 1;
		goto fail;
	}
	for(idx = 0; idx < OTP_LEN_TXVGA; idx++)
	{
		txvga[idx] = (txvga_gain_save[idx*2] << 4) | txvga_gain_save[idx*2+1];
	}
	if(0 != otp_write(txvga, OTP_TXVGA, OTP_LEN_TXVGA))
	{
		ret = 2;
		goto fail;
	}

	if(0 != otp_write(&fofs_save, OTP_FOFS, OTP_LEN_FOFS))
	{
		ret = 3;
		goto fail;
	}

	txp_diff[0] = (bg_txp_diff << 4) | ng_txp_diff;
	txp_diff[1] = bg_txp_gap;
	if(0 != otp_write(txp_diff, OTP_TXP_DIFF, OTP_LEN_TXP_DIFF))
	{
		ret = 4;
		goto fail;
	}
	if(0 != otp_submit())
	{
		ret = 5;
		goto fail;
	}
	return 0;

fail:
	return ret;
}

int otp_mac_addr(char *str)
{
	unsigned char mac[6] = {0}, val_c;
	unsigned long val;
	int ret, idx;

	if(strlen(str) != 12)
	{
		ret = 1;
		goto fail;
	}
	for(idx = 0; idx < OTP_LEN_MAC; idx++)
	{
		val_c = *(str+(idx*2));
		if(1 != hextoul((char *) &val_c, &val))
		{
			ret = 2;
			goto fail;
		}
		val_c = (unsigned char) val & 0xff;
		mac[idx] |= val_c << 4;

		val_c = *(str+(idx*2+1));
		if(1 != hextoul((char *) &val_c, &val))
		{
			ret = 2;
			goto fail;
		}
		val_c = (unsigned char) val & 0xff;
		mac[idx] |= val_c;
	}
	if(0 != otp_load())
	{
		ret = 3;
		goto fail;
	}
	if(0 != otp_write(mac, OTP_MAC_ADDR, OTP_LEN_MAC))
	{
		ret = 4;
		goto fail;
	}
        if(0 != otp_submit())
        {
                ret = 5;
                goto fail;
        }
	memcpy(default_mac_addr, mac, OTP_LEN_MAC);

	return 0;

fail:
	return ret;
}
#ifdef CONFIG_MP_TEST
int mp_check_otp(char *str)
{
	int rc, len, tlen;
	unsigned char obuf[16] = "", *startc;

	startc = str;

	if(OTP_MEM_SIZE == (rc = otp_load(OTP_MEM_SIZE)))
	{
		tlen = len = sprintf(startc, "OTP\r\n");

		if(1 == (rc = otp_read(obuf, FOFS, 1)))
			len = sprintf(startc += len, "fofs: %02x\r\n", obuf[0]);
		else
			len = sprintf(startc += len, "fofs: Not Found\r\n");
		tlen += len;

		if(14 == (rc = otp_read(obuf, RFC1, 14)))
		{
			len = sprintf(startc += len, "txp: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n"\
			, obuf[0], obuf[1], obuf[2], obuf[3], obuf[4], obuf[5], obuf[6]\
			, obuf[7], obuf[8], obuf[9], obuf[10], obuf[11], obuf[12], obuf[13]);
		}
		else
			len = sprintf(startc += len, "txp: Not Found\r\n");
		tlen += len;

		if(6 == (rc = otp_read(obuf, MAC_ADDR, 6)))
			len = sprintf(startc += len, "mac: %02x %02x %02x %02x %02x %02x\r\n"\
						, obuf[0], obuf[1], obuf[2], obuf[3], obuf[4], obuf[5]);
		else
			len = sprintf(startc += len, "mac: Not Found\r\n");
		tlen += len;

		if(!ldev->sf.fdb.id)//7000U/S
		{
			if(1 == (rc = otp_read(obuf, TIMEOUT_SEC, 1)))
				len = sprintf(startc += len, "timeout: %02x\r\n", obuf[0]);
			else
				len = sprintf(startc += len, "timeout: Not Found\r\n");
			tlen += len;
#if 0
			if((rc == OTP_MEM_SIZE) || ((rc == 1) && ((obuf[0] & 0x0f) != 0)))
			{
				int timeout = 0;
				otp_write((u8 *)&timeout, TIMEOUT_SEC, 1);
			}
#endif
		}

		otp_end();
	}
	else
	{
		tlen = len = sprintf(startc, "OTP\r\nlynx otp check: otp load fail\r\n");
		printf("lynx otp check: otp load fail\n");
		return -1;
	}

	printf("%s", str);
	return tlen;
}

#endif

#if defined(CONFIG_SCHED)
#define WLA_TEST_THREAD_STACK_SIZE  (1024*1024)
unsigned char wla_test_thread_stack[WLA_TEST_THREAD_STACK_SIZE];
#endif
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
volatile int ddr_test_start = 0;
#if defined(CONFIG_SCHED)
#define DDR_TEST_THREAD_STACK_SIZE  (1024*1024)
unsigned char ddr_test_thread_stack[DDR_TEST_THREAD_STACK_SIZE];
void ddr_test(void)
{
	while(ddr_test_start)
	{
		do_ddr_access_pattern();
	}
	thread_exit();
}
#endif
#endif

CMD_DECL(wla_test_cmd)
{
	if (0 >= argc)
		goto help;

	if (!strcmp("stop", argv[0])) {
		lapp->ampdu_tx_mask = acfg_p->ampdu_tx_mask;
		acfg_p->ampdu_tx_mask = 0;
		if (acfg_p->start)
		{
			acfg_p->start = 0;
			acfg_p->indicate_stop = 1;
		}
	} else if (!strcmp("start", argv[0])) {
		if (acfg_p->start) {
			printf("The program has exceuted!\n");
			return ERR_OK;
		}
		if (acfg_p->indicate_stop) {
			printf("The program not stop yet!\n");
			return ERR_OK;
		}
		acfg_p->start = 1;
		// clear counter
		bb_register_write(0, 0x80, 0xc0);
		// enable counter
		bb_register_write(0, 0x80, 0x80);
#if defined(CONFIG_FREERTOS)||defined(CONFIG_OS)
#else
		ldev->raise_event(1<<EVT_WTEST_NUM);
#endif
		acfg_p->ampdu_tx_mask = lapp->ampdu_tx_mask;
		lapp->ampdu_tx_mask = 0xffff;
#if defined(CONFIG_ATE_DUT)
		if(argc == 2)
		{
			rx_result_num = atoi(argv[1]);
			rx_result_save = 1;
		}
#endif
#if defined(CONFIG_SCHED)
		thread_create(wla_test, (void *) 0, &wla_test_thread_stack[WLA_TEST_THREAD_STACK_SIZE], WLA_TEST_THREAD_STACK_SIZE);
#else
		wla_test();
#endif
	} else if (!strcmp("cfg", argv[0])) {
		printf("start:%d\n", acfg_p->start);
		printf("role:%d, key=%d bcap=%d\n", acfg_p->role, acfg_p->cipher_type, acfg_p->bcap);
		printf("channel:%d, secondary_channel=%d\n", lapp->channel, lapp->bandwidth);
//  	printf("Peer STA's address: %s\n", ether_ntoa((struct ether_addr *)acfg_p->tx_addr));
//  	printf("My address: %s\n", ether_ntoa((struct ether_addr *)acfg_p->my_addr));
		printf("-- RX --\n   drop:%d, echo:%d, dump:%d, expect:%d, filter:0x%08x\n",
						acfg_p->rx_drop, acfg_p->rx_echo,  acfg_p->rx_dump,
						acfg_p->expect_count, acfg_p->rx_filter);
		printf("   ampdu=%d\n", acfg_p->rx_ampdu);
		printf("-- TX --\n   echo:%d, repeat: %d, burst: %d,"
						"sleep time: %dms, length: %dBytes\n",
						acfg_p->tx_echo, acfg_p->tx_repeat, acfg_p->tx_burst_num,
						acfg_p->tx_sleep_time, acfg_p->tx_len);
		printf("   tx_wh:%d, tx_amsdu:%d, tx_ampdu:%d, tid:%d, wds=%d\n",
						acfg_p->tx_whdr, acfg_p->tx_amsdu, acfg_p->tx_ampdu, acfg_p->tx_ampdu_tid, acfg_p->wds);
		printf("   tx_sgi=%d, tx_gf=%d, tx_sp=%d, tx_rate=%d\n",
						acfg_p->tx_sgi, acfg_p->tx_green_field, acfg_p->tx_short_preamble,
						acfg_p->tx_rate);
		printf("   tx_pkt=%d, tx_noack=%d tx_basize=%d tx_balen=%d\n",
						acfg_p->tx_pkt, acfg_p->tx_noack, acfg_p->tx_ba_win_size, acfg_p->tx_ba_max_len);
		printf("   tx_frag=%d, tx_rts=%d\n", acfg_p->tx_frag, acfg_p->tx_rts);
		printf("   rssi: offset=%d, lna_gain_tbl=(%d, %d, %d, %d)\n", acfg_p->rssi_offset,
						acfg_p->rssi_lna_tbl[0], acfg_p->rssi_lna_tbl[1], 
						acfg_p->rssi_lna_tbl[2], acfg_p->rssi_lna_tbl[3]);
#if defined(BB_NOTCH_FILTER_TEST)
	} else if (!strcmp("bbtest", argv[0])) {
		if(argc != 2)
		{
		}
		else if(0 == atoi(argv[1]))
		{
			printf("bb test stop\n");
			bb_notch_filter_test_enable = 0;
		}
		else if(1 == atoi(argv[1]))
		{
			printf("bb test start\n");
			bb_notch_filter_test_enable = 1;
		}
#endif
	} else if (!strcmp("tx_sleep", argv[0])) {
		if (argc != 2)
			goto help;
		acfg_p->tx_sleep_time = atoi(argv[1]);
		if (1 > acfg_p->tx_sleep_time)
			acfg_p->tx_sleep_time = 1;
	} else if (!strcmp("chan", argv[0])) {
		if (argc < 2)
			goto help;

		lapp->channel = atoi(argv[1]);
		lapp->bandwidth = 0;

		if (argc >= 3)
			lapp->bandwidth = atoi(argv[2]);
	} else if (!strcmp("txrate", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->tx_rate = atoi(argv[1]);
	} else if (!strcmp("tx_sgi", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->tx_sgi = atoi(argv[1]);
	} else if (!strcmp("tx_gf", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->tx_green_field = atoi(argv[1]);
	} else if (!strcmp("tx_sp", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->tx_short_preamble = atoi(argv[1]);
	} else if (!strcmp("tx_amsdu", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->tx_amsdu = atoi(argv[1]);
	} else if (!strcmp("tx_wh", argv[0])) {
		if (argc < 2)
			goto help;

		acfg_p->tx_whdr = atoi(argv[1]);
		acfg_p->tx_pkt = 0;
		if (argc >= 3)
			acfg_p->tx_pkt = atoi(argv[2]);
	} else if (!strcmp("tx_noack", argv[0])) {
		if (argc < 2)
			goto help;

		acfg_p->tx_noack = !!atoi(argv[1]);
	} else if (!strcmp("tx_basize", argv[0])) {
		int val;

		if (argc < 2)
			goto help;
		val = atoi(argv[1]);
		if ((val != 8) && (val != 16) && (val != 32))
			goto help;

		acfg_p->tx_ba_win_size = val;
	} else if (!strcmp("tx_balen", argv[0])) {
		int val;

		if (argc < 2)
			goto help;
		val = atoi(argv[1]);
		if ((val < 0) || (val > 3))
			goto help;

		acfg_p->tx_ba_max_len = val;
	} else if (!strcmp("tx_ampdu", argv[0])) {
		int val;

		if (argc < 2)
			goto help;

		if (argc >= 3) {
			val = atoi(argv[2]);
			if ((val < 0) || (val >= 8))
				goto help;
			acfg_p->tx_ampdu_tid = val;
		}

		if ((acfg_p->tx_ampdu = !!atoi(argv[1]))) {
			acfg_p->tx_whdr = 0;
			acfg_p->tx_pkt = 0;
			acfg_p->tx_rate = 15;
		}
	} else if (!strcmp("tx_frag", argv[0])) {
		if (argc < 2)
			goto help;

		acfg_p->tx_frag = !!atoi(argv[1]);
	} else if (!strcmp("tx_rts", argv[0])) {
		if (argc < 2)
			goto help;

		acfg_p->tx_rts = !!atoi(argv[1]);
	} else if (!strcmp("wds", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->wds = atoi(argv[1]);
#if 0
	} else if (!strcmp("addr", argv[0])) {
		if (argc != 2)
			goto help;
		
		memcpy(acfg_p->tx_addr, (char *)ether_aton(argv[1]), 6);
	} else if (!strcmp("maddr", argv[0])) {
		if (argc != 2)
			goto help;

		memcpy(acfg_p->my_addr, (char *)ether_aton(argv[1]), 6);
#endif
	} else if (!strcmp("tx", argv[0])) {
		if (argc < 2)
			goto help;

		if (!strcmp("repeat", argv[1]))
			acfg_p->tx_repeat = 0xffffffff;
		else
#if !defined(WLA_TEST)
			acfg_p->tx_repeat = atoi(argv[1]);
#else
			acfg_p->tx_repeat = 0xffffffff;
#endif

		if (argc >= 3)
			acfg_p->tx_len = atoi(argv[2]);

		if (argc >= 4)
			acfg_p->tx_burst_num = atoi(argv[3]);//range: 1 ~ 32

		if (argc >= 5)
			acfg_p->tx_echo = 1;
		else
			acfg_p->tx_echo = 0;

		acfg_p->rx_drop = 0;
		acfg_p->rx_echo = 0;
	} else if (!strcmp("dump", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->rx_dump = atoi(argv[1]);
	} else if (!strcmp("filter", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->rx_filter = atoi(argv[1]);
	} else if (!strcmp("rx", argv[0])) {
		if (!strcmp("echo", argv[1]))
			acfg_p->rx_echo = 1;
		else
			acfg_p->rx_drop = 1;

		acfg_p->tx_repeat = 0; /* disable tx */
		if (argc >= 3)
			acfg_p->expect_count = atoi(argv[2]);
	} else if (!strcmp("rx_ampdu", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->rx_ampdu = !!atoi(argv[1]);
	} else if (!strcmp("stat", argv[0])) {
		printf("Total: %d\nPass: %d\nFail: %d\nTX: %d\nRX: %d\n",
						acfg_p->expect_count, acfg_p->rx_count, acfg_p->fail_count,
						acfg_p->tx_count, acfg_p->rx_count);
#ifdef CONFIG_OS
		printf("ALLOC: %d\nFREE: %d\n", acfg_p->alloc_count, acfg_p->free_count);
#endif
	} else if (!strcmp("role", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->role = atoi(argv[1]);
	} else if (!strcmp("bcap", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->bcap = atoi(argv[1]);
        } else if (!strcmp("cmd_sleep", argv[0])) {
		if (argc != 2)
			goto help;

		acfg_p->cmd_sleep_time = atoi(argv[1]);
	} else if (!strcmp("bbcnt", argv[0])) {
		unsigned long value;

		// disable counter
		bb_register_write(0, 0x80, 0x0);
		// read ok counter high byte
		value = (bb_register_read(0, 0x89) << 8);
		// read ok counter low byte
		value |= bb_register_read(0, 0x8a);
		
		printf("%d\n", value);
	} else if (!strcmp("single_tone", argv[0])) {
		int set_en;

		if(argc != 2)
			goto help;
		set_en = atoi(argv[1]);
		if(set_en)
			PMUREG_UPDATE32(0xf8, 0x16, 0x1e);
		else
			PMUREG_UPDATE32(0xf8, 0, 0x1e);
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
        } else if (!strcmp("ddr_test", argv[0])) {
		if (argc != 3)
                        goto help;
		idle_cycle = atoi(argv[1]);
		pattern = atoi(argv[2]);
		printf("config ddr_test idle_cycle: %d, access pattern: %d\n", idle_cycle, pattern);
#endif
        } else if (!strcmp("txp_diff", argv[0])) {
                unsigned int txvga_diff, bg_gap;
                if(argc == 1)
                {
                        serial_printf("%x %x %x\n", bg_txp_diff, ng_txp_diff, bg_txp_gap);
                        return ERR_OK;
                }
                if(argc != 3)
                        goto help;

                if(sscanf(argv[1], "%x", &txvga_diff) != 1)
                        goto help;
                if(sscanf(argv[2], "%x", &bg_gap) != 1)
                        goto help;

                if(((txvga_diff >> 4) > 12) || ((txvga_diff & 0x0f) > 12) ||\
                        !((bg_gap == 0x06) || (bg_gap == 0x03)))
                {
                        serial_printf("Invalid value\n");
                        goto help;
                }

                bg_txp_diff = (txvga_diff >> 4);
                ng_txp_diff = (txvga_diff & 0x0f);
                bg_txp_gap = bg_gap;

                serial_printf("bg diff %x\nng diff %x\n", bg_txp_diff, ng_txp_diff);
                serial_printf("b g mode txvga gap %x\n", bg_txp_gap);
	} else if (!strcmp("per", argv[0])) {
		unsigned char err_rate[32];
		if(argc != 1)
			goto help;
		check_pkt_err_rate((char *) err_rate);
		printf("%s", err_rate);
	} else if (!strcmp("txvga", argv[0])) {
		unsigned char gain, val, txvga_t;
		if(argc == 1)
		{
			txvga_t = txvga_gain[lapp->channel - 1];
			if(txvga_t >= 0x0 && txvga_t <= 0x6)
				printf("%d\n", 50 + txvga_t);
			else
				printf("%d\n", 44 + (txvga_t - 0xa));
			udelay(acfg_p->cmd_sleep_time*1000);
		}
		else if(argc == 2)
		{
			if(!strcmp("save", argv[1]))
			{
				txvga_gain_save[lapp->channel - 1] = txvga_gain[lapp->channel - 1];
			}
			else
			{
				if(1 == strlen(argv[1]))
				{
					gain = string_vga_converter(argv[1][0]);
				}
				else if(2 == strlen(argv[1]))
				{
					txvga_t = atoi(argv[1]);

					if(txvga_t == txvga_def)
						gain = 0;
					else if(txvga_t < txvga_def)
					{
						if(txvga_t < (txvga_def - 6))
							txvga_t = txvga_def - 6;
						gain = 16 - (txvga_def - txvga_t);
					}
					else//txvga > txvga_def
					{
						if(txvga_t > (txvga_def + 6))
							txvga_t = txvga_def + 6;
						gain = txvga_t - txvga_def;
					}
				}
				else
				{
					return ERR_OK;
				}
				if(acfg_p->tx_rate >= 1 && acfg_p->tx_rate <= 4)
				{
					val = (bb_register_read(2, 0x80) & 0xf0) | gain;//bb gain bit 3 ~ 0
					bb_register_write(2, 0x80, val);//b mode
				}
				else if(acfg_p->tx_rate >= 5 && acfg_p->tx_rate <= 12)
				{
					val = (bb_register_read(2, 0x81) & 0xf0) | gain;//bb gain bit 3 ~ 0
					bb_register_write(2, 0x81, val);//g mode
				}
				else if(acfg_p->tx_rate >= 13 && acfg_p->tx_rate <= 20)
				{
					val = (bb_register_read(2, 0x82) & 0xf0) | gain;//bb gain bit 3 ~ 0
					bb_register_write(2, 0x82, val);//n mode
				}
				txvga_gain[lapp->channel - 1] = gain;
			}
		}
	} else if (!strcmp("txvga_dump", argv[0])) {
		int idx, txvga_gain_decimal[14];
		for(idx=0;idx<14;idx++)
			txvga_gain_decimal[idx] = decimal_vga_converter(txvga_gain[idx]);
		printf("txvga gain(decimal): %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			txvga_gain_decimal[0], txvga_gain_decimal[1], txvga_gain_decimal[2], txvga_gain_decimal[3], txvga_gain_decimal[4],
			txvga_gain_decimal[5], txvga_gain_decimal[6], txvga_gain_decimal[7], txvga_gain_decimal[8], txvga_gain_decimal[9],
			txvga_gain_decimal[10], txvga_gain_decimal[11], txvga_gain_decimal[12], txvga_gain_decimal[13]);
	} else if (!strcmp("txvga_otp", argv[0])) {
		int idx, txvga_otp_ret;
		unsigned char txvga[OTP_LEN_TXVGA];
		if(0 != otp_load())
		{
			txvga_otp_ret = 1;
		}
		else
		{
			for(idx = 0; idx < OTP_LEN_TXVGA; idx++)
			{
				txvga[idx] = (txvga_gain_save[idx*2] << 4) | txvga_gain_save[idx*2+1];
			}
			if(0 != otp_write(txvga, OTP_TXVGA, OTP_LEN_TXVGA))
			{
				txvga_otp_ret = 2;
			}
			else
			{
				if(0 != otp_submit())
				{
					txvga_otp_ret = 3;
				}
				else
				{
					txvga_otp_ret = 0;
				}
			}
		}
		printf("%d\n", txvga_otp_ret);
	} else if (!strcmp("fofs_otp", argv[0])) {
		int fofs_otp_ret;
		if(0 != otp_load())
		{
			fofs_otp_ret = 1;
		}
		else
		{
			if(0 != otp_write(&fofs_save, OTP_FOFS, OTP_LEN_FOFS))
			{
				fofs_otp_ret = 2;
			}
			else
			{
				if(0 != otp_submit())
				{
					fofs_otp_ret = 3;
				}
				else
				{
					fofs_otp_ret = 0;
				}
			}
		}
		printf("%d\n", fofs_otp_ret);
	} else if (!strcmp("otp_space", argv[0])) {
		printf("%d\n", otp_get_avaliable_space());
	} else if (!strcmp("mp_dump", argv[0])) {
		printf("txvga gain: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			txvga_gain_save[0], txvga_gain_save[1], txvga_gain_save[2], txvga_gain_save[3],
			txvga_gain_save[4], txvga_gain_save[5], txvga_gain_save[6], txvga_gain_save[7],
			txvga_gain_save[8], txvga_gain_save[9], txvga_gain_save[10], txvga_gain_save[11],
			txvga_gain_save[12], txvga_gain_save[13]);
		printf("fofs: %02x\n", fofs_save);
		printf("txp_diff: %02x %02x %02x\n", bg_txp_diff, ng_txp_diff, bg_txp_gap);
	} else if (!strcmp("dut_pass", argv[0])) {
		int dut_pass_ret;
		dut_pass_ret = dut_pass();
		printf("%d\n", dut_pass_ret);
	} else if (!strcmp("mac_addr", argv[0])) {
		unsigned char buf[10];
		int otp_mac_ret;
		if(argc == 1)
		{
			if(0 == otp_load())
			{
				if(OTP_LEN_MAC == otp_read(buf, OTP_MAC_ADDR))
					memcpy(default_mac_addr, buf, OTP_LEN_MAC);
			}
			printf("%02x %02x %02x %02x %02x %02x\n", default_mac_addr[0], default_mac_addr[1], default_mac_addr[2],
				default_mac_addr[3], default_mac_addr[4], default_mac_addr[5]);
		}
		else if(argc == 2)
		{
			otp_mac_ret = otp_mac_addr(argv[1]);
			printf("%d\n", otp_mac_ret);
		}
	} else if (!strcmp("freq_offset_test", argv[0])) {
		acfg_p->en_freq_offset = 1;
		acfg_p->freq_offset_count = 0;
		acfg_p->freq_offset_sum = 0;
		acfg_p->rssi_sum = 0;
		acfg_p->self_cfo = atoi(argv[1]);
		printf("enable freq offset test : self_cfo = %d\n", acfg_p->self_cfo);
	} else if (!strcmp("fofs", argv[0])) {
		if (argc == 1)
		{
			printf("%d\n", fofs);
		}
		else if(argc == 2)
		{
			int idx = 0, isvalid = 1;
			if(!strcmp("save", argv[1]))
				fofs_save = fofs;
			else
			{
				while(idx < strlen(argv[1])) {
					if(!(argv[1][idx] >= '0' && argv[1][idx] <= '9')) {
						isvalid = 0;
						break;
					}
					idx++;
				}
				if(isvalid) {
					fofs = atoi(argv[1]);
					rf_write(8, fofs << 19);
				}
			}
		}
		else
			goto help;
	} else if (!strcmp("count", argv[0])) {
		if(argc == 1)
			freq_offset_calc(3);
		else
			goto help;
	} else if (!strcmp("rssi", argv[0])) {
		if (argc == 1)
		{
			freq_offset_calc(2);
			return ERR_OK;
		}
		if (argc != 6)
			goto help;

		acfg_p->rssi_offset = atoi(argv[1]);
		acfg_p->rssi_lna_tbl[0] = atoi(argv[2]);
		acfg_p->rssi_lna_tbl[1] = atoi(argv[3]);
		acfg_p->rssi_lna_tbl[2] = atoi(argv[4]);
		acfg_p->rssi_lna_tbl[3] = atoi(argv[5]);
	} else if (!strcmp("rx_phy_info_verify_start", argv[0])) {
		enable_sniffer_mode();
		rx_phy_info_record = 1;
		return ERR_OK;
	} else if (!strcmp("rx_phy_info_flush", argv[0])) {
		rx_phy_info_storage_idx = 0;
		return ERR_OK;
	} else if (!strcmp("rx_phy_info_dump", argv[0])) {
		dump_rx_phy_info();
		return ERR_OK;
#if defined(CONFIG_ATE_DUT)
	} else if (!strcmp("result", argv[0])) {
		freq_offset_calc(1);//fofs
		printf("OK\n");
		freq_offset_calc(3);//count
		printf("OK\n");
		freq_offset_calc(2);//rssi
	} else if (!strcmp("rx_result", argv[0])) {
		if(argc != 2)
			goto help;
		int num = atoi(argv[1]), pnum;
		if(num > 9)
			num = 9;
		for(pnum = 0; pnum <= num; pnum++)
			printf("r%d:\n%d\n%d\n%d\n", pnum, rx_result[pnum][0], rx_result[pnum][1], rx_result[pnum][2]);
#endif
	} else if (!strcmp("ez", argv[0])) {
		wt_ez();
	} else if (!strcmp("rez", argv[0])) {
		wt_rez();
	} else {
		goto help;
	}
	return ERR_OK;
help:
	printf("wt start (start the program)\n"
					 "\tstop (force to stop)\n"
					 "\tcfg (show all configuration)\n"
					 "\trole <STA:0x1, AP:0x2>\n"
					 "\tbcap <0/1> (bcap table)\n"
					 "\tstat\n"
					 "\taddr <aa:bb:cc:dd:ee:ff> (peer STA's mac address)\n"
					 "\tmaddr <aa:bb:cc:dd:ee:ff> (my mac address)\n");
	printf("\tkey <none:0, WEP40:1, WEP104:2, TKIP:3, CCMP:4, SMS4:5>\n"
					 "\ttx_sleep <tx_sleep_time> (ms)\n"
					 "\tchan <channel_num> <second channel: 1:Above, 3:Below>\n"
					 "\tbbcnt\n"
					 "\trx <drop|echo> <expect num>\n"
					 "\trx_ampdu <0/1> <tid> (RX AMPDU)\n"
					 "\tdump <0: no show, 0x1:simple, 0x2:payload> (dump RX packet)\n");
	printf("\tfilter <MT:0x1, beacon:0x2, mgt:0x4, ctrl:0x8, data:0x10, bcast:0x20> (RX desired packet)\n"
					 "\ttxrate <1~4:CCK, 5~12:OFDM, 13~20:MCS> (fixed TX rate)\n"
					 "\ttx_gf <0/1> (Green Field)\n"
					 "\ttx_sp <0/1> (Short Preamble)\n"
					 "\ttx_sgi <0/1> (Short Guard Interval)\n");
	printf("\ttx_wh <0/1> (wlan header)\n"
					 "\twds <0/1> (WDS frame)\n"
					 "\ttx_amsdu <0/1> (TX AMSDU)\n"
					 "\ttx_ampdu <0/1> <tid> (TX AMPDU)\n"
					 "\ttx_noack <0/1> (ack policy)\n"
					 "\ttx_basize <8/16/32> (ba windown size)\n"
					 "\ttx_balen <0x0:8k, 0x1:16k, 0x2:32k, 0x3:64k> (ba max length)\n");
	printf("\ttx_frag <0/1> (fragment)\n"
					 "\ttx_rts <0/1> (TX RTS)\n"
					 "\ttx <count>/repeat <payload_length> <burst> <echo>\n"
					 "\ttxvga <value|save>\n");
	printf("\tfreq_offset_test <self_cfo>\n");
	printf("\tfofs; read freq offset avg\n");
	printf("\tcount; read packet error rate\n");
	printf("\trssi <offset> <lna gain 0> <lna gain 1> <lna gain 2> <lna gain 3>\n");
	printf("\twt ez\n");
	printf("\twt rez\n");
	printf("\twt rx_phy_info_verify_start\n");
	printf("\twt rx_phy_info_flush\n");
	printf("\twt rx_phy_info_dump\n");

	return ERR_OK;
}

CLI_CMD(wt, wla_test_cmd, "wt <argv>; WLA test");

CMD_DECL(mp_cmd)
{
	if (0 >= argc)
		goto help;

	if (!strcmp("otp", argv[0])) {
		unsigned char buf[10];
		int size, otp_ready = 0;
		printf("OTP\n");
		if(0 == otp_load())
		{
			otp_ready = 1;
		}
		else
		{
			printf("load fail\n");
		}
		if(otp_ready && (OTP_LEN_FOFS == (size = otp_read(buf, OTP_FOFS))))
		{
			printf("fofs: %02x\n", buf[0]);
		}
		else
			printf("fofs: Not Found\n");
		if(otp_ready && (OTP_LEN_TXVGA == (size = otp_read(buf, OTP_TXVGA))))
		{
			printf("txp: %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
		}
		else
			printf("txp: Not Found\n");
		if(otp_ready && (OTP_LEN_MAC == (size = otp_read(buf, OTP_MAC_ADDR))))
		{
			printf("mac: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		}
		else
			printf("mac: Not Found\n");
	}
	else if(!strcmp("version", argv[0])) {
		printf("%s\n", BOOT_REVNUM);
	}
	return ERR_OK;
help:
	printf("mp otp ; dump mp otp data\n");
	return ERR_OK;
}

CLI_CMD(mp, mp_cmd, "mp <argv>; Manufacture Test Program");

#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
CMD_DECL(ddr_test_cmd)
{
	if (!strcmp("stop", argv[0])) {
		ddr_test_start = 0;
		return ERR_OK;
	}
	if(argc == 2) {
#if defined(CONFIG_SCHED)
		if(ddr_test_start)
		{
			printf("ddr_test already started\n");
			return ERR_OK;
		}
		else
		{
			idle_cycle = atoi(argv[0]);
			pattern = atoi(argv[1]);
			printf("config and start ddr_test idle_cycle: %d, access pattern: %d\n", idle_cycle, pattern);
			ddr_test_start = 1;
		}
		thread_create(ddr_test, (void *) 0, &ddr_test_thread_stack[DDR_TEST_THREAD_STACK_SIZE], DDR_TEST_THREAD_STACK_SIZE);
#else
		printf("please config sched for ddr test\n");
#endif
		return ERR_OK;
	}
	printf("ddr_test [idle] [pattern] or ddr_test stop\n");
	return ERR_OK;
}
CLI_CMD(ddr_test, ddr_test_cmd, "ddr_test [idle] [pattern] or ddr_test stop");
#endif
void freq_offset_calc(int option)
{
	int count = acfg_p->freq_offset_count;
	int offset_sum = acfg_p->freq_offset_sum;
	int rssi_sum = acfg_p->rssi_sum;
	int self_cfo = acfg_p->self_cfo;
	int offset_avg, rssi_avg;

	offset_avg = ((offset_sum/count) * (-2)) + self_cfo;
	rssi_avg = rssi_sum/count;

	if(option == 0)
		printf("freq offset calc result: count=%d, self_cfo=%d, offset_sum=%d, offset_avg=%d, rssi_sum=%d, rssi_avg=%d\n", count, self_cfo, offset_sum, offset_avg, rssi_sum, rssi_avg);
	else if(option == 1)
		printf("%d\n", offset_avg);
	else if(option == 2)
		printf("%d\n", rssi_avg);
	else if(option == 3)
		printf("%d\n", count);
}

void wt_ez(void)
{
	lrf_set_pll(2442);
	//lrf_tx_on();

	set_txcal_txvga(0);

	// wt chan 7
	lapp->channel = 7;
	lapp->bandwidth = 0;

	// wt txrate 20
	acfg_p->tx_rate = 20;

	// wt tx 10000000 512 3
	acfg_p->tx_repeat = 10000000;
	acfg_p->tx_len = 512;
	acfg_p->tx_burst_num = 3;
	acfg_p->tx_echo = 0;
	acfg_p->rx_drop = 0;
	acfg_p->rx_echo = 0;
}

void wt_rez(void)
{
	lrf_set_pll(2442);

	// wt chan 7
	lapp->channel = 7;
	lapp->bandwidth = 0;

	// wt rx drop 1
	acfg_p->rx_drop = 1;
	acfg_p->tx_repeat = 0; /* disable tx */
	acfg_p->expect_count = 1;

	// rfc analog_on
	rf_write(18, (rf_read(18) & 0x007FFFFF) | 0xA0800000);
}

//for Boot form flash ate_dut test
#ifdef CONFIG_ATE_DUT
void save_wt_rx_result(void)//rssi, fofs, count
{
	int count = acfg_p->freq_offset_count;
	int offset_sum = acfg_p->freq_offset_sum;
	int rssi_sum = acfg_p->rssi_sum;
	int self_cfo = acfg_p->self_cfo;
	int offset_avg, rssi_avg;

	offset_avg = ((offset_sum/count) * (-2)) + self_cfo;
	rssi_avg = rssi_sum/count;

	rx_result[rx_result_num][0] = rssi_avg;//rssi
	rx_result[rx_result_num][1] = offset_avg;//fofs
	rx_result[rx_result_num][2] = count;//count
	rx_result_save = 0;
}
#endif

#ifdef CONFIG_ATE
void ate_set_chan(u8 chan, u8 s_chan)
{
	lapp->channel = chan;
	lapp->bandwidth = s_chan ;
}
#endif

