/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file sflash_gpio_spi.c
*   \brief Access Sflash by GPIO
*   \author Montage
*/
#ifdef CONFIG_SFLASH
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <flash_api.h>
#include <arch/chip.h>
#include <common.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#if 0
#define gspi_log printf
#else
#define gspi_log(x , ...)
#endif

#define CONFIG_GPIO_SPI_CS      15
#define CONFIG_GPIO_SPI_CLK     14
#define CONFIG_GPIO_SPI_DO      16
#define CONFIG_GPIO_SPI_DI      13

#define GPIO_SPI_CLK_W  1
#define GPIO_SPI_TXRX_SIZE  32

#define GPIO_SPI_CS     (1<<CONFIG_GPIO_SPI_CS)
#define GPIO_SPI_CLK    (1<<CONFIG_GPIO_SPI_CLK)
#define GPIO_SPI_DI     (1<<CONFIG_GPIO_SPI_DI)
#define GPIO_SPI_DO     (1<<CONFIG_GPIO_SPI_DO)

#define GPIO_SPI_MASK_OP    (GPIO_SPI_CS|GPIO_SPI_DO|GPIO_SPI_CLK)
#define GPIO_SPI_MASK_IP    (GPIO_SPI_DI)
#define GPIO_SPI_MASK       (GPIO_SPI_MASK_IP|GPIO_SPI_MASK_OP)

/* SPI MODE 0~3(CPOL|CPHA) define relationship with CLK, DO and DI */
/* sflash only support MODE 0/3 */
#define GPIO_SPI_CPOL   0
#define GPIO_SPI_CPHA   0

#define BIT31 1<<31

#define cs_high() { GPREG(GPSET) = GPIO_SPI_CS; }
#define cs_low() { GPREG(GPCLR) = GPIO_SPI_CS; }
//#define   spidelay(n)     {int _www; for (_www=0; _www< n; _www++); }
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static unsigned int cs_state = 0;
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
inline void spidelay(n)
{
    int _www;
    for (_www = 0; _www < n; _www++) ;
}

inline void setsck(unsigned int cpol)
{
    if (cpol == 1)
        GPREG(GPSET) = GPIO_SPI_CLK;    // set high
    else
        GPREG(GPCLR) = GPIO_SPI_CLK;    // set low
}

inline void setmosi(unsigned int word)
{
    if (word)
        GPREG(GPSET) = GPIO_SPI_DO;     // set high
    else
        GPREG(GPCLR) = GPIO_SPI_DO;     // set low
}

inline unsigned int getmiso(void)
{
    if (GPREG(GPVAL) & GPIO_SPI_MASK_IP)
        return 1;
    else
        return 0;
}

/* CPHA=0 means sample on the leading (first) clock edge,
while CPHA=1 means sample on the trailing (second) clock edge,
regardless of whether that clock edge is rising or falling*/
static inline unsigned int
spi_txrx_be(unsigned nsecs, unsigned int word, unsigned char bits)
{
    for (; bits > 0; bits--)
    {
        /* setup MSB (to slave) on trailing edge */
        setmosi(word & BIT31);
#if (GPIO_SPI_CPHA==0)
        spidelay(nsecs);
        setsck(!GPIO_SPI_CPOL);
#else
        setsck(!GPIO_SPI_CPOL);
        spidelay(nsecs);
#endif
        /* sample MSB (from slave) on leading edge */
        word <<= 1;
        word |= getmiso();
#if (GPIO_SPI_CPHA==0)
        spidelay(nsecs);
        setsck(GPIO_SPI_CPOL);
#else
        setsck(GPIO_SPI_CPOL);
        spidelay(nsecs);
#endif
    }
    return word;
}

#ifdef CONFIG_GPIO_SPI
int spi_read(unsigned int bank, unsigned int data_bit_len,
             unsigned char *rx_buf)
#else
int gpio_spi_read(unsigned int bank, unsigned int data_bit_len,
                  unsigned char *rx_buf)
#endif
{
    unsigned int *rc = (unsigned int *) rx_buf;
    if (cs_state == 0)
    {
        cs_low();
    }
    else if (cs_state == 2)
    {
        cs_low();               //setcs(0)
        cs_state = 1;
    }
    gspi_log("bit_len=%d\n", data_bit_len);
    while (data_bit_len > 0)
    {
        if (data_bit_len >= GPIO_SPI_TXRX_SIZE)
        {
            *rc = spi_txrx_be(GPIO_SPI_CLK_W, 0, GPIO_SPI_TXRX_SIZE);
            data_bit_len -= GPIO_SPI_TXRX_SIZE;
            rc++;
        }
        else
        {
            *rc = spi_txrx_be(GPIO_SPI_CLK_W, 0, data_bit_len);
            *rc <<= (GPIO_SPI_TXRX_SIZE - data_bit_len);
            data_bit_len = 0;
        }
        gspi_log("SPI read =0x%08x\n", rx_buf);
    }
    if (cs_state == 0)
        cs_high();              //setcs(1)
    return 0;
}

#ifdef CONFIG_GPIO_SPI
int spi_write(unsigned int bank, unsigned int data_bit_len,
              unsigned char *tx_buf)
#else
int gpio_spi_write(unsigned int bank, unsigned int data_bit_len,
                   unsigned char *tx_buf)
#endif
{
    unsigned int *rc = (unsigned int *) tx_buf;
    if (cs_state == 0)
    {
        cs_low();               //setcs(0);
    }
    else if (cs_state == 2)
    {
        cs_low();               //setcs(0);
        cs_state = 1;
    }
    while (data_bit_len > 0)
    {
        gspi_log("SPI write =0x%08x\n", *rc);
        if (data_bit_len >= GPIO_SPI_TXRX_SIZE)
        {
            spi_txrx_be(GPIO_SPI_CLK_W, *rc, GPIO_SPI_TXRX_SIZE);
            data_bit_len -= GPIO_SPI_TXRX_SIZE;
            rc++;
        }
        else
        {
            spi_txrx_be(GPIO_SPI_CLK_W, *rc, data_bit_len);
            data_bit_len = 0;
        }
    }
    if (cs_state == 0)
        cs_high();              //setcs(1);
    return 0;
}

#ifdef CONFIG_GPIO_SPI
inline void spi_keep_cs(unsigned int bank, int keep)
#else
inline void gpio_spi_keep_cs(unsigned int bank, int keep)
#endif
{
    if (keep)
        cs_state = 2;
    else
    {
        if (cs_state)
            cs_high();          //setcs(1);
        cs_state = 0;
    }
}

#ifdef CONFIG_GPIO_SPI
inline int spi_ready(unsigned int bank)
#else
inline int gpio_spi_ready(unsigned int bank)
#endif
{
    return 1;
}

void gpio_spi_init(void)
{
    //set gpio pin
    GPREG(GPSEL) |= GPIO_SPI_MASK;      // as gpio pin
    GPREG(GPDIR) |= GPIO_SPI_MASK_OP;   // 1: output
    GPREG(GPDIR) &= ~GPIO_SPI_MASK_IP;  // 0: input pin
    //init state for spi pin
    /* CS=1, DO=1, CLK=1(mode3) CLK=0(mode0) */
#if (GPIO_SPI_CPHA==0 && GPIO_SPI_CPOL==0)
    GPREG(GPSET) = GPIO_SPI_CS | GPIO_SPI_DO;
    GPREG(GPCLR) = GPIO_SPI_CLK;
#elif (GPIO_SPI_CPHA==1 && GPIO_SPI_CPOL==1)
    GPREG(GPSET) = GPIO_SPI_MASK_OP;
#else
    printf("NOT SUPPORT SPI MODE 1/2 \n");
#endif
}

void gpio_spi_finish(void)
{
    GPREG(GPSEL) &= ~GPIO_SPI_MASK;     // disable gpio
}

#if 1
int do_gs(int argc, char *argv[])
{
    int i, rc;
    int rw = 0xff;
    int failed = 0;
    unsigned char rx_buf[32];
    unsigned char *tx_buf;
    unsigned int data_len = 0xbfc20000;
    unsigned long end = 0xbfc30000;
    unsigned int data = 0;
    unsigned int rep = 1;
    unsigned int bank = 1;

    if (argc > 0)
    {
        if (!hextoul(argv[0], &rw))
            goto err1;
    }
    if (argc > 1)
    {
        if (!hextoul(argv[1], &data_len))
            goto err1;
    }
    if (argc > 2)
    {
        if (!hextoul(argv[2], &data))
            goto err1;
    }

    if (rw == 1)
    {
#ifdef CONFIG_GPIO_SPI
        if ((rc = spi_read(bank, data_len, rx_buf)))
#else
        if ((rc = gpio_spi_read(bank, data_len, rx_buf)))
#endif
        {
            failed |= (1 << 1);
        }
        else
        {
            gspi_log("read data =", rx_buf[0]);
            for (i = 0; i < (data_len + 7) / 8; i += 4)
            {
                gspi_log("%2x ", rx_buf[i]);
            }
        }
    }
    else if (rw == 2)
    {
        tx_buf = (unsigned char *) &data;
#ifdef CONFIG_GPIO_SPI
        if ((rc = spi_write(bank, data_len, tx_buf)))
#else
        if ((rc = gpio_spi_write(bank, data_len, tx_buf)))
#endif
        {
            failed |= (1 << 2);
        }
    }
    else if (rw == 3)
    {
#ifdef CONFIG_GPIO_SPI
        spi_keep_cs(bank, data_len);
#else
        gpio_spi_keep_cs(bank, data_len);
#endif
    }
    else if (rw == 4)
    {
        gpio_spi_init();
    }
    else if (rw == 5)
    {
        gpio_spi_finish();
    }
    gspi_log("\nDone: %s \n", (0 == failed) ? "PASSED" : "FAILED");
    return (0 == failed) ? ERR_OK : ERR_PARM;

  help:
  err1:
    return ERR_PARM;
}

cmdt cmdt_do_gs[] __attribute__ ((section("cmdt"))) =
{
    {
    "gs", do_gs,
            "gpio_spi 1(read)/2(write)/3(keep_cs)/4(init)/5(finish) bit_length(hex) string"}
,};
#endif
#endif
