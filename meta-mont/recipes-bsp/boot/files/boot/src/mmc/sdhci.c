/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file sdhci.c
*   \brief SDHC Driver
*   \author Montage
*/
#include <common.h>
#include <byteorder.h>
#include <mmc.h>
#include <sdhci.h>

/**
 * fls - find last (most-significant) bit set
 *  \param x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u))
    {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u))
    {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u))
    {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u))
    {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u))
    {
        x <<= 1;
        r -= 1;
    }
    return r;
}

#define DRIVER_NAME "mmc"
static void sdhci_dumpregs(struct sdhci_host *host)
{
    printf(DRIVER_NAME ": ============== REGISTER DUMP ==============\n");

    printf(DRIVER_NAME ": Sys addr: 0x%08x | Version:  0x%08x\n",
           sdhci_readl(host, SDHCI_DMA_ADDRESS),
           sdhci_readw(host, SDHCI_HOST_VERSION));
    printf(DRIVER_NAME ": Blk size: 0x%08x | Blk cnt:  0x%08x\n",
           sdhci_readw(host, SDHCI_BLOCK_SIZE),
           sdhci_readw(host, SDHCI_BLOCK_COUNT));
    printf(DRIVER_NAME ": Argument: 0x%08x | Trn mode: 0x%08x\n",
           sdhci_readl(host, SDHCI_ARGUMENT),
           sdhci_readw(host, SDHCI_TRANSFER_MODE));
    printf(DRIVER_NAME ": Present:  0x%08x | Host ctl: 0x%08x\n",
           sdhci_readl(host, SDHCI_PRESENT_STATE),
           sdhci_readb(host, SDHCI_HOST_CONTROL));
    printf(DRIVER_NAME ": Power:    0x%08x | Blk gap:  0x%08x\n",
           sdhci_readb(host, SDHCI_POWER_CONTROL),
           sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
    printf(DRIVER_NAME ": Wake-up:  0x%08x | Clock:    0x%08x\n",
           sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
           sdhci_readw(host, SDHCI_CLOCK_CONTROL));
    printf(DRIVER_NAME ": Timeout:  0x%08x | Int stat: 0x%08x\n",
           sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
           sdhci_readl(host, SDHCI_INT_STATUS));
    printf(DRIVER_NAME ": Int enab: 0x%08x | Sig enab: 0x%08x\n",
           sdhci_readl(host, SDHCI_INT_ENABLE),
           sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
    printf(DRIVER_NAME ": AC12 err: 0x%08x | Slot int: 0x%08x\n",
           sdhci_readw(host, SDHCI_ACMD12_ERR),
           sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
    printf(DRIVER_NAME ": Caps:     0x%08x | Caps_1:   0x%08x\n",
           sdhci_readl(host, SDHCI_CAPABILITIES),
           sdhci_readl(host, SDHCI_CAPABILITIES_1));
    printf(DRIVER_NAME ": Cmd:      0x%08x | Max curr: 0x%08x\n",
           sdhci_readw(host, SDHCI_COMMAND),
           sdhci_readl(host, SDHCI_MAX_CURRENT));
    printf(DRIVER_NAME ": Host ctl2: 0x%08x\n",
           sdhci_readw(host, SDHCI_HOST_CONTROL2));

    printf(DRIVER_NAME ": ===========================================\n");
}

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
    unsigned long timeout;

    if (host->ops->platform_reset_enter)
        host->ops->platform_reset_enter(host, mask);

    /* Wait max 100 ms */
    timeout = 100;
    sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
    while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask)
    {
        if (timeout == 0)
        {
            printf("%s: Reset 0x%x never completed.\n", __func__, (int) mask);
            break;
        }
        timeout--;
        mdelay(1);
    }

    if (host->ops->platform_reset_exit)
        host->ops->platform_reset_exit(host, mask);
}

static void sdhci_cmd_done(struct sdhci_host *host, struct mmc_cmd *cmd)
{
    int i;
    if (cmd->resp_type & MMC_RSP_136)
    {
        /* CRC is stripped so we need to do some shifting. */
        for (i = 0; i < 4; i++)
        {
            cmd->response[i] = sdhci_readl(host,
                                           SDHCI_RESPONSE + (3 - i) * 4) << 8;
            if (i != 3)
                cmd->response[i] |= sdhci_readb(host,
                                                SDHCI_RESPONSE + (3 - i) * 4 -
                                                1);
        }
    }
    else
    {
        cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
    }
}

static void sdhci_transfer_pio(struct sdhci_host *host, struct mmc_data *data)
{
    int i;
    char *offs;
    u32 scratch;
    for (i = 0; i < data->blocksize; i += 4)
    {
        offs = data->dest + i;
        if (data->flags == MMC_DATA_READ)
        {
            scratch = sdhci_readl(host, SDHCI_BUFFER);
            *(u32 *) offs = le32_to_cpu(scratch);
        }
        else
        {
            scratch = *(u32 *) offs;
            sdhci_writel(host, cpu_to_le32(scratch), SDHCI_BUFFER);
        }
    }
}

static int sdhci_transfer_data(struct sdhci_host *host, struct mmc_data *data,
                               unsigned int start_addr)
{
    unsigned int stat, rdy, mask, timeout, block = 0;

    timeout = 10000000;
    rdy = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
    mask = SDHCI_DATA_AVAILABLE | SDHCI_SPACE_AVAILABLE;
    do
    {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);
        if (stat & SDHCI_INT_ERROR)
        {
            printf("%s: Error detected in status(0x%X)!\n", __func__, stat);
            return -1;
        }
        if (stat & rdy)
        {
            if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & mask))
                continue;
            sdhci_writel(host, rdy, SDHCI_INT_STATUS);
            sdhci_transfer_pio(host, data);
            data->dest += data->blocksize;
            if (++block >= data->blocks)
                break;
        }
        if (timeout-- > 0)
        {
//                      udelay(10);
            SDHC_DELAY();
        }
        else
        {
            printf("%s: Transfer data timeout\n", __func__);
            return -1;
        }
    }
    while (!(stat & SDHCI_INT_DATA_END));
    return 0;
}

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value. Max timeout can be redefined
 * in board config file.
 */
#ifndef CONFIG_SDHCI_CMD_MAX_TIMEOUT
#define CONFIG_SDHCI_CMD_MAX_TIMEOUT		3200
#endif
#define CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT	100

int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd,
                       struct mmc_data *data)
{
    struct sdhci_host *host = mmc->priv;
    unsigned int stat = 0;
    int ret = 0;
    int trans_bytes = 0, is_aligned = 1;
    u32 mask, flags, mode;
    unsigned int time = 0, start_addr = 0;
    unsigned int retry = 10000;

    /* Timeout unit - ms */
    static unsigned int cmd_timeout = CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT;

    sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
    mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

    /* We shouldn't wait for data inihibit for stop commands, even
       though they might use busy signaling */
    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
        mask &= ~SDHCI_DATA_INHIBIT;

    while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask)
    {
        if (time >= cmd_timeout)
        {
            printf("%s: MMC busy ", __func__);
            if (2 * cmd_timeout <= CONFIG_SDHCI_CMD_MAX_TIMEOUT)
            {
                cmd_timeout += cmd_timeout;
                printf("timeout increasing to: %u ms.\n", cmd_timeout);
            }
            else
            {
                puts("timeout.\n");
                return COMM_ERR;
            }
        }
        time++;
        mdelay(1);
    }

    mask = SDHCI_INT_RESPONSE;
    /* Command Complete is not generated
       by the response of CMD12 or CMD23 */
    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
        mask &= ~SDHCI_INT_RESPONSE;
    if (!(cmd->resp_type & MMC_RSP_PRESENT))
        flags = SDHCI_CMD_RESP_NONE;
    else if (cmd->resp_type & MMC_RSP_136)
        flags = SDHCI_CMD_RESP_LONG;
    else if (cmd->resp_type & MMC_RSP_BUSY)
    {
        flags = SDHCI_CMD_RESP_SHORT_BUSY;
        /* Command Complete is not generated
           by MMC_RSP_BUSY */
        mask &= ~SDHCI_INT_RESPONSE;
        mask |= SDHCI_INT_DATA_END;
    }
    else
        flags = SDHCI_CMD_RESP_SHORT;

    if (cmd->resp_type & MMC_RSP_CRC)
        flags |= SDHCI_CMD_CRC;
    if (cmd->resp_type & MMC_RSP_OPCODE)
        flags |= SDHCI_CMD_INDEX;
    if (data)
        flags |= SDHCI_CMD_DATA;

    /* Set Transfer mode regarding to data flag */
    if (data != 0)
    {
        sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
        mode = SDHCI_TRNS_BLK_CNT_EN;
        trans_bytes = data->blocks * data->blocksize;
        if (data->blocks > 1)
            mode |= SDHCI_TRNS_MULTI;

        if (data->flags == MMC_DATA_READ)
            mode |= SDHCI_TRNS_READ;

        sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
                                            data->blocksize), SDHCI_BLOCK_SIZE);
        sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
        sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
    }

    sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
    sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
#ifdef CONFIG_MMC_TRACE
    sdhci_dumpregs(host);
#endif
    do
    {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);
        if (stat & SDHCI_INT_ERROR)
            break;
        if (--retry == 0)
            break;
        SDHC_DELAY();
    }
    while ((stat & mask) != mask);

    if (retry == 0)
    {
        if (host->quirks & SDHCI_QUIRK_BROKEN_R1B)
            return 0;
        else
        {
            printf("%s: Timeout for status update!\n", __func__);
            return TIMEOUT;
        }
    }

    if ((stat & (SDHCI_INT_ERROR | mask)) == mask)
    {
        sdhci_cmd_done(host, cmd);
        sdhci_writel(host, mask, SDHCI_INT_STATUS);
    }
    else
        ret = -1;

    if (!ret && data)
        ret = sdhci_transfer_data(host, data, start_addr);

    if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
        mdelay(1);

    stat = sdhci_readl(host, SDHCI_INT_STATUS);
    sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
    if (!ret)
        return 0;

    sdhci_reset(host, SDHCI_RESET_CMD);
    sdhci_reset(host, SDHCI_RESET_DATA);
    if (stat & SDHCI_INT_TIMEOUT)
        return TIMEOUT;
    else
        return COMM_ERR;
}

static int sdhci_set_clock(struct mmc *mmc, unsigned int clock)
{
    struct sdhci_host *host = mmc->priv;
    unsigned int div, clk, timeout;

    if (clock && clock == host->clock)
        return 0;

    sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

    if (clock == 0)
        goto out;

#ifdef CONFIG_FPGA
    if (clock > CONFIG_SYS_SDIO_FPGA_MAX_CLK)
    {
        clock = CONFIG_SYS_SDIO_FPGA_MAX_CLK;
        printf("SDIO RUNNING MAX CLK in FPGA: %d Hz\n",
               CONFIG_SYS_SDIO_FPGA_MAX_CLK);
    }
#endif

    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
    {
        /* Version 3.00 divisors must be a multiple of 2. */
        if (mmc->cfg->f_max <= clock)
            div = 1;
        else
        {
            for (div = 2; div < SDHCI_MAX_DIV_SPEC_300; div += 2)
            {
                if ((mmc->cfg->f_max / div) <= clock)
                {
                    printf("SDIO RUNNING CLK in REAL: %d Hz\n",
                           (mmc->cfg->f_max / div));
                    break;
                }
            }
        }
    }
    else
    {
        /* Version 2.00 divisors must be a power of 2. */
        for (div = 1; div < SDHCI_MAX_DIV_SPEC_200; div *= 2)
        {
            if ((mmc->cfg->f_max / div) <= clock)
            {
                printf("SDIO RUNNING CLK in REAL: %d Hz\n",
                       (mmc->cfg->f_max / div));
                break;
            }
        }
    }
    div >>= 1;

    clk = (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
    clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
        << SDHCI_DIVIDER_HI_SHIFT;
    clk |= SDHCI_CLOCK_INT_EN;
    sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

    /* Wait max 20 ms */
    timeout = 20;
    while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
             & SDHCI_CLOCK_INT_STABLE))
    {
        if (timeout == 0)
        {
            printf("%s: Internal clock never stabilised.\n", __func__);
            return -1;
        }
        timeout--;
        mdelay(1);
    }

    clk |= SDHCI_CLOCK_CARD_EN;
    sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

  out:
    host->clock = clock;
    return 0;
}

static void sdhci_set_power(struct sdhci_host *host, unsigned short power)
{
    u8 pwr = 0;

    if (power != (unsigned short) -1)
    {
        switch (1 << power)
        {
            case MMC_VDD_165_195:
                pwr = SDHCI_POWER_180;
                break;
            case MMC_VDD_29_30:
            case MMC_VDD_30_31:
                pwr = SDHCI_POWER_300;
                break;
            case MMC_VDD_32_33:
            case MMC_VDD_33_34:
                pwr = SDHCI_POWER_330;
                break;
        }
    }

    if (pwr == 0)
    {
        sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
        return;
    }

    if (host->quirks & SDHCI_QUIRK_NO_SIMULT_VDD_AND_POWER)
        sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);

    pwr |= SDHCI_POWER_ON;

    sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
}

void sdhci_set_ios(struct mmc *mmc)
{
    u32 ctrl;
    struct sdhci_host *host = mmc->priv;

    if (mmc->clock != host->clock)
        sdhci_set_clock(mmc, mmc->clock);

    /* Set bus width */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
    if (mmc->bus_width == 8)
    {
        ctrl &= ~SDHCI_CTRL_4BITBUS;
        if ((SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300) ||
            (host->quirks & SDHCI_QUIRK_USE_WIDE8))
            ctrl |= SDHCI_CTRL_8BITBUS;
    }
    else
    {
        if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
            ctrl &= ~SDHCI_CTRL_8BITBUS;
        if (mmc->bus_width == 4)
            ctrl |= SDHCI_CTRL_4BITBUS;
        else
            ctrl &= ~SDHCI_CTRL_4BITBUS;
    }

    if (mmc->card_caps & MMC_MODE_HS)
        ctrl |= SDHCI_CTRL_HISPD;
    else
        ctrl &= ~SDHCI_CTRL_HISPD;

    if (host->quirks & SDHCI_QUIRK_NO_HISPD_BIT)
        ctrl &= ~SDHCI_CTRL_HISPD;

#ifdef CONFIG_MMC_UHS
    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
    {
        u16 clk, ctrl_2;
        unsigned int clock;

        /* In case of UHS-I modes, set High Speed Enable */
        if ((mmc->card_caps & MMC_MODE_MMC_HS200) ||
            (mmc->card_caps & MMC_MODE_UHS_SDR50) ||
            (mmc->card_caps & MMC_MODE_UHS_SDR104) ||
            (mmc->card_caps & MMC_MODE_UHS_DDR50) ||
            (mmc->card_caps & MMC_MODE_UHS_SDR25))
            ctrl |= SDHCI_CTRL_HISPD;

        ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
        if (!(ctrl_2 & SDHCI_CTRL_PRESET_VAL_ENABLE))
        {
            sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
            /*
             * We only need to set Driver Strength if the
             * preset value enable is not set.
             */
            ctrl_2 &= ~SDHCI_CTRL_DRV_TYPE_MASK;
//                      if (ios->drv_type == MMC_SET_DRIVER_TYPE_A)
//                              ctrl_2 |= SDHCI_CTRL_DRV_TYPE_A;
//                      else if (ios->drv_type == MMC_SET_DRIVER_TYPE_C)
//                              ctrl_2 |= SDHCI_CTRL_DRV_TYPE_C;

            sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);
        }
        else
        {
            /*
             * According to SDHC Spec v3.00, if the Preset Value
             * Enable in the Host Control 2 register is set, we
             * need to reset SD Clock Enable before changing High
             * Speed Enable to avoid generating clock gliches.
             */

            /* Reset SD Clock Enable */
            clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
            clk &= ~SDHCI_CLOCK_CARD_EN;
            sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

            sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);

            /* Re-enable SD Clock */
            clock = host->clock;
            host->clock = 0;
            sdhci_set_clock(mmc, clock);
        }

        /* Reset SD Clock Enable */
        clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
        clk &= ~SDHCI_CLOCK_CARD_EN;
        sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

        ctrl_2 = sdhci_readw(host, SDHCI_HOST_CONTROL2);
        /* Select Bus Speed Mode for host */
        ctrl_2 &= ~SDHCI_CTRL_UHS_MASK;
        if (mmc->card_caps & MMC_MODE_MMC_HS200)
            ctrl_2 |= SDHCI_CTRL_HS_SDR200;
        else if (mmc->card_caps & MMC_MODE_UHS_SDR12)
            ctrl_2 |= SDHCI_CTRL_UHS_SDR12;
        else if (mmc->card_caps & MMC_MODE_UHS_SDR25)
            ctrl_2 |= SDHCI_CTRL_UHS_SDR25;
        else if (mmc->card_caps & MMC_MODE_UHS_SDR50)
            ctrl_2 |= SDHCI_CTRL_UHS_SDR50;
        else if (mmc->card_caps & MMC_MODE_UHS_SDR104)
            ctrl_2 |= SDHCI_CTRL_UHS_SDR104;
        else if (mmc->card_caps & MMC_MODE_UHS_DDR50)
            ctrl_2 |= SDHCI_CTRL_UHS_DDR50;
        sdhci_writew(host, ctrl_2, SDHCI_HOST_CONTROL2);

        /* Re-enable SD Clock */
        clock = host->clock;
        host->clock = 0;
        sdhci_set_clock(mmc, clock);
    }
    else
#endif
    {
        sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
    }
}

int sdhci_init(struct mmc *mmc)
{
    struct sdhci_host *host = mmc->priv;

    sdhci_set_power(host, fls(mmc->cfg->voltages) - 1);

    if (host->quirks & SDHCI_QUIRK_NO_CD)
    {
        unsigned int status;

        sdhci_writeb(host, SDHCI_CTRL_CD_TEST_INS | SDHCI_CTRL_CD_TEST,
                     SDHCI_HOST_CONTROL);

        status = sdhci_readl(host, SDHCI_PRESENT_STATE);
        while ((!(status & SDHCI_CARD_PRESENT)) ||
               (!(status & SDHCI_CARD_STATE_STABLE)) ||
               (!(status & SDHCI_CARD_DETECT_PIN_LEVEL)))
            status = sdhci_readl(host, SDHCI_PRESENT_STATE);
    }

    /* Enable only interrupts served by the SD controller */
    sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
                 SDHCI_INT_ENABLE);
    /* Mask all sdhci interrupt sources */
    sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

    sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK |
                 SDHCI_INT_CARD_REMOVE | SDHCI_INT_CARD_INSERT,
                 SDHCI_INT_ENABLE);
    sdhci_writel(host, SDHCI_INT_CARD_REMOVE | SDHCI_INT_CARD_INSERT,
                 SDHCI_SIGNAL_ENABLE);

    return 0;
}

int sdhci_getcd(struct mmc *mmc)
{
    struct sdhci_host *host = mmc->priv;
    return (sdhci_readl(host, SDHCI_PRESENT_STATE) &
            SDHCI_CARD_DETECT_PIN_LEVEL) ? 1 : 0;
}

int sdhci_getwp(struct mmc *mmc)
{
    struct sdhci_host *host = mmc->priv;
    return (sdhci_readl(host, SDHCI_PRESENT_STATE) & SDHCI_WRITE_PROTECT) ? 0 :
        1;
}

void sdhci_reset_all(struct mmc *mmc)
{
    struct sdhci_host *host = mmc->priv;
    sdhci_reset(host, SDHCI_RESET_ALL);
}

#ifdef CONFIG_MMC_UHS
int sdhci_start_signal_voltage_switch(struct mmc *mmc)
{
    u8 pwr;
    u16 clk, ctrl;
    u32 present_state;
    struct sdhci_host *host = mmc->priv;

    /*
     * Signal Voltage Switching is only applicable for Host Controllers
     * v3.00 and above.
     */
    if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300)
        return 0;

    /*
     * We first check whether the request is to set signalling voltage
     * to 3.3V. If so, we change the voltage to 3.3V and return quickly.
     */
    ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
    if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_330)
    {
        /* Set 1.8V Signal Enable in the Host Control2 register to 0 */
        ctrl &= ~SDHCI_CTRL_VDD_180;
        sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

//              if (host->mmc->caps3 & MMC_CAP3_CONTROL_VOLTAGE)
//                      GPREG(VOLCTRL) &= ~(1<<0);

        /* Wait for 5ms */
        mdelay(5);

        /* 3.3V regulator output should be stable within 5 ms */
        ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
        if (!(ctrl & SDHCI_CTRL_VDD_180))
            return 0;
        else
        {
            printf(DRIVER_NAME ": Switching to 3.3V "
                   "signalling voltage failed\n");
            return COMM_ERR;
        }
    }
    else if (!(ctrl & SDHCI_CTRL_VDD_180) &&
             (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180))
    {
        /* Stop SDCLK */
        clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
        clk &= ~SDHCI_CLOCK_CARD_EN;
        sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

        /* Check whether DAT[3:0] is 0000 */
        present_state = sdhci_readl(host, SDHCI_PRESENT_STATE);
        if (!((present_state & SDHCI_DATA_LVL_MASK) >> SDHCI_DATA_LVL_SHIFT))
        {
            /*
             * Enable 1.8V Signal Enable in the Host Control2
             * register
             */
            ctrl |= SDHCI_CTRL_VDD_180;
            sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

//                      if (host->mmc->caps3 & MMC_CAP3_CONTROL_VOLTAGE)
//                              GPREG(VOLCTRL) |= (1<<0);

            /* Wait for 5ms */
            mdelay(5);

            ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
            if (ctrl & SDHCI_CTRL_VDD_180)
            {
                /* Provide SDCLK again and wait for 1ms */
                clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
                clk |= SDHCI_CLOCK_CARD_EN;
                sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
                mdelay(1);

                /*
                 * If DAT[3:0] level is 1111b, then the card
                 * was successfully switched to 1.8V signaling.
                 */
                present_state = sdhci_readl(host, SDHCI_PRESENT_STATE);
                if ((present_state & SDHCI_DATA_LVL_MASK) ==
                    SDHCI_DATA_LVL_MASK)
                    return 0;
            }
        }

        /*
         * If we are here, that means the switch to 1.8V signaling
         * failed. We power cycle the card, and retry initialization
         * sequence by setting S18R to 0.
         */
        pwr = sdhci_readb(host, SDHCI_POWER_CONTROL);
        pwr &= ~SDHCI_POWER_ON;
        sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);

        /* Wait for 1ms as per the spec */
        mdelay(1);
        pwr |= SDHCI_POWER_ON;
        sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);

        printf(DRIVER_NAME ": Switching to 1.8V signalling "
               "voltage failed, retrying with S18R set to 0\n");
        return COMM_ERR;
    }
    else
        /* No signal voltage switch required */
        return 0;
}

static void sdhci_do_enable_preset_value(struct sdhci_host *host, int enable)
{
    u16 ctrl;

    /* Host Controller v3.00 defines preset value registers */
    if (SDHCI_GET_VERSION(host) < SDHCI_SPEC_300)
        return;

    ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);

    /*
     * We only enable or disable Preset Value if they are not already
     * enabled or disabled respectively. Otherwise, we bail out.
     */
    if (enable && !(ctrl & SDHCI_CTRL_PRESET_VAL_ENABLE))
    {
        ctrl |= SDHCI_CTRL_PRESET_VAL_ENABLE;
        sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
    }
    else if (!enable && (ctrl & SDHCI_CTRL_PRESET_VAL_ENABLE))
    {
        ctrl &= ~SDHCI_CTRL_PRESET_VAL_ENABLE;
        sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
    }
}

static void sdhci_enable_preset_value(struct mmc *mmc, int enable)
{
    struct sdhci_host *host = mmc->priv;

    sdhci_do_enable_preset_value(host, enable);

    if (1)                      // SDHCI_QUIRK2_RESET_CLOCK_AFTER_PRESET
    {
        unsigned int clock;
        u16 clk = 0;

        /* Reset SD Clock Enable */
        clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
        clk &= ~SDHCI_CLOCK_CARD_EN;
        sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

        /* Re-enable SD Clock */
        clock = host->clock;
        host->clock = 0;
        sdhci_set_clock(mmc, clock);
    }
}
#endif

static const struct mmc_ops sdhci_ops = {
    .send_cmd = sdhci_send_command,
    .set_ios = sdhci_set_ios,
    .init = sdhci_init,
    .getcd = sdhci_getcd,
    .getwp = sdhci_getwp,
    .reset = sdhci_reset_all,
#ifdef CONFIG_MMC_UHS
    .start_signal_voltage_switch = sdhci_start_signal_voltage_switch,
    .enable_preset_value = sdhci_enable_preset_value,
#endif
};

int add_sdhci(struct sdhci_host *host, u32 max_clk, u32 min_clk)
{
    unsigned int caps, caps1;

    host->cfg.name = host->name;
    host->cfg.ops = &sdhci_ops;

    sdhci_reset(host, SDHCI_RESET_ALL);

    host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

    caps = sdhci_readl(host, SDHCI_CAPABILITIES);
    caps1 = sdhci_readl(host, SDHCI_CAPABILITIES_1);

    if (max_clk)
        host->cfg.f_max = max_clk;
    else
    {
        if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
            host->cfg.f_max = (caps & SDHCI_CLOCK_V3_BASE_MASK)
                >> SDHCI_CLOCK_BASE_SHIFT;
        else
            host->cfg.f_max = (caps & SDHCI_CLOCK_BASE_MASK)
                >> SDHCI_CLOCK_BASE_SHIFT;
        host->cfg.f_max *= 1000000;
    }
    if (host->cfg.f_max == 0)
    {
        printf("%s: Hardware doesn't specify base clock frequency\n", __func__);
        return -1;
    }
    if (min_clk)
        host->cfg.f_min = min_clk;
    else
    {
        if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
            host->cfg.f_min = host->cfg.f_max / SDHCI_MAX_DIV_SPEC_300;
        else
            host->cfg.f_min = host->cfg.f_max / SDHCI_MAX_DIV_SPEC_200;
    }

    host->cfg.voltages = 0;
    if (caps & SDHCI_CAN_VDD_330)
        host->cfg.voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;
    if (caps & SDHCI_CAN_VDD_300)
        host->cfg.voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
    if (caps & SDHCI_CAN_VDD_180)
        host->cfg.voltages |= MMC_VDD_165_195;

    if (host->quirks & SDHCI_QUIRK_BROKEN_VOLTAGE)
        host->cfg.voltages |= host->voltages;

    host->cfg.host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT;
    if (SDHCI_GET_VERSION(host) >= SDHCI_SPEC_300)
    {
        if (caps & SDHCI_CAN_DO_8BIT)
            host->cfg.host_caps |= MMC_MODE_8BIT;
    }
    /* Any UHS-I mode in caps implies SDR12 and SDR25 support. */
    if (caps1 &
        (SDHCI_SUPPORT_SDR104 | SDHCI_SUPPORT_SDR50 | SDHCI_SUPPORT_DDR50))
        host->cfg.host_caps |= MMC_MODE_UHS_SDR12 | MMC_MODE_UHS_SDR25;
    /* SDR104 supports also implies SDR50 support */
    if (caps1 & SDHCI_SUPPORT_SDR104)
        host->cfg.host_caps |= MMC_MODE_UHS_SDR104 | MMC_MODE_UHS_SDR50;
    else if (caps1 & SDHCI_SUPPORT_SDR50)
        host->cfg.host_caps |= MMC_MODE_UHS_SDR50;
    if (caps1 & SDHCI_SUPPORT_DDR50)
        host->cfg.host_caps |= MMC_MODE_UHS_DDR50;
    if (host->host_caps)
        host->cfg.host_caps |= host->host_caps;
#ifndef CONFIG_MMC_SUPPORT_SDR104
    host->cfg.host_caps &= ~MMC_MODE_UHS_SDR104;
#endif
#ifndef CONFIG_MMC_SUPPORT_DDR50
    host->cfg.host_caps &= ~MMC_MODE_UHS_DDR50;
#endif

    host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

    sdhci_reset(host, SDHCI_RESET_ALL);

    host->mmc = mmc_create(&host->cfg, host);
    if (host->mmc == NULL)
    {
        printf("%s: mmc create fail!\n", __func__);
        return -1;
    }

    sdhci_init(host->mmc);

    return 0;
}
