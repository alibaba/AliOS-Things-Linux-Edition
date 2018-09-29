/*=============================================================================+
|                                                                              |
| Copyright 2018                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
#include <arch/chip.h>
#include <arch/irq.h>
#include <common.h>
#include <netprot.h>
#include <lib.h>
#include <otp.h>
#include <pmu.h>
#include <mt_types.h>
#include <reg.h>

#if 1

void reset_devices(void)
{
    unsigned long reset_device_ids[] = { DEVICE_ID_HNAT, DEVICE_ID_SWP2, DEVICE_ID_SWP1, DEVICE_ID_SWP0,
                                         DEVICE_ID_SWP1_PORT, DEVICE_ID_SWP0_PORT, DEVICE_ID_SDIO, DEVICE_ID_PDMA, 
                                         DEVICE_ID_USB, DEVICE_ID_USBOTG, 0 };

    pmu_reset_devices(reset_device_ids);
    return;
}

static char *powercfg_str(unsigned int value)
{
    if(value&0xffff0000)
        return "dynamic";
    else if(value)
        return "on";
    else
        return "off";
}

void powercfg_show(void)
{
    unsigned int powercfg = bootvars.powercfg;

    printf("Power configurations: %08x\n", bootvars.powercfg);

    printf("usb:      %s\n", powercfg_str((powercfg & (POWERCTL_FLAG_USB_STATIC|POWERCTL_FLAG_USB_DYNAMIC))));
    printf("sdio:     %s\n", powercfg_str((powercfg & (POWERCTL_FLAG_SDIO_STATIC|POWERCTL_FLAG_SDIO_DYNAMIC))));
    printf("ether:    %s\n", powercfg_str((powercfg & (POWERCTL_FLAG_ETHERNET_STATIC|POWERCTL_FLAG_ETHERNET_DYNAMIC))));
    printf("acodec:   %s\n", powercfg_str((powercfg & (POWERCTL_FLAG_AUDIO_CODEC_STATIC|POWERCTL_FLAG_AUDIO_CODEC_DYNAMIC))));
    printf("tsi:      %s\n", powercfg_str((powercfg & (POWERCTL_FLAG_TSI_STATIC|POWERCTL_FLAG_TSI_DYNAMIC))));
    printf("wifi:     %s\n", powercfg_str((powercfg & (POWERCTL_FLAG_WIFI_STATIC|POWERCTL_FLAG_WIFI_DYNAMIC))));
}

#define POWERCFG_VALUE_OFF        0
#define POWERCFG_VALUE_ON         1
#define POWERCFG_VALUE_DYNAMIC    2
static unsigned int modify_flag(unsigned int powercfg, unsigned int static_flag, unsigned int dynamic_flag, int value)
{
    if(value==POWERCFG_VALUE_DYNAMIC)
    {
        powercfg = (powercfg | dynamic_flag);
        powercfg = (powercfg | static_flag);
    }
    else if(value==POWERCFG_VALUE_ON)
    {
        powercfg = (powercfg & ~dynamic_flag);
        powercfg = (powercfg | static_flag);
    }
    else if(value==POWERCFG_VALUE_OFF)
    {
        powercfg = (powercfg & ~dynamic_flag);
        powercfg = (powercfg & ~static_flag);
    }

    return powercfg;
}

extern int set_cmd(int argc, char **argv);
int parse_powercfg(int argc, char **argv)
{
    unsigned int value = 0;
    unsigned int powercfg = bootvars.powercfg;
    char set_input0[16], set_input1[32];
    char *set_input[2];

    if(!strcmp(argv[1], "on")||!strcmp(argv[1], "1"))
        value = POWERCFG_VALUE_ON;
    else if(!strcmp(argv[1], "off")||!strcmp(argv[1], "0"))
        value = POWERCFG_VALUE_OFF;
    else if(!strcmp(argv[1], "dynamic")||!strcmp(argv[1], "2"))
        value = POWERCFG_VALUE_DYNAMIC;
    else
        return -1;

    if(!strcmp(argv[0], "usb"))
        powercfg = modify_flag(powercfg, POWERCTL_FLAG_USB_STATIC, POWERCTL_FLAG_USB_DYNAMIC, value);
    else if (!strcmp(argv[0], "sdio"))
        powercfg = modify_flag(powercfg, POWERCTL_FLAG_SDIO_STATIC, POWERCTL_FLAG_SDIO_DYNAMIC, value);
    else if (!strcmp(argv[0], "ether"))
        powercfg = modify_flag(powercfg, POWERCTL_FLAG_ETHERNET_STATIC, POWERCTL_FLAG_ETHERNET_DYNAMIC, value);
    else if (!strcmp(argv[0], "acodec"))
        powercfg = modify_flag(powercfg, POWERCTL_FLAG_AUDIO_CODEC_STATIC, POWERCTL_FLAG_AUDIO_CODEC_DYNAMIC, value);
    else if (!strcmp(argv[0], "tsi"))
        powercfg = modify_flag(powercfg, POWERCTL_FLAG_TSI_STATIC, POWERCTL_FLAG_TSI_DYNAMIC, value);
    else if (!strcmp(argv[0], "wifi"))
        powercfg = modify_flag(powercfg, POWERCTL_FLAG_WIFI_STATIC, POWERCTL_FLAG_WIFI_DYNAMIC, value);
    else
        return -1;

    sprintf(set_input0, "%s", "powercfg");
    sprintf(set_input1, "%08x", powercfg);
    set_input[0] = set_input0;
    set_input[1] = set_input1;

    set_cmd(2, set_input);

    powercfg_show();

    return 0;
}

void powercfg_help(void)
{
    printk("powercfg <device name> <on/off>\n");
    printk("   <device name>: usb sdio ether acodec tsi wifi\n");
}

/*
    PMU power down test results (register 0xbf00480c)    3.3V

    AUDIO_CODEC(30):    0:398mA, 1: 380mA , diff: 18mA
    USB_PHY(28):        0:398mA, 1: 368mA , diff: 30mA
    ETH_PHY(27):        0:398mA, 1: 373mA , diff: 25mA
    MADC(26):           0:398mA, 1: 398mA , diff:    ?
    WIFI_ADC(25):       0:397mA, 1: 390mA , diff:  7mA
    WIFI_DAC(24):       0:398mA, 1: 398mA , diff:    ?
    RF(23):             0:398mA, 1: 398mA , diff:    ?
    SDIO_IO(19):        0:398mA, 1: 398mA , diff:    ?
    USB(3):             0:398mA, 1: 403mA , diff:  5mA
    WMAC(2):            0:398mA, 1: 405mA , diff:  7mA
    BB(31):             0:398mA, 1: 406mA , diff:  8mA
    OTP                                              ?
    PLL 1G              0: 398mA, 1: 279mA
    PLL 480M            0: 398mA, 1: 317mA
    OSC40M              0: 398mA, 1: 357mA
    BUCK_SOC            0: 398mA, 1: 378mA

    533Mhz DDR             397mA(idle)/463mA(memory test)
    400Mhz DDR             388mA(idle)/462mA(memory test)

    OpenWRT AP mode  (A2)
    WIFI_ADC(25):       0:347mA, 1: 338mA , diff:  9mA
    WIFI_DAC(24):       0:347mA, 1: 347mA , diff:    ?
    RF(23):             0:347mA, 1: 324mA , diff: 23mA
    RF_BUCK(18):        0:347mA, 1: 347mA , diff:    ?

    0xBF00_48F8 - RF_REG_CTRL   ( bit[1:0] == 3, software control )

    PA_ON(4)            0:303mA, 1: 503mA
    RX_ON(3)            0:303mA  1: 350mA
    TX_ON(2)            0:303mA, 1: 325mA

    0xBF00_4F04 - RF_CTRL_4_REG

    RF reg control Bit(0)

    ew bf004f04  80400b10  , 355mA
    ew bf004f04  80400b11  , 324mA

*/
/*
    Gated clock test results  (register 0xbf004a58/PMU_CLOCK_ENABLE)    3.3V

    0: CPU          396mA  345mA  51mA
    1: DBUS         396mA  383mA  13mA
    2: PBUS         397mA  352mA  45mA
    0/1/2:          396mA  329mA  67mA

    4: WIFIMAC      398mA  387mA  11mA
    5: BB           398mA  394mA   4mA
    6: RTC          398mA  398mA
    7: PCM          398mA  398mA
    8: SPDIF        398mA  398mA
    9/10/11: SWP012 398mA  392mA   6mA
    12:        HNAT 398mA  396mA   2mA
    13:        GSPI 398mA  398mA
    14:         TSI 398mA  397mA   1mA
    15:         AES 398mA  398mA
    16:        SDIO 397mA  395mA   2mA
    17:         USB 397mA  397mA
    18:      USBOTG 397mA  397mA
    19:        EPHY 397mA  397mA
    20:         DDR
    21:     GRAPHIC
    22:       EJTAG 393mA  393mA
    23:       TIMER 395mA  395mA
    24:       SMI   396mA  396mA
    25/26/27: UART  397mA  395mA   2mA
    28:       PDMA  397mA  396mA   1mA
    29:        PWM  396mA  396mA

    ALL ON:   397mA
    ALL OFF:  310mA
*/

void powercfg_apply(void)
{
    unsigned int powercfg = bootvars.powercfg;
    unsigned long pmu_clock_enable_reg = REG_READ32(PMU_CLOCK_ENABLE);
    unsigned long pmu_stdby_ctrl_reg = REG_READ32(PMU_STDBY_CTRL);
    unsigned long pmu_buck_ctrl_reg = REG_READ32(PMU_BUCK_CTRL);
    unsigned long pmu_audio_ctrl_reg = REG_READ32(PMU_ADC_DAC_REG_CTRL);

    if(0==(powercfg & POWERCTL_FLAG_USB_STATIC))
    {
        pmu_clock_enable_reg &= ~(PMU_CLOCK_USBOTG|PMU_CLOCK_USB);
        REG_UPDATE32(PMU_CTRL, PMU_CTRL_USB_OFF, PMU_CTRL_USB_OFF);

        /* turn off USB PHY will also stop EPHY */
        if(0==(powercfg & POWERCTL_FLAG_ETHERNET_STATIC))
        {
            pmu_stdby_ctrl_reg |= STDBY_PD_USB_PHY;
            pmu_buck_ctrl_reg &= ~(BUCK_ENABLE_USB);
        }
    }

    if(0==(powercfg & POWERCTL_FLAG_SDIO_STATIC))
    {
        pmu_clock_enable_reg &= ~(PMU_CLOCK_SDIO);
        pmu_stdby_ctrl_reg |= STDBY_PD_SDIO_IO;
    }

    if(0==(powercfg & POWERCTL_FLAG_ETHERNET_STATIC))
    {
        pmu_clock_enable_reg &= ~(PMU_CLOCK_EPHY | PMU_CLOCK_HNAT | PMU_CLOCK_SWP2 | PMU_CLOCK_SWP1 | PMU_CLOCK_SWP0);
        pmu_stdby_ctrl_reg |= STDBY_PD_EPHY;
    }

    if(0==(powercfg & POWERCTL_FLAG_AUDIO_CODEC_STATIC))
    {
        pmu_audio_ctrl_reg &= ~(INTERNAL_ADC_EN | INTERNAL_DAC_EN);
        pmu_stdby_ctrl_reg |= STDBY_PD_AUDIO_CODEC;
    }
    else
    {
        pmu_audio_ctrl_reg |= (INTERNAL_ADC_EN | INTERNAL_DAC_EN);
        REG_WRITE32(PMU_AUDIO_DAC_REG2, 0x5540A555); // stop noise when enable internal dac
    }

    if(0==(powercfg & POWERCTL_FLAG_TSI_STATIC))
    {
        pmu_clock_enable_reg &= ~(PMU_CLOCK_TSI);
    }

    if(0==(powercfg & POWERCTL_FLAG_WIFI_STATIC))
    {
        pmu_clock_enable_reg &= ~(PMU_CLOCK_BB | PMU_CLOCK_WMAC);
        pmu_stdby_ctrl_reg |= STDBY_PD_RF;
    }

    REG_WRITE32(PMU_CLOCK_ENABLE, pmu_clock_enable_reg);
    REG_WRITE32(PMU_STDBY_CTRL, pmu_stdby_ctrl_reg);
    REG_WRITE32(PMU_BUCK_CTRL, pmu_buck_ctrl_reg);
    REG_WRITE32(PMU_ADC_DAC_REG_CTRL, pmu_audio_ctrl_reg);
}

int powercfg_cmd(int argc, char **argv)
{
    if (argc < 1)
    {
        powercfg_show();
    }
    else if((argc == 1) && !strcmp(argv[0], "apply"))
    {
        powercfg_apply();
    }
    else if(argc == 2)
    {
        if(0 > parse_powercfg(argc, argv))
            powercfg_help();
    }
    else
    {
        powercfg_help();
    }

    return ERR_OK;
}

cmdt cmdt_powercfg __attribute__ ((section("cmdt"))) =
{
"powercfg", powercfg_cmd, "powercfg <device name> <on/off/dynamic>\n" };

#endif

void pmu_reset_devices(unsigned long *device_ids)
{
    int i = 0;
    unsigned long curr_id;
    unsigned long pmu_reset_reg24_mask = 0;
    unsigned long pmu_reset_reg25_mask = 0;

    if(device_ids==NULL)
    {
        return;
    }

    while(1)
    {
        curr_id = device_ids[i];
        if((curr_id/100)==24)
        {
            pmu_reset_reg24_mask |= (0x01 << (curr_id%100));
        }
        else if((curr_id/100)==25)
        {
            pmu_reset_reg25_mask |= (0x01 << (curr_id%100));
        }
        else
        {
            break;
        }

        i++;
    }

    if(pmu_reset_reg24_mask)
    {
        //printf("Reset PMU 0x%08x mask %08x\n", PMU_RESET_REG24, pmu_reset_reg24_mask);
        REG_UPDATE32(PMU_RESET_REG24, 0x0, pmu_reset_reg24_mask);
    }

    if(pmu_reset_reg25_mask)
    {
        //printf("Reset PMU 0x%08x mask %08x\n", PMU_RESET_REG25, pmu_reset_reg25_mask);
        REG_UPDATE32(PMU_RESET_REG25, 0x0, pmu_reset_reg25_mask);
    }

    udelay(1);

    if(pmu_reset_reg25_mask)
    {
        REG_UPDATE32(PMU_RESET_REG25, 0xffffffff, pmu_reset_reg25_mask);
    }

    if(pmu_reset_reg24_mask)
    {
        REG_UPDATE32(PMU_RESET_REG24, 0xffffffff, pmu_reset_reg24_mask);
    }

    return;
}

void pmu_set_gpio_function(int *gpio_ids, unsigned long *gpio_funcs)
{
    int i = 0;
    int curr_id;
    unsigned long func;
    u32 gpio_func_0_7_val = 0;
    u32 gpio_func_8_15_val = 0;
    u32 gpio_func_16_23_val = 0;
    u32 gpio_func_24_31_val = 0;
    u32 gpio_func_32_39_val = 0;
    u32 gpio_func_0_7_mask = 0;
    u32 gpio_func_8_15_mask = 0;
    u32 gpio_func_16_23_mask = 0;
    u32 gpio_func_24_31_mask = 0;
    u32 gpio_func_32_39_mask = 0;

    if(gpio_ids==0)
        return;
    while(1)
    {
        curr_id = gpio_ids[i];
        func = gpio_funcs[i];
        if(curr_id < 0)
        {
            break;
        }
        else if(curr_id < 8)
        {
            gpio_func_0_7_mask |= (0x0F << (curr_id * 4));
            gpio_func_0_7_val |=  ((0x0F & func) << (curr_id * 4));
        }
        else if(curr_id < 16)
        {
            gpio_func_8_15_mask |= (0x0F << ((curr_id - 8) * 4));
            gpio_func_8_15_val |=  ((0x0F & func) << ((curr_id - 8) * 4));
        }
        else if(curr_id < 24)
        {
            gpio_func_16_23_mask |= (0x0F << ((curr_id - 16) * 4));
            gpio_func_16_23_val |=  ((0x0F & func) << ((curr_id - 16) * 4));
        }
        else if(curr_id < 32)
        {
            gpio_func_24_31_mask |= (0x0F << ((curr_id - 24) * 4));
            gpio_func_24_31_val |=  ((0x0F & func) << ((curr_id - 24) * 4));
        }
        else if(curr_id < 40)
        {
            gpio_func_32_39_mask |= (0x0F << ((curr_id - 32) * 4));
            gpio_func_32_39_val |=  ((0x0F & func) << ((curr_id - 32) * 4));
        }

        i++;
    }

    if(gpio_func_0_7_mask)
        REG_UPDATE32(PMU_GPIO_FUNC_0_7, gpio_func_0_7_val, gpio_func_0_7_mask);

    if(gpio_func_8_15_mask)
        REG_UPDATE32(PMU_GPIO_FUNC_8_15, gpio_func_8_15_val, gpio_func_8_15_mask);

    if(gpio_func_16_23_mask)
        REG_UPDATE32(PMU_GPIO_FUNC_16_23, gpio_func_16_23_val, gpio_func_16_23_mask);

    if(gpio_func_24_31_mask)
        REG_UPDATE32(PMU_GPIO_FUNC_24_31, gpio_func_24_31_val, gpio_func_24_31_mask);

    if(gpio_func_32_39_mask)
        REG_UPDATE32(PMU_GPIO_FUNC_32_39, gpio_func_32_39_val, gpio_func_32_39_mask);

}

void pmu_set_gpio_driving_strength(int *gpio_ids, unsigned long *gpio_vals)
{
    int i = 0;
    int curr_id;
    unsigned long value;
    u32 gpio_driver_0_15_mask = 0;
    u32 gpio_driver_0_15_val = 0;
    u32 gpio_driver_16_31_mask = 0;
    u32 gpio_driver_16_31_val = 0;
    u32 gpio_driver_32_47_mask = 0;
    u32 gpio_driver_32_47_val = 0;

    if(gpio_ids==NULL)
        return;

    while(1)
    {
        curr_id = gpio_ids[i];
        value = gpio_vals[i];
        if(curr_id < 0)
        {
            break;
        }
        else if(curr_id < 16)
        {
            gpio_driver_0_15_mask |= (0x03 << (curr_id * 2));
            gpio_driver_0_15_val |=  ((0x03 & value) << (curr_id * 2));
        }
        else if(curr_id < 32)
        {
            gpio_driver_16_31_mask |= (0x03 << ((curr_id - 16) * 2));
            gpio_driver_16_31_val |= ((0x03 & value) << ((curr_id - 16) * 2));
        }
        else if(curr_id < 48)
        {
            gpio_driver_32_47_mask |= (0x03 << ((curr_id - 32) * 2));
            gpio_driver_32_47_val |= ((0x03 & value) << ((curr_id - 32) * 2));
        }

        i++;
    }

    if(gpio_driver_0_15_mask)
        REG_UPDATE32(PMU_GPIO_DRIVER_0_15, gpio_driver_0_15_val, gpio_driver_0_15_mask);

    if(gpio_driver_16_31_mask)
        REG_UPDATE32(PMU_GPIO_DRIVER_16_31, gpio_driver_16_31_val, gpio_driver_16_31_mask);

    if(gpio_driver_32_47_mask)
        REG_UPDATE32(PMU_GPIO_DRIVER_32_47, gpio_driver_32_47_val, gpio_driver_32_47_mask);

}

struct clock_cfg
{
    u16 post_div;
    u16 pre_div;
    u32 clk_rate;
};

static struct clock_cfg cpu_clk_cfg[] =
{
    { 0, 0,     550000 * 1000, },   /* 0: default */
    { 0, 0x8c0, 914243 * 1000, },
    { 0, 0x900, 888888 * 1000, },
    { 0, 0x940, 864902 * 1000, },
    { 0, 0x980, 842105 * 1000, },
    { 0, 0x9C0, 820479 * 1000, },
    { 0, 0xA00, 800000 * 1000, },
    { 0, 0xA40, 780518 * 1000, },
    { 0, 0xA80, 761904 * 1000, },   /* 5 */
    { 0, 0xAC0, 744158 * 1000, },
    { 0, 0xB00, 727272 * 1000, },
    { 0, 0xB40, 711136 * 1000, },
    { 0, 0xB80, 695652 * 1000, },
    { 0, 0xBC0, 680827 * 1000, },   /* 10 */
    { 0, 0xC00, 666666 * 1000, },
    { 0, 0xC40, 653082 * 1000, },
    { 0, 0xC80, 640000 * 1000, },
    { 0, 0xCC0, 627431 * 1000, },
    { 0, 0xD00, 615384 * 1000, },   /* 15 */
    { 0, 0xD40, 603791 * 1000, },
    { 0, 0xD80, 592592 * 1000, },
    { 0, 0xDC0, 581801 * 1000, },
    { 0, 0xE00, 571428 * 1000, },
    { 0, 0xE40, 561419 * 1000, },   /* 20 */
    { 0, 0xE80, 551724 * 1000, },
#if 0
    { 0, 0xEC0, 542358 * 1000, },
    { 0, 0xF00, 533333 * 1000, },
    { 0, 0xF40, 524603 * 1000, },
    { 0, 0xF80, 516129 * 1000, },
    { 0, 0xFC0, 507923 * 1000, },
#endif
};
static int cpu_curr_clk_cfg_no;

#define NUM_OF_CPU_CLK_CONF (sizeof(cpu_clk_cfg)/sizeof(struct clock_cfg))

unsigned long clk_get_cpu_lpj(void)
{
#if defined(CONFIG_FPGA)
    return 375000;
#else
    if(0==cpu_curr_clk_cfg_no)
        return 2742857;
    return (cpu_clk_cfg[cpu_curr_clk_cfg_no].clk_rate / 200);
#endif
}

static struct clock_cfg axi_clk_cfg[] =
{
    { 2, 0xB3C, 177777 * 1000, },   /* 0: default */
    { 3, 0xB3C, 118683 * 1000, },
    { 4, 0xB3C,  89012 * 1000, },
    { 5, 0xB3C,  71209 * 1000, },
    { 6, 0xB3C,  59341 * 1000, },
    { 7, 0xB3C,  50864 * 1000, },
    { 8, 0xB3C,  44506 * 1000, },
    { 2, 0xA7C, 190400 * 1000, },
    { 2, 0x9FC, 200000 * 1000, },
    { 2, 0x97C, 210500 * 1000, },
    { 2, 0x93C, 216200 * 1000, },
    { 2, 0x8BC, 228500 * 1000, },
    { 2, 0x87C, 235726 * 1000, },
};

static int axi_curr_clk_cfg_no;

#define NUM_OF_AXI_CLK_CONF (sizeof(axi_clk_cfg)/sizeof(struct clock_cfg))

static void clk_cpu(int cfg_no)
{
    if((cfg_no >= 0) && (cfg_no < NUM_OF_CPU_CLK_CONF))
    {
        cpu_curr_clk_cfg_no = cfg_no;
        // CPU clock settings
        /*  POST  PRE
             0  0x840     969Mhz
             0  0x8C0     914Mhz
             0  0x900     888Mhz
             0  0x940     865Mhz
             0  0x980     842Mhz
             0  0xA00     800Mhz

             0  0xA40     780Mhz
             0  0xA80     762Mhz
             0  0xAC0     744Mhz

             0  0xEC0     550Mhz
         */
        if(cfg_no==0)
        {
            REG_UPDATE32(PMU_XDIV_REG2_47_32, XDIV_REG2_CPU_550M, XDIV_REG2_CPU_550M);
            REG_UPDATE32(PMU_CPLL_XDIV_REG2_0_31, 0, (CPU_550M_UPDATE|CPU_550M_EN));
            REG_UPDATE32(PMU_CPLL_XDIV_REG2_0_31, (CPU_550M_UPDATE|CPU_550M_EN), (CPU_550M_UPDATE|CPU_550M_EN));
        }
        else
        {
            REG_UPDATE32(PMU_XDIV_REG2_47_32, 0, XDIV_REG2_CPU_550M);
            REG_UPDATE32(PMU_CPLL_XDIV_REG2_0_31, 0, (CPU_550M_UPDATE|CPU_550M_EN));
            //REG_UPDATE32(PMU_CPLL_XDIV_REG2_0_31, 0, CPU_550M_EN);

            REG_UPDATE32(PMU_XDIV_REG_31_0, 0, CPU_NDIV_UPDATE | CPU_FDIV_UPDATE);

            REG_UPDATE32(PMU_XDIV_REG2_47_32, cpu_clk_cfg[cfg_no].pre_div, XDIV_CPU_PRE_DIV);

            REG_UPDATE32(PMU_XDIV_REG_31_0, CPU_NDIV_UPDATE | CPU_FDIV_UPDATE | cpu_clk_cfg[cfg_no].post_div,
                          CPU_NDIV_UPDATE | CPU_FDIV_UPDATE | XDIV_CPU_POST_DIV);
        }
    }
}

static void clk_axi(int cfg_no)
{
    if((cfg_no >= 0) && (cfg_no < NUM_OF_AXI_CLK_CONF))
    {
        axi_curr_clk_cfg_no = cfg_no;

        // DBUS clock settings
        REG_UPDATE32(PMU_XDIV_REG_31_0, 0, DBUS_FDIV_UPDATE | DBUS_NDIV_UPDATE);

        REG_UPDATE32(PMU_XDIV_REG2_31_0, axi_clk_cfg[cfg_no].post_div, XDIV_DBUS_POST_DIV);

        REG_UPDATE32(PMU_XDIV_REG_31_0, DBUS_FDIV_UPDATE | DBUS_NDIV_UPDATE, DBUS_FDIV_UPDATE | DBUS_NDIV_UPDATE);
    }
}

static int parse_clk(int argc, char **argv)
{
    int cfg_no;

    if(!strcmp(argv[0], "cpu"))
    {
        cfg_no = atoi(argv[1]);

        if((cfg_no >= 0) && (cfg_no < NUM_OF_CPU_CLK_CONF))
        {
            printf("Change CPU clock to %dMhz (%d)\n",
                   cpu_clk_cfg[cfg_no].clk_rate / (1000 * 1000), cfg_no);

            clk_cpu(cfg_no);
        }
    }
    else if(!strcmp(argv[0], "axi"))
    {
        cfg_no = atoi(argv[1]);

        if((cfg_no >= 0) && (cfg_no < NUM_OF_AXI_CLK_CONF))
        {
            printf("Change AXI clock to %dMhz (%d)\n",
                   axi_clk_cfg[cfg_no].clk_rate / (1000 * 1000), cfg_no);

            clk_axi(cfg_no);
        }
    }

    return ERR_OK;
}

static void clk_show(void)
{
    printk("CPU: %dMhz (%d)\n", cpu_clk_cfg[cpu_curr_clk_cfg_no].clk_rate / (1000 * 1000),
           cpu_curr_clk_cfg_no);
    printk("AXI: %dMhz (%d)\n", axi_clk_cfg[axi_curr_clk_cfg_no].clk_rate / (1000 * 1000),
           axi_curr_clk_cfg_no);
}

static void clk_help(void)
{
    int i;
    printf("clk <cpu/axi> <cfg_no>\n");

    for(i=0;i<NUM_OF_CPU_CLK_CONF;i++)
    {
        printf("     cpu %2d: %dMhz\n", i, cpu_clk_cfg[i].clk_rate / (1000 * 1000));
    }

    printf("\n");

    for(i=0;i<NUM_OF_AXI_CLK_CONF;i++)
    {
        printf("     axi %d: %dMhz\n", i, axi_clk_cfg[i].clk_rate / (1000 * 1000));
    }
}

static int clk_cmd(int argc, char **argv)
{
    if (argc < 1)
    {
        clk_show();
    }
    else if(argc == 2)
    {
        if(0 > parse_clk(argc, argv))
            clk_help();
    }
    else
    {
        clk_help();
    }

    return ERR_OK;
}

cmdt cmdt_clk __attribute__ ((section("cmdt"))) =
{
"clk", clk_cmd, "clk\n" };

void clkcfg_apply(void)
{
    unsigned int clkcfg = bootvars.clkcfg;

    if(clkcfg)
    {
        clk_cpu((clkcfg & CLKCTRL_FLAG_CPU));
        clk_axi(((clkcfg & CLKCTRL_FLAG_AXI) >> 8));
    }
}

