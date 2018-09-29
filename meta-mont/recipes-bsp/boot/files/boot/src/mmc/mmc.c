/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file mmc.c
*   \brief MMC Driver
*   \author Montage
*/
#include <arch/cpu.h>
#include <common.h>
#include <mmc.h>

struct mmc mmc_dev;

#ifdef CONFIG_MMC_UHS
int mmc_set_signal_voltage(struct mmc *mmc, int signal_voltage, int cmd11)
{
    int err;
    struct mmc_cmd cmd;

    if ((signal_voltage != MMC_SIGNAL_VOLTAGE_330) && cmd11)
    {
        cmd.cmdidx = SD_SWITCH_VOLTAGE;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);
        if (err)
            return err;

        if (!mmc_host_is_spi(mmc) && (cmd.response[0] & MMC_STATUS_ERROR))
            return COMM_ERR;
    }

    mmc->signal_voltage = signal_voltage;
    if (mmc->cfg->ops->start_signal_voltage_switch)
        err = mmc->cfg->ops->start_signal_voltage_switch(mmc);

    return err;
}
#endif

int mmc_getwp(struct mmc *mmc)
{
    int wp;

    if (mmc->cfg->ops->getwp)
        wp = mmc->cfg->ops->getwp(mmc);
    else
        wp = 0;

    return wp;
}

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    int ret;

#ifdef CONFIG_MMC_TRACE
    int i;
    u8 *ptr;

    printf("CMD_SEND:%d\n", cmd->cmdidx);
    printf("\t\tARG\t\t\t 0x%08X\n", cmd->cmdarg);
    ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
    switch (cmd->resp_type)
    {
        case MMC_RSP_NONE:
            printf("\t\tMMC_RSP_NONE\n");
            break;
        case MMC_RSP_R1:
            printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%08X \n", cmd->response[0]);
            break;
        case MMC_RSP_R1b:
            printf("\t\tMMC_RSP_R1b\t\t 0x%08X \n", cmd->response[0]);
            break;
        case MMC_RSP_R2:
            printf("\t\tMMC_RSP_R2\t\t 0x%08X \n", cmd->response[0]);
            printf("\t\t          \t\t 0x%08X \n", cmd->response[1]);
            printf("\t\t          \t\t 0x%08X \n", cmd->response[2]);
            printf("\t\t          \t\t 0x%08X \n", cmd->response[3]);
            printf("\n");
            printf("\t\t\t\t\tDUMPING DATA\n");
            for (i = 0; i < 4; i++)
            {
                int j;
                printf("\t\t\t\t\t%03d - ", i * 4);
                ptr = (u8 *) & cmd->response[i];
                ptr += 3;
                for (j = 0; j < 4; j++)
                    printf("%02X ", *ptr--);
                printf("\n");
            }
            break;
        case MMC_RSP_R3:
            printf("\t\tMMC_RSP_R3,4\t\t 0x%08X \n", cmd->response[0]);
            break;
        default:
            printf("\t\tERROR MMC rsp not supported\n");
            break;
    }
#else
    ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
#endif
    return ret;
}

int mmc_send_status(struct mmc *mmc, int timeout)
{
    struct mmc_cmd cmd;
    int err, retries = 5;
#ifdef CONFIG_MMC_TRACE
    int status;
#endif

    cmd.cmdidx = MMC_CMD_SEND_STATUS;
    cmd.resp_type = MMC_RSP_R1;
    if (!mmc_host_is_spi(mmc))
        cmd.cmdarg = mmc->rca << 16;

    do
    {
        err = mmc_send_cmd(mmc, &cmd, NULL);
        if (!err)
        {
            if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
                (cmd.response[0] & MMC_STATUS_CURR_STATE) != MMC_STATE_PRG)
                break;
            else if (cmd.response[0] & MMC_STATUS_MASK)
            {
                printf("Status Error: 0x%08X\n", cmd.response[0]);
                return COMM_ERR;
            }
        }
        else if (--retries < 0)
            return err;

        mdelay(1);

    }
    while (timeout--);

#ifdef CONFIG_MMC_TRACE
    status = (cmd.response[0] & MMC_STATUS_CURR_STATE) >> 9;
    printf("CURR STATE:%d\n", status);
#endif
    if (timeout <= 0)
    {
        printf("Timeout waiting card ready\n");
        return TIMEOUT;
    }

    return 0;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
    struct mmc_cmd cmd;

    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = len;

    return mmc_send_cmd(mmc, &cmd, NULL);
}

struct mmc *find_mmc_device(int dev_num)
{
    struct mmc *mmc = &mmc_dev;
    if (mmc->has_init)
        return mmc;
    else
        return NULL;
}

static u32 mmc_read_blocks(struct mmc *mmc, void *dst, u32 start, u32 blkcnt)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    if (blkcnt > 1)
        cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
    else
        cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

    if (mmc->high_capacity)
        cmd.cmdarg = start;
    else
        cmd.cmdarg = start * mmc->read_bl_len;

    cmd.resp_type = MMC_RSP_R1;

    data.dest = dst;
    data.blocks = blkcnt;
    data.blocksize = mmc->read_bl_len;
    data.flags = MMC_DATA_READ;

    if (mmc_send_cmd(mmc, &cmd, &data))
        return 0;

    if (blkcnt > 1)
    {
        cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
        cmd.cmdarg = 0;
        cmd.resp_type = MMC_RSP_R1b;
        if (mmc_send_cmd(mmc, &cmd, NULL))
        {
            printf("mmc fail to send stop cmd\n");
            return 0;
        }
    }

    return blkcnt;
}

static int mmc_bread(int dev_num, u32 start, u32 blkcnt, void *dst)
{
    u32 cur, blocks_todo = blkcnt;

    if (blkcnt == 0)
        return -1;

    struct mmc *mmc = find_mmc_device(dev_num);
    if (!mmc)
        return -1;

    if (mmc_set_blocklen(mmc, mmc->read_bl_len))
        return -1;

    do
    {
        cur = (blocks_todo > mmc->cfg->b_max) ? mmc->cfg->b_max : blocks_todo;
        if (mmc_read_blocks(mmc, dst, start, cur) != cur)
            return -1;
        blocks_todo -= cur;
        start += cur;
        dst += cur * mmc->read_bl_len;
    }
    while (blocks_todo > 0);

    return 0;
}

static int mmc_erase_t(struct mmc *mmc, u32 start, u32 blkcnt)
{
    struct mmc_cmd cmd;
    u32 end;
    int err, start_cmd, end_cmd;

    if (mmc->high_capacity)
    {
        end = start + blkcnt - 1;
    }
    else
    {
        end = (start + blkcnt - 1) * mmc->write_bl_len;
        start *= mmc->write_bl_len;
    }

    if (IS_SD(mmc))
    {
        start_cmd = SD_CMD_ERASE_WR_BLK_START;
        end_cmd = SD_CMD_ERASE_WR_BLK_END;
    }
    else
    {
        start_cmd = MMC_CMD_ERASE_GROUP_START;
        end_cmd = MMC_CMD_ERASE_GROUP_END;
    }

    cmd.cmdidx = start_cmd;
    cmd.cmdarg = start;
    cmd.resp_type = MMC_RSP_R1;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    cmd.cmdidx = end_cmd;
    cmd.cmdarg = end;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    cmd.cmdidx = MMC_CMD_ERASE;
    cmd.cmdarg = SECURE_ERASE;
    cmd.resp_type = MMC_RSP_R1b;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    return 0;

  err_out:
    puts("mmc erase failed\n");
    return err;
}

int mmc_berase(int dev_num, u32 start, u32 blkcnt)
{
    int err = 0;
    struct mmc *mmc = find_mmc_device(dev_num);
    u32 blk = 0, blk_r = 0;
    int timeout = 1000;

    if (!mmc)
        return -1;

    if ((start % mmc->erase_grp_size) || (blkcnt % mmc->erase_grp_size))
        printf("\n\nCaution! Your devices Erase group is 0x%x\n"
               "The erase range would be change to "
               "0x%llx~0x%llx\n\n",
               mmc->erase_grp_size, start & ~(mmc->erase_grp_size - 1),
               ((start + blkcnt + mmc->erase_grp_size)
                & ~(mmc->erase_grp_size - 1)) - 1);

    while (blk < blkcnt)
    {
        blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
            mmc->erase_grp_size : (blkcnt - blk);
        err = mmc_erase_t(mmc, start + blk, blk_r);
        if (err)
            break;

        blk += blk_r;

        /* Waiting for the ready status */
        if (mmc_send_status(mmc, timeout))
            return -1;
    }

    return 0;
}

static u32 mmc_write_blocks(struct mmc *mmc, u32 start,
                            u32 blkcnt, const void *src)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    int timeout = 1000;

    if (blkcnt == 0)
        return 0;
    else if (blkcnt == 1)
        cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
    else
        cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

    if (mmc->high_capacity)
        cmd.cmdarg = start;
    else
        cmd.cmdarg = start * mmc->write_bl_len;

    cmd.resp_type = MMC_RSP_R1;

    data.src = src;
    data.blocks = blkcnt;
    data.blocksize = mmc->write_bl_len;
    data.flags = MMC_DATA_WRITE;

    if (mmc_send_cmd(mmc, &cmd, &data))
    {
        printf("mmc write failed\n");
        return 0;
    }

    /* SPI multiblock writes terminate using a special
     * token, not a STOP_TRANSMISSION request.
     */
    if (!mmc_host_is_spi(mmc) && blkcnt > 1)
    {
        cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
        cmd.cmdarg = 0;
        cmd.resp_type = MMC_RSP_R1b;
        if (mmc_send_cmd(mmc, &cmd, NULL))
        {
            printf("mmc fail to send stop cmd\n");
            return 0;
        }
    }

    /* Waiting for the ready status */
    if (mmc_send_status(mmc, timeout))
        return 0;

    return blkcnt;
}

int mmc_bwrite(int dev_num, u32 start, u32 blkcnt, const void *src)
{
    u32 cur, blocks_todo = blkcnt;

    struct mmc *mmc = find_mmc_device(dev_num);
    if (!mmc)
        return -1;

    if (mmc_set_blocklen(mmc, mmc->write_bl_len))
        return -1;

    do
    {
        cur = (blocks_todo > mmc->cfg->b_max) ? mmc->cfg->b_max : blocks_todo;
        if (mmc_write_blocks(mmc, start, cur, src) != cur)
            return -1;
        blocks_todo -= cur;
        start += cur;
        src += cur * mmc->write_bl_len;
    }
    while (blocks_todo > 0);

    return 0;
}

static int mmc_go_idle(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err;

    mdelay(1);

    cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_NONE;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    mdelay(100);

    return 0;
}

static int sd_send_op_cond(struct mmc *mmc)
{
    int timeout;
    int err;
    struct mmc_cmd cmd;
#ifdef CONFIG_MMC_UHS
    u32 ocr = OCR_S18R;

  try_again:
#endif
    timeout = 1000;
    do
    {
        cmd.cmdidx = MMC_CMD_APP_CMD;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
        cmd.resp_type = MMC_RSP_R3;

        /*
         * Most cards do not answer if some reserved bits
         * in the ocr are set. However, Some controller
         * can set bit 7 (reserved for low voltages), but
         * how to manage low voltages SD card is not yet
         * specified.
         */
        cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 : (mmc->cfg->voltages & 0xff8000);

        if (mmc->version == SD_VERSION_2)
            cmd.cmdarg |= OCR_HCS;

#ifdef CONFIG_MMC_UHS
        cmd.cmdarg |= ocr;
#endif

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        mdelay(5);
    }
    while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

    if (timeout <= 0)
        return UNUSABLE_ERR;

    if (mmc->version != SD_VERSION_2)
        mmc->version = SD_VERSION_1_0;

    if (mmc_host_is_spi(mmc))
    {                           /* read OCR for spi */
        cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
        cmd.resp_type = MMC_RSP_R3;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;
    }

    mmc->ocr = cmd.response[0];

#ifdef CONFIG_MMC_UHS
    if ((mmc->ocr & 0x41000000) == 0x41000000)
    {
        err = mmc_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_180, 1);
        if (err)
        {
            ocr = 0;
            goto try_again;
        }
    }
#endif

    mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
    mmc->rca = 0;

    return 0;
}

/* We pass in the cmd since otherwise the init seems to fail */
static int mmc_send_op_cond_iter(struct mmc *mmc, struct mmc_cmd *cmd,
                                 int use_arg)
{
    int err;

    cmd->cmdidx = MMC_CMD_SEND_OP_COND;
    cmd->resp_type = MMC_RSP_R3;
    cmd->cmdarg = 0;
    if (use_arg && !mmc_host_is_spi(mmc))
    {
        cmd->cmdarg =
            (mmc->cfg->voltages &
             (mmc->op_cond_response & OCR_VOLTAGE_MASK)) |
            (mmc->op_cond_response & OCR_ACCESS_MODE);

        if (mmc->cfg->host_caps & MMC_MODE_HC)
            cmd->cmdarg |= OCR_HCS;
    }
    err = mmc_send_cmd(mmc, cmd, NULL);
    if (err)
        return err;
    mmc->op_cond_response = cmd->response[0];
    return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err, i;

    /* Some cards seem to need this */
    mmc_go_idle(mmc);

    /* Asking to the card its capabilities */
    mmc->op_cond_pending = 1;
    /* only query ocr, not to set */
    for (i = 0; i < 1; i++)
    {
        err = mmc_send_op_cond_iter(mmc, &cmd, i != 0);
        if (err)
            return err;

        /* exit if not busy (flag seems to be inverted) */
        if (mmc->op_cond_response & OCR_BUSY)
            return 0;
    }
    return IN_PROGRESS;
}

int mmc_complete_op_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int timeout = 1000;
    u32 start;
    int err;

    mmc->op_cond_pending = 0;
    start = how_long(0);
    do
    {
        err = mmc_send_op_cond_iter(mmc, &cmd, 1);
        if (err)
            return err;
        if (how_long(start) > timeout)
            return UNUSABLE_ERR;
        mdelay(1);
    }
    while (!(mmc->op_cond_response & OCR_BUSY));

    if (mmc_host_is_spi(mmc))
    {                           /* read OCR for spi */
        cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
        cmd.resp_type = MMC_RSP_R3;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;
    }

    mmc->version = MMC_VERSION_UNKNOWN;
    mmc->ocr = cmd.response[0];

    mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
    mmc->rca = 1;

    return 0;
}

static int mmc_send_ext_csd(struct mmc *mmc, u8 * ext_csd)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    int err;

    /* Get the Card Status Register */
    cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    data.dest = (char *) ext_csd;
    data.blocks = 1;
    data.blocksize = MMC_MAX_BLOCK_LEN;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd(mmc, &cmd, &data);

    return err;
}

static int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
    struct mmc_cmd cmd;
    int timeout = 1000;
    int ret;

    cmd.cmdidx = MMC_CMD_SWITCH;
    cmd.resp_type = MMC_RSP_R1b;
    cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
        (index << 16) | (value << 8) | set;

    ret = mmc_send_cmd(mmc, &cmd, NULL);

    /* Waiting for the ready status */
    if (!ret)
        ret = mmc_send_status(mmc, timeout);

    return ret;

}

static int mmc_change_freq(struct mmc *mmc)
{
    ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
    char cardtype;
    int err;

    mmc->card_caps = 0;

    if (mmc_host_is_spi(mmc))
        return 0;

    /* Only version 4 supports high-speed */
    if (mmc->version < MMC_VERSION_4)
        return 0;

    err = mmc_send_ext_csd(mmc, ext_csd);

    if (err)
        return err;

    cardtype = ext_csd[EXT_CSD_CARD_TYPE] & 0xf;

    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

    if (err)
        return err;

    /* Now check to see that it worked */
    err = mmc_send_ext_csd(mmc, ext_csd);

    if (err)
        return err;

    /* No high-speed support */
    if (!ext_csd[EXT_CSD_HS_TIMING])
        return 0;

    /* High Speed is set, there are two types: 52MHz and 26MHz */
    if (cardtype & MMC_HS_52MHZ)
        mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
    else
        mmc->card_caps |= MMC_MODE_HS;

    return 0;
}

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
    switch (part_num)
    {
        case 0:
            mmc->capacity = mmc->capacity_user;
            break;
        case 1:
        case 2:
            mmc->capacity = mmc->capacity_boot;
            break;
        case 3:
            mmc->capacity = mmc->capacity_rpmb;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            mmc->capacity = mmc->capacity_gp[part_num - 4];
            break;
        default:
            return -1;
    }

    return 0;
}

int mmc_switch_part(int dev_num, unsigned int part_num)
{
    struct mmc *mmc = find_mmc_device(dev_num);
    int ret;

    if (!mmc)
        return -1;

    ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
                     (mmc->part_config & ~PART_ACCESS_MASK)
                     | (part_num & PART_ACCESS_MASK));
    if (ret)
        return ret;

    return mmc_set_capacity(mmc, part_num);
}

int mmc_getcd(struct mmc *mmc)
{
    int cd;

    if (mmc->cfg->ops->getcd)
        cd = mmc->cfg->ops->getcd(mmc);
    else
        cd = 1;

    return cd;
}

static int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 * resp)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    /* Switch the frequency */
    cmd.cmdidx = SD_CMD_SWITCH_FUNC;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = (mode << 31) | 0xffffff;
    cmd.cmdarg &= ~(0xf << (group * 4));
    cmd.cmdarg |= value << (group * 4);

    data.dest = (char *) resp;
    data.blocksize = 64;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    return mmc_send_cmd(mmc, &cmd, &data);
}

static int sd_set_bus_width(struct mmc *mmc, int width)
{
    int err;
    struct mmc_cmd cmd;

    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;

    cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
    cmd.resp_type = MMC_RSP_R1;
    switch (width)
    {
        case MMC_BUS_WIDTH_1:
            cmd.cmdarg = 0;
            break;
        case MMC_BUS_WIDTH_4:
            cmd.cmdarg = 2;
            break;
        default:
            return COMM_ERR;
    }
    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        return err;
}

static int sd_read_scr(struct mmc *mmc)
{
    int err;
    struct mmc_cmd cmd;
    ALLOC_CACHE_ALIGN_BUFFER(u32, scr, 2);
    struct mmc_data data;
    int timeout = 3;

    /* Read the SCR to find out if this card supports higher speeds */
    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    cmd.cmdidx = SD_CMD_APP_SEND_SCR;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    timeout = 3;

  retry_scr:
    data.dest = (char *) scr;
    data.blocksize = 8;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd(mmc, &cmd, &data);

    if (err)
    {
        if (timeout--)
            goto retry_scr;

        return err;
    }

    mmc->scr[0] = __be32_to_cpu(scr[0]);
    mmc->scr[1] = __be32_to_cpu(scr[1]);

    return 0;
}

static int mmc_read_switch(struct mmc *mmc)
{
    int err;
    struct mmc_cmd cmd;
    ALLOC_CACHE_ALIGN_BUFFER(u32, switch_status, 16);
    int timeout = 4;

    while (timeout--)
    {
        /* between SD_CMD_APP_SEND_SCR and SD_SWITCH_CHECK, to wait for a moment */
        mdelay(1);
        err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 0, (u8 *) switch_status);

        if (err)
            return err;

        /* The high-speed function is busy.  Try again */
        if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
            break;
    }

    if (!timeout)
        return TIMEOUT;

    mmc->bus_mode = __be32_to_cpu(switch_status[3]) >> 16;

    if (mmc->version == SD_VERSION_3)
        mmc->bus_mode &= 0x1F;
    else
        mmc->bus_mode &= 0x03;

    return 0;
}

static int sd_change_freq(struct mmc *mmc)
{
    int err;
    ALLOC_CACHE_ALIGN_BUFFER(u32, switch_status, 16);

    mmc->card_caps = 0;
    mmc->bus_mode = 0;
    mmc->bus_speed = 0;

    if (mmc_host_is_spi(mmc))
        return 0;

    err = sd_read_scr(mmc);

    if (err)
        return err;

    switch ((mmc->scr[0] >> 24) & 0xf)
    {
        case 0:
            mmc->version = SD_VERSION_1_0;
            break;
        case 1:
            mmc->version = SD_VERSION_1_10;
            break;
        case 2:
            mmc->version = SD_VERSION_2;
            if ((mmc->scr[0] >> 15) & 0x1)
                mmc->version = SD_VERSION_3;
            break;
        default:
            mmc->version = SD_VERSION_1_0;
            break;
    }

    if (mmc->scr[0] & SD_DATA_4BIT)
        mmc->card_caps |= MMC_MODE_4BIT;

    /* Version 1.0 doesn't support switching */
    if (mmc->version == SD_VERSION_1_0)
        return 0;

    /* Fetch switch information from card */
    err = mmc_read_switch(mmc);

    if (err)
        return err;

#ifdef CONFIG_MMC_UHS
    if (!(mmc->cfg->host_caps & (MMC_MODE_UHS_SDR12 | MMC_MODE_UHS_SDR25 |
                                 MMC_MODE_UHS_SDR50 | MMC_MODE_UHS_SDR104 |
                                 MMC_MODE_UHS_DDR50)))
    {
        mmc->ocr &= ~OCR_S18R;
    }
    if (mmc->ocr & OCR_S18R)
    {
        if ((mmc->cfg->host_caps & MMC_MODE_UHS_SDR104) &&
            (mmc->bus_mode & SD_MODE_UHS_SDR104))
        {
            mmc->bus_mode = MMC_MODE_UHS_SDR104;
            mmc->bus_speed = UHS_SDR104_BUS_SPEED;
        }
        else if ((mmc->cfg->host_caps & MMC_MODE_UHS_DDR50) &&
                 (mmc->bus_mode & SD_MODE_UHS_DDR50))
        {
            mmc->bus_mode = MMC_MODE_UHS_DDR50;
            mmc->bus_speed = UHS_DDR50_BUS_SPEED;
        }
        else if ((mmc->
                  cfg->host_caps & (MMC_MODE_UHS_SDR104 | MMC_MODE_UHS_SDR50))
                 && (mmc->bus_mode & SD_MODE_UHS_SDR50))
        {
            mmc->bus_mode = MMC_MODE_UHS_SDR50;
            mmc->bus_speed = UHS_SDR50_BUS_SPEED;
        }
        else if ((mmc->
                  cfg->host_caps & (MMC_MODE_UHS_SDR104 | MMC_MODE_UHS_SDR50 |
                                    MMC_MODE_UHS_SDR25))
                 && (mmc->bus_mode & SD_MODE_UHS_SDR25))
        {
            mmc->bus_mode = MMC_MODE_UHS_SDR25;
            mmc->bus_speed = UHS_SDR25_BUS_SPEED;
        }
        else if ((mmc->
                  cfg->host_caps & (MMC_MODE_UHS_SDR104 | MMC_MODE_UHS_SDR50 |
                                    MMC_MODE_UHS_SDR25 | MMC_MODE_UHS_SDR12))
                 && (mmc->bus_mode & SD_MODE_UHS_SDR12))
        {
            mmc->bus_mode = MMC_MODE_UHS_SDR12;
            mmc->bus_speed = UHS_SDR12_BUS_SPEED;
        }

        /* between SD_SWITCH_CHECK and SD_SWITCH_SWITCH, to wait for a moment */
        mdelay(1);
        err =
            sd_switch(mmc, SD_SWITCH_SWITCH, 0, mmc->bus_speed,
                      (u8 *) switch_status);

        if (err)
            return err;

        if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) ==
            (mmc->bus_speed << 24))
        {
            mmc->card_caps |= mmc->bus_mode;
        }
        else
        {
            printf("Problem setting bus speed mode!\n");
        }
    }
    else
#endif
    {
        /*
         * If the host doesn't support SD_HIGHSPEED, do not switch card to
         * HIGHSPEED mode even if the card support SD_HIGHSPPED.
         * This can avoid furthur problem when the card runs in different
         * mode between the host.
         */
        if (!((mmc->cfg->host_caps & MMC_MODE_HS_52MHz) &&
              (mmc->cfg->host_caps & MMC_MODE_HS)))
            return 0;

        /* between SD_SWITCH_CHECK and SD_SWITCH_SWITCH, to wait for a moment */
        mdelay(1);
        err =
            sd_switch(mmc, SD_SWITCH_SWITCH, 0, HIGH_SPEED_BUS_SPEED,
                      (u8 *) switch_status);

        if (err)
            return err;

        if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000)
            mmc->card_caps |= MMC_MODE_HS;
    }

    return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
    10000,
    100000,
    1000000,
    10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
    0,                          /* reserved */
    10,
    12,
    13,
    15,
    20,
    25,
    30,
    35,
    40,
    45,
    50,
    55,
    60,
    70,
    80,
};

static void mmc_set_ios(struct mmc *mmc)
{
    if (mmc->cfg->ops->set_ios)
        mmc->cfg->ops->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, u32 clock)
{
    if (clock > mmc->cfg->f_max)
        clock = mmc->cfg->f_max;

    if (clock < mmc->cfg->f_min)
        clock = mmc->cfg->f_min;

    mmc->clock = clock;

    mmc_set_ios(mmc);
}

static void mmc_set_bus_width(struct mmc *mmc, u32 width)
{
    mmc->bus_width = width;

    mmc_set_ios(mmc);
}

static int mmc_startup(struct mmc *mmc)
{
    int err, i;
    u32 mult, freq;
    u64 cmult, csize, capacity;
    struct mmc_cmd cmd;
    ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
    ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, MMC_MAX_BLOCK_LEN);
    int timeout = 1000;

#ifdef CONFIG_MMC_SPI_CRC_ON
    if (mmc_host_is_spi(mmc))
    {                           /* enable CRC check for spi */
        cmd.cmdidx = MMC_CMD_SPI_CRC_ON_OFF;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 1;
        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;
    }
#endif

    /* Put the Card in Identify Mode */
    cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID : MMC_CMD_ALL_SEND_CID;        /* cmd not supported in spi */
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    memcpy(mmc->cid, cmd.response, 16);

    /*
     * For MMC cards, set the Relative Address.
     * For SD cards, get the Relatvie Address.
     * This also puts the cards into Standby State
     */
    if (!mmc_host_is_spi(mmc))
    {                           /* cmd not supported in spi */
        cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
        cmd.cmdarg = mmc->rca << 16;
        cmd.resp_type = MMC_RSP_R6;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        if (IS_SD(mmc))
            mmc->rca = (cmd.response[0] >> 16) & 0xffff;
    }

    /* Get the Card-Specific Data */
    cmd.cmdidx = MMC_CMD_SEND_CSD;
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    /* Waiting for the ready status */
    mmc_send_status(mmc, timeout);

    if (err)
        return err;

    mmc->csd[0] = cmd.response[0];
    mmc->csd[1] = cmd.response[1];
    mmc->csd[2] = cmd.response[2];
    mmc->csd[3] = cmd.response[3];

    if (mmc->version == MMC_VERSION_UNKNOWN)
    {
        int version = (cmd.response[0] >> 26) & 0xf;

        switch (version)
        {
            case 0:
                mmc->version = MMC_VERSION_1_2;
                break;
            case 1:
                mmc->version = MMC_VERSION_1_4;
                break;
            case 2:
                mmc->version = MMC_VERSION_2_2;
                break;
            case 3:
                mmc->version = MMC_VERSION_3;
                break;
            case 4:
                mmc->version = MMC_VERSION_4;
                break;
            default:
                mmc->version = MMC_VERSION_1_2;
                break;
        }
    }

    /* divide frequency by 10, since the mults are 10x bigger */
    freq = fbase[(cmd.response[0] & 0x7)];
    mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

    mmc->tran_speed = freq * mult;

    mmc->dsr_imp = ((cmd.response[1] >> 12) & 0x1);
    mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);

    if (IS_SD(mmc))
        mmc->write_bl_len = mmc->read_bl_len;
    else
        mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);

    if (mmc->high_capacity)
    {
        csize = (mmc->csd[1] & 0x3f) << 16 | (mmc->csd[2] & 0xffff0000) >> 16;
        cmult = 8;
    }
    else
    {
        csize = (mmc->csd[1] & 0x3ff) << 2 | (mmc->csd[2] & 0xc0000000) >> 30;
        cmult = (mmc->csd[2] & 0x00038000) >> 15;
    }

    mmc->capacity_user = (csize + 1) << (cmult + 2);
    mmc->capacity_user *= mmc->read_bl_len;
    mmc->capacity_boot = 0;
    mmc->capacity_rpmb = 0;
    for (i = 0; i < 4; i++)
        mmc->capacity_gp[i] = 0;

    if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
        mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

    if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
        mmc->write_bl_len = MMC_MAX_BLOCK_LEN;

    if ((mmc->dsr_imp) && (0xffffffff != mmc->dsr))
    {
        cmd.cmdidx = MMC_CMD_SET_DSR;
        cmd.cmdarg = (mmc->dsr & 0xffff) << 16;
        cmd.resp_type = MMC_RSP_NONE;
        if (mmc_send_cmd(mmc, &cmd, NULL))
            printf("MMC: SET_DSR failed\n");
    }

    /* Select the card, and put it into Transfer Mode */
    if (!mmc_host_is_spi(mmc))
    {                           /* cmd not supported in spi */
        cmd.cmdidx = MMC_CMD_SELECT_CARD;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = mmc->rca << 16;
        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;
    }

    /*
     * For SD, its erase group is always one sector
     */
    mmc->erase_grp_size = 1;
    mmc->part_config = MMCPART_NOAVAILABLE;
    if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4))
    {
        /* check  ext_csd version and capacity */
        err = mmc_send_ext_csd(mmc, ext_csd);
        if (!err && (ext_csd[EXT_CSD_REV] >= 2))
        {
            /*
             * According to the JEDEC Standard, the value of
             * ext_csd's capacity is valid if the value is more
             * than 2GB
             */
            capacity = ext_csd[EXT_CSD_SEC_CNT] << 0
                | ext_csd[EXT_CSD_SEC_CNT + 1] << 8
                | ext_csd[EXT_CSD_SEC_CNT + 2] << 16
                | ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
            capacity *= MMC_MAX_BLOCK_LEN;
            if ((capacity >> 20) > 2 * 1024)
                mmc->capacity_user = capacity;
        }

        switch (ext_csd[EXT_CSD_REV])
        {
            case 1:
                mmc->version = MMC_VERSION_4_1;
                break;
            case 2:
                mmc->version = MMC_VERSION_4_2;
                break;
            case 3:
                mmc->version = MMC_VERSION_4_3;
                break;
            case 5:
                mmc->version = MMC_VERSION_4_41;
                break;
            case 6:
                mmc->version = MMC_VERSION_4_5;
                break;
        }

        /*
         * Host needs to enable ERASE_GRP_DEF bit if device is
         * partitioned. This bit will be lost every time after a reset
         * or power off. This will affect erase size.
         */
        if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
            (ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB))
        {
            err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                             EXT_CSD_ERASE_GROUP_DEF, 1);

            if (err)
                return err;

            /* Read out group size from ext_csd */
            mmc->erase_grp_size =
                ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * MMC_MAX_BLOCK_LEN * 1024;
        }
        else
        {
            /* Calculate the group size from the csd value. */
            int erase_gsz, erase_gmul;
            erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
            erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
            mmc->erase_grp_size = (erase_gsz + 1) * (erase_gmul + 1);
        }

        /* store the partition info of emmc */
        if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
            ext_csd[EXT_CSD_BOOT_MULT])
            mmc->part_config = ext_csd[EXT_CSD_PART_CONF];

        mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

        mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

        for (i = 0; i < 4; i++)
        {
            int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
            mmc->capacity_gp[i] = (ext_csd[idx + 2] << 16) +
                (ext_csd[idx + 1] << 8) + ext_csd[idx];
            mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
            mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
        }
    }

    err = mmc_set_capacity(mmc, mmc->part_num);
    if (err)
        return err;

    if (IS_SD(mmc))
    {

        err = sd_change_freq(mmc);

        if (err)
            return err;

        /* Restrict card's capabilities by what the host can do */
        mmc->card_caps &= mmc->cfg->host_caps;

        if (mmc->card_caps & MMC_MODE_4BIT)
        {
            err = sd_set_bus_width(mmc, MMC_BUS_WIDTH_4);
            if (err)
                return err;

            mmc_set_bus_width(mmc, 4);
        }

        if (mmc->card_caps & MMC_MODE_HS)
            mmc->tran_speed = 50000000;
        else
            mmc->tran_speed = 25000000;

        mmc_set_clock(mmc, mmc->tran_speed);

#ifdef CONFIG_MMC_UHS
        if (mmc->card_caps &
            (MMC_MODE_UHS_SDR12 | MMC_MODE_UHS_SDR25 | MMC_MODE_UHS_SDR50 |
             MMC_MODE_UHS_SDR104 | MMC_MODE_UHS_DDR50))
        {

            if (mmc->cfg->ops->enable_preset_value)
                mmc->cfg->ops->enable_preset_value(mmc, 1);
        }
#endif
    }
    else
    {
        err = mmc_change_freq(mmc);

        if (err)
            return err;

        /* Restrict card's capabilities by what the host can do */
        mmc->card_caps &= mmc->cfg->host_caps;

        {
            int idx;

            /* An array of possible bus widths in order of preference */
            static unsigned ext_csd_bits[] = {
                EXT_CSD_BUS_WIDTH_8,
                EXT_CSD_BUS_WIDTH_4,
                EXT_CSD_BUS_WIDTH_1,
            };

            /* An array to map CSD bus widths to host cap bits */
            static unsigned ext_to_hostcaps[] = {
                [EXT_CSD_BUS_WIDTH_4] = MMC_MODE_4BIT,
                [EXT_CSD_BUS_WIDTH_8] = MMC_MODE_8BIT,
            };

            /* An array to map chosen bus width to an integer */
            static unsigned widths[] = {
                8, 4, 1,
            };

            for (idx = 0; idx < ARRAY_SIZE(ext_csd_bits); idx++)
            {
                unsigned int extw = ext_csd_bits[idx];

                /*
                 * Check to make sure the controller supports
                 * this bus width, if it's more than 1
                 */
                if (extw != EXT_CSD_BUS_WIDTH_1 &&
                    !(mmc->cfg->host_caps & ext_to_hostcaps[extw]))
                    continue;

                err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                                 EXT_CSD_BUS_WIDTH, extw);

                if (err)
                    continue;

                mmc_set_bus_width(mmc, widths[idx]);

                err = mmc_send_ext_csd(mmc, test_csd);
                if (!err && ext_csd[EXT_CSD_PARTITIONING_SUPPORT]
                    == test_csd[EXT_CSD_PARTITIONING_SUPPORT]
                    && ext_csd[EXT_CSD_ERASE_GROUP_DEF]
                    == test_csd[EXT_CSD_ERASE_GROUP_DEF]
                    && ext_csd[EXT_CSD_REV]
                    == test_csd[EXT_CSD_REV]
                    && ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
                    == test_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
                    && memcmp(&ext_csd[EXT_CSD_SEC_CNT],
                              &test_csd[EXT_CSD_SEC_CNT], 4) == 0)
                {

                    mmc->card_caps |= ext_to_hostcaps[extw];
                    break;
                }
            }

            if (mmc->card_caps & MMC_MODE_HS)
            {
                if (mmc->card_caps & MMC_MODE_HS_52MHz)
                    mmc->tran_speed = 52000000;
                else
                    mmc->tran_speed = 26000000;
            }

            mmc_set_clock(mmc, mmc->tran_speed);
        }
    }

    printf("Vendor: Man %06x Snr %04x%04x\n",
           mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
           (mmc->cid[3] >> 16) & 0xffff);
    printf("Product: %c%c%c%c%c%c\n", mmc->cid[0] & 0xff,
           (mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
           (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
           (mmc->cid[2] >> 24) & 0xff);
    printf("Revision: %d.%d\n", (mmc->cid[2] >> 20) & 0xf,
           (mmc->cid[2] >> 16) & 0xf);

    if (IS_SD(mmc))
    {
        if (mmc->card_caps & MMC_MODE_4BIT)
            printf("SD: 4 BIT ");
        else
            printf("SD: 1 BIT ");
#ifdef CONFIG_MMC_UHS
        if (mmc->card_caps & MMC_MODE_UHS_SDR12)
            printf("SDR12\n");
        else if (mmc->card_caps & MMC_MODE_UHS_SDR25)
            printf("SDR25\n");
        else if (mmc->card_caps & MMC_MODE_UHS_SDR50)
            printf("SDR50\n");
        else if (mmc->card_caps & MMC_MODE_UHS_SDR104)
            printf("SDR104\n");
        else if (mmc->card_caps & MMC_MODE_UHS_DDR50)
            printf("DDR50\n");
        else if (mmc->card_caps & MMC_MODE_HS)
            printf("%d Hz\n", mmc->tran_speed);
        else
#endif
            printf("%d Hz\n", mmc->tran_speed);
    }
    else
    {
        printf("MMC: %d BIT %d Hz\n", mmc->bus_width, mmc->tran_speed);
    }

    return 0;
}

static int mmc_send_if_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err;

    cmd.cmdidx = SD_CMD_SEND_IF_COND;
    /* We set the bit if the host supports voltages between 2.7 and 3.6 V */
    cmd.cmdarg = ((mmc->cfg->voltages & 0xff8000) != 0) << 8 | 0xaa;
    cmd.resp_type = MMC_RSP_R7;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    if ((cmd.response[0] & 0xff) != 0xaa)
        return UNUSABLE_ERR;
    else
        mmc->version = SD_VERSION_2;

    return 0;
}

struct mmc *mmc_create(const struct mmc_config *cfg, void *priv)
{
    struct mmc *mmc;

    /* quick validation */
    if (cfg == NULL || cfg->ops == NULL || cfg->ops->send_cmd == NULL ||
        cfg->f_min == 0 || cfg->f_max == 0 || cfg->b_max == 0)
        return NULL;

    mmc = &mmc_dev;
    mmc->cfg = cfg;
    mmc->priv = priv;

    /* Setup dsr related values */
    mmc->dsr_imp = 0;
    mmc->dsr = 0xffffffff;

    mmc->block_read = mmc_bread;
    mmc->block_write = mmc_bwrite;
    mmc->block_erase = mmc_berase;
    return mmc;
}

void mmc_destroy(struct mmc *mmc)
{
    memset(mmc, 0, sizeof (struct mmc));
}

int mmc_start_init(struct mmc *mmc)
{
    int err;

    if (mmc_getcd(mmc) == 0)
    {
        mmc->has_init = 0;
        printf("MMC: no card present\n");
        return NO_CARD_ERR;
    }

    if (mmc->has_init)
        return 0;

    mmc_set_clock(mmc, 1);
    mmc_set_bus_width(mmc, 1);

    /* Reset the Card */
    err = mmc_go_idle(mmc);

    if (err)
        return err;

    /* The internal partition reset to user partition(0) at every CMD0 */
    mmc->part_num = 0;

    /* Test for SD version 2 */
    err = mmc_send_if_cond(mmc);

    /* Now try to get the SD card's operating condition */
    err = sd_send_op_cond(mmc);

    /* If the command timed out, we check for an MMC card */
    if (err == TIMEOUT)
    {
        err = mmc_send_op_cond(mmc);

        if (err && err != IN_PROGRESS)
        {
            printf("Card did not respond to voltage select!\n");
            return UNUSABLE_ERR;
        }
    }

    if (err == IN_PROGRESS)
        mmc->init_in_progress = 1;

    return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
    int err = 0;

    if (mmc->op_cond_pending)
        err = mmc_complete_op_cond(mmc);

    if (!err)
        err = mmc_startup(mmc);
    if (err)
        mmc->has_init = 0;
    else
        mmc->has_init = 1;
    mmc->init_in_progress = 0;
    return err;
}

int mmc_init(struct mmc *mmc)
{
    int cnt = 20;
    int err = IN_PROGRESS;
    unsigned start = how_long(0);

    while (cnt--)
    {
        if (mmc->has_init)
            return 0;
        if (!mmc->init_in_progress)
            err = mmc_start_init(mmc);

        if (!err || err == IN_PROGRESS)
            err = mmc_complete_init(mmc);

        if (!err)
            break;
    }
    if (err)
        mmc_deinit(mmc);
    printf("%s: %d, time %lu\n", __func__, err, how_long(start));
    return err;
}

void mmc_deinit(struct mmc *mmc)
{
    mmc->has_init = 0;
    if (mmc->cfg->ops->reset)
        mmc->cfg->ops->reset(mmc);
    if (mmc->cfg->ops->init)
        mmc->cfg->ops->init(mmc);
}

int mmc_set_dsr(struct mmc *mmc, u16 val)
{
    mmc->dsr = val;
    return 0;
}
