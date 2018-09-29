/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file i2c_analog.c
*   \brief access wm8750 register by I2C
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <arch/chip.h>
#include <i2c.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
/* hw i2c slave register space */
#define HW_I2C_R00_OFS              0x00
#define HW_I2C_R01_DAT              0x01
#define HW_I2C_R02_DAT              0x02
#define HW_I2C_R03_DAT              0x03
#define HW_I2C_R04_DAT              0x04
#define HW_I2C_R07_CMD              0x07

#define HW_I2C_CMD_NULL             0x00
#define HW_I2C_CMD_WRITE            0x03
#define HW_I2C_CMD_READ             0x05

#define ANALOG_I2C_ADDR (0x34 << 1)
#define analog_log(fmt, args...)        //printf(fmt, ##args)
#define dump_hex(a, b, c) { printf(a"\n"); diag_dump_buf_32bit(b ,c); }
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
//36 word(4bytes) analog register bank address 0xaf005800
static unsigned int analog_regs[] = {
    0x80003000, 0x00004444, 0x00000000, 0xb1040202,     /*  0 */
    0x00000000, 0x00000000, 0x000f1011, 0x000001ff,     /*  4 */
    0x03c003c0, 0x00000020, 0x00000000, 0x00800000,     /*  8 */
    0x00000000, 0x00000000, 0x00000000, 0x000aa120,     /* 12 */
    0x0000fa06, 0x00000000, 0x00000000, 0x00000000,     /* 16 */
    0x0000001f, 0x00000000, 0x00000000, 0x00000000,     /* 20 */
    0x00000010, 0x00000000, 0x00000000, 0x00000000,     /* 24 */
    0x00000202, 0x33337777, 0x00010000, 0x00000001,     /* 28 */
    0x00000000, 0x11114444, 0x00010002, 0x00000000,     /* 32 */
};

#define ANALOG_REGS_BUFFER_SIZE (sizeof(analog_regs)/sizeof(unsigned int))

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void diag_dump_buf_with_offset_32bit(unsigned int *p,
                                     unsigned int s, unsigned int *base)
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
        for (i = 0; i < 4; i++)
        {
            if (i < (int) s / 4)
            {
                printf("%08X ", p[i]);
            }
            else
            {
                printf("         ");
            }
        }
        printf("\n");
        s -= 16;
        p += 4;
    }
}

void diag_dump_buf_32bit(void *p, unsigned int s)
{
    diag_dump_buf_with_offset_32bit((unsigned int *) p, s, 0);
}

/*!
 * function: analog_reg_write_hw
 *
 *  \brief write analog register by I2C
 *  \param reg
 *  \param value
 *  \return int
 */
int analog_reg_write_hw(int reg, unsigned char value)
{
    int rc;
    analog_log("[analog register write]\n");

    i2c_start();

    rc = i2c_write(ANALOG_I2C_ADDR);
    if (rc)
        goto frc;
    rc = i2c_write(reg);        // addr
    if (rc)
        goto frc;
    rc = i2c_write(value);      // data

  frc:
    i2c_stop();
    if (rc)
        printf("analog write reg%d fail!!!\n", reg);
    return rc;
}

/* hwi2c slave mode to access analog register bank by word alignment */
int analog_reg_write(int reg, unsigned int value)
{
    unsigned int val = value;
    int ret;
    int i;

    ret = analog_reg_write_hw(HW_I2C_R00_OFS, reg);
    if (ret)
        goto frc;
    for (i = HW_I2C_R01_DAT; i <= HW_I2C_R04_DAT; i++)
    {
        ret = analog_reg_write_hw(i, val);
        if (ret)
            goto frc;
        val >>= 8;
    }
    ret = analog_reg_write_hw(HW_I2C_R07_CMD, HW_I2C_CMD_WRITE);
    if (ret)
        goto frc;
    ret = analog_reg_write_hw(HW_I2C_R07_CMD, HW_I2C_CMD_NULL);
    if (ret >= 0)
    {
        if ((reg / 4) < ANALOG_REGS_BUFFER_SIZE)
        {
            analog_regs[reg / 4] = value;
        }
    }
  frc:
    return ret;
}

/*!
 * function: analog_reg_read_hw
 *
 *  \brief read analog register by I2C
 *  \param reg
 *  \return
 */
int analog_reg_read_hw(int reg)
{
    unsigned char data;
    int rc;
    analog_log("[analog register read]\n");

    i2c_start();

    rc = i2c_write(ANALOG_I2C_ADDR);
    if (rc)
        goto frc;
    rc = i2c_write(reg);        // addr
    if (rc)
        goto frc;
    i2c_restart();
    rc = i2c_write(ANALOG_I2C_ADDR | 0x01);
    if (rc)
        goto frc;
    data = i2c_read(1);         // data(8 bits)

  frc:
    i2c_stop();
    if (rc)
    {
        printf("analog read reg%d fail!!!\n", reg);
        return rc;
    }
    else
        return data;
}

/* hwi2c slave mode to access analog register bank by word alignment */
unsigned int analog_reg_read(int reg)
{
    unsigned int val = 0;
    int ret;
    int i;

    ret = analog_reg_write_hw(HW_I2C_R00_OFS, reg);
    if (ret)
        goto frc;
    ret = analog_reg_write_hw(HW_I2C_R07_CMD, HW_I2C_CMD_READ);
    if (ret)
        goto frc;
    for (i = HW_I2C_R04_DAT; i >= HW_I2C_R01_DAT; i--)
    {
        val <<= 8;
        val |= analog_reg_read_hw(i);
    }
    ret = analog_reg_write_hw(HW_I2C_R07_CMD, HW_I2C_CMD_NULL);
    if (ret)
        goto frc;
    if ((reg / 4) < ANALOG_REGS_BUFFER_SIZE)
    {
        analog_regs[reg / 4] = val;
    }
  frc:
    return val;
}

void analog_reg_read_all(void)
{
    int i;
    for (i = 0; i < ANALOG_REGS_BUFFER_SIZE; i++)
        analog_reg_read(i * 4);
}

void analog_auto_test(void)
{
    static unsigned int data = 0xffffffff;
    unsigned int value;
#define ANALOG_AUTO_TEST_NUM 2
    int t = ANALOG_AUTO_TEST_NUM;
    int ret[ANALOG_AUTO_TEST_NUM];
    int i;

    while (t > 0)
    {
        ret[t - 1] = 1;
        if (t == 2)
            printf("auto read:\n");
        else
        {
            printf("auto write:\n");
            for (i = 0; i < ANALOG_REGS_BUFFER_SIZE; i++)
                analog_reg_write(i * 4, data);
            data -= 0x01010101;
        }

        analog_reg_read_all();
        for (i = 0; i < ANALOG_REGS_BUFFER_SIZE; i++)
        {
            value = ANAREG(i);
#if 1
// it will be removed in next bitfile, because the busy bit is useless
            /* remove analog_regs_busy bit */
            if (i == 0)
                value &= (~(1 << 31));
#endif
            if (analog_regs[i] != value)
            {
                printf("reg%d diff 0x%08x:0x%8x\n", i * 4, analog_regs[i],
                       ANAREG(i));
                ret[t - 1] = 0;
            }
        }
        t--;
    }

    // show memory dump by i2c and cpu
    dump_hex("analog Regs:", analog_regs, ANALOG_REGS_BUFFER_SIZE * 4);
    mem_dump_cmd(2, (char *[])
                 {
                 "dw", "af005800", "0x90"} +1);

    printf("\n");
    t = ANALOG_AUTO_TEST_NUM;
    while (t > 0)
    {
        if (t == 2)
            printf("auto read test:");
        else
            printf("auto write test:");
        printf("%s\n", ret[t - 1] ? "PASS" : "FAIL");
        t--;
    }
}

/*!
 * function: analog_init
 *
 *  \brief initialize I2C for accessing analog
 *  \return
 */
void analog_init(void)
{
    analog_log("[init I2C module for access analog register]\n");

    printf("FPGA HW I2C Slave Mode Verification:\n");
    printf("\tPINMUX Only I2C_AUX vs SWI2C_AUX\n");
    printf("\tSDR Mode\n");

    /* enable I2C function */
    if (!(GPREG(PINMUX) & EN_SIP_FNC))
    {
        // SDR mode
        GPREG(PINMUX) |= EN_I2C_AUX_FNC;
    }
    else
    {
        // DDR mode, don't support
        printf("don't verify HW I2C in DDR mode\n");
    }

    i2c_init();
}

/*!
 * function: analog_finish
 *
 *  \brief finish I2C setting
 *  \return
 */
void analog_finish(void)
{
    analog_log("[finish I2C module]\n");
    i2c_finish();
}

/*!
 * function: analog
 *
 *  \brief command to access analog register by I2C
 *  \param argc
 *  \param argv
 *  \return
 */
int analog(int argc, char *argv[])
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

    if (!strcmp(cmd, "w"))
    {
        if (no & 3)
        {
            printf("reg is word alignment\n");
            goto err;
        }
        if (analog_reg_write(no, data))
            failed |= (1 << 0);
        printf("write reg%d = 0x%08x\n", no, data);
    }
    else if (!strcmp(cmd, "r"))
    {
        no = (no & ~(3));
        data = analog_reg_read(no);
        printf("read reg%d = 0x%08x\n", no, data);
    }
    else if (!strcmp(cmd, "all"))
    {
        analog_reg_read_all();
        dump_hex("analog Regs:", analog_regs, ANALOG_REGS_BUFFER_SIZE * 4);
    }
    else if (!strcmp(cmd, "i"))
        analog_init();
    else if (!strcmp(cmd, "f"))
        analog_finish();
    else if (!strcmp(cmd, "at"))
        analog_auto_test();
    else
        goto err;

    //printf("failed: %ld \n",failed);
    //printf("Done: %s \n", (0==failed)? "PASSED" : "FAILED");
    return (0 == failed) ? ERR_OK : ERR_PARM;

  help:
  err:
    return ERR_HELP;
}

cmdt cmdt_analog[] __attribute__ ((section("cmdt"))) =
{
    {
    "ana", analog,
            "analog i/f ; init/finish control interface\n"
            "analog r/w <reg> <data> ; read/write register\n"
            "analog at ; auto test r/w registers\n"}
,};
