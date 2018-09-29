/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file wla_bb.c
*   \brief  wla wlan baseband functions.
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
//#include <panther_dev.h>
#include <mac_ctrl.h>
#include <panther_rf.h>
#include <rf.h>
#include <rfc.h>
#include <panther_debug.h>
#include <bb.h>
#include <ip301.h>
#include <panther_dev.h>
#include <os_compat.h>
#include "mac_regs.h"

int printf(char *fmt, ...);

#ifndef CONFIG_MONTE_CARLO
unsigned char txvga_gain[14];
unsigned char txvga_gain_save[14];
unsigned char fofs;
unsigned char fofs_save;
unsigned char bg_txp_diff;
unsigned char ng_txp_diff;
unsigned char bg_txp_gap;
char fem_product_id[8];
int fem_en;
#endif

struct bb_regs bb_init_tbl_r5c_j [] = {
    {0x00, 0x01},  // reset BB to default value
	/* reg 0x01 MUST at second entry */
	{0x01, 0x80},  // toggle RXHP and old bb_LMAC interface
	{0xF4, 0x00},
    {0x02, 0x31},  // TX IQ swap: V7 is different with V5
    {0xf2, 0x80},  // DAC CLK 40MHz
    {0xf3, 0x22},  // ADC CLK 40MHz
    {0x54, 0x2b},  // disable on-fly IQmismatch compenstion (0x23 enable)
	{0x05, 0x80},  // To pass TX mask of B mode
	{0x10, 0x00},	// AGC table
	{0x11, 0x78},
	{0x10, 0x01},
	{0x11, 0x9c},
	{0x10, 0x02},
	{0x11, 0x30},
	{0x10, 0x03},
	{0x11, 0x7d},
	{0x10, 0x04},
	{0x11, 0x30},
	{0x10, 0x05},
	{0x11, 0x42},
	{0x10, 0x06},
	{0x11, 0x00},
	{0x10, 0x07},
	{0x11, 0x2c},
	{0x10, 0x08},
	{0x11, 0x46},
	{0x10, 0x09},
	{0x11, 0x72},
	//{0x11, 0xb2}, /* David asks to change CCA threshold to pass CE */
	{0x10, 0x0a},
	{0x11, 0xe4},
	{0x10, 0x0b},
	{0x11, 0xb4},
	{0x10, 0x0c},
	{0x11, 0x0a},
	{0x10, 0x0d},
	{0x11, 0x2b},
	{0x10, 0x0e},
	{0x11, 0x30},
	{0x10, 0x0f},
	{0x11, 0x04},
	{0x10, 0x10},
	{0x11, 0x5b},
};

struct bb_regs bb_init_tbl_r81_a1 [] = {
    {0x00, 0x01},  // reset BB to default value
	/* reg 0x01 MUST at second entry */
	{0x01, 0x80},  // toggle RXHP and old bb_LMAC interface
	{0xF4, 0x00},
    {0x02, 0x31},  // TX IQ swap: V7 is different with V5
    {0xf2, 0x80},  // DAC CLK 40MHz
    {0xf3, 0x22},  // ADC CLK 40MHz
    {0x54, 0x2b},  // disable on-fly IQmismatch compenstion (0x23 enable)
	{0x05, 0x80},  // To pass TX mask of B mode

	{0x10, 0x00},	// AGC table
	{0x11, 0x78},
	{0x10, 0x01},
	{0x11, 0x9c},
	{0x10, 0x02},
	{0x11, 0x30},
	{0x10, 0x03},
	{0x11, 0x7d},
	{0x10, 0x04},
	{0x11, 0x30},
	{0x10, 0x05},
	{0x11, 0x42},
	{0x10, 0x06},
	{0x11, 0x0f}, /* 0x8f -> 0x0f for simulation */
	{0x10, 0x07},
	{0x11, 0x6c},
	{0x10, 0x08},
	{0x11, 0x46}, /* 0x86 -> 0x66 -> 46 for atheros's ACK */
	{0x10, 0x09},
	{0x11, 0x71},
	//{0x11, 0xb2}, /* David asks to change CCA threshold to pass CE */
	{0x10, 0x0a},
	{0x11, 0xef},
	{0x10, 0x0b},
	{0x11, 0xb0},
	{0x10, 0x0c},
	{0x11, 0x0a},
	{0x10, 0x0d},
	{0x11, 0x2b},
	{0x10, 0x0e},
	{0x11, 0x30},
	{0x10, 0x0f},
	{0x11, 0x04},
	{0x10, 0x10},
	{0x11, 0x5b},

	{0x12, 0x07}, /* lms */
	{0x13, 0x03}, /* 2nd-Avg-Mode: 0-auto, 2-NO, 3-Yes */
	{0x12, 0x05}, /* dmp_q */
	{0x13, 0x03}, /* [2:0] dmp_q */
	{0x12, 0x0b}, /* bond_advance_pt */
	{0x13, 0x04}, /* [3:0] bond_advance_pt */
	{0x12, 0x0e}, /* csd_det */
	{0x13, 0x04}, /* [7] csd_des_en, [6] csd_ptk, [5] rcfo_tme_en, [3:0] ch_pow_fall_cnt */
	{0x12, 0x08}, 
	{0x13, 0x30}, /* Reduce step size of channel estimation to comply with Broadcom AP */
};
struct bb_regs_ahb bb_init_tbl_r81_a1_ahb [] = {
    {0, 0x00, 0x01},  // reset BB to default value
        /* reg 0x01 MUST at second entry */
        {0, 0x01, 0x80},  // toggle RXHP and old bb_LMAC interface
        {0, 0xF4, 0x00},
    {0, 0x02, 0x31},  // TX IQ swap: V7 is different with V5
    {0, 0xf2, 0x80},  // DAC CLK 40MHz
    {0, 0xf3, 0x22},  // ADC CLK 40MHz
    {0, 0x54, 0x2b},  // disable on-fly IQmismatch compenstion (0x23 enable)
        {0, 0x05, 0x80},  // To pass TX mask of B mode

//      {0x10, 0x00},   // AGC table
//      {0x11, 0x78},
        {1, 0x00, 0x78},
//      {0x10, 0x01},
//      {0x11, 0x9c},
        {1, 0x01, 0x9c},
//      {0x10, 0x02},
//      {0x11, 0x30},
        {1, 0x02, 0x30},
//      {0x10, 0x03},
//      {0x11, 0x7d},
        {1, 0x03, 0x7d},
//      {0x10, 0x04},
//      {0x11, 0x30},
        {1, 0x04, 0x30},
//      {0x10, 0x05},
//      {0x11, 0x42},
        {1, 0x05, 0x42},
//      {0x10, 0x06},
//      {0x11, 0x0f}, /* 0x8f -> 0x0f for simulation */
        {1, 0x06, 0x0f},
//      {0x10, 0x07},
//      {0x11, 0x6c},
        {1, 0x07, 0x6c},
//      {0x10, 0x08},
//      {0x11, 0x46}, /* 0x86 -> 0x66 -> 46 for atheros's ACK */
        {1, 0x08, 0x46},
//      {0x10, 0x09},
//      {0x11, 0x71},
        {1, 0x09, 0x71},
//none  //{0x11, 0xb2}, /* David asks to change CCA threshold to pass CE */
//      {0x10, 0x0a},
//      {0x11, 0xef},
        {1, 0x0a, 0xef},
//      {0x10, 0x0b},
//      {0x11, 0xb0},
        {1, 0x0b, 0xb0},
//      {0x10, 0x0c},
//      {0x11, 0x0a},
        {1, 0x0c, 0x0a},
//      {0x10, 0x0d},
//      {0x11, 0x2b},
        {1, 0x0d, 0x2b},
//      {0x10, 0x0e},
//      {0x11, 0x30},
        {1, 0x0e, 0x30},
//      {0x10, 0x0f},
//      {0x11, 0x04},
        {1, 0x0f, 0x04},
//      {0x10, 0x10},
//      {0x11, 0x5b},
        {1, 0x10, 0x5b},
//      {0x12, 0x07}, /* lms */
//      {0x13, 0x03}, /* 2nd-Avg-Mode: 0-auto, 2-NO, 3-Yes */
        {2, 0x07, 0x03},
//      {0x12, 0x05}, /* dmp_q */
//      {0x13, 0x03}, /* [2:0] dmp_q */
        {2, 0x05, 0x03},
//      {0x12, 0x0b}, /* bond_advance_pt */
//      {0x13, 0x04}, /* [3:0] bond_advance_pt */
        {2, 0x0b, 0x04},
//      {0x12, 0x0e}, /* csd_det */
//      {0x13, 0x04}, /* [7] csd_des_en, [6] csd_ptk, [5] rcfo_tme_en, [3:0] ch_pow_fall_cnt */
        {2, 0x0e, 0x04},
//      {0x12, 0x08}, 
//      {0x13, 0x30}, /* Reduce step size of channel estimation to comply with Broadcom AP */
        {2, 0x08, 0x30},
};
#if defined (CONFIG_MONTE_CARLO)
struct bb_regs_ahb bb_init_tbl_r88_panther_ahb [] = {
    {0, 0x00, 0x01},  // reset BB to default value
        /* reg 0x01 MUST at second entry */
        {0, 0x01, 0x80},  // toggle RXHP and old bb_LMAC interface
        {0, 0xF4, 0x00},
    {0, 0x02, 0x31},  // TX IQ swap: V7 is different with V5
    {0, 0xf2, 0x80},  // DAC CLK 40MHz
    {0, 0xf3, 0x22},  // ADC CLK 40MHz
    {0, 0x54, 0x2b},  // disable on-fly IQmismatch compenstion (0x23 enable)
        {0, 0x05, 0x80},  // To pass TX mask of B mode
        {1, 0x0, 0xaa},//AGC table for Panther & MONTECARLO
        {1, 0x1, 0x9c},
        {1, 0x2, 0x30},
        {1, 0x3, 0x7d},
        {1, 0x4, 0x46},
        {1, 0x5, 0x41},
        {1, 0x6, 0x8f},
        {1, 0x7, 0x27},
        {1, 0x8, 0x46},
        {1, 0x9, 0x71},
        {1, 0x0a, 0x58},
        {1, 0x0b, 0x45},
        {1, 0x0c, 0x0c},
        {1, 0x0d, 0x27},
        {1, 0x0e, 0x30},
        {1, 0x0f, 0x05},
        {1, 0x10, 0x6f},
        {1, 0x11, 0x00},
        {1, 0x12, 0x32},
        {1, 0x13, 0x00},
        {1, 0x14, 0x20},
        {1, 0x16, 0x96},
        {1, 0x17, 0x1a},
        {1, 0x40, 0x7f},
};
#else
struct bb_regs_ahb bb_init_tbl_r88_panther_ahb [] = {
    {0, 0x00, 0x01},  // reset BB to default value
        /* reg 0x01 MUST at second entry */
        {0, 0x01, 0x80},  // toggle RXHP and old bb_LMAC interface
        {0, 0xF4, 0x00},
    {0, 0x02, 0x33},  // TX IQ swap: V7 is different with V5
    {0, 0xf2, 0x80},  // DAC CLK 40MHz
    {0, 0xf3, 0x22},  // ADC CLK 40MHz
    {0, 0x54, 0x2b},  // disable on-fly IQmismatch compenstion (0x23 enable)
        {0, 0x05, 0x80},  // To pass TX mask of B mode
        {1, 0x0, 0x9c},//AGC table for Panther & Panther rf
        {1, 0x1, 0x66},
        {1, 0x2, 0x30},
        {1, 0x3, 0x7d},
        {1, 0x4, 0x30},
        {1, 0x5, 0x42},
        {1, 0x6, 0x8f},
        {1, 0x7, 0x23},
        {1, 0x8, 0x66},
        {1, 0x9, 0x71},
        {1, 0x0a, 0x18},
        {1, 0x0b, 0x3d},
        {1, 0x0c, 0x0a},
        {1, 0x0d, 0x2f},
        {1, 0x0e, 0x70},
        {1, 0x0f, 0x04},
        {1, 0x10, 0x5b},
        {1, 0x11, 0x08},
        {1, 0x12, 0x64},
        {1, 0x13, 0xf0},
        {1, 0x14, 0x00},
        {1, 0x16, 0x96},
        {1, 0x17, 0x1a},
        {1, 0x40, 0x7f},
};
#endif
struct bb_regs bb_init_tbl_r86_panther [] = {
    {0x00, 0x01},	// reset BB to default value
	/* reg 0x01 MUST at second entry */
	{0x01, 0x80},	// toggle RXHP and old bb_LMAC interface // mark
	{0xF4, 0x00},	// mark
    {0x02, 0x33},	// IQ swap
    {0xf2, 0x80},	// DAC CLK 40MHz
    {0xf3, 0x22},	// ADC CLK 40MHz
    {0x54, 0x2b},	// disable on-fly IQmismatch compenstion (0x23 enable)
	{0x05, 0x80},	// To pass TX mask of B mode	// mark

	{0x10, 0x00},	// AGC table, 2015/12/29 update by Barrios
	{0x11, 0x9c},
	{0x10, 0x01},
	{0x11, 0x66},
	{0x10, 0x02},
	{0x11, 0x30},
	{0x10, 0x03},
	{0x11, 0x7d},
	{0x10, 0x04},
	{0x11, 0x30},
	{0x10, 0x05},
	{0x11, 0x42},
	{0x10, 0x06},
	{0x11, 0x8f},	/* 0x8f -> 0x0f for simulation */
	{0x10, 0x07},	// Initial gain setting
	{0x11, 0x23},	// Initial gain setting to 70dB
	{0x10, 0x08},
	{0x11, 0x66},	/* 0x86 -> 0x66 -> 46 for atheros's ACK */
	{0x10, 0x09},
	{0x11, 0x71},
	{0x10, 0x0a},	// LNA gain setting
	{0x11, 0x08},	// H=44dB, L=34dB, M=20dB
	{0x10, 0x0b},	// LNA gain setting
	{0x11, 0x3d},
	{0x10, 0x0c},	// LNA gain setting
	{0x11, 0x0a},
	{0x10, 0x0d},	// Gain switching point
	{0x11, 0x2f},
	{0x10, 0x0e},	// Gain switching point
	{0x11, 0x70},	// h--(44dB)--M--(32dB)--L--(20dB)--UL
	{0x10, 0x0f},	// Gain switching point
	{0x11, 0x04},
	{0x10, 0x10},	// Gain switching point
	{0x11, 0x5b},
	{0x10, 0x11},	// Gain switching point
	{0x11, 0x08},

	{0x12, 0x07}, /* lms */
	{0x13, 0x03}, /* 2nd-Avg-Mode: 0-auto, 2-NO, 3-Yes */
	{0x12, 0x05}, /* dmp_q */
	{0x13, 0x03}, /* [2:0] dmp_q */
	{0x12, 0x0b}, /* bond_advance_pt */
	{0x13, 0x04}, /* [3:0] bond_advance_pt */
	{0x12, 0x0e}, /* csd_det */
	{0x13, 0x04}, /* [7] csd_des_en, [6] csd_ptk, [5] rcfo_tme_en, [3:0] ch_pow_fall_cnt */
	{0x12, 0x08}, 
	{0x13, 0x30}, /* Reduce step size of channel estimation to comply with Broadcom AP */
};

struct bb_rev_t {
	u8 rf_rev;
	u8 rev;
	u8 size;
	struct bb_regs *tbl;
}bb_revsion_tbl[] = {
	{RFCHIP_MT301_A1, 0x5C, sizeof(bb_init_tbl_r5c_j)/sizeof(struct bb_regs), bb_init_tbl_r5c_j},
	{RFCHIP_MT301_A1, 0x81, sizeof(bb_init_tbl_r81_a1)/sizeof(struct bb_regs), bb_init_tbl_r81_a1},
	{RFCHIP_MT301_A1, 0x83, sizeof(bb_init_tbl_r81_a1)/sizeof(struct bb_regs), bb_init_tbl_r81_a1},
	{RFCHIP_MT301_A1, 0x84, sizeof(bb_init_tbl_r81_a1)/sizeof(struct bb_regs), bb_init_tbl_r81_a1},
	{RFCHIP_MT301_A1, 0x86, sizeof(bb_init_tbl_r81_a1)/sizeof(struct bb_regs), bb_init_tbl_r81_a1},
	{RFCHIP_PANTHER, 0x86, sizeof(bb_init_tbl_r86_panther)/sizeof(struct bb_regs), bb_init_tbl_r86_panther},
	{0, 0, 0, 0},
};


struct bb_regs rfc_tbl[MAX_RFC_REG_NUM];

u8 bb_gain_level [] = {
	0xA,	/* 0 */		/* bb gain -3.5dB */
	0xB,	/* 1 */		/* bb gain -3.0dB */
	0xC,	/* 2 */		/* bb gain -2.5dB */
	0xD,	/* 3 */		/* bb gain -2.0dB */
	0xE,	/* 4 */		/* bb gain -1.5dB */
	0xF,	/* 5 */		/* bb gain -1.0dB */
	0x0,	/* 6 */		/* bb gain  0.0dB */
	0x1,	/* 7 */		/* bb gain  0.5dB */
	0x2,	/* 8 */		/* bb gain  1.0dB */
	0x3,	/* 9 */		/* bb gain  1.5dB */
	0x4,	/* 10 */	/* bb gain  2.0dB */
	0x5,	/* 11 */	/* bb gain  2.5dB */
	0x6,	/* 12 */	/* bb gain  3.0dB */
};
#if 1
//for AHB mode
u8 bb_register_read(int group,u32 bb_reg)
{
    u8 value=0;
    u32 data, dst;

    if(group == 0)
    {
        dst = (bb_reg/4) * 4;
    }
    else if(group == 1)
    {
        dst = ((bb_reg/4) * 4) + 0x100;
    }
    else if(group ==2)
    {
        dst = ((bb_reg/4) * 4) + 0x200;
    }
    else if(group==3)
    {
        dst = ((bb_reg/4) * 4) + 0x300;
    }
    else
    {
        printf("XXX: UnKonwn group %d\n", group);
        return 0;
    }
    data = BBREG_READ32(dst);

    value = (data >> 8*(bb_reg % 4)) & 0xffUL;

    //printf("\t\t\t\tread %d\t%08x\t%02x\n", group, bb_reg, value);
    return value;
}
#endif
#if 0
//for SPI mode
//#define BB_SPI_DOUBLE_CONFIRM 1
u8 bb_register_read(int bb_reg)
{
    u8 value=0; //avoid compiler warning
    u32 data;

#ifdef BB_SPI_DOUBLE_CONFIRM
	u8 tmp, i=0;
	do{
		tmp = value;
#endif
		//WLA_DBG(WLADEBUG, "In bb_register_read(), value = 0x%x, tmp = 0x%x\n", value, tmp);
		data = ( (0x01UL << 17)              /* start SPI */
				| ((bb_reg & 0xff) << 8)    /* register address */
				);

		MACREG_WRITE32(BB_SPI_BASE, data);

		while( (MACREG_READ32(BB_SPI_BASE) & BB_SPI_DONE) != BB_SPI_DONE );

		data = MACREG_READ32(BB_SPI_BASE);

		value = (data & 0xff);

#ifdef BB_SPI_DOUBLE_CONFIRM
		/* Reg.76 will be changed after been read. */
		if(bb_reg == 0x76)
			break;

		if(i++ > 10)
		{
			serial_printf("Fail in bb_register_read(0x%x), value = 0x%x, tmp = 0x%x\n", bb_reg, value, tmp);
			break;
		}
	} while(value != tmp);
#endif

    return value;
}
#endif
#if 1
//for AHB mode
void bb_register_write(int group, u32 bb_reg, u8 value)
{
//    printf("\t\t\t\twrite %d\t%08x\t%02x\n", group, bb_reg, value);
    u32 data, dst, mask=0;

    if(group==0)
    {
        dst = (bb_reg/4) * 4;
    }
    else if(group==1)
    {
        dst = ((bb_reg/4) * 4) + 0x100;
    }
    else if(group==2)
    {
        dst = ((bb_reg/4) * 4) + 0x200;
    }
    else if(group==3)
    {
        dst = ((bb_reg/4) * 4) + 0x300;
    }
    else
    {
        printf("XXX: UnKonwn group %d\n", group);
        return;
    }
    data =BBREG_READ32(dst);

//    printf("read %08x\t%08x ", dst, data);
    if(bb_reg%4==0)
    {
        mask |= 0x000000ffUL;
    }
    else if(bb_reg%4==1)
    {
        mask |= 0x0000ff00UL;
    }
    else if(bb_reg%4==2)
    {
        mask |= 0x00ff0000UL;
    }
    else if(bb_reg%4==3)
    {
        mask |= 0xff000000UL;
    }
    data = ( (data & ~(mask)) | ((value << 8*(bb_reg % 4)) & (mask)) );
    // reg0 will reset BB by write any value except zero
    // write the reg1,2,3 with the LSByte==0, so the BB won't be reset
    if(((dst==0) && (group==0)) && (bb_reg!=0))
    {
        mask = 0xffffff00UL;
        data = data & mask;
    }

//    printf("write %08x\t%08x\n", dst ,data);

    BBREG_WRITE32(dst, data);
}
#endif
#if 0
void bb_register_write(int bb_reg, u8 value)
{
    u32 data;

    data = ((0x01UL << 17)              /* start SPI */
            |  (0x01UL << 16)           /* SPI write */
            | ((bb_reg & 0xff) << 8)    /* register address */
            |  value );                 /* data */

#ifdef BB_SPI_DOUBLE_CONFIRM
	u8 tmp=0, i=0;
	//char buf[16];
	do{
#endif
                printf("before\n");
		MACREG_WRITE32(BB_SPI_BASE, data);
                printf("after\n");
		while( (MACREG_READ32(BB_SPI_BASE) & BB_SPI_DONE) != BB_SPI_DONE );
#ifdef BB_SPI_DOUBLE_CONFIRM
		if(bb_reg == 0)
			break;
#if 0
		if((bb_reg == 0XA2) && (rf_iface->type == RF_CTRL_W6))
			break;
#endif
		
		tmp = bb_register_read(bb_reg);
		
		if(i++> 10)
		{
			//serial_printf("Fail in bb_register_write ");
			//serial_printf("the write value = 0x%x\n", value);
			//serial_printf("the bb_register_read(0x%x) = 0x%x\n", bb_reg, tmp);
			break;
			//gets(buf);
		}
	} while(tmp != value);
#endif
	
    return;
}
#endif
void bb_set_40mhz_mode(int channel_type, int freq)
{

	/* for 80Mhz mode, reverse ADC clock to meet timing*/
	/* reg.0x2 bit.6 = ADC clock invert */
	bb_register_write(0, 0x02, (bb_register_read(0, 0x02) | 0x40)); /* new ADC/DAC board */

//	bb_register_write(0x12, 0x1);		
//	bb_register_write(0x13, 0x77);	/* lp2sig fine tuned */
        bb_register_write(2, 0x1, 0x77);
//	bb_register_write(0x12, 0x4);		
//	bb_register_write(0x13, 0x40);	/* disable tone-erased around DC for carrier offset */
        bb_register_write(2, 0x4, 0x40);

    if(channel_type == BW40MHZ_SCA)
        bb_register_write(0, 0x01, ( (bb_register_read(0, 0x01) & (~(0x03))) | 0x03));    // HT40+
    else if(channel_type == BW40MHZ_SCB)
        bb_register_write(0, 0x01, ( (bb_register_read(0, 0x01) & (~(0x03))) | 0x02));    // HT40-

    bb_register_write(0, 0xF2, (bb_register_read(0, 0xF2) | 0x02));    // Set BR_F2[1] =1 (DAC 80M enable)
    bb_register_write(0, 0xF3, (bb_register_read(0, 0xF3) | 0x40));    // Set BR_F3[6] = 1 (ADC 80M enable)
	
#if defined(CONFIG_RFC)
	/* config the calibrated rfc related register */
	if(ldev->rf.chip_ver == RFCHIP_PANTHER)
		config_rfc_parm(1, freq);
#if 0 //defined(CONFIG_MONTE_CARLO)
	else
		config_rfc_parm_new(0x101);	// 0x101 : accept manual setting & 40MHz Mode
#endif
#endif
}

void bb_set_20mhz_mode(int freq)
{
	/* reg.0x2 bit.6 = ADC clock invert */
	bb_register_write(0, 0x02, (bb_register_read(0, 0x02) & 0xBF)); /* new ADC/DAC board */
			
//	bb_register_write(0x12, 0x1);		
//	bb_register_write(0x13, 0x57);
        bb_register_write(2, 0x1, 0x57);
//	bb_register_write(0x12, 0x4);		
//	bb_register_write(0x13, 0x0);
        bb_register_write(2, 0x4, 0x0);

    bb_register_write(0, 0x01, (bb_register_read(0, 0x01) & (~(0x03))));
    bb_register_write(0, 0xF2, (bb_register_read(0, 0xF2) & (~(0x02))));
    bb_register_write(0, 0xF3, (bb_register_read(0, 0xF3) & (~(0x40))));

#if defined(CONFIG_RFC)
	/* config the calibrated rfc related register */
	if(ldev->rf.chip_ver == RFCHIP_PANTHER)
		config_rfc_parm(0, freq);
#if defined(CONFIG_MONTE_CARLO)
	else
		config_rfc_parm_new(0x100);	// 0x100 : accept manual setting & 20MHz Mode
#endif
#endif
}

u8 bb_set_cca_level(u8 level)
{
	u8 val;

	/*
		Higher cca value means there are lots of packets in the air,
		then BB gets more difficulties to send packets.
	 */
	level &= 0xfc;		/* bit[7:2] */
//	bb_register_write(0x10, 0x9);
//	val = bb_register_read(0x11);
        val = bb_register_read(1, 0x9);
	val = (val & 0x3) | level;
//	bb_register_write(0x11, val);
        bb_register_write(1, 0x9, val);

	return val;
}

#ifndef CONFIG_MONTE_CARLO
void bb_set_tx_gain(u8 txpwr)
{
        unsigned char btxpwr, gtxpwr, ntxpwr;

        btxpwr = gtxpwr = ntxpwr = txpwr;
        if(bg_txp_gap == 0x03)
        {
                unsigned int bb_addr;
                for(bb_addr = 0x4c; bb_addr < 0x53; bb_addr += 2)
                {
                        bb_register_write(2, bb_addr, 0x02);
                        bb_register_write(2, bb_addr + 1, 0xcc);
                }
        }

        if((txpwr >= bg_txp_diff) && (txpwr <= 0x06))
                btxpwr -= bg_txp_diff;
        else if((txpwr >= 0x0a) && (txpwr <= (0x0a + bg_txp_diff)))
                btxpwr = 0x0a;
        else
                btxpwr = (txpwr + (0x10 - bg_txp_diff)) & (0x0f);

        //serial_printf("%x b %x\n", txpwr, btxpwr);
        btxpwr = (bb_register_read(2, 0x80) & 0xf0) | btxpwr;
        bb_register_write(2, 0x80, btxpwr);

        /* Sets G mode TX gain */
        gtxpwr = (bb_register_read(2, 0x81) & 0xf0) | gtxpwr;
        bb_register_write(2, 0x81, gtxpwr);

        /* Sets N mode TX gain */
        if((txpwr >= ng_txp_diff) && (txpwr <= 0x06))
                ntxpwr -= ng_txp_diff;
        else if((txpwr >= 0x0a) && (txpwr <= (0x0a + ng_txp_diff)))
                ntxpwr = 0x0a;
        else
                ntxpwr = (txpwr + (0x10 - ng_txp_diff)) & (0x0f);

        ntxpwr = (bb_register_read(2, 0x82) & 0xf0) | ntxpwr;
        bb_register_write(2, 0x82, ntxpwr);
}
#else
void bb_set_tx_gain(u8 level)
{
	u8 val, max, max_level;

	max_level = sizeof(bb_gain_level);	/* 0 - 12 */
	if (level >= max_level)
		level = max_level - 1;
	val = bb_gain_level[level];

	/* max TX power gain = level plus 1.5dB */
	if (level + 3 >= max_level)
		max_level -= 1;
	else
		max_level = level + 3;
	max = bb_gain_level[max_level];

	/* Barrios suggests no need to adjust B mode TX gain */
	/* Sets G mode TX gain */
//	bb_register_write(0x12, 0x81);
	val = (bb_register_read(2, 0x81) & 0xf0) | val;
//	bb_register_write(0x13, val);
        bb_register_write(2, 0x81, val);
	/* Sets N mode TX gain */
//	bb_register_write(0x12, 0x82);
	val = (bb_register_read(2, 0x82) & 0xf0) | val;
//	bb_register_write(0x13, val);
        bb_register_write(2, 0x82, val);
	/*
		Sets max TX power gain if TX rate is
		smaller than the threashold
	 */
//	bb_register_write(0x12, 0x83);
	val = (bb_register_read(2, 0x83) & 0xf0) | max;
//	bb_register_write(0x13, val);
        bb_register_write(2, 0x83, val);
	/*
		Sets TX power threshold to 3.
		N mode		G mode	value
		MCS0/MCS32	6M		0
		MCS1		9M		1
		MCS2		12M		2
		MCS3		18M		3
		MCS4		24M		4
		MCS5		36M		5
		MCS6		48M		6
		MCS7		54M		7
	 */
//	bb_register_write(0x12, 0x84);
//	bb_register_write(0x13, 0x3);
        bb_register_write(2, 0x84, 0x3);
}
#endif

u8 bb_get_tx_gain(void)
{
	u8 val, i=0;

//	bb_register_write(0x12, 0x81);
//	val = bb_register_read(0x13) & 0xf;
        val = bb_register_read(2, 0x81) & 0xf;

	while (1) {
		if (val == bb_gain_level[i])
			break;
		i++;
	}
	return i;
}

u8 bb_rssi_decode(u8 val, int rssi_offset, u8 *lna_gain_tble)
{
	u8 lna_gain_b[4] = {0, 12, 15, 21};
	u32 lna_gain, total_gain, rssi_gdb, max_gdb=0;

	if(!lna_gain_tble)
		lna_gain_tble = lna_gain_b;

	/* RSSI field{1'b0, 2'b_LNA, 5'b_VGA} */
    lna_gain = lna_gain_tble[(val >> 5) & 0x3]; 

	max_gdb = lna_gain_tble[3] + 31;

	/* total gain = LNA gain + VGA gain */
	total_gain = lna_gain + (val & 0x1f);
	/* reverse gain for signal strength */
	rssi_gdb = max_gdb - total_gain;
//WLA_DBG(WLADEBUG, "total_gain=%d, rssi_gdb=%d\n", total_gain, rssi_gdb);
	/* 1gdb =2dB */
	return ((rssi_gdb*2) - rssi_offset);
}

void bb_reset(void)
{
	u32 hw_mode = 0;
        printf("bb_reset\n");
	/* FIXME:Hardware reset BB */
	//WLA_DBG(WLADEBUG, "RESET BB .. OK\n");

	//MACREG_WRITE32(HW_RESET, 0xffffe0ff);

	/* 	$param_hw_mode: 
		bit.0 == 1 : external bb mode
		bit.0 == 0 : internal bb mode */
	//hw_mode = WLA_CDB_GET($parm_hw_mode, 0);
	if(hw_mode & 0x1)
	{
		/* FIXME:Setup External BB Mode  */
		/* Reduce BB SPI clock rate on ASIC with external BB */
		MACREG_WRITE32(BB_SPI_CLK_DIV, 0x20); 
	}
	/* reset BB to default value */
	bb_register_write(0, 0x0, 0x1);

#if 0
	bb_init();

	if(info->bandwidth_type == BW20_ONLY)
		bb_set_20mhz_mode();
	else	
		bb_set_40mhz_mode(info->bandwidth_type);
#endif
}

void rf_bb_check(void)
{
	u8 rev, bb_reg01 = 0;
#if defined(CONFIG_MONTE_CARLO)
	u32 pattern = 0x5a5a;
	u32 ret_val = 0;
#endif
	struct bb_rev_t *bb_rev_tbl = &bb_revsion_tbl[0];

#ifdef RFC_I2C_TEST
	/* FIXME: for the rfc i2c test */
	ldev->rf.if_type = RF_CTRL_I2C;
	ldev->rf.chip_ver = RFCHIP_PANTHER;
	goto exit;
#endif

	ldev->rf.if_type = RF_CTRL_UNKNOW;

#if defined(CONFIG_MONTE_CARLO)

	/* check 6 wire architecture */
	bb_register_write(0, 0x1, 0x80);
	bb_register_write(0, 0xF4, 0x00);

	mt301_reg_write_w6(0, pattern);
	if((ret_val = mt301_reg_read_w6(0)) == pattern)
	{
                printf("assign RF_CTRL_W6\n");
		bb_reg01 = 0x80;
		ldev->rf.if_type = RF_CTRL_W6;
		goto exit;
	}
        return;
#endif	// CONFIG_MONTE_CARLO

	bb_register_write(0, 0x1, 0x00);
	/* reg.0xf4 is fixed as 0 */
#if 0 /* Write pattern to RF0 will make system hang */
	rf_write(0, pattern);
	if(rf_read(0) == pattern)
	{
		bb_reg01 = 0x00;
		ldev->rf.if_type = RF_CTRL_DIRECT;
		goto exit;
	}
#else
	bb_reg01 = 0x00;
	ldev->rf.if_type = RF_CTRL_DIRECT;
	ldev->rf.chip_ver = RFCHIP_PANTHER;
#endif

#if defined(CONFIG_MONTE_CARLO) || defined(RFC_I2C_TEST)
exit:
#endif
//	DBG_PRINTF(INFO, "rf_interface_check return %x\n", ldev->rf.if_type);
	bb_register_write(0, 0x01, bb_reg01);

	/* check RF version */
	if(ldev->rf.chip_ver == 0)
	{
		ldev->rf.chip_ver = rf_read(0x1f); 
	}

	/* set rf default tx power to level 8 */
	ldev->rf.power_level = 8;

	/* check BB revision */
	rev = bb_register_read(0, 0x00);

	do
	{
		if(bb_rev_tbl->rev == 0)
			break;
		/* RF revision matched ? */
		if(ldev->rf.chip_ver != bb_rev_tbl->rf_rev)
			continue;
		/* BB revision matched */
		if(bb_rev_tbl->rev != rev)
			continue;
	
		ldev->bb_reg_tbl = (u32)bb_rev_tbl;
		bb_rev_tbl->tbl[1].val = bb_reg01;	
		break;
	}while(bb_rev_tbl += 1);
	
}

#define TOTAL_REG_CNT   (17 + 1 + 5)
void backup_bb_result(u8 restore_data[TOTAL_REG_CNT])
{
	u32 i, reg_addr;
	/* save BB20~30,33,5A~5E */
	i=0;
	for (reg_addr=0x20UL; reg_addr<=0x30UL; reg_addr++)
	{
		restore_data[i] = bb_register_read(0, reg_addr);
		//DBG_PRINTF(INFO, "save BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}
	restore_data[i] = bb_register_read(0, 0x33);
	//DBG_PRINTF(INFO, "save BBreg33 data = %02x\n", restore_data[i]);
	i++;
	for (reg_addr=0x5aUL; reg_addr<=0x5eUL; reg_addr++)
	{
		restore_data[i] = bb_register_read(0, reg_addr);
		//DBG_PRINTF(INFO, "save BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}
	//DBG_PRINTF(INFO, "save BBreg data cnt = %d\n", i);
}

void restore_bb_result(u8 restore_data[TOTAL_REG_CNT])
{
	u32 i, reg_addr;
	/* restore BB20~30,33,5A~5E after reset BB */
	i=0;
	for (reg_addr=0x20UL; reg_addr<=0x30UL; reg_addr++)
	{
		bb_register_write(0, reg_addr, restore_data[i]);
		//DBG_PRINTF(INFO, "restore BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}
	bb_register_write(0, 0x33, restore_data[i]);
	//DBG_PRINTF(INFO, "restore BBreg33 data = %02x\n", restore_data[i]);
	i++;
	for (reg_addr=0x5aUL; reg_addr<=0x5eUL; reg_addr++)
	{
		bb_register_write(0, reg_addr, restore_data[i]);
		//DBG_PRINTF(INFO, "restore BBreg%02x data = %02x\n", reg_addr, restore_data[i]);
		i++;
	}

}

void bb_init(void)
{
	u32 j, sz;
	u8 restore_data[TOTAL_REG_CNT];

/*
	if(0 == ldev->bb_reg_tbl) {
		serial_printf("No BB init table ??\n");
		return;
	}

	init_tbl = ((struct bb_rev_t *)(ldev->bb_reg_tbl))->tbl;
	sz = ((struct bb_rev_t *)(ldev->bb_reg_tbl))->size;
*/

	backup_bb_result(restore_data);
        sz = (sizeof(bb_init_tbl_r88_panther_ahb)/sizeof(struct bb_regs_ahb));
	// reset BB
	if (0 < sz)
	{
		bb_register_write(bb_init_tbl_r88_panther_ahb[0].group, bb_init_tbl_r88_panther_ahb[0].num, bb_init_tbl_r88_panther_ahb[0].val);
	}
	restore_bb_result(restore_data);
	for(j = 1; j < sz; j++)
	{
//		bb_register_write(0, init_tbl[j].num, init_tbl[j].val);
            bb_register_write(bb_init_tbl_r88_panther_ahb[j].group, bb_init_tbl_r88_panther_ahb[j].num, bb_init_tbl_r88_panther_ahb[j].val);
	}
	/* enable CSD auto detect algorithm */
	bb_register_write(2, 0x7, 0x3);

	/* enable the digital gain for RSSI computation */
	bb_register_write(0, 0x1d, 0x8);
}

void bb_rx_counter_ctrl(u32 op)
{
	switch(op)
	{
		case BB_RX_CNT_RESET:
			bb_register_write(0, 0x80, 0xC0);
			break;
		case BB_RX_CNT_ENABLE:
			bb_register_write(0, 0x80, 0x80);
			break;
		case BB_RX_CNT_DISABLE:
			bb_register_write(0, 0x80, 0x00);
			break;
	}
}

u32 bb_rx_counter_read(u8 addr)
{
	/***** 
		Just for active counter & busy counter now.
		If add new counter read action, the progress should be modified.
	 *****/

	u8 i;
	u32 val=0;

	for(i=0; i<3; i++)
	{
		/* set the counter address to BB Reg.0x14 */
		//bb_register_write(3, 0x14, addr + i);
		/* read counter value from BB Reg.0x15 */
		//val = (val << 8) | bb_register_read(3, 0x15);
		val = (val << 8) | bb_register_read(3, addr + i);
	}

	return val;
}

u8 bb_read_noise_floor(void)
{
	u8 val;

	val = bb_register_read(0, 0x1E);

	return val;
}
