/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file sdhci-mt.c
*   \brief Montage SDHC Driver
*   \author Montage
*/
#ifdef CONFIG_SDHC

#include <arch/chip.h>
#include <arch/irq.h>
#include <common.h>
#include <sdhci.h>

struct sdhci_host host_dev;
struct sdhci_host *host;

static void sdhci_mt_platform_reset_enter(struct sdhci_host *host, u8 mask)
{
    if (mask & SDHCI_RESET_ALL)
    {
        /* enable SD clock */
        ANAREG(CLKEN) |= SD_CLK_EN;

        /* SD driving adjustment */
        GPREG(DRIVER1) &= ~(DRV_SDIO);

        /*
         *  bit 27, 26, 25, 24 :
         *  bit 23, 22, 21, 20 : MDIO_AUX, SWI2C_AUX,    SWI2C, RF_LEGA
         *  bit 19, 18, 17, 16 : JTAG_AUX,   LED_AUX, UART_AUX, I2C_AUX
         *  bit 15, 14, 13, 12 :  PWM_AUX,      ROM1,     JTAG,     AGC
         *  bit 11, 10,  9,  8 :      PWM,       LED,     UART,    MDIO
         *  bit  7,  6,  5,  4 :     PCM1,      PCM0,     ETH1,    ETH0
         *  bit  3,  2,  1,  0 :     SDIO,        W6,      SIP,     SDR
         *
         *  For PINMUX, Enable SDIO function. Max(SDIO_FNC + ETH0_FNC) <=1
         *  ETH0 must be disabled, when SDIO is enabled
         *  MDIO must be disabled, when SDIO is enabled
         */

//              if (host->mmc->caps3 & MMC_CAP3_CONTROL_VOLTAGE)
//                      ANAREG(VOLCTRL) &= ~(3<<0);
//              else
        ANAREG(VOLCTRL) |= (1 << 1);

        /*
         * SDHC forcely disable mdio pin and eth0 function
         * So if DDR mode(SIP mode) and MDIO is used by external ether phy,
         * ether will lose its function
         * (DDR mode + external ETH0 or ETH1 will be failed)
         * (DDR mode + internal ether will be OK)
         */
        /* disable PINMUX mdio function */
        GPREG(PINMUX) &= ~(EN_MDIO_FNC);
        /* disable PINMUX eth0 function */
        GPREG(PINMUX) &= ~(EN_ETH0_FNC);

        /* enable PINMUX sdio function */
        GPREG(PINMUX) |= (EN_SDIO_FNC);
        /* disable external BB mode */
        GPREG(GDEBUG) &= ~(0x90 << 24);
        /* enable W6 */
        GPREG(GDEBUG) |= (1 << 22);
        /* enable sdio module */
        GPREG(GDEBUG) |= (1 << 21);
        /* turn on sdio gated clock */
        GPREG(SWRST) &= ~(PAUSE_SDIO);
        /* reset sdio module */
        GPREG(SWRST) &= ~(SWRST_SDIO);
        mdelay(1);
        GPREG(SWRST) |= (SWRST_SDIO);
    }
}

static void sdhci_mt_platform_reset_exit(struct sdhci_host *host, u8 mask)
{
    if (mask & SDHCI_RESET_ALL)
    {
        /* system endian setting */
        // AHB bus: little to little, AMB bus: little to big
        GPREG(SDIO) =
            (GPREG(SDIO) & ~(0x3UL << SDIO_ENDIAN_PIN)) | (0x2UL <<
                                                           SDIO_ENDIAN_PIN);
    }
}

static struct sdhci_ops sdhci_mt_ops = {
    .platform_reset_enter = sdhci_mt_platform_reset_enter,
    .platform_reset_exit = sdhci_mt_platform_reset_exit,
};

static void sdhc_irqhandler(void *irqno)
{
    u32 intmask;

    intmask = sdhci_readl(host, SDHCI_INT_STATUS);

//      printf("*** got interrupt: 0x%08x\n", intmask);

    if (intmask & (SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE))
    {
        u32 present =
            sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_CARD_PRESENT;
        if (present)
        {
            printf("Card Inserted...\n");
            mmc_init(host->mmc);
        }
        else
        {
            printf("Card Removed...\n");
            mmc_deinit(host->mmc);
        }
        sdhci_writel(host,
                     intmask & (SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE),
                     SDHCI_INT_STATUS);
        intmask &= ~(SDHCI_INT_CARD_INSERT | SDHCI_INT_CARD_REMOVE);
    }
    sdhci_writel(host, intmask, SDHCI_INT_STATUS);
}

void sdhc_init(void)
{
    int err;

    host = &host_dev;
    host->name = "montage-sdhci";
    host->ioaddr = (void *) SDIO_BASE;
    host->ops = &sdhci_mt_ops;
    host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;
    host->host_caps = MMC_MODE_HC;

    err = add_sdhci(host, CONFIG_SYS_SDIO_MAX_CLK, CONFIG_SYS_SDIO_MIN_CLK);

    if (!err)
    {
        request_irq(IRQ_SDHC, &sdhc_irqhandler, (void *) IRQ_SDHC);
    }

    printf("init %s for SD clock MAX:%d Hz MIN:%d Hz\n", host->name,
           host->cfg.f_max, host->cfg.f_min);
}

#ifdef CONFIG_I2CS
void mmc_disable_interrupt(void)
{
    sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);
}

void mmc_enable_interrupt(void)
{
    mmc_deinit(host->mmc);
}
#endif

int mmc_cmd(int argc, char *argv[])
{
    u32 start, blkcnt;
    u32 tstart;
    char cmd;
    u32 err = 0;
    u8 *buf = (u8 *) uncached_addr(buf_address);
    switch (argc)
    {
        case 4:
            if (!hextoul(argv[3], &buf))
                goto err;
        case 3:
            if (1 != sscanf(argv[2], "%d", &blkcnt))
                goto err;
            if (1 != sscanf(argv[1], "%d", &start))
                goto err;
            cmd = argv[0][0];
            if (cmd != 'r' && cmd != 'w' && cmd != 'e')
                goto err;
            break;
        default:
            goto err;
    }

    printf("input: cmd=%c, start=%d, blkcnt=%d, buf=0x%x\n", cmd, start, blkcnt,
           buf);

    tstart = how_long(0);
    switch (cmd)
    {
        case 'r':
            err = host->mmc->block_read(0, start, blkcnt, buf);
            break;
        case 'w':
            err = host->mmc->block_write(0, start, blkcnt, buf);
            break;
        case 'e':
            err = host->mmc->block_erase(0, start, blkcnt);
            break;
    }
    if (!err)
    {
        printf("cmd '%c' OK(%d ms)\n", cmd, how_long(tstart));
        return ERR_OK;
    }
    else
    {
        printf("cmd '%c' FAIL(%d ms)\n", cmd, how_long(tstart));
        return ERR_LAST;
    }

  err:
    return ERR_HELP;
}

cmdt cmdt_mmc[] __attribute__ ((section("cmdt"))) =
{
    {
    "mmc", mmc_cmd,
            "mmc r <start> <blkcnt> <dst> ;mmc read block\n"
            "mmc w <start> <blkcnt> <src> ;mmc write block\n"
            "mmc e <start> <blkcnt> ;mmc erase block"}
,};
#endif
