/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cmd2.c
*   \brief Extended Command
*   \author Montage
*/

#include <arch/chip.h>
#include <common.h>
#include <lib.h>
#include <usb.h>
#include "sflash/include/flash_api.h"

#define CHECK_BAD_BLOCK    1

#ifdef CONFIG_CMD_BB
/*!
 * function:
 *
 *  \brief
 *  \param
 *  \return
 */

#define BB_SPI_BASE (MI_BASE+0x38c0)
#define BB_SPI_DONE 0x80000000UL
#define BBREG(reg) ((volatile unsigned long*)(BB_SPI_BASE))[reg]

unsigned char bb_register_read(int bb_reg)
{
    unsigned char value;
    unsigned int data;

    data = ((0x01UL << 17)      /* start SPI */
            | ((bb_reg & 0xff) << 8)    /* register address */
        );

    BBREG(0) = data;

    while ((BBREG(0) & BB_SPI_DONE) != BB_SPI_DONE) ;
    data = BBREG(0);

    value = (data & 0xff);

    return value;
}

unsigned int bb_register_write(int bb_reg, unsigned char value)
{
    unsigned int data;

    data = ((0x01UL << 17)      /* start SPI */
            | (0x01UL << 16)    /* SPI write */
            | ((bb_reg & 0xff) << 8)    /* register address */
            | value);           /* data */

    BBREG(0) = data;

    while ((BBREG(0) & BB_SPI_DONE) != BB_SPI_DONE) ;

    data = BBREG(0);

    value = (data & 0xff);

    return value;
}

int bb_cmd(int argc, char *argv[])
{
    int set = 0;
    int reg;
    int val;

    if (argc > 0)
    {
        if (1 != sscanf(argv[0], "%x", &reg))
            goto help;
    }
    else
    {
        printf("BB reg ");
        for (reg = 0; reg < 255; reg++)
        {
            val = bb_register_read(reg) & 0xff;
            if ((reg % 16) == 0)
                printf("\n%02X: ", reg);
            printf("%02x ", val);
        }
        return ERR_OK;
    }

    if (argc > 1)
    {
        if (1 != sscanf(argv[1], "%x", &val))
            goto help;
        bb_register_write(reg, val);
        set++;
    }
    else
    {
        val = bb_register_read(reg);
    }
    printf("%sBB%x = %02x\n", set ? "SET " : "", reg, val);
    return ERR_OK;
  help:
    printf("BB [reg] [val]\n\r");
    return ERR_PARM;
}

cmdt cmdt_bb __attribute__ ((section("cmdt"))) =
{
"bb", bb_cmd, "bb <reg> <val> ;Access BB register"};
#endif

#ifdef CONFIG_CMD_EXMII
int exmii_cmd(int argc, char *argv[])
{
    int set = 0;
    int reg;
    int val;
    GPREG(0xb0) = 0x6f;         /*
                                   [3]enbale external MII;
                                   [2]enable external MII Speed 100;
                                   [1]full external MII duplex;
                                   [0]pause */

    cm_mdio_wr(20, 5, 0x16);    /* default */
    cm_mdio_wr(20, 6, 0x1f1f);  /* mac learning */
    /* vlan setting ; port based;  port 0 ~ 4 group */
    cm_mdio_wr(23, 0, 0x1f1f);
    cm_mdio_wr(23, 1, 0x1f1f);
    cm_mdio_wr(23, 2, 0x1f);
    cm_mdio_wr(23, 3, 0x0);

    /* !!! After enable external MII, want to reset switch */
#if 1
    /* switch reset */
    printk("Reset switch \n");
    GPREG(SWRST) &= ~(SWRST_SW);        //reset switch
    GPREG(SWRST) |= (SWRST_SW);
    mdelay(50);                 //wait switch ready
    printf("Wait SW ready..\n");
    while (cm_mdio_rd(20, 0) == 0) ;
#endif

#if 1
    /* ADC edge */
    int i, j;
    for (i = 0; i < 5; i++)
    {
        cm_mdio_wr(i, 20, i);   //ADC edge
        cm_mdio_wr(i, 23, 0x5e);
        cm_mdio_wr(i, 30, 8);   //ADC gain
        cm_mdio_wr(i, 31, 0xe700);
        for (j = 150; j > 0; j--) ;
        cm_mdio_wr(i, 0, 0x8000);       //reset PHY instead of restart AN
    }
#endif

    return ERR_OK;
  help:
    return ERR_PARM;
}

cmdt cmdt_exmii __attribute__ ((section("cmdt"))) =
{
"ex", exmii_cmd, "exmii ;change to External MII; For throughput test"};
#endif

#ifdef CONFIG_USB
extern int dev_index;
extern int usb_stor_curr_dev;   /* current device */
int usb_stor_auto_cmd(int argc, char *argv[])
{
    char *str_ptr[4];
    char str[4][10];
    int ts;
    if (argc > 1)
        goto help;

    printf("===Step 1===\n");
    ts = clock_get();
    do
    {
        usb_stop();
        if (!usb_init())
            usb_stor_curr_dev = usb_stor_scan(1);
    }
    while (dev_index < 2 && (how_long(ts) < 5 * CONFIG_SYS_HZ));

    if (dev_index < 2)
    {
        printf("Error!!! can't found USB device\n");
        goto help;
    }

    printf("===Step 2===\n");
    ts = clock_get();
    while ((usb_stor_curr_dev < 0) && (how_long(ts) < 15 * CONFIG_SYS_HZ))
    {
        usb_stop();
        if (!usb_init())
            usb_stor_curr_dev = usb_stor_scan(1);
    }

    if (usb_stor_curr_dev < 0)
    {
        printf("Error!!! can't found USB Storage device\n");
        goto help;
    }
    printf("===Step 3=== copy 64kb from flash to memory:0x80500000\n");
    /* replace flash_read_bytes with flash_control_read_bytes @ 20180124 by Edden.
       because flash_read_bytes only used for sflash_controller.c */
    flash_control_read_bytes(0, 0x80500000, 0x10000, CHECK_BAD_BLOCK);

    printf("===Step 4=== write 64kb into USB Storage\n");
    strcpy(str[0], "write");
    strcpy(str[1], "80500000");
    strcpy(str[2], "0");
    strcpy(str[3], "80");//1<<7
    str_ptr[0] = &str[0][0];
    str_ptr[1] = &str[1][0];
    str_ptr[2] = &str[2][0];
    str_ptr[3] = &str[3][0];
    if (do_usb(4, str_ptr))
    {
        printf("Error!!!, when write 64kb into USB Storage\n");
    }

    printf("===Step 5=== read 64kb from USB Storage to memory:80700000\n");
    strcpy(str[0], "read");
    strcpy(str[1], "80700000");
    strcpy(str[2], "0");
    strcpy(str[3], "80");//1<<7
    str_ptr[0] = &str[0][0];
    str_ptr[1] = &str[1][0];
    str_ptr[2] = &str[2][0];
    str_ptr[3] = &str[3][0];
    if (do_usb(4, str_ptr))
    {
        printf("Error!!!, read 64kb from USB Storage\n");
    }

    printf("===Step 6=== compare 64kb\n");
    strcpy(str[0], "80500000");
    strcpy(str[1], "80700000");
    strcpy(str[2], "10000");
    str_ptr[0] = &str[0][0];
    str_ptr[1] = &str[1][0];
    str_ptr[2] = &str[2][0];
    if (cmd_cmp(3, str_ptr))
        printf("Error!!!, 64kb isn't equal\n");
    else
        printf("Congiration, USB Auto Test pass\n");

    usb_stop();

    return 0;
  help:
    printf("uat ;USB Auto Test\n\r");
    return -1;
}

cmdt cmdt_usb_auto __attribute__ ((section("cmdt"))) =
{
"uat", usb_stor_auto_cmd, "uat ;USB Auto Test"};
#endif

#ifdef CONFIG_CMD_CS3_SELECT
int cs3_cmd(int argc, char *argv[])
{
    int val;

    if (argc != 1)
        goto err_arg;

    if (1 != sscanf(argv[0], "%d", &val))
        goto err_arg;

    GPREG(CS3SEL) &= ~(0x3f);   // Select CS3 to GPIO#val
    GPREG(CS3SEL) |= val;
    GPREG(CS3SEL) |= 1 << 7;    // Enable CS3 to GPIO function

    printf("0x5084 = 0x%x\n", GPREG(CS3SEL));

    return ERR_OK;
  err_arg:
    printf("gpio no : 0 ~ 38\n");
    return ERR_PARM;
}

cmdt cmdt_cs3 __attribute__ ((section("cmdt"))) =
{
"cs3", cs3_cmd, "cs3 <dec> ;CS3 select a gpio pin"};
#endif

#ifdef CONFIG_CMD_USB_MODE
int umode_cmd(int argc, char *argv[])
{
    int val;

    if (argc == 0)
    {
        switch (GPREG(USBMOD))
        {
            case 0:
                printf("USB host mode\n");
                break;
            case 1:
                printf("USB device mode\n");
                break;
            case 2:
            case 3:
                printf("USB OTG mode\n");
                break;
            default:
                printf("USB unknown mode\n");
        }
    }
    else if (argc == 1)
    {
        if (1 != sscanf(argv[0], "%d", &val) || (val > 3))
            goto help;

        GPREG(USBMOD) = val;
        /* check */
        if (val != GPREG(USBMOD))
            printf("SOC not yet support it.\n");
    }
    else
        goto help;

    return ERR_OK;
  help:
    return ERR_HELP;
}

cmdt cmdt_umode __attribute__ ((section("cmdt"))) =
{
"umode", umode_cmd, "umode <num> ; USB mode 0:host 1:device 2/3:OTG"};
#endif
