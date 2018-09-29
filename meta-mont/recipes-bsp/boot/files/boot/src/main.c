/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file main.c
*   \brief c main entry
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <ddr_config.h>
#include <common.h>
#include <netprot.h>
#include <led_api.h>
#include <cm_mac.h>
#include <config.h>
#include <netdev.h>
#include <lib.h>
#include <dhcpd.h>
#include <otp.h>
#include <image.h>
#include <arch/trap.h>
#include <arch/irq.h>
#include <sched.h>
#include <pinmux.h>
#include <pmu.h>
#include <gpio_driver_strength.h>
#include "sflash/include/flash_api.h"
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#ifdef CONFIG_KEYWORD_TO_STOP
#define KEYWORD CONFIG_KEYWORD_TO_STOP
#define KEYWORD_SZ (sizeof(CONFIG_KEYWORD_TO_STOP)/sizeof(char)-1)      //exclude string end
#else
#define KEYWORD "any key"
#endif

/*=============================================================================+
| Define for Function Switch                                                   |
+=============================================================================*/
#define INIT_FLASH

#if defined(CONFIG_FPGA)
#define UART_CLK   (40 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE  115200
#else
#define UART_CLK   (120 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE  115200
#endif

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#ifdef CONFIG_LED
char led_gpio_pin[] = {
    -1,
};
#endif
#ifdef CONFIG_KEYWORD_TO_STOP
char keyword_str[KEYWORD_SZ] = { CONFIG_KEYWORD_TO_STOP };
#endif

extern unsigned char netbuf[CONFIG_NETBUF_SZ * CONFIG_NETBUF_NUM];

char boot3_submode;
#ifdef CONFIG_GDMA
extern void gdma_init(void);
#endif
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void uart_controller_rx_init(void)
{
    int i;

    URREG(URCS) = (URCS_REN | ((UART_CLK/UART_TARGET_BAUD_RATE)<<URCS_BRSHFT));

    /* cleanup RX FIFO, if any */
    for (i = 0; i < 32; i++)
    {
        URREG(URBR);
    }
}

/*!
 * function: check_stop_condition
 *
 *  \brief check whether want to stop
 *  \return 0:don't stop 1:stop
 */
int check_stop_condition(void)
{
    static int i = 0;
    if (tstc())
    {
        if(boot3_submode == MPTOOL_SUBMODE)
            return 0;
#ifdef CONFIG_KEYWORD_TO_STOP
        if (getchar() == keyword_str[i])
            i++;
        if (i >= KEYWORD_SZ)
            return 1;
#else
        i = getchar();
        //printf("key=%d\n",i);
        if (i)
            return 1;
#endif
    }
    printf(".");
    return 0;
}

#define IPL_BOOT_FAILURE_CHECK_MAGIC   0x45A9C637UL
#define IPL_BOOT_FAILURE_CHECK_MAGIC2  0xFF0000FFUL
#define IPL_BOOT_FAILURE_CHECK_ADDR    0xB0007FF8UL

#define BOOT2_BOOT_FAILURE_CHECK_MAGIC   0x45A84637UL
#define BOOT2_BOOT_FAILURE_CHECK_MAGIC2  0xFF8001FFUL
#define BOOT2_BOOT_FAILURE_CHECK_MAGIC2_STOP_IN_BOOT2 0xFF4002FFUL
#define BOOT2_BOOT_FAILURE_CHECK_MAGIC2_BOOT_RECOVERY 0xFF2004FFUL
#define BOOT2_BOOT_FAILURE_CHECK_ADDR    0xB0007FF0UL

static int last_boot_failure = 0;
static int boot_stop_at_boot2 = 0;
static int boot_to_recovery = 0;

#define B2_TEXT 0x83800000
#define B3_TEXT 0x83000000

#define WATCHDOG_TIMER_1MS 0x20 // the Unit of watchdog timer is 32kHz
#define ENABLE_WATCHDOG    0x1000000

void turn_off_watchdog(void)
{
    PMUREG_WRITE32(0x20, 0x0);
}

void turn_on_watchdog(void)
{
    unsigned int time = 0;
    if(bootvars.watchdog_timer != 0)
    {
        time = bootvars.watchdog_timer * WATCHDOG_TIMER_1MS & 0x00FFFFFF;    // bit[0:23]
        PMUREG_UPDATE32(0x20, 0x01FFFFFF, (time | ENABLE_WATCHDOG));         // bit[24] set 1 to enable
    }
    else
    {
        turn_off_watchdog();
    }
}

int is_boot2(void)
{
    int ptr_for_check = (int)(&boot_to_recovery);
    if(ptr_for_check >= B2_TEXT)
    {
        return 1;
    }
    return 0;
}

void start_boot_failure_detection(void)
{
    unsigned long *boot2_checkval = (unsigned long *) BOOT2_BOOT_FAILURE_CHECK_ADDR;

    if (bootvars.mode > 0 && bootvars.recovery != 0 && is_boot2())
    {
        boot2_checkval[0] = BOOT2_BOOT_FAILURE_CHECK_MAGIC;
        boot2_checkval[1] = BOOT2_BOOT_FAILURE_CHECK_MAGIC2;

        turn_on_watchdog();
    }
}

void boot_control_and_failure_detection(void)
{
    unsigned long *ipl_checkval = (unsigned long *) IPL_BOOT_FAILURE_CHECK_ADDR;
    unsigned long *boot2_checkval = (unsigned long *) BOOT2_BOOT_FAILURE_CHECK_ADDR;

    /* clear IPL boot failure check value, IPL->BOOT2 considered to be success */
    if((ipl_checkval[0] == IPL_BOOT_FAILURE_CHECK_MAGIC)
        && (ipl_checkval[1] == IPL_BOOT_FAILURE_CHECK_MAGIC2))
        ipl_checkval[1] = 0;

    if(boot2_checkval[0] == BOOT2_BOOT_FAILURE_CHECK_MAGIC)
    {
        if(boot2_checkval[1] == BOOT2_BOOT_FAILURE_CHECK_MAGIC2)
            last_boot_failure = 1;
        else if(boot2_checkval[1] == BOOT2_BOOT_FAILURE_CHECK_MAGIC2_STOP_IN_BOOT2)
            boot_stop_at_boot2 = 1;
        else if(boot2_checkval[1] == BOOT2_BOOT_FAILURE_CHECK_MAGIC2_BOOT_RECOVERY)
            boot_to_recovery = 1;

        boot2_checkval[1] = 0;
    }

    turn_off_watchdog();

    if(!is_boot2())
    {
        boot_to_recovery = 0;
        last_boot_failure = 0;
    }
}

void print_boot_banner(void)
{
    int little_endian = 0;
#if !defined(CONFIG_FPGA)
#if defined(CONFIG_USE_DDR3)
#if defined(CONFIG_FREQ666)
    char dram_type[] = "DDR3-667";
#else
    char dram_type[] = "DDR3-533";
#endif
#else
#if defined(CONFIG_FREQ396)
    char dram_type[] = "DDR2-400";
#else
    char dram_type[] = "DDR2-533";
#endif
#endif
    unsigned int ddr_size = *(volatile unsigned long *)DDR_SIZE_INFO_ADDR;
#endif

#define PIN_STRAP_REG_ADDR  0xbf004828UL
    if (0 == (*((volatile unsigned long *)PIN_STRAP_REG_ADDR) & 0x01))
        little_endian = 1;

    //dbg_log(LOG_VERBOSE, "\nPanther booting @ %x:%x\n", B2_TEXT, CONFIG_BOOT_UNC_LOAD);
#if defined(CONFIG_FPGA)
    dbg_log(LOG_INFO, "\nPanther(%s) FPGA", little_endian?"LE":"BE");
#else
    dbg_log(LOG_INFO, "\nPanther(%s) %s-%dM", little_endian?"LE":"BE", dram_type, ddr_size);
#endif

    if (otp_parse_config(OTP_ENABLE_SECURE_SHIFT))
        dbg_log(LOG_INFO, " S");

    switch(0xf & *((volatile unsigned long *)0xbf005524))
    {
        case BOOT_FROM_NAND:
            dbg_log(LOG_INFO, " NAND");
            break;
        case BOOT_FROM_NAND_WITH_OTP:
            dbg_log(LOG_INFO, " NAND_OTP");
            break;
        case BOOT_FROM_NOR:
            dbg_log(LOG_INFO, " NOR");
            break;
        case BOOT_FROM_SD:
            dbg_log(LOG_INFO, " SD");
            break;
        case BOOT_FROM_UART:
            dbg_log(LOG_INFO, " UART");
            break;
        default:
            break;
    }

#if defined(NOR_4ADDR_MODE)
    dbg_log(LOG_INFO, " 4ADDR");
#endif

    if(last_boot_failure)
        dbg_log(LOG_INFO, " R");

    dbg_log(LOG_INFO, " %d %02x:%x %08x\n"
            , chip_revision
            ,(0xff & *((volatile unsigned long *)0xbf004810))
            ,*((volatile unsigned long *)0xbf0048FC)
            ,*((volatile unsigned long *)0xbf005520));
}

static int rx_stop_char_count = 0;
int stop_boot_process(void)
{
    while (serial_poll())
    {
        if(serial_getc()=='X')
            rx_stop_char_count++;
        else
            rx_stop_char_count = 0;
    }

    if(rx_stop_char_count>=3)
        return 1;
    else
        return 0;
}

#if defined(CONFIG_ATE)

int dbg_log_level = LOG_DBG;
void cmain()
{
    chip_revision_detection();
    //boot_control_and_failure_detection();

    uart_controller_rx_init();
    serial_init();

    //otp_read_config();

    print_boot_banner();

    //board_init();

    trap_init();
    irq_init();

    clock_init();

    irq_enable();

    //if(0>flash_init(otp_get_boot_type()))
    //    printf("flash init failed.\n");

    /* load otp data before access */
    //otp_load();

    config_init(NULL);

    //clkcfg_apply();
    //pinmux_apply();
    //gpio_setting_apply();
    //gpio_driver_strength_apply();

    dbg_log_level = LOG_VERBOSE;

    nbuf_init((void *) (((unsigned int) netbuf) | 0xa0000000), CONFIG_NETBUF_SZ,
              CONFIG_NETBUF_NUM);
    cmd_init();

    //board_init2();

    //cdb_save(savecdb);

#ifdef CONFIG_DHCPD
    dhcpd_init();
#endif

    bootvars.network = 0;
    eth_probe();

#ifdef CONFIG_AI
    //ai_init();
#endif

#ifdef CONFIG_USB_LOOPBACK_DEV
    usb_lb_dev_init();
#endif

#ifdef CONFIG_SDHC
    sdhc_init();
#endif

#ifdef CONFIG_GDMA
    gdma_init();
#endif

#ifdef CONFIG_MINI_SDHC
    // always re-init the SDHCI on each mmc_cmd
    //mini_sdhc_init();
#endif

#ifdef CONFIG_TSI
    tsi_init();
#endif

#ifdef CONFIG_VBUS_DETECTION
    vbus_detection_init();
#endif

#ifdef CONFIG_WIFI
    dratini_start();
#endif

#if defined(CONFIG_SCHED)
    /* start the scheduler */
    sched_init();
#endif

    while (1)
    {
        cmd_loop();
    }
    //eth_reset();
    while (1) ;
}

#else

/*!
 * function:
 *
 *  \brief
 *  \return
 */
int dbg_log_level = LOG_DBG;
void cmain()
{
    int savecdb = 0;
    int err_val = 0; // err value from cmd_go()

    chip_revision_detection();
    boot_control_and_failure_detection();

#if defined(TEST_MODE_BOARD)
    *(volatile unsigned long *)0xbf004a24 = 0x00011110UL;  //gpio 2526 2728
    *(volatile unsigned long *)0xbf004a30 = 0x45540000UL;
#endif

    uart_controller_rx_init();
    serial_init();

    otp_read_config();

    print_boot_banner();

    //board_init();

#ifndef CONFIG_FPGA
    //cheetah_phy_down(1, 0);
#endif
#ifdef CONFIG_UART_EN_DET
    gpio_uart_detect_init();
#endif

    trap_init();
    irq_init();

    clock_init();

    irq_enable();

#ifdef INIT_FLASH
    if(0>flash_init(otp_get_boot_type()))
    {
        printf("flash init failed.\n");
    }
    else
    {
#if defined(NOR_4ADDR_MODE)
        if(otp_get_boot_type()==BOOT_FROM_NOR)
            otp_program_nor_4addr_mode();
#endif
    }
#endif

    /* load otp data before access */
    otp_load();

    config_init(&savecdb);

    clkcfg_apply();
    pinmux_apply();
    gpio_setting_apply();
    gpio_driver_strength_apply();

    dbg_log_level = LOG_VERBOSE;

    nbuf_init((void *) (((unsigned int) netbuf) | 0xa0000000), CONFIG_NETBUF_SZ,
              CONFIG_NETBUF_NUM);
    cmd_init();

    //board_init2();

#ifdef CONFIG_I2CS
    goto DO_I2CS;
#endif

#ifdef CONFIG_LED
    led_init(led_gpio_pin);
#endif
    cdb_save(savecdb);

    if (bootvars.recovery == 0)
    {
        boot_to_recovery = 0;
        last_boot_failure = 0;
    }

    if(is_boot2())
    {
        if(uart_loopback_check())
        {
            char *argv[] = { "mptool" };
            boot3_submode = MPTOOL_SUBMODE;
            cmd_go(1, &argv[0]);
        }
    }
    else
    {
        printf("-- @ boot3 --\n");
        bootvars.network = 1;

        if (!memcmp((void *)BOOT_CMDLINE_DRAM_ADDR, "mptool", 6))
        {
            printf("-- manual to mptool --\n");
            memset((void *)BOOT_CMDLINE_DRAM_ADDR, 0, COMMAND_LINE_SIZE);
            boot3_submode = MPTOOL_SUBMODE;
        }
        else if (!memcmp((void *)BOOT_CMDLINE_DRAM_ADDR, "recovery", 8))
        {
            printf("-- to recovery --\n");
            memset((void *)BOOT_CMDLINE_DRAM_ADDR, 0, COMMAND_LINE_SIZE);
            boot3_submode = RECOVER_SUBMODE;
        }
        goto skip_boot2;
    }

    //printf("boot_to_recovery %d, last_boot_failure %d, boot_stop_at_boot2 %d\n", boot_to_recovery, last_boot_failure, boot_stop_at_boot2);
    char *m = (char *) boot_mode_str(bootvars.mode);
    if (boot_to_recovery != 0 || ((last_boot_failure != 0) && (bootvars.mode > 0)))
    {
        if(bootvars.recovery == RECOVERY_ONE)
        {
            char *argv[] = { "recovery" };
            printf("-- go to recovery --\n");
            cmd_go(1, &argv[0]);
        }
        if(bootvars.recovery == RECOVERY_TWO)
        {
            char *argv[] = { "second" };
            printf("-- go to second image offset --\n");
            cmd_go(1, &argv[0]);
        }
    }
    else if (!boot_stop_at_boot2 && (bootvars.mode > 0) && !gpio_swrst_check())
    {
#if 0
#ifdef CONFIG_MP_TEST
        if (check_mptest())
        {
            char *argv[] = { "mptest" };
            cmd_go(1, &argv[0]);
        }
#endif
#endif
        dbg_log_level = LOG_DBG;
        dbg_log(LOG_DBG, "\nLoading %s.\n",m);
        if(bootvars.quiet)
            dbg_log_level = LOG_DBG;

        if(!stop_boot_process())
            err_val = cmd_go(1, &m);

        rx_stop_char_count = 0;
        dbg_log_level = LOG_VERBOSE;
    }
skip_boot2:
#ifdef CONFIG_LED
    if (gpio_swrst_check())
        led_set(LED_STATUS, LED_BLINK, 1000, 2000);
#endif

#ifdef CONFIG_DHCPD
    dhcpd_init();
#endif

#ifdef CONFIG_I2CS
  DO_I2CS:
    /* skip eth init, but we need to access bb */
    GPREG(SWRST) &= ~(PAUSE_SW | PAUSE_HNAT | PAUSE_WIFI);      //Enalbe gating clock of switch, hnat and wifi module
    cheetah_phy(PORT_ETH1_IDX, 0);      //phy up
    MACREG_UPDATE32(HW_RESET, 0, HW_RESET_WIFI_PAUSE | HW_RESET_BB_PAUSE);      //Enable gating clock of wifi and bb
#else
    eth_probe();
#ifndef CONFIG_FPGA
    //cheetah_phy_up(1, 0);
#endif
#endif

#ifdef CONFIG_LED
    led_hw_init();
#endif
#ifdef CONFIG_AI
    //ai_init();
#endif
#ifdef CONFIG_USB_LOOPBACK_DEV
    usb_lb_dev_init();
#endif
#ifdef CONFIG_SDHC
    sdhc_init();
#endif
#ifdef CONFIG_MINI_SDHC
    // always re-init the SDHCI on each mmc_cmd
    //mini_sdhc_init();
#endif
#ifdef CONFIG_SD_RECOVERY
    if (err_val == -1)
    {
        printf("image failed, recover from SD card..\n");
        sdrc();
    }
#else
    UNUSED(err_val);
#endif
#ifdef CONFIG_I2CS
    /*
     * I2CS support to access
     * Analog, EPHY, USB PHY Registers
     */
    i2cs_cmd_go(100);
#endif
#ifdef CONFIG_GDMA
    gdma_init();
#endif
#ifdef CONFIG_MPEGTS_ASYNCFIFO
    asyncfifo_init();
#endif
#ifdef CONFIG_TSI
    tsi_init();
#endif
#ifdef CONFIG_VBUS_DETECTION
    vbus_detection_init();
#endif

#ifdef CONFIG_WIFI
    dratini_start();
#endif

#if defined(CONFIG_SCHED)
    /* start the scheduler */
    sched_init();
#endif

    while (1)
    {
        cmd_loop();
    }
    //eth_reset();
    while (1) ;
}

#endif

