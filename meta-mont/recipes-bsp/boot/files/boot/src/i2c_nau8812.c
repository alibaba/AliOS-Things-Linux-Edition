/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file i2c_nau8812.c
*   \brief access nau8812 register by I2C
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
/* NAU8812 register space */

#define NAU8812_R00_RESET		    0x00
#define NAU8812_R01_POWER1		    0x01
#define NAU8812_R02_POWER2		    0x02
#define NAU8812_R03_POWER3		    0x03
#define NAU8812_R04_IFACE		    0x04
#define NAU8812_R05_COMP		    0x05
#define NAU8812_R06_CLOCK1		    0x06
#define NAU8812_R07_CLOCK2		    0x07
#define NAU8812_R08_GPIO		    0x08
#define NAU8812_R0A_DAC			    0x0A
#define NAU8812_R0B_DACVOL 	        0x0B
#define NAU8812_R0E_ADC			    0x0E
#define NAU8812_R0F_ADCVOL		    0x0F
#define NAU8812_R12_EQ1			    0x12
#define NAU8812_R13_EQ2			    0x13
#define NAU8812_R14_EQ3			    0x14
#define NAU8812_R15_EQ4			    0x15
#define NAU8812_R16_EQ5			    0x16
#define NAU8812_R18_DACLIM1		    0x18
#define NAU8812_R19_DACLIM2		    0x19
#define NAU8812_R1B_NOTCH1		    0x1B
#define NAU8812_R1C_NOTCH2		    0x1C
#define NAU8812_R1D_NOTCH3		    0x1D
#define NAU8812_R1E_NOTCH4		    0x1E
#define NAU8812_R20_ALC1		    0x20
#define NAU8812_R21_ALC2		    0x21
#define NAU8812_R22_ALC3		    0x22
#define NAU8812_R23_NGATE		    0x23
#define NAU8812_R24_PLLN		    0x24
#define NAU8812_R25_PLLK1		    0x25
#define NAU8812_R26_PLLK2		    0x26
#define NAU8812_R27_PLLK3		    0x27
#define NAU8812_R28_VIDEO		    0x28
#define NAU8812_R2C_INPUT		    0x2C
#define NAU8812_R2D_INPPGA  	    0x2D
#define NAU8812_R2F_ADCBOOST	    0x2F
#define NAU8812_R31_OUTPUT		    0x31
#define NAU8812_R32_MIX 	        0x32
#define NAU8812_R36_SPKVOL          0x36
#define NAU8812_R38_MOUTMIX		    0x38
#define NAU8812_R3A_PWRL            0x3A
#define NAU8812_R3B_PCMSLOT         0x3B

#define NAU8812_R45_HVCTRL          0x45
#define NAU8812_R46_ALCEnhance1     0x46
#define NAU8812_R47_ALCEnhance2     0x47
#define NAU8812_R49_ADDIFCTRL       0x49
#define NAU8812_R4B_POWTIEOFFCTRL   0x4B
#define NAU8812_R4C_ALCP2PDET       0x4C
#define NAU8812_R4D_ALCPEAKDET      0x4D
#define NAU8812_R4E_CTRLSTATUS      0x4E
#define NAU8812_R4F_OUTTIECTRL      0x4F

#define NAU8812_I2C_ADDR 0x34
#define nau8812_log(fmt, args...)       //printf(fmt, ##args)
#define dump_hex(a, b, c) { printf(a"\n"); diag_dump_buf_16bit(b ,c); }
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static unsigned short nau8812_regs[] = {
    0x0000, 0x0000, 0x0000, 0x0000,     // 0-3
    0x0050, 0x0000, 0x0140, 0x0000,     // 4-7
    0x0000, 0x0000, 0x0000, 0x00ff,     // 8-11
    0x00ff, 0x0000, 0x0100, 0x00ff,     // 12-15
    0x00ff, 0x0000, 0x012c, 0x002c,     // 16-19
    0x002c, 0x002c, 0x002c, 0x0000,     // 20-23
    0x0032, 0x0000, 0x0000, 0x0000,     // 24-27
    0x0000, 0x0000, 0x0000, 0x0000,     // 28-31
    0x0038, 0x000b, 0x0032, 0x0000,     // 32-35
    0x0008, 0x000c, 0x0093, 0x00e9,     // 36-39
    0x0000, 0x0000, 0x0000, 0x0000,     // 40-43
    0x0003, 0x0010, 0x0010, 0x0100,     // 44-47
    0x0100, 0x0002, 0x0001, 0x0001,     // 48-51
    0x0039, 0x0039, 0x0039, 0x0039,     // 52-55
    0x0001, 0x0001, 0x0000, 0x0000,     // 56-59
    0x0020, 0x0000, 0x00ef, 0x001a,     // 60-63
    0x00ca, 0x0124, 0x0000, 0x0000,     // 64-67
    0x0000, 0x0001, 0x0000, 0x0000,     // 68-71
    0x0000, 0x0000, 0x0000, 0x0000,     // 72-75
    0x0000, 0x0000, 0x0000, 0x0000,     // 76-79
};

#define NAU8812_REGS_BUFFER_SIZE (sizeof(nau8812_regs)/sizeof(unsigned short))
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void diag_dump_buf_with_offset_16bit(unsigned short *p,
                                     unsigned int s, unsigned short *base)
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

void diag_dump_buf_16bit(void *p, unsigned int s)
{
    diag_dump_buf_with_offset_16bit((unsigned short *) p, s, 0);
}

/*!
 * function: nau8812_reg_write_hw
 *
 *  \brief write nau8812 register by I2C
 *  \param reg
 *  \param value
 *  \return int
 */
int nau8812_reg_write_hw(int reg, unsigned short value)
{
    int rc;
    nau8812_log("[wm8750 register write]\n");

    i2c_start();

    rc = i2c_write(NAU8812_I2C_ADDR);
    if (rc)
        goto frc;
    rc = i2c_write(((reg << 1) | (value >> 8)));        // addr(7bit) + data(first bit)
    if (rc)
        goto frc;
    rc = i2c_write((char) (value & 0x00FF));    // data(8bit)

  frc:
    i2c_stop();
    if (rc)
        printf("nau8812 write reg%d fail!!!\n", reg);
    return rc;
}

int nau8812_reg_write(int reg, unsigned short value)
{
    int ret = nau8812_reg_write_hw(reg, value);
    if (ret >= 0)
    {
        if (reg < NAU8812_REGS_BUFFER_SIZE)
        {
            nau8812_regs[reg] = value;
        }
    }
    return ret;
}

/*!
 * function: nau8812_reg_read_hw
 *
 *  \brief read nau8812 register by I2C
 *  \param reg
 *  \return
 */
int nau8812_reg_read_hw(int reg)
{
    union i2c_smbus_data
    {
        unsigned char byte[2];
        unsigned short word;
    } data =
    {
    .word = 0};
    int rc;
    nau8812_log("[nau8812 register read]\n");

    i2c_start();

    rc = i2c_write(NAU8812_I2C_ADDR);
    if (rc)
        goto frc;
    rc = i2c_write((reg << 1)); // addr(7bit)
    if (rc)
        goto frc;
    i2c_restart();
    rc = i2c_write(NAU8812_I2C_ADDR | 0x01);
    if (rc)
        goto frc;
    data.byte[0] = i2c_read(0); // data(first 1 bit)
    data.byte[1] = i2c_read(1); // data(8 bits)

  frc:
    i2c_stop();
    if (rc)
    {
        printf("nau8812 read reg%d fail!!!\n", reg);
        return rc;
    }
    else
        return data.word;
}

int nau8812_reg_read(int reg)
{
    int val;
    val = nau8812_reg_read_hw(reg);
    if (val >= 0)
    {
        if (reg < NAU8812_REGS_BUFFER_SIZE)
        {
            nau8812_regs[reg] = val;
        }
    }
    return val;
}

void nau8812_reg_read_all(void)
{
    int i;
    for (i = NAU8812_R00_RESET; i < NAU8812_REGS_BUFFER_SIZE; i++)
        nau8812_reg_read(i);
}

/*!
 * function: nau8812_init
 *
 *  \brief initialize I2C for accessing nau8812
 *  \return
 */
void nau8812_init(void)
{
    nau8812_log("[init I2C module for access nau8812 register]\n");
    i2c_init();
}

/*!
 * function: nau8812_finish
 *
 *  \brief finish I2C setting
 *  \return
 */
void nau8812_finish(void)
{
    nau8812_log("[finish I2C module]\n");
    i2c_finish();
}

/*!
 * function: nau8812_bypass
 *
 *  \brief line bypass setting
 *  \return
 */
void nau8812_bypass(void)
{
    nau8812_log("[do bypass setting]\n");
    nau8812_reg_write(NAU8812_R00_RESET, 0x001);
    nau8812_reg_read_all();

    nau8812_reg_write(NAU8812_R01_POWER1, 0x008);       // ABIASEN=1
    nau8812_reg_write(NAU8812_R02_POWER2, 0x010);       // BSTEN=1
    nau8812_reg_write(NAU8812_R03_POWER3, 0x064);       // NSPKEN=1, PSPKEN=1, SPKMXEN=1
    nau8812_reg_write(NAU8812_R2F_ADCBOOST, 0x170);     // PGABST(+20dB) PMICBSTGAIN(input)
    nau8812_reg_write(NAU8812_R32_MIX, 0x002);  // BYPSPK=1
}

/*!
 * function: nau8812_adc_dac
 *
 *  \brief
 *  \return
 */
void nau8812_adc_dac(void)
{
    nau8812_log("[do adc_dac setting]\n");
    nau8812_reg_write(NAU8812_R00_RESET, 0x001);
    nau8812_reg_read_all();

    nau8812_reg_write(NAU8812_R01_POWER1, 0x128);       // DCBUFEN, PLLEN, ABIASEN
    nau8812_reg_write(NAU8812_R02_POWER2, 0x011);       // BSTEN, ADCEN (input)
    nau8812_reg_write(NAU8812_R03_POWER3, 0x065);       // NSPKEN, PSPKEN, SPKMXEN, DACEN (output)
    nau8812_reg_write(NAU8812_R04_IFACE, 0x010);        // 16 bits, I2S
    nau8812_reg_write(NAU8812_R06_CLOCK1, 0x008);       // CLKM=0(MCLK from Ours)
    // MCLKSEL=0(MCLK/1), BCLKSEL=2(MCLK/4)
    nau8812_reg_write(NAU8812_R2F_ADCBOOST, 0x070);     // PMICBSTGAIN(input)
    printf("defalut Slave mode & 16 bits & I2S Format\n");
}

/*!
 * function: nau8812
 *
 *  \brief command to access nau8812 register by I2C
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
        nau8812_reg_write(NAU8812_R04_IFACE,
                          (nau8812_regs[NAU8812_R04_IFACE] & ~0x018) | no);
    }
    else if (!strcmp(cmd, "rol"))
    {
        no = (no == 0);
        nau8812_reg_write(NAU8812_R06_CLOCK1,
                          (nau8812_regs[NAU8812_R06_CLOCK1] & ~0x01) | (no <<
                                                                        0));
    }
    else if (!strcmp(cmd, "dat"))
    {
        nau8812_reg_write(NAU8812_R04_IFACE,
                          (nau8812_regs[NAU8812_R04_IFACE] & ~0x60) | (no <<
                                                                       5));
    }
    else if (!strcmp(cmd, "bclk"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        nau8812_reg_write(NAU8812_R04_IFACE,
                          (nau8812_regs[NAU8812_R04_IFACE] & ~0x100) | (no <<
                                                                        8));
    }
    else if (!strcmp(cmd, "dacphs"))
    {
        no = (no == 0) ? 0x0 : 0x1;
        nau8812_reg_write(NAU8812_R04_IFACE,
                          (nau8812_regs[NAU8812_R04_IFACE] & ~0x04) | (no <<
                                                                       2));
    }
    else if (!strcmp(cmd, "msel"))
    {
        int mode[8] = { 1, 0, 2, 3, 4, 6, 8, 12 };      // mode[1] is 1.5
        nau8812_reg_write(NAU8812_R06_CLOCK1,
                          (nau8812_regs[NAU8812_R06_CLOCK1] & ~0xe0) | (no <<
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
        nau8812_reg_write(NAU8812_R06_CLOCK1,
                          (nau8812_regs[NAU8812_R06_CLOCK1] & ~0x1c) | (no <<
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
        if (nau8812_reg_write(no, data))
            failed |= (1 << 0);
        printf("write reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "r"))
    {
        data = nau8812_reg_read(no);
        printf("read reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "all"))
    {
        nau8812_reg_read_all();
        dump_hex("nau8812 Regs:", nau8812_regs, NAU8812_REGS_BUFFER_SIZE * 2);
    }
    else if (!strcmp(cmd, "i"))
        nau8812_init();
    else if (!strcmp(cmd, "f"))
        nau8812_finish();
    else if (!strcmp(cmd, "b"))
        nau8812_bypass();
    else if (!strcmp(cmd, "io"))
        nau8812_adc_dac();
    else
        goto err;

    //printf("failed: %ld \n",failed);
    //printf("Done: %s \n", (0==failed)? "PASSED" : "FAILED");
    return (0 == failed) ? ERR_OK : ERR_PARM;

  help:
  err:
    return ERR_HELP;
}

cmdt cmdt_nau8812[] __attribute__ ((section("cmdt"))) =
{
    {
    "nau", nau_cmd,
            "IMCLK = 256 * sample rate\n"
            "nau8812 i/f ; init/finish control interface\n"
            "nau8812 r/w <reg> <data> ; read/write register\n"
            "nau8812 all ; read all registers\n"
            "nau8812 b ; bypass setting\n"
            "nau8812 io ; adc dac setting\n"
            "nau8812 ifm <no>(0:PCM 1:I2S)\n"
            "nau8812 rol <no>(0:master 1:slave)\n"
            "nau8812 dat <no>(0:16bit 1:20bit 2:24bit 3:32bit)\n"
            "nau8812 bclk <no>(0:normal 1:inverted])\n"
            "nau8812 dacphs <no>(0:L 1:R channel (channel swap)])\n"
            "nau8812 msel <no>(0:1 1:1.5 2:2 3:3 4:4 5:6 6:8 7:12 (divided by))\n"
            "nau8812 bsel <no>(0:1 1:2 2:4 3:8 4:16 5:32 (divided by))"}
,};
