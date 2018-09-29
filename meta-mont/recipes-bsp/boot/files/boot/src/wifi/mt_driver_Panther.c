/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2017 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/* ---------------------------------------------------------------------------
 *
 * File:               mt_driver_Panther.c
 *
 * Current version:    0.00.09
 *
 * Description:        IC driver of Panther.
 *
 *
 * Log:  Description             Version     Date            Author
 *        --------------------------------------------------------------------
 *        Create                 0.00.00     2017.06.19      BingJu.W
 *        Update                 0.00.01     2017.06.25      Shimingfu    pll function
 *        Update                 0.00.02     2017.07.05      BingJu.W     I2C access methord
 *        Update                 0.00.03     2017.07.11      Shimingfu    PLL function
 *        Update                 0.00.04     2017.07.11      FangY        Init Function
 *        Update                 0.00.05     2017.07.14      FangY        SetABB Function update 
 *        Update                 0.00.06     2017.07.17      Shiming.fu   Update Init function
 *        Update                 0.00.07     2017.07.20      Shiming.fu   Update Set PLL function
 *        Update                 0.00.08     2017.07.24      Shiming.fu   Update init function 
 *                                                           (Panther_Set_RF_Reg(handle, 2, 0x77486fcd); Panther_Set_RF_Reg(handle, 3, 0x84889c00);)
 *        Update                 0.00.09     2017.08.9        Yang Fang update BW setting in init function
 *****************************************************************************/
#define PANTHER_ASIC_INIT_TEST

#if !defined(PANTHER_ASIC_INIT_TEST)
#include "mt_driver_Panther.h"

#include "..\i2c.h"
#endif

#ifdef PANTHER_ASIC_INIT_TEST
#include "mt_driver_Panther.h"
//#include <linux/i2c.h>

int printf(char *fmt, ...);

BOOL Wrap_RBUS_Get(U32 addr, U32 *data)
{
//  printf("Wrap_RBUS_Get: addr = 0x%x, data = 0x%x\n", addr, *data);
	*data = *((volatile unsigned long *)addr);

	return TRUE;
}

BOOL Wrap_RBUS_Set(U32 addr, U32 data)
{
//  printf("Wrap_RBUS_Set: addr = 0x%x, data = 0x%x\n", addr, data);
	*((volatile unsigned long *)addr) = data;

	return TRUE;
}

BOOL Wrap_PBUS_Get(U32 addr, U32 *data)
{
	*data = *((volatile unsigned long *)addr);

	return TRUE;
}

BOOL Wrap_PBUS_Set(U32 addr, U32 data)
{
	*((volatile unsigned long *)addr) = data;

	return TRUE;
}

void panther_rf_reg_dump(void)
{
#if 0
	U32 i = 0;
	U32 addr;

//  printk("DUMP panther rf registers:\n");

	for(i = 0; i < 30; i++)
	{
		addr = 0xbf004f00 + i*4;
//  	printk("reg=%d, addr=0x%x, val=0x%lx\n", i, addr, *((volatile unsigned long*)addr));
	}
#endif
}

MT_DEVICE_SETTINGS_PANTHER mt_handle;
#endif

BOOL DABUS_Get_Reg(U16 addr, U32 *data)
{
	//	TODO:
	//	Get registers by DABUS

	//return Wrap_DABUS_Get(addr, data);
	U32 reg_addr = 0;
	reg_addr = 0xbf000000+addr;
	return Wrap_RBUS_Get(reg_addr, data);
}

BOOL DABUS_Set_Reg(U16 addr, U32 data)
{
	//	TODO:
	//	Set registers by DABUS

	//return Wrap_DABUS_Set(addr, data);
	U32 reg_addr = 0;
	reg_addr = 0xbf000000+addr;
	return Wrap_RBUS_Set(reg_addr, data);
}

BOOL RBUS_Get_Reg(U32 addr, U32 *data)
{
	//	TODO:
	//	Get registers by RBUS

	return Wrap_RBUS_Get(addr, data);
}

BOOL RBUS_Set_Reg(U32 addr, U32 data)
{
	//	TODO:
	//	Get registers by RBUS

	return Wrap_RBUS_Set(addr, data);
}
BOOL PBUS_Get_Reg(U32 addr, U32 *data)
{
	//	TODO:
	//	Get registers by PBUS

	return Wrap_PBUS_Get(addr, data);
}

BOOL PBUS_Set_Reg(U32 addr, U32 data)
{
	//	TODO:
	//	Get registers by PBUS

	return Wrap_PBUS_Set(addr, data);
}


BOOL Panther_Get_Reg(MT_Panther_Handle handle, U32 addr, U32 *data)
{
	BOOL bOK = FALSE;

	//if((addr>=0xbf004a00)&&(addr<=0xbf004fff))
	{
	//	bOK = DABUS_Get_Reg((U16)addr, data);
	}
	//else
	if((addr>=0xbf001000)&&(addr<=0xbf002fff))
	{
		bOK = PBUS_Get_Reg(addr, data);
	}
	else
	{
		bOK = RBUS_Get_Reg(addr, data);
	}
	return bOK;
}

BOOL Panther_Set_Reg(MT_Panther_Handle handle, U32 addr, U32 data)
{
	BOOL bOK = FALSE;

	//if((addr>=0xbf004a00)&&(addr<=0xbf004fff))
	{
	//	bOK = DABUS_Set_Reg((U16)addr, data);
	}
	//else
	if((addr>=0xbf001000)&&(addr<=0xbf002fff))
	{
		bOK = PBUS_Set_Reg(addr, data);
	}
	else
	{
		bOK = RBUS_Set_Reg(addr, data);
	}

	return bOK;
}

U32 SetSeveralBits2Data(U32 target_data, U32 source_bits, U8 bit_high, U8 bit_low)
{
	U32 tmp_data;
	U8 bit_tmp;

	if((bit_high > 31) || (bit_low > 31))
	{
		return target_data;
	}

	if(bit_high == bit_low)
	{
		source_bits &= 0x01;
		source_bits <<= bit_low;

		tmp_data = 0x01 << bit_low;

		target_data &= ~tmp_data;
		target_data |= source_bits;

		return target_data;
	}

	if(bit_high < bit_low)
	{
		bit_tmp = bit_high;
		bit_high = bit_low;
		bit_low = bit_tmp;
	}

	tmp_data = 0xFFFFFFFF;
	tmp_data >>= bit_low;
	tmp_data <<= bit_low;
	tmp_data <<= (31 - bit_high);
	tmp_data >>= (31 - bit_high);
	tmp_data &= 0xFFFFFFFF;

	source_bits <<= bit_low;
	source_bits &= tmp_data;

	target_data &= ~tmp_data;
	target_data |= source_bits;


	return target_data;
}

BOOL Panther_Get_RF_Reg(MT_Panther_Handle handle, U8 index, U32 *data)
{
	U16 addr = 0;
	BOOL bOK = FALSE;


	addr = 0x4F00 + index * 4;
	bOK = DABUS_Get_Reg(addr, data);

	return bOK;
}

BOOL Panther_Set_RF_Reg(MT_Panther_Handle handle, U8 index, U32 data)
{
	U16 addr = 0;
	BOOL bOK = FALSE;

	addr = 0x4F00 + index * 4;
	bOK = DABUS_Set_Reg(addr, data);


	return bOK;
}


BOOL Panther_Set_RF_Reg_Bits(MT_Panther_Handle handle, U8 index, U32 data, U8 bit_high, U8 bit_low)
{
	U32 ulData;
	BOOL bOK = FALSE;

	bOK = Panther_Get_RF_Reg(handle, index, &ulData);

	if(bOK)
	{
		ulData = SetSeveralBits2Data(ulData, data, bit_high, bit_low);
		bOK = Panther_Set_RF_Reg(handle, index, ulData);
	}

	return bOK;
}

void udelay(unsigned int time);
BOOL mt_Panther_Init(MT_Panther_Handle handle)
{
	int delay = 1000;
	U32 ulData = 0;
	if(handle == NULL)
	{
		return FALSE;
	}

	if(handle->bInitOK == FALSE)
	{
		handle->ucDevAddr		 = 0x68;

		handle->bManualEn		 = 1;			// manual_en
		handle->bLoBpfBypassEn	 = 0;			// lobpf_bypass_en

		handle->iPLLMode		 = 1;

		handle->bInitOK			 = TRUE;

		handle->iVersionNumber	 = 11;			// 0.00.11
		handle->iVersionTime	 = 16081220;
	}
		// --------  pll setting: only do one time after power on ----------------

	    Panther_Get_Reg(handle, 0xBF0048F8,&ulData);
	    ulData =SetSeveralBits2Data(ulData,0X01,0,0);
		Panther_Set_Reg(handle, 0xBF0048F8,ulData);

		Panther_Set_RF_Reg_Bits(handle, 0, 0x07, 2, 0);         //XPD PLL /BB
		Panther_Set_RF_Reg_Bits(handle, 1, 0x01, 0, 0);
		Panther_Set_RF_Reg_Bits(handle, 10, 1, 26, 26);         //xpd_bb_bias

	    Panther_Set_RF_Reg(handle, 2, 0x77486fcd);   // ngm_en=1, ngm_ci=3, xor_ci=2,pi_ci=1
	    Panther_Set_RF_Reg(handle, 3, 0x84669c00);   // winsel=1,cpm=cpa=6,c2=7,c1=0
		Panther_Set_RF_Reg(handle, 4, 0x000000ca);
		Panther_Set_RF_Reg(handle, 5, 0x4860512d);
		Panther_Set_RF_Reg(handle, 6, 0x80108240);
		Panther_Set_RF_Reg(handle, 7, 0x00000000);
	    Panther_Set_RF_Reg(handle,27, 0x0000a321);
		Panther_Set_RF_Reg(handle,23, 0x00007e0c);
	    Panther_Set_RF_Reg_Bits(handle, 23, 0x00, 8, 8);	     // pll_soft_resetb = 0;	
	    Panther_Set_RF_Reg_Bits(handle, 23, 0x01, 8, 8);	     // pll_soft_resetb = 1;
		Panther_Set_RF_Reg_Bits(handle, 1, 1, 11, 11);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 0, 1, 2, 2);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 1, 1, 0, 0);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 28, 0, 0, 0);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 28, 1, 0, 0);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 28, 0, 0, 0);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 28, 1, 1, 1);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 28, 3, 6, 5);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 28, 0, 8, 7);
		udelay(delay);
		Panther_Set_RF_Reg_Bits(handle, 1, 0, 11, 11);
		udelay(delay);

	   // --------------------------------------------------------------------------------

	panther_rf_reg_dump();
	return TRUE;
}


/*RF Set PLL*/
BOOL mt_Panther_RF_set_pll(MT_Panther_Handle handle, U32 ulLoFreq)
{
#ifdef PANTHER_ASIC_INIT_TEST
//  BOOL bOK = TRUE;

	U32	FLO = 0x00;
//	U8	RDIV = 1;   // Fref = 40/1;
	U8	channel_val = 0;
//	U32	NF16 = 0x00;
//  DB	Fvco = 0;
//  U8	buf = 0;
//  U32	buf32 = 0,
	U32 iResult = 1;
//  U32 ulData = 0;
#else
	BOOL bOK = TRUE;

	U32	FLO = 0x00;
	U8	RDIV = 1;   // Fref = 40/1;
	U8	channel_val = 0;
	U32	NF16 = 0x00;
	DB	Fvco = 0;
	U8	buf = 0;
	U32	buf32 = 0,
	iResult = 1;
	U32 ulData = 0;
#endif


	if(handle == NULL)
	{
		return FALSE;
	}

	handle->ulLoFreqMHz = ulLoFreq;
	//FLO = handle->tuner_int_FLO_MHz;	// from FLO_MHz in the GUI
	FLO = (U32)ulLoFreq;
	channel_val = FLO - 2400;
//  Fvco = FLO * 1.5;					// output to GUI;
//  handle->ulFVCOFreqMHz = (U32)Fvco;

//	NF16 = FLO * RDIV;
/*
	Panther_Get_Reg(handle, 0x000C085c, &ulData);
	ulData = SetSeveralBits2Data(ulData, 0x00, 12, 12);
	Panther_Set_Reg(handle, 0x000C085c, ulData);				// RF_pd = 0

	Panther_Set_RF_Reg_Bits(handle, 0, 0x03, 1, 0);			// xpd_vco,xpd_pll;
	Panther_Set_RF_Reg_Bits(handle, 0, 1, 12, 12);				// xpd_lodiv;
	Panther_Set_RF_Reg_Bits(handle, 1, 1, 0, 0);				// reg_ctrl_mode;
	Panther_Set_RF_Reg_Bits(handle, 25, 1, 8, 8);				// XTAL_EN_CTRL = 1
	Panther_Set_RF_Reg_Bits(handle, 25, 1, 6, 6);				// PLL_EN_CTRL = 1
	*/

	// --------  pll setting ----------------
		/*
    Panther_Set_RF_Reg(handle, 2, 0x77486fcc); // ?? ngm and ngm_ci??
    Panther_Set_RF_Reg(handle, 3, 0x00cc8d00);
	Panther_Set_RF_Reg(handle, 4, 0x000000ca);
	Panther_Set_RF_Reg(handle, 5, 0x4860512d);
	Panther_Set_RF_Reg(handle, 6, 0x80108240);
	Panther_Set_RF_Reg(handle, 7, 0x00000000);
    Panther_Set_RF_Reg(handle,27, 0x0000a321);
	// Panther_Set_RF_Reg(handle,23, 0x00007e0c);
    Panther_Set_RF_Reg_Bits(handle, 23, 0x00007e, 31, 8);
	// Panther_Set_RF_Reg_Bits(handle, 23, 0x00, 8, 8);	     // pll_soft_resetb = 0;	
    // Panther_Set_RF_Reg_Bits(handle, 23, 0x01, 8, 8);	     // pll_soft_resetb = 1;
		*/
	// note:
	// if using iph for vco_bias: rf_addr2[26:25]=<00>; rf_addr2[17:16]=<11>; 
	// if using vtr for vco_bias: rf_addr2[26:25]=<11>; rf_addr2[17:16]=00/01/10/11; 

    Panther_Set_RF_Reg_Bits(handle, 2, 0x01, 24, 24);	     // vco_pdt_en = 1;
	Panther_Set_RF_Reg_Bits(handle, 23, 0x00, 12, 12);	     // pll_data_update_en = 0;
	Panther_Set_RF_Reg_Bits(handle, 27, 0x01, 13, 13);	     // vco_aac_monitor_en = 1;
	if (handle->bLoBpfBypassEn)   //???
	{
		Panther_Set_RF_Reg_Bits(handle, 6, 1, 16, 16);				// lobpf_bypass_en(sm)=1;
		Panther_Set_RF_Reg_Bits(handle, 2, 0, 13, 13);				// lobpf_en=0;
        Panther_Set_RF_Reg_Bits(handle, 22, 0x15, 20, 16);			//0x13 (if Reg22[26]==0)
	}
	else
	{
		Panther_Set_RF_Reg_Bits(handle, 6, 0, 16, 16);				// lobpf_bypass_en(sm)=1;
		Panther_Set_RF_Reg_Bits(handle, 2, 1, 13, 13);				// lobpf_en=0;
		Panther_Set_RF_Reg_Bits(handle, 22, 0x07, 20, 16);			//
	}
    Panther_Set_RF_Reg_Bits(handle, 23, channel_val, 7, 0);	 // FLO=2400+channel_sel;	
    Panther_Set_RF_Reg_Bits(handle, 23, 0x00, 10, 10);	     // pll_ch_update = 0;	
    Panther_Set_RF_Reg_Bits(handle, 23, 0x01, 10, 10);	     // pll_ch_update = 1;

    Panther_Set_RF_Reg_Bits(handle, 23, 0x00, 12, 12);	     // pll_data_update_en = 1;
    Panther_Set_RF_Reg_Bits(handle, 27, 0x00, 13, 13);	     // vco_aac_monitor_en = 0;
    Panther_Set_RF_Reg_Bits(handle, 2, 0x00, 24, 24);	     // vco_pdt_en = 0;

	return (iResult == 1) ? TRUE : FALSE;
}


void mt_Panther_Set_TX(MT_Panther_Handle handle)
{
	Panther_Set_RF_Reg(handle, 0, 0x59F2EB);
	Panther_Set_RF_Reg(handle, 1, 0xB8000311);
	Panther_Set_RF_Reg(handle, 8, 0x0);
	Panther_Set_RF_Reg(handle, 13, 0x72495AB1);
	Panther_Set_RF_Reg(handle, 16, 0x5E2296);
	Panther_Set_RF_Reg(handle, 20, 0x2620218);
	Panther_Set_RF_Reg(handle, 21, 0x40689);
	Panther_Set_RF_Reg(handle, 29, 0x3BC9);

	return;
}

void mt_Panther_Set_RX(MT_Panther_Handle handle)
{
	Panther_Set_RF_Reg(handle, 0, 0x59B30F);
	Panther_Set_RF_Reg(handle, 1, 0x3000311);
	Panther_Set_RF_Reg(handle, 8, 0x0);
	Panther_Set_RF_Reg(handle, 13, 0x72495AB1);
	Panther_Set_RF_Reg(handle, 16, 0x5E2296);
	Panther_Set_RF_Reg(handle, 20, 0x2620218);
	Panther_Set_RF_Reg(handle, 21, 0x40689);

	return;
}




void mt_Panther_set_TS_Mux_mode(MT_Panther_Handle handle, U8 iTsMuxIndex)
{
//	U8 TST_EN1, TST_EN2_I, TST_EN2_Q, TST_EN3, TST_EN4, TST_EN5, TST_EN6, TST_EN7_I, TST_EN7_Q, TST_EN8, testbuf_en, tstbuf_sel1, tstbuf_sel0;
	U8 TST_EN1 = 0, TST_EN3 = 0, TST_EN4 = 0, TST_EN5 = 0, TST_EN6 = 0, TST_EN7 = 0, TST_EN8 = 0;
	U8 bb_rx_cali_mode = 0, bb_tx_cali_mode = 0;
//  U8 ucTmp = 0;
	U16 usTmp = 0;

	U32 ulData08 = 0, ulData10 = 0, ulData18 = 0, ulData = 0;

	// Set ATE TEST_REG	0x000C_0834[22:20] = 001
//	Panther_Get_Reg(handle, 0x000C0834, &ulData);
//	ulData = SetSeveralBits2Data(ulData, 1, 22, 20);
//	Panther_Set_Reg(handle, 0x000C0834, ulData);

	// Set ATE TEST_REG 0xBF00_4AFC[7:4] = 0001
	Panther_Get_Reg(handle, 0xBF004AFC, &ulData);
	ulData = SetSeveralBits2Data(ulData, 1, 7,4);
	Panther_Set_Reg(handle, 0xBF004AFC, ulData);

	handle->iTSMuxMode = iTsMuxIndex;

	switch(iTsMuxIndex)
	{
		case 0:			// 	Normal operation mode
			TST_EN1		 = 0;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 0;
			TST_EN6		 = 1;
			TST_EN7		 = 0;
			TST_EN8		 = 1;
			bb_rx_cali_mode = 0;
			bb_tx_cali_mode = 0;
			break;

		case 1:			//	Rx test mode
			TST_EN1		 = 1;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 0;
			TST_EN6		 = 0;
			TST_EN7		 = 0;
			TST_EN8		 = 1;
			bb_rx_cali_mode = 0;
			bb_tx_cali_mode = 0;
			break;

		case 2:			//	Rx test mode2
			TST_EN1		 = 1;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 0;
			TST_EN6		 = 0;
			TST_EN7		 = 0;
			TST_EN8		 = 0;
			bb_rx_cali_mode = 1;
			bb_tx_cali_mode = 0;
			break;
			
		case 3:			//	Tx test mode
			TST_EN1		 = 0;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 1;
			TST_EN6		 = 0;
			TST_EN7		 = 0;
			TST_EN8		 = 0;
			bb_rx_cali_mode = 0;
			bb_tx_cali_mode = 0;
			break;

		case 4:			//	RX TX_CAL test mode
			TST_EN1		 = 1;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 1;
			TST_EN6		 = 0;
			TST_EN7		 = 0;
			TST_EN8		 = 0;
			bb_rx_cali_mode	= 1;
			bb_tx_cali_mode	= 0;
			break;

		case 5:			//	ADC test mode
			TST_EN1		 = 1;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 0;
			TST_EN6		 = 0;
			TST_EN7		 = 0;
			TST_EN8		 = 0;
			bb_rx_cali_mode	= 0;
			bb_tx_cali_mode	= 0;
			break;

		case 6:			//	DAC test mode
			TST_EN1		 = 0;
			TST_EN3		 = 0;
			TST_EN4		 = 0;
			TST_EN5		 = 1;
			TST_EN6		 = 1;
			TST_EN7		 = 0;
			TST_EN8		 = 0;
			bb_rx_cali_mode = 0;
			bb_tx_cali_mode = 0;
			break;

		default:
			break;
	}

	Panther_Get_RF_Reg(handle, 8, &ulData08);   //bb_rx_cali_mode  8[7]
	Panther_Get_RF_Reg(handle, 10, &ulData10);  //bb_tx_cali_mode  10[21]

	// RF Reg.01 [16]
	//ulData01 = SetSeveralBits2Data(ulData01, testbuf_en, 16, 16);

    //RF Reg.08 [7]
    ulData08 = SetSeveralBits2Data(ulData08, bb_rx_cali_mode, 7, 7);

	//RF Reg.10 [21]
    ulData10 = SetSeveralBits2Data(ulData10, bb_tx_cali_mode, 21, 21);

	// RF Reg.10 [25:24]
	//ucTmp = ((tstbuf_sel1 << 1) + tstbuf_sel0) & 0x03;
	//ulData10 = SetSeveralBits2Data(ulData10, ucTmp, 25, 24);

	// RF Reg.18 [31:22]
	//usTmp = 0;
	//usTmp += TST_EN7_Q;
	//usTmp <<= 1;
	//usTmp += TST_EN7_I;
	usTmp = 0;
    usTmp += TST_EN7;
	usTmp <<= 1;
	usTmp += TST_EN6;
	usTmp <<= 1;
	usTmp += TST_EN5;
	usTmp <<= 1;
	usTmp += TST_EN4;
	usTmp <<= 1;
	usTmp += TST_EN3;
	//usTmp <<= 1;
	//usTmp += TST_EN2_Q;
	//usTmp <<= 1;
	//usTmp += TST_EN2_I;
	usTmp <<= 1;
	usTmp += TST_EN1;
	usTmp <<= 1;
	usTmp += TST_EN8;
	ulData18 = SetSeveralBits2Data(ulData18, usTmp, 31, 22);

	Panther_Set_RF_Reg(handle, 18, ulData18);
	Panther_Set_RF_Reg(handle, 10, ulData10);
	Panther_Set_RF_Reg(handle, 8, ulData08);

	return;
}


void mt_Panther_set_ABB(MT_Panther_Handle handle)
{
	U32 uiData = 0;

	// 0[2]=1,			//bb_xpd: 1-poweron, 0-powerdown
	// 0[3]=1,			//dco_xpd: 1-poweron, 0-powerdown
	// 0[14]=1,			//tx_lpf_xpd: 1-poweron, 0-powerdown
	// 0[30]=1,			//RX_AGC_EN: RX AGC enable; b'0: manual gain control mode, b'1: AGC mode
	// 0[31]=1,			//TX_AGC_EN: TX AGC enable; b'0: manual gain control mode, b'1: AGC mode

	Panther_Get_RF_Reg(handle, 0, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x01, 2, 2);
	uiData = SetSeveralBits2Data(uiData, 0x01, 3, 3);
	uiData = SetSeveralBits2Data(uiData, 0x01, 14, 14);
	uiData = SetSeveralBits2Data(uiData, 0x01, 30, 30);
	uiData = SetSeveralBits2Data(uiData, 0x01, 31, 31);
	Panther_Set_RF_Reg(handle, 0, uiData);

	Panther_Get_RF_Reg(handle, 10, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x01, 26, 26);
    Panther_Set_RF_Reg(handle, 10, uiData);

	// 1[11]=1£¬		//clk_bbcal_en: enable signal of output clk for bb calibration, 1 en able; 0 disable
	// 1[16]=0,			//tstbuf_en: ABB testbuf enable
	// 1[18:17]=00,		//bb_test_en[1:0]: 00-disable£¬01-RXBB_I_channel dc test£¬10-RXBB_Q_channel dc test
	Panther_Get_RF_Reg(handle, 1, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x01, 11, 11);
	uiData = SetSeveralBits2Data(uiData, 0x00, 16, 16);
	uiData = SetSeveralBits2Data(uiData, 0x00, 18, 17);
	Panther_Set_RF_Reg(handle, 1, uiData);

	// 9[1:0]=10,		//tia_cvcm[1:0]
	// 9[2]=0,			//tia_ci:  set iph for tia_op: 0: 20u   1:25u
	// 9[3]=0,			//tx_lpf_test_en
	// 9[5:4]=10,		//bbpga1_cvcm[1:0]
	// 9[8:6]=101,		//bbpga1_cri_ext[2:0]
	// 9[11:9]=001,		//bbpga1_crf_ext[2:0]
	// 9[13:12]=10,		//bbfiter_cvcm[1:0]
	// 9[19:14]=010001,	//bbfilter_csel_ext[5:0]
	// 9[23:20]=0110,	//bbfilter_Rsel_ext[3:0]
	// 9[27:24]=0110,	//bbfilter_rsel_ext[3:0]
	// 9[30:28]=010,	//tx_lpf_cg_ext[2:0]
	// 9[31]=0,			//tx_lpf_ci£ºset iph for opamp: 0: 20u   1:25u
	Panther_Get_RF_Reg(handle, 9, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x02, 1, 0);
	uiData = SetSeveralBits2Data(uiData, 0x00, 2, 2);
	uiData = SetSeveralBits2Data(uiData, 0x00, 3, 3);
	uiData = SetSeveralBits2Data(uiData, 0x02, 5, 4);
	uiData = SetSeveralBits2Data(uiData, 0x05, 8, 6);
	uiData = SetSeveralBits2Data(uiData, 0x01, 11, 9);
	uiData = SetSeveralBits2Data(uiData, 0x02, 13, 12);
	uiData = SetSeveralBits2Data(uiData, 0x11, 19, 14);
	uiData = SetSeveralBits2Data(uiData, 0x06, 23, 20);
	uiData = SetSeveralBits2Data(uiData, 0x06, 27, 24);
	uiData = SetSeveralBits2Data(uiData, 0x02, 30, 28);
	uiData = SetSeveralBits2Data(uiData, 0x00, 31, 31);
	Panther_Set_RF_Reg(handle, 9, uiData);

	// 10[5:0]=010001,		//bbcali_csel_set[5:0]: set the default value in the counter
	// 10[7:6]=10,			//bbpga2_vcm[1:0]
	// 10[12:8]=01010,		//bbpga2_cri_ext[4:0]
	// 10[14:13]=00,		//bbpga2_crf_ext[1:0]
	// 10[20:15]=000000,	//bb_ci[5:0]: set iph for bb_op:0: 20u   1:25u
	// 10[21]=0,			//bb_mode: 0: normal mode£¬1: tx_cali mode
	// 10[22]=0,			//bbpga2_ci: current programmable for test mode, 0:I0, 1:5*I0
	// 10[23]=0,			//bb_bw: 0£º20MHz mode£¬1£º40MHz mode
	// 10[25:24]=00,		//tstbuf_sel[1:0]: test buffer select signal 00: TIA I channel, 01: bb I channel, 10: TIA Q channel, 11: bb Q channel
	// 10[26]=0,			//stress_pro
	// 10[31:30]=10,		//tx_lpf_cv[1:0]: set the CM for filter 0: 0.671V, 1: 0.71V, 2: 0.75V, 3: 0.79V
	Panther_Get_RF_Reg(handle, 10, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x11, 5, 0);
	uiData = SetSeveralBits2Data(uiData, 0x02, 7, 6);
	uiData = SetSeveralBits2Data(uiData, 0x0A, 12, 8);
	uiData = SetSeveralBits2Data(uiData, 0x00, 20, 15);
	uiData = SetSeveralBits2Data(uiData, 0x00, 21, 21);
	uiData = SetSeveralBits2Data(uiData, 0x00, 22, 22);
	uiData = SetSeveralBits2Data(uiData, 0x00, 23, 23);
	uiData = SetSeveralBits2Data(uiData, 0x00, 25, 24);
	uiData = SetSeveralBits2Data(uiData, 0x00, 26, 26);
	uiData = SetSeveralBits2Data(uiData, 0x02, 31, 30);
	Panther_Set_RF_Reg(handle, 10, uiData);

	// 11[5:0]=000000,		//dco_dac_ip_ext[5:0]: dac input data for p port of I channel
	// 11[11:6]=000000,		//dco_dac_in_ext[5:0]: dac input data for n port of I channel
	// 11[17:12]=000000,	//dco_dac_qp_ext[5:0]: dac input data for p port of Qchannel
	// 11[23:18]=000000,	//dco_dac_qn_ext[5:0]: dac input data for n port of Q channel
	// 11[25:24]=01,		//dco_dac_lsb_ext[1:0]: select iunit: 0-5u 1-2.5u 2-1.67u 3-1.25u;
	// 11[28:26]=010,		//dco_dac_lsb_track_ext[2:0]: LSB£º1-113.6nA, 2-151.5nA, 3-227.3nA, 4-343.3nA, 5-500nA
	// 11[30:29]=11,		//dco_polar_ext
	Panther_Get_RF_Reg(handle, 11, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x00, 5, 0);
	uiData = SetSeveralBits2Data(uiData, 0x00, 11, 6);
	uiData = SetSeveralBits2Data(uiData, 0x00, 17, 12);
	uiData = SetSeveralBits2Data(uiData, 0x00, 23, 18);
	uiData = SetSeveralBits2Data(uiData, 0x01, 25, 24);
	uiData = SetSeveralBits2Data(uiData, 0x02, 28, 26);
	uiData = SetSeveralBits2Data(uiData, 0x03, 30, 29);
	Panther_Set_RF_Reg(handle, 11, uiData);

	// 12[30]=0,			//tia_cr_tx_cali
	// 12[31]=0,			//bbpga1_ci: current programmable for test mode, 0:I0, 1:5*I0
	Panther_Get_RF_Reg(handle, 12, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x00, 30, 30);
	uiData = SetSeveralBits2Data(uiData, 0x00, 31, 31);
	Panther_Set_RF_Reg(handle, 12, uiData);

	// 18[31:22]=00 1001 0000,	//Test mode switch
	Panther_Set_RF_Reg_Bits(handle, 18, 0x90, 31, 22);

	// 25[7]=1,				//ABB_EN_CTRL: b'0: On/Off control by Slave-LDO, b'1: On/Off control by block's enable pin
	Panther_Set_RF_Reg_Bits(handle, 25, 0x01, 7, 7);

	// 26[0]=1,				//dcoc_cal_done: DCOC Calibration done; b'1: calibration done and write DAC code into internal register
	// 26[1]=1,				//dcoc_dac_ctrl: DCOC DAC code control mode; b'0: manual mode, b'1: calibration code mode
	Panther_Get_RF_Reg(handle, 26, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x01, 0, 0);
	uiData = SetSeveralBits2Data(uiData, 0x01, 1, 1);
	Panther_Set_RF_Reg(handle, 26, uiData);

	// 28[0]=0,				//cal_soft_reset: b'1: reset
	// 28[1]=1,				//wake_up: b'1: wake up
	// 28[5]=1,				//lpf_cal_start_ext
	// 28[6]=1,				//lpf_cal_wakeup_ext
	// 28[8:7]=00,			//bbcali_cap_ext_en[1:0]
	// 28[14:9]=010001,		//bbcali_cap_ext[5:0]: from CPU, analog name is bbcali_csel_set, cal top name is bbcali_cap_ext
	// 28[15]=1,			//bbcali_bw_ext_en
	Panther_Get_RF_Reg(handle, 28, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x00, 0, 0);
	uiData = SetSeveralBits2Data(uiData, 0x01, 1, 1);
	uiData = SetSeveralBits2Data(uiData, 0x01, 5, 5);
	uiData = SetSeveralBits2Data(uiData, 0x01, 6, 6);
	uiData = SetSeveralBits2Data(uiData, 0x00, 8, 7);
	uiData = SetSeveralBits2Data(uiData, 0x11, 14, 9);
	uiData = SetSeveralBits2Data(uiData, 0x01, 15, 15);
	Panther_Set_RF_Reg(handle, 28, uiData);

	// 0x000C_0A24[1]=0,	//ADC internal Vcm disable
	Panther_Get_Reg(handle, 0xBF004C24, &uiData);
	uiData = SetSeveralBits2Data(uiData, 0x00, 1, 1);
	Panther_Set_Reg(handle, 0xBF004C24, uiData);

	panther_rf_reg_dump();
	return;
}

