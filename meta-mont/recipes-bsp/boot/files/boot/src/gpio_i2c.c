/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file gpio_i2c.c
*   \brief I2C master by gpio
*   \author Montage
*/
#ifdef CONFIG_GPIO_I2C
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
#include <udc.h>
#include <lib.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define BYTE_MSB 0x80

#define i2c_log(fmt, args...) printf(fmt, ##args)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#define I2C_PRERLO      (0x00)
#define I2C_PRERHI      (0x04)
#define I2C_CTR         (0x08)
#define I2C_TX          (0x0c)
#define I2C_RX          (0x0c)
#define I2C_CMD         (0x10)

#define     I2C_CMD_START       0x80
#define     I2C_CMD_STOP        0x40
#define     I2C_CMD_READ        0x20
#define     I2C_CMD_WRITE       0x10
#define     I2C_CMD_ACK         0x08
#define     I2C_CMD_IACK        0x01     /* interrupt acknowledge */

#define I2C_STA         (0x10)

#define REG0_ADDR 0x00
#define REG1_ADDR 0x01
#define REG2_ADDR 0x02
#define REG3_ADDR 0x03
#define REG4_ADDR 0x04
#define REG5_ADDR 0x05
#define REG6_ADDR 0x06
#define REG7_ADDR 0x07

#define RF_READ_CMD     0x0D
#define RF_WRITE_CMD    0x0B

#define I2C_RF_DEV_ADDR 0x34
#define I2C_START_READ  1
#define I2C_START_WRITE 0
#define I2C_STOP        1
#define I2C_NACK        BIT7
#define I2C_RETRY_CNT   10

#define I2CREG_READ32(x)    (*(volatile u32 *) (I2C_BASE + (x)))

#define I2CREG_WRITE32(x,val)	do { \
    i2c_log("[I2CREG_WRITE32] %x, %x\n", x, (u32)val); \
    (*(volatile u32 *)(I2C_BASE + (x)) = (u32)(val)); \
} while(0)


static int i2c_already_init = 0;
int wait_ack()
{
    //i2c_log("%s\n", __func__);
    u32 state;
    u8 wait_ack = 1;
    while (wait_ack)
    {
        state = I2CREG(I2C_STA);
        wait_ack = (state & BIT1);
    }

    i2c_log("wait_ack ");
    if (state & BIT7)
    {
        i2c_log("no ");
    }
    i2c_log("ack\n");

    return state;
}

void i2c_init(void)
{
    u32 prescale = 19;//(clk / (5 * speed)) - 1;

    I2CREG(I2C_CTR) = 0x00;     // turn off the core
    I2CREG(I2C_CMD) = 0x01;     // clear pending interrupt
    I2CREG(I2C_PRERLO) = (prescale & 0x000000FFUL);
    I2CREG(I2C_PRERHI) = ((prescale & 0x0000FF00UL) >> 8);

    I2CREG(I2C_CTR) = 0x00000080UL;     // master, enable i2c core
    printf("I2C_CTR %x\n", I2CREG(I2C_CTR));
}

void i2c_finish(void)
{
    I2CREG(I2C_CTR) = 0x00;     // turn off the core
    I2CREG(I2C_CMD) = 0x01;     // clear pending interrupt
}

int i2c_start(u8 addr, u8 is_read)
{
    //i2c_log("%s\n", __func__);
    I2CREG_WRITE32(I2C_TX, (((addr << 1) & 0xfe) + is_read) & 0x000000ffUL);
    i2c_log("i2c_start\n");
    I2CREG_WRITE32(I2C_CMD, (I2C_CMD_START | I2C_CMD_WRITE));
    return (wait_ack() & I2C_NACK);
}

#if 0
void i2c_stop(void)
{
    i2c_log("%s\n", __func__);
    I2CREG(I2C_CMD) = 0x00000040UL;
    wait_ack();
}

#endif

int i2c_write(u8 reg, u8 is_stop)
{
    i2c_log("%s %x\n", __func__, reg);
    I2CREG_WRITE32(I2C_TX, (reg & 0x000000ffUL));
    if (1 == is_stop) {
        i2c_log("stop\n");
        I2CREG_WRITE32(I2C_CMD, (I2C_CMD_STOP | I2C_CMD_WRITE));
    }
    else {
        I2CREG_WRITE32(I2C_CMD, I2C_CMD_WRITE);
    }

    return (wait_ack() & I2C_NACK);
}

void i2c_read(unsigned int *out, u8 reg, u8 is_stop)
{
    i2c_log("%s %x\n", __func__, (reg & 0x000000ffUL));

    I2CREG_WRITE32(I2C_CMD, I2C_CMD_READ);
    
    if (wait_ack() & BIT7)
        return;

    *out = I2CREG_READ32(I2C_RX);
    i2c_log("data %x\n", *out);

    if (1 == is_stop) {
        i2c_log("stop\n");
        I2CREG_WRITE32(I2C_CMD, (I2C_CMD_READ | I2C_CMD_STOP | I2C_CMD_ACK));
    }
    
    return;
}

void i2c_write_reg(u8 reg, u8 data)
{
#if 0
    int retry_cnt;
    for (retry_cnt = 0; I2C_RETRY_CNT > retry_cnt; retry_cnt++)
    {
        if (i2c_start(I2C_RF_DEV_ADDR, I2C_START_WRITE))
            continue;
        if (i2c_write(reg, !I2C_STOP))
            continue;
        if (i2c_write(data, I2C_STOP))
            continue;
        break;
    }
    i2c_log("%s reg(%x):%x for %d times retry\n", __func__, reg, data,
            retry_cnt);
#else
    i2c_start(I2C_RF_DEV_ADDR, I2C_START_WRITE);
    i2c_write(reg, !I2C_STOP);
    i2c_write(data, I2C_STOP);
    i2c_log("%s reg(%x):%x\n", __func__, reg, data);
#endif

    return;
}

void i2c_read_reg(u8 reg, u8 * data)
{
#if 0
    int rc = 0, retry_cnt;
    for (retry_cnt = 0; I2C_RETRY_CNT > retry_cnt; retry_cnt++)
    {
        if (i2c_start(I2C_RF_DEV_ADDR, I2C_START_WRITE))
            continue;
        if (i2c_write(reg, I2C_STOP))
            continue;

        if (i2c_start(I2C_RF_DEV_ADDR, I2C_START_READ))
            continue;
        i2c_read(&rc, I2C_STOP);
        break;
    }
    *data = rc & 0xffUL;
    i2c_log("%s reg(%x):%x for %d times retry\n", __func__, reg, *data,
            retry_cnt);
#else
    unsigned int rc = 0;
    i2c_start(I2C_RF_DEV_ADDR, I2C_START_WRITE);
    i2c_write(reg, 0);

    i2c_start(I2C_RF_DEV_ADDR, I2C_START_READ);
    i2c_read(&rc, reg, I2C_STOP);
    *data = rc & 0xffUL;
    i2c_log("%s reg(%x):%x\n", __func__, reg, *data);
#endif
    return;
}

void i2c_aon_send_reg(unsigned long addr)
{
    int reg0_data, reg5_data, reg6_data;

    reg0_data = addr & 0xff;
    reg5_data = (addr >> 8) & 0xff;
    reg6_data = (addr >> 16) & 0xff;
    i2c_log("addr 0x%02x%02x%02x\n", reg6_data, reg5_data, reg0_data);

    i2c_write_reg(REG0_ADDR, reg0_data);
    i2c_write_reg(REG5_ADDR, reg5_data);
    i2c_write_reg(REG6_ADDR, reg6_data);

    return;
}

void i2c_aon_send_cmd(unsigned long cmd)
{
    u8 rx_data = 0;

    i2c_write_reg(REG7_ADDR, (cmd & 0xffUL));
    i2c_write_reg(REG7_ADDR, 0x00);
    i2c_read_reg(REG7_ADDR, &rx_data);

    i2c_log("read %02x\n", rx_data);
    return;
}

unsigned long i2c_aon_read_data(void)
{
    unsigned long rx_data = 0;
    u8 rx_byte;

    i2c_read_reg(REG1_ADDR, &rx_byte);
    rx_data |= (rx_byte & 0xffUL);

    i2c_read_reg(REG2_ADDR, &rx_byte);
    rx_data |= (rx_byte & 0xffUL) << 8;

    i2c_read_reg(REG3_ADDR, &rx_byte);
    rx_data |= (rx_byte & 0xffUL) << 16;

    i2c_read_reg(REG4_ADDR, &rx_byte);
    rx_data |= (rx_byte & 0xffUL) << 24;

    return rx_data;
}

void i2c_aon_write_data(unsigned long data)
{
    u8 reg1_data, reg2_data, reg3_data, reg4_data;

    reg1_data = data & 0xff;
    reg2_data = (data >> 8) & 0xff;
    reg3_data = (data >> 16) & 0xff;
    reg4_data = (data >> 24) & 0xff;

    i2c_write_reg(REG4_ADDR, reg4_data);
    i2c_write_reg(REG3_ADDR, reg3_data);
    i2c_write_reg(REG2_ADDR, reg2_data);
    i2c_write_reg(REG1_ADDR, reg1_data);
    return;
}

unsigned long lynx_i2c_read_data(unsigned long addr)
{
    if (0 == i2c_already_init)
    {
        i2c_init();
        i2c_already_init = 1;
    }

    i2c_aon_send_reg(addr);
    i2c_aon_send_cmd(RF_READ_CMD);
    return i2c_aon_read_data();
}

void lynx_i2c_write_data(unsigned long addr, unsigned long data)
{
    if (0 == i2c_already_init)
    {
        i2c_init();
        i2c_already_init = 1;
    }

    i2c_aon_send_reg(addr);
    i2c_aon_write_data(data);
    i2c_aon_send_cmd(RF_WRITE_CMD);
    return;
}

#if 0
#define CONFIG_GPIO_I2C_SDA_SDR 33
#define CONFIG_GPIO_I2C_SCL_SDR 32
#define CONFIG_GPIO_I2C_SDA_DDR 31
#define CONFIG_GPIO_I2C_SCL_DDR 30

#define DIS_SWI2C_FNC     (EN_UART_FNC     | \
                           EN_SWI2C_FNC    | \
                           EN_I2C_AUX_FNC  | \
                           EN_UART_AUX_FNC | \
                           EN_SWI2C_AUX_FNC)
#define DIS_SWI2C_AUX_FNC DIS_SWI2C_FNC

#define EN_SWI2C_FNC_SDR  EN_SWI2C_AUX_FNC
#define EN_SWI2C_FNC_DDR  EN_SWI2C_FNC
#define DIS_SWI2C_FNC_SDR DIS_SWI2C_AUX_FNC
#define DIS_SWI2C_FNC_DDR DIS_SWI2C_FNC

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

#define SDA_HIGH()  GPREG(SDA_GPSET) = GPIO_I2C_SDA
#define SDA_LOW()   GPREG(SDA_GPCLR) = GPIO_I2C_SDA
#define SCL_HIGH()  GPREG(SCL_GPSET) = GPIO_I2C_SCL
#define SCL_LOW()   GPREG(SCL_GPCLR) = GPIO_I2C_SCL

inline static void I2C_DELAY(void)
{
    int _www;
    for (_www = 0; _www < GPIO_I2C_CLKW; _www++) ;
}

inline static void I2C_DUMMY(void)
{
    int _www;
    for (_www = 0; _www < 10; _www++)
    {
        SCL_LOW();
        I2C_DELAY();
        SCL_HIGH();
        I2C_DELAY();
    }
}

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static void mi2c_init(void)
{
    if (!(GPREG(PINMUX) & EN_SIP_FNC))
    {
        // SDR mode
        mi2c.sda_gpval = (CONFIG_GPIO_I2C_SDA_SDR / 32) ? GP2VAL : GPVAL;
        mi2c.scl_gpval = (CONFIG_GPIO_I2C_SCL_SDR / 32) ? GP2VAL : GPVAL;
        mi2c.sda = CONFIG_GPIO_I2C_SDA_SDR;
        mi2c.scl = CONFIG_GPIO_I2C_SCL_SDR;
        mi2c.enable = EN_SWI2C_FNC_SDR;
        mi2c.disable = DIS_SWI2C_FNC_SDR;
    }
    else
    {
        // DDR mode
        mi2c.sda_gpval = (CONFIG_GPIO_I2C_SDA_DDR / 32) ? GP2VAL : GPVAL;
        mi2c.scl_gpval = (CONFIG_GPIO_I2C_SCL_DDR / 32) ? GP2VAL : GPVAL;
        mi2c.sda = CONFIG_GPIO_I2C_SDA_DDR;
        mi2c.scl = CONFIG_GPIO_I2C_SCL_DDR;
        mi2c.enable = EN_SWI2C_FNC_DDR;
        mi2c.disable = DIS_SWI2C_FNC_DDR;
    }

}

static void i2c_grant(int enabled)
{
    static int rc = 0;
    if (enabled)
    {
        rc = GPREG(PINMUX);
        GPREG(PINMUX) &= ~(DIS_GPIO_I2C_FNC);
        GPREG(PINMUX) |= EN_GPIO_I2C_FNC;
        if (1)
        {
            /* additional STOP after got grant */
            /* special patch for our hwi2c slave mode */
            SDA_LOW();
            I2C_DELAY();
            SCL_HIGH();
            I2C_DELAY();
            SDA_HIGH();
            I2C_DELAY();

            I2C_DUMMY();
        }
    }
    else
    {
        GPREG(PINMUX) = rc;
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
    }
}

static void i2c_get_grant(void)
{
    i2c_grant(1);
}

static void i2c_put_grant(void)
{
    i2c_grant(0);
}

// change SDA to input pin
void i2c_sda_input(void)
{
    GPREG(SDA_GPDIR) &= ~GPIO_I2C_SDA;
}

// change SDA to output pin
void i2c_sda_output(void)
{
    GPREG(SDA_GPDIR) |= GPIO_I2C_SDA;
}

int i2c_sda_data(void)
{
    return (GPREG(SDA_GPVAL) & GPIO_I2C_SDA);
}

void i2c_start(void)
{
    i2c_get_grant();

    SDA_HIGH();
    i2c_sda_output();
    SDA_LOW();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

void i2c_restart(void)
{
    SDA_HIGH();
    i2c_sda_output();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SDA_LOW();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

void i2c_stop(void)
{
    SDA_LOW();
    i2c_sda_output();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SDA_HIGH();
    I2C_DELAY();

    i2c_put_grant();
}

void i2c_ack(void)
{
    SDA_LOW();
    i2c_sda_output();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

void i2c_nack(void)
{
    SDA_HIGH();
    i2c_sda_output();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

int i2c_read_bit(void)
{
    int rc;
    i2c_sda_input();
    I2C_DELAY();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    rc = i2c_sda_data();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
    return rc;
}

int i2c_read(int end)
{
    unsigned char byte = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        byte <<= 1;
        if (i2c_read_bit())
            byte |= 1;
    }
    if (end)
        i2c_nack();
    else
        i2c_ack();
    return byte;
}

void i2c_write_bit(int bit)
{
    if (bit)
        SDA_HIGH();
    else
        SDA_LOW();
    i2c_sda_output();
    I2C_DELAY();
    I2C_DELAY();
    SCL_HIGH();
    I2C_DELAY();
    I2C_DELAY();
    SCL_LOW();
    I2C_DELAY();
}

int i2c_write(unsigned char byte)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        i2c_write_bit((byte & BYTE_MSB) == BYTE_MSB);
        byte <<= 1;
    }
    return i2c_read_bit();
}

void i2c_init(void)
{
    printf
        ("warning: make sure Codec has MCLK!! if using I2C to program Codec\n");

    mi2c_init();

    GPREG(SDA_GPSEL) |= GPIO_I2C_SDA;   // select sda gpio pin
    GPREG(SCL_GPSEL) |= GPIO_I2C_SCL;   // select scl gpio pin
    GPREG(SDA_GPDIR) |= GPIO_I2C_SDA;   // as sda output pin
    GPREG(SCL_GPDIR) |= GPIO_I2C_SCL;   // as scl output pin
    GPREG(SDA_GPSET) = GPIO_I2C_SDA;    // sda output high
    GPREG(SCL_GPSET) = GPIO_I2C_SCL;    // scl output high
}

void i2c_finish(void)
{
    GPREG(SDA_GPSEL) &= ~GPIO_I2C_SDA;  // disable sda gpio pin
    GPREG(SCL_GPSEL) &= ~GPIO_I2C_SCL;  // disable scl gpio pin
}
#endif

int i2c_gpio_read(int argc, char *argv[])
{
    unsigned long addr;

    if (argc > 0 && !hextoul(argv[0], &addr))
        return ERR_HELP;
    if (argc != 1)
        return ERR_HELP;

    printf("%08x:", addr);
    printf(" %08x\n", lynx_i2c_read_data(addr));

    return ERR_OK;
}

int i2c_gpio_write(int argc, char *argv[])
{
    unsigned long addr, data;

    if (argc > 0 && !hextoul(argv[0], &addr))
        return ERR_HELP;
    if (argc > 1 && !hextoul(argv[1], &data))
        return ERR_HELP;
    if (argc != 2)
        return ERR_HELP;

    lynx_i2c_write_data(addr, data);

    return ERR_OK;
}

cmdt cmdt_i2c[] __attribute__ ((section("cmdt"))) =
{
    {
    "id", i2c_gpio_read, "id <address> ; i2c read momory"}
    ,
    {
    "ie", i2c_gpio_write, "ie <address> <data> ; i2c write momory"}
,};

#endif
