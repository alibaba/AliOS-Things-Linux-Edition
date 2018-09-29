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
#include <panther_dev.h>
#include <panther_debug.h>
#include <panther_rf.h>
#include <panther_rfac.h>
#include <rfc_comm.h>
#include <rf.h>
#include <bb.h>
#include <rfc_panther.h>
#include <os_compat.h>

#include "mac_regs.h"

#define ADC_CTRL                       0x10
       #define DAC_DIVSEL_80M                  0x00000008
       #define ADC_DIVSEL_240M                 0x00000004

int printf(char *fmt, ...);

struct rf_tbl {
	unsigned int address;
	unsigned int val;
	unsigned int mask;
};

extern int is_rxloop_panther;
extern int is_enable_rf12_rw;

#ifdef APPLY_LYNX_TX_LOOPBACK
struct rf_tbl tx_loopback_regs[] = {
	{0, 0x0051F0EF, 0x0},
	{1, 0xBC000311, 0x0},
#if (CONFIG_ROM_VER > 1)
	{8, 0x00b80000, 0x0},
#else
	{8, 0x00F80000, 0x0},
#endif
	{9, 0x26646361, 0x0},
#if (CONFIG_ROM_VER > 1)
	{10, 0x84200A91, 0x0},
#else
	{10, 0x80200A91, 0x0},
#endif
	{11, 0x11000000, 0x0},
	{12, 0x40004820, 0x0},
#if (CONFIG_ROM_VER > 1)
	{13, 0x72491BBB, 0},
#else
	{13, 0x7249DAB5, 0x0},
#endif
#if (CONFIG_ROM_VER > 1)
	{14, 0x73EF09, 0},
#else
	{14, 0x0073F109, 0x0},
#endif
#if (CONFIG_ROM_VER > 1)
	{15, 0x10911F9D, 0},
#else
	{15, 0x00911F9D, 0x0},
#endif
	{16, 0x005E2296, 0x0},
	{17, 0x000723C3, 0x0},
	{18, 0x2400110C, 0x0},	// for Sam's request
	{19, 0x2211BADB, 0x0},
	{20, 0x02620198, 0x0},
	{21, 0x00040689, 0x0},
	{24, 0x00010000, 0x0},
	{26, 0x00000000, 0x3},
	{28, 0x02C4A214, 0x0},
	{29, 0x00003BC9, 0x0},
};
#else
struct rf_tbl tx_loopback_regs[] = {
	{0, 0x0051E0EF, 0x0},
	{1, 0xBC000311, 0x0},
	{8, 0x0, 0x00000180},
	{10, 0x00200000, 0x00200000},
	{18, 0xA08110CC, 0xFF800000},
	{19, 0x7411BADB, 0x0},
	{29, 0x00003BC9, 0x0},
};
#endif

struct rf_tbl rx_loopback_regs[] = {
	{0, 0x41F22F, 0x0},
	{1, 0xBA004311, 0x0},
	{4, 0xc0, 0xc0},
#if (CONFIG_ROM_VER > 1)
	{8, 0xb80000, 0x0},
#else
	{8, 0xF80000, 0x0},
#endif
	{9, 0x26646361, 0x0},
#if (CONFIG_ROM_VER > 1)
	{10, 0x84000A91, 0x0},
#else
	{10, 0x80000A91, 0x0},
#endif
	{11, 0x11000000, 0x0},
	{12, 0x40004820, 0x0},
	{13, 0x7249DAB1, 0x0},
	{14, 0x73F10A, 0x0},
	{15, 0x911F9D, 0x0},
	{16, 0x5E2296, 0x0},
	{17, 0x723C3, 0x0},
	{18, 0x2400110C, 0x0},
	{19, 0x2211BADB, 0x0},
	{20, 0x2620198, 0x0},
	{21, 0x40689, 0x0},
	{24, 0x10000, 0x0},
	{26, 0x00000000, 0x3},
	{28, 0x2C4A214, 0x0},
	{29, 0x3BC9, 0x0},
};

struct rf_tbl lynx_rx_loopback_after_tx_loopback_regs[] = {
	{0, 0x0051F20F, 0x0},
	{1, 0xAB000311, 0x0},
	{10, 0, 0x200000},
	{29, 0x3BC8, 0x0},
	{16, 0x015E2296, 0x0},
};

struct rf_tbl panther_rx_loopback_after_tx_loopback_regs[] = {
	{0, 0x0041F22F, 0x0},
	{1, 0xBA004311, 0x0},
	{8, 0x00000180, 0x00000180},
	{10, 0x0, 0x00200000},
	{18, 0xA08110CC, 0xFF800000},
	{19, 0x7411BADB, 0x0},
	{29, 0x00003BC9, 0x0},
};

int lrf_k_pll_set_ch(int freq, int bw)
{
	unsigned int mask= (1 << 23);

	lrf_channel_pll_cal(freq);
	rf_update(1, 0x1, 0x1);		// reg_ctrl_mode: manual mode

	lrf_set_freq(freq);	// set to channel 1

	if(bw == 0)  // do it in 20 MHz only
	{
		rf_update(10, 0, mask);
		PLLREG(ADC_CTRL) |= (DAC_DIVSEL_80M|ADC_DIVSEL_240M);
	}
	else
	{
		rf_update(10, mask, mask);
		PLLREG(ADC_CTRL) &= ~(DAC_DIVSEL_80M|ADC_DIVSEL_240M);
	}

	return 0;
}

#define TXVGA_SIZE	6
int set_txcal_txvga(int gain)
{
	int idx = 0;
#ifdef APPLY_LYNX_TXVGA_TABLE
	int gain_tbl[TXVGA_SIZE] = {3, 0, -3, -6, -9, -12};
	unsigned char gsel_txmod_tbl[TXVGA_SIZE] = {0x4, 0x3, 0x2, 0x1, 0x1, 0x1};
	unsigned char tx_lpf_cg_tbl[TXVGA_SIZE] = {0x4, 0x4, 0x4, 0x4, 0x3, 0x2};
#else
	int gain_tbl[TXVGA_SIZE] = {3, 0, -3, -6, -9, -12};
	unsigned char gsel_txmod_tbl[TXVGA_SIZE] = {0x5, 0x4, 0x3, 0x2, 0x1, 0x1};
	unsigned char tx_lpf_cg_tbl[TXVGA_SIZE] = {0x3, 0x3, 0x3, 0x3, 0x3, 0x2};
#endif

	/*	rf team says: 
		GSEL_TXMOD[2:0] is after mixer gain , so it won't have IQ imbalance
		issue,
		tx_lpg_cg[2:0]  is bb low pass filter gain before mixer, it have both IQ
		path. We have no choice to set it fix during all calibration,
		It will limit TX VGA = -6:3: 3 , -9 -12 are illegal settings
	*/

	for(idx=0; idx<TXVGA_SIZE; idx++)
	{
		if(gain_tbl[idx] <= gain)
			break;
	}

#ifndef APPLY_LYNX_TXVGA_TABLE
	if(idx == 1)
	{
		RFC_DBG(RFC_DBG_INFO, "set txcal txvga with gain = 0, but gain 0 are using wrong dB value now\n");
	}
#endif

	if(idx >= TXVGA_SIZE)
		idx = TXVGA_SIZE-1;

	/*
		write  rf  addr18[8:6]   GSEL_TXMOD[2:0]
		write  rf  addr9[30:28]  tx_lpf_cg[2:0]
	*/

	RFC_DBG(RFC_DBG_INFO, "%s(): idx=%d, gain=%d, gsel=0x%x, tx_lpf=0x%x\n", __FUNCTION__, idx, gain, gsel_txmod_tbl[idx], tx_lpf_cg_tbl[idx]);

	rf_update(18, (gsel_txmod_tbl[idx] << 6), (0x7 << 6));
	rf_update(9, (tx_lpf_cg_tbl[idx] << 28), (0x7 << 28));

	return 0;
}

int read_txcal_txvga(void)
{
	int idx = 0;
	int gain;
#ifdef APPLY_LYNX_TXVGA_TABLE
	int gain_tbl[4] = {3, 0, -3, -6};
	unsigned char gsel_txmod_tbl[4] = {0x4, 0x3, 0x2, 0x1};
	unsigned char tx_lpf_cg_tbl[4] = {0x4, 0x4, 0x4, 0x4};
#else
	int gain_tbl[4] = {3, 0, -3, -6};
	unsigned char gsel_txmod_tbl[4] = {0x5, 0x4, 0x3, 0x2};
	unsigned char tx_lpf_cg_tbl[4] = {0x3, 0x3, 0x3, 0x3};
#endif
	unsigned int reg18, reg9;

	reg18 = rf_read(18);
	reg9 = rf_read(9);

	reg18 = (reg18 >> 6) & 0x7;
	reg9 = (reg9 >> 28) & 0x7;

	for(idx=0; idx<4; idx++)
	{
		if((reg18 == gsel_txmod_tbl[idx]) && (reg9 == tx_lpf_cg_tbl[idx]))
			break;
	}

	if(idx >= 4)
		gain = -127;
	else
		gain = gain_tbl[idx];

	return gain;
}

int set_txcal_rxvga(int gain)
{
	unsigned char rxgain_code;
	rf_update(9, (0x5 << 6), (0x7 << 6));

	if(gain <= -3)
		rxgain_code = 0x0;
	else if(gain <= 0)
		rxgain_code = 0x1;
	else if(gain <= 3)
		rxgain_code = 0x2;
	else if(gain <= 6)
		rxgain_code = 0x3;
	else if(gain <= 9)
		rxgain_code = 0x4;
	else if(gain <= 12)
		rxgain_code = 0x5;
	else // if(gain <= 15) & others
		rxgain_code = 0x6;

	rf_update(9, (rxgain_code << 9), (0x7 << 9));	// reg.9 bit.[11:9]

	return 0;
}

int set_rxcal_rxvga(int gain)
{
	/*
		data[] = {bbpga1_cri_ext[2:0], bbpga1_crf_ext[2:0], bbfilter_R_sel_ext[3:0],
		 			bbfilter_r_sel_ext[3:0], bbpga2_cri_ext[4:0], bbpga2_crf_ext[1:0]}
		bbpga1_cri_ext[2:0]	=>	addr9[8:6]
		bbpga1_crf_ext[2:0]	=>	addr9[11:9]
		bbfilter_R_sel_ext[3:0]	=>	addr9[23:20]
		bbfilter_r_sel_ext[3:0]	=>	addr9[27:24]
		bbpga2_cri_ext[4:0]	=>	addr10[12:8]
		bbpga2_crf_ext[1:0]	=>	addr10[14:13]
		RX_AGC_EN	=>	addr0[30]
		RXFE_RFGAIN[1:0]	=>	addr16[24:23]
	*/
	unsigned char data[16][6] = {
				{5, 1, 0, 0, 5, 0},
				{5, 1, 1, 0, 5, 0},
				{5, 1, 2, 0, 5, 0},
				{5, 1, 3, 0, 5, 0},
				{5, 1, 3, 1, 5, 0},
				{5, 1, 3, 2, 5, 0},
				{5, 1, 3, 3, 5, 0},
				{5, 1, 4, 3, 5, 0},
				{5, 1, 5, 3, 5, 0},
				{5, 3, 3, 3, 5, 0},
				{5, 3, 4, 3, 5, 0},
				{5, 3, 5, 3, 5, 0},
				{5, 3, 6, 3, 5, 0},
				{5, 3, 6, 4, 5, 0},
				{5, 3, 6, 5, 5, 0},
				{5, 3, 6, 6, 5, 0},
			};
	unsigned int val, mask;

	if(gain <= 0)
		gain = 0;
#if 0		// not sure whether panther is needed or not, date: 08/22
	else if(gain >= 20)
		gain = 10;
#else
	else if(gain >= 30)
		gain = 15;	// 30 >> 1
#endif
	else
		gain = gain >> 1;

	mask = 0x0FF00FC0;
	val = (data[gain][0] << 6) | (data[gain][1] << 9) | (data[gain][2] << 20) | (data[gain][3] << 24);
	rf_update(9, val, mask);
	
	mask = 0x7F00;
	val = (data[gain][4] << 8) | (data[gain][5] << 13);
	rf_update(10, val, mask);

	rf_update(0, 0, (1 << 30));
#if 0
	serial_printf("array[%d] = %d, %d, %d, %d, %d, %d\n", gain, data[gain][0], data[gain][1], data[gain][2], data[gain][3], data[gain][4], data[gain][5]);
	serial_printf("rf reg.9=0x%x, reg.10=0x%x, reg.0=0x%x\n", rf_read(9), rf_read(10), rf_read(0));
#endif	

    return 0;
}

int read_txcal_rx_gain(void) // return gain;
{
	int gain=0;
	unsigned int rxgain_code = ((rf_read(9) >> 9) & 0x7);

	if(rxgain_code == 0x0)
		gain = -3;
	else if(rxgain_code == 0x1)
		gain = 0;
	else if(rxgain_code == 0x2)
		gain = 3;
	else if(rxgain_code == 0x3)
		gain = 6;
	else if(rxgain_code == 0x4)
		gain = 9;
	else if(rxgain_code == 0x5)
		gain = 12;
	else if(rxgain_code == 0x6)
		gain = 15;

	return gain;
} 

void set_tx_loopback(void)
{
#if 1
	int i, count;
	struct rf_tbl *entry;

	count = (sizeof(tx_loopback_regs))/(sizeof(struct rf_tbl));

	for(i=0; i<count; i++)
	{
		entry = &tx_loopback_regs[i];

        if(!is_enable_rf12_rw && entry->address == 0x12)
        {
            continue;
        }

		if(entry->mask == 0)
			rf_write(entry->address, entry->val);
		else
			rf_update(entry->address, entry->val, entry->mask);

		if(entry->address == 18)
		{
			RFC_DBG(RFC_DBG_INFO, "!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!\n");
			RFC_DBG(RFC_DBG_INFO, "tx_loopback_regs with address 18 are not defined now !!\n");
		}
	}
#else
	rf_write(0, 0x51F0EF);
	rf_write(1, 0xBC000311);
	rf_write(8, 0xF80000);
	rf_write(9, 0x26646361);
	rf_write(10, 0x80200A91);
	rf_write(11, 0x11000000);
	rf_write(12, 0x40004820);
	rf_write(13, 0x7249DAB5);
	rf_write(14, 0x73F109);
	rf_write(15, 0x911F9D);
	rf_write(16, 0x5E2296);
	rf_write(17, 0x723C3);
	rf_write(18, 0x2400110C);	/* FIXME: 0x2400110C */
	//rf_write(18, 0x3480110C);	// for Sam's request, not confirm
	rf_write(19, 0x2211BADB);
	rf_write(20, 0x2620198);
	rf_write(21, 0x40689);
	rf_write(24, 0x10000);
	val = (rf_read(26) & ~0x3);
	rf_write(26, val);
	rf_write(28, 0x2C4A214);
	rf_write(29, 0x3BC9);
#endif
}

void set_rx_loopback(void)
{
#if 1
	/* FIXME: regs has not been updated to the newest setting */
	int i, count;
	struct rf_tbl *entry;

	count = (sizeof(rx_loopback_regs))/(sizeof(struct rf_tbl));

	for(i=0; i<count; i++)
	{
		entry = &rx_loopback_regs[i];

        if(!is_enable_rf12_rw && entry->address == 0x12)
        {
            continue;
        }

		if(entry->mask == 0)
			rf_write(entry->address, entry->val);
		else
			rf_update(entry->address, entry->val, entry->mask);
	}
#else
	rf_write(0, 0x41F22F);
	rf_write(1, 0xBA004311);
	val = (rf_read(4) & 0xFFFFFF3F) | 0xC0;
	rf_write(4, val);			/* FIXME: doc says that skip reg.4 setting */
	rf_write(8, 0xF80000);
	rf_write(9, 0x26646361);
	rf_write(10, 0x80000A91);
	rf_write(11, 0x11000000);
	rf_write(12, 0x40004820);
	rf_write(13, 0x7249DAB1);
	rf_write(14, 0x73F10A);
	rf_write(15, 0x911F9D);
	rf_write(16, 0x5E2296);
	rf_write(17, 0x723C3);
	rf_write(18, 0x2400110C);
	rf_write(19, 0x2211BADB);
	rf_write(20, 0x2620198);
	rf_write(21, 0x40689);
	rf_write(24, 0x10000);
	val = (rf_read(26) & ~0x3);
	rf_write(26, val);
	rf_write(28, 0x2C4A214);
	rf_write(29, 0x3BC9);
#endif
}

void set_rx_loopback_after_tx_loopback(void)
{
#if 1
	int i, count;
	struct rf_tbl *entry;

    if (is_rxloop_panther)
    {
        count = (sizeof(panther_rx_loopback_after_tx_loopback_regs))/(sizeof(struct rf_tbl));
    
        for(i=0; i<count; i++)
        {
            entry = &panther_rx_loopback_after_tx_loopback_regs[i];

			if(!is_enable_rf12_rw && entry->address == 0x12)
			{
				continue;
			}

            if(entry->mask == 0)
                rf_write(entry->address, entry->val);
            else
                rf_update(entry->address, entry->val, entry->mask);
    
            if(entry->address == 18)
            {
                RFC_DBG(RFC_DBG_INFO, "!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!\n");
                RFC_DBG(RFC_DBG_INFO, "panther_rx_loopback_after_tx_loopback_regs with address 18 are not defined now !!\n");
            }
        }
    }
    else
    {
        count = (sizeof(lynx_rx_loopback_after_tx_loopback_regs))/(sizeof(struct rf_tbl));
    
        for(i=0; i<count; i++)
        {
            entry = &lynx_rx_loopback_after_tx_loopback_regs[i];

			if(!is_enable_rf12_rw && entry->address == 0x12)
			{
				continue;
			}

            if(entry->mask == 0)
                rf_write(entry->address, entry->val);
            else
                rf_update(entry->address, entry->val, entry->mask);
    
            if(entry->address == 18)
            {
                RFC_DBG(RFC_DBG_INFO, "!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!\n");
                RFC_DBG(RFC_DBG_INFO, "lynx_rx_loopback_after_tx_loopback_regs with address 18 are not defined now !!\n");
            }
        }
    }
#else
	rf_write(0, 0x0051F20F);
	rf_write(1, 0xAB000311);
	rf_update(10, 0, 1 << 21);
	rf_write(29, 0x3BC8);
	rf_write(16, 0x015E2296);	// RXFE=ULG
#endif
}

double final_i, final_q;
void panther_set_iqcal_vga(int loop_type, int rxvga, int txvga, int bw, int freq, int iqswap)
{
//  int dcoc_dac_lsb, dcoc_dac_lsb_trac;

	RFC_DBG(RFC_DBG_INFO, "%s(): loop_type=%d, rxvga=%d, txvga=%d, bw=%d, freq=%d, iqswap=%d\n", 
			__FUNCTION__, loop_type, rxvga, txvga, bw, freq, iqswap);

	if(loop_type == TXLOOP)
	{
		set_tx_loopback();
		udelay(10000);	// set delay here to make sure set_tx_loopback success.

		set_txcal_txvga(txvga);
#if 0
		set_txcal_rxvga(rxvga);
#else
		set_txcal_rxvga(rxvga);     // for test, date: 08/30
		set_dcoc_cal_done(1);
		set_dcoc_dac_ctrl(0);

		/* force set iqswap = 0*/
		bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2));
//  	search_dcoc_lsb_and_track(5, -1, &dcoc_dac_lsb, &dcoc_dac_lsb_trac);	// for speed up, date: 08/30
//  	detect_dco_dac(5, dcoc_dac_lsb, dcoc_dac_lsb_trac, -1, NULL, NULL);
		detect_dco_dac(5, 2, 1, -1, NULL, NULL, &final_i, &final_q);      // panther test, date: 08/30
		bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2) | (iqswap << 1));
#endif
	}
	else // if(loop_type == RXLOOP)
	{
#if 0
		set_rx_loopback();
#else
		set_rx_loopback_after_tx_loopback();
#endif
		udelay(10000);	// set delay here to make sure set_rx_loopback success.

		set_txcal_txvga(txvga) ;
		set_rxcal_rxvga(rxvga) ;

		set_dcoc_cal_done(1);
		set_dcoc_dac_ctrl(0);

		/* force set iqswap = 0*/
		bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2));
		//search_dcoc_lsb_and_track(5, -1, &dcoc_dac_lsb, &dcoc_dac_lsb_trac);
		detect_dco_dac(5, 0, 5, -1, NULL, NULL, &final_i, &final_q);      // panther test, date: 08/30
		bb_register_write(0, 0x2, (bb_register_read(0, 0x2) & ~0x2) | (iqswap << 1));

//  	set_dcoc_cal_done(1);
//  	set_dcoc_dac_ctrl(1);
	}
}


