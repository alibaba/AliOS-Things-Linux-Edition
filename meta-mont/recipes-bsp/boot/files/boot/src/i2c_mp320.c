/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file i2c_mp320.c
*   \brief access mp320 register by I2C
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <i2c.h>
#include <lib.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
/* MP320 register space */

#define MP320_LINVOL    0x00
#define MP320_RINVOL    0x01
#define MP320_LOUT1V    0x02
#define MP320_ROUT1V    0x03
#define MP320_ADCDAC    0x05
#define MP320_IFACE     0x07
#define MP320_SRATE     0x08
#define MP320_LDAC      0x0a
#define MP320_RDAC      0x0b
#define MP320_BASS      0x0c
#define MP320_TREBLE    0x0d
#define MP320_RESET     0x0f
#define MP320_3D        0x10
#define MP320_ALC1      0x11
#define MP320_ALC2      0x12
#define MP320_ALC3      0x13
#define MP320_NGATE     0x14
#define MP320_LADC      0x15
#define MP320_RADC      0x16
#define MP320_ADCTL1    0x17
#define MP320_ADCTL2    0x18
#define MP320_PWR1      0x19
#define MP320_PWR2      0x1a
#define MP320_ADCTL3    0x1b
#define MP320_ADCIN     0x1f
#define MP320_LADCIN    0x20
#define MP320_RADCIN    0x21
#define MP320_LOUTM1    0x22
#define MP320_LOUTM2    0x23
#define MP320_ROUTM1    0x24
#define MP320_ROUTM2    0x25
#define MP320_MOUTM1    0x26
#define MP320_MOUTM2    0x27
#define MP320_LOUT2V    0x28
#define MP320_ROUT2V    0x29
#define MP320_MOUTV     0x2a

/* CSB is low/high; addr is 0x34/0x36 */
static int MP320_CSB = 1;
#define MP320_I2C_ADDR (MP320_CSB ? 0x1B : 0x1A)
#define mp320_log(fmt, args...) //printf(fmt, ##args)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static unsigned short mp320_reg[] = {
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
 * function: mp320_reg_write
 *
 *  \brief write mp320 register by I2C
 *  \param reg
 *  \param data
 *  \return int
 */
int mp320_reg_write(unsigned char reg, unsigned short data)
{
    int rc;
    mp320_log("[mp320 register write]\n");

    i2c_start(MP320_I2C_ADDR, 0);

    rc = i2c_write((reg << 1) + ((data & 0x100) >> 8), 0);
    if (rc)
        goto frc;
    rc = i2c_write((data & 0xff), 1);

  frc:
    //i2c_stop();
    if (rc)
        printf("mp320 write reg%d fail!!!\n", reg);
    else
        mp320_reg[reg] = (data & 0x1ff);        //upadte cached value
    return rc;
}

/*!
 * function: mp320_reg_read
 *
 *  \brief read mp320 register by I2C
 *  \return
 */
void mp320_reg_read(unsigned char reg, unsigned short *ptr)
{
    mp320_log("[mp320 register read]\n");
    *ptr = mp320_reg[reg];
}

/*!
 * function: mp320_init
 *
 *  \brief initialize I2C for accessing mp320
 *  \return
 */
void mp320_init(void)
{
    mp320_log("[init I2C module for access mp320 register]\n");
    i2c_init();
}

/*!
 * function: mp320_finish
 *
 *  \brief finish I2C setting
 *  \return
 */
void mp320_finish(void)
{
    mp320_log("[finish I2C module]\n");
    i2c_finish();
}

/*!
 * function: mp320_bypass
 *
 *  \brief line bypass setting
 *  \param isInput2
 *  \return
 */
void mp320_bypass(int isInput2)
{
    mp320_log("[do bypass setting]\n");
    mp320_reg_write(0x0f, 0x000);       // RESET 0x0F
    mp320_reg_write(0x19, 0x0C0);       // VREF 0x19
    mp320_reg_write(0x1a, 0x078);       // L/ROUT1_2 0x1A
    mp320_reg_write(0x02, 0x179);       // LOUT1 0x02
    mp320_reg_write(0x03, 0x179);       // ROUT1 0x03
    mp320_reg_write(0x22, 0x0A0);       // LI2LO 0x22
    mp320_reg_write(0x25, 0x0A0);       // RI2RO 0x25
    mp320_reg_write(0x28, 0x179);       // LOUT2 0x28
    mp320_reg_write(0x29, 0x179);       // ROUT2 0x29

    // LMIXSEL/RMIXSEL default are LINPUT1/RINPUT1 0x22/0x24[2:0]
    if (isInput2)
    {
        mp320_reg_write(0x20, 0x040);   // Set the ADCL signal to LINPUT2 R32
        mp320_reg_write(0x21, 0x040);   // Set the ADCL signal to RINPUT2 R33
        // set LMIXSEL/RMIXSEL to LINPUT2/RINPUT2
        mp320_reg_write(0x22, 0x0A1);   // Set Left Input Selection is LINPUT2, and LI2LO
        mp320_reg_write(0x24, 0x001);   // Set Right Input Selection is RINPUT2
    }
}

void mp320_init_i2s()
{
    i2c_init();
    mp320_log("[do adc_dac setting]\n");
    mp320_reg_write(0x0f, 0x000);       // RESET R15
    mp320_reg_write(0x19, 0x0fc);
    mp320_reg_write(0x1a, 0x1f8);
    mp320_reg_write(0x0a, 0x1ff);
    mp320_reg_write(0x0b, 0x1ff);
    mp320_reg_write(0x02, 0x17f);
    mp320_reg_write(0x03, 0x17f);
    mp320_reg_write(0x28, 0x1f9);
    mp320_reg_write(0x29, 0x1f9);
    mp320_reg_write(0x00, 0x117);
    mp320_reg_write(0x01, 0x117);
    mp320_reg_write(0x22, 0x150);
    mp320_reg_write(0x25, 0x150);

    mp320_reg_write(0x07, 0x002);
    mp320_reg_write(0x08, 0x000); // 8,16,32,48,96k
    mp320_reg_write(0x05, 0x000);
    printf("defalut Slave mode & 16 bits & I2S Format\n");

}

void mp320_init_pcm()
{
    i2c_init();
    mp320_log("[do adc_dac setting]\n");
    mp320_reg_write(0x0f, 0x000);       // RESET R15
    mp320_reg_write(0x0a, 0x1ff);
    mp320_reg_write(0x0b, 0x1ff);
    mp320_reg_write(0x02, 0x1f9);
    mp320_reg_write(0x03, 0x1f9);
    mp320_reg_write(0x28, 0x1f9);
    mp320_reg_write(0x29, 0x1f9);
    mp320_reg_write(0x00, 0x179);
    mp320_reg_write(0x01, 0x179);
    mp320_reg_write(0x22, 0x150);
    mp320_reg_write(0x25, 0x150);
    mp320_reg_write(0x19, 0x0c2);
    mp320_reg_write(0x1a, 0x1f8);

    mp320_reg_write(0x07, 0x003);
    mp320_reg_write(0x08, 0x000);
    mp320_reg_write(0x05, 0x000);
    printf("defalut Slave mode & 16 bits & PCM Format\n");
}

/*!
 * function: mp320
 *
 *  \brief command to access mp320 register by I2C
 *  \param argc
 *  \param argv
 *  \return
 */
int mpcodec_cmd(int argc, char *argv[])
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
        mp320_reg_write(MP320_IFACE, (mp320_reg[MP320_IFACE] & ~0x03) | no);
    }
    else if (!strcmp(cmd, "rol"))
    {
        no = (no == 0);
        mp320_reg_write(MP320_IFACE,
                        (mp320_reg[MP320_IFACE] & ~0x40) | (no << 6));
    }
    else if (!strcmp(cmd, "dat"))
    {
        mp320_reg_write(MP320_IFACE,
                        (mp320_reg[MP320_IFACE] & ~0x0c) | (no << 2));
    }
    else if (!strcmp(cmd, "bclk"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        mp320_reg_write(MP320_IFACE,
                        (mp320_reg[MP320_IFACE] & ~0x80) | (no << 7));
    }
    else if (!strcmp(cmd, "div2"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        mp320_reg_write(MP320_SRATE,
                        (mp320_reg[MP320_SRATE] & ~0x40) | (no << 6));
    }
    else if (!strcmp(cmd, "sr"))
    {
        mp320_reg_write(MP320_SRATE,
                        (mp320_reg[MP320_SRATE] & ~0x3e) | (no << 1));
    }
    else if (!strcmp(cmd, "w"))
    {
        if (mp320_reg_write(no, data))
            failed |= (1 << 0);
        printf("write reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "r"))
    {
        mp320_reg_read(no, (unsigned short *) &data);
        printf("read reg%d = 0x%03x\n", no, (data >> 16));
    }
    else if (!strcmp(cmd, "i"))
    {
        // default dev 0 is MP320
        MP320_CSB = (no) ? 0 : 1;
        printf("wm codec address: 0x%02x\n", MP320_I2C_ADDR);
        mp320_init();
    }
    else if (!strcmp(cmd, "f"))
        mp320_finish();
    else if (!strcmp(cmd, "b"))
    {
        no = (no == 1);
        mp320_bypass(no);
        // 0 is LINPUT/RINPUT; 1 is LINPUT2/RINPUT2
    }
    else if (!strcmp(cmd, "pcm"))
    {
        mp320_init_pcm();
    }
    else if (!strcmp(cmd, "i2s"))
    {
        mp320_init_i2s();
    }
    else if (!strcmp(cmd, "d"))
    {
        int idx;
        for (idx = 0; idx <= 0x2a; idx++)
        {                       //mp320 has 0x2a registers
            if ((idx != 0x4) && (idx != 0x6) && (idx != 0x9) && (idx != 0xf))
            {                   //these are reserve register
                mp320_reg_read(idx, (unsigned short *) &data);
                printf("read reg%d = 0x%03x\n", idx, (data >> 16));
            }
        }
    }
    else if (!strcmp(cmd, "mic"))
    {
        mp320_reg_write(MP320_PWR1, (mp320_reg[MP320_PWR1] & ~0x2) | (no << 1));
    }
    else
        goto err;

    //printf("failed: %ld \n",failed);
    //printf("Done: %s \n", (0==failed)? "PASSED" : "FAILED");
    return (0 == failed) ? ERR_OK : ERR_PARM;

  err:
    return ERR_HELP;
}

cmdt cmdt_mp320[] __attribute__ ((section("cmdt"))) =
{
    {
    "mp", mpcodec_cmd,
            "mp320 i/f ; init/finish control interface\n"
            "mp320 r/w <reg> <data> ; read/write register\n"
            "mp320 d ; dump registers except reserve register\n"
            "mp320 b <no>(0:INPUT1 1:INPUT2); bypass setting\n"
            "mp320 pcm ; init pcm\n"
            "mp320 i2s ; init i2s\n"
            "mp320 ifm <no>(0:PCM 1:I2S)\n"
            "mp320 rol <no>(0:master 1:slave)\n"
            "mp320 dat <no>(0:16bit 1:20bit 2:24bit 3:32bit)\n"
            "mp320 bclk <no>(0:normal 1:inverted])\n"
            "mp320 div2 <no>(0:normal 1:divided by 2)\n"
            "mp320 mic <no>(0:disable 1:enable); MICBIAS\n"
            "mp320 sr <no>(SR[4:0])"}
,};
