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

#define TLV320AIC3111_ADDR 0x18
#define tlv320_log(fmt, args...) //printf(fmt, ##args)

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
/*!
 * function: tlv320_reg_write
 *
 *  \brief write tlv320 register by I2C
 *  \param reg
 *  \param data
 *  \return int
 */
int tlv320_reg_write(unsigned char reg, unsigned short data)
{
    int rc;
    tlv320_log("[tlv320 register write]\n");

    i2c_start(TLV320AIC3111_ADDR, 0);

    rc = i2c_write((reg & 0xff), 0);
    if (rc)
        goto frc;
    
    rc = i2c_write((data & 0xff), 1);

frc:
    if (rc)
        printf("tlv320 write reg%d fail!!!\n", reg);

    return rc;
}

/*!
 * function: mp320_reg_read
 *
 *  \brief read mp320 register by I2C
 *  \return
 */
void tlv320_reg_read(unsigned char reg, unsigned int *ptr)
{
    int rc;
    tlv320_log("[tlv320 register read]\n");
    i2c_start(TLV320AIC3111_ADDR, 0);

    rc = i2c_write(reg, 0);
    if (rc)
        goto frc;

    i2c_start(TLV320AIC3111_ADDR, 1);
    i2c_read(ptr, (reg & 0xff), 1);

frc:
    if (rc)
        printf("tlv320 read reg%d fail!!!\n", reg);

    return;
}

/*!
 * function: mp320_init
 *
 *  \brief initialize I2C for accessing mp320
 *  \return
 */
void tlv320_init(void)
{
    tlv320_log("[init I2C module for access mp320 register]\n");
    i2c_init();
}

/*!
 * function: mp320_finish
 *
 *  \brief finish I2C setting
 *  \return
 */
void tlv320_finish(void)
{
    tlv320_log("[finish I2C module]\n");
    i2c_finish();
}

/*!
 * function: tlv
 *
 *  \brief command to access tlv register by I2C
 *  \param argc
 *  \param argv
 *  \return
 */
int tlv_cmd(int argc, char *argv[])
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
    
    if (!strcmp(cmd, "i"))
    {
        tlv320_init();
    }
    else if (!strcmp(cmd, "w"))
    {
        if (tlv320_reg_write(no, data))
            failed |= (1 << 0);
        printf("write reg%d = 0x%03x\n", no, data);
    }
    else if (!strcmp(cmd, "r"))
    {
        tlv320_reg_read(no, &data);
        printf("read reg%d = 0x%02x\n", no, data);
    }
    
    return ERR_OK;
  err:
    return ERR_HELP;
}

cmdt cmdt_tlv320[] __attribute__ ((section("cmdt"))) =
{
    {
    "tlv", tlv_cmd,
            "tlv i/f ; init/finish control interface\n"
            "tlv r/w <reg> <data> ; read/write register\n"}
,};
