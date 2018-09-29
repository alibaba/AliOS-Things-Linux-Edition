/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file i2c_wm8750.c
*   \brief access wm8750 register by I2C
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <i2c.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
/* WM8750 register space */

#define WM8750_LINVOL    0x00
#define WM8750_RINVOL    0x01
#define WM8750_LOUT1V    0x02
#define WM8750_ROUT1V    0x03
#define WM8750_ADCDAC    0x05
#define WM8750_IFACE     0x07
#define WM8750_SRATE     0x08
#define WM8750_LDAC      0x0a
#define WM8750_RDAC      0x0b
#define WM8750_BASS      0x0c
#define WM8750_TREBLE    0x0d
#define WM8750_RESET     0x0f
#define WM8750_3D        0x10
#define WM8750_ALC1      0x11
#define WM8750_ALC2      0x12
#define WM8750_ALC3      0x13
#define WM8750_NGATE     0x14
#define WM8750_LADC      0x15
#define WM8750_RADC      0x16
#define WM8750_ADCTL1    0x17
#define WM8750_ADCTL2    0x18
#define WM8750_PWR1      0x19
#define WM8750_PWR2      0x1a
#define WM8750_ADCTL3    0x1b
#define WM8750_ADCIN     0x1f
#define WM8750_LADCIN    0x20
#define WM8750_RADCIN    0x21
#define WM8750_LOUTM1    0x22
#define WM8750_LOUTM2    0x23
#define WM8750_ROUTM1    0x24
#define WM8750_ROUTM2    0x25
#define WM8750_MOUTM1    0x26
#define WM8750_MOUTM2    0x27
#define WM8750_LOUT2V    0x28
#define WM8750_ROUT2V    0x29
#define WM8750_MOUTV     0x2a

/* CSB is low/high; addr is 0x34/0x36 */
static int WM8750_CSB = 1;
#define WM8750_I2C_ADDR (WM8750_CSB ? 0x1B : 0x1A)
#define wm8750_log(fmt, args...)        //printf(fmt, ##args)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static unsigned short wm8750_reg[] = {
    0x0097, 0x0097, 0x0079, 0x0079,     /*  0 */
    0x0000, 0x0008, 0x0000, 0x000a,     /*  4 */
    0x0000, 0x0000, 0x00ff, 0x00ff,     /*  8 */
    0x000f, 0x000f, 0x0000, 0x0000,     /* 12 */
    0x0000, 0x007b, 0x0000, 0x0032,     /* 16 */
    0x0000, 0x00c3, 0x00c3, 0x00c0,     /* 20 */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 24 */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 28 */
    0x0000, 0x0000, 0x0050, 0x0050,     /* 32 */
    0x0050, 0x0050, 0x0050, 0x0050,     /* 36 */
    0x0079, 0x0079, 0x0079,     /* 40 */
};

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
/*!
 * function: wm8750_reg_write
 *
 *  \brief write wm8750 register by I2C
 *  \param reg
 *  \param data
 *  \return int
 */
int wm8750_reg_write(unsigned char reg, unsigned short data)
{
    int rc;
    wm8750_log("[wm8750 register write]\n");

    i2c_start(WM8750_I2C_ADDR, 0);

    // command format(27): dev_addr(7)+cmd(1);ack(1);
    //                     addr(7)+data(1);ack(1);
    //                     data(8);ack(1)
    //rc = i2c_write(WM8750_I2C_ADDR);
    //if (rc)
    //    goto frc;
    rc = i2c_write((reg << 1) + ((data & 0x100) >> 8), 0);
    if (rc)
        goto frc;
    rc = i2c_write((data & 0xff), 1);

  frc:
    //i2c_stop();
    if (rc)
        printf("wm8750 write reg%d fail!!!\n", reg);
    else
        wm8750_reg[reg] = (data & 0x1ff);       //upadte cached value
    return rc;
}

/*!
 * function: wm8750_reg_read
 *
 *  \brief read wm8750 register by I2C
 *  \return
 */
void wm8750_reg_read(unsigned char reg, unsigned short *ptr)
{
    wm8750_log("[wm8750 register read]\n");
    *ptr = wm8750_reg[reg];
}

/*!
 * function: wm8750_init
 *
 *  \brief initialize I2C for accessing wm8750
 *  \return
 */
void wm8750_init(void)
{
    wm8750_log("[init I2C module for access wm8750 register]\n");
    i2c_init();
}

/*!
 * function: wm8750_finish
 *
 *  \brief finish I2C setting
 *  \return
 */
void wm8750_finish(void)
{
    wm8750_log("[finish I2C module]\n");
    i2c_finish();
}

/*!
 * function: wm8750_bypass
 *
 *  \brief line bypass setting
 *  \param isInput2
 *  \return
 */
void wm8750_bypass(int isInput2)
{
    wm8750_log("[do bypass setting]\n");
    wm8750_reg_write(0x0f, 0x000);      // RESET 0x0F
    wm8750_reg_write(0x19, 0x0C0);      // VREF 0x19
    wm8750_reg_write(0x1a, 0x078);      // L/ROUT1_2 0x1A
    wm8750_reg_write(0x02, 0x179);      // LOUT1 0x02
    wm8750_reg_write(0x03, 0x179);      // ROUT1 0x03
    wm8750_reg_write(0x22, 0x0A0);      // LI2LO 0x22
    wm8750_reg_write(0x25, 0x0A0);      // RI2RO 0x25
    wm8750_reg_write(0x28, 0x179);      // LOUT2 0x28
    wm8750_reg_write(0x29, 0x179);      // ROUT2 0x29

    // LMIXSEL/RMIXSEL default are LINPUT1/RINPUT1 0x22/0x24[2:0]
    if (isInput2)
    {
        wm8750_reg_write(0x20, 0x040);  // Set the ADCL signal to LINPUT2 R32
        wm8750_reg_write(0x21, 0x040);  // Set the ADCL signal to RINPUT2 R33
        // set LMIXSEL/RMIXSEL to LINPUT2/RINPUT2
        wm8750_reg_write(0x22, 0x0A1);  // Set Left Input Selection is LINPUT2, and LI2LO
        wm8750_reg_write(0x24, 0x001);  // Set Right Input Selection is RINPUT2
    }
}

/*!
 * function: wm8750_adc_dac
 *
 *  \brief
 *  \param isInput2
 *  \return
 */
void wm8750_adc_dac(int isInput2)
{
    wm8750_log("[do adc_dac setting]\n");
    wm8750_reg_write(0x0f, 0x000);      // RESET R15
    wm8750_reg_write(0x19, 0x1c0);      // Power up VMID[5Kohm] and VREF R25
    wm8750_reg_write(0x19, 0x0fc);      // Power up VMID[50Kohm] and VREF R25
    wm8750_reg_write(0x1a, 0x180);      // Power up DACL & DACR R26
    wm8750_reg_write(0x1a, 0x1e0);      // Power up R/LOUT1 leaving DACL/R set R26
    wm8750_reg_write(0x02, 0x179);      // Set LOUT1 Update bit to '1' & Volume Level to default R2
    wm8750_reg_write(0x03, 0x179);      // Set ROUT1 Update bit to '1' & Volume Level to default R3
    wm8750_reg_write(0x05, 0x000);      // Set DAC mute off R5
    wm8750_reg_write(0x0a, 0x1ff);      // Set LDAC Update bit to '1' & Volume Level to default R10
    wm8750_reg_write(0x0b, 0x1ff);      // Set RDAC Update bit to '1' & Volume Level to default R11

    if (isInput2)
    {
        wm8750_reg_write(0x20, 0x040);  // Set the ADCL signal to LINPUT2 R32
        wm8750_reg_write(0x21, 0x040);  // Set the ADCL signal to RINPUT2 R33
    }

    wm8750_reg_write(0x22, 0x150);      // Set the LD2LO bit (Left DAC to Left Output) R34
    wm8750_reg_write(0x25, 0x150);      // Set the RD2RO bit (Right DAC to Right Output) R37
    wm8750_reg_write(0x28, 0x179);      // Set LOUT2 Update bit to '1' & Volume Level to default R40
    wm8750_reg_write(0x29, 0x179);      // Set ROUT2 Update bit to '1' & Volume Level to default R41

    wm8750_reg_write(0x00, 0x117);      // Set Left Input Volume Update bit to '1' & Volume Level to default R0
    wm8750_reg_write(0x01, 0x117);      // Set Right Input Volume Update bit to '1' & Volume Level to default R1

    wm8750_reg_write(0x07, 0x002);      // BCLK normal & Slave mode & 16 bits & I2S Format R7
    wm8750_reg_write(0x08, 0x040);      // Normal mode R8 & MCLK/256(8kHz) & CLKDIV2=1
    printf("defalut Slave mode & 16 bits & PCM Format\n");
}

/*!
 * function: wm8750
 *
 *  \brief command to access wm8750 register by I2C
 *  \param argc
 *  \param argv
 *  \return
 */
int wm_cmd(int argc, char *argv[])
{
    char *cmd;
    unsigned int failed = 0, no = 0, data;

    switch (argc)
    {
        case 3:
            if (!hextoul(argv[2], &data))
                goto err;
        case 2:
            if (!hextoul(argv[1], &no))
                goto err;
        case 1:
            cmd = argv[0];
            break;
        case 0:
        default:
            goto err;
            break;
    }

    if (!strcmp(cmd, "ifm"))
    {
        no = (no == 0) ? 0x3 : 0x2;
        wm8750_reg_write(WM8750_IFACE, (wm8750_reg[WM8750_IFACE] & ~0x03) | no);
    }
    else if (!strcmp(cmd, "rol"))
    {
        no = (no == 0);
        wm8750_reg_write(WM8750_IFACE,
                         (wm8750_reg[WM8750_IFACE] & ~0x40) | (no << 6));
    }
    else if (!strcmp(cmd, "dat"))
    {
        wm8750_reg_write(WM8750_IFACE,
                         (wm8750_reg[WM8750_IFACE] & ~0x0c) | (no << 2));
    }
    else if (!strcmp(cmd, "bclk"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        wm8750_reg_write(WM8750_IFACE,
                         (wm8750_reg[WM8750_IFACE] & ~0x80) | (no << 7));
    }
    else if (!strcmp(cmd, "div2"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        wm8750_reg_write(WM8750_SRATE,
                         (wm8750_reg[WM8750_SRATE] & ~0x40) | (no << 6));
    }
    else if (!strcmp(cmd, "sr"))
    {
        wm8750_reg_write(WM8750_SRATE,
                         (wm8750_reg[WM8750_SRATE] & ~0x3e) | (no << 1));
    }
    else if (!strcmp(cmd, "w"))
    {
        if (wm8750_reg_write(no, data))
            failed |= (1 << 0);
        printf("write reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "r"))
    {
        wm8750_reg_read(no, (unsigned short *) &data);
        printf("read reg%d = 0x%03x\n", no, (data >> 16));
    }
    else if (!strcmp(cmd, "i"))
    {
        // default dev 0 is WM8750
        WM8750_CSB = (no) ? 0 : 1;
        printf("wm codec address: 0x%02x\n", WM8750_I2C_ADDR);
        wm8750_init();
    }
    else if (!strcmp(cmd, "f"))
        wm8750_finish();
    else if (!strcmp(cmd, "b"))
    {
        no = (no == 1);
        wm8750_bypass(no);
        // 0 is LINPUT/RINPUT; 1 is LINPUT2/RINPUT2
    }
    else if (!strcmp(cmd, "io"))
    {
        no = (no == 1);
        wm8750_adc_dac(no);
        // 0 is LINPUT/RINPUT; 1 is LINPUT2/RINPUT2
    }
    else if (!strcmp(cmd, "d"))
    {
        int idx;
        for (idx = 0; idx <= 0x2a; idx++)
        {                       //wm8750 has 0x2a registers
            if ((idx != 0x4) && (idx != 0x6) && (idx != 0x9) && (idx != 0xf))
            {                   //these are reserve register
                wm8750_reg_read(idx, (unsigned short *) &data);
                printf("read reg%d = 0x%03x\n", idx, (data >> 16));
            }
        }
    }
    else if (!strcmp(cmd, "mic"))
    {
        wm8750_reg_write(WM8750_PWR1,
                         (wm8750_reg[WM8750_PWR1] & ~0x2) | (no << 1));
    }
    else
        goto err;

    //printf("failed: %ld \n",failed);
    //printf("Done: %s \n", (0==failed)? "PASSED" : "FAILED");
    return (0 == failed) ? ERR_OK : ERR_PARM;

  help:
  err:
    return ERR_HELP;
}

cmdt cmdt_wm8750[] __attribute__ ((section("cmdt"))) =
{
    {
    "wm", wm_cmd,
            "wm8750 i/f ; init/finish control interface\n"
            "wm8750 r/w <reg> <data> ; read/write register\n"
            "wm8750 d ; dump registers except reserve register\n"
            "wm8750 b <no>(0:INPUT1 1:INPUT2); bypass setting\n"
            "wm8750 io <no>(0:INPUT1 1:INPUT2); adc dac setting\n"
            "wm8750 ifm <no>(0:PCM 1:I2S)\n"
            "wm8750 rol <no>(0:master 1:slave)\n"
            "wm8750 dat <no>(0:16bit 1:20bit 2:24bit 3:32bit)\n"
            "wm8750 bclk <no>(0:normal 1:inverted])\n"
            "wm8750 div2 <no>(0:normal 1:divided by 2)\n"
            "wm8750 mic <no>(0:disable 1:enable); MICBIAS\n"
            "wm8750 sr <no>(SR[4:0])"}
,};
