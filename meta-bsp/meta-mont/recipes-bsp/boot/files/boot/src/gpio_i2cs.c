/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file gpio_i2cs.c
*   \brief I2C slave by gpio
*   \author Montage
*/
#ifdef CONFIG_I2CS
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
#include <netdev.h>
#include <cm_mac.h>
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

#define BYTE_MSB 0x80

//#define I2CS_PRETEND_A1
//#define I2CS_MEASURE_BOOT_TIME
/* use GPIO6 to measure boot time, default it is low */
#define GPIO_NUM_BOOT_TIME 6

//#define I2CS_DEBUG                  // show access address in console
#define i2cs_log(fmt, args...) printf(fmt, ##args)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

struct i2cs_pin
{
    u32 chip_id;
    u8 sda_sdr;
    u8 scl_sdr;
    u8 sda_ddr;
    u8 scl_ddr;
    u32 en_swi2c_fnc_sdr;
    u32 en_swi2c_fnc_ddr;
    u32 dis_swi2c_fnc_sdr;
    u32 dis_swi2c_fnc_ddr;
};
static struct i2cs_pin mpin[] = {
/*    chip_id,     sda_sdr, scl_sdr, sda_ddr, scl_ddr,           en_sdr,           en_ddr,           dis_sdr,           dis_ddr */
#ifdef CONFIG_FPGA
    {CHIP_ID_A0, 33, 32, 31, 30, EN_SWI2C_AUX_FNC, EN_SWI2C_FNC,
     (EN_UART_FNC | EN_SWI2C_FNC | EN_I2C_AUX_FNC | EN_UART_AUX_FNC |
      EN_SWI2C_AUX_FNC),
     (EN_UART_FNC | EN_SWI2C_FNC | EN_I2C_AUX_FNC | EN_UART_AUX_FNC |
      EN_SWI2C_AUX_FNC)},
#else
    {CHIP_ID_A0, 29, 28, 29, 28, EN_FUNCMODE, EN_FUNCMODE,
     (EN_SDIO_FNC | EN_ETH0_FNC),
     (EN_SDIO_FNC | EN_ETH0_FNC)},
    {CHIP_ID_A1, 23, 22, 9, 11, EN_FUNCMODE, EN_FUNCMODE,
     (EN_MDIO_AUX_FNC | EN_PCM1_FNC),
     (EN_ADC_OUT_FNC | EN_LED_FNC)},
#endif
};

#define MPIN_NUM (sizeof(mpin)/sizeof(struct i2cs_pin))

struct phy_reg
{
    u8 offset;
    u16 value;
};
static struct phy_reg phy_init[] = {
    {0x1f, 0x0000},
    {0x14, 0x6000},
    {0x1f, 0x0002},
    {0x11, 0x8050},
    {0x12, 0x8975},
    {0x13, 0xba60},
    {0x1f, 0x0004},
    {0x12, 0x5a40},
    {0x1f, 0x0001},
    {0x10, 0xf598},
    {0x11, 0xa528},
    {0x12, 0x0f90},
    {0x17, 0x6bbd},
    {0x17, 0x6bbc},
    {0x18, 0xf400},
    {0x13, 0xa4d8},
    {0x14, 0x3780},
    {0x15, 0xb600},
    {0x16, 0xf900},
};

#define PHY_INIT_NUM (sizeof(phy_init)/sizeof(struct phy_reg))
static struct phy_reg phy_lut14_reg[] = {
    {0x1f, 0x0002},
    {0x16, 0x208c},
    {0x16, 0x2118},
    {0x16, 0x21a2},
    {0x16, 0x2228},
    {0x16, 0x22aa},
    {0x16, 0x2328},
    {0x16, 0x23a2},
    {0x16, 0x2418},
    {0x16, 0x248c},
    {0x16, 0x2500},
    {0x16, 0x258c},
    {0x16, 0x2618},
    {0x16, 0x26a2},
    {0x16, 0x2728},
    {0x16, 0x27aa},
    {0x16, 0x2828},
    {0x16, 0x28a2},
    {0x16, 0x291c},
    {0x16, 0x299c},
    {0x16, 0x2a1c},
    {0x16, 0x2a9c},
    {0x16, 0x2b1c},
    {0x16, 0x2b9c},
    {0x16, 0x2c1c},
    {0x16, 0x2c9c},
    {0x16, 0x2d1c},
    {0x16, 0x2d9c},
    {0x16, 0x2e1c},
    {0x16, 0x2e8e},
    {0x16, 0x0e8c},
};

#define PHY_LUT14_REG_NUM (sizeof(phy_lut14_reg)/sizeof(struct phy_reg))
static struct phy_reg phy_lut15_reg[] = {
    {0x1f, 0x0002},
    {0x16, 0x208c},
    {0x16, 0x2118},
    {0x16, 0x21a2},
    {0x16, 0x2228},
    {0x16, 0x22aa},
    {0x16, 0x2328},
    {0x16, 0x23a2},
    {0x16, 0x2418},
    {0x16, 0x248c},
    {0x16, 0x2500},
    {0x16, 0x258c},
    {0x16, 0x2618},
    {0x16, 0x26a2},
    {0x16, 0x2728},
    {0x16, 0x27aa},
    {0x16, 0x2828},
    {0x16, 0x28a2},
    {0x16, 0x291e},
    {0x16, 0x299e},
    {0x16, 0x2a1e},
    {0x16, 0x2a9e},
    {0x16, 0x2b1e},
    {0x16, 0x2b9e},
    {0x16, 0x2c1e},
    {0x16, 0x2c9e},
    {0x16, 0x2d1e},
    {0x16, 0x2d9e},
    {0x16, 0x2e1e},
    {0x16, 0x2e90},
    {0x16, 0x0e8c},
};

#define PHY_LUT15_REG_NUM (sizeof(phy_lut15_reg)/sizeof(struct phy_reg))
static struct phy_reg phy_lutmau_reg[] = {
    {0x1f, 0x0002},
    {0x16, 0x208c},
    {0x16, 0x2118},
    {0x16, 0x21a2},
    {0x16, 0x2228},
    {0x16, 0x22aa},
    {0x16, 0x2328},
    {0x16, 0x23a2},
    {0x16, 0x2418},
    {0x16, 0x248c},
    {0x16, 0x2500},
    {0x16, 0x258c},
    {0x16, 0x2618},
    {0x16, 0x26a2},
    {0x16, 0x2728},
    {0x16, 0x27aa},
    {0x16, 0x2828},
    {0x16, 0x28a8},
    {0x16, 0x2926},
    {0x16, 0x29a4},
    {0x16, 0x2a22},
    {0x16, 0x2aa0},
    {0x16, 0x2b1e},
    {0x16, 0x2b9e},
    {0x16, 0x2c1e},
    {0x16, 0x2c9e},
    {0x16, 0x2d1e},
    {0x16, 0x2d9e},
    {0x16, 0x2e1e},
    {0x16, 0x2e90},
    {0x16, 0x0e8c},
};

#define PHY_LUTMAU_REG_NUM (sizeof(phy_lutmau_reg)/sizeof(struct phy_reg))

struct i2c_core
{
    /* gpval->gpset->gpclr->gpdir->gpsel */
    u16 sda_gpval;
    u16 scl_gpval;
    u32 sda;
    u32 scl;
    u32 enable;
    u32 disable;
};
static struct i2c_core mi2c;
#define SDA_GPVAL               (mi2c.sda_gpval+GPVAL)
#define SDA_GPSET               (mi2c.sda_gpval+GPSET)
#define SDA_GPCLR               (mi2c.sda_gpval+GPCLR)
#define SDA_GPDIR               (mi2c.sda_gpval+GPDIR)
#define SDA_GPSEL               (mi2c.sda_gpval+GPSEL)
#define SCL_GPVAL               (mi2c.scl_gpval+GPVAL)
#define SCL_GPSET               (mi2c.scl_gpval+GPSET)
#define SCL_GPCLR               (mi2c.scl_gpval+GPCLR)
#define SCL_GPDIR               (mi2c.scl_gpval+GPDIR)
#define SCL_GPSEL               (mi2c.scl_gpval+GPSEL)
#define GPIO_I2C_SDA            (1 << (mi2c.sda % 32))
#define GPIO_I2C_SCL            (1 << (mi2c.scl % 32))
#define EN_GPIO_I2C_FNC         (mi2c.enable)
#define DIS_GPIO_I2C_FNC        (mi2c.disable)
#define GPIO_I2C_CLKW           0x300

enum
{
    I2CS_STATE_IDLE = 1,
    I2CS_STATE_SLAVE,
    I2CS_STATE_GMA,
    I2CS_STATE_GDA,
    I2CS_STATE_PDA,
    I2CS_STATE_PDA_ACK,
    I2CS_STATE_ACK,
    I2CS_STATE_NACK,
    I2CS_ACT_IDLE = 11,
    I2CS_ACT_START,
    I2CS_ACT_STOP,
    I2CS_ACT_RISING,
    I2CS_ACT_FALLING
};

#define I2CS_DEV_ANALOG       (0x00)
#define I2CS_DEV_USB_PHY_TEST (0x01)
#define I2CS_DEV_USB_PHY_CTRL (0x02)
#define I2CS_DEV_ETH_PHY      (0x03)
#define I2CS_DEV_BB           (0x04)
#define I2CS_DEV_AUTO         (0x7E)
#define I2CS_DEV_CHEETAH      (0x7F)
struct i2cs_device
{
    u8 dev_id;
    u32 reg_base;
};
static struct i2cs_device i2cs_devs[] = {
    {I2CS_DEV_ANALOG, 0xaf005800},
    {I2CS_DEV_USB_PHY_TEST, 0xaf00c200},
    {I2CS_DEV_USB_PHY_CTRL, 0xaf00c300},
    {I2CS_DEV_ETH_PHY, 0x00000000},
    {I2CS_DEV_BB, 0x00000000},
    {I2CS_DEV_AUTO, 0x00000000},
    {I2CS_DEV_CHEETAH, 0x00000000},
};

#define I2CS_DEV_NUM (sizeof(i2cs_devs)/sizeof(struct i2cs_device))
static unsigned char dev_id = I2CS_DEV_ANALOG;
static unsigned char hw_i2c_regs[8] = { 0 };

static unsigned char hw_i2c_index = 0;

union hw_i2c_data32
{
    unsigned char byte[4];
    unsigned long dword;
};
static union hw_i2c_data32 hw_i2c_value32 = {.dword = 0 };

static unsigned char hw_i2c_vindex32 = 0;
static union hw_i2c_data32 hw_i2c_addr32 = {.dword = 0x80000000 };

static unsigned long hw_i2c_aindex32 = 0;

#define AUTO_CMD_MT             3
#define AUTO_CMD_PHY_RESET      4
#define AUTO_CMD_PHY_RAND_TX    5
#define AUTO_CMD_PHY_TEST_TX    6
#define AUTO_CMD_PHY_LUT14      7
#define AUTO_CMD_PHY_LUT15      8
#define AUTO_CMD_PHY_LUTMAU     9
#define AUTO_CMD_SDHC_DISABLE   10
#define AUTO_CMD_SDHC_ENABLE    11
#define AUTO_CMD_SDHC_IO_READ   12
#define AUTO_CMD_SDHC_IO_WRITE  13
#define AUTO_CMD_SDHC_IO_STOP   14
#define AUTO_CMD_SDHC_READ      15
#define AUTO_CMD_SDHC_WRITE     16
#define AUTO_CMD_STOP           255
static unsigned long auto_i2c_reg = 0;
static unsigned long auto_i2c_result = 0;

#define I2CS_DATA_INIT(x,y)     ((x) = (0x01000100 | (y)))
#define I2CS_DATA_DONE(x)       ((x) & 0x00010000) ? 1 : 0

#define SDA_HIGH()  GPREG(SDA_GPSET) = GPIO_I2C_SDA
#define SDA_LOW()   GPREG(SDA_GPCLR) = GPIO_I2C_SDA
#define SDA_DATA()  (GPREG(SDA_GPVAL) & GPIO_I2C_SDA) ? 1 : 0
#define SDA_OUT()   GPREG(SDA_GPDIR) |= GPIO_I2C_SDA
#define SDA_IN()    GPREG(SDA_GPDIR) &= ~GPIO_I2C_SDA
#define SCL_HIGH()  GPREG(SCL_GPSET) = GPIO_I2C_SCL
#define SCL_LOW()   GPREG(SCL_GPCLR) = GPIO_I2C_SCL
#define SCL_DATA()  (GPREG(SCL_GPVAL) & GPIO_I2C_SCL) ? 1 : 0

static unsigned char i2c_run = 1;
static int restore_i2cs_test_pinmux = 0;
static int restore_mem_test_pinmux = 0;

int phy_tx_test = 0;
int phy_tx_test_ready = 0;

extern cmdev g_cmdev;
extern netbuf[];
extern struct nbuf_cfg nbuf_cfg;

inline static void I2C_DELAY(void)
{
    int _www;
    for (_www = 0; _www < GPIO_I2C_CLKW; _www++) ;
}

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

static void i2cs_grant(int enabled)
{
    static int rc = 0;
    if (enabled)
    {
        rc = GPREG(PINMUX);
        GPREG(PINMUX) &= ~(DIS_GPIO_I2C_FNC);
        GPREG(PINMUX) |= EN_GPIO_I2C_FNC;
        restore_i2cs_test_pinmux = GPREG(PINMUX);
    }
    else
    {
        GPREG(PINMUX) = rc;

#if  0                          /* ----- #if 0 : If0Label_1 ----- */
        if (1)
        {
            /* avoid to appear some garbage on UART */
            {
                int i;
                for (i = 0; i < 100; i++)
                    I2C_DELAY();
            }
            printf("\n");
        }
#endif                          /* ----- #if 0 : If0Label_1 ----- */

    }
}

static void i2cs_get_grant(void)
{
    i2cs_grant(1);
}

static void i2cs_put_grant(void)
{
    i2cs_grant(0);
}

static void mi2c_init(void)
{
    u16 chip_id = ANAREG(CHIPID) & 0xFFFF;
    int i;

#ifdef I2CS_PRETEND_A1
    chip_id = CHIP_ID_A1;
#endif

    for (i = 0; i < MPIN_NUM; i++)
    {
        if (chip_id == mpin[i].chip_id)
        {
            if (!(GPREG(PINMUX) & EN_SIP_FNC))
            {
                // SDR mode
                mi2c.sda_gpval = (mpin[i].sda_sdr / 32) ? GP2VAL : GPVAL;
                mi2c.scl_gpval = (mpin[i].scl_sdr / 32) ? GP2VAL : GPVAL;
                mi2c.sda = mpin[i].sda_sdr;
                mi2c.scl = mpin[i].scl_sdr;
                mi2c.enable = mpin[i].en_swi2c_fnc_sdr;
                mi2c.disable = mpin[i].dis_swi2c_fnc_sdr;
            }
            else
            {
                // DDR mode
                mi2c.sda_gpval = (mpin[i].sda_ddr / 32) ? GP2VAL : GPVAL;
                mi2c.scl_gpval = (mpin[i].scl_ddr / 32) ? GP2VAL : GPVAL;
                mi2c.sda = mpin[i].sda_ddr;
                mi2c.scl = mpin[i].scl_ddr;
                mi2c.enable = mpin[i].en_swi2c_fnc_ddr;
                mi2c.disable = mpin[i].dis_swi2c_fnc_ddr;

                if (chip_id == CHIP_ID_A1)
                {
                    /*
                     * For A1 chip,
                     *     LQFP128 : T31 (GPIO22) I2C_SCL & T32 (GPIO23) I2C_SDA
                     *     QFN88:    L10 (GPIO11) I2C_SCL
                     *                L8 (GPIO9)  I2C_SDA for WIFI ADC/DAC/SDIO/DDR
                     *                L2 (GPIO2)  I2C_SDA for Analog/EPHY/USB PHY/DDR
                     * using GPIO9 to determine which GPIO9/2 is SDA
                     */
                    i2cs_get_grant();
//                    GPREG(SDA_GPSEL) |= GPIO_I2C_SDA;  // select sda gpio pin
                    GPREG(SDA_GPDIR) &= ~GPIO_I2C_SDA;  // as sda input pin
                    if (!SDA_DATA())
                    {
                        // SDA is GPIO2
                        mi2c.sda_gpval = (2 / 32) ? GP2VAL : GPVAL;
                        mi2c.sda = 2;
                        mi2c.disable = (EN_PWM_FNC | EN_SDIO_FNC | EN_ETH0_FNC);
                        /* disable SD clock */
                        ANAREG(CLKEN) &= ~(SD_CLK_EN);
                        printf("QFN88 disable SDHC!!\n");
                    }
                    else
                    {
                        // SDA is GPIO9
                        mi2c.sda_gpval = (9 / 32) ? GP2VAL : GPVAL;
                        mi2c.sda = 9;
                        mi2c.disable =
                            (EN_PWM_FNC | EN_ADC_OUT_FNC | EN_LED_FNC);
                    }
                    i2cs_put_grant();
                }
            }
            break;
        }
    }
    if (i == MPIN_NUM)
        printf("Error: can't identify chip id\n");
}

void i2cs_init(void)
{
    mi2c_init();
    i2cs_get_grant();

//    GPREG(SDA_GPSEL) |= GPIO_I2C_SDA;  // select sda gpio pin
//    GPREG(SCL_GPSEL) |= GPIO_I2C_SCL;  // select scl gpio pin
    GPREG(SDA_GPDIR) &= ~GPIO_I2C_SDA;  // as sda input pin
    GPREG(SCL_GPDIR) &= ~GPIO_I2C_SCL;  // as scl input pin

#ifdef I2CS_MEASURE_BOOT_TIME
//    GPREG(GPSEL) |= (1 << GPIO_NUM_BOOT_TIME);
    GPREG(GPVAL) &= (~(1 << GPIO_NUM_BOOT_TIME));
    GPREG(GPDIR) |= (1 << GPIO_NUM_BOOT_TIME);
    mi2c.disable |= (EN_ADC_OUT_FNC);
#endif
}

void i2cs_finish(void)
{
//    GPREG(SDA_GPSEL) &= ~GPIO_I2C_SDA; // disable sda gpio pin
//    GPREG(SCL_GPSEL) &= ~GPIO_I2C_SCL; // disable scl gpio pin

    i2cs_put_grant();
}

void sdhc_io_read(void)
{
// GPIO 24~29 0~5
#define GPIO_NUMS_SDHC_IO_TEST 0x3F00007F
#define GPIO_NUMS_SDHC_IO_R_TEST GPIO_NUMS_SDHC_IO_TEST
#define GPIO_NUMS_SDHC_IO_W_TEST 0x3F000037

    // disable PINMUX for SDHC and ETH0 and ADC and MDIO, until sdhc_io_stop()
#ifdef CONFIG_SDHC
    mmc_disable_interrupt();
#endif
    GPREG(PINMUX) &=
        ~(EN_SDIO_FNC | EN_ETH0_FNC | EN_MDIO_FNC | EN_ADC_OUT_FNC);

//    GPREG(GPSEL) |= (GPIO_NUMS_SDHC_IO_TEST);
    GPREG(GPDIR) &= ~(GPIO_NUMS_SDHC_IO_R_TEST);
/*
    * The tester can read GPIO registers to get the GPIO input data.
    * The tester stop the read test with Stop command.
*/
}

void sdhc_io_write(void)
{
    int scl, sda;
    int sclo = 0;
    int cnt = 0;

    sdhc_io_read();
    GPREG(GPDIR) |= (GPIO_NUMS_SDHC_IO_W_TEST);
/*
    * The tester generate a 50MHz clock on GOIO3 (OL2).
    * F/W detects GPIO6's value (1/0). Then write the same value on other SDIO interface.
    * F/W repeats 20 clock cycles . F/W stops output data on SDIO.
    * F/W waits for SDIO stop command
*/
    do
    {
        scl = GPREG(GPVAL);
        sda = scl & (1 << 6);
        scl = scl & (1 << 3);

        if (!sclo && scl)
        {
            if (sda)
                GPREG(GPSET) = GPIO_NUMS_SDHC_IO_W_TEST;
            else
                GPREG(GPCLR) = GPIO_NUMS_SDHC_IO_W_TEST;
            cnt++;
        }

        sclo = scl;
    }
    while (cnt < 20);
}

void sdhc_io_stop(void)
{
    GPREG(PINMUX) = restore_i2cs_test_pinmux;
#ifdef CONFIG_SDHC
    mmc_enable_interrupt();
#endif
}

void mem_test_prepare(void)
{
#define GPIO_NUM_MEMORY_TEST 27
    // T41:GPIO27 output to HIGH
//    GPREG(GPSEL) |= (1 << GPIO_NUM_MEMORY_TEST);
    GPREG(GPVAL) |= (1 << GPIO_NUM_MEMORY_TEST);
    GPREG(GPDIR) |= (1 << GPIO_NUM_MEMORY_TEST);
    // disable PINMUX for SDHC and ETH0, until I2C action next time
    restore_mem_test_pinmux = GPREG(PINMUX);
#ifdef CONFIG_SDHC
    mmc_disable_interrupt();
#endif
    GPREG(PINMUX) &= ~(EN_SDIO_FNC | EN_ETH0_FNC);
}

/*
 * The memory test methodology
 * 1.  Address A0 - A12 must be toggled.  Walking 0 and Walking 1.
 * For example:  0000_0000_0000 >>> 0000_0000_0001 >>>  0000_0000_0010 ...
 * 2. BS0, BS1 (Bank Select) must be toggled.  Walking 0 and Walking 1.
 * 3. DQ0 - QD15 must be toggled.  Walking 0 and Walking 1.
 * 4. Using R/W commands
 */
int mem_test_go(void)
{
    u8 *addr8;
    u16 *addr16;
    u32 readback;
    u32 mt_start = 0xa0000000;
    u32 mt_end = 0xa2000000;
    u32 mt_offset = 0;
    u32 start = mt_start;
    u32 pattern[4] = { 0x00000000, 0x55555555, 0xaaaaaaaa, 0xffffffff };
    u32 c;
    u32 i;

    printf("mt start\n");

    do
    {
        for (c = 0; c < 10; c++)
        {
            if (start & 1)
            {
                addr8 = (u8 *) start;
                for (i = 0; i < sizeof (pattern) / sizeof (u32); i++)
                {
                    memcpy(addr8, &pattern[i], 4);
                    memcpy(&readback, addr8, 4);
                    if (readback != pattern[i])
                        return -1;
                }
            }
            else
            {
                addr16 = (u16 *) start;
                for (i = 0; i < sizeof (pattern) / sizeof (u32); i++)
                {
                    *addr16 = (pattern[i] >> 16);
                    *(addr16 + 1) = pattern[i];
                    readback = (*addr16 << 16) | (*(addr16 + 1));
                    if (readback != pattern[i])
                        return -1;
                }
            }
        }

        if (!mt_offset)
            mt_offset = 1;
        else
            mt_offset <<= 1;

        start = (mt_start | mt_offset);
    }
    while (start < mt_end);

    printf("mt done\n");
    return 0;
}

void mem_test_done(void)
{
    trap_init();
    GPREG(GPVAL) &= ~(1 << GPIO_NUM_MEMORY_TEST);
}

void mem_test_restore(void)
{
    GPREG(PINMUX) = restore_mem_test_pinmux;
    restore_mem_test_pinmux = 0;
}

int i2cs_phy_int(unsigned char p, unsigned int phyid)
{
    unsigned int id;
    unsigned short id1, id2;

    if (!phy_tx_test)
        return 0;

    id1 = cm_mdio_rd(p, 2);
    id2 = cm_mdio_rd(p, 3);
    id = (id1 << 16) | id2;
    if (id == phyid)
    {
        /* Swap to page 1 */
        cm_mdio_wr(p, 31, 1);
        cm_mdio_wr(p, 16, 0xb7e0);
        /* AFE tx control: enable termination impedance calibration */
        cm_mdio_wr(p, 17, 0xa528);
#if (CONFIG_REF_CLK==25000000)
        cm_mdio_wr(p, 18, (cm_mdio_rd(p, 18) & 0x07ff));
        cm_mdio_wr(p, 18, (cm_mdio_rd(p, 18) | 0x1800));
#endif
        cm_mdio_wr(p, 19, 0xa4d8);
        /* AFE rx control */
        cm_mdio_wr(p, 20, 0x3780);
        /* ADC VCM = 1.00 */
        cm_mdio_wr(p, 22, (cm_mdio_rd(p, 22) | 0x6000));
#if (CONFIG_REF_CLK==25000000)
        cm_mdio_wr(p, 23, (cm_mdio_rd(p, 23) & 0xe01f));
        cm_mdio_wr(p, 23, (cm_mdio_rd(p, 23) | 0x1500));
#endif
        /* Swap to page 2 */
        cm_mdio_wr(p, 31, 2);
        /* AGC thresholds, org=0x4030 */
        cm_mdio_wr(p, 17, 0x8059);
        /* DSP initial val */
        cm_mdio_wr(p, 18, 0x8975);
        cm_mdio_wr(p, 19, 0xba60);
        /* Swap to page 4 */
        cm_mdio_wr(p, 31, 4);
        /* force 10M FULL */
        cm_mdio_wr(p, 18, 0x5a54);
        /* RJ45 loopback */
        cm_mdio_wr(p, 16, 0x0001);

        /* Swap to page 0 */
        cm_mdio_wr(p, 31, 0);
        /* RMII V1.2 */
        cm_mdio_wr(p, 19, (cm_mdio_rd(p, 19) | 0x0040));
        /* Enable MDIX */
        cm_mdio_wr(p, 20, (cm_mdio_rd(p, 20) | 0x3000));
    }
    else
    {
        printf("Unknow phy id:%x\n", id);
        return 0;
    }
    /* Fallback to 100M, Full duplex mode */
    cm_mdio_wr(p, 4, 0x05e1);
    /* reset / disable auto-negotiation */
    cm_mdio_wr(p, 0, 0x8100);

    /* Delay for reset */
    mdelay(2000);

    return 1;
}

int i2cs_phy_status(short phyno)
{
    if (phy_tx_test)
        return 1;
    else
        return 0;
}

void reinit_eth(int for_test)
{
    eth_disable();
    memset(&nbuf_cfg, 0, sizeof (struct nbuf_cfg));
    nbuf_init((void *) (((unsigned int) netbuf) | 0xa0000000), CONFIG_NETBUF_SZ,
              CONFIG_NETBUF_NUM);

    if (for_test)
        phy_tx_test = 1;
    else
        phy_tx_test = 0;
    phy_tx_test_ready = 1;

    eth_probe();
}

void phy_reset(void)
{
    printf("%s\n", __func__);
    reinit_eth(0);
}

void phy_reset_loopback(void)
{
    printf("%s\n", __func__);
    reinit_eth(1);
}

int phy_rand_tx(void)
{
    unsigned long len = 1500, val = 0, loop = 0xFFFFFFFF;
    short mode = 2;
    cmdev *cmd = &g_cmdev;
    printf("%s\n", __func__);
    if (!phy_tx_test_ready)
        return -1;
    cm_tx_test(cmd, len, val, loop, mode);
    return 0;
}

int phy_test_tx(void)
{
    unsigned long len = 1500, val = 0xff, loop = 0xFFFFFFFF;
    short mode = 0;
    cmdev *cmd = &g_cmdev;
    printf("%s\n", __func__);
    if (!phy_tx_test_ready)
        return -1;
    cm_tx_test(cmd, len, val, loop, mode);
    return 0;
}

void phy_lut14(void)
{
    int i;
    printf("%s\n", __func__);
    for (i = 0; i < PHY_LUT14_REG_NUM; i++)
        cm_mdio_wr(1, phy_lut14_reg[i].offset, phy_lut14_reg[i].value);
}

void phy_lut15(void)
{
    int i;
    printf("%s\n", __func__);
    for (i = 0; i < PHY_LUT15_REG_NUM; i++)
        cm_mdio_wr(1, phy_lut15_reg[i].offset, phy_lut15_reg[i].value);
}

void phy_lutmau(void)
{
    int i;
    printf("%s\n", __func__);
    for (i = 0; i < PHY_LUTMAU_REG_NUM; i++)
        cm_mdio_wr(1, phy_lutmau_reg[i].offset, phy_lutmau_reg[i].value);
}

int auto_cmd(void)
{
    u8 *buf = (u8 *) uncached_addr(buf_address);
    u8 cmd = auto_i2c_reg & 0xFF;
    u8 arg1 = (auto_i2c_reg >> 8) & 0xFF;
    u8 arg2 = (auto_i2c_reg >> 16) & 0xFF;
    u8 arg3 = (auto_i2c_reg >> 24) & 0xFF;
    char *argv[3];
    char argv1[10] = { 0 }, argv2[10] =
    {
    0}, argv3[10] =
    {
    0};
    int ret = ERR_OK;
    int i, j;

    argv[0] = argv1;
    argv[1] = argv2;
    argv[2] = argv3;

#ifdef CONFIG_SDHC
    switch (cmd)
    {
        case AUTO_CMD_SDHC_READ:
        case AUTO_CMD_SDHC_WRITE:
            if (DIS_GPIO_I2C_FNC & EN_SDIO_FNC)
                return ERR_PARM;
        default:
            break;
    }
#endif
    switch (cmd)
    {
        case AUTO_CMD_MT:
            // remember to re-enable SDHC and EPHY
            mem_test_prepare();
            ret = mem_test_go();
            mem_test_done();
            break;
        case AUTO_CMD_PHY_RESET:
            if (arg1 == 0)
                phy_reset_loopback();
            if (arg1 == 1)
                phy_reset();
            break;
        case AUTO_CMD_PHY_RAND_TX:
            ret = phy_rand_tx();
            break;
        case AUTO_CMD_PHY_TEST_TX:
            ret = phy_test_tx();
            break;
        case AUTO_CMD_PHY_LUT14:
            phy_lut14();
            break;
        case AUTO_CMD_PHY_LUT15:
            phy_lut15();
            break;
        case AUTO_CMD_PHY_LUTMAU:
            phy_lutmau();
            break;
#ifdef CONFIG_SDHC
        case AUTO_CMD_SDHC_DISABLE:
            mmc_disable_interrupt();
            break;
        case AUTO_CMD_SDHC_ENABLE:
            mmc_enable_interrupt();
            break;
        case AUTO_CMD_SDHC_IO_READ:
            sdhc_io_read();
            break;
        case AUTO_CMD_SDHC_IO_WRITE:
            sdhc_io_write();
            break;
        case AUTO_CMD_SDHC_IO_STOP:
            sdhc_io_stop();
            break;
        case AUTO_CMD_SDHC_READ:
            argv1[0] = 'r';
            sprintf(argv2, "%d", ((arg1) ? arg1 : 0));
            sprintf(argv3, "%d", ((arg2) ? arg2 : 1));
            if ((ret = mmc_cmd(3, argv)) == ERR_OK)
            {
                for (i = ((arg2) ? arg2 : 1), j = 0; i > 0; i--, j++)
                {
                    sprintf(argv1, "dw");
                    sprintf(argv2, "0x%x", buf + j * 512);
                    sprintf(argv3, "0x%x", 512);
                    mem_dump_cmd(2, argv + 1);
                }
                buf_address = (unsigned long) buf;
            }
            break;
        case AUTO_CMD_SDHC_WRITE:
            j = ((arg2) ? arg2 : 1);
            for (i = 0; i < j * 512; i += 2)
            {
                if (arg3 == 0)
                {
                    buf[i] = (unsigned char) rand();
                    buf[i + 1] = (unsigned char) rand();
                }
                else
                {
                    buf[i] = 0x55;
                    buf[i + 1] = 0xAA;
                }
            }
            argv1[0] = 'w';
            sprintf(argv2, "%d", ((arg1) ? arg1 : 0));
            sprintf(argv3, "%d", ((arg2) ? arg2 : 1));
            ret = mmc_cmd(3, argv);
            break;
#endif
        case AUTO_CMD_STOP:
            i2c_run = 0;
            break;
        default:
            break;
    }
    return ret;
}

void do_write(unsigned char dev)
{
    volatile unsigned int *reg = (unsigned int *) i2cs_devs[dev].reg_base;
    unsigned char ofs = hw_i2c_regs[HW_I2C_R00_OFS] >> 2;
    unsigned int val = 0;
    int i;

    for (i = HW_I2C_R04_DAT; i >= HW_I2C_R01_DAT; i--)
    {
        val <<= 8;
        val |= hw_i2c_regs[i];
    }

    if (dev_id == I2CS_DEV_ETH_PHY)
    {
        cm_mdio_wr(1, hw_i2c_regs[HW_I2C_R00_OFS], val);
#ifdef I2CS_DEBUG
        printf("WRITE ETH PHY 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
        val = cm_mdio_rd(1, hw_i2c_regs[HW_I2C_R00_OFS]);
        printf("READ BACK ETH PHY 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS],
               val);
#endif
    }
    else if (dev_id == I2CS_DEV_BB)
    {
        bb_register_write(hw_i2c_regs[HW_I2C_R00_OFS], val);
#ifdef I2CS_DEBUG
        printf("WRITE BB 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
        val = bb_register_read(hw_i2c_regs[HW_I2C_R00_OFS]);
        printf("READ BACK BB 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
    }
    else if (dev_id == I2CS_DEV_AUTO)
    {
#ifdef I2CS_DEBUG
        printf("WRITE AUTO 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
        if (hw_i2c_regs[HW_I2C_R00_OFS] == 0)
        {
            auto_i2c_reg = val;
            if ((auto_i2c_result = auto_cmd()) != ERR_OK)
                printf("AUTO ERROR\n");
            else
                printf("AUTO COMPLETE\n");
        }
    }
    else
    {
        reg[ofs] = val;
#ifdef I2CS_DEBUG
        printf("WRITE 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
        val = reg[ofs];
        printf("READ BACK 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
    }
}

void do_read(unsigned char dev)
{
    volatile unsigned int *reg = (unsigned int *) i2cs_devs[dev].reg_base;
    unsigned char ofs = hw_i2c_regs[HW_I2C_R00_OFS] >> 2;
    unsigned int val = 0;
    int i;

    if (dev_id == I2CS_DEV_ETH_PHY)
    {
        val = cm_mdio_rd(1, hw_i2c_regs[HW_I2C_R00_OFS]);
#ifdef I2CS_DEBUG
        printf("READ ETH PHY 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
    }
    else if (dev_id == I2CS_DEV_BB)
    {
        val = bb_register_read(hw_i2c_regs[HW_I2C_R00_OFS]);
#ifdef I2CS_DEBUG
        printf("READ BB 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
    }
    else if (dev_id == I2CS_DEV_AUTO)
    {
        if (hw_i2c_regs[HW_I2C_R00_OFS] == 0)
        {
            val = auto_i2c_reg;
        }
        else if (hw_i2c_regs[HW_I2C_R00_OFS] == 1)
        {
            val = ((auto_i2c_result == ERR_OK) ? 1 : 0);
        }
#ifdef I2CS_DEBUG
        printf("READ AUTO 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
    }
    else
    {
        val = reg[ofs];
#ifdef I2CS_DEBUG
        printf("READ 0x%x = 0x%x\n", hw_i2c_regs[HW_I2C_R00_OFS], val);
#endif
    }

    for (i = HW_I2C_R01_DAT; i <= HW_I2C_R04_DAT; i++)
    {
        hw_i2c_regs[i] = val & 0xff;
        val >>= 8;
    }
}

void do_cmd(unsigned char dev)
{
    if (--dev >= I2CS_DEV_NUM)
        return;
    switch (hw_i2c_regs[HW_I2C_R07_CMD])
    {
        case HW_I2C_CMD_WRITE:
            do_write(dev);
            break;
        case HW_I2C_CMD_READ:
            do_read(dev);
            break;
        default:
            break;
    }
}

unsigned char i2cs_device_address(unsigned char addr)
{
    unsigned char dev = (addr >> 1);
    int i;
    /* check our device address tables */
    for (i = 0; i < I2CS_DEV_NUM; i++)
    {
        if (dev == i2cs_devs[i].dev_id)
        {
            dev_id = i2cs_devs[i].dev_id;
            return (i + 1);
        }
    }
    return 0;
}

int i2cs_fetch(int *pscl, int *psda, int *psclo, int *psdao)
{
    int act = I2CS_ACT_IDLE;
    int scl = *pscl, sda = *psda;
    int sclo = *psclo, sdao = *psdao;

    scl = SCL_DATA();
    sda = SDA_DATA();
    if (sclo && scl)
    {
        if (sdao && !sda)
            act = I2CS_ACT_START;
        else if (!sdao && sda)
            act = I2CS_ACT_STOP;
    }
    else if (!sclo && scl)
    {
        act = I2CS_ACT_RISING;
    }
    else if (sclo && !scl)
    {
        act = I2CS_ACT_FALLING;
    }
    sclo = scl;
    sdao = sda;

    *pscl = scl;
    *psda = sda;
    *psclo = sclo;
    *psdao = sdao;
    return act;
}

void i2cs_main_loop(void)
{
    int state = I2CS_STATE_IDLE;
    int next_state = I2CS_STATE_IDLE;
    int act = I2CS_ACT_IDLE;
    int scl = 1, sda = 1;
    int sclo = 1, sdao = 1;
    int rdy = 0;
    int cmd = 0;
    unsigned int byte = 0;
    unsigned char dev = 0;
    i2cs_init();

#ifdef I2CS_MEASURE_BOOT_TIME
    GPREG(GPVAL) |= (1 << GPIO_NUM_BOOT_TIME);
#endif

    i2c_run = 1;
    while (i2c_run)
    {
        if ((act = i2cs_fetch(&scl, &sda, &sclo, &sdao)) == I2CS_ACT_IDLE)
            continue;
        switch (state)
        {
            case I2CS_STATE_IDLE:
                if (act == I2CS_ACT_START)
                {
                    if (restore_mem_test_pinmux)
                    {
                        mem_test_restore();
                    }

                    I2CS_DATA_INIT(byte, 0);
                    state = I2CS_STATE_SLAVE;
                }
                break;
            case I2CS_STATE_SLAVE:
                if (act == I2CS_ACT_FALLING)
                    break;
                if (act == I2CS_ACT_RISING)
                {
                    byte <<= 1;
                    byte |= sda;
                    if (I2CS_DATA_DONE(byte))
                    {
                        if ((dev = i2cs_device_address((unsigned char) byte)))
                        {
                            state = I2CS_STATE_ACK;
                            if (dev_id == I2CS_DEV_CHEETAH)
                            {
                                if (byte & 0x01)
                                {
                                    if (hw_i2c_addr32.dword >= 0x80000000)
                                    {
                                        hw_i2c_value32.dword =
                                            *((volatile unsigned long *)
                                              hw_i2c_addr32.dword);
                                    }
                                    else
                                    {
                                        hw_i2c_value32.dword = 0;
                                    }
                                    hw_i2c_vindex32 = 0;
                                    I2CS_DATA_INIT(byte,
                                                   hw_i2c_value32.byte[(sizeof
                                                                        (hw_i2c_value32)
                                                                        - 1) -
                                                                       hw_i2c_vindex32++]);
                                    next_state = I2CS_STATE_PDA;
                                }
                                else
                                {
                                    hw_i2c_aindex32 = 0;
                                    I2CS_DATA_INIT(byte, 0);
                                    next_state = I2CS_STATE_GMA;
                                }
                            }
                            else
                            {
                                if (byte & 0x01)
                                {
                                    I2CS_DATA_INIT(byte,
                                                   hw_i2c_regs[hw_i2c_index++]);
                                    next_state = I2CS_STATE_PDA;
                                }
                                else
                                {
                                    I2CS_DATA_INIT(byte, 0);
                                    next_state = I2CS_STATE_GMA;
                                }
                            }
                        }
                        else
                            goto error;
                    }
                    break;
                }
                goto error;
            case I2CS_STATE_GMA:
                if (act == I2CS_ACT_FALLING)
                    break;
                if (act == I2CS_ACT_RISING)
                {
                    byte <<= 1;
                    byte |= sda;
                    if (I2CS_DATA_DONE(byte))
                    {
                        byte = (unsigned char) byte;
                        if (dev_id == I2CS_DEV_CHEETAH)
                        {
                            hw_i2c_addr32.byte[hw_i2c_aindex32++] = byte;
                            state = I2CS_STATE_ACK;
                            I2CS_DATA_INIT(byte, 0);
                            if (hw_i2c_aindex32 < sizeof (hw_i2c_addr32))
                            {
                                next_state = I2CS_STATE_GMA;
                            }
                            else
                            {
                                hw_i2c_vindex32 = 0;
                                next_state = I2CS_STATE_GDA;
                            }
                        }
                        else
                        {
                            if (byte < sizeof (hw_i2c_regs))
                            {
                                hw_i2c_index = byte;
                                state = I2CS_STATE_ACK;
                                I2CS_DATA_INIT(byte, 0);
                                next_state = I2CS_STATE_GDA;
                            }
                            else
                            {
                                state = I2CS_STATE_NACK;
                                next_state = I2CS_STATE_IDLE;
                            }
                        }
                    }
                    break;
                }
                goto error;
            case I2CS_STATE_GDA:
                if (act == I2CS_ACT_START)
                {
                    state = I2CS_STATE_SLAVE;
                    break;
                }
                if (act == I2CS_ACT_FALLING)
                    break;
                if (act == I2CS_ACT_RISING)
                {
                    byte <<= 1;
                    byte |= sda;
                    if (I2CS_DATA_DONE(byte))
                    {
                        byte = (unsigned char) byte;
                        if (dev_id == I2CS_DEV_CHEETAH)
                        {
                            hw_i2c_value32.byte[(sizeof (hw_i2c_value32) - 1) -
                                                hw_i2c_vindex32++] = byte;
                            state = I2CS_STATE_ACK;
                            I2CS_DATA_INIT(byte, 0);
                            if (hw_i2c_vindex32 < sizeof (hw_i2c_value32))
                            {
                                next_state = I2CS_STATE_GDA;
                            }
                            else
                            {
                                if (hw_i2c_addr32.dword >= 0x80000000)
                                {
                                    *((volatile unsigned long *)
                                      hw_i2c_addr32.dword) =
             hw_i2c_value32.dword;
                                }
                                next_state = I2CS_STATE_IDLE;
                            }
                        }
                        else
                        {
                            hw_i2c_regs[hw_i2c_index] = byte;
                            if (hw_i2c_index == HW_I2C_R07_CMD)
                                cmd = 1;
                            state = I2CS_STATE_ACK;
                            next_state = I2CS_STATE_IDLE;
                        }
                    }
                    break;
                }
                goto error;
            case I2CS_STATE_PDA:
                if (act == I2CS_ACT_FALLING)
                {
                    if (I2CS_DATA_DONE(byte))
                    {
                        state = I2CS_STATE_PDA_ACK;
                        SDA_IN();
                    }
                    else
                    {
                        if (byte & BYTE_MSB)
                            SDA_HIGH();
                        else
                            SDA_LOW();
                        SDA_OUT();
                    }
                    break;
                }
                if (act == I2CS_ACT_RISING)
                {
                    byte <<= 1;
                    break;
                }
                goto error;
            case I2CS_STATE_PDA_ACK:
                if (act == I2CS_ACT_FALLING)
                    break;
                if (act == I2CS_ACT_RISING)
                {
                    if (!sda)
                    {
                        if (dev_id == I2CS_DEV_CHEETAH)
                        {
                            if (hw_i2c_vindex32 < sizeof (hw_i2c_value32))
                            {
                                I2CS_DATA_INIT(byte,
                                               hw_i2c_value32.byte[(sizeof
                                                                    (hw_i2c_value32)
                                                                    - 1) -
                                                                   hw_i2c_vindex32++]);
                                state = I2CS_STATE_PDA;
                                break;
                            }
                        }
                        else
                        {
                            if (hw_i2c_index < sizeof (hw_i2c_regs))
                            {
                                I2CS_DATA_INIT(byte,
                                               hw_i2c_regs[hw_i2c_index++]);
                                state = I2CS_STATE_PDA;
                                break;
                            }
                        }
                    }
#ifdef I2CS_DEBUG
                    else
                    {
                        if (dev_id == I2CS_DEV_CHEETAH)
                        {
                            if (hw_i2c_addr32.dword >= 0x80000000)
                                printf("READ 0x%x = 0x%x\n",
                                       hw_i2c_addr32.dword,
                                       hw_i2c_value32.dword);
                            else
                                printf("READ Invalid addr 0x%x\n",
                                       hw_i2c_addr32.dword);
                        }
                    }
#endif
                }
                goto error;
            case I2CS_STATE_ACK:
            case I2CS_STATE_NACK:
                if (act == I2CS_ACT_FALLING)
                {
                    if (rdy)
                    {
                        if (next_state == I2CS_STATE_PDA)
                        {
                            if (byte & BYTE_MSB)
                                SDA_HIGH();
                            else
                                SDA_LOW();
                            SDA_OUT();
                        }
                        else
                        {
                            SDA_IN();
                            if (state == I2CS_STATE_ACK)
                            {
                                if (cmd)
                                    do_cmd(dev);
#ifdef I2CS_DEBUG
                                if ((dev_id == I2CS_DEV_CHEETAH)
                                    && (next_state == I2CS_STATE_IDLE))
                                {
                                    if (hw_i2c_addr32.dword >= 0x80000000)
                                    {
                                        printf("WRITE 0x%x = 0x%x\n",
                                               hw_i2c_addr32.dword,
                                               hw_i2c_value32.dword);
                                        printf("READ BACK 0x%x = 0x%x\n",
                                               hw_i2c_addr32.dword,
                                               *((volatile unsigned long *)
                                                 hw_i2c_addr32.dword));
                                    }
                                    else
                                    {
                                        printf("WRITE Invalid addr 0x%x\n",
                                               hw_i2c_addr32.dword);
                                    }
                                }
#endif
                            }
                        }
                        cmd = 0;
                        rdy = 0;
                        state = next_state;
                    }
                    else
                    {
                        if (state == I2CS_STATE_ACK)
                            SDA_LOW();
                        else
                            SDA_HIGH();
                        SDA_OUT();
                    }
                    break;
                }
                if (act == I2CS_ACT_RISING)
                {
                    rdy = 1;
                    break;
                }
                goto error;
            default:
              error:
                SDA_IN();
                state = I2CS_STATE_IDLE;
                break;
        }
    }

    i2cs_finish();
}

int i2cs_cmd_go(int wait)
{
    unsigned int time;
    char c = 0;

    printf("Press 'B' or 'b' to break!!\nOtherwise Enter I2C slave mode.\n");
    time = clock_get();
    do
    {
        if (tstc())
            c = getchar();
        if (c == 'B' || c == 'b')
            break;
    }
    while (how_long(time) < wait);

    if (c != 'B' && c != 'b')
    {
        printf("Enter I2C slave mode...\n");
        i2cs_main_loop();
    }
    else
    {
        printf("Re-initialize eth!!");
        reinit_eth(0);
    }

    return ERR_OK;
}

int i2cs_cmd(int argc, char *argv[])
{
    return i2cs_cmd_go(2000);
}

cmdt cmdt_i2cs[] __attribute__ ((section("cmdt"))) =
{
    {
    "i2cs", i2cs_cmd, ""}
,};
#endif
