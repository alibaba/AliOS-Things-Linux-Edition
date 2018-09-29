/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wla_rf.c
*   \brief  wla wlan RF control function.
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <panther_debug.h>
#include <mac_ctrl.h>
#include <rf.h>
#include <ip301.h>
#include <bb.h>
#include <panther_rf.h>
#include <rfc.h>
#include <panther_dev.h>
#include <rfc_panther.h>

#include "mac_regs.h"

int printf(char *fmt, ...);

struct rf_regs rf_tbl[MAX_RF_REG_NUM];

extern unsigned int *rxdc_rec_ptr;
struct panther_dev *ldev;
struct panther_dev _ldev;

#ifdef RFC_I2C_TEST
extern unsigned int panther_i2c_rf_read(char reg);
void panther_i2c_rf_write(char reg, unsigned int val);
void panther_i2c_rf_update(char reg, unsigned int val, unsigned int mask);
#endif

unsigned int rf_read(char reg)
{
#if defined(CONFIG_MONTE_CARLO)
	//serial_printf("%s(type=%d): reg=%d\n", __FUNCTION__, ldev->rf.if_type, reg);
#else
//  serial_printf("Panther:%s(type=%d):  reg=%d, val=0x%x\n", __FUNCTION__, ldev->rf.if_type, reg, RFREG_READ_DIRECT(reg));
#endif

	if(ldev->rf.if_type == RF_CTRL_DIRECT)
		return RFREG_READ_DIRECT(reg);
#ifdef RFC_I2C_TEST
	else if(ldev->rf.if_type == RF_CTRL_I2C)
		return panther_i2c_rf_read(reg);
#endif
#if defined(CONFIG_MONTE_CARLO)
	else if(ldev->rf.if_type == RF_CTRL_W6)
		return mt301_reg_read_w6(reg);
#endif
	else	// FIXME: error case
		return 0;
}

void rf_write(char reg, int val)
{
#if defined(CONFIG_MONTE_CARLO)
	//serial_printf("%s(type=%d): reg=%d, val=0x%x\n", __FUNCTION__, ldev->rf.if_type, reg, val);
#else
//  serial_printf("Panther:%s(type=%d): reg=%d, val=0x%x\n", __FUNCTION__, ldev->rf.if_type, reg, val);
#endif

	if(ldev->rf.if_type == RF_CTRL_DIRECT)
		RFREG_WRITE_DIRECT(reg, val);
#ifdef RFC_I2C_TEST
	else if(ldev->rf.if_type == RF_CTRL_I2C)
		panther_i2c_rf_write(reg, val);
#endif
#if defined(CONFIG_MONTE_CARLO)
	else if(ldev->rf.if_type == RF_CTRL_W6)
		mt301_reg_write_w6(reg, val);
#endif

#if !defined(CONFIG_MONTE_CARLO)
//  rf_read(reg);
#endif
}

void rf_update(char reg, int val, unsigned int mask)
{
#if defined(CONFIG_MONTE_CARLO)
	//serial_printf("%s(type=%d): reg=%d, val=0x%x, mask=%x\n", 
	//		__FUNCTION__, ldev->rf.if_type, reg, val, mask);
#else
//  serial_printf("Panther:%s(type=%d): reg=%d, val=0x%x, mask=%x\n",
//  		__FUNCTION__, ldev->rf.if_type, reg, val, mask);
#endif

	if(ldev->rf.if_type == RF_CTRL_DIRECT) 
		RFREG_UPDATE_DIRECT(reg, val, mask);
#if defined(CONFIG_MONTE_CARLO)
	else if(ldev->rf.if_type == RF_CTRL_W6) 
	{
		unsigned int newval = mt301_reg_read_w6(reg);     
    	newval = (( newval & ~(mask) ) | ( (val) & (mask) ));
    	mt301_reg_write_w6(reg, newval);      
	}
#endif
#ifdef RFC_I2C_TEST
	else if(ldev->rf.if_type == RF_CTRL_I2C)
		panther_i2c_rf_update(reg, val, mask);
#endif

#if !defined(CONFIG_MONTE_CARLO)
//  rf_read(reg);
#endif
}

void rf_set_40mhz_channel(int primary_ch_freq, int channel_type)
{
#if defined(CONFIG_MONTE_CARLO)
	if(ldev->rf.if_type == RF_CTRL_W6) 
		mt301_set_40mhz_channel(primary_ch_freq, channel_type);
	else
#endif
		lrf_set_40mhz_channel(primary_ch_freq, channel_type);
}

void rf_set_tx_gain(unsigned long gain)
{
#if defined(CONFIG_MONTE_CARLO)
	if(ldev->rf.if_type == RF_CTRL_W6) 
		mt301_set_tx_power(gain);
	else
#endif
		lrf_set_tx_power(gain);
}

unsigned int rf_get_tx_gain(unsigned long gain)
{
#if defined(CONFIG_MONTE_CARLO)
	if(ldev->rf.if_type == RF_CTRL_W6) 
		return mt301_get_tx_power();
	else
#endif
		return lrf_get_tx_power();
}

extern int scan_all_lna_vga_dc(void);
extern unsigned int *pre_rxdc_rec_ptr;
int rf_init(void)
{
#if defined(CONFIG_MONTE_CARLO)
	int i;
	// only for external Montecarlo
	*(volatile u32*)0xbf004f48 = 0x3000108c;
	*(volatile u32*)0xbf004afc = 0x00000074;
	*(volatile u32*)0xbf004c7c = 0x3e4277d0;
#endif
	printf("rf_init\n");

#if 0
    //global little endian switch
    MACREG_UPDATE32(0x8e4, 0x2, 0x2);
#endif
#if 0
	MACREG_UPDATE32(LMAC_CNTL, 0x1, LMAC_CNTL_TSTART);
	udelay(1000);
	MACREG_UPDATE32(LMAC_CNTL, 0x0, LMAC_CNTL_TSTART);
#endif
#ifdef RFC_I2C_TEST
	if ((PMUREG(PKG_MODE_CTRL) & ATE_MODE) == TEST_EN) {
		/* set ADC/DAC to EVB test mode */
		rf_update(18, 0xff800000, 0x34800000);
		/* turn off TX_ON/RX_ON/PA_ON */
		PMUREG(RF_REG_CTRL) = 1;
		/* disable internal common voltage */
		PLLREG(WIFI_ADC_SETTING) = 0x91;
		rf_write(0, 0x5babe0);
		rf_write(1, 0x400310);
	}
#endif
	ldev = &_ldev;
	bb_reset();
	printf("finish bb_reset\n");
	rf_bb_check();
	printf("finish rf_bb_check\n");

#if defined(CONFIG_FPGA)
	if(ldev->rf.if_type == RF_CTRL_UNKNOW)
	{
		printf("No RF chip found on FPGA platform, forced to use MT-301\n");
		ldev->rf.if_type = RF_CTRL_W6;
	}
#endif

#if defined(CONFIG_MONTE_CARLO)
	if(ldev->rf.if_type == RF_CTRL_W6)
	{
		printf("RF_CTRL_W6\n");
		mt301_init();

		/* tuning for per RF IC */
		for(i=0; i<MAX_RF_REG_NUM; i++)
		{
			if(rf_tbl[i].num == 0)
				break;
			printf("tuning for per RF IC\n");
			WLA_DBG(INFO, "RF write 0x%02x=0x%02x\n", rf_tbl[i].num, rf_tbl[i].val);
			rf_write(rf_tbl[i].num, rf_tbl[i].val);
		
		}
	}
	else
#endif
	{
		printf("NONE RF_CTRL_W6\n");
		panther_rf_init();

//#ifndef CONFIG_RFC_AT_BOOT
//        bb_init();  /* FIXME: need it? (rxdc cal) */
//
//        panther_rfc_process();
//        /* FIXME: PLL need recal after rfc */
//
//        lrf_channel_pll_cal(0);
//        lrf_set_freq(2412); // channel 1
//#else
//        config_rfc_parm(0, 2412);
//
//        bb_init();  /* for rxdc */
//        MACREG_UPDATE32(LMAC_CNTL, LMAC_CNTL_TSTART, LMAC_CNTL_TSTART);
//        config_rxdc(rxdc_rec_ptr);
//        MACREG_UPDATE32(LMAC_CNTL, 0, LMAC_CNTL_TSTART);
//        /* FIXME: remoce it after debug */
//#endif
	}

	return 0;
}

