/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file i2c_nau8822.c
*   \brief access nau8822 register by I2C
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
/* NAU8822 register space */

#define NAU8822_R00_RESET		    0x00
#define NAU8822_R01_POWER1		    0x01
#define NAU8822_R02_POWER2		    0x02
#define NAU8822_R03_POWER3		    0x03
#define NAU8822_R04_IFACE		    0x04
#define NAU8822_R05_COMP		    0x05
#define NAU8822_R06_CLOCK1		    0x06
#define NAU8822_R07_CLOCK2		    0x07
#define NAU8822_R08_GPIO		    0x08
#define NAU8822_R09_JDECT1		    0x09
#define NAU8822_R0A_DAC			    0x0A
#define NAU8822_R0B_LDACVOL	        0x0B
#define NAU8822_R0C_RDACVOL		    0x0C
#define NAU8822_R0D_JDECT2		    0x0D
#define NAU8822_R0E_ADC			    0x0E
#define NAU8822_R0F_LADCVOL		    0x0F
#define NAU8822_R10_RADCVOL		    0x10
#define NAU8822_R12_EQ1			    0x12
#define NAU8822_R13_EQ2			    0x13
#define NAU8822_R14_EQ3			    0x14
#define NAU8822_R15_EQ4			    0x15
#define NAU8822_R16_EQ5			    0x16
#define NAU8822_R18_DACLIM1		    0x18
#define NAU8822_R19_DACLIM2		    0x19
#define NAU8822_R1B_NOTCH1		    0x1B
#define NAU8822_R1C_NOTCH2		    0x1C
#define NAU8822_R1D_NOTCH3		    0x1D
#define NAU8822_R1E_NOTCH4		    0x1E
#define NAU8822_R20_ALC1		    0x20
#define NAU8822_R21_ALC2		    0x21
#define NAU8822_R22_ALC3		    0x22
#define NAU8822_R23_NGATE		    0x23
#define NAU8822_R24_PLLN		    0x24
#define NAU8822_R25_PLLK1		    0x25
#define NAU8822_R26_PLLK2		    0x26
#define NAU8822_R27_PLLK3		    0x27
#define NAU8822_R29_3D			    0x29
#define NAU8822_R2B_RSPKMIX		    0x2B
#define NAU8822_R2C_INPUT		    0x2C
#define NAU8822_R2D_LINPPGA  	    0x2D
#define NAU8822_R2E_RINPPGA		    0x2E
#define NAU8822_R2F_LADCBOOST	    0x2F
#define NAU8822_R30_RADCBOOST	    0x30
#define NAU8822_R31_OUTPUT		    0x31
#define NAU8822_R32_LMIX 	        0x32
#define NAU8822_R33_RMIX		    0x33
#define NAU8822_R34_LHPVOL		    0x34
#define NAU8822_R35_RHPVOL		    0x35
#define NAU8822_R36_LSPKVOL         0x36
#define NAU8822_R37_RSPKVOL		    0x37
#define NAU8822_R38_MOUTMIX		    0x38
#define NAU8822_R39_AUX1MIX		    0x39
#define NAU8822_R3A_PWRL            0x3A
#define NAU8822_R3B_LPCMSLOT        0x3B
#define NAU8822_R3C_MISC		    0x3C
#define NAU8822_R3D_RPCMSLOT	    0x3D
#define NAU8822_R46_ALCEnhance1     0x46
#define NAU8822_R47_ALCEnhance2     0x47
#define NAU8822_R49_ADDIFCTRL       0x49
#define NAU8822_R4A_TIEOFFOVDR	    0x4A
#define NAU8822_R51_PTIEOFFCTRL	    0x51
#define NAU8822_R4C_ALCP2PDET       0x4C
#define NAU8822_R4D_ALCPEAKDET      0x4D
#define NAU8822_R4E_CTRLSTATUS      0x4E
#define NAU8822_R4F_OUTTIECTRL      0x4F

#define NAU8822_I2C_ADDR 0x34
#define nau8822_log(fmt, args...)       //printf(fmt, ##args)
#define dump_hex(a, b, c) { printf(a"\n"); diag_dump_buf_16bit(b ,c); }
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static unsigned short nau8822_regs[] = {
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x00...0x03 */
    0x0050, 0x0000, 0x0140, 0x0000,     /* 0x04...0x07 */
    0x0000, 0x0000, 0x0000, 0x00ff,     /* 0x08...0x0b */
    0x00ff, 0x0000, 0x0100, 0x00ff,     /* 0x0c...0x0f */
    0x00ff, 0x0000, 0x012c, 0x002c,     /* 0x10...0x13 */
    0x002c, 0x002c, 0x002c, 0x0000,     /* 0x14...0x17 */
    0x0032, 0x0000, 0x0000, 0x0000,     /* 0x18...0x1b */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x1c...0x1f */
    0x0038, 0x000b, 0x0032, 0x0000,     /* 0x20...0x23 */
    0x0008, 0x000c, 0x0093, 0x00e9,     /* 0x24...0x27 */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x28...0x2b */
    0x0033, 0x0010, 0x0010, 0x0100,     /* 0x2c...0x2f */
    0x0100, 0x0002, 0x0001, 0x0001,     /* 0x30...0x33 */
    0x0039, 0x0039, 0x0039, 0x0039,     /* 0x34...0x37 */
    0x0001, 0x0001, 0x0000, 0x0000,     /* 0x38...0x3b */
    0x0020, 0x0020, 0x0000, 0x001a,     /* 0x3c...0x3f */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x40...0x43 */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x44...0x47 */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x48...0x4b */
    0x0000, 0x0000, 0x0000, 0x0000,     /* 0x4c...0x4f */
    0x0000, 0x0000,             /* 0x50...0x53 */
};

#define NAU8822_REGS_BUFFER_SIZE (sizeof(nau8822_regs)/sizeof(unsigned short))
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static void diag_dump_buf_with_offset_16bit(unsigned short *p,
                                            unsigned int s,
                                            unsigned short *base)
{
    int i;
    if ((unsigned int) s > (unsigned int) p)
    {
        s = (unsigned int) s - (unsigned int) p;
    }
    while ((int) s > 0)
    {
        if (base)
        {
            printf("%08X: ", (unsigned int) p - (unsigned int) base);
        }
        else
        {
            printf("%08X: ", p);
        }
        for (i = 0; i < 8; i++)
        {
            if (i < (int) s / 2)
            {
                printf("%04X ", p[i]);
                if (i == 3)
                    printf(" ");
            }
            else
            {
                printf("     ");
            }
        }
        printf("\n");
        s -= 16;
        p += 8;
    }
}

static void diag_dump_buf_16bit(void *p, unsigned int s)
{
    diag_dump_buf_with_offset_16bit((unsigned short *) p, s, 0);
}

/*!
 * function: nau8822_reg_write_hw
 *
 *  \brief write nau8822 register by I2C
 *  \param reg
 *  \param value
 *  \return int
 */
int nau8822_reg_write_hw(int reg, unsigned short value)
{
    int rc;
    nau8822_log("[wm8750 register write]\n");

    i2c_start();

    rc = i2c_write(NAU8822_I2C_ADDR);
    if (rc)
        goto frc;
    rc = i2c_write(((reg << 1) | (value >> 8)));        // addr(7bit) + data(first bit)
    if (rc)
        goto frc;
    rc = i2c_write((char) (value & 0x00FF));    // data(8bit)

  frc:
    i2c_stop();
    if (rc)
        printf("nau8822 write reg%d fail!!!\n", reg);
    return rc;
}

int nau8822_reg_write(int reg, unsigned short value)
{
    int ret = nau8822_reg_write_hw(reg, value);
    if (ret >= 0)
    {
        if (reg < NAU8822_REGS_BUFFER_SIZE)
        {
            nau8822_regs[reg] = value;
        }
    }
    return ret;
}

/*!
 * function: nau8822_reg_read_hw
 *
 *  \brief read nau8822 register by I2C
 *  \param reg
 *  \return
 */
int nau8822_reg_read_hw(int reg)
{
    union i2c_smbus_data
    {
        unsigned char byte[2];
        unsigned short word;
    } data =
    {
    .word = 0};
    int rc;
    nau8822_log("[nau8822 register read]\n");

    i2c_start();

    rc = i2c_write(NAU8822_I2C_ADDR);
    if (rc)
        goto frc;
    rc = i2c_write((reg << 1)); // addr(7bit)
    if (rc)
        goto frc;
    i2c_restart();
    rc = i2c_write(NAU8822_I2C_ADDR | 0x01);
    if (rc)
        goto frc;
    data.byte[0] = i2c_read(0); // data(first 1 bit)
    data.byte[1] = i2c_read(1); // data(8 bits)

  frc:
    i2c_stop();
    if (rc)
    {
        printf("nau8822 read reg%d fail!!!\n", reg);
        return rc;
    }
    else
        return data.word;
}

int nau8822_reg_read(int reg)
{
    int val;
    val = nau8822_reg_read_hw(reg);
    if (val >= 0)
    {
        if (reg < NAU8822_REGS_BUFFER_SIZE)
        {
            nau8822_regs[reg] = val;
        }
    }
    return val;
}

void nau8822_reg_read_all(void)
{
    int i;
    for (i = NAU8822_R00_RESET; i < NAU8822_REGS_BUFFER_SIZE; i++)
        nau8822_reg_read(i);
}

/*!
 * function: nau8822_init
 *
 *  \brief initialize I2C for accessing nau8822
 *  \return
 */
void nau8822_init(void)
{
    nau8822_log("[init I2C module for access nau8822 register]\n");
    i2c_init();
}

/*!
 * function: nau8822_finish
 *
 *  \brief finish I2C setting
 *  \return
 */
void nau8822_finish(void)
{
    nau8822_log("[finish I2C module]\n");
    i2c_finish();
}

/*!
 * function: nau8822_bypass
 *
 *  \brief line bypass setting
 *  \return
 */
void nau8822_bypass(void)
{
    nau8822_log("[do bypass setting]\n");
    nau8822_reg_write(NAU8822_R00_RESET, 0x001);
    nau8822_reg_read_all();

    nau8822_reg_write(NAU8822_R01_POWER1, 0x02d);       // ABIASEN=1, PLL=1, REFIMP=1
    nau8822_reg_write(NAU8822_R02_POWER2, 0x1b0);       // RHPEN=1, NHPEN=1, RBSTEN=1, LBSTEN=1
    nau8822_reg_write(NAU8822_R03_POWER3, 0x00c);       // RMIXEN=1, LMIXEN=1
    nau8822_reg_write(NAU8822_R2F_LADCBOOST, 0x007);    // LAUXBSTGAIN=7
    nau8822_reg_write(NAU8822_R30_RADCBOOST, 0x007);    // RAUXBSTGAIN=7
    nau8822_reg_write(NAU8822_R32_LMIX, 0x002); // BYPSPK=1
    nau8822_reg_write(NAU8822_R33_RMIX, 0x002); // BYPSPK=1
}

/*!
 * function: nau8822_adc_dac
 *
 *  \brief
 *  \return
 */
void nau8822_adc_dac(void)
{
    nau8822_log("[do adc_dac setting]\n");
    nau8822_reg_write(NAU8822_R00_RESET, 0x001);
    nau8822_reg_read_all();

    nau8822_reg_write(NAU8822_R01_POWER1, 0x12d);       // DCBUFEN, PLLEN, ABIASEN, REFIMP
    nau8822_reg_write(NAU8822_R02_POWER2, 0x1b3);       // RHPEN, NHPEN, RBSTEN, LBSTEN, RADCEN, LADCEN
    nau8822_reg_write(NAU8822_R03_POWER3, 0x00f);       // RMIXEN, LMIXEN, RDACEN, LDACEN
    nau8822_reg_write(NAU8822_R04_IFACE, 0x010);        // 16 bits, I2S
    nau8822_reg_write(NAU8822_R06_CLOCK1, 0x008);       // CLKM=0(MCLK from Ours)
    // MCLKSEL=0(MCLK/1), BCLKSEL=2(MCLK/4)
    nau8822_reg_write(NAU8822_R2F_LADCBOOST, 0x077);    // LPGABSTGAIN(LIN), LAUXBSTGAIN=7(AUXIN)
    nau8822_reg_write(NAU8822_R30_RADCBOOST, 0x077);    // RPGABSTGAIN(LIN), RAUXBSTGAIN=7(AUXIN)
    printf("defalut Slave mode & 16 bits & I2S Format\n");
}

/*!
 * function: nau8822
 *
 *  \brief command to access nau8822 register by I2C
 *  \param argc
 *  \param argv
 *  \return
 */
int nau_cmd(int argc, char *argv[])
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
        no = (no == 0) ? 0x18 : 0x10;
        nau8822_reg_write(NAU8822_R04_IFACE,
                          (nau8822_regs[NAU8822_R04_IFACE] & ~0x018) | no);
    }
    else if (!strcmp(cmd, "rol"))
    {
        no = (no == 0);
        nau8822_reg_write(NAU8822_R06_CLOCK1,
                          (nau8822_regs[NAU8822_R06_CLOCK1] & ~0x01) | (no <<
                                                                        0));
    }
    else if (!strcmp(cmd, "dat"))
    {
        nau8822_reg_write(NAU8822_R04_IFACE,
                          (nau8822_regs[NAU8822_R04_IFACE] & ~0x60) | (no <<
                                                                       5));
    }
    else if (!strcmp(cmd, "bclk"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        nau8822_reg_write(NAU8822_R04_IFACE,
                          (nau8822_regs[NAU8822_R04_IFACE] & ~0x100) | (no <<
                                                                        8));
    }
    else if (!strcmp(cmd, "dacphs"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        nau8822_reg_write(NAU8822_R04_IFACE,
                          (nau8822_regs[NAU8822_R04_IFACE] & ~0x04) | (no <<
                                                                       2));
    }
    else if (!strcmp(cmd, "msel"))
    {
        int mode[8] = { 1, 0, 2, 3, 4, 6, 8, 12 };      // mode[1] is 1.5
        nau8822_reg_write(NAU8822_R06_CLOCK1,
                          (nau8822_regs[NAU8822_R06_CLOCK1] & ~0xe0) | (no <<
                                                                        5));
        if (no == 1)
            printf("IMCLK = MCLK/1.5\n");
        else if (no >= 0 && no < 8)
            printf("IMCLK = MCLK/%d\n", mode[no]);
        else
        {
            printf("unknown MCLKSEL = %d\n", no);
            goto err;
        }
    }
    else if (!strcmp(cmd, "bsel"))
    {
        int mode[6] = { 1, 2, 4, 8, 16, 32 };
        nau8822_reg_write(NAU8822_R06_CLOCK1,
                          (nau8822_regs[NAU8822_R06_CLOCK1] & ~0x1c) | (no <<
                                                                        2));
        if (no >= 0 && no < 6)
            printf("BCLK = IMCLK/%d\n", mode[no]);
        else
        {
            printf("unknown BCLKSEL = %d\n", no);
            goto err;
        }
    }
    else if (!strcmp(cmd, "w"))
    {
        if (nau8822_reg_write(no, data))
            failed |= (1 << 0);
        printf("write reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "r"))
    {
        data = nau8822_reg_read(no);
        printf("read reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "all"))
    {
        nau8822_reg_read_all();
        dump_hex("nau8822 Regs:", nau8822_regs, NAU8822_REGS_BUFFER_SIZE * 2);
    }
    else if (!strcmp(cmd, "i"))
        nau8822_init();
    else if (!strcmp(cmd, "f"))
        nau8822_finish();
    else if (!strcmp(cmd, "b"))
        nau8822_bypass();
    else if (!strcmp(cmd, "io"))
        nau8822_adc_dac();
    else if (!strcmp(cmd, "timeslot"))
        nau8822_reg_write(NAU8822_R3C_MISC,
                          (nau8822_regs[NAU8822_R3C_MISC] & ~0x100) | (no <<
                                                                       8));
    else if (!strcmp(cmd, "slt"))
    {
        if (no == 0)
            nau8822_reg_write(NAU8822_R3B_LPCMSLOT, (1 + data * 8));
        else
            nau8822_reg_write(NAU8822_R3D_RPCMSLOT, (1 + data * 8));
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

cmdt cmdt_nau8822[] __attribute__ ((section("cmdt"))) =
{
    {
    "nau", nau_cmd,
            "IMCLK = 256 * sample rate\n"
            "nau8822 i/f ; init/finish control interface\n"
            "nau8822 r/w <reg> <data> ; read/write register\n"
            "nau8822 all ; read all registers\n"
            "nau8822 b ; bypass setting\n"
            "nau8822 io ; adc dac setting\n"
            "nau8822 ifm <no>(0:PCM 1:I2S)\n"
            "nau8822 rol <no>(0:master 1:slave)\n"
            "nau8822 dat <no>(0:16bit 1:20bit 2:24bit 3:32bit)\n"
            "nau8822 bclk <no>(0:normal 1:inverted])\n"
            "nau8822 dacphs <no>(0:L 1:R channel (channel swap)])\n"
            "nau8822 msel <no>(0:1 1:1.5 2:2 3:3 4:4 5:6 6:8 7:12 (divided by))\n"
            "nau8822 bsel <no>(0:1 1:2 2:4 3:8 4:16 5:32 (divided by))\n"
            "== PCM ==\n"
            "nau8222 timeslot <no>(0:disable 1:enable)\n"
            "nau8822 slt <ch>(0:L 1:R) <no>(0~7f)"}
,};
