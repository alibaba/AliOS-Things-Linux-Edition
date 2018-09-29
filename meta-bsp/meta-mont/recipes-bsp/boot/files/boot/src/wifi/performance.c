/*=============================================================================+
|                                                                              |
| Copyright 2018                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*!
*   \file performance.c
*   \brief  performance config function.
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <arch/chip.h>
#include "performance.h"
#include "bb.h"
#include "rf.h"
#include "mac_regs.h"

void bb_fem_init(int version)
{
    bb_register_write(2, 0x0, 0x40);
    bb_register_write(2, 0x1, 0x4);
    bb_register_write(1, 0x6, 0x8b);
    bb_register_write(1, 0x7, 0x1c);
    bb_register_write(1, 0x8, 0x45);
    bb_register_write(1, 0xa, 0x1f);
    bb_register_write(1, 0x13, 0x0);

    bb_register_write(1, 0x10, 0xff);
    bb_register_write(2, 0x21, 0x3);
    bb_register_write(2, 0x22, 0x80);
    bb_register_write(1, 0x12, 0xe1);
}

void panther_fem_config(int version)
{
    rf_update(9, 0x2 << 28, 0x7 << 28);
    rf_update(18, 0x1 << 6, 0x7 << 6);

    bb_fem_init(version);
}

void panther_fem_init(int version)
{
    /* switch pin mux */
    u32 val;

    val = GPREG(0x18);
    val &= 0x0fffffff;
    val |= (0x1 << 28);
    GPREG(0x18) = val;

    val = GPREG(0x18);
    val &= 0xff0fffff;
    val |= (0x1 << 20);
    GPREG(0x18) = val;

    val = GPREG(0x2c);
    val &= 0x0fff3fff;
    val |= (0x1 << 15);
    GPREG(0x2c) = val;

    val = GPREG(0x2c);
    val &= 0x0ffff3ff;
    val |= (0x1 << 11);
    GPREG(0x2c) = val;

    panther_fem_config(version);
}

void panther_without_fem_init(void)
{
    bb_register_write(1, 0x15, 0xf);
    bb_register_write(2, 0x24, 0x27);
    bb_register_write(2, 0x25, 0x20);
    bb_register_write(2, 0x26, 0x6);
    bb_register_write(2, 0x27, 0x2);
}

void panther_channel_config(int channel, int version)
{
#if defined(CONFIG_CHANNEL_14_IMPROVE)
    if(channel == 14)
    {
        bb_register_write(1, 0x7, 0x21);
        bb_register_write(2, 0x0, 0x40);
        bb_register_write(2, 0x1, 0x4);
    }
    else
    {
        if(version == 1)
        {
            bb_register_write(1, 0x7, 0x1c);
            bb_register_write(2, 0x0, 0x40);
            bb_register_write(2, 0x1, 0x4);
        }
        else
        {
            bb_register_write(1, 0x7, 0x23);
            bb_register_write(2, 0x0, 0x0);
            bb_register_write(2, 0x1, 0x57);
        }
    }
#endif
}

void panther_throughput_config(void)
{
#if defined(CONFIG_THROUGHPUT_IMPROVE)
    MACREG_WRITE32(FREE_CONTROL_DELAY_0, ((0x0 << 24) & DAC_ON_TON)
                                       | ((0x10 << 16) & DAC_ON_TOFF)
                                       | ((0x01 << 8) & BB_TXPE_TON)
                                       | (0x0 & BB_TXPE_TOFF));

    MACREG_WRITE32(FREE_CONTROL_DELAY_1, ((0x10 << 24) & RF_TX_EN_TON)
                                       | ((0x01 << 16) & RF_TX_EN_TOFF)
                                       | ((0x14 << 8) & PA_EN_TON)
                                       | (0x01 & PA_EN_TOFF));

    MACREG_WRITE32(LMAC_OPTION_SEL, SIFSCHK_OPT | STOP_BKOFF_CNT_OPT);

    MACREG_WRITE32(BB_PROC, (0x18 & BB_PROCTIME));

    MACREG_WRITE32(EXTEND_PD, (0x0 & EXTEND_PERIOD));

    MACREG_UPDATE32(PHYDLY_SET, 0x14 << 8, PHYDLY_SET_RFTXDELY);
#endif
}
