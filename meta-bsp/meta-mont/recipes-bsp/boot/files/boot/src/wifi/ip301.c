/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file ip301.c
*   \brief  ip301 driver.
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <panther_dev.h>
#include <panther_rf.h>
#include <rf.h>
#include <rfc.h>
#include <rfac.h>
#include <ip301.h>
#include <panther_debug.h>
#include <mac_ctrl.h>
#include <arch/chip.h>
#include <bb.h>
#include <os_compat.h>

#include "mac_regs.h"

#define ADC_CTRL                       0x10
       #define DAC_DIVSEL_80M                  0x00000008
       #define ADC_DIVSEL_240M                 0x00000004

#define PKG_MODE_CTRL          0x34
#define ATE_MODE                        0x000000E0
#define TEST_EN                         0x00000080

int printf(char *fmt, ...);

#ifdef DRAGONITE_RFC_DEBUG
#define RFDBG(_l, _str...) 		DBG_PRINTF(_l, _str)
#else
#define RFDBG(_l, _str...)
#endif
extern struct panther_dev *ldev;
void mt301_reg_write_w6(unsigned char reg, unsigned int data)
{
	int timeout = 0, dst;
	u32 rdata, mask=0;
	u32 value=0;
//	RFDBG(INFO,"mt301_reg_write_w6 %02x\t%04x\n", reg, data);
//	bb_register_write(0, 0xA4, (0xFF & (data >> 8)));
	dst = 0xA4;
	rdata = BBREG_READ32(dst);
	mask = 0x000000ffUL;
	value = 0xff & (data >> 8);
	rdata = ((rdata & ~(mask) ) | ( (value & mask) ));

//	bb_register_write(0, 0xA5, (0xFF & data));
	mask = 0x0000ff00UL;
	value = 0xff & data;
	rdata = ( (rdata & ~(mask)) | ((value << 8) & (mask)) );

	BBREG_WRITE32(dst, rdata);

//	bb_register_write(0, 0xA2, reg);
	dst = 0xA0;
	rdata = BBREG_READ32(dst);
	mask = 0x00ff0000UL;
	value = 0xff & reg;
	rdata = ( (rdata & ~(mask)) | ((value << 16) & (mask)) );

//	bb_register_write(0, 0xA3, (0xFF & (data >> 16)));
	mask = 0xff000000UL;
	value = 0xff & (data >> 16);
	rdata = ( (rdata & ~(mask)) | ((value << 24) & (mask)) );
	BBREG_WRITE32(dst, rdata);
	
#if 0
	bb_register_write(0, 0xA3, (0xFF & (data >> 16)));
	bb_register_write(0, 0xA4, (0xFF & (data >> 8)));
	bb_register_write(0, 0xA5, (0xFF & data));

	bb_register_write(0, 0xA2, reg);
#endif
	if(reg==2)
		RFDBG(INFO, "=====> mt301 write reg %x, data %x\n", reg, data);

    /* read BRA2 and get msb bit (like operation &0x80) until the result is 0x0 */
	while(bb_register_read(0, 0xA2) & 0x80)
	{
		if(timeout++>100000)
		{
			RFDBG(ERROR, "===> mt301 write fail!!! reg %x, data %x\n", reg, data);
			return;
		}
	}

	return;
}

unsigned int mt301_reg_read_w6(unsigned char reg)
{
	int timeout = 0;
	unsigned int data;

	bb_register_write(0, 0xA2, (reg | 0x20));

    /* read BRA2 and get msb bit (like operation &0x80) until the result is 0x0 */
	while(bb_register_read(0, 0xA2) & 0x80)
	{
		if(timeout++>100000)
		{
			RFDBG(ERROR, "===>mt301 read fail!!! reg %x\n", reg);
			return -2;
		}
	}

	data = ((bb_register_read(0, 0xA6) << 16) | (bb_register_read(0, 0xA7) << 8) | bb_register_read(0, 0xA8));
//	RFDBG(INFO, "mt301_reg_read_w6 %02x\t%04x\n", reg, data);
	return data;
}


struct channel{
	u16	num;
	u16 freq;
	u32	reg_val;
} mt301_channel_data[] = {
	{ 1, 2412, 0x02864b },
    { 2, 2417, 0x0288cb },
    { 3, 2422, 0x028b4b },
    { 4, 2427, 0x028dcb },
    { 5, 2432, 0x02804c },
    { 6, 2437, 0x0282cc },
    { 7, 2442, 0x02854c },
    { 8, 2447, 0x0287cc },
    { 9, 2452, 0x028a4c },
    { 10, 2457, 0x028ccc },
    { 11, 2462, 0x028f4c },
    { 12, 2467, 0x0281cd },
    { 13, 2472, 0x02844d },
    { 14, 2484, 0x028a4d },
	{ 255, 2520, 0x028c4e },
};

#define CHANNEL_WIDTH_20MHZ		0
#define CHANNEL_WIDTH_40MHZ		1


void mt301_set_40mhz_channel(int channel_num, int mode)
{
	int primary_ch_freq;
	int channel_freq;
	int i;

	for(i=0;i<(sizeof(mt301_channel_data)/sizeof(mt301_channel_data[0]));i++)
	{
		if(mt301_channel_data[i].num == channel_num)
		{
			break;
		}
	}

	if(i >= sizeof(mt301_channel_data)/sizeof(mt301_channel_data[0]))
		return;

	primary_ch_freq = mt301_channel_data[i].freq;

	if(mode == BW40MHZ_SCN)
	{
		rf_write(0x2, mt301_channel_data[i].reg_val);
		/* for mt301_g patch (tx filter code calibration has problem) */
		if((ldev->rf.chip_ver == RFCHIP_MT301_A0) || (ldev->rf.chip_ver == RFCHIP_MT301_A1))
			rf_write(0x8, rf_read(0x8) & 0xfffff3ff);
		else
			rf_write(0x8, rf_read(0x8) & 0xffff3fff);
#if !defined(CONFIG_FPGA)
		PLLREG(ADC_CTRL) |= (DAC_DIVSEL_80M|ADC_DIVSEL_240M);
#endif
		return;
	}
 
	if(mode == BW40MHZ_SCA)
	{
		channel_freq = primary_ch_freq + 10;
	}
	else if(mode == BW40MHZ_SCB)
	{
		channel_freq = primary_ch_freq - 10;
	}
        else
        {
		channel_freq = primary_ch_freq;
        }

	for(i=0;i<(sizeof(mt301_channel_data)/sizeof(mt301_channel_data[0]));i++)
	{
		if(mt301_channel_data[i].freq == channel_freq)
		{
			break;
		}
	}

	if(i >= sizeof(mt301_channel_data)/sizeof(mt301_channel_data[0]))
		return;

	rf_write(0x2, mt301_channel_data[i].reg_val);
	/* for mt301_g patch (tx filter code calibration has problem) */
	if((ldev->rf.chip_ver == RFCHIP_MT301_A0) || (ldev->rf.chip_ver == RFCHIP_MT301_A1))
		rf_write(0x8, rf_read(0x8) | 0x0C00);
	else
		rf_write(0x8, rf_read(0x8) | 0xC000);
#if !defined(CONFIG_FPGA)
	PLLREG(ADC_CTRL) &= ~(DAC_DIVSEL_80M|ADC_DIVSEL_240M);
#endif

	return;
}

int mt301_recover(void)
{
	int ext_rf = 0;

	if ((PMUREG(PKG_MODE_CTRL) & ATE_MODE) == TEST_EN)
		ext_rf = 1;
	rf_write(0x0, 0x3E954);
	rf_write(0x1, 0x8000);
	rf_write(0x6, 0x10BD);
	rf_write(0x16, 0x12626); /* Disable internal PA, it will degrade B-mode throughput */
	rf_write(0x17, 0x30A40);
	rf_write(0x19, 0x3555);
	//rf_write(0x7, 0x1CC0);
	if (ext_rf)
 		rf_write(0x7, 0x3CC0); /* increase TX power */
	else
		rf_write(0x7, 0x2CC0); /* increase TX power */
	rf_write(0x9, 0x19040);
	if (ext_rf)
		rf_write(0x11, 0x4D12);
	else
		rf_write(0x11, 0x4D02);

    /* maximum bandwidth of rxhp to reduce rxhp setting time */
	rf_write(0x15, 0x3000);

	bb_register_write(0, 0x02, 0x31); // Sam and Jerry Asked
	return 0;
}

//const unsigned short mt301_gain_level[] = {0x58C0, 0x3CC0, 0x3CC0, 0x3CC0 /* for test */};
/* level 0 means 15dbm */
const unsigned short mt301_gain_level[] =
#if 0 
		{	0x54C0, /* default */ 
			0x16C0, /* 1dbm */ 
			0x18C0, 
			0x1AC0, 
			0x1EC0, 
			0x20C0, 
			0x22C0, 
			0x26C0, 
			0x28C0, 
			0x30C0, 
			0x32C0, 
			0x3CC0, 
			0x40C0, 
			0x46C0, 
			0x4CC0, 
			0x54C0, 
			0x60C0,
			0x64C0	/* 17dbm */
		};
#else /* FIXME: update it for new RF */
		{	0x2CC0, /* default */ 
			0x2CC0, /* 1dbm */ 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0, 
			0x2CC0,
			0x2CC0	/* 17dbm */
		};
#endif
const unsigned short mt301_gain_level_ext_rf[] =
/* FIXME: update it for new RF */
{	0x3CC0, /* default */ 
	0x3CC0, /* 1dbm */ 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0, 
	0x3CC0,
	0x3CC0	/* 17dbm */
};

void mt301_set_tx_power(unsigned int level)
{
	u16 val;
	const unsigned short *gain_level;

	if ((PMUREG(PKG_MODE_CTRL) & ATE_MODE) == TEST_EN)
		gain_level = mt301_gain_level_ext_rf;
	else
		gain_level = mt301_gain_level;

	if(level < (sizeof(mt301_gain_level)/sizeof(unsigned short)))
		val = gain_level[level];
	else
		val = gain_level[0];
	rf_write(0x7, val);
}

unsigned int mt301_get_tx_power(void)
{
	unsigned int i;
	u32 val = rf_read(0x7);
	const unsigned short *gain_level;

	if ((PMUREG(PKG_MODE_CTRL) & ATE_MODE) == TEST_EN)
		gain_level = mt301_gain_level_ext_rf;
	else
		gain_level = mt301_gain_level;

	for(i=0; i<(sizeof(mt301_gain_level)/(sizeof(unsigned short))); i++)
	{
		if(gain_level[i] == val)
                    break;
	}

	/* FIXME: does need to handle (i >= 18)? */

	return i;
}

int mt301_init(void)
{
	int ret = 0;
	printf("mt301_init\n");
#if defined(CONFIG_RFC)
	ret = mt301_calibration(7, 1);
#endif

    if(0>ret)
        goto Failed;
#if defined(CONFIG_RFC)

	mt301_recover();

	rfc_process_new(0);
	
#endif

	mt301_recover();

    RFDBG(INFO, "RF init MT301-%d done\n", ldev->rf.chip_ver);

#if 0
	if(IP301_NORMAL_MODE != rf_read(0x0))
		WLA_DBG(WLAERROR, "????????????????????????????????IP301 REG0 has abnormal value, 0x%02x\n", rf_read(0x0));
#endif

    return 0;

Failed:
    printk("RF init MT301 failed\n");
    RFDBG(ERROR, "RF init MT301-%d failed\n", ldev->rf.chip_ver);
    return ret;
}

#define TXLOOP 0
#define RXLOOP 1
#define NON_LOOP 2

#if 1
u32 txcaltxvga = 0;
u32 txcalrxvga = 0;
u32 rxcaltxvga = 0;
u32 rxcalrxvga = 0;
#endif

void mt301_set_iqcal(u8 loop_type)
{
    RFDBG(INFO, "MANUAL txcaltxvga %x, txcalrxvga %x, rxcaltxvga %x, rxcalrxvga %x\n",
    		txcaltxvga, txcalrxvga, rxcaltxvga, rxcalrxvga);

	if (TXLOOP == loop_type)
	{
		//Configure TX LOOPBACK
		RFDBG(INFO, "Set mt301 to tx loopback mode\n");
		
		rf_write(0x0, 0x3ffe0);

		// RX VGA gain setting for TX Calibration
		if(txcalrxvga)
		{
			rf_write(0x9, txcalrxvga);
		}
		else
		{
			rf_write(0x9, 0x7c);
		}

		// TX VGA gain setting for TX Calibration
		if(txcaltxvga)
		{
			rf_write(0x7, txcaltxvga);
		}
		else
		{
			rf_write(0x7, 0xa1f8);
		}

		// TX IQ Calibration
		rf_write(0x1, 0xa2a);
	}
	else if  (RXLOOP == loop_type)
	{
		//Configure RX LOOPBACK
		RFDBG(INFO, "Set mt301 to rx loopback mode\n");

		rf_write(0x0, 0x3fffe);
	
		// RX VGA gain setting for RX Calibration
		if(rxcalrxvga)
		{
			rf_write(0x9, rxcalrxvga);
		}
		else
		{
			{    
				//rf_write(0x9, 0x8c);
				rf_write(0x9, ((9 * 2) << 3));
			}
		}

		// TX VGA gain setting for RX Calibration
		if(rxcaltxvga)
		{
			rf_write(0x7, rxcaltxvga);
		}
		else
		{
			{    
				rf_write(0x7, 0x65f8);
				rf_write(0x7, (12 << 9) | (rf_read(0x7) & 0x1ff));
			}
		}
		
		// RX IQ Calibration
		rf_write(0x1, 0x0629);
	}
	else
		RFDBG(WARN, "Not supported loopback type!\n");
}

void mt301_set_iqcal_vga(u8 loop_type, u32 rxvga, u32 txvga)
{
    //RFDBG(INFO, "MANUAL txvga %x, rxvga %x\n", txvga, rxvga);
	int txvga_gain_code[11] = { 12, 15, 19, 24, 30, 37, 47, 60, 76, 96, 127, };
	//u32 reg0;
	u32 reg9_o = rf_read(0x9);
	u32 reg9 = reg9_o & 0xFFFFFE07;
	u32 reg7_o = rf_read(0x7);

	//reg0 = rf_read(0x0);
	if (TXLOOP == loop_type)
	{
		//Configure TX LOOPBACK
		//RFDBG(INFO, "Set mt301 to tx loopback mode\n");
		
		if((ldev->rf.chip_ver != RFCHIP_MT301_A0) && (ldev->rf.chip_ver != RFCHIP_MT301_A1)) 
		{
			// rx lo buffer on (reg.0 bit.2), verified by KC
			rf_write(0x0, 0x3ffe4);
		}

		// RX VGA gain setting for TX Calibration
		if(txcalrxvga)
		{
			rf_write(0x9, txcalrxvga | reg9);
		}
		else
		{
			if(rxvga)
			{
				if((ldev->rf.chip_ver != RFCHIP_MT301_A0) && (ldev->rf.chip_ver != RFCHIP_MT301_A1)) 
				{
					rf_write(0x9, ((rxvga*2) << 3) | reg9);
				}
				else
				{
					/* if ((BW==40Mhz)&&(in tone = 18Mhz)) rxvga = 24;
					   else rxvga = 18;
					   rxvga : unit of dB, located at reg9[8:3] 		*/
					rf_write(0x9, (reg9_o & 0x3fe07) | (rxvga*2 << 3));
				}
			}
			else
			{
				rf_write(0x9, 0x7c | reg9);
			}
		}

		// TX VGA gain setting for TX Calibration
		if(txcaltxvga)
		{
			rf_write(0x7, txcaltxvga);
		}
		else
		{
			if((ldev->rf.chip_ver == RFCHIP_MT301_A0) || (ldev->rf.chip_ver == RFCHIP_MT301_A1)) 
			{
				//  txvga = 60; for all conditions
				//  located at reg7[15:9]
				rf_write(0x7, (reg7_o& 0x301ff) | (txvga_gain_code[txvga] <<9));
			}
			else
			{
				//rf_write(0x7, ((txvga_gain_code[txvga] << 9) | (rf_read(0x7) & 0x1ff)));
				rf_write(0x7, ((txvga_gain_code[txvga] << 9) | 0x1f8));
			}
		}

		// TX IQ Calibration
		if((ldev->rf.chip_ver != RFCHIP_MT301_A0) && (ldev->rf.chip_ver != RFCHIP_MT301_A1)) 
		{
			//rf_write(0x1, 0xa2a);
			rf_write(0x1, (rf_read(0x1) & 0xfffff3fc) | 0x802);
		}	
		else
		{
			rf_write(0x15, 0x0);
			rf_write(0x19, 0x3555);
			rf_write(0x0,  0x3ffe0);
			rf_write(0x1,  0x8802);
			rf_write(0x17, 0x0);
		}
	}
	else if  (RXLOOP == loop_type)
	{
		//Configure RX LOOPBACK
		//RFDBG(INFO, "Set mt301 to rx loopback mode\n");

		// all blocks on for tx & rx, verified by KC
		if((ldev->rf.chip_ver != RFCHIP_MT301_A0) && (ldev->rf.chip_ver != RFCHIP_MT301_A1)) 
		{
			//rf_write(0x0, 0x3ffff);
			rf_write(0x0, 0x3fffe);
		}
	
		// RX VGA gain setting for RX Calibration
		if(rxcalrxvga)
		{
			rf_write(0x9, rxcalrxvga | reg9);
		}
		else
		{
			if(rxvga)
			{
				if((ldev->rf.chip_ver != RFCHIP_MT301_A0) && (ldev->rf.chip_ver != RFCHIP_MT301_A1)) 
					rf_write(0x9, ((rxvga*2) << 3) | reg9);
				else
				{
					// rxvga : unit of dB, located at reg9[8:3], Lab test : rxvga=20
			        rf_write(0x9, (reg9_o & 0x3fe07) | (rxvga*2 << 3));
				}
			}
			else
			{
				if((ldev->rf.chip_ver == RFCHIP_MT301_A0) || (ldev->rf.chip_ver == RFCHIP_MT301_A1))
				{
					/* RF CHIP J (MT301) doesn't need this step */
				}
				else
				{    
					//rf_write(0x9, 0x8c);
					rf_write(0x9, ((9 * 2) << 3) | reg9);
				}
			}
		}

		// TX VGA gain setting for RX Calibration
		if(rxcaltxvga)
		{
			rf_write(0x7, rxcaltxvga);
		}
		else
		{
			if((ldev->rf.chip_ver == RFCHIP_MT301_A0) || (ldev->rf.chip_ver == RFCHIP_MT301_A1)) 
			{
				//  Lab test : txvga = 10;
				//  located at reg7[15:9]
				rf_write(0x7, (reg7_o& 0x301ff) | (txvga_gain_code[txvga] <<9));
			}
			else
			{   
				//rf_write(0x7, 0x65f8);
				//rf_write(0x7, ((txvga_gain_code[txvga] << 9) | (rf_read(0x7) & 0x1ff)));
				rf_write(0x7, ((txvga_gain_code[txvga] << 9) | 0x1f8));
			}
		}
		
		// RX IQ Calibration
		if((ldev->rf.chip_ver != RFCHIP_MT301_A0) && (ldev->rf.chip_ver != RFCHIP_MT301_A1)) 
		{
			//rf_write(0x1, 0x0629);
			rf_write(0x1, (rf_read(0x1) & 0xfffff3fc) | 0x401);
		}
		else
		{
			rf_write(0x15, 0x0);
			rf_write(0x19, 0x3555);
			rf_write(0x0,  0x3fffe);
			rf_write(0x1,  0x8409);
			rf_write(0x17, 0x0);
		}
	}
	else if(NON_LOOP == loop_type)
	{
		/* Disable TX/RX loopback function */
		//rf_write(0x0, 0x3e954);
	}
	else
		RFDBG(WARN, "Not supported loopback type!\n");

}
