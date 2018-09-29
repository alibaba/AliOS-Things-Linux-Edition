/* Simple SPI master API */
#include <common.h>
#include <lib.h>
#include <mt_types.h>
#include <arch/chip.h>
#include <pmu.h>
#include <pinmux.h>
#include <lib.h>

#define HARDWARE_SPI_LOW_SPEED_SETTING

#if defined(HARDWARE_SPI_LOW_SPEED_SETTING)
    #define SPI_INT_DLY   0xe
    #define SPI_TXD_DELAY (0x1f << 1)
static unsigned long default_clkdiv = 0x40; // 120Mhz / 64 = 1.8Mhz
#else
    #define SPI_INT_DLY   3
    #define SPI_TXD_DELAY (1 << 1)
static unsigned long default_clkdiv = 10; // 120Mhz / 10 = 12Mhz
#endif

#define REG(base,offset) (*(volatile unsigned int*)(base+(offset)))
#define GSPIREG(offset)  REG(GSPI_BASE,offset)

/* SPI bank register */
#define DFIFO          0x000
#define CH0_BAUD       0x100
#define  SPI_DLYSHFT      28    /* shift for delay field */
#define CH0_MODE_CFG   0x104
#define  SPI_DUMYH        (1<<31)       /* high impedance for dummy */
#define  SPI_CSRTRN       (1<<30)       /* cs retrun to default automatically */
#define  SPI_BYTEPKG      (1<<29)       /* pack data unit at byte */
#define  SPI_WTHSHFT      24    /* shift for data width field */
#define  SPI_CLDSHFT      18    /* shift for CLK delay field */
#define  SPI_CSDSHFT      16    /* shift for CS delay field */
#define  SPI_DBYTE        (1<<29)       /* data unit: byte */
#define  SPI_CPHA_WR      (1<<14)       /* cpha, data write */
#define  SPI_CFG_CPOL     (1<<13)       /* cpol */
#define  SPI_LSB          (1<<12)       /* least significant bit first */
#define  SPI_CPHA_RD      (1<<11)       /* cpha, data read */
#define  SPI_DPACK        (1<<10)       /* enable data package */
#define  SPI_HBYTE        (1<<9)        /* high byte first */
#define  SPI_FSHMODE      (1<<8)        /* change to flash mode */
#define CH1_BAUD       0x108
#define CH1_MODE_CFG   0x10C
#define CH2_BAUD       0x110
#define CH2_MODE_CFG   0x114
#define CH3_BAUD       0x118
#define CH3_MODE_CFG   0x11C
#define SPI_TC         0x120
#define SPI_CTRL       0x124
#define  SPI_C1NSHFT      30    /* shift for cmd1 io num field */
#define  SPI_C1LSHFT      24    /* shift for cmd1 len field */
#define  SPI_C0NSHFT      22    /* shift for cmd0 io num field */
#define  SPI_C0LSHFT      16    /* shift for cmd0 len field */
#define  SPI_DNTSHFT      14    /* shift for data io num field */
#define  SPI_DULSHFT      3     /* shift for dummy len field */
#define  SPI_DIRSHFT      1     /* shift for direction field */
#define  SPI_TRIGGER      (1<<0)        /* kick */
#define FIFO_CFG       0x128
#define SMPL_MODE      0x12C
#define PIN_MODE       0x130
#define  SPI_IO3SHFT       6    /* shift for io3 direction field */
#define  SPI_IO2SHFT       4    /* shift for io2 direction field */
#define  SPI_IO1SHFT       2    /* shift for io1 direction field */
#define  SPI_IO0SHFT       0    /* shift for io0 direction field */
#define  SPI_PININ         0    /* in direction */
#define  SPI_PINOUT        1    /* out direction */
#define  SPI_PINDIN        2    /* in direction at duplex mode, in/out direction at half-duplex mode */
#define  SPI_PINDOUT       3    /* out direction at duplex mode, in/out direction at half-duplex mode */
#define PIN_CTRL       0x134
#define  SPI_3WMODE       (1<<16)       /* 3 wire mode */
#define  SPI_03SHFT        6    /* shift for MOSI3 setting field */
#define  SPI_02SHFT        4    /* shift for MOSI2 setting field */
#define  SPI_01SHFT        2    /* shift for MOSI1 setting field */
#define  SPI_00SHFT        0    /* shift for MOSI0 setting field */
#define  SPI_OUTLOW        1    /* output low level */
#define  SPI_OUTHGH        2    /* output high level */
#define CH_MUX         0x138
#define  SPI_IO1CS3       (1<<2)        /* use io1 as channel3's cs */
#define  SPI_IO2CS2       (1<<1)        /* use io2 as channel2's cs */
#define  SPI_IO3CS1       (1<<1)        /* use io3 as channel1's cs */
#define STA            0x140
#define  SPI_BUSY         (1<<16)       /* spi busy state */
#define  SPI_TCSHFT        8    /* shift for TXD FIFO count state field */
#define  SPI_RCSHFT        0    /* shift for RXD FIFO count state field */
#define INT_STA        0x144
#define CMD_FIFO       0x148

static unsigned long ch_baud_cfg_base;
static unsigned long ch_mode_cfg_base;

void spi_wait_finish(void)
{
    u32 ahb_read_data;

    ahb_read_data = GSPIREG(STA);
    //printk("%x\n", ahb_read_data);

    while (ahb_read_data & SPI_BUSY)
    {
        ahb_read_data = GSPIREG(STA);
        //printk("%x\n", ahb_read_data);
    }
}

u32 spi_read_write_data(unsigned int nsecs, unsigned int cpol, unsigned int cpha, u32 word, u8 bits)
{
    u32 word2;
    u32 ch_mode = (ch_mode_cfg_base | ((bits-1) << SPI_WTHSHFT));
    unsigned int clkdiv = (nsecs * 2) * 120 / 1000;   /* SPI base clock rate is 120Mhz */

    GSPIREG(CH0_BAUD) = (ch_baud_cfg_base | clkdiv);
    GSPIREG(CH1_BAUD) = (ch_baud_cfg_base | clkdiv);

    if (cpol)
        ch_mode |= SPI_CFG_CPOL;

    if (cpha)
        ch_mode |= (SPI_CPHA_WR | SPI_CPHA_RD);

    GSPIREG(CH0_MODE_CFG) = ch_mode;
    GSPIREG(CH1_MODE_CFG) = ch_mode;

    GSPIREG(SPI_TC) = 1;
    GSPIREG(SPI_CTRL) = ( (0x03 << 9) | (0x03 << SPI_DIRSHFT) | SPI_TRIGGER );

    GSPIREG(DFIFO) = word;

    spi_wait_finish();

    word2 = GSPIREG(DFIFO); // & 0xff;

    //if((word!=0xff)||(word2!=0xff))
    printk("TX %02x RX %02x\n", word & 0xff, word2 & 0xff);

    return word2;
}


#define SPI_CS_GPIO 19

/*
    pinmux: 17=0,18=0,19=3,20=0
    gpio_driving: 17=3,18=3,19=3,20=3

    1. Use pin19 as GPIO for software based chip-select control
    2. Use pin18 (SPI_DO), pin17(SPI_DI), pin20(SPI_CK)
    3. To test this configuration, connect and pull-high GPIO 17/18/19/20 on the test platform
*/

static void spi_pinmux(void)
{
    int gpio_ids[] = { 17, 18, SPI_CS_GPIO, 20, -1};
    unsigned long gpio_vals[] = { 0, 0, 3, 0, 0};

    pmu_set_gpio_function(gpio_ids, gpio_vals);
}

extern void pmu_set_gpio_driving_strength(int *gpio_ids, unsigned long *gpio_funcs);
static void spi_driving(void)
{
    int gpio_ids[] = { 17, 18, SPI_CS_GPIO, 20, -1};
    unsigned long gpio_vals[] = { 3, 3, 3, 3, 0};

    pmu_set_gpio_driving_strength(gpio_ids, gpio_vals);
}


static int spi_cs_high = 0;
static int spi_nsecs = 1250;
static int spi_cpol = 0;
static int spi_cpha = 0;
void spi_setup(int cs_high, unsigned int nsecs, unsigned int cpol, unsigned int cpha)
{
    spi_cs_high = cs_high;
    spi_nsecs = nsecs;
    spi_cpol = cpol;
    spi_cpha = cpha;
}

/* CS is low-active */
static void spi_chipselect(int is_active)
{
    int gpio = SPI_CS_GPIO;
    int output_0;

    output_0 = spi_cs_high ? !is_active : is_active;

    if(output_0)
    {
        // output 0
        PMUREG_UPDATE32(GPIO_OEN, 0, (0x01 << gpio));
        PMUREG_WRITE32(GPIO_ODC, (0x01 << gpio));
    }
    else
    {
        // output 1
        PMUREG_UPDATE32(GPIO_OEN, 0, (0x01 << gpio));
        PMUREG_WRITE32(GPIO_ODS, (0x01 << gpio));
    }

    mdelay(20);
}

void spi_transfer(unsigned char* tx, unsigned char* rx, int length)
{
    int i;

    for(i=0;i<length;i++)
    {
        rx[i] = 0xff & spi_read_write_data(spi_nsecs, spi_cpol, spi_cpha, tx[i], 8);
    }
}

void spi_init(void)
{
    ch_baud_cfg_base = (SPI_INT_DLY << SPI_DLYSHFT);
    ch_mode_cfg_base = (SPI_DUMYH | SPI_CSRTRN | SPI_BYTEPKG | 
                        (3 << SPI_CLDSHFT) | (3 << SPI_CSDSHFT) | SPI_TXD_DELAY);

    spi_pinmux();

    spi_driving();

    spi_chipselect(0);

    GSPIREG(CH0_BAUD) = (ch_baud_cfg_base | default_clkdiv);
    GSPIREG(CH1_BAUD) = (ch_baud_cfg_base | default_clkdiv);

    GSPIREG(CH0_MODE_CFG) = ch_mode_cfg_base;
    GSPIREG(CH1_MODE_CFG) = ch_mode_cfg_base;

    GSPIREG(PIN_CTRL) = ( (SPI_OUTHGH << SPI_03SHFT) | (SPI_OUTHGH << SPI_02SHFT) );
    GSPIREG(PIN_MODE) = ( (SPI_PINOUT << SPI_IO0SHFT) | (SPI_PININ << SPI_IO1SHFT) );

    mdelay(20);
}

int spit_cmd(int argc, char *argv[])
{
#define TX_RX_BUFSIZE 64
    unsigned char tx[TX_RX_BUFSIZE];
    unsigned char rx[TX_RX_BUFSIZE];

    unsigned char cmd52[] = { 0xff, 0x74, 0x00, 0x00, 0x0c, 0x00, 0x39 };
    //unsigned char cmd52[] = { 0xff, 0x74, 0x80, 0x00, 0x0c, 0x08, 0x9f };
    unsigned char cmd0[] =  { 0xff, 0x40, 0x00, 0x00, 0x00, 0x00, 0x95 };

    memset((void *) &tx[0], 0xff, TX_RX_BUFSIZE);
    memset((void *) &rx[0], 0xff, TX_RX_BUFSIZE);

    spi_setup(0, 25, 0, 0);
    spi_init();

#if 1
    spi_chipselect(1);

    spi_transfer(tx, rx, 29);

    spi_transfer(tx, rx, 10);

    spi_chipselect(0);
#endif

#if 1
    spi_setup(1, 25, 0, 0);
    spi_chipselect(1);

    /*
	 * Do a burst with chipselect active-high.  We need to do this to
	 * meet the requirement of 74 clock cycles with both chipselect
	 * and CMD (MOSI) high before CMD0 ... after the card has been
	 * powered up to Vdd(min), and so is ready to take commands.
     */
    spi_transfer(tx, rx, 18);
    spi_setup(0, 25, 0, 0);
    spi_chipselect(0);
#endif

#if 1
    memcpy(tx, cmd52, 7);
    spi_chipselect(1);
    spi_transfer(tx, rx, 18);
    spi_chipselect(0);
    printf("CMD52 RESP %02x %02x %2x %2x\n", rx[7], rx[8], rx[9], rx[10]);
#endif

    memcpy(tx, cmd0, 7);
    spi_chipselect(1);
    spi_transfer(tx, rx, 17);
    spi_chipselect(0);
    printf("CMD0 RESP %02x %02x\n", rx[7], rx[8]);

    return ERR_OK;
}

cmdt cmdt_gspit __attribute__ ((section("cmdt"))) =
{
    "spit", spit_cmd, "spit\n"
};


