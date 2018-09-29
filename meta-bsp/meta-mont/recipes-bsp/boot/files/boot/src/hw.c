/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file hw.c
*   \brief Chip Initialization
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/irq.h>
#include <arch/chip.h>
#include <common.h>
#include <lib.h>
#include <netprot.h>
#include <cm_mac.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define MHZ 1000000
#define MAX_FREQ 0x1E000        //122880
#define CPU_CLK_MODE_NUM 13
#define SYS_CLK_MODE_NUM 6
#define HVER_SYSCLK_BITMAP 0xf
#define HVER_CPUCLK_BITMAP 0xf
#define HVER_IODRV_BITMAP 0x3
#define HVER_SYSCLK_SHIFT 16
#define HVER_CPUCLK_SHIFT 20
#define HVER_IODRV_SHIFT 28
#define HVER_WTDOG_SHIFT 30
#define HVER_CLK_CUSTOM_MODE 0xff
#define MIN_HIGH_PERIOD 2
#define MAX_USE_FREQ 320000000
#define MIN_USE_FREQ 60000000
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#if defined(CONFIG_TODO)
/* CPU: 60.95, 64, 80, 120, 160, 202.11, 240, 295.38, 320, 384, 480, 512, 640 MHz */
static unsigned int cpu_div[CPU_CLK_MODE_NUM] = {
    0x7fc, 0x7f0, 0x7c0, 0x780, 0x680, 0x398, 0x380, 0x1d0, 0x280, 0x1a0, 0x180,
    0x0f0, 0x0c0
};

/* SYS: 60.95, 64, 80, 120, 150.59, 160 MHz */
static unsigned int bus_div[SYS_CLK_MODE_NUM] =
    { 0x7fc, 0x7f0, 0x7c0, 0x780, 0x688, 0x3c0 };
static unsigned char postdiv[CLKDIV_POSTDIV_NUM] = { 1, 2, 3, 4, 2, 4, 6, 8 };
#endif

#ifdef CONFIG_GPIO_SWRST
// gpio
static unsigned int bit;
#endif
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void cheetah_pad_mode(int mode);
void board_init2();
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern int sscanf(char *buf, char *fmt, ...);
extern unsigned short cm_mdio_rd(unsigned short pa, unsigned short ra);
extern void cheetah_timer_reinit(unsigned int tmrclk);
extern void cheetah_uart_reinit(unsigned int uartclk);
extern void cm_mdio_wr(unsigned short pa, unsigned short ra,
                       unsigned short val);
extern void cm_set_cpu_port();
extern void cm_smi_init(void);
extern int pinmux_pin_func_gpio(int pin_number);
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
/*!
 * function:
 *
 *  \brief
 *  \param mode
 *  \return
 */

int cheetah_cpuclk(int mode)
{
#ifdef CONFIG_TODO
    unsigned int sysclk, div, cpu, sys;

    sysclk = CONFIG_SYS_CLK;

    cpu =
        HVER_CPUCLK_BITMAP & (mode >> (HVER_CPUCLK_SHIFT - HVER_SYSCLK_SHIFT));
    sys = HVER_SYSCLK_BITMAP & (mode >> 0);
    if (mode == HVER_CLK_CUSTOM_MODE)   //use bootldr variable `PLL`
        div = bootvars.pll;
    else if ((cpu >= CPU_CLK_MODE_NUM) || (sys >= SYS_CLK_MODE_NUM))    //skip wrong mode setting
        return -1;
    else
        div = (cpu_div[cpu] << CLKDIV_CPUFFST) | bus_div[sys];

    sysclk =
        (MAX_FREQ / postdiv[(div & SYS_CLK_POSTDIV) >> CLKDIV_CPUSHFT] /
         (div & SYS_CLK_PREDIV)) * MHZ;
    //printf("mode=0x%x\n", mode);
    //printf("div=0x%08x\n", div);
    //printf("sysclk=0x%08x\n", sysclk);
    ANAREG(CLKDIV) = div;
    ANAREG(CLKDIV) |= (CPU_CLK_UPDATE | SYS_CLK_UPDATE);
    ANAREG(CLKDIV);             //read-back
    /*
     * 1. the minimum high period is 2 cycle for update signal
     * 2. increase the high period because lower bus frequency exist possibly
     */
    idelay(MIN_HIGH_PERIOD * (MAX_USE_FREQ / MIN_USE_FREQ + 1));
    ANAREG(CLKDIV) = div;

    // re-init uart baud
    cheetah_uart_reinit(sysclk);
#endif
    return 0;
}

/*!
 * function:
 *
 *  \brief if uart is loopback, close uart fnc
 *  \return
 */
#define LOOP_SET 2
inline int uart_loopback_check(void)
{
    unsigned int restore_pinmux[5]={0};
    int pin1, pin2;
    int loopback = 0;
    int idx;

    for(idx=0; idx<5; idx++)
    {
        restore_pinmux[idx] = GPREG(GPIO_SEL0+idx*4);
    }

    pin1 = 1 << 22; //uart1_rx
    pin2 = 1 << 21; //uart1_tx

    //PIN change to GPIO function
    if(ERR_OK != pinmux_pin_func_gpio(21))
        goto done;
    if(ERR_OK != pinmux_pin_func_gpio(22))
        goto done;

    //GPIO FUNC
    //NOTE: 0:output dir, 1:input dir
    PMUREG(GPSEL) = PMUREG(GPSEL) | pin1 | pin2;
    PMUREG(GPDIR) = (PMUREG(GPDIR) | pin1) & ~pin2;
    mdelay(1);
    for(idx=0; idx<LOOP_SET; idx++)
    {
        PMUREG(GPCLR) = pin2;
        mdelay(1);
        if (!(PMUREG(GPVAL) & pin1))
        {
            PMUREG(GPSET) = pin2;
            mdelay(1);
            if (PMUREG(GPVAL) & pin1)
                loopback = 1;
            else
            {
                loopback = 0;
                break;
            }
        }
        else
        {
            loopback = 0;
            break;
        }
    }

done:
    if (!loopback)
    {
        for(idx=0; idx<5; idx++)
            GPREG(GPIO_SEL0+idx*4) = restore_pinmux[idx];
    }

    //printf("loopback check %d\n", loopback);
    return loopback;
}

inline int gpio_swrst_check(void)
{
#ifdef  CONFIG_GPIO_SWRST
    return !(GPREG(GPVAL) & bit);
#else
    return 0;
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

inline void gpio_swrst_init()
{
#ifdef  CONFIG_GPIO_SWRST
    bit = (1 << CONFIG_GPIO_SWRST);
    //printf("GPIO SWRST bit=%04x\n", bit);
    GPREG(GPDIR) &= ~bit;       //0: input
    GPREG(GPSEL) |= bit;        //1: gpio mode
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
#ifdef  CONFIG_UART_EN_DET
int boot_detect_flag = 0;
void gpio_uart_detect_init()
{
    int boot_detect_bit;
    boot_detect_bit = (1 << GPIO_UART_DET_NUM);
    GPREG(GPDIR) &= ~boot_detect_bit;   //0: input
    GPREG(GPSEL) |= boot_detect_bit;    //1: gpio mode

    if (!(GPREG(GPVAL) & boot_detect_bit))      // vlaue 0 enable tx
    {
        boot_detect_flag = 1;
    }

}
#endif
/*!
 * function:
 *
 *  \brief
 *  \return
 */

void cdbrst_check(void)
{
#ifdef CONFIG_PUSHTIME_TO_RSTCDB
    char cdbrst = 0;
    char *parm[] = { "fc", "0" };
    char *m = (char *) boot_mode_str(bootvars.mode);
    unsigned int start_time = clock_get();

    while (gpio_swrst_check())
    {
        if (!cdbrst && how_long(start_time) > CONFIG_PUSHTIME_TO_RSTCDB)
        {
            flash_cmd(0, &parm[1]);
            cdbrst = 1;
        }
    }
    printf("press time=%dms\n", how_long(0));   //calculate from clock_init()
    if (cdbrst)
    {
        printf("Reset to default!\n");
        board_init2();
        cmd_go(1, &m);
    }
#endif
}

#ifdef CONFIG_FPGA
/*!
 * function:
 *
 *  \brief
 *  \param en
 *  \return
 */

void func_mode_enable(int en)
{
#ifdef CONFIG_TODO
    if (en)                     /* external BB */
        GPREG(GDEBUG) = FUNC_MODE_EN | EXT_BB;
    else                        /* internal BB */
        GPREG(GDEBUG) = 0;
#endif
}
#endif
#ifndef CONFIG_FPGA
/*!
 * function:
 *
 *  \brief
 *  \return
 */
void ana_init(void)
{
#ifdef CONFIG_TODO
    /* Enable RMII & WIFI clock */
    ANAREG(CLKEN) |= (RMII_CLK_EN | WIFI_CLK_EN);
    /* Config ephy address = 1 */
    ANAREG(RMIIADDR) &= ~0x1f;
    ANAREG(RMIIADDR) |= 0x1;
#endif
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void hw_moudle_init(void)
{
#ifdef CONFIG_TODO
    // enable USB module in default
    /* turn on usb gated clock */
    GPREG(SWRST) &= ~(PAUSE_USB);
    /* reset usb module */
    GPREG(SWRST) &= ~(SWRST_USB);
    idelay(100);
    GPREG(SWRST) |= (SWRST_USB);

    //Patch:USB PHY PLL setting
#if (CONFIG_REF_CLK==40000000)
    USBREG(USB_PHY_PLL) = 0x6c1080fe;
#elif (CONFIG_REF_CLK==25000000)
    USBREG(USB_PHY_PLL) = 0x5ca083fe;
#else
#error "don't supoort this frequency"
#endif

    //tune USB PHY
    USBREG(PHY_DIG_CTRL) |= RECOVERY_CLK_INV;
    USBREG(USB_REG0) |= SQ_DELAY;
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void board_init()
{
    hw_moudle_init();
    cm_smi_init();
#ifdef CONFIG_FPGA
    func_mode_enable(0);
#endif
#ifdef  CONFIG_GPIO_SWRST
    gpio_swrst_init();
    //if (gpio_swrst_check())
    //{
    // don't set cpu freq if in diag mode
    //    return;
    //}
#endif
#ifndef CONFIG_FPGA
    ana_init();
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void board_init2()
{
    unsigned int hver = bootvars.hver;
    unsigned int sys, cpu;

    if (!gpio_swrst_check())    //normal
    {
        cpu = HVER_CPUCLK_BITMAP & (hver >> HVER_CPUCLK_SHIFT);
        sys = HVER_SYSCLK_BITMAP & (hver >> HVER_SYSCLK_SHIFT);
        cheetah_cpuclk((cpu << (HVER_CPUCLK_SHIFT - HVER_SYSCLK_SHIFT)) + sys); // set clock
        cheetah_pad_mode(HVER_IODRV_BITMAP & (hver >> HVER_IODRV_SHIFT));       // set pad driving current
    }
    else                        // push button is press
    {
        cpu = ((CONFIG_BOOT_HWVER) >> HVER_CPUCLK_SHIFT) & HVER_CPUCLK_BITMAP;  // cpu mode from default config
        sys = ((CONFIG_BOOT_HWVER) >> HVER_SYSCLK_SHIFT) & HVER_SYSCLK_BITMAP;  // sys mode from default config
        cheetah_cpuclk(cpu);    // set 125M clock
        cheetah_pad_mode(sys);  // set pad driving current
    }
    if (!(hver & (1 << HVER_WTDOG_SHIFT)))      //wdog disabled b30=0
    {
        TMREG(T2CN) = 0;        // disable wdog
    }

    printf("WDOG CTRL=%08x\n", TMREG(T2CN));
#ifdef CONFIG_CMD_CPUCLK
    cmd_clk(0, 0);
#endif
#ifdef CONFIG_CMD_PAD
    cmd_pad(0, 0);
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
/*
    overwrite_ipl_config

    bits[31:8] : 24bits OTP config to overwrite
    bits[7:4]  : boot_type to overwrite
    bits[1]    : enable boot_type overwrite
    bits[0]    : enable OTP config overwrite
*/
#define __PMU_CTRL             0x0004
#define __PMU_SLP_TMR_CTRL     0x0014
#define __PMU_WATCHDOG         0x0020
#define __PMU_SOFTWARE_GPREG   0x00FC
extern void reset_devices(void);

#if defined(NOR_4ADDR_MODE) || defined(NAND_OTP_MODE)
void delayed_restart_async(void)
{
    printf("Restarting in 3 seconds...\n");

    PMUREG(__PMU_WATCHDOG) = 0x01017700UL;
}
#endif

int cmd_rst(int argc, char *argv[])
{
    unsigned long overwrite_ipl_config = 0;

    if (argc)
        sscanf(argv[0], "%x", &overwrite_ipl_config);

    if (overwrite_ipl_config == 0)
    {
        if(__REG_READ32(__PMU_SOFTWARE_GPREG))
        {
            printf("Rebooting (%08x).\n", __REG_READ32(__PMU_SOFTWARE_GPREG));
            reset_devices();
            PMUREG(__PMU_SLP_TMR_CTRL) = 0x01000001;
            PMUREG(__PMU_CTRL) = 0x00000081;
        }
        else
        {
            printf("Rebooting now.\n");
            PMUREG(__PMU_WATCHDOG) = 0x01000001;
        }

        while (1) ;
    }
    else
    {
        printf("Rebooting with IPL config 0x%08x.\n", overwrite_ipl_config);
        *((volatile unsigned long *) 0xbf0048fc) = overwrite_ipl_config;
        *((volatile unsigned long *) 0xbf004814) = 0x01000001;
        *((volatile unsigned long *) 0xbf004804) = 0x5;
    }
    return ERR_OK;
}

cmdt cmdt_rst __attribute__ ((section("cmdt"))) =
{
"reboot", cmd_rst, "reboot <overwrite-ipl-config>"};

#if defined(CONFIG_FPGA)
/* 
    Example usage of susp test command

1. build miniboot.bin (http://tpesw1/svn/panther/private/alvis/miniboot)
   
2.
set file miniboot.bin
set buf b0000000
tftp
dump b0000000
susp

 */
int cmd_susp(int argc, char *argv[])
{
    void (*func) (void);

    func = (void *) 0x90000000UL;

    dcache_flush();
    icache_inv();

    func();
    asm volatile ("nop;nop;nop;");

    return ERR_OK;
}
#else

#include "str.h"
int cmd_susp(int argc, char *argv[])
{
    int susp_tick;
    void (*func) (void);

    if(argc>=1)
    {
        susp_tick = atoi(argv[0]);
        printf("suspend for %d ms\n", susp_tick);
        __REG_WRITE32(0x4814, susp_tick);
    }

#if 1
    dcache_flush();
    memcpy((void *) 0xb0000000, str_fw, sizeof(str_fw));
#endif
    func = (void *) 0x90000000UL;

    dcache_flush();
    icache_inv();

    func();
    asm volatile ("nop;nop;nop;");

    return ERR_OK;
}

#endif

cmdt cmdt_susp __attribute__ ((section("cmdt"))) =
{
"susp", cmd_susp, "suspend"};

int cmd_boot(int argc, char *argv[])
{
    int i;
    int big_endian = 1;

    for(i=0;i<argc;i++)
    {
        if(!strcmp(argv[i], "sd"))
            __REG_WRITE32(0x48fc, 0x0040004B);
        else if(!strcmp(argv[i], "little"))
            big_endian = 0;
        else if(!strcmp(argv[i], "big"))
            big_endian = 1;
    }

    __REG_WRITE32(0x4810, __REG_READ32(0x4810));

    if(big_endian)
    {
        __REG_WRITE32(0x4814, 0x010000ff);
        __REG_UPDATE32(0x4828, 0x1, 0x1);
        __REG_WRITE32(0x4804, 0x00C5);
    }
    else
    {
        __REG_WRITE32(0x4814, 0x010000ff);
        __REG_UPDATE32(0x4828, 0x0, 0x1);
        __REG_WRITE32(0x4804, 0x00C5);
    }

    return ERR_OK;
}

cmdt cmdt_boot __attribute__ ((section("cmdt"))) =
{
"boot", cmd_boot, "reboot with endian selection"};


#define NUM_DRV_MODE 4
#define NUM_DRV_REG 2
#define DRAM_SLEWRATE 0x50
#define JTAG_SLEWRATE 0x50000000
unsigned int paddrv[NUM_DRV_MODE][NUM_DRV_REG] = {
    {0x05555555 | JTAG_SLEWRATE, 0x0000000a | DRAM_SLEWRATE},   /* typical; dram=12mA others=8mA */
    {0x00000000 | JTAG_SLEWRATE, 0x00000000 | DRAM_SLEWRATE},   /* emi; all=4mA */
    {0x0fffffff | JTAG_SLEWRATE, 0x0000000f | DRAM_SLEWRATE},   /* timing; all=16mA */
    {0x05555555 | JTAG_SLEWRATE, 0x00000005 | DRAM_SLEWRATE},   /* typical2; all=8mA */
};

/*!
 * function:
 *
 *  \brief
 *  \param mode
 *  \return
 */

void cheetah_pad_mode(int mode)
{
#ifdef CONFIG_TODO
    volatile unsigned int *pt = &GPREG(DRIVER1);
    short i;

    if (!(mode < NUM_DRV_MODE))
        return;
    for (i = 0; i < NUM_DRV_REG; i++)
        pt[i] = paddrv[mode][i];
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \param phy
 *  \param down
 *  \return
 */
void cheetah_phy(unsigned short phy, unsigned int down)
{
    unsigned short val;

    val = cm_mdio_rd(phy, 0);
    if (down)
        cm_mdio_wr(phy, 0, val | 0x0800);
    else
        cm_mdio_wr(phy, 0, val & ~0x0800);
}

/*!
 * function:
 *
 *  \brief
 *  \param lan_en
 *  \param wan_en
 *  \return
 */

void cheetah_phy_up(unsigned int lan_en, unsigned int wan_en)
{
    if (lan_en)
    {
        printf("cheetah_phy_up...LAN_PORT(%d)\n", PORT_ETH1_IDX);
        cheetah_phy(PORT_ETH1_IDX, 0);
    }
    if (wan_en)
    {
        printf("cheetah_phy_up...WAN_PORT(%d)\n", PORT_ETH0_IDX);
        cheetah_phy(PORT_ETH0_IDX, 0);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param lan_en
 *  \param wan_en
 *  \return
 */

void cheetah_phy_down(unsigned int lan_en, unsigned int wan_en)
{
    if (lan_en)
    {
        printf("\ncheetah_phy_down...LAN_PORT(%d)", PORT_ETH1_IDX);
        cheetah_phy(PORT_ETH1_IDX, 1);
    }
    if (wan_en)
    {
        printf("\ncheetah_phy_down...WAN_PORT(%d)", PORT_ETH0_IDX);
        cheetah_phy(PORT_ETH0_IDX, 1);
    }
}

/*!
 * function: sdram_size
 *
 *  \brief
 *  \return SDRAM size
 */
int sdram_size(void)
{
    volatile unsigned int *tp, *addr0 = (void *) 0xa0000000;
    unsigned int odata, size, test = 0x5a3c0f78;

    //scan from 32 to 256 MB
    for (size = 0x02000000; size < 0x10000000; size <<= 1)
    {
        tp = (void *) phy_to_virt(size);
        odata = *tp;
        *tp = test;
        if (*addr0 == test)
        {
            *tp = odata;
            break;
        }
        else
            *tp = odata;
    }

    return size;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int sdram_size_cmd(int argc, char *argv[])
{
    printf("total= %x\n", sdram_size());
    return ERR_OK;
}

cmdt cmdt_sdram __attribute__ ((section("cmdt"))) =
{
"sdram", sdram_size_cmd, "sdram size"};

#if 1
/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int cmd_switch(int argc, char *argv[])
{
    cm_set_cpu_port();
    return ERR_OK;
}

cmdt cmdt_switch __attribute__ ((section("cmdt"))) =
{
"switch", cmd_switch, "switch reset"};
#endif

#if defined(BURST_READ_TEST)
unsigned long burst_read_test(void)
{
    int i, j;
    unsigned long acc = 0;
    volatile unsigned long *p = 0xA0000000;
    unsigned int time[3];

    time[0] = clock_get_clk();
    for (i = 0; i < 0x1000 / 4; i++)
        p[i] = i;

    time[1] = clock_get_clk();
    p = 0xA0000000;
    while (1)
    {
        acc += (p[0] + p[1] + p[2] + p[3]);
        p += 4;

        if ((unsigned long) p > 0xA0001000)
            break;
    }
    time[2] = clock_get_clk();
    printf("%u, %u, %u\n", time[0], time[1], time[2]);

    return acc;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int cmd_burst_read(int argc, char *argv[])
{
    burst_read_test();
    return ERR_OK;
}

cmdt cmdt_burst_read __attribute__ ((section("cmdt"))) =
{
"burst_read", cmd_burst_read, "test bus read rate"};
#endif

int chip_revision;
void chip_revision_detection(void)
{
    if((*(volatile unsigned long *)0xbfc03aa0UL)==0x27bdffd8)
        chip_revision = 1;
    else if((*(volatile unsigned long *)0xbfc03aa0UL)==0xac627ff8)
        chip_revision = 2;
}

