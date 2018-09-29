/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file ate.h
*   \brief Registers definition
*   \author Montage
*/

#ifndef ATE_H
#define ATE_H

/*==============================================================+
| define this to avoid using some unnecessary code,             |
| maybe these code will use in the future.                      |
+==============================================================*/
//      #define CONFIG_TODO
#include <arch/chip.h>

#define USB_TEST
#define EPHY_TEST
#define DDR_TEST
#define MADC_TEST
#define WT_TEST
#define AUD_TEST

//Result register
#define RESULT1  0xBF0048FC //ETH
#define RESULT2  0xBF005510 //USB[13:0] and reg test[16]
#define RESULT3  0xBF005514 //WRT
#define RESULT4  0xBF005520 //RF
#define RESULT5  0xBF005524 //DDR

#if 0
//AUDIO Test Register
#define CLK_EN_CTRL     0xBF004A58
#define ANA_TEST_CTRL   0xBF004C28
#define CPLL_REG        0xBF004C88
#define AUDIO_ADC       0xBF004C8C
#endif

#define ANA_TEST_CTRL   0x28
#define CPLL_REG        0x88
#define AUDIO_ADC       0x8C


#define USB0_BASE   0xBF00C000
#define USB1_BASE   0xBF00D000

#define USB0REGR(offset) REG(USB0_BASE,offset)
#define USB1REGR(offset) REG(USB1_BASE,offset)

#define USB0REGW(offset, val) REG(USB0_BASE,offset) = (unsigned int)(val)
#define USB1REGW(offset, val) REG(USB1_BASE,offset) = (unsigned int)(val)
#define USB0_UPDATE32(x, val, mask) do {                  \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (USB0_BASE + (x)); \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(USB0_BASE + (x)) = newval;    \
} while(0)
#define USB1_UPDATE32(x, val, mask) do {                  \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (USB1_BASE + (x)); \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(USB1_BASE + (x)) = newval;    \
} while(0)

#define USB_TEST_MODE_REG               0x200
#define USB_TEST_PKT_CFG_REG            0x210
#define USB_TEST_PKT_STS_REG            0x214
#define USB_STD_PKT_CTRL                0x218
#define USB_BIST_CFG_REG                0x220
#define USB_BIST_PKT_LEN_REG            0x224
#define USB_BIST_INVLD_LEN_REG          0x228
#define USB_BIST_BIT_ERR_DETECTED_REG   0x22C
#define USB_BIST_BIR_ERR_CNT_REG        0x230
#define USB_BIST_OVERFLOW_CNT_REG       0x234
#define USB_BIST_RXERROR_CNT_REG        0x238
#define USB_BIST_RXDATA_CNT_REG         0x23C
#define USB_PHY_DIG_CFG                 0x300

enum {
    HIGH_SPEED,
    FULL_SPEED,
    LOW_SPEED
};

enum {
    WIFI_TX,
    WIFI_RX,
    WIFI_STOP
};

#define RFREG_READ32(offset)         REG(RF_BASE, offset)
#define RFREG_WRITE32(offset, val)   REG(RF_BASE, offset) = (unsigned int)(val)
#define RFREG_UPDATE32(x, val, mask) do {                   \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (RF_BASE + (x));   \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(RF_BASE + (x)) = newval;    \
} while(0)

#define ANAREG_READ32(offset)         REG(ANA_BASE, offset)
#define ANAREG_WRITE32(offset, val)   REG(ANA_BASE, offset) = (unsigned int)(val)
#define ANAREG_UPDATE32(x, val, mask) do {                   \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (ANA_BASE + (x));   \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(ANA_BASE + (x)) = newval;    \
} while(0)
#endif
