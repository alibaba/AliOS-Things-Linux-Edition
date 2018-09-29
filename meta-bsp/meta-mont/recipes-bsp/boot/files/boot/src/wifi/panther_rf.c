/*=============================================================================+
|                                                                              |
| Copyright 2015                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file panther_rf.c
*   \brief  RF driver.
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <lib.h>
#include <panther_dev.h>
//#include <wla_cfg.h>
#include <mac_ctrl.h>
#include <panther_debug.h>
//#include <cmd.h>
#include <cli_api.h>
#include <panther_rf.h>
#include <panther_rfac.h>
#include <rfc_comm.h>
#include <arch/chip.h>
#include <rf.h>
#include <bb.h>
#include <mt_driver_Panther.h>

#if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <asm/mach-panther/idb.h>
#endif

int printf(char *fmt, ...);
int net_tftp(int put, unsigned long dest_address, unsigned int bytecount,char *file);
void bb_dump_init_setting(void);
void bb_dump_start(void);
int is_dump_finish(void);
int is_bb_buffer_overflow(void);
void bb_dump(void);
void beacon_tx(u32 ch, u32 pkt_num);
void unicast_tx(u32 ch, u32 pkt_num);

struct rfc_cal_reg rfc_result_panther[2], *rfc_result_ptr=&rfc_result_panther[0];

// 22 = vga_max - vga_min + 1 = 26 - 5 + 1
unsigned int rxdc_rec[22], *rxdc_rec_ptr=&rxdc_rec[0];	

struct rf_tbl {
	unsigned int address;
	unsigned int val;
	unsigned int mask;
};

struct rf_tbl rf_init_regs[] = {
	{0, 0x4059B3E8, 0},
	{1, 0x80400310, 0},
	{2, 0x7402FCC, 0},
	{3, 0xCC8D00, 0},
	{4, 0xCA, 0},
	{5, 0x4860512D, 0},
	{6, 0x80108240, 0},
	{7, 0x0, 0},
//#if (CONFIG_ROM_VER > 1)
	{8, 0x980000, 0},
//#else
//        {8, 0xF80000, 0},
//#endif
	{9, 0x46646D61, 0},
	{10, 0x84000A91, 0},
	{11, 0x11000000, 0},
	{12, 0x40004820, 0},
#if (CONFIG_PANTHER_CHIP_VERSION >= 1)
	{13, 0x73495BBB, 0},
	{14, 0x0073F108, 0},
	{15, 0x0091AFAD, 0},
	{16, 0x5E2296, 0},
	{17, 0x2FC723C3, 0},
	{18, 0xA001108C, 0},
	{19, 0x3211BADB, 0},
	{20, 0x2620198, 0},
	{21, 0x8040689, 0},
	{22, 0x87478384, 0},
	{23, 0x7E0C, 0},
	{24, 0xFC710000, 0},
#else
	{13, 0x72491BBB, 0},
	{14, 0x73EF09, 0},
	{15, 0x918F9D, 0},
	{16, 0x5E2296, 0},
	{17, 0x130723C3, 0},
	{18, 0xA001108C, 0},
	{19, 0x3211BADB, 0},
	{20, 0x2620198, 0},
	{21, 0x8040689, 0},
	{22, 0x87478384, 0},
	{23, 0x7E0C, 0},
	{24, 0x88810000, 0},
#endif
	{25, 0x0, 0},
	{26, 0x8, 0},
	{27, 0xA321, 0},
	{28, 0x2C4A214, 0},
	{29, 0x3BC8, 0},	
};


struct pll_data {
	unsigned int msb;
	unsigned int lsb;
};

struct pll_data pll_record[14];

struct channel{
	u16	num;
	u16 freq;
} lrf_channel_data[] = {
	{ 1, 2412 },
    { 2, 2417 },
    { 3, 2422 },
    { 4, 2427 },
    { 5, 2432 },
    { 6, 2437 },
    { 7, 2442 },
    { 8, 2447 },
    { 9, 2452 },
    { 10, 2457 },
    { 11, 2462 },
    { 12, 2467 },
    { 13, 2472 },
    { 14, 2484 },
	{ 255, 2520 },
};

#define CHANNEL_WIDTH_20MHZ		0
#define CHANNEL_WIDTH_40MHZ		1

int lrf_chan2idx(int channel_num)
{
	int i;

	for(i=0;i<(sizeof(lrf_channel_data)/sizeof(lrf_channel_data[0]));i++)
	{
		if(lrf_channel_data[i].num == channel_num)
		{
			break;
		}
	}
	
	return i;
}

int lrf_freq2idx(int freq)
{
	int i;

	for(i=0;i<(sizeof(lrf_channel_data)/sizeof(lrf_channel_data[0]));i++)
	{
		if(lrf_channel_data[i].freq == freq)
		{
			break;
		}
	}
	
	return i;
}

int lrf_ch2freq(int ch)
{
	int i;
	int freq = lrf_channel_data[0].freq;

	for(i=0;i<(sizeof(lrf_channel_data)/sizeof(lrf_channel_data[0]));i++)
	{
		if(lrf_channel_data[i].num == ch)
		{
			freq = lrf_channel_data[i].freq;
			break;
		}
	}
	
	return freq;
}

void lrf_set_40mhz_channel(int channel_num, int channel_type)
{
    int central_ch_freq, idx;
#ifndef CONFIG_MONTE_CARLO
    int new_val;
#endif

    if(channel_type == BW40MHZ_SCA)
	    channel_num += 2;
    else if(channel_type == BW40MHZ_SCB)
	    channel_num -= 2;

    idx = lrf_chan2idx(channel_num);

    if(idx >= sizeof(lrf_channel_data)/sizeof(lrf_channel_data[0]))
	    return;

    central_ch_freq = lrf_channel_data[idx].freq;

    lrf_set_pll(central_ch_freq);

#ifndef CONFIG_MONTE_CARLO
    new_val = *(volatile unsigned int *)0xbf004c7c;
    new_val &= ~(0x80000000);
    if (channel_type)
    {
        rf_update(10, 0x00800000, 0x00800000);
        new_val |= 0x80000000;
    }
    else
    {
        rf_update(10, 0x0, 0x00800000);
    }

    *(volatile unsigned int *)0xbf004c7c = new_val;
#endif
}

struct gain_level {
	unsigned char gsel_txmod;
	unsigned char tx_lpf_cfg;
} lrf_gain_data[] = {
	{0x4, 0x4},		/* 0 */		/* rf gain 3 */
	{0x1, 0x2},		/* 1 */		/* rf gain -12 */
	{0x1, 0x3},		/* 2 */		/* rf gain -9 */
	{0x1, 0x3},		/* 3 */		/* rf gain -9 */
	{0x1, 0x4},		/* 4 */		/* rf gain -6 */
	{0x1, 0x4},		/* 5 */		/* rf gain -6 */
	{0x1, 0x4},		/* 6 */		/* rf gain -6 */
	{0x2, 0x4},		/* 7 */		/* rf gain -3 */
	{0x2, 0x4},		/* 8 */		/* rf gain -3 */
	{0x2, 0x4},		/* 9 */		/* rf gain -3 */
	{0x3, 0x4},		/* 10 */	/* rf gain 0 */
	{0x3, 0x4},		/* 11 */	/* rf gain 0 */
	{0x3, 0x4},		/* 12 */	/* rf gain 0 */
	{0x4, 0x4},		/* 13 */	/* rf gain 3 */
	{0x4, 0x4},		/* 14 */	/* rf gain 3 */
	{0x4, 0x4},		/* 15 */	/* rf gain 3 */
};

void lrf_set_tx_power(unsigned int level)
{
	struct gain_level *ptr;

	if(level >= (sizeof(lrf_gain_data)/sizeof(struct gain_level)))
		level = 0;

	ptr = &lrf_gain_data[level];

	rf_update(18, ptr->gsel_txmod << 6, 0x7 << 6);
	rf_update(9, ptr->tx_lpf_cfg << 28, 0x7 << 28);
#ifdef CONFIG_IOT_DEMO
	/* minus 3db */
	rf_write(9, 0x36646361);
#endif

	WLAN_DBG("%s(): set pwr level to %d\n", __FUNCTION__, level);
}

unsigned int lrf_get_tx_power(void)
{
	unsigned int i;
	//u32 val = rf_read(0x7);
	unsigned char gsel_txmod = ((rf_read(18) >> 6) & 0x7);
	unsigned char tx_lpf_cfg = ((rf_read(9) >> 28) & 0x7);
	struct gain_level *ptr;

	for(i=0; i<(sizeof(lrf_gain_data)/sizeof(struct gain_level)); i++)
	{
		ptr = &lrf_gain_data[i];

		if((gsel_txmod == ptr->gsel_txmod) && (tx_lpf_cfg == ptr->tx_lpf_cfg))
			break;
	}

	return i;
}

int lrf_rf_on(int is_tx)
{
	if(is_tx)
	{
		rf_write(0x0, 0x59F2EB);
		rf_write(0x1, 0xB8000311);
	}
	else
	{
		rf_write(0x0, 0x59B30F);
		rf_write(0x1, 0x3000311);
	}
	rf_write(0x2, 0x9412F0C);
	rf_write(0x3, 0x338D00); /* Disable internal PA, it will degrade B-mode throughput */
	rf_write(0x4, 0x4);
	rf_write(0x5, 0x2301);
	rf_write(0x6, 0x504960);
	rf_write(0x7, 0x1108240); /* increase TX power */
	rf_write(0x8, 0);
	rf_write(0x9, 0x43646761);
	rf_write(0xa, 0x80000591);
	rf_write(0xb, 0x11000000);
	rf_write(0xc, 0x4860);
	rf_write(0xd, 0x72495AB1);
	rf_write(0xe, 0x73F118);
	rf_write(0xF, 0x911F8D);
	rf_write(0x10, 0x5E2296);
	rf_write(0x11, 0x723C3);
	rf_write(0x12, 0x240010CC);
	rf_write(0x13, 0x2411BADB);
	rf_write(0x14, 0x2620218);
	rf_write(0x15, 0x40689);
	rf_write(0x16, 0x0);
	rf_write(0x17, 0x6182);
	rf_write(0x18, 0x10000);
	rf_write(0x19, 0x0);
	rf_write(0x1a, 0x0);
	rf_write(0x1b, 0x0);
	rf_write(0x1c, 0x2C4A214);
	if(is_tx)
		rf_write(0x1d, 0x3BC9);
	else
		rf_write(0x1d, 0x3BC8);

	return 0;
}

void panther_rf_txrx_init(void)
{
    int i, count;
    struct rf_tbl *entry;

    count = (sizeof(rf_init_regs))/(sizeof(struct rf_tbl));

    for(i = 0; i < count; i++)
    {
	entry = &rf_init_regs[i];
	rf_write(entry->address, entry->val);
    }
}

void panther_rf_txrx_ctrl_mode(void)
{
    rf_write(0, 0x5BABE0);
    rf_write(1, 0x400311);
    rf_write(2, 0x7402FCC);
    rf_write(3, 0xCC8D00);
    rf_write(4, 0xCA);
    rf_write(5, 0x4860512D);
    rf_write(6, 0x80108240);
    rf_write(7, 0x0);
    rf_write(8, 0x980000);
    rf_write(9, 0x46646361);
    rf_write(10, 0x84000A91);
    rf_write(11, 0x11000000);
    rf_write(12, 0x40004820);
#if (CONFIG_PANTHER_CHIP_VERSION >= 1)
    rf_write(13, 0x73495BBB);
    rf_write(14, 0x0073F108);
    rf_write(15, 0x0091AFAD);
    rf_write(16, 0x5E2296);
    rf_write(17, 0x2FC723C3);
    rf_write(18, 0xA001108C);
    rf_write(19, 0x3211BADB);
    rf_write(20, 0x2620198);
    rf_write(21, 0x8040689);
    rf_write(22, 0x87478384);
    rf_write(23, 0x7E0C);
    rf_write(24, 0xFC710000);
#else
    rf_write(13, 0x72491BBB);
    rf_write(14, 0x73EF09);
    rf_write(15, 0x918F9D);
    rf_write(16, 0x5E2296);
    rf_write(17, 0x130723C3);
    rf_write(18, 0xA001108C);
    rf_write(19, 0x3211BADB);
    rf_write(20, 0x2620198);
    rf_write(21, 0x8040689);
    rf_write(22, 0x87478384);
    rf_write(23, 0x7E0C);
    rf_write(24, 0x88810000);
#endif
    rf_write(25, 0x0);
    rf_write(26, 0x8);
    rf_write(27, 0xA321);
    rf_write(28, 0x2C4A214);
    rf_write(29, 0x3BC8);
}

void panther_rf_txrx_on(void)
{
    panther_rf_txrx_init();
    panther_rf_txrx_ctrl_mode();
//#if 1
//        int i, count;
//        struct rf_tbl *entry;
//
//        count = (sizeof(rf_init_regs))/(sizeof(struct rf_tbl));
//
//        for(i=0; i<count; i++)
//        {
//                entry = &rf_init_regs[i];
//#if 0
//                if(entry->mask == 0)
//                        rf_write(entry->address, entry->val);
//                else
//                        rf_update(entry->address, entry->val, entry->mask);
//#else
//                /* rf init table only do all regs overwrite */
//                rf_write(entry->address, entry->val);
//#endif
//        }
//
//#else
//        rf_write(0, 0x4059B3E8);
//        rf_write(1, 0x80400310);
//        rf_write(2, 0x9412F0C);
//        rf_write(3, 0x338D00);
//        rf_write(4, 0xc4);
//        rf_write(5, 0x2301);
//        rf_write(6, 0x504960);
//        rf_write(7, 0x1108240);
//#if (CONFIG_ROM_VER > 1)
//        rf_write(8, 0x980000);
//#else
//        rf_write(8, 0xF80000);
//#endif
//        rf_write(9, 0x26646D61);
//#if (CONFIG_ROM_VER > 1)
//        rf_write(10, 0x84000A91);
//#else
//        rf_write(10, 0x80000A91);
//#endif
//        rf_write(11, 0x11000000);
//        rf_write(12, 0x40004820);
//#if (CONFIG_ROM_VER > 1)
//        rf_write(13, 0x72491BBB);
//#else
//        rf_write(13, 0x7249DAB1);
//#endif
//#if (CONFIG_ROM_VER > 1)
//        rf_write(14, 0x73EF09);
//#else
//        rf_write(14, 0x73F10A);
//#endif
//#if (CONFIG_ROM_VER > 1)
//        rf_write(15, 0x10911F9D);
//#else
//        rf_write(15, 0x911F9D);
//#endif
//        rf_write(16, 0x5E2296);
//        rf_write(17, 0x723C3);
//        rf_write(18, 0x2400110C);
//        rf_write(19, 0x2211BADB);
//        rf_write(20, 0x2620198);
//        rf_write(21, 0x40689);
//        rf_write(22, 0x0);
//        rf_write(23, 0x6182);
//        rf_write(24, 0x10000);
//        rf_write(25, 0x0);
//        rf_write(26, 0x0);
//        rf_write(27, 0x0);
//        rf_write(28, 0x2C4A214);
//        rf_write(29, 0x3BC8);
//#endif
}

int lrf_lpf_cal(void)
{
	unsigned int val, mask;
	int k=0, ret=0;

	val = 1 << 11;
	rf_update(1, val, val);
	rf_update(28, 0, 0x1);
	rf_update(28, 1, 0x1);

	val = (0x3 << 5) | 0x2;	// Addr28[1] = b'1，Addr28[6:5] = b'11，Addr28[8:7] = b'00
	mask = (0xf << 5) | 0x2;
	rf_update(28, val, mask);

	udelay(1000);

	while(k++ < 10)
	{
		val = rf_read(38);

		if(val & 0x1)
			break;
	}

	if(k >= 10)
	{
		//WLAN_DBG("%s(): calibration is not done\n", __FUNCTION__);
		ret = -1;
	}

	mask = 1 << 11;
	rf_update(1, 0, mask);

	return ret;
}

/*RF Set PLL : freq = 2412..., etc */
extern MT_DEVICE_SETTINGS_PANTHER mt_handle;
int lrf_set_pll(unsigned int freq)
{
#if 1
    //WLAN_DBG("Panther:%s(): start, freq = %d\n", __FUNCTION__, freq);
    return mt_Panther_RF_set_pll(&mt_handle, freq);
#else
    unsigned int flo = 0x00;
    unsigned int nf16 = 0x00;
    unsigned int tmp;
    //unsigned int counter=0;
    unsigned char rdiv = 1;   // Fref = 40/1;
    unsigned char channel_val = 0;
    int ret = 0;

    int manual_en = 1;
    int lobpf_bypass_en = 0;

    volatile u32 *ptr;

    flo = freq;
    channel_val = freq - 2400;
    nf16 = freq * rdiv;

    WLAN_DBG("%s(): start, freq = %d\n", __FUNCTION__, freq);

    // -------- reset pll calibration ----------------
    ptr = (u32*)0xC085C;	//SDIO_CTRL_REG
    *ptr = *ptr & (~(1 << 12));	// RF pd=0

    rf_update(0, 0x3, 0x3);		// xpd_vco,xpd_pll;
    rf_update(0, 1<<12, 1<<12);	// xpd_lodiv;
    rf_update(1, 1, 1);			// reg_ctrl_mode;
    rf_update(25, 1<<8, 1<<8);	// XTAL_EN_CTRL = 1
    rf_update(25, 1<<6, 1<<6);	// PLL_EN_CTRL = 1

    /*	pi_ci=<10>; fb_sel=<11>;lobpf=1; lobpf_ngm_en=1; 
	    vco_aac_cv=[10];vco_Kvco_tune=0;	lobpf_ci=[0010];	*/
    rf_write(2, 0x23496f0e);
    /*	lpf_cc1 = <00>; lpf_cc2=<111>; lpf_cr2=<101>; cpm_ci=[1111],cpa_ci=<1000>;
	    pll_clk_ten=pll_clk_tsel=pll_test_en=dll_test_en=lodiv_test_en=vco_test_en=0;
	    pll_afc_en =1;																	*/
    rf_write(3, 0x48ffc00);

    rf_update(25, ((channel_val & 0xff) << 14), (0xff << 14));	// set channel[7:0];

    // cae 01:
    tmp = 0x3 << 1;
    rf_update(7, tmp, tmp);			// dll_fix_sel=<11>;
    rf_update(7, 0, (1<<9));		// dll_cal_sel=0 ;

    if (manual_en)
    {
	    rf_write(5, 0xa334);

	    rf_update(6, (0x72 << 16), (0xff << 16));		// lobpf_start_ini_sel = 1;
	    rf_update(6, nf16, 0x3fff); 					// set NF16
	    rf_update(6, (rdiv << 14), (0x7 << 14)); 		// set RDIV


	    tmp = (1 << 24);
	    rf_update(7, 0, tmp);	// pll_soft_resetb=0;
	    rf_update(7, tmp, tmp);	// pll_soft_resetb=1;
    }
    else
    {
	    rf_write(5, 0xa331);	// pll_afc_initial_sel = 0
	    rf_update(6, (0x50 << 16), (0xff << 16));	// lobpf_start_ini_sel = 0;
    }

    if (lobpf_bypass_en)
    {
	    tmp = 0x3 << 16;
	    rf_update(7, tmp, tmp);		// {lobpf_initial_sel,lobpf_bypass_en}=[11];
	    tmp = 1 << 20;
	    rf_update(7, tmp, tmp);			// lobpf_start=1;
	    rf_update(2, 0, (0x3 << 13));	// lobpf_ngm_en=0; lobpf_en=0;
    }
    else
    {
	    rf_update(7, 0, (0x3 << 16));	// {lobpf_initial_sel,lobpf_bypass_en}=[00];
	    tmp = 1 << 20;
	    rf_update(7, tmp, tmp);			// lobpf_start=1;
	    tmp = 0x3 << 13;
	    rf_update(2, tmp, tmp);		// lobpf_ngm_en=1; lobpf_en=1;
    }

    tmp = 1 << 24;
    rf_update(7, 0, tmp);		// pll_soft_resetb=0;
    rf_update(7, tmp, tmp);		// pll_soft_resetb=1;
    rf_update(5, 0, 0x1);			// pll_afc_start = 1;
    rf_update(5, 1, 0x1); 		// pll_afc_start = 1;

#if defined(CONFIG_FPGA) && defined(RFC_I2C_TEST)
    rf_update(1, 0, 1);			// reg_ctrl_mode;
#endif

#if 0
    if (manual_en)
    {
	    /* {lobpf_initial_sel,lobpf_bypass_en}=[00]; set nf16; set rdiv; */
	    tmp = ((0x72) << 16) |	((rdiv & 0x7) << 14) | (nf16 & 0x3fff);
	    rf_update(6, tmp, 0xffffff);

	    rf_update(25, ((channel_val & 0xff) << 14), (0xff << 14));	// set channel[7:0];
	    rf_update(3, (0x88 << 16), (0xff << 16));	// set auxiliary.cp[3:0] and main.cp[3:0];

	    // -------- start pll calibration ----------------
	    rf_update(7, (1 << 24), (1 << 24));			// pll_soft_resetb = 1
	    rf_update(5, 1, 1);							// pll_afc_start = 1;

	    // -------- after pll calibration ----------------

	    while(!((tmp = rf_read(37)) & 0x4))
	    {
		    udelay(1000);

		    if(counter++ > 3)
		    {
			    ret = -1;		// Fail
			    break;
		    }
	    }

	    if(!ret)
	    {
		    WLAN_DBG("%s(): success\n", __FUNCTION__);
		    rf_update(3, 0, (1 << 26));			// set pll_afc_en = 0;
	    }
	    else
	    {
		    WLAN_DBG("%s(): fail\n", __FUNCTION__);
	    }
    }
#endif
    return ret;
#endif
}

int lrf_tx_on(void)
{
    rf_write(0, 0x59F2EB);
    rf_write(1, 0xB8000311);
//  rf_write(2, 0x7402FCC);
//  rf_write(3, 0xCC8D00);
//  rf_write(4, 0xCA);
//  rf_write(5, 0x4860512D);
//  rf_write(6, 0x80108240);
//  rf_write(7, 0x0);
    rf_write(8, 0x980000);
    rf_write(9, 0x46646361);
    rf_write(10, 0x84000A91);
    rf_write(11, 0x11000000);
    rf_write(12, 0x40004820);
#if (CONFIG_PANTHER_CHIP_VERSION >= 1)
    rf_write(13, 0x73495BBB);
    rf_write(14, 0x0073F108);
    rf_write(15, 0x0091AFAD);
    rf_write(16, 0x5E2296);
    rf_write(17, 0x2FC723C3);
    rf_write(18, 0xA001108C);
    rf_write(19, 0x7411BADB);
    rf_write(20, 0x9DC198);
    rf_write(21, 0x9A00689);
//  rf_write(22, 0x87478384);
//  rf_write(23, 0x7E0C);
    rf_write(24, 0xFC710000);
#else
    rf_write(13, 0x72491BBB);
    rf_write(14, 0x73EF09);
    rf_write(15, 0x918F9D);
    rf_write(16, 0x5E2296);
    rf_write(17, 0x130723C3);
    rf_write(18, 0xA001108C);
    rf_write(19, 0x7411BADB);
    rf_write(20, 0x9DC198);
    rf_write(21, 0x9A00689);
//  rf_write(22, 0x87478384);
//  rf_write(23, 0x7E0C);
    rf_write(24, 0x88810000);
#endif
//  rf_write(25, 0x0);
//  rf_write(26, 0x8);
//  rf_write(27, 0xA321);
//  rf_write(28, 0x2C4A214);
    rf_write(29, 0x3BC9);

    return 0;
}

int lrf_rx_on(void)
{
    rf_write(0, 0x59B30F);
    rf_write(1, 0x3000311);
    rf_write(2, 0x7402FCC);
    rf_write(3, 0xCC8D00);
//  rf_write(4, 0xCA);
//  rf_write(5, 0x4860512D);
//  rf_write(6, 0x80108240);
    rf_write(7, 0x0);
    rf_write(8, 0x980000);
    rf_write(9, 0x46646361);
    rf_write(10, 0x84000A91);
    rf_write(11, 0x11000000);
    rf_write(12, 0x40004820);
    rf_write(13, 0x72491BBB);
    rf_write(14, 0x73EF09);
    rf_write(15, 0x918F9D);
    rf_write(16, 0x5E2296);
    rf_write(17, 0x130723C3);
    rf_write(18, 0xA001108C);
    rf_write(19, 0x3211BADB);
    rf_write(20, 0x2620198);
    rf_write(21, 0x8040689);
//  rf_write(22, 0x87478384);
//  rf_write(23, 0x7E0C);
    rf_write(24, 0x88810000);
//  rf_write(25, 0x0);
//  rf_write(26, 0x8);
    rf_write(27, 0xA321);
    rf_write(28, 0x2C4A214);
    rf_write(29, 0x3BC8);

    return 0;
}

int lrf_bbdump_init(void)
{
    bb_dump_init_setting();

    return 0;
}

int lrf_bbdump_start(void)
{
#if 0
    unsigned long byte_count = 0x1000000;
    unsigned int buf = 0x82000000UL;
    unsigned int merge_buf = 0x83000000UL;
    char *fname = "bbdump";
    int ac = 1;
    unsigned int cur_addr = 0;

    // dump bb data into buf
    bb_dump_start();

    if (!is_dump_finish())
    {
	printf("bbdump isn't finish !!\n");
	goto leave;
    }

    if (is_bb_buffer_overflow())
    {
	printf("bbdump buffer overflow !!\n");
	goto leave;
    }

    cur_addr = get_bbdump_cur_addr();

    printf("filename: %s, buf: 0x%08x, len: 0x%x\n", fname, buf, byte_count);
    eth_open(0);
    // only have part1
    if (cur_addr == 0x2000000)
    {
	net_tftp(ac, buf, byte_count, fname);
    }
    else
    {
	//   | start addr | <--part 2--> | cur addr | <--part 1--> | end addr |
	memcpy(merge_buf, cur_addr | 0x80000000, (0x3000000 - cur_addr));  // part 1
	memcpy(merge_buf + (0x3000000 - cur_addr), 0x82000000, (cur_addr - 0x2000000));  // part 2
	net_tftp(ac, merge_buf, byte_count, fname);
    }
#else
    unsigned long byte_count = 0x400000;   // 4M
    unsigned int buf = 0x82000000UL;
    char *fname = "bbdump";
    int ac = 1;

    // dump bb data into buf
    bb_dump_start();

    if (!is_dump_finish())
    {
	printf("bbdump isn't finish !!\n");
	goto leave;
    }

    if (is_bb_buffer_overflow())
    {
	printf("bbdump buffer overflow !!\n");
	goto leave;
    }

    printf("filename: %s, buf: 0x%08x, len: 0x%x\n", fname, buf, byte_count);
    eth_open(0);
    net_tftp(ac, buf, byte_count, fname);
#endif

leave:
    return 0;
}

int lrf_rfc_dump(void)
{
    lrf_bbdump_init();

    bb_register_write(0, 0xc2, 0x1);
    bb_register_write(0, 0xc0, 0xe);

    lrf_bbdump_start();

    return 0;
}

int lrf_channel_pll_cal(unsigned int flo)
{
	int i;
	int check_count=0;
//      volatile u32 *ptr;
	unsigned int tmp;
	//unsigned int flo = 0x00;
	unsigned char channel_val;
	int start_ch, end_ch, ch_idx;
#if 0	/* timing measure */
	int t_val = 0;
#endif					
	// step0: Initial Setting （the setting as following will do one time by soft ware after power on for chip)

	if((flo) && ((ch_idx = lrf_freq2idx(flo)) <= 13))
	{
		start_ch = ch_idx;
		end_ch = ch_idx;
	}
	else
	{
		start_ch = 0;
		end_ch = 13;
	}

//      ptr = (u32*)0xC085C;			//SDIO_CTRL_REG
//      *ptr = *ptr & (~(1 << 12));		// Reg085C[12]=0(RF_pd = 0)

	tmp = 0x3 << 6;
	rf_update(4, tmp, tmp);		// Edward: turn on TX/RX LO buffer by RF team

	rf_update(0, 0x3, 0x3);		// xpd_vco,xpd_pll;
	tmp = 1 << 12;
	rf_update(0, tmp, tmp);		// xpd_lodiv;
	rf_update(1, 1, 1);			// reg_ctrl_mode;
	tmp = 1 << 8;
	rf_update(25, tmp, tmp);	// XTAL_EN_CTRL = 1
	tmp = 1 << 6;
	rf_update(25, tmp, tmp);	// PLL_EN_CTRL = 1
	/*	pi_ci=<10>; fb_sel=<11>;lobpf=1; lobpf_ngm_en=1; 
		vco_aac_cv=[10];vco_Kvco_tune=0;	lobpf_ci=[0010];	*/
	rf_write(2, 0x23496f0e);
	/*	lpf_cc1 = <00>; lpf_cc2=<111>; lpf_cr2=<101>; cpm_ci=[1111],cpa_ci=<1000>;
		pll_clk_ten=pll_clk_tsel=pll_test_en=dll_test_en=lodiv_test_en=vco_test_en=0;
		pll_afc_en =1;																	*/
	rf_write(3, 0x48ffc00);
	tmp = 0x3 << 1;
	rf_update(7, tmp, tmp);			// dll_fix_sel=<11>;
	rf_update(7, 0, (1<<9));		// dll_cal_sel=0 ;
	rf_write(5, 0xa32c);			// manual
	rf_update(6, (0x72 << 16), (0xff << 16));		// 0x72 for manual;
	rf_update(7, 0, (0x3 << 16));		// {lobpf_initial_sel,lobpf_bypass_en}=[00];
	tmp = 0x1 << 20;
	rf_update(7, tmp, tmp);			// lobpf_start=1;
	tmp = (0x3 << 13);
	rf_update(2, tmp, tmp);			// lobpf_ngm_en=1; lobpf_en=1;
	tmp = (1 << 24);
	rf_update(7, 0, tmp);			// pll_soft_resetb=0;
	rf_update(7, tmp, tmp);			// pll_soft_resetb=1;
						
	// step1: scan 14 channels and save calibration result PLL_data_out[47:0] (by MCU)
	for(i=start_ch; i <= end_ch; i++)
	{
		// step1.1 set frequency channel
		flo = lrf_channel_data[i].freq;
		channel_val = flo - 2400;
			
		//WLAN_DBG("%s(): flo=%d, start_ch=%d, end_ch=%d\n", __FUNCTION__, flo, start_ch, end_ch);

		rf_update(26, 0x8, 0x8);		// pll_sch_en_dsp = 1;
#if 0 /* timing measure */
		CLK_MEASURE_START;
#endif
		rf_update(5, 0, 0x1);			// pll_afc_start = 0;
		rf_update(25, ((channel_val & 0xff) << 14), (0xff << 14));	// set channel[7:0];
		rf_update(6, flo, 0x3fff); 		// set NF16
		rf_update(5, 0x1, 0x1);			// pll_afc_start = 1;

		//udelay(1000);	// delay 1 ms /* remove it measure actual calibration time */

		check_count = 0;

		while(check_count++ < 1000000)
		{
			tmp = rf_read(32);
			if(tmp & 0x10000)
			{
				break;
			}
		}
#if 0 /* timing measure */
		CLK_MEASURE_END(t_val);
#endif
		if(check_count >= 1000000)
		{
			WLAN_DBG("%s(): freq=%d, chan=%d, pll cal fail !!\n", __FUNCTION__, flo, i+1);
		}
#if 0 /* timing measure */
		WLAN_DBG("%s(): measures calibrate chan[%d] time:%d\n", __FUNCTION__, i+1, (t_val<<2)*7);
#endif
		pll_record[i].msb = rf_read(32) & 0xffff;
		pll_record[i].lsb = rf_read(31);
	}

	rf_update(1, 0, 0x1);			// reg_ctrl_mode: normal mode;

#if 0
	for(i=0; i<14; i++)
	{
		WLAN_DBG("%s(): chan=%d, pll data(reg.0x32 : reg.0x31) = {0x%04x : 0x%08x}!!\n", 
				__FUNCTION__, i+1, pll_record[i].msb, pll_record[i].lsb);
	}
#endif
        return 0;
}

int lrf_set_freq(unsigned int flo)
{
	/* flo = frequency (ex: 2442) */
	int idx;
	//volatile u32 *ptr;
	unsigned int tmp;
	unsigned char channel_val;

#if 0	/* timing measure */
	int t_val = 0;
	CLK_MEASURE_START;
#endif
	idx = lrf_freq2idx(flo);
	if(idx > 14)
		idx = 0;
	channel_val = flo - 2400;
	
	tmp = 0x3 << 6;
	rf_update(4, tmp, tmp);		// Edward: turn on TX/RX LO buffer by RF team

	tmp = 1 << 3;
	rf_update(26, tmp, tmp);
	rf_update(5, 0, 0x1);			// pll_afc_start = 0;
	rf_update(25, ((channel_val & 0xff) << 14), (0xff << 14));	// set channel[7:0];
	rf_update(6, flo, 0x3fff); 		// set NF16

	rf_update(23, pll_record[idx].msb, 0xffff);
	rf_write(22, pll_record[idx].lsb);
	
	rf_update(5, 0x1, 0x1);			// pll_afc_start = 1;
	
#if 0	/* timing measure */
	CLK_MEASURE_END(t_val);
	WLAN_DBG("%s(): measures switch chan time:%d\n", __FUNCTION__, (t_val<<2)*7);
#endif
        return 0;
}

int lrf_voltage_temperature(int delay_vot, int delay_reg9, int debug)
{
	/* Note: refer to the file: Temp sensor for Lynx_A1_0505b.docx */

	unsigned int shift9, shift7_6;
	unsigned int tmp;
	//int dbg_level = RFC_DBG_INFO;
	unsigned int vout, vout1, vout2;
	unsigned int voltage, temper;

	shift7_6 = (0x3 << 6);
	shift9 = (1 << 9);

	/* init */
	rf_update(0, 0, (1 << 13));
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 0, rf_read(0));
	tmp = 1 << 21;
	rf_update(0, tmp, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 0, rf_read(0));
	tmp = 1 << 6;
	rf_update(1, tmp, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 1, rf_read(1));
	udelay(delay_vot);
#if (CONFIG_ROM_VER > 1)
	rf_update(24, 0, (1 << 9));
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
#endif
	rf_update(24, 0, (1 << 8));
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	rf_update(3, 0, (0x7 << 27));
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 3, rf_read(3));
#if (CONFIG_ROM_VER > 1)
	tmp = 1 << 30;
	rf_update(27, tmp, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
	udelay(delay_reg9);
	rf_update(27, 0, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
#else
	rf_update(24, shift9, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	udelay(delay_reg9);
	rf_update(24, 0, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
#endif
	udelay(delay_vot);

	/* get vout1 */
	rf_update(24, (2 << 6), shift7_6);

	//time1 = micros();
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	//time2 = micros();
	//RFC_DBG(RFC_DBG_ERR, "[RFC_TIME] %s(%d): time1=%d, time2=%d\n", __FUNCTION__, __LINE__, time1, time2);
	vout1 = rf_read(36) & 0x7FF;
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 36, rf_read(36));
#if (CONFIG_ROM_VER > 1)
	tmp = 1 << 30;
	rf_update(27, tmp, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
	udelay(delay_reg9);
	rf_update(27, 0, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
#else
	rf_update(24, shift9, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	udelay(delay_reg9);
	rf_update(24, 0, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
#endif
	//WLAN_DBG("%s(): vout1=%d\n", __FUNCTION__, vout1);
	udelay(delay_vot);

	/* get vout2 */
	rf_update(24, (3 << 6), shift7_6);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	vout2 = rf_read(36) & 0x7FF;
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 36, rf_read(36));
#if (CONFIG_ROM_VER > 1)
	tmp = 1 << 30;
	rf_update(27, tmp, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
	udelay(delay_reg9);
	rf_update(27, 0, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
#else
	rf_update(24, shift9, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	udelay(delay_reg9);
	rf_update(24, 0, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
#endif
	//WLAN_DBG("%s(): vout2=%d\n", __FUNCTION__, vout2);
	udelay(delay_vot);

	/* get vout */
	rf_update(24, 0, shift7_6);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	rf_update(24, 0, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	vout = rf_read(36) & 0x7FF;
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 36, rf_read(36));
#if (CONFIG_ROM_VER > 1)
	tmp = 1 << 30;
	rf_update(27, tmp, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
	udelay(delay_reg9);
	rf_update(27, 0, tmp);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 27, rf_read(27));
#else
	rf_update(24, shift9, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
	udelay(delay_reg9);
	rf_update(24, 0, shift9);
	if(debug)
		WLAN_DBG("rf reg.%d = 0x%x\n", 24, rf_read(24));
#endif
	//WLAN_DBG("%s(): vout=%d\n", __FUNCTION__, vout);

	voltage = ((vout - vout1 + 1024)*1000000) / (vout2 - vout1 + 1024);
	temper = (voltage - 892200)/3366;

	WLAN_DBG("%s(): vout=%d, vout1=%d, vout2=%d, voltage(10^6)=%d, temper=%d\n", __FUNCTION__, vout, vout1, vout2, voltage, temper);

	return temper;
}

void config_rfc_parm(int parm, int freq)
{
	struct rfc_cal_reg *reg;
	int bw=0;
	//u8 reg5a;

	/* FIXME: panther not cal 40MHz */
	//bw = parm & 0x1;

	reg = &rfc_result_ptr[bw];

	if(!reg->is_failed)
	{
		/* txlo */
		bb_register_write(0, 0x25, reg->tx_dc_i);
		bb_register_write(0, 0x30, reg->tx_dc_q);

		bb_register_write(0, 0x20, reg->balancer_nm);
//      	bb_register_write(0, 0x21, reg->tx_a11);
//      	bb_register_write(0, 0x22, reg->tx_a12);
		bb_register_write(0, 0x23, reg->tx_a21);
		bb_register_write(0, 0x24, reg->tx_a22);
		bb_register_write(0, 0x2a, reg->rx_dc_i);
		bb_register_write(0, 0x31, reg->rx_dc_q);
		bb_register_write(0, 0x33, reg->a21a22_ext);
		bb_register_write(0, 0x5b, reg->rolloff_rx_coe);
		bb_register_write(0, 0x5c, reg->phaseshifter_rx_alfa);
		bb_register_write(0, 0x5d, reg->rolloff_tx_coe);
		bb_register_write(0, 0x5e, reg->phaseshifter_tx_alfa);
		
		/* rx */
		bb_register_write(0, 0x28, reg->rx_a21);
		bb_register_write(0, 0x29, reg->rx_a22);
		//bb_register_write(0x5a, reg->filter_switch);
	}
	else
	{
		WLAN_DBG("[RFC_DBG] %s(): restore_rfc_default_value !!!!!!!!\n", __FUNCTION__);
		bb_register_write(0, 0x20, 0x0);
//      	bb_register_write(0, 0x21, 0x7f);
//      	bb_register_write(0, 0x22, 0x0);
		bb_register_write(0, 0x23, 0x0);
		bb_register_write(0, 0x24, 0x7f);
		bb_register_write(0, 0x25, 0x0);
//      	bb_register_write(0, 0x26, 0x7f);
//      	bb_register_write(0, 0x27, 0x0);
		bb_register_write(0, 0x28, 0x0);
		bb_register_write(0, 0x29, 0x7f);
	}

#if 0
	BBDBG(0x20);
	BBDBG(0x21);
	BBDBG(0x22);
	BBDBG(0x23);
	BBDBG(0x24);
	BBDBG(0x25);
	BBDBG(0x28);
	BBDBG(0x29);
	BBDBG(0x30);
	BBDBG(0x31);
	BBDBG(0x33);
	BBDBG(0x5a);
	BBDBG(0x5b);
	BBDBG(0x5c);
	BBDBG(0x5d);
	BBDBG(0x5e);
#endif
}

void set_bb_vgagain(unsigned int input)
{
	/*	input : 5 bits data
		bb reg.3 :
		%Bit7 MSB=Force LNA VGA to fix gain mode
		%Bit[6:5] =LNA gain
		%Bit[4:0] =VGA gain
	*/
	unsigned char value;
	
	value = bb_register_read(0, 0x3);
	value &= 0x60;
	value |= (0x80 | (input & 0x1f));

	bb_register_write(0, 3, value);
}

void set_dcoc_dac_ctrl(int not_manual)
{
	unsigned int value = 0;

	if(not_manual)
		value = 0x2;

	/* rf reg.26 bit.1 : 	1 = Normal operation Mode, 
							0 = Manual mode during Calibration */
	rf_update(26, value, 0x2);	// Normal operation Mode
}

void set_dcoc_cal_done(int is_done)
{
	/* rf reg.26 bit.0 : 	1 = This VGA is Done, 
							0 = Under Calibration 	*/

	is_done = !!is_done;	// avoid that is_done > 1

	rf_update(26, is_done, 0x1);
}

int config_rxdc(unsigned int *rec)
{
	// vga calibration range
	int vga_min = 5;
	int vga_max = 26;
	int vga;
	int idx = 0;

	// set 0 to start
	set_dcoc_dac_ctrl(0);
	
	// start DCOC calibration
	for(vga = vga_min;  vga <= vga_max; vga++)
	{
		set_bb_vgagain(vga);
		set_dcoc_cal_done(0);

		/* FIXME: */
		rf_write(11, rec[idx++]);

		set_dcoc_cal_done(1);       
	}

	// set 1 to quit DCOC calibration
	set_dcoc_dac_ctrl(1);

	return 0;
}

CMD_DECL(panther_rf_test_cmd)
{
	char *str;

#if 0
	WLAN_DBG("%s(): argc=%d\n", __FUNCTION__, argc);
	for(i=0; i<argc; i++)
		WLAN_DBG("    argv[%d]=%s\n", i, argv[i]);
#endif
	if(argc < 1)
	{
	 	WLAN_DBG("lrf set_pll <channel: 1~14>\n");
		WLAN_DBG("    tx_on\n");
		WLAN_DBG("    rx_on\n");
		WLAN_DBG("    bbdump_init\n");
		WLAN_DBG("    bbdump_start\n");
		WLAN_DBG("    rfc_dump\n");
		WLAN_DBG("    btx <channel> <pkt_num>, for beacon tx test\n");
		WLAN_DBG("    utx <channel> <pkt_num>, for unicast tx test\n");
		return 0;
	}

	str = argv[0];

	if(!strcmp(str, "set_pll"))
	{
		int channel=1;

		if(argc < 2)
			return 1;
		
		channel = simple_strtol(argv[1], NULL, 10);

		lrf_set_pll(lrf_channel_data[channel-1].freq);
	}
	else if(!strcmp(str, "tx_on"))
	{
		lrf_tx_on();
	}
	else if(!strcmp(str, "rx_on"))
	{
		lrf_rx_on();
	}
	else if(!strcmp(str, "bbdump_init"))
	{
	        lrf_bbdump_init();
	}
	else if(!strcmp(str, "bbdump_start"))
	{
		lrf_bbdump_start();
	}
	else if(!strcmp(str, "rfc_dump"))
	{
		lrf_rfc_dump();
	}
	else if(!strcmp(str, "btx"))
	{
		int pkt_num = 1;
		int channel = 1;

		if(argc >= 3)
		    pkt_num = simple_strtol(argv[2], NULL, 10);

		if(argc >= 2)
		    channel = simple_strtol(argv[1], NULL, 10);

		beacon_tx(channel, pkt_num);
	}
	else if(!strcmp(str, "utx"))
	{
		int pkt_num = 1;
		int channel = 1;

		if(argc >= 3)
		    pkt_num = simple_strtol(argv[2], NULL, 10);

		if(argc >= 2)
		    channel = simple_strtol(argv[1], NULL, 10);

		unicast_tx(channel, pkt_num);
	}
	else
	{
		WLAN_DBG("%s(): error command %s\n", __FUNCTION__, argv[1]);
	}

	return 0;
}

CMD_DECL(bb_dump_cmd)
{
#if 0
    unsigned long byte_count = 0x1000000;
    unsigned int buf = 0x82000000UL;
    unsigned int merge_buf = 0x83000000UL;
    char *fname = "bbdump";
    int ac = 1;
    unsigned int cur_addr = 0;

    if (argc >= 1)
	fname = argv[0];

    // dump bb data into buf
    bb_dump();

    if (!is_dump_finish())
    {
	printf("bbdump isn't finish !!\n");
	goto leave;
    }

    if (is_bb_buffer_overflow())
    {
	printf("bbdump buffer overflow !!\n");
	goto leave;
    }

    cur_addr = get_bbdump_cur_addr();

    printf("filename: %s, buf: 0x%08x, len: 0x%x\n", fname, buf, byte_count);
    eth_open(0);
    // only have part1
    if (cur_addr == 0x2000000)
    {
	net_tftp(ac, buf, byte_count, fname);
    }
    else
    {
	//   | start addr | <--part 2--> | cur addr | <--part 1--> | end addr |
	memcpy(merge_buf, cur_addr | 0x80000000, (0x3000000 - cur_addr));  // part 1
	memcpy(merge_buf + (0x3000000 - cur_addr), 0x82000000, (cur_addr - 0x2000000));  // part 2
	net_tftp(ac, merge_buf, byte_count, fname);
    }
#else
    unsigned long byte_count = 0x3800000;
    unsigned int buf = 0x80800000UL;
    char *fname = "bbdump";
    int ac = 1;

    if (argc >= 1)
	fname = argv[0];

    // dump bb data into buf
    bb_dump();

    if (!is_dump_finish())
    {
	printf("bbdump isn't finish !!\n");
	goto leave;
    }

    if (is_bb_buffer_overflow())
    {
	printf("bbdump buffer overflow !!\n");
	goto leave;
    }

    printf("filename: %s, buf: 0x%08x, len: 0x%x\n", fname, buf, byte_count);
    eth_open(0);
    net_tftp(ac, buf, byte_count, fname);
#endif

leave:
    return 0;
}

#if 0
cmdt cmdt_lrf __attribute__ ((section("cmdt"))) =
	{"lrf", lrf_test_cmd, "lrf cmd\n"};
#else
//shell_cmd("lrf", "lrf <argv>; panther rf test", "", panther_rf_test_cmd);
//shell_cmd("bbdump", "bbdump <argv>; dump bb mem data", "", bb_dump_cmd);
CLI_CMD(lrf, panther_rf_test_cmd, "lrf <argv>; panther rf test");
CLI_CMD(bbdump, bb_dump_cmd, "bbdump <argv>; dump bb mem data");
#endif

#if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
struct seq_file *sf;
static char buf[300];
#define MAX_ARGV_NUM 8
static int panther_rf_write(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    memset(buf, 0, 300);

    if (count > 0 && count < 299)
    {
        if (copy_from_user(buf, buffer, count))
            return -EFAULT;
        buf[count-1]='\0';
    }

    return count;
}

extern int get_args (const char *string, char *argvs[]);
static int panther_rf_show(struct seq_file *s, void *priv)
{
    int rc;
    int argc ;
    char *argv[MAX_ARGV_NUM] ;

    //sc->seq_file = s;
    sf = s;
    argc = get_args((const char *)buf, argv);
	rc = panther_rf_test_cmd(argc, argv);
    //sc->seq_file = NULL;
    sf = NULL;

    return 0;
}

static int panther_rf_open(struct inode *inode, struct file *file)
{
    int ret;

    ret = single_open(file, panther_rf_show, NULL);

    return ret;
}

static const struct file_operations lrf_fops = {
	.open       = panther_rf_open,
	.read       = seq_read,
	.write      = panther_rf_write,
	.llseek     = seq_lseek,
	.release    = single_release,
};

struct idb_command idb_panther_rf_test_cmd =
{
    .cmdline = "lrf",
    .help_msg = "lrf                         RF command list", 
    .func = panther_rf_test_cmd,
};
#endif

int lrf_test_init(void)
{
#if defined(CONFIG_PANTHER_INTERNAL_DEBUGGER)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
    struct proc_dir_entry *res;
#endif

    register_idb_command(&idb_panther_rf_test_cmd);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
    if (!proc_create("lrf", S_IWUSR | S_IRUGO, NULL, &lrf_fops))
        return -EIO;

#else
    res = create_proc_entry("lrf", S_IWUSR | S_IRUGO, NULL);
    if (!res)
        return -ENOMEM;

    res->proc_fops = &lrf_fops;
#endif
#endif

    return 0;
}

void panther_rf_init(void)
{
    //int i = 0;
    /* Turn on RF power */
//  PMUREG_UPDATE32(0x60, 0x03800000UL, 0x03800000UL);
//  PMUREG(STDBY_PD_CTRL) &= ~STDBY_RF_PD;

//  printf("panther_rf_txrx_on()\n");
    panther_rf_txrx_on();
//  udelay(10000000);
//  printf("========================================\n");
//  for(i = 0; i < 40; i++)
//  {
//      printf("RF reg %d = 0x%x\n", i, *(volatile unsigned int*)(0xbf004f00 + 4 * i));
//  }

//  printf("mt_Panther_Init()\n");
    mt_Panther_Init(&mt_handle);
//  udelay(10000000);
//  for(i = 0; i < 40; i++)
//  {
//      printf("RF reg %d = 0x%x\n", i, *(volatile unsigned int*)(0xbf004f00 + 4 * i));
//  }

//  printf("mt_Panther_set_ABB()\n");
    mt_Panther_set_ABB(&mt_handle);
//  for(i = 0; i < 40; i++)
//  {
//      WLAN_DBG("RF reg %d = 0x%x\n", i, *(volatile unsigned int*)(0xbf004f00 + 4 * i));
//  }

    //lrf_test_init();
    lrf_set_pll(2412);      // set to channel 1

#if 1   // because default value is not 0x7f, so we modfiy default value here
    rfc_result_ptr[0].tx_a22 = 0x7f;
    rfc_result_ptr[0].rx_a22 = 0x7f;
#endif

//#if defined(CONFIG_FPGA) && defined(RFC_I2C_TEST)
//        lrf_set_pll(2412);      // set to channel 1
//#else
//  lrf_channel_pll_cal(0);
//  lrf_set_freq(2412);     // channel 1
//#endif

//  lrf_lpf_cal();  // filter calibration
    WLAN_DBG("%s(): bb reg 0x2 = 0x%x\n", __FUNCTION__, bb_register_read(0, 2));
}
