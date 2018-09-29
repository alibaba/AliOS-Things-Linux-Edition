/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfac.c
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <ip301.h>
#include <math.h>
#include <rf.h>
#include <rfc_comm.h>
#include <rfac.h>
#include <panther_debug.h>
#include <os_compat.h>
#include <bb.h>
#include <os_compat.h>
#include <panther_dev.h>

int printf(char *fmt, ...);

#ifdef CONFIG_RFAC

#if defined(RFAC_ANALYSIS)
#define MAX_RFAC_ITERATIONS  1000
static int rfac_test_case_no;
static u8 __vco_curves[MAX_RFAC_ITERATIONS][10];
static u32 __i_path_txlo_result[MAX_RFAC_ITERATIONS];
static u32 __q_path_txlo_result[MAX_RFAC_ITERATIONS];
static u32 __i_path_rxdc_result[MAX_RFAC_ITERATIONS][RF_DC_LEVEL];
static u32 __q_path_rxdc_result[MAX_RFAC_ITERATIONS][RF_DC_LEVEL];
double __tx_filter_chk[MAX_RFAC_ITERATIONS][4];       // 0:i_f0, 1:i_f1, 2:q_f0, 3:q_f1
double __rx_filter_chk[MAX_RFAC_ITERATIONS][4];       // 0:i_f0, 1:i_f1, 2:q_f0, 3:q_f1
#endif

#if 1 // for defined(RFAC_ANALYSIS)
#define TXLO_CALIBRATION_MAX_RETRY  30
int txlo_calibration_max_retry = TXLO_CALIBRATION_MAX_RETRY;
#endif
#define RXDC_CALIBRATION_MAX_RETRY  30
int rxdc_calibration_max_retry = RXDC_CALIBRATION_MAX_RETRY;


#if 0
int vco_calibration(int internal_ldo)
{
	// VCO auto-calibration
	// Relevant registers: reg6, reg0, reg1, reg12, reg4, reg5
	u32 reg12, reg5;
	u32 i;

	u8 lastval;
	u8 vco_curve[10];

    RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: using %s\n", internal_ldo ? "Internal LDO" : "External LDO");

    // setup internal LDO on/off
    if(internal_ldo)
    {
        rf_write(0x6, 0x3698);
        udelay(2);
    }
    else
    {
        rf_write(0x6, 0x369f);
        udelay(2);
    }

    rf_write(0x11, 0x80b);
    udelay(1);

    // setup sub-blocks on/off
    rf_write(0x0, 0x3ffef);      // VCO on, TX on, RX on
    udelay(2);

    // disable then enable VCO auto-calibration
    rf_write(0x1, 0x000);
    udelay(5);
    rf_write(0x1, 0x200);
	udelay(20000);

    reg12 = rf_read(0x12);
    if(0x3!=(0x3 & reg12))       // check reg12[0] and reg12[1]
    {
        RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    // read 10-curve register value
    for(i=0;i<5;i++)
    {
        //rf_update(0x4, (i << 11), 0x3800);
   		rf_write(0x4, (rf_read(0x4) & ~0x3800) | (i << 11));
 
        reg5 = rf_read(0x5);

        if(0==(reg5 & 0x20000))
        {
            RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: reg5 BIT17 not ok (0x%x), i = %d\n", reg5, i);
            return -2;
        }

        if(0==(reg5 & 0x00100))
        {
            RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: reg5 BIT8 not ok (0x%x), i = %d\n", reg5, i);
            return -3;
        }

        vco_curve[i*2] = ((reg5 >> 9) & 0xff);
        vco_curve[i*2 + 1] = (reg5 & 0xff);
    }

    RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: done, ");
    for(i=0;i<10;i++)
    {
        RFC_DBG(RFC_DBG_INFO, " %d", vco_curve[i]);
    }
    RFC_DBG(RFC_DBG_INFO, "\n\n");

#if defined(RFAC_ANALYSIS)
    for(i=0;i<10;i++)
    {
        __vco_curves[rfac_test_case_no][i] = vco_curve[i];
    }
#endif

    // check the results
    lastval = 0xFF;
    for(i=0;i<10;i++)
    {
        if(lastval < vco_curve[i])
        {
            RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: possibly wrong result\n");
            return -4;
        }
        else
        {
            lastval = vco_curve[i];
        }
    }

    rf_write(0x1, 0x200);
    return 0;
}

int tx_rx_filter_calibration(u8 bw, u8 tx_filter_code, u8 rx_filter_code)
{
	/* Tx Filter Code Setting : Manually
	   Rx Filter Code Setting : Automatic */

    u32 reg12, reg13, regf, reg8, reg1;
	u32 count, tune_rxfil;

    
	RFC_DBG(RFC_DBG_INFO, "%s(): bw = %d, tx filter code = %x, rx filter code = 0x%x\n",
			__FUNCTION__, bw, tx_filter_code, rx_filter_code);

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "%s(): reg13 0x%x before calibration\n", __FUNCTION__, reg13);

	/* ============== RX Auto Calibration ============== */
	reg8 = rf_read(0x8);
	if(bw)	/* 40MHz */
    	rf_write(0x8, 0xE000 | ((rx_filter_code & 0x7) << 7) | (reg8 & 0x1C7F));
	else	/* 20MHz */
    	rf_write(0x8, 0x2000 | ((rx_filter_code & 0x7) << 7) | (reg8 & 0x1C7F));
    udelay(2);

    // Start calibration
	reg1 = rf_read(0x1);
	reg1 = reg1 & ~(0x30);
    rf_write(0x1, reg1);
    udelay(50);
    rf_write(0x1, reg1 | 0x020);
    udelay(65);

    // Check test ready
    reg12 = rf_read(0x12);
    if(0x4!=(0x4 & reg12))       // check reg12[2], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "%s(): reg12 not ready (0x%x)\n", __FUNCTION__, reg12);
        return -1; 
    }

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "%s(): reg13 0x%x after rx calibration\n", __FUNCTION__, reg13);

	tune_rxfil = (reg13 & 0x3fe00) >> 9;
	for(count=1; count <= 8; count++)
	{
		if((1 << count) & tune_rxfil)
			break;
	}

	if(count > 8)
	{
        RFC_DBG(RFC_DBG_INFO, "%s(): filter_vco_coeff is wrong(0x%x)\n", __FUNCTION__, tune_rxfil);
        return -1;
    }

	tune_rxfil = (count-1)*2 + (tune_rxfil & 0x1);

	reg8 = ((tx_filter_code & 0x7) << 4) | tune_rxfil;
	if(bw)	/* 40MHz */
		reg8 = reg8 | 0xc000;

	rf_write(0x8, reg8);
    udelay(2);
	reg8 = rf_read(0x8);

	reg1 = rf_read(0x1);
	reg1 = (reg1 & 0xffffffcf) | 0x10;
	rf_write(0x1, reg1);
    udelay(2);

	regf = rf_read(0xf);
	regf = regf | 0x3;
	rf_write(0xf, regf);

    RFC_DBG(RFC_DBG_INFO, "%s(): reg8 0x%x after tx & rx reg_tune_fill_bypass_value setup\n\n", __FUNCTION__, reg8);
    RFC_DBG(RFC_DBG_INFO, "%s(): reg13 0x%x after tx & rx reg_tune_fill_bypass_value setup\n\n", __FUNCTION__, reg13);

	return 0;
}

int rx_filter_calibration(u8 bw, u8 filter_code)
{
#if 1
    // RX filter auto-calibration (per filter bandwidth)
    // Relevant registers: reg8, reg0, reg1, reg12, reg13
    // Input: bw => 0: 20MHz Mode, 1: 40MHz Mode
	//		  filter_code => RX filter bandwidth selection (calibration for on BW only)
    // Assume VCO has been selected and calibrated

    u32 reg12, reg13, reg8, reg0;

    RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: bw = %d, filter_code = %x\n", bw, filter_code);

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: reg13 0x%x before calibration\n", reg13);

    // select RX and setup RX filter bandwidth, reg8[9:7]= 111 max BW, 000 min BW
	// reg8[15] => 0: 20MHz Mode, 1: 40MHz Mode
	reg8 = rf_read(0x8);
	if(bw)	/* 40MHz */
    	rf_write(0x8, 0xB000 | ((filter_code & 0x7) << 7) | (reg8 & 0x7C7F));
	else	/* 20MHz */
    	rf_write(0x8, 0x3000 | ((filter_code & 0x7) << 7) | (reg8 & 0x7C7F));
    udelay(2);

    // setup sub-blocks on/off
	reg0 = rf_read(0x0);
    rf_write(0x0, 0x3ffef);      // VCO on, TX on, RX on
    udelay(2);

    // Start calibration
    rf_write(0x1, 0x000);
    udelay(5);
    rf_write(0x1, 0x020);
    udelay(65);

    // Check test ready
    reg12 = rf_read(0x12);
    if(0x4!=(0x4 & reg12))       // check reg12[2], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: done, reg13 0x%x after calibration\n\n", reg13);

    rf_write(0x1, 0x200);
	/* restore the reg 0 value, for kepping the rfc calibration value. By KC. */
    rf_write(0x0, reg0);
#endif
    return 0;
}

int tx_filter_calibration(u8 bw, u8 filter_code)
{
#if 1
    // Relevant registers: reg8, reg0, reg1, reg12, reg13
    // Input:  bw => 0: 20MHz Mode, 1: 40MHz Mode
	//		   filter_code => TX filter bandwidth selection (calibration for on BW only)
    // Assume VCO has been selected and calibrated

    u32 reg12, reg13, reg8, reg0;

    RFC_DBG(RFC_DBG_INFO, "TX filter auto-calibration: bw = %d, filter_code = %x\n", bw, filter_code);

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "TX filter auto-calibration: reg13 0x%x before calibration\n", reg13);

    // select TX and setup TX filter bandwidth, reg8[12:10]= 111 max BW, 000 min BW
	// reg8[14] => 0: 20MHz Mode, 1: 40MHz Mode
	reg8 = rf_read(0x8);
	if(bw)	/* 40MHz */
    	rf_write(0x8, 0x4200 | ((filter_code & 0x7) << 10) | (reg8 & 0x83FF));
	else	/* 20MHz */
    	rf_write(0x8, 0x0200 | ((filter_code & 0x7) << 10) | (reg8 & 0x83FF));
    udelay(2);

    // setup sub-blocks on/off
	reg0 = rf_read(0x0);
    rf_write(0x0, 0x3ffef);      // VCO on, TX on, RX on
    udelay(2);

    // Start calibration
    rf_write(0x1, 0x000);
    udelay(5);
    rf_write(0x1, 0x020);
    udelay(65);

    // Check test ready
    reg12 = rf_read(0x12);
    if(0x4!=(0x4 & reg12))       // check reg12[2], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "TX filter auto-calibration: reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "TX filter auto-calibration: done, reg13 0x%x after calibration\n\n", reg13);

    rf_write(0x1, 0x200);
	/* restore the reg 0 value, for kepping the rfc calibration value. By KC */
    rf_write(0x0, reg0);
#endif
    return 0;
}


int txlo_calibration(int channel)
{
    // Relevant registers: reg2, reg0, reg7, reg1, reg12
    // Input: Select an arbitrary channel
    // Assume VCO has been selected and calibrated

    u32 reg12;
    u32 i_path_txlo_result, q_path_txlo_result;
    int retry = 0;

    u32 i_path_txlo_result_prev = 0xffff;
    u32 q_path_txlo_result_prev = 0xffff;

Retry:

    RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: channel %d\n", channel);

    reg12 = rf_read(0x12);
    RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: reg12 0x%x before calibration\n", reg12);

    // Set up DAC: DAC turn on; DAC = 0;
    bb_register_write(0x1c, 0xee);          // Set BB register 0x1C to the value of 0xEE

    // select channel
    mt301_set_40mhz_channel(channel, 0);
	udelay(30);      // Wait 30us for PLL locking time

    // setup sub-blocks on/off
    rf_write(0x0, 0x3fe00);      // VCO on, TX on, RX off
    udelay(2);

    // TX VGA setting, VGA_vc<6:0>=1111111
    rf_write(0x7, 0xffc0);
    udelay(2);

    // I path calibration, reg1[6]=1 and reg1[8] = 0
    rf_write(0x1, 0x000);
    udelay(5);
    rf_write(0x1, 0x040);
    udelay(20);

    // Check I-path test ready and save I-path result
    reg12 = rf_read(0x12);
    if(0x10!=(0x10 & reg12))       // check reg12[4], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: I-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    i_path_txlo_result = (reg12 >> 5) & 0xff;       // I_path_TX_LO_result = read reg12[12:5]

    // Q path calibration, reg1[6]=1 and reg1[8] = 1
    rf_write(0x1, 0x000);
    udelay(5);
    rf_write(0x1, 0x140);
    udelay(20);

    // Check Q-path test ready and save Q-path result
    reg12 = rf_read(0x12);
    if(0x10!=(0x10 & reg12))       // check reg12[4], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: Q-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    q_path_txlo_result = (reg12 >> 5) & 0xff;       // Q_path_TX_LO_result = read reg12[12:5]

    RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: done, i_path_txlo_result 0x%x, q_path_txlo_result 0x%x\n\n", i_path_txlo_result, q_path_txlo_result);

    rf_write(0x1, 0x200);

#if defined(RFAC_ANALYSIS)
    __i_path_txlo_result[rfac_test_case_no] = i_path_txlo_result;
    __q_path_txlo_result[rfac_test_case_no] = q_path_txlo_result;
#endif

    if(txlo_calibration_max_retry)
    {
		s32 delta_i = (s32)i_path_txlo_result_prev - (s32)i_path_txlo_result;
		s32 delta_q = (s32)q_path_txlo_result_prev - (s32)q_path_txlo_result;
		if((-1 > delta_i) || (delta_i > 1) || (-1 > delta_q) || (delta_q > 1))
        {
            if(retry < txlo_calibration_max_retry)
            {
                i_path_txlo_result_prev = i_path_txlo_result;
                q_path_txlo_result_prev = q_path_txlo_result;
                RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: retry %d\n", retry);
                retry++;
                goto Retry;
            }
            else
            {
                RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: failed\n");
                return -1;
            }
        }
    }

    return 0;
}

int rxdc_calibration(u8 gain)
{
    // Relevant registers: reg7, reg0, reg9, reg1, reg12, reg13
    // Assume VCO has been selected and calibrated

    u32 reg12, reg13;
    u32 i_path_rxdc_result, q_path_rxdc_result;
	u32 retry = 0;
    u32 i_path_rxdc_result_prev = 0xffff;
    u32 q_path_rxdc_result_prev = 0xffff;

    RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: %s gain mode\n", (gain==0) ? "high" : ((gain==1) ? "middle" : "ultra-low"));

rxdc_retry:

    // Read reg13[8:0]
    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: reg13 0x%x before calibration\n", reg13);

    // setup LNA gain mode
    if(gain==0)
        rf_write(0x7, 0x38);         // high gain; gpk=111
    else if(gain==1)
        rf_write(0x7, 0x3c);         // middle gain
	else if(gain==2)
        rf_write(0x7, 0x3d);         // low gain
    else
        rf_write(0x7, 0x3f);         // ultra-low gain
    udelay(1);

    // VCO on, RX on, RX_SVLP off, TX off
    rf_write(0x0, 0x3e0ff);
    udelay(2);

    // I path, reg9[17:16]=01 for I path and set RGCL<5:0>=111111
    rf_write(0x9, 0x101fc);
    udelay(200);

    // Set reg1[3:2]=01 to disable rxdc_force but start tuning
    rf_write(0x1, 0x0);
    udelay(200);
    rf_write(0x1, 0x004);
	udelay(12000);

    // Check I-path test ready and save I-path result
    reg12 = rf_read(0x12);
    if(0x08!=(0x08 & reg12))
    {
        RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: I-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);
    i_path_rxdc_result = reg13 & 0x01ff;      // Read reg13[8:0] after calibration

    // Q path, reg9[17:16]=11 for I path and set RGCL<5:0>=111111 
    rf_write(0x9, 0x301fc);
    udelay(200);

    // Start Q-calibration
    rf_write(0x1, 0x0);
    udelay(200);
    rf_write(0x1, 0x004);
	udelay(12000);

    // Check Q-path test ready and save Q-path result
    reg12 = rf_read(0x12);
    if(0x08!=(0x08 & reg12))
    {
        RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: Q-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);           // 	Read reg13[8:0] after calibration
    q_path_rxdc_result = reg13 & 0x01ff;

    // paste the I/Q result to regc/regb/rega
    if(gain==0)
        rf_write(0x14, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // high gain; regc
    else if(gain==1)
        rf_write(0xc, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // middle gain; regb
	else if(gain==2)
        rf_write(0xb, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // low gain; regb
    else
        rf_write(0xa, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // ultra-low gain; rega

    // Disable the calibration
    rf_write(0x1, 0x0);
    udelay(200);

    RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: done, i_path_rxdc_result 0x%x, q_path_rxdc_result 0x%x\n\n", i_path_rxdc_result, q_path_rxdc_result);

    rf_write(0x1, 0x200);

#if defined(RFAC_ANALYSIS)
    __i_path_rxdc_result[rfac_test_case_no][gain] = i_path_rxdc_result;
    __q_path_rxdc_result[rfac_test_case_no][gain] = q_path_rxdc_result;
#endif

    if(rxdc_calibration_max_retry)
    {
		s32 delta_i = (s32)i_path_rxdc_result_prev - (s32)i_path_rxdc_result;
		s32 delta_q = (s32)q_path_rxdc_result_prev - (s32)q_path_rxdc_result;
		if((-3 > delta_i) || (delta_i > 3) || (-3 > delta_q) || (delta_q > 3))
        {
            if(retry < rxdc_calibration_max_retry)
            {
                i_path_rxdc_result_prev = i_path_rxdc_result;
                q_path_rxdc_result_prev = q_path_rxdc_result;
                RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: retry %d\n", retry);
                retry++;
                goto rxdc_retry;
            }
            else
            {
                RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: failed\n");
                return -1;
            }
        }
    }

    return 0;
}

int rf_calibration(int channel, int internal_ldo)
{
    int ret = 0;
    int i;
	unsigned char buf[2];

	RFC_DBG(RFC_DBG_INFO, "RF auto-calibration: rfchip=%d, %s, channel %d\n", ldev->rf.chip_ver, internal_ldo ? "Internal LDO" : "External LDO", channel);

    ret = vco_calibration(internal_ldo);
    if(0>ret)
        goto Failed;

	if(ldev->rf.chip_ver == RFCHIP_IP301_G)
	{
		ret = tx_rx_filter_calibration(0, 5, 4);
		if(0>ret)
			goto Failed;
	}
	else
	{
		ret = rx_filter_calibration(0, 4);
		if(0>ret)
			goto Failed;

		ret = tx_filter_calibration(0, 4);
		if(0>ret)
			goto Failed;
	}
    
	ret = txlo_calibration(channel);
    if(0>ret)
        goto Failed;

	/* new RF add low gain calibration */
	for(i=0;i<4;i++)
	{
		ret = rxdc_calibration(i);
		if(0>ret)
			goto Failed;
	}

    RFC_DBG(RFC_DBG_INFO, "@@ RF auto-calibration (RFAC): done.\n\n");
    return 0;

Failed:
	while(1)
	{
		RFC_DBG(RFC_DBG_INFO, "RF auto-calibration (RFAC) Fail, Please seek assistance from Stan (#2629). Press c to continue.\n\n");
#ifdef CONFIG_TODO
		WLA_GETS(buf);
		if(strncmp(buf, "c", 1) == 0)
#endif
			break;
	}
    return ret;
}
#endif //0
int mt301_vco_calibration(int internal_ldo)
{
	// VCO auto-calibration
	// Relevant registers: reg6, reg0, reg1, reg12, reg4, reg5
	u32 reg12, reg5;
	u32 i;

	u8 lastval;
	u8 vco_curve[10];

    RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: using %s\n", internal_ldo ? "Internal LDO" : "External LDO");

	rf_write(0x15, 0x1005); //shutdown mode
	rf_write(0x1, 0x8000); // Internal VCO LDO
	rf_write(0x6, 0x10bd); // Master and DIG LDO control
	rf_write(0x7, 0x7c0); // 1.8V Slave LDO of PA power 
	rf_write(0x17, 0x308fe); // Slave LDO control 
	rf_write(0x3, 0x1284); //Internal loop filter
	rf_write(0x11, 0x2cb2); //internal loop filter
    udelay(2);

    // setup sub-blocks on/off
    rf_write(0x0, 0x3ffef);      // VCO on, TX on, RX on
	rf_write(0x0, 0x3fe00);      // VCO on, TX on, RX off
    udelay(2);

    // disable then enable VCO auto-calibration
    rf_write(0x1, 0x8200);
	udelay(20000);

    reg12 = rf_read(0x12);
    if(0x3!=(0x3 & reg12))       // check reg12[0] and reg12[1]
    {
        RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    // read 10-curve register value
    for(i=0;i<5;i++)
    {
        //rf_update(0x4, (i << 11), 0x3800);
   		rf_write(0x4, (rf_read(0x4) & ~0x3800) | (i << 11));
 
        reg5 = rf_read(0x5);

        if(0==(reg5 & 0x20000))
        {
            RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: reg5 BIT17 not ok (0x%x), i = %d\n", reg5, i);
            return -2;
        }

        if(0==(reg5 & 0x00100))
        {
            RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: reg5 BIT8 not ok (0x%x), i = %d\n", reg5, i);
            return -3;
        }

        vco_curve[i*2] = ((reg5 >> 9) & 0xff);
        vco_curve[i*2 + 1] = (reg5 & 0xff);
    }

    RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: done, ");
    for(i=0;i<10;i++)
    {
        RFC_DBG(RFC_DBG_INFO, " %d", vco_curve[i]);
    }
    RFC_DBG(RFC_DBG_INFO, "\n\n");

#if defined(RFAC_ANALYSIS)
    for(i=0;i<10;i++)
    {
        __vco_curves[rfac_test_case_no][i] = vco_curve[i];
    }
#endif

    // check the results
    lastval = 0xFF;
    for(i=0;i<10;i++)
    {
        if(lastval < vco_curve[i])
        {
            RFC_DBG(RFC_DBG_INFO, "VCO auto-calibration: possibly wrong result\n");
            return -4;
        }
        else
        {
            lastval = vco_curve[i];
        }
    }

    rf_write(0x1, 0x8000);
    return 0;
}

int mt301_rx_filter_calibration(u8 bw, u8 filter_code)
{
#if 1
    // RX filter auto-calibration (per filter bandwidth)
    // Relevant registers: reg8, reg0, reg1, reg12, reg13
    // Input: bw => 0: 20MHz Mode, 1: 40MHz Mode
	//		  filter_code => RX filter bandwidth selection (calibration for on BW only)
    // Assume VCO has been selected and calibrated

    u32 reg12, reg13, reg8, reg0;

    RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: bw = %d, filter_code = %x\n", bw, filter_code);

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: reg13 0x%x before calibration\n", reg13);

    // select RX and setup RX filter bandwidth, reg8[9:7]= 111 max BW, 000 min BW
	// reg8[15] => 0: 20MHz Mode, 1: 40MHz Mode
#if 0	/* FIXME: work around ? */
	reg8 = rf_read(0x8);
	if(bw)	/* 40MHz */
    	rf_write(0x8, 0xB000 | ((filter_code & 0x7) << 7) | (reg8 & 0x7C7F));
	else	/* 20MHz */
    	rf_write(0x8, 0x3000 | ((filter_code & 0x7) << 7) | (reg8 & 0x7C7F));
#else
	/* bw=0, filter_code=4 */
	reg8 = (rf_read(0x8) & 0x0c00) | 0x28000;
    rf_write(0x8, reg8);
#endif
    udelay(2);

    // setup sub-blocks on/off
	reg0 = rf_read(0x0);
    rf_write(0x0, 0x3FE00);      // VCO on, TX on
	rf_write(0x17, 0x308FE);		// Added
    udelay(2);

    // Start calibration
    rf_write(0x1, 0x8000);
    udelay(5);
    rf_write(0x1, 0x8020);
    udelay(65);

    // Check test ready
    reg12 = rf_read(0x12);
    if(0x4!=(0x4 & reg12))       // check reg12[2], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "RX filter auto-calibration: done, reg13 0x%x after calibration\n\n", reg13);

	reg13 = (reg13 >>10) & 0x1f;
	reg13 = (reg13 <2) ? 0 : reg13-2;
	reg13 = reg13*33; // *(32 + 1)
	rf_write(0x8, reg8 +reg13);

    rf_write(0x1, 0x8000);
	/* restore the reg 0 value, for kepping the rfc calibration value. By KC. */
    rf_write(0x0, reg0);
#endif
    return 0;
}

int mt301_txlo_calibration(int channel)
{
    // Relevant registers: reg2, reg0, reg7, reg1, reg12
    // Input: Select an arbitrary channel
    // Assume VCO has been selected and calibrated

    u32 reg12;
    u32 i_path_txlo_result, q_path_txlo_result;
    int retry = 0;

    u32 i_path_txlo_result_prev = 0xffff;
    u32 q_path_txlo_result_prev = 0xffff;

Retry:

    RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: channel %d\n", channel);

    reg12 = rf_read(0x12);
    RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: reg12 0x%x before calibration\n", reg12);

    // Set up DAC: DAC turn on; DAC = 0;
    bb_register_write(0, 0x1c, 0xee);          // Set BB register 0x1C to the value of 0xEE

    // select channel
    //ip301_set_channel(0, channel);
	rf_write(0x2, 0x28c4e);
	udelay(30);      // Wait 30us for PLL locking time

    // setup sub-blocks on/off
    rf_write(0x0, 0x3fe00);      // VCO on, TX on, RX off
    udelay(2);

    // TX VGA setting, VGA_vc<6:0>=1111111
	rf_write(0x7, 0x7c0); // Add, VGA
	rf_write(0x16, 0x12627); // Add, TXPA
    udelay(2);

    // I path calibration, reg1[6]=1 and reg1[8] = 0
    rf_write(0x1, 0x8000);
    udelay(5);
    rf_write(0x1, 0x8040);
    udelay(20);

    // Check I-path test ready and save I-path result
    reg12 = rf_read(0x12);
    if(0x10!=(0x10 & reg12))       // check reg12[4], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: I-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    i_path_txlo_result = (reg12 >> 5) & 0xff;       // I_path_TX_LO_result = read reg12[12:5]

    // Q path calibration, reg1[6]=1 and reg1[8] = 1
    rf_write(0x1, 0x8000);
    udelay(5);
    rf_write(0x1, 0x8140);
    udelay(20);

    // Check Q-path test ready and save Q-path result
    reg12 = rf_read(0x12);
    if(0x10!=(0x10 & reg12))       // check reg12[4], (should be "1" and means calibration done)
    {
        RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: Q-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    q_path_txlo_result = (reg12 >> 5) & 0xff;       // Q_path_TX_LO_result = read reg12[12:5]

    RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: done, i_path_txlo_result 0x%x, q_path_txlo_result 0x%x\n\n", i_path_txlo_result, q_path_txlo_result);

    rf_write(0x1, 0x8000);

#if defined(RFAC_ANALYSIS)
    __i_path_txlo_result[rfac_test_case_no] = i_path_txlo_result;
    __q_path_txlo_result[rfac_test_case_no] = q_path_txlo_result;
#endif

    if(txlo_calibration_max_retry)
    {
		s32 delta_i = (s32)i_path_txlo_result_prev - (s32)i_path_txlo_result;
		s32 delta_q = (s32)q_path_txlo_result_prev - (s32)q_path_txlo_result;
		if((-1 > delta_i) || (delta_i > 1) || (-1 > delta_q) || (delta_q > 1))
        {
            if(retry < txlo_calibration_max_retry)
            {
                i_path_txlo_result_prev = i_path_txlo_result;
                q_path_txlo_result_prev = q_path_txlo_result;
                RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: retry %d\n", retry);
                retry++;
                goto Retry;
            }
            else
            {
                RFC_DBG(RFC_DBG_INFO, "TXLO auto-calibration: failed\n");
                return -1;
            }
        }
    }

    return 0;
}

int mt301_rxdc_calibration(u8 gain)
{
    // Relevant registers: reg7, reg0, reg9, reg1, reg12, reg13
    // Assume VCO has been selected and calibrated

    u32 reg12, reg13;
    u32 i_path_rxdc_result, q_path_rxdc_result;
	u32 retry = 0;
    u32 i_path_rxdc_result_prev = 0xffff;
    u32 q_path_rxdc_result_prev = 0xffff;

    RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: %s gain mode\n", (gain==0) ? "high" : ((gain==1) ? "middle" : "ultra-low"));

rxdc_retry:

    // Read reg13[8:0]
    reg13 = rf_read(0x13);
    RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: reg13 0x%x before calibration\n", reg13);

    // setup LNA gain mode
    if(gain==0)
        rf_write(0x7, 0x10020);         // high gain; gpk=111
    else if(gain==1)
        rf_write(0x7, 0x10024);         // middle gain
	else if(gain==2)
        rf_write(0x7, 0x10025);         // low gain
    else
        rf_write(0x7, 0x10027);         // ultra-low gain
    udelay(1);

    // VCO on, RX on, RX_SVLP off, TX off
	//rf_write(0x7, 0x10020); // 1.8V Slave LDO of PA power 
	rf_write(0x17, 0x37840); // Slave LDO control 
	rf_write(0x0, 0x3e0ff);
    udelay(2);

    // I path, reg9[17:16]=01 for I path and set RGCL<5:0>=111111
    rf_write(0x9, 0x101fc);
    udelay(200);

    // Set reg1[3:2]=01 to disable rxdc_force but start tuning
    rf_write(0x1, 0x8000);
    udelay(200);
    rf_write(0x1, 0x8004);

#if 0
	int i=0;
	for(i=0; i<=0x1f; i++)
		RFC_DBG(RFC_DBG_INFO, "rf reg.0x%02x = 0x%x\n", i, rf_read(i));
#endif
	
	udelay(12000);

    // Check I-path test ready and save I-path result
    reg12 = rf_read(0x12);
    if(0x08!=(0x08 & reg12))
    {
        RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: I-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);
    i_path_rxdc_result = reg13 & 0x01ff;      // Read reg13[8:0] after calibration

    // Q path, reg9[17:16]=11 for I path and set RGCL<5:0>=111111 
    rf_write(0x9, 0x301fc);
    udelay(200);

    // Start Q-calibration
    rf_write(0x1, 0x8000);
    udelay(200);
    rf_write(0x1, 0x8004);

	udelay(12000);

    // Check Q-path test ready and save Q-path result
    reg12 = rf_read(0x12);
    if(0x08!=(0x08 & reg12))
    {
        RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: Q-path reg12 not ready (0x%x)\n", reg12);
        return -1;
    }

    reg13 = rf_read(0x13);           // 	Read reg13[8:0] after calibration
    q_path_rxdc_result = reg13 & 0x01ff;

    // paste the I/Q result to regc/regb/rega
    if(gain==0)
        rf_write(0x14, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // high gain; regc
    else if(gain==1)
        rf_write(0xc, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // middle gain; regb
	else if(gain==2)
        rf_write(0xb, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // low gain; regb
    else
        rf_write(0xa, (q_path_rxdc_result << 9) | i_path_rxdc_result);         // ultra-low gain; rega

    // Disable the calibration
    rf_write(0x1, 0x8000);
    udelay(200);

    RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: done, i_path_rxdc_result 0x%x, q_path_rxdc_result 0x%x\n\n", i_path_rxdc_result, q_path_rxdc_result);


	i_path_rxdc_result = (i_path_rxdc_result >> 3);
	q_path_rxdc_result = (q_path_rxdc_result >> 3);
#if defined(RFAC_ANALYSIS)
    __i_path_rxdc_result[rfac_test_case_no][gain] = i_path_rxdc_result;
    __q_path_rxdc_result[rfac_test_case_no][gain] = q_path_rxdc_result;
#endif

    if(rxdc_calibration_max_retry)
    {
		s32 delta_i = (s32)i_path_rxdc_result_prev - (s32)i_path_rxdc_result;
		s32 delta_q = (s32)q_path_rxdc_result_prev - (s32)q_path_rxdc_result;
		if((-3 > delta_i) || (delta_i > 3) || (-3 > delta_q) || (delta_q > 3))
        {
            if(retry < rxdc_calibration_max_retry)
            {
                i_path_rxdc_result_prev = i_path_rxdc_result;
                q_path_rxdc_result_prev = q_path_rxdc_result;
                RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: retry %d\n", retry);
                retry++;
                goto rxdc_retry;
            }
            else
            {
                RFC_DBG(RFC_DBG_INFO, "RXDC auto-calibration: failed\n");
                return -1;
            }
        }
    }

    return 0;
}


int mt301_calibration(int channel, int internal_ldo)
{
    int ret = 0;
    int i;
 
	RFC_DBG(RFC_DBG_INFO, "RF auto-calibration: rfchip=%d, %s, channel %d\n", ldev->rf.chip_ver, internal_ldo ? "Internal LDO" : "External LDO", channel);

	RFC_DBG(RFC_DBG_INFO, "~~~~Jerry asks to DAC does not enter sleep mode~~~\n");
	bb_register_write(0, 0x1c, 0xaa);
	
	//serial_printf("%s(%d)\n", __func__, __LINE__);
	ret = mt301_vco_calibration(internal_ldo);

	//serial_printf("%s(%d)\n", __func__, __LINE__);
	if(0>ret)
		goto Failed;

	//serial_printf("%s(%d)\n", __func__, __LINE__);
	ret = mt301_rx_filter_calibration(0, 4);
	if(0>ret)
		goto Failed;

	ret = mt301_txlo_calibration(channel);
	if(0>ret)
	{
		goto Failed;
	}

	/* new RF add low gain calibration */
	for(i=0;i<4;i++)
	{
		ret = mt301_rxdc_calibration(i);
		if(0>ret)
			goto Failed;
	}

	RFC_DBG(RFC_DBG_INFO, "~~~Jerry asks to re-enter sleep mode~~~\n");
	bb_register_write(0, 0x1c, 0x0);

    RFC_DBG(RFC_DBG_INFO, "@@ RF auto-calibration (RFAC): done.\n\n");
    return 0;

Failed:
#ifdef CONFIG_TODO
	while(1)
#endif
	{
		RFC_DBG(RFC_DBG_INFO, "RF auto-calibration (RFAC) Fail, Please seek assistance from Stan (#2629). Press c to continue.\n\n");
#ifdef CONFIG_TODO
		//WLA_GETS(buf);
		if(strncmp(buf, "c", 1) == 0)
			break;
#endif
	}
    
	return ret;
}

#endif //CONFIG_RFAC
