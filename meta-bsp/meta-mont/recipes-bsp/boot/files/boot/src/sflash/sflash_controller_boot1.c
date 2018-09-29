/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file sflash_controller_boot1.c
*   \brief c sflash_controller for boot1
*   \author Montage
*/

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/ 
#define BOOT_MODE_BOOT1

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>

#include "include/cache.h"
#include "include/flash_config.h"
#include "include/flash_api.h"
#include "include/flash_db.h"
#include "include/pdma_driver.h"
#include "include/netprot.h"
#include "include/ddr_config.h"

#ifdef BOOT_MODE_IPL
#include <common/chip.h>
#else
#include <byteorder.h>
#include <arch/chip.h>
#endif

#ifdef BOOT_MODE_BOOT2
#include <image.h>
#include <ubimage.h>
#endif

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

#ifdef IPL_DEBUG_ENABLE
extern void panther_putc(char c);
#define PANTHER_CHAR(data)   panther_putc(data)
#else
#define PANTHER_CHAR(data)
#endif


#if defined(SIM)
#define sflash_udelay(n) cosim_delay_ns( (n) * 1000)
#elif !defined(BOOT_MODE_BOOT2)
#define udelay(n) {int _www; for (_www=0; _www< 100; _www++); }
#endif

#define CTRL_BUF_SIZE 64

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#ifdef CONFIG_PDMA
#ifdef PDMA_INTERRUPT
u32 pdma_status = PDMA_INTERRUPT;
#else
u32 pdma_status = PDMA_POLLING;
#endif
#endif

// aes key from "none", "register" or "otp": set default as from register
u32 aes_enable_status = DISABLE_SECURE;

flashdesc *pf_head = 0;
flashdesc flashcfg[1] = {
            {
                base:FLASH_BASE_CS0
            },
        };

u32 ctrl_buf[CTRL_BUF_SIZE / 4] __attribute__ ((aligned (32)));
u32 *pCtrl_buf;
u32 read_data = 0;

u32 is_enable_secure_boot = 0;

u32 is_ecc_cant_handle = 0;

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
u32 sflash_check_busy(drvarg * sfd, int loop);
u32 get_max_block_num(u32 pf_index);

u32 init_secure_settings(u32 action);
void read_otp_key(void);
void change_aes_enable_status(u32 aes_status);


/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
#ifdef SERIAL_FLASH_TEST
extern void nand_test(void);
#endif

#ifdef BOOT_MODE_BOOT2
extern u32 ch6_descr[4] __attribute__ ((aligned (32)));
#endif
extern u32 ch7_descr[4] __attribute__ ((aligned (32)));

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void sf_memcpy(void *dst, void *src, int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
    {
        d[i] = s[i];
    }
}

#if 0
void sf_flush_cache_all()
{
    HAL_DCACHE_WB_INVALIDATE_ALL();
}
#endif

/*!
 * function: spi_send_cmd
 *  \brief serial flash read word
 *  \param cmd1_ionum:
 *  \param cmd1_ddr_mode:
 *  \param cmd1_length:
 *  \param cmd0_ionum:
 *  \param cmd0_ddr_mode:
 *  \param cmd0_length:
 *  \param data_ionum:
 *  \param data_ddr_mode:
 *  \param trans_cnt:
 *  \param trans_dir: 2:Rx, 1:Tx, 3:Rx/Tx
 *  \param dummy_length:
 *  \param channel:
 *  \return
 */
void spi_send_cmd(u32 cmd1_ionum, u32 cmd1_ddr_mode, u32 cmd1_length, u32 cmd0_ionum,
                  u32 cmd0_ddr_mode, u32 cmd0_length, u32 data_ionum, u32 data_ddr_mode,
                  u32 trans_cnt, u32 trans_dir, u32 dummy_length, u32 channel)
{
    SPIREG(SPI_TC) = trans_cnt;
    SPIREG(SPI_CTRL) =
        (0x1 | (trans_dir << 1) | (dummy_length << 3) | (1 << (channel + 9)) |
         (data_ddr_mode << 13) | (data_ionum << 14) | (cmd0_length << 16) |
         (cmd0_ddr_mode << 21) | (cmd0_ionum << 22) | (cmd1_length << 24) |
         (cmd1_ddr_mode << 29) | (cmd1_ionum << 30));
}

/*!
 * function: spi_write_cmd
 *  \brief serial flash write command
 *  \param write_cnt: count (unit: 4yte)
 *  \return
 */
void spi_write_cmd(u32 write_cnt)
{
    if (write_cnt > CTRL_BUF_SIZE)
    {
        dbg_log(LOG_INFO, "Write invalid index of ctrl_buf\n");
        dbg_log(LOG_INFO, "Max size = %d, write index = %d\n", CTRL_BUF_SIZE, write_cnt);
    }

    u32 i, j;

    for (i = 0; i < write_cnt; i = i + 1)
    {
        j = i % 4;
        SPIREG(CMD_FIFO) = ctrl_buf[(i / 4)] >> ((3 - j) * 8);
    }
}

/*!
 * function: spi_read_data
 *  \brief serial flash read word
 *  \param read_cnt: count (unit: 4yte)
 *  \param dma_sel: dma or not
 *  \return
 */
void spi_read_data_cpy(u32 src, u32 read_cnt, u32 dma_sel, u32 aes_control)
{
    u32 read_counter, read_res_counter, read_fifo_cnt, ahb_read_data;
    u32 i;
#ifdef CONFIG_PDMA
    pdma_descriptor descriptor;
    if (dma_sel)
    {
        descriptor.channel = 7;
        descriptor.desc_addr = UNCACHED_ADDR(ch7_descr);
        descriptor.next_addr = 0;
        descriptor.src_addr = PHYSICAL_ADDR(CH7_SLV_ADDR);
        descriptor.dma_total_len = read_cnt << 2;
        descriptor.aes_ctrl = aes_control;
        descriptor.intr_enable = 1;
        descriptor.src = 0;
        descriptor.dest = 3;
        descriptor.fifo_size = 31;
        descriptor.valid = PDMA_VALID;

        if (src == 0)
        {
            descriptor.dest_addr = PHYSICAL_ADDR(pCtrl_buf);
        }
        else
        {
            descriptor.dest_addr = PHYSICAL_ADDR(src);
        }

        pdma_desc_set(&descriptor);

        // Kick channel 7
        PDMAREG(PDMA_CTRL) = (1 << PDMA_CH_SPI_RX);

#if defined(PDMA_INTERRUPT) && !defined(BOOT_MODE_BOOT1)
        // wait until PDMA finish work
        if (pdma_status == PDMA_INTERRUPT)
        {
            while (pdma_intr_check())
            {
                udelay(10);
            }
        }
        else
        {
            pdma_pooling_wait();
        }
#else
        pdma_pooling_wait();
#endif
    }
    else
#endif
    {
	    read_counter = 0;
	    read_res_counter = read_cnt;
	    while (read_counter != read_cnt)
	    {
	        read_fifo_cnt = 0;
	        ahb_read_data = SPIREG(STA);
	        read_fifo_cnt = ahb_read_data & 0x3f;
	        if (read_res_counter >= read_fifo_cnt)
	        {
	            for (i = 0; i < read_fifo_cnt; i = i + 1)
	            {
	                if (src != 0)
                    {
                        *(volatile u32 *) (src + (read_counter + i) * 4) = SPIREG(DFIFO);
//	                        dbg_log(LOG_INFO, "data = 0x%x, addr = 0x%x\n", *(volatile u32 *) (src + i * 4), (src + i * 4));
                    }
                    else
                    {
	                    ctrl_buf[read_counter + i] = SPIREG(DFIFO);
//	                        dbg_log(LOG_INFO, "ctrl_buf[read_counter + i]  = 0x%x\n", ctrl_buf[read_counter + i] );
                    }
	            }
	            read_counter = read_counter + read_fifo_cnt;
	            read_res_counter = read_res_counter - read_fifo_cnt;
	        }
	        else
	        {
	            for (i = 0; i < read_res_counter; i = i + 1)
	            {
	                if (src != 0)
                    {
                        *(volatile u32 *) (src + (read_counter + i) * 4) = SPIREG(DFIFO);
//	                        dbg_log(LOG_INFO, "data = 0x%x, addr = 0x%x\n", *(volatile u32 *) (src + i * 4), (src + i * 4));
                    }
                    else
                    {
	                    ctrl_buf[read_counter + i] = SPIREG(DFIFO);
//	                        dbg_log(LOG_INFO, "ctrl_buf[read_counter + i]  = 0x%x\n", ctrl_buf[read_counter + i] );
                    }
	            }
	            read_counter = read_cnt;
	            read_res_counter = 0;
	        }
	    }
 	}

#ifdef BOOT_DEBUG_ENABLE
    if (src != 0)
    {
        // for debug used, get first 4 bytes
        read_data = *(u32 *) src;
    }
#endif

}

void spi_read_data(u32 read_cnt, u32 dma_sel, u32 aes_control)
{
    spi_read_data_cpy(0, read_cnt, dma_sel, aes_control);
}

/*!
 * function: spi_check_finish
 *  \brief wait SPI busy state gone
 *  \return
 */
void spi_check_finish(void)
{
    u32 ahb_read_data;

    ahb_read_data = SPIREG(STA);
    while (ahb_read_data & SPI_BUSY)
    {
        ahb_read_data = SPIREG(STA);
    }
}

/*!
 * function: spi_write_data
 *  \brief serial flash write word
 *  \param write_cnt: count (unit: 4yte)
 *  \param dma_sel: dma or not
 *  \param index: start index
 *  \return
 */
void spi_write_data(u32 src, u32 write_cnt, u32 dma_sel, u32 index, u32 aes_control)
{
    u32 write_counter, write_res_counter, write_fifo_cnt, ahb_read_data;
    u32 i;
// only boot2 support pdma write now
#if defined(CONFIG_PDMA) && defined(BOOT_MODE_BOOT2)
    pdma_descriptor descriptor;
    if (dma_sel)
    {
        descriptor.channel = 6;
        descriptor.desc_addr = UNCACHED_ADDR(ch6_descr);
        descriptor.next_addr = 0;
        descriptor.dest_addr = PHYSICAL_ADDR(CH6_SLV_ADDR);
        descriptor.dma_total_len = write_cnt << 2;
        descriptor.aes_ctrl = aes_control;
        descriptor.intr_enable = 1;
        descriptor.src = 3;
        descriptor.dest = 0;
        descriptor.fifo_size = 31;
        descriptor.valid = PDMA_VALID;

        if (src == 0)
        {
            descriptor.src_addr = PHYSICAL_ADDR(pCtrl_buf);
        }
        else
        {
            descriptor.src_addr = PHYSICAL_ADDR(src);
        }

        pdma_desc_set(&descriptor);

        // Kick channel 6
        PDMAREG(PDMA_CTRL) = (1 << PDMA_CH_SPI_TX);

        // wait until PDMA finish work
        if (pdma_status == PDMA_INTERRUPT)
        {
            while (pdma_intr_check())
            {
                udelay(10);
            }
        }
        else
        {
            pdma_pooling_wait();
        }
    }
    else
#endif
    {
        write_counter = 0;
        write_res_counter = write_cnt;
        while (write_counter != write_cnt)
        {
            ahb_read_data = SPIREG(STA);
            write_fifo_cnt = 32 - ((ahb_read_data >> SPI_TCSHFT) & 0x3f);

            if (write_res_counter >= write_fifo_cnt)
            {
                for (i = 0; i < write_fifo_cnt; i = i + 1)
                {
                    if (src == 0)
                    {
                        SPIREG(DFIFO) = ctrl_buf[write_counter + i + index];
                    }
                    else
                    {
                        SPIREG(DFIFO) = *(volatile u32 *)(src + (write_counter + i + index)*4);
                    }
                }
                write_counter = write_counter + write_fifo_cnt;
                write_res_counter = write_res_counter - write_fifo_cnt;
            }
            else
            {
                for (i = 0; i < write_res_counter; i = i + 1)
                {
                    if (src == 0)
                    {
                        SPIREG(DFIFO) = ctrl_buf[write_counter + i + index];
                    }
                    else
                    {
                        SPIREG(DFIFO) = *(volatile u32 *)(src + (write_counter + i + index)*4);
                    }
                }
                write_counter = write_cnt;
                write_res_counter = 0;
            }
        }
    }
}

#ifdef BOOT_MODE_BOOT2
/*!
 * function: sflash_write_enable
 *
 *  \brief Enable serial flash write
 *  \param sfd: dev pointer
 *  \param enable: 0 disable , 1 enable
 *  \return status byte
 */
void sflash_write_enable(drvarg * sfd, short enable)
{
    int channel = 0;
    u32 addr_cnt = 0, data_cnt = 0;
    u32 cmd = (enable ? SF_CMD_WRITE_ENABLE : SF_CMD_WRITE_DISABLE);

    ctrl_buf[0] = cmd << SF_CMD_S;
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x0, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();
}

/*!
 * function: sflash_erase
 *
 *  \brief Enable serial flash write
 *  \param sfd: dev pointer
 *  \param addr: erase from flash address
 *  \param type: block erase or bulk erase (whole chip)
 *  \return status byte
 */
static int sflash_erase(drvarg * sfd, u32 addr, int type,
                        u32 ecmd)
{
    int rc = 0;
    int channel = 0;
    u32 addr_cnt = 3, data_cnt = 0;
    u32 cmd = SF_CMD_SEC_ERASE;
    u32 page_size = pf_head->page_size;
    u32 full_page = page_size * 2 - 1;

    sflash_write_enable(sfd, 1);
    if (ecmd)
    {
        cmd = ecmd;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_buf[0] = (cmd << SF_CMD_S) + ((addr & ~full_page) / (page_size * 2));
    }
    else
    {
        ctrl_buf[0] = (cmd << SF_CMD_S) + addr;
    }
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x0, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    if (sflash_check_busy(sfd, SF_ERASE_TO))
    {
        rc = SF_RC_ERASE_TO;
    }
    sflash_write_enable(sfd, 0);
    return rc;
}
#endif

/*!
 * function: sflash_id
 *  \brief Get the manufacture/device id
 *  \param sfd: dev pointer
 *  \return 32bit word id,
 *      b31-b24: M7-M0, b23-b8: D15-D0
 */
static int sflash_id(drvarg * sfd)
{
    int rc, channel = 0;
    u32 addr_cnt = 0, data_cnt = 4;
    u32 cmd = SF_CMD_READ_ID;

    ctrl_buf[0] = cmd << SF_CMD_S;
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data(1, 0, 0);     //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();
    rc = ctrl_buf[0];

    return rc;
}

/*!
 * function: sflash_get_status
 *  \brief Get serial flash status byte
 *  \param sfd: dev pointer
 *  \return status byte
 */
static u32 sflash_get_status(drvarg * sfd)
{
    int rc, channel = 0;
    u32 addr_cnt, data_cnt;
    u32 cmd;
    is_ecc_cant_handle = 0;

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr_cnt = 1;
        data_cnt = 1;
        cmd = SF_CMD_GET_FEATURE;

        ctrl_buf[0] = (cmd << SF_CMD_S) + (SF_STATUS_REG << 16);
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_read_data(1, 0, 0); //unit: 4 byte, len=(data_cnt+3)/4=1
        spi_check_finish();
//	        rc = ctrl_buf[0] >> 24;
        rc = ctrl_buf[0];

        if (rc & (0x30 << 24))  // ecc failed case
        {
//          dbg_log(LOG_INFO, "internal ECC got error, rc = 0x%02x\n", rc);

            if (((rc >> 24) & 0x30) == 0x20)
            {
                dbg_log(LOG_INFO, "ECC correct error bits failed !!\n");
                *pCtrl_buf = 0x00000000;    // clear first int of pCtrl_buf
                is_ecc_cant_handle = 1;
            }
        }

        if (rc & (0x08 >> 24))  // program failed case
        {
            dbg_log(LOG_INFO, "failed program failed, rc = 0x%02x\n", rc);
        }

        if (rc & (0x04 >> 24))  // erase failed case
        {
            dbg_log(LOG_INFO, "failed erase failed, rc = 0x%02x\n", rc);
        }
    }
    else
    {
        addr_cnt = 0;
        data_cnt = 1;
        cmd = SF_CMD_READ_STATUS;

        ctrl_buf[0] = cmd << SF_CMD_S;
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_read_data(1, 0, 0); //unit: 4 byte, len=(data_cnt+3)/4=1
        spi_check_finish();
//	        rc = ctrl_buf[0] >> 24;
        rc = ctrl_buf[0];
    }

    return rc;
}

/*!
 * function: sflash_check_busy
 *
 *  \brief Check serial flash busy within wait loop time
 *  \param sfd: dev pointer
 *  \param loop: how long before timeout
 *  \return busy or not
 */
u32 sflash_check_busy(drvarg * sfd, int loop)
{
    int i;
    u32 srb;

    for (i = 0; i < SF_ERASE_TO; i++)
    {
        srb = sflash_get_status(sfd);
        if (0 == (srb & (SF_SR_BUSY << 24)))
        {
            return 0;
        }

        if (is_ecc_cant_handle)
        {
            return 0;
        }
        udelay(1);
    }
    return 1;
}

#if (CONFIG_READ_IO_MODE == 2)
/*!
 * function: nand_flash_dualspeed_read_bytes
 *  \param sfd: dev pointer
 *  \param offset: offset in flash
 *  \param count: total count of read bytes
 *  \param dma_sel: is use PDMA
 *  \return read word
 */
u32 nand_flash_dualspeed_read_bytes(drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control)
{
    int rc, channel = 0;
    u32 addr_cnt = 3, data_cnt = 0;
    u32 dummy_offset;
    u32 page_size = pf_head->page_size;
    u32 full_page = page_size * 2 - 1;

    ctrl_buf[0] =
        (SF_CMD_READ2CACHE << SF_CMD_S) + ((offset & ~full_page) / (page_size * 2));
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    if (sflash_check_busy(sfd, SF_ERASE_TO))
    {
        return SF_RC_READ_TO;
    }

    data_cnt = count;
    addr_cnt = 3;
    dummy_offset = (8 - pf_head->cmd_dummy_byte * 8);
    ctrl_buf[0] = (SF_CMD_DUAL_FAST_READ << SF_CMD_S) | ((offset & full_page) << dummy_offset);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 1, 0, data_cnt, 0x2, pf_head->read_dummy_byte * 8, channel);
    spi_write_cmd(1 + addr_cnt);
    spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);
    spi_check_finish();
    rc = read_data;

    return rc;
}

/*!
 * function: sflash_dualspeed_read_bytes
 *  \param sfd: dev pointer
 *  \param offset: offset in flash
 *  \param count: total count of read bytes
 *  \param dma_sel: is use PDMA
 *  \return read word
 */
u32 sflash_dualspeed_read_bytes(drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control)
{
    int rc, channel = 0;
    u32 addr_cnt = 3;
    u32 data_cnt = count;
    u32 cmd = SF_CMD_DUAL_FAST_READ;

    ctrl_buf[0] = (cmd << SF_CMD_S) + offset;
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 1, 0, data_cnt, 0x2, 8, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);   //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();
    rc = read_data;

    return rc;
}
#elif (CONFIG_READ_IO_MODE == 4)
/*!
 * function: nand_flash_quadspeed_read_bytes
 *  \param sfd: dev pointer
 *  \param offset: offset in flash
 *  \param count: total count of read bytes
 *  \param dma_sel: is use PDMA
 *  \return read word
 */
u32 nand_flash_quadspeed_read_bytes(drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control)
{
    int rc, channel = 0;
    u32 addr_cnt = 3, data_cnt = 0;
    u32 dummy_offset;
    u32 page_size = pf_head->page_size;
    u32 full_page = page_size * 2 - 1;

    ctrl_buf[0] =
        (SF_CMD_READ2CACHE << SF_CMD_S) + ((offset & ~full_page) / (page_size * 2));
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    if (sflash_check_busy(sfd, SF_ERASE_TO))
    {
        return SF_RC_READ_TO;
    }

    data_cnt = count;
    addr_cnt = 3;
    dummy_offset = (8 - pf_head->cmd_dummy_byte * 8);
    ctrl_buf[0] = (SF_CMD_QUAD_FAST_READ << SF_CMD_S) | ((offset & full_page) << dummy_offset);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 3, 0, data_cnt, 0x2, pf_head->read_dummy_byte * 8, channel);
    spi_write_cmd(1 + addr_cnt);
    spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);
    spi_check_finish();
    rc = read_data;

    return rc;
}

/*!
 * function: sflash_quadspeed_read_bytes
 *  \param sfd: dev pointer
 *  \param offset: offset in flash
 *  \param count: total count of read bytes
 *  \param dma_sel: is use PDMA
 *  \return read word
 */
u32 sflash_quadspeed_read_bytes(drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control)
{
    int rc, channel = 0;
    u32 addr_cnt = 3;
    u32 data_cnt = count;
    u32 cmd = SF_CMD_QUAD_FAST_READ;

    ctrl_buf[0] = (cmd << SF_CMD_S) + offset;
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 3, 0, data_cnt, 0x2, 8, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);        //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();
    rc = read_data;

    return rc;
}
#else
/*!
 * function: nand_flash_read_word
 *  \param sfd: dev pointer
 *  \param offset: offset in flash
 *  \param count: total count of read bytes
 *  \param dma_sel: is use PDMA
 *  \return read word
 */
u32 nand_flash_read_bytes(drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control)
{
    int rc, channel = 0;
    u32 addr_cnt = 3, data_cnt = 0;
    u32 dummy_offset;
    u32 page_size = pf_head->page_size;
    u32 full_page = page_size * 2 - 1;

    ctrl_buf[0] =
        (SF_CMD_READ2CACHE << SF_CMD_S) + ((offset & ~full_page) / (page_size * 2));
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    if (sflash_check_busy(sfd, SF_ERASE_TO))
    {
        return SF_RC_READ_TO;
    }

    data_cnt = count;
    addr_cnt = 3;
    dummy_offset = (8 - pf_head->cmd_dummy_byte * 8);
    ctrl_buf[0] = (SF_CMD_FAST_READ << SF_CMD_S) | ((offset & full_page) << dummy_offset);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, pf_head->read_dummy_byte * 8, channel);
    spi_write_cmd(1 + addr_cnt);
    spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);
    spi_check_finish();
    rc = read_data;

    return rc;
}

/*!
 * function: sflash_read_bytes
 *  \param sfd: dev pointer
 *  \param offset: offset in flash
 *  \param count: total count of read bytes
 *  \param dma_sel: is use PDMA
 *  \return read word
 */
u32 sflash_read_bytes(drvarg * sfd, u32 offset, u32 src, u32 count, u32 dma_sel, u32 aes_control)
{
    int rc, channel = 0;
    u32 addr_cnt = 3;
    u32 data_cnt = count;
    u32 cmd = SF_CMD_READ;

    if(otp_parse_config(OTP_NOR_4ADDR_SHIFT))
    {
        cmd = SF_CMD_4READ;
        addr_cnt = 4;
        ctrl_buf[0] = (cmd << SF_CMD_S) + (offset>>8);
        ctrl_buf[1] = (offset & 0xff) << 24;
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
        spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
        spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);   //unit: 4 byte, len=(data_cnt+3)/4=1
    }
    else
    {
        ctrl_buf[0] = (cmd << SF_CMD_S) + offset;
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
        spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
        spi_read_data_cpy(src, (data_cnt+3)/4, dma_sel, aes_control);   //unit: 4 byte, len=(data_cnt+3)/4=1
    }
    spi_check_finish();
    rc = read_data;

    return rc;
}

#endif

void nand_read_protect_register(void)
{
    int channel = 0;
    u32 addr_cnt = 1, data_cnt = 1;
    u32 cmd = SF_CMD_GET_FEATURE;

    ctrl_buf[0] = (cmd << SF_CMD_S) + (SF_PROTECT_REG << 16);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data(1, 0, 0);    //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();

    // rc:00 => ECC disable, rc:10 => ECC enable
    dbg_log(LOG_VERBOSE, "A0h status = %08x\n", ctrl_buf[0]);
}

/*!
 * function: nand_unlock
 *
 *  \brief unlock block protection
 *  \return
 */
void nand_unlock(void)
{
    int channel = 0;
    u32 addr_cnt = 1, data_cnt = 1;
    dbg_log(LOG_VERBOSE, "enter unlock()\n");

    ctrl_buf[0] = (SF_CMD_SET_FEATURE << SF_CMD_S) + (SF_PROTECT_REG << 16);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    ctrl_buf[0] = 0 << 24;
    spi_write_data(0, 1, 0, 0, 0);    //unit: 4 byte, len=(datalen+3)/4=1
    spi_check_finish();
}

u32 nand_read_feature_register(void)
{
    int channel = 0;
    u32 addr_cnt = 1, data_cnt = 1;
    u32 cmd = SF_CMD_GET_FEATURE;

    ctrl_buf[0] = (cmd << SF_CMD_S) + (SF_FEATURE_REG << 16);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data(1, 0, 0);        //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();

    return ctrl_buf[0];
}

/*!
 * function: nand_change_ecc_feature
 *
 *  \brief change ecc status to enable/disable
 *  \return
 */
void nand_change_ecc_feature(u32 ecc_status)
{
    int channel = 0;
    u32 addr_cnt = 1, data_cnt = 1;
    u8 status = (nand_read_feature_register() & ~0x10) & 0xFF;

    ctrl_buf[0] = (SF_CMD_SET_FEATURE << SF_CMD_S) + (SF_FEATURE_REG << 16);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte

    ctrl_buf[0] = ((ecc_status << 4) | status) << 24;

    spi_write_data(0, 1, 0, 0, 0);    //unit: 4 byte, len=(datalen+3)/4=1
    spi_check_finish();
}

void nand_quard_speed_enable(void)
{
    int channel = 0;
    u32 addr_cnt = 1, data_cnt = 1;
    u8 status = 0x10;//(nand_read_feature_register() & ~0x01) & 0xFF;

    ctrl_buf[0] = (SF_CMD_SET_FEATURE << SF_CMD_S) + (SF_FEATURE_REG << 16);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte

#if (CONFIG_READ_IO_MODE == 4) || (CONFIG_WRITE_IO_MODE == 4)
    ctrl_buf[0] = (status | 0x01) << 24;
#else
    ctrl_buf[0] = status << 24;
#endif

    spi_write_data(0, 1, 0, 0, 0);    //unit: 4 byte, len=(datalen+3)/4=1
    spi_check_finish();
}

/*!
 * function: nand_reset
 *
 *  \brief reset NAND flash
 *  \return
 */
void nand_reset(void)
{
    int channel = 0;
    u32 addr_cnt = 0, data_cnt = 0;

    ctrl_buf[0] = (SF_CMD_RESET << SF_CMD_S);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x3, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    // add 1.5ms delay
    udelay(1500);
}

/*!
 * function: nor_reset
 *
 *  \brief reset NOR flash
 *  \return
 */
void nor_reset(void)
{
    int channel = 0;
    u32 addr_cnt = 0, data_cnt = 0;

    ctrl_buf[0] = (SF_CMD_RST_EN << SF_CMD_S);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x3, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    ctrl_buf[0] = (SF_CMD_RST_MEM << SF_CMD_S);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x3, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_check_finish();

    // add 1.5ms delay
    udelay(1500);
}

int flash_read_bytes_no_check_block(u32 addr, u32 src, u32 len);

#ifdef BAD_BLOCK_MANAGE
u32 nand_is_bad_block(u32 block_index)
{
    int ret = DETECT_GOOD_BLOCK;
    int rc = 0;
    int rc2 = 0;
    u32 block_num = get_max_block_num(0);
    u32 block_size = pf_head->block_size;
    u32 page_size = pf_head->page_size;

    if (block_index >= block_num)
    {
        dbg_log(LOG_INFO, "Check with invalid block index = %d\n", block_index);
        return DETECT_OUT_RANGE;
    }

    // according to datasheet, we must disable ecc before check bad block
    nand_change_ecc_feature(DISABLE_ECC);
    change_aes_enable_status(DISABLE_SECURE);

    /*
     *  read first byte of spare area of 1st and 2nd page of each block
     */
    rc = flash_read_bytes_no_check_block((block_index * block_size) + page_size, 0, 4);
    rc = *pCtrl_buf;
    if ((rc & 0xff000000) != 0xff000000)
    {
        ret = DETECT_BAD_BLOCK;
    }

    if (pf_head->check_page_type == BAD_BLOCK_CHECK_TWO_PAGE)
    {
        rc2 = flash_read_bytes_no_check_block((block_index * block_size) + (page_size * 3), 0, 4);
        rc2 = *pCtrl_buf;
        if ((rc2 & 0xff000000) != 0xff000000)
        {
            ret = DETECT_BAD_BLOCK;
        }
    }

    // after check bad block finished, enable ecc again
    nand_change_ecc_feature(ENABLE_ECC);

    return ret;
}

/*!
 * function: skip_bad_block
 *  \param  block_num: valid block number 
 *  \param  ret_type: return type, valid block_index or offset from input block_index 
 *  \return index of valid block number
 */
int skip_bad_block(u32 block_index, u32 ret_type)
{
    u32 ori_index = block_index;
    u32 valid_index = block_index;
    u32 block_num = get_max_block_num(0);
    int i = 0;
    u32 result = 0;

//	    dbg_log(LOG_INFO, "block_index = %d\n", block_index);
    for (i = ori_index; i < block_num; i++, valid_index++)
    {
        result = nand_is_bad_block(i);

        switch (result)
        {
            case DETECT_BAD_BLOCK:
                dbg_log(LOG_INFO, "Skip bad block, index = %d\n", i);
                break;
            case DETECT_OUT_RANGE:
                return DETECT_OUT_RANGE;
            default:
                break;
        }

        if (result == DETECT_GOOD_BLOCK)
        {
            break;
        }
    }

    if (ret_type == RET_TYPE_INDEX)
    {
        return valid_index;
    }
    else    // RET_TYPE_OFFSET
    {
        return (valid_index - ori_index);
    }
}

#ifdef BOOT_MODE_BOOT2
int get_shift_bad_block_num(u32 block_index)
{
    int shift_num = 0;
    int i;

    for (i = 0; i < block_index; i++)
    {
        if (nand_is_bad_block(i))
        {
            shift_num++;
        }
    }

    return shift_num;
}

void find_all_bad_block(void)
{
    int bPass = 1;
    int rc = 0;
    int i = 0;
    u32 block_num = get_max_block_num(0);

    for (i = 0; i < block_num; i++)
    {
        if (nand_is_bad_block(i))
        {
            bPass = 0;
            dbg_log(LOG_INFO, "\nBad block found, block index = %d\n", i);
        }
    }

    if (bPass)
    {
        dbg_log(LOG_INFO, "All block pass !!!!\n");
    }
}

u32 get_bad_block_num_by_index(u32 start_index, u32 end_index)
{
    int i = 0;
    u32 block_num = 0;

    for (i = start_index; i <= end_index; i++)
    {
        if (nand_is_bad_block(i))
        {
            block_num++;
        }
    }

    return block_num;
}

void mark_bad_block(u32 block_index)
{
    dbg_log(LOG_INFO, "Flash_Control: mark_virtual_bad_block()\n");

    int rc = 0;
    int rc2 = 0;
    u32 block_num = get_max_block_num(0);
    u32 block_size = pf_head->block_size;
    u32 page_size = pf_head->page_size;
    u32 bad_tag = 0x00000000;
#ifdef CONFIG_PDMA
    *(volatile unsigned int*)(UNCACHED_ADDR(&bad_tag)) = 0x00000000;
#endif

    if (block_index >= block_num)
    {
        dbg_log(LOG_INFO, "Please mark block in valid range, max block = %d\n", block_num);
    }

    change_aes_enable_status(DISABLE_SECURE);

    flash_erase_no_check_block(block_index * block_size, 4);
    rc = flash_write_no_check_block(block_index * block_size + page_size, (u32)&bad_tag, 4);
    if (pf_head->check_page_type == BAD_BLOCK_CHECK_TWO_PAGE)
    {
        rc2 = flash_write_no_check_block((block_index * block_size) + (page_size * 3), (u32)&bad_tag, 4);
    }
}

void mark_bad_block_by_interval(u32 block_index, u32 len)
{
    dbg_log(LOG_INFO, "Flash_Control: mark_bad_block_by_interval()\n");

    int i = 0;
    int rc = 0;
    int rc2 = 0;
    u32 block_num = get_max_block_num(0);
    u32 block_size = pf_head->block_size;
    u32 page_size = pf_head->page_size;
    u32 bad_tag = 0x00000000;
    u32 start_index = block_index;
    u32 end_index = (block_index + len);
#ifdef CONFIG_PDMA
    *(volatile unsigned int*)(UNCACHED_ADDR(&bad_tag)) = 0x00000000;
#endif

    if (end_index >= block_num)
    {
        dbg_log(LOG_INFO, "Please mark block in valid range, max block = %d\n", block_num);
    }

    change_aes_enable_status(DISABLE_SECURE);

    for (i = start_index; i < end_index; i++)
    {
        flash_erase_no_check_block(i * block_size, 4);
        rc = flash_write_no_check_block(i * block_size + page_size, (u32)&bad_tag, 4);
        if (pf_head->check_page_type == BAD_BLOCK_CHECK_TWO_PAGE)
        {
            rc2 = flash_write_no_check_block((i * block_size) + (page_size * 3), (u32)&bad_tag, 4);
        }
    }
}

void mark_good_block(u32 block_index)
{
    dbg_log(LOG_INFO, "Flash_Control: mark_good_block()\n");

    int rc = 0;
    int rc2 = 0;
    u32 block_num = get_max_block_num(0);
    u32 block_size = pf_head->block_size;
    u32 page_size = pf_head->page_size;
    u32 gd_tag = 0xffffffff;
#ifdef CONFIG_PDMA
    *(volatile unsigned int*)(UNCACHED_ADDR(&gd_tag)) = 0xffffffff;
#endif

    if (block_index >= block_num)
    {
        dbg_log(LOG_INFO, "Please unmark block in valid range, max block = %d\n", block_num);
    }

    change_aes_enable_status(DISABLE_SECURE);

    flash_erase_no_check_block(block_index * block_size, 4);
    rc = flash_write_no_check_block(block_index * block_size + page_size, (u32)&gd_tag, 4);
    if (pf_head->check_page_type == BAD_BLOCK_CHECK_TWO_PAGE)
    {
        rc2 = flash_write_no_check_block((block_index * block_size) + (page_size * 3), (u32)&gd_tag, 4);
    }
}

void mark_good_block_by_interval(u32 block_index, u32 len)
{
    dbg_log(LOG_INFO, "Flash_Control: mark_good_block_by_interval()\n");

    int i = 0;
    int rc = 0;
    int rc2 = 0;
    u32 block_num = get_max_block_num(0);
    u32 block_size = pf_head->block_size;
    u32 page_size = pf_head->page_size;
    u32 gd_tag = 0xffffffff;
    u32 start_index = block_index;
    u32 end_index = (block_index + len);
#ifdef CONFIG_PDMA
    *(volatile unsigned int*)(UNCACHED_ADDR(&gd_tag)) = 0xffffffff;
#endif

    if (end_index >= block_num)
    {
        dbg_log(LOG_INFO, "Please unmark block in valid range, max block = %d\n", block_num);
    }

    change_aes_enable_status(DISABLE_SECURE);

    for (i = start_index; i < end_index; i++)
    {
        flash_erase_no_check_block(i * block_size, 4);
        rc = flash_write_no_check_block(i * block_size + page_size, (u32)&gd_tag, 4);
        if (pf_head->check_page_type == BAD_BLOCK_CHECK_TWO_PAGE)
        {
            rc2 = flash_write_no_check_block((i * block_size) + (page_size * 3), (u32)&gd_tag, 4);
        }
    }
}
#endif
#endif

#ifdef BOOT_MODE_BOOT2
void wait_wip_finish(void)
{
    int rc = 0, channel = 0;
    u32 addr_cnt = 0, data_cnt = 1;
    u32 cmd = SF_CMD_READ_STA_REG1;
    int retry_cnt = 1;

    while (retry_cnt != 0)
    {
        ctrl_buf[0] = (cmd << SF_CMD_S);
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_read_data(1, 0, 0); //unit: 4 byte, len=(data_cnt+3)/4=1
        spi_check_finish();

        // rc:01 => WIP busy
        dbg_log(LOG_INFO, "RE1 status = 0x%08x\n", rc);
        rc = ctrl_buf[0] >> 24;

        if ((rc & SF_SR_BUSY) == 0)
        {
            break;
        }
        
        retry_cnt--;
        mdelay(1);
    }    
}

void nor_enable_qe(void)
{
    int rc, channel = 0;
    u32 addr_cnt = 0, data_cnt = 1;
    u32 cmd = SF_CMD_WRITE_STA_REG2;

    flashdesc *fp = &flashcfg[0];
    drvarg *sfd = &(fp->arg);

    sflash_write_enable(sfd, 1);

    // enable QE bit in status register-2
    ctrl_buf[0] = (cmd << SF_CMD_S);

    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x1, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte

#if (CONFIG_READ_IO_MODE == 4) || (CONFIG_WRITE_IO_MODE == 4)
    ctrl_buf[0] = 0x02 << 24;
#else
    ctrl_buf[0] = 0x00 << 24;
#endif

    spi_write_data(0, 1, 0, 0, 0);    //unit: 4 byte, len=(datalen+3)/4=1
    spi_check_finish();
    sflash_write_enable(sfd, 0);

    wait_wip_finish();
}

void nor_read_status_register2(void)
{
    int rc, channel = 0;
    u32 addr_cnt = 0, data_cnt = 1;
    u32 cmd = SF_CMD_READ_STA_REG2;

    ctrl_buf[0] = (cmd << SF_CMD_S);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data(1, 0, 0);        //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();
    rc = ctrl_buf[0] >> 24;

    // rc:00 => QE disable, rc:02 => QE enable
    dbg_log(LOG_INFO, "NOR register 2 status = 0x%08x\n", rc);

    nor_enable_qe();
}

void nor_read_status_register3(void)
{
    int rc, channel = 0;
    u32 addr_cnt = 0, data_cnt = 1;
    u32 cmd = SF_CMD_READ_STA_REG3;

    ctrl_buf[0] = (cmd << SF_CMD_S);
    spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, data_cnt, 0x2, 0, channel);
    spi_write_cmd(1 + addr_cnt);        //unit: 1 byte
    spi_read_data(1, 0, 0);        //unit: 4 byte, len=(data_cnt+3)/4=1
    spi_check_finish();
    rc = ctrl_buf[0] >> 24;

    dbg_log(LOG_INFO, "NOR register 3 status = 0x%08x\n", rc);
}
#endif

#ifdef CHECK_AFTER_WRITE
int write_check_nand(u32 src, u32 addr, u32 datalen, u32 aes_control)
{
    int rc = 0;
    int i = 0, j = 0;
    flashdesc *fp = &flashcfg[0];
    u32 read_size;
    u32 read_data;

    if (src == 0)
    {
        // no need to check, so return success directly now
        return CHECK_WRITE_SUCCESS;
    }

    if (aes_control != 0)
    {
        // no need to check, data have been encrypted
        return CHECK_WRITE_SUCCESS;
    }

    // because write_data and read_data are both int array,
    // so need to convert length to datalen/4
    for (i = 0; i < datalen; i += CTRL_BUF_SIZE)
    {
        read_size = ((datalen - i) >= CTRL_BUF_SIZE) ? CTRL_BUF_SIZE : (datalen - i);
        rc = flash_read_without_pdma(addr + i, read_size);

        for (j = 0; j < read_size; j += 4)
        {
            read_data = cpu_to_be32(ctrl_buf[j/4]);
            if (*(u32 *)(src + i + j) != read_data)
            {
                dbg_log(LOG_INFO, "NAND flash write failed !!!\n");
                dbg_log(LOG_INFO, "addr = 0x%x\n", addr + i + j);
                dbg_log(LOG_INFO, "data = 0x%x\n", read_data);
                dbg_log(LOG_INFO, "write_data = 0x%x\n", *(u32 *)(src + i + j));
                return CHECK_WRITE_FAIL;
            }
        }
    }

    return CHECK_WRITE_SUCCESS;
}

int write_check_nor(u32 src, u32 addr, short datalen, u32 aes_control)
{
    int rc = 0;
    int i = 0, j = 0;
    flashdesc *fp = &flashcfg[0];
    u32 read_size;
    u32 read_data;

    if (src == 0)
    {
        // no need to check, so return success directly now
        return CHECK_WRITE_SUCCESS;
    }

    if (aes_control != 0)
    {
        // no need to check, data have been encrypted
        return CHECK_WRITE_SUCCESS;
    }

    // because write_data and read_data are both int array,
    // so need to convert length to datalen/4
    for (i = 0; i < datalen; i += CTRL_BUF_SIZE)
    {
        read_size = ((datalen - i) >= CTRL_BUF_SIZE) ? CTRL_BUF_SIZE : (datalen - i);
        rc = flash_read_without_pdma(addr + i, read_size);

        for (j = 0; j < read_size; j += 4)
        {
            read_data = cpu_to_be32(ctrl_buf[j/4]);
            if (*(u32 *)(src + i + j) != read_data)
            {
                dbg_log(LOG_INFO, "NOR flash write failed !!!\n");
                dbg_log(LOG_INFO, "addr = 0x%x\n", addr + i + j);
                dbg_log(LOG_INFO, "data = 0x%x\n", read_data);
                dbg_log(LOG_INFO, "write_data = 0x%x\n", *(u32 *)(src + i + j));
                return CHECK_WRITE_FAIL;
            }
        }
    }

    return CHECK_WRITE_SUCCESS;
}
#endif

#ifdef BOOT_MODE_BOOT2
#if (CONFIG_WRITE_IO_MODE == 4)
/*=============================================================================+
| Quardra Speed                                                                |
+=============================================================================*/
/*!
 * function: nand_flash_quadspeed_program
 *  \param     sfd: dev pointer
 *  \param    addr: where to write on flash
 *  \param     len: how many bytes to write
 *  \param     buf: from buffer
 *  \param dma_sel: is use PDMA
 *  \return check result code
 */
int nand_flash_quadspeed_program(drvarg * sfd, u32 addr, u32 len, u32 buf, u32 dma_sel, u32 aes_control)
{
    int rc = 0;
    u32 datalen;
    u8 *src = (u8 *) buf;
    int channel = 0;
    u32 addr_cnt = 3;
    int start_offset = 0;
    u32 page_size = pf_head->page_size;
    u32 full_page = page_size * 2 - 1;  // if page_size = 0x800, full_page = 0xFFF
    u32 write_addr = (addr % (page_size * 2));

    if (write_addr >= page_size)
    {
        // means write spare area
        start_offset = len;
    }
    else if (write_addr != 0 && (write_addr + len) > page_size)
    {
        start_offset = page_size - write_addr;
    }

    for (; len > 0; addr += (full_page + 1))
    {
        if (start_offset != 0)
        {
            datalen = start_offset;
        }
        else
        {
            // program load random data
            if (len > page_size)
            {
                datalen = page_size;
            }
            else
            {
                datalen = len;
            }
        }

        sflash_write_enable(sfd, 1);

//	        dbg_log(LOG_INFO, "addr = 0x%x\n", addr);
//	        dbg_log(LOG_INFO, "data = 0x%x\n", (*(u32 *) src));
//	        dbg_log(LOG_INFO, "datalen = 0x%x\n", datalen);
	
        addr_cnt = 2;
        ctrl_buf[0] = (SF_CMD_QUAD_WRITE << SF_CMD_S) + ((addr & full_page) << 8);        
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 3, 0, datalen, 0x1, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_write_data((u32)src, (datalen + 3) / 4, dma_sel, 0, aes_control);        //unit: 4 byte, len=(datalen+3)/4=1
        spi_check_finish();

        //program execute command
        addr_cnt = 3;
        ctrl_buf[0] = (SF_CMD_EXECUTE << SF_CMD_S) + ((addr & ~full_page) / (page_size * 2));
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, 0, 0x1, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_check_finish();

        if (sflash_check_busy(sfd, SF_PROG_TO))
        {
            rc = SF_RC_PROG_TO;
            dbg_log(LOG_INFO, "timeout\n");
            break;
        }

#ifdef CHECK_AFTER_WRITE
        if (write_check_nand((u32)src, addr, datalen, aes_control))
        {
            rc = SF_RC_PROG_FAIL;
            break;
        }
#endif
        len -= datalen;
        src += datalen;

		// reset start_offset and align addr
        if (start_offset != 0)
        {
            start_offset = 0;
            addr = (addr / (full_page + 1)) * (full_page + 1);
        }
    }
    sflash_write_enable(sfd, 0);
    return rc;
}

/*!
 * function: sflash_quadspeed_program
 *  \param     sfd: dev pointer
 *  \param    addr: where to write on flash
 *  \param     len: how many bytes to write
 *  \param     buf: from buffer
 *  \param dma_sel: is use PDMA
 *  \return check result code
 */
int sflash_quadspeed_program(drvarg * sfd, u32 addr, u32 len, u32 buf, u32 dma_sel, u32 aes_control)
{
    int rc = 0;
    u32 datalen;
    u8 *src = (u8 *) buf;
    int channel = 0;
    u32 addr_cnt = 3;
    u32 progress = addr;

    for (; len > 0; addr += NOR_PAGE_SIZE)
    {
        if (len >= NOR_PAGE_SIZE)
        {
            datalen = NOR_PAGE_SIZE;
        }
        else
        {
            datalen = len;
        }

        sflash_write_enable(sfd, 1);

        ctrl_buf[0] = (SF_CMD_QUAD_WRITE << SF_CMD_S) + addr;
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 3, 0, datalen, 0x1, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_write_data((u32)src, (datalen + 3)/4, dma_sel, 0, aes_control);        //unit: 4 byte, len=(datalen+3)/4=1
        spi_check_finish();

        if (sflash_check_busy(sfd, SF_PROG_TO))
        {
            rc = SF_RC_PROG_TO;
            dbg_log(LOG_INFO, "timeout\n");
            break;
        }

#ifdef CHECK_AFTER_WRITE
        if (write_check_nor((u32)src, addr, datalen, aes_control))
        {
            rc = SF_RC_PROG_FAIL;
            break;
        }
#endif
        len -= datalen;
        src += datalen;
    }

    sflash_write_enable(sfd, 0);
    return rc;
}
/*=============================================================================+
| Normal Speed                                                                 |
+=============================================================================*/
#else
/*!
 * function: nand_flash_program
 *  \param     sfd: dev pointer
 *  \param    addr: where to write on flash
 *  \param     len: how many bytes to write
 *  \param     buf: from buffer
 *  \param dma_sel: is use PDMA
 *  \return check result code
 */
int nand_flash_program(drvarg * sfd, u32 addr, u32 len, u32 buf, u32 dma_sel, u32 aes_control)
{
    int rc = 0;
    u32 datalen;
    u8 *src = (u8 *) buf;
    int channel = 0;
    u32 addr_cnt = 3;
    int start_offset = 0;
    u32 page_size = pf_head->page_size;
    u32 full_page = page_size * 2 - 1;  // if page_size = 0x800, full_page = 0xFFF
    u32 write_addr = (addr % (page_size * 2));

    if (write_addr >= page_size)
    {
        // means write spare area
        start_offset = len;
    }
    else if (write_addr != 0 && (write_addr + len) > page_size)
    {
        start_offset = page_size - write_addr;
    }

    for (; len > 0; addr += (full_page + 1))
    {
        if (start_offset != 0)
        {
            datalen = start_offset;
        }
        else
        {
            // program load random data
            if (len > page_size)
            {
                datalen = page_size;
            }
            else
            {
                datalen = len;
            }
        }

        sflash_write_enable(sfd, 1);
        
//	        dbg_log(LOG_INFO, "addr = 0x%x\n", addr);
//	        dbg_log(LOG_INFO, "data = 0x%x\n", (*(u32 *) src));
//	        dbg_log(LOG_INFO, "datalen = 0x%x\n", datalen);

        addr_cnt = 2;
        ctrl_buf[0] = (SF_CMD_WRITE << SF_CMD_S) + ((addr & full_page) << 8);
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, datalen, 0x1, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte   
        spi_write_data((u32)src, (datalen + 3) / 4, dma_sel, 0, aes_control);        //unit: 4 byte, len=(datalen+3)/4=1
        spi_check_finish();

        //program execute command
        addr_cnt = 3;
        ctrl_buf[0] = (SF_CMD_EXECUTE << SF_CMD_S) + ((addr & ~full_page) / (page_size * 2));
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, 0, 0x1, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_check_finish();

        if (sflash_check_busy(sfd, SF_PROG_TO))
        {
            rc = SF_RC_PROG_TO;
            dbg_log(LOG_INFO, "timeout\n");
            break;
        }

#ifdef CHECK_AFTER_WRITE
        if (write_check_nand((u32)src, addr, datalen, aes_control))
        {
            rc = SF_RC_PROG_FAIL;
            break;
        }
#endif
        len -= datalen;
        src += datalen;

        // reset start_offset and align addr
        if (start_offset != 0)
        {
            start_offset = 0;
            addr = (addr / (full_page + 1)) * (full_page + 1);
        }
    }
    sflash_write_enable(sfd, 0);
    return rc;
}

/*!
 * function: sflash_program
 *  \param     sfd: dev pointer
 *  \param    addr: where to write on flash
 *  \param     len: how many bytes to write
 *  \param     buf: from buffer
 *  \param dma_sel: is use PDMA
 *  \return check result code
 */
int sflash_program(drvarg * sfd, u32 addr, u32 len, u32 buf, u32 dma_sel, u32 aes_control)
{
    int rc = 0;
    u32 datalen;
    u8 *src = (u8 *) buf;
    int channel = 0;
    u32 addr_cnt = 3;

    for (; len > 0; addr += NOR_PAGE_SIZE)
    {
        if (len >= NOR_PAGE_SIZE)
        {
            datalen = NOR_PAGE_SIZE;
        }
        else
        {
            datalen = len;
        }

        sflash_write_enable(sfd, 1);

//	        dbg_log(LOG_INFO, "addr = 0x%x\n", addr);
//	        dbg_log(LOG_INFO, "data = 0x%x\n", (*(u32 *) src));
//	        dbg_log(LOG_INFO, "datalen = 0x%x\n", datalen);

        ctrl_buf[0] = (SF_CMD_WRITE << SF_CMD_S) + addr;
        spi_send_cmd(0, 0, 1, 0, 0, addr_cnt, 0, 0, datalen, 0x1, 0, channel);
        spi_write_cmd(1 + addr_cnt);    //unit: 1 byte
        spi_write_data((u32)src, (datalen + 3)/4, dma_sel, 0, aes_control);        //unit: 4 byte, len=(datalen+3)/4=1
        spi_check_finish();

        if (sflash_check_busy(sfd, SF_PROG_TO))
        {
            rc = SF_RC_PROG_TO;
            dbg_log(LOG_INFO, "timeout\n");
            break;
        }

#ifdef CHECK_AFTER_WRITE
        if (write_check_nor((u32)src, addr, datalen, aes_control))
        {
            rc = SF_RC_PROG_FAIL;
            break;
        }
#endif
        len -= datalen;
        src += datalen;
    }

    sflash_write_enable(sfd, 0);
    return rc;
}
#endif
#endif  // end BOOT_MODE_BOOT2

void sflash_read_func_init(flashdesc *fp)
{
    if (fp->type == FLASH_TYPE_NAND)
    {
// NAND FLASH//    
#if (CONFIG_READ_IO_MODE == 2)
        fp->read_bytes = nand_flash_dualspeed_read_bytes;
#elif (CONFIG_READ_IO_MODE == 4)
        fp->read_bytes = nand_flash_quadspeed_read_bytes;
#else
        fp->read_bytes = nand_flash_read_bytes;
#endif
    }
//////////////////////////////////////////////////
    else
    {
    // NOR FLASH
#if (CONFIG_READ_IO_MODE == 2)
        fp->read_bytes = sflash_dualspeed_read_bytes;
#elif (CONFIG_READ_IO_MODE == 4)
        fp->read_bytes = sflash_quadspeed_read_bytes;
#else
        fp->read_bytes = sflash_read_bytes;
#endif
//////////////////////////////////////////////////
    }
}

#ifdef BOOT_MODE_BOOT2
void sflash_write_func_init(flashdesc *fp)
{
    fp->erase = sflash_erase;

    if (fp->type == FLASH_TYPE_NAND)
    {
// NAND FLASH//
#if (CONFIG_WRITE_IO_MODE == 4)
        fp->prog = nand_flash_quadspeed_program;
#else
        fp->prog = nand_flash_program;
#endif
//////////////////////////////////////////////////
    }
    else
    {
// NOR FLASH
#if (CONFIG_WRITE_IO_MODE == 4)
        fp->prog = sflash_quadspeed_program;
#else
        fp->prog = sflash_program;
#endif
    }
//////////////////////////////////////////////////
}
#endif

void sflash_init_otp_nand_config(struct flash_dev_tab *fdb)
{
    // if need to test, fdb->flags = CMD_DUMMY_START | ENABLE_READ_DUMMY | (NAND_PAGE_SIZE_2K << PAGE_SIZE_SHIFT) | BAD_BLOCK_CHECK_ONE_PAGE;
//	    otp_config |= 1 << OTP_READ_CMD_DUMMY_SHIFT;
//	    otp_config |= 1 << OTP_READ_DATA_DUMMY_SHIFT;
//	    otp_config |= 0 << OTP_PAGE_SIZE_SHIFT;
//	    otp_config |= 0 << OTP_CHECK_BAD_BLOCK_SHIFT;
//	    efuse_write((u8*)&otp_config, 32, 2);
//	    otp_read_config();
    
    // parse setting from OTP NAND config
    fdb->id    = SF_ID_DEFAULT;         // default
    fdb->sdesc = &uniform_256KB[11];    // default

    // paese nand flash config
    fdb->flags = 0;

    if (otp_parse_config(OTP_READ_CMD_DUMMY_SHIFT))
    {
        fdb->flags |= CMD_DUMMY_START;
    }

    if (otp_parse_config(OTP_READ_DATA_DUMMY_SHIFT))
    {
        fdb->flags |= ENABLE_READ_DUMMY;
    }

    if (otp_parse_config(OTP_CHECK_BAD_BLOCK_SHIFT))
    {
        fdb->flags |= BAD_BLOCK_CHECK_TWO_PAGE;
    }

    fdb->flags |= (otp_parse_config(OTP_PAGE_SIZE_SHIFT) << PAGE_SIZE_SHIFT);
}

void sflash_init_ch_setting(void)
{
    dbg_log(LOG_VERBOSE, "before_SPIREG(CH0_BAUD) = %x\n", SPIREG(CH0_BAUD));
    dbg_log(LOG_VERBOSE, "before_SPIREG(CH0_MODE_CFG) = %x\n", SPIREG(CH0_MODE_CFG));

#if defined(IPL)
    unsigned long div;

    div = otp_parse_config(OTP_FLASH_CLK_DIV_SHIFT);
    if(div)
        div = (0x01 << div);
    else
        div = SPI_CLK_DIV;

    SPIREG(CH0_BAUD) = ((SPI_INT_DLY << SPI_DLYSHFT) | div);
#else
    SPIREG(CH0_BAUD) = ((SPI_INT_DLY << SPI_DLYSHFT) | SPI_CLK_DIV);
#endif
    SPIREG(CH0_MODE_CFG) =
        (SPI_DUMYH | SPI_CSRTRN | SPI_BYTEPKG | (7 << SPI_WTHSHFT) |
         (0 << SPI_CLDSHFT) | (4 << SPI_CSDSHFT) | SPI_DPACK | SPI_HBYTE |
         SPI_FSHMODE | SPI_TXD_DELAY);

#if (CONFIG_READ_IO_MODE == 2)
    SPIREG(CH1_BAUD) = ((SPI_INT_DLY << SPI_DLYSHFT) | SPI_CLK_DIV);
    SPIREG(CH1_MODE_CFG) =
        (SPI_DUMYH | SPI_CSRTRN | SPI_BYTEPKG | (7 << SPI_WTHSHFT) |
         (0 << SPI_CLDSHFT) | (4 << SPI_CSDSHFT) | SPI_DPACK | SPI_HBYTE |
         SPI_FSHMODE | SPI_TXD_DELAY);
#endif
#if (CONFIG_READ_IO_MODE == 4 || CONFIG_WRITE_IO_MODE == 4)
    SPIREG(CH2_BAUD) = ((SPI_INT_DLY << SPI_DLYSHFT) | SPI_CLK_DIV);
    SPIREG(CH2_MODE_CFG) =
        (SPI_DUMYH | SPI_CSRTRN | SPI_BYTEPKG | (7 << SPI_WTHSHFT) |
         (0 << SPI_CLDSHFT) | (4 << SPI_CSDSHFT) | SPI_DPACK | SPI_HBYTE |
         SPI_FSHMODE | SPI_TXD_DELAY);
    SPIREG(CH3_BAUD) = ((SPI_INT_DLY << SPI_DLYSHFT) | SPI_CLK_DIV);
    SPIREG(CH3_MODE_CFG) =
        (SPI_DUMYH | SPI_CSRTRN | SPI_BYTEPKG | (7 << SPI_WTHSHFT) |
         (0 << SPI_CLDSHFT) | (4 << SPI_CSDSHFT) | SPI_DPACK | SPI_HBYTE |
         SPI_FSHMODE | SPI_TXD_DELAY);
#endif
}

void sflash_init_db(flashdesc *fp, struct flash_dev_tab *fdb)
{
    u32 flags = fdb->flags;

    switch ((flags >> PAGE_SIZE_SHIFT) & 0x3)
    {
        case NAND_PAGE_SIZE_2K:
            fp->page_size = BASE_PAGE_SIZE;
            break;
        case NAND_PAGE_SIZE_4K:
            fp->page_size = BASE_PAGE_SIZE * 2;
            break;
        case NAND_PAGE_SIZE_8K:
            fp->page_size = BASE_PAGE_SIZE * 4;
            break;
        case NAND_PAGE_SIZE_16K:
            fp->page_size = BASE_PAGE_SIZE * 8;
            break;
        default:
            fp->page_size = BASE_PAGE_SIZE;
            break;
    }

    if (fp->type == FLASH_TYPE_NAND)
    {
        fp->block_size = fp->page_size * PAGES_PER_BLOCK * 2;
    }
    else
    {
        fp->block_size = NOR_BLOCK_SIZE;
    }

    fp->cmd_dummy_byte = (flags & CMD_DUMMY_START) ? 1 : 0;
    fp->read_dummy_byte = (flags & ENABLE_READ_DUMMY) ? 1 : 0;
    fp->check_page_type = (flags & BAD_BLOCK_CHECK_TWO_PAGE) ? BAD_BLOCK_CHECK_TWO_PAGE : BAD_BLOCK_CHECK_ONE_PAGE;

    fp->blockNum = (fdb->sdesc)->num;
    fp->sdesc = fdb->sdesc;
    fp->size = fp->block_size * fp->blockNum;
}

u32 boot_block_num;
void load_boot_block_num(void) __attribute__ ((noinline));
void load_boot_block_num(void)
{
    if(chip_revision>=2)
        boot_block_num = (*((volatile unsigned long *)0xbf005524UL) >> 8) & 0xFF;
    else
        boot_block_num = 0;
}
void set_boot_block_num(u32 block_num)
{
    *((volatile unsigned long *)0xbf005524UL) &= 0xFFFF00FFUL;
    *((volatile unsigned long *)0xbf005524UL) |= ((block_num & 0x0FF) << 8);
}

/*!
 * function: flash_init
 *
 *  \brief initial and detect flash
 *  \return check result code
 */
int sflash_init(flashdesc *fp, int boot_type)
{
    int rc, i;
    u32 is_need_read_id = 0;
    struct flash_dev_tab *fdb;

    if ( (boot_type != BOOT_FROM_NOR)
        && (boot_type != BOOT_FROM_NAND)
        && (boot_type != BOOT_FROM_NAND_WITH_OTP))
    {
        dbg_log(LOG_INFO, "invalid BOOT type\n");
        goto error;
    }

    if(chip_revision>=2)
        load_boot_block_num();

    sflash_init_ch_setting();

    // is enable secure boot
    if (otp_parse_config(OTP_ENABLE_SECURE_SHIFT))
    {
        dbg_log(LOG_VERBOSE, "enable secure boot\n");
        read_otp_key();
        is_enable_secure_boot = 1;
    }
    else
    {
        dbg_log(LOG_VERBOSE, "disable secure boot\n");
        is_enable_secure_boot = 0;
    }

    switch (boot_type)
    {
        case BOOT_FROM_NOR:
            dbg_log(LOG_VERBOSE, "Boot from NOR flash\n");
            fp->type = FLASH_TYPE_NOR;
            // is_need_read_id = 1;
            break;
        case BOOT_FROM_NAND:
            dbg_log(LOG_VERBOSE, "Boot from NAND flash\n");
            fp->type = FLASH_TYPE_NAND;
            is_need_read_id = 1;
            break;
        case BOOT_FROM_NAND_WITH_OTP:
            dbg_log(LOG_INFO, "Boot from NAND flash with OTP configuration\n");
            fp->type = FLASH_TYPE_NAND;
            fdb = &sflash_db[SFL_DEVTAB_DEFAULT + 1];
            sflash_init_otp_nand_config(fdb);
            break;
        default:
            goto error;
    }

    fp->arg.bank = 0x0;
    pf_head = fp;

    if (fp->type == FLASH_TYPE_NAND)
    {
//#ifdef BOOT_MODE_IPL
        //nand_reset();   // becuase we may load boot code from gdb, so nand_reset is needed now
        //nand_unlock();  // unlock is needed in boot2 to avoid that device block have locked 
//#endif
#ifdef BOOT_MODE_BOOT2
        nand_quard_speed_enable();
        // rc:00 => ECC disable, rc:10 => ECC enable
        dbg_log(LOG_VERBOSE, "B0h status = %08x\n", nand_read_feature_register());
        nand_read_protect_register();
#endif
    }
    else
    {
#ifdef BOOT_MODE_IPL
        if ((0x01 << otp_parse_config(OTP_FLASH_CLK_DIV_SHIFT))==SPI_CLK_DIV)
            nor_reset();
#endif
#ifdef BOOT_MODE_BOOT2
        nor_reset();
        nor_read_status_register2();
        nor_read_status_register3();
#endif
    }

    sflash_read_func_init(fp);
#ifdef BOOT_MODE_BOOT2
    sflash_write_func_init(fp);
#endif

    if (is_need_read_id)
    {
        // get flash id to do further initialization
        fp->get_id = sflash_id;
        rc = fp->get_id(&(fp->arg));
        fp->id = rc;
        dbg_log(LOG_VERBOSE, "flash id: %08x\n", rc);

        // NAND_FLASH check
        if (fp->type == FLASH_TYPE_NAND)
        {
            for (i = 0, fdb = &sflash_db[0]; i < SFL_DEVTAB_DEFAULT; i++, fdb++)
            {
                if (((fp->id & 0xffffff00) == (fdb->id & 0xffffff00))
                    || ((fp->id & 0xffff00) == (fdb->id & 0x00ffff00)))    //compare device id only
                {
                    dbg_log(LOG_VERBOSE, "Found corresponding device on flash_db\n");
                    goto found;
                }
            }

            goto error;
        }
#if 0	// read id for NOR flash
        else
        {
        // NOR FLASH check
            fp->type = FLASH_TYPE_NOR;

            dbg_log(LOG_INFO, "sflash_capacity_detect(rc)\n");
            if (sflash_capacity_detect(rc))
            {
                dbg_log(LOG_INFO, "Valid support manufacturer\n");
                fdb = &sflash_db[SFL_DEVTAB_DEFAULT];
            }
            else
            {
                dbg_log(LOG_INFO, "Unknown manufacturer\n");
                fdb = &sflash_db[SFL_DEVTAB_DEFAULT + 1];
            }
        }
#endif
    }

    if (fp->type == FLASH_TYPE_NOR)
    {
		dbg_log(LOG_INFO, "Boot from NOR flash with default configuration\n");
        fdb = &sflash_db[SFL_DEVTAB_DEFAULT + 1];
    }

found:
    sflash_init_db(fp, fdb);
    return fp->size;

error:
    return 0;
}

#ifdef CONFIG_FLASH_SW_LOCK
#define SW_LOCK_SECTOR_SIZE 0x10000
void flash_swlock_info(void)
{
    int i;
    u8 show = ~(0);
    flashdesc *fp = &flashcfg[0];
    u32 end = (fp->size - 1) / SW_LOCK_SECTOR_SIZE;

    dbg_log(LOG_INFO, "SW Lock Range:\n");
    for (i = 0; i <= end; i++)
    {
        if (fp->sw_lock[i] == show)
        {
            if (show)
            {
                dbg_log(LOG_INFO, "0x%08x ~ ", (i << 16));
            }
            else
            {
                dbg_log(LOG_INFO, "0x%08x\n", (i << 16) - 1);
            }
            show = ~show;
        }
    }
}

int flash_lock(u32 start, u32 len)
{
    int i;
    u32 end = start + len - 1;
    flashdesc *fp = &flashcfg[0];

    if (end >= fp->size)
    {
        return -1;
    }

    start /= SW_LOCK_SECTOR_SIZE;
    end /= SW_LOCK_SECTOR_SIZE;

    for (i = start; i <= end; i++)
    {
        fp->sw_lock[i] = ~(0);
    }
    return 0;
}

int flash_unlock(u32 start, u32 len)
{
    int i;
    u32 end = start + len - 1;
    flashdesc *fp = &flashcfg[0];

    if (end >= fp->size)
    {
        return -1;
    }

    start /= SW_LOCK_SECTOR_SIZE;
    end /= SW_LOCK_SECTOR_SIZE;

    for (i = start; i <= end; i++)
    {
        fp->sw_lock[i] = 0;
    }
    return 0;
}
#endif

#ifdef CONFIG_PDMA
void switch_pdma_interrupt(u32 status)
{
    // two status now: PDMA_POLLING(default) or PDMA_INTERRUPT
    pdma_status = status;
}
#endif

int flash_init(int boot_type)
{
    dbg_log(LOG_VERBOSE, "Flash_Control: flash_init()\n");

    int rc = 0;
    int size = 0;
    int i;
    flashdesc *fp, *lp;

#if 0
    if (pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head != null\n");
        return FL_ERR_NOT_INIT;
    }
#endif

    // initialize pointer of ctrl_buf for convenience
    pCtrl_buf = (u32 *)UNCACHED_ADDR(ctrl_buf);

    lp = 0;
    for (i = 0; i < CONFIG_FLASH_BANK_NO; i++)
    {
        fp = &flashcfg[i];
        rc = sflash_init(fp, boot_type);
        if (rc > 0)
        {
            dbg_log(LOG_VERBOSE, "flash%d base:%08x id:%08x sz:%0x\n", i, fp->base, fp->id, fp->size);

            if (lp)
            {
                lp->next = fp;
            }
            lp = fp;
            size += fp->size;
        }
        else
        {
            dbg_log(LOG_INFO, "flash init failed: %d\n", rc);
            goto fail;
        }
    }
    pf_head = &flashcfg[0];
    dbg_log(LOG_VERBOSE, "total size = %x\n", size);

#ifdef CONFIG_PDMA
    pdma_init();
#endif

#ifdef SERIAL_FLASH_TEST
    nand_test();
#endif

    return size;

fail:
    pf_head = 0;
    return -1;
}

#ifdef BOOT_MODE_BOOT2
/*!
 * function: flash_erase
 *
 *  \brief erase flash with length
 *  \param addr: address (offset) on flash
 *  \param len: how many bytes to erase
 *  \param check_sel: is need check and skip bad block
 *  \return check result code
 */
int flash_erase(u32 addr, u32 len, u32 check_sel)
{
    int rc = 0;
    u32 saddr = 0;
    flashsec *sd;
    u32 j, i;
    u32 progress = 0;
    flashdesc *flp;
    u32 ctrl_ratio = 1;
    u32 erase_offset = 0;
    u32 pre_skip_bad = 0;
    u32 block_size;
    u32 page_size;

    if (!pf_head)
    {
        return FL_ERR_NOT_INIT;
    }

    block_size = pf_head->block_size;
    page_size = pf_head->page_size;
#ifdef BAD_BLOCK_MANAGE
    if (pf_head->type == FLASH_TYPE_NAND && check_sel == CHECK_BAD_BLOCK)
    {
        pre_skip_bad = get_shift_bad_block_num(addr/block_size);
    }
#endif

    sd = pf_head->sdesc;
    // adapt len, because usually 1 page only have 0x800 bytes
    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_ratio = 2;
        len = len * ctrl_ratio;
    }

    for (saddr = i = j = 0; (saddr < pf_head->size) && (saddr < (addr + len));)
    {
        if ((saddr + block_size) <= addr)
        {
            goto next;
        }
        if ((saddr - progress) >= block_size)
        {
            dbg_log(LOG_INFO, ".");
            progress = saddr;
        }

#ifdef BAD_BLOCK_MANAGE
        if (pf_head->type == FLASH_TYPE_NAND && check_sel == CHECK_BAD_BLOCK)
        {
            while (pre_skip_bad > 0)
            {
                erase_offset += skip_bad_block(saddr / block_size + erase_offset, RET_TYPE_OFFSET);
                erase_offset++;
                pre_skip_bad--;
            }
            erase_offset += skip_bad_block(saddr / block_size + erase_offset, RET_TYPE_OFFSET);
        }
#endif

//	        dbg_log(LOG_INFO, "\nerase block num = %d, saddr = 0x%x\n", j + erase_offset, saddr + erase_offset * block_size);
        rc = pf_head->erase(&(pf_head->arg), saddr + erase_offset * block_size, SF_ERASE_SEC, sd[i].ecmd);
        if (rc)
        {
            dbg_log(LOG_INFO, "erase failed !!\n");
            return rc;
        }
      next:
        if (++j < sd[i].num)   //within the same sector size
        {
            saddr += block_size;
        }
        else
        {
            saddr += block_size;
            i++;
            j = 0;
        }
    }

    return rc;
}

/*!
 * function: flash_erase_skip_bad_block
 *
 *  \brief erase flash with length, skip bad block, not shift erase index
 *  \param addr: address (offset) on flash
 *  \param len: how many bytes to erase
 *  \return check result code
 */
int flash_erase_skip_bad_block(u32 addr, u32 len)
{
    int rc = 0;
    u32 saddr = 0;
    flashsec *sd;
    u32 j, i;
    u32 progress = 0;
    flashdesc *flp;
    u32 ctrl_ratio = 1;
    u32 block_size;
    u32 page_size;

    if (!pf_head)
    {
        return FL_ERR_NOT_INIT;
    }

    block_size = pf_head->block_size;
    page_size = pf_head->page_size;
    sd = pf_head->sdesc;
    // adapt len, because usually 1 page only have 0x800 bytes
    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_ratio = 2;
        len = len * ctrl_ratio;
    }

    for (saddr = i = j = 0; (saddr < pf_head->size) && (saddr < (addr + len));)
    {
        if ((saddr + block_size) <= addr)
        {
            goto next;
        }
        if ((saddr - progress) >= block_size)
        {
            dbg_log(LOG_INFO, ".");
            progress = saddr;
        }

#ifdef BAD_BLOCK_MANAGE
        if (pf_head->type == FLASH_TYPE_NAND)
        {
            if(nand_is_bad_block(saddr / block_size))
            {
                rc = 0;
                goto next;
            }
        }
#endif

        rc = pf_head->erase(&(pf_head->arg), saddr, SF_ERASE_SEC, sd[i].ecmd);
        if (rc)
        {
            dbg_log(LOG_INFO, "erase failed !!\n");
            return rc;
        }
next:
        if (++j < sd[i].num)   //within the same sector size
        {
            saddr += block_size;
        }
        else
        {
            saddr += block_size;
            i++;
            j = 0;
        }
    }

    return rc;
}

int flash_erase_no_check_block(u32 addr, u32 len)
{
    return flash_erase(addr, len, NO_CHECK_BAD_BLOCK);
}

/*!
 * function: flash_write
 *
 *  \brief Write to serial flash from buffer
 *  \param addr: address (offset) on flash
 *  \param from: from buffer
 *  \param len: how many bytes to write
 *  \param check_sel: is need check and skip bad block
 *  \return check result code
 */
int flash_write(u32 addr, u32 from, u32 len, u32 check_sel)
{
    int rc = 0;
    flashdesc *flp;
    u32 erase_offset = 0;
    u32 pre_skip_bad = 0;
    u32 datalen;
    u32 aes_control;
    u32 block_size;
    u32 page_size;
    u32 full_page;

    // how many bytes offset from block start to addr(need handle of NAND control)
    u32 start_offset = 0;
    // how many bytes could use in the block(also need to handle of NAND control)
    u32 use_size = 0;
    // for write in correct flash memory address in NAND,
    // because page size may be 0x800, but 0x800 ~ 0xfff is spare area
    u32 ctrl_ratio = 1;

    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

    aes_control = init_secure_settings(ACTION_WRITE);

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_ratio = 2;
    }

    block_size = pf_head->block_size;
    page_size = pf_head->page_size;
    full_page = page_size * 2 - 1;
    use_size = block_size / ctrl_ratio;
    start_offset = (((addr % block_size) & ~full_page) / ctrl_ratio) + (addr & full_page);
#ifdef BAD_BLOCK_MANAGE
    if (pf_head->type == FLASH_TYPE_NAND && check_sel == CHECK_BAD_BLOCK)
    {
        pre_skip_bad = get_shift_bad_block_num(addr/block_size);
    }
#endif

    while (len > 0)
    {
        dbg_log(LOG_INFO, ".");

        // over current block, start_offset just need to use once
        if (start_offset != 0 && (start_offset + len) > use_size)
        {
            datalen = use_size - start_offset;
            start_offset = 0;
        }
        else
        {
            datalen = (len > use_size) ? use_size : len;
        }

#ifdef BAD_BLOCK_MANAGE
        if (pf_head->type == FLASH_TYPE_NAND && check_sel == CHECK_BAD_BLOCK)
        {
            while (pre_skip_bad > 0)
            {
                erase_offset += skip_bad_block(addr / block_size + erase_offset, RET_TYPE_OFFSET);
                erase_offset++;
                pre_skip_bad--;
            }
            erase_offset += skip_bad_block(addr / block_size + erase_offset, RET_TYPE_OFFSET);
        }
#endif

#ifdef CONFIG_PDMA
        rc = pf_head->prog(&(pf_head->arg), addr + erase_offset * block_size, datalen, from, PDMA_SWITCH, aes_control);
#else
        rc = pf_head->prog(&(pf_head->arg), addr + erase_offset * block_size, datalen, from, 0, aes_control);
#endif
        len -= datalen;
        from += datalen;
        // move to next block start address in flash
        addr = ((addr / block_size) + 1) * block_size;
    }

    return rc;
}

int flash_write_no_check_block(u32 addr, u32 from, u32 len)
{
    return flash_write(addr, from, len, NO_CHECK_BAD_BLOCK);
}
#endif

u32 curr_flash_boot_attempts;
#ifdef IPL
int flash_read_bytes(u32 addr, u32 src, int len, u32 check_sel)
{
    int rc = 0;
    flashdesc *flp;
    u32 read_addr;
    u32 erase_offset = 0;
    u32 datalen = 0;
    u32 aes_control;
    u32 block_size;
    u32 page_size;

    // for page management in inner loop
    int inner_len = 0;
    u32 inner_addr = 0;
    // how many bytes could use in the block(also need to handle of NAND control)
    u32 use_size = 0;
    // for write in correct flash memory address in NAND,
    // because page size may be 0x800, but 0x800 ~ 0xfff is spare area
    u32 ctrl_ratio = 1;

    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

#ifndef CONFIG_PDMA
    sf_flush_cache_all();
#endif

    aes_control = init_secure_settings(ACTION_READ);

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_ratio = 2;
    }

    block_size = pf_head->block_size;
    page_size = pf_head->page_size;
    use_size = block_size / ctrl_ratio;
	
    // block management loop
    while (len > 0)
    {
        dbg_log(LOG_INFO, ".");

        // over current block, start_offset just need to use once
        datalen = (len > use_size) ? use_size : len;
        
#ifdef BAD_BLOCK_MANAGE
        if (pf_head->type == FLASH_TYPE_NAND)
        {
            if(check_sel == CHECK_BAD_BLOCK)
            {
                erase_offset += skip_bad_block(addr / block_size + erase_offset, RET_TYPE_OFFSET);
            }
            else if (check_sel == INCREASE_BAD_BLOCK)
            {
                erase_offset = curr_flash_boot_attempts;
                curr_flash_boot_attempts++;
            }
        }
#endif
        // page managment loop
        inner_addr = addr;
        while (datalen > 0)
        {
            inner_len = (datalen > BASE_PAGE_SIZE) ? BASE_PAGE_SIZE : datalen;

            read_addr = inner_addr + (erase_offset * block_size);
#ifdef CONFIG_PDMA
            rc = pf_head->read_bytes(&(pf_head->arg), read_addr, src, inner_len, PDMA_SWITCH, aes_control);
#else
            rc = pf_head->read_bytes(&(pf_head->arg), read_addr, src, inner_len, 0, aes_control);
#endif
            datalen -= inner_len;
            len -= inner_len;
            src += inner_len;
            inner_addr = (inner_addr + inner_len);
            // means have read end of page
            if (inner_addr % page_size == 0 && pf_head->type == FLASH_TYPE_NAND)
            {
                inner_addr = (inner_addr + page_size);
            }
        }
        // end of page managment loop

        // move to next block start address in flash
        addr = ((addr / block_size) + 1) * block_size;
    }

#ifndef CONFIG_PDMA
    sf_flush_cache_all();
#endif

    return rc;
}
#else
extern u32 boot_block_num;
int is_print_progress = 0;
int flash_read_bytes(u32 addr, u32 src, int len, u32 check_sel)
{
    int rc = 0;
    //flashdesc *flp;
    u32 read_addr;
    u32 erase_offset = 0;
    u32 datalen = 0;
    u32 aes_control;
    u32 block_size;
    u32 page_size;
    u32 full_page;
#if defined(BOOT_MODE_BOOT2) && defined(BAD_BLOCK_MANAGE)
    u32 pre_skip_bad = 0;
#endif

    // for page management in inner loop
    int inner_len = 0;
    u32 inner_addr = 0;
    // how many bytes offset from block start to addr(need handle of NAND control)
    u32 start_offset = 0;
    // how many bytes could use in the block(also need to handle of NAND control)
    u32 use_size = 0;
    // for write in correct flash memory address in NAND,
    // because page size may be 0x800, but 0x800 ~ 0xfff is spare area
    u32 ctrl_ratio = 1;

    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

#ifndef CONFIG_PDMA
    sf_flush_cache_all();
#endif

    aes_control = init_secure_settings(ACTION_READ);

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_ratio = 2;
    }

    block_size = pf_head->block_size;
    page_size = pf_head->page_size;
    full_page = page_size * 2 - 1;
    use_size = block_size / ctrl_ratio;
    start_offset = (((addr % block_size) & ~full_page) / ctrl_ratio ) + (addr & full_page);

    if(check_sel == CHECK_BAD_BLOCK)
    {
        if (pf_head->type == FLASH_TYPE_NAND)
        {
            erase_offset += skip_bad_block(addr / block_size + erase_offset, RET_TYPE_OFFSET);
            if(chip_revision<2)
                set_boot_block_num(erase_offset);
        }
    }

    // block management loop
    while (len > 0)
    {
        if (is_print_progress)
        {
            dbg_log(LOG_INFO, ".");
        }

        // over current block, start_offset just need to use once
        if (start_offset != 0 && (start_offset + len) > use_size)
        {
            datalen = use_size - start_offset;
            start_offset = 0;
        }
        else
        {
            datalen = (len > use_size) ? use_size : len;
        }
        /* Boot1 code (from byte 0 ~ 16/24k, uses 2048bytes per AES CBC)  (IPL fixed to use 2048 AES CBC)
           Blank page (from byte 24k ~ 32k , fill with 0xff for erased page reference check)
           Boot2 code (from byte 32k ~ 128k, uses 1024bytes per AES CBC)

           This BOOT2_IMG_AES_CBC_PAGE_SIZE setting is only for BOOT1 load/decrypt Blank page/BOOT2 code only

           We mixed two kinds of AEC CBC block size to add more protect to our secure boot code.
           The end user cannot produce secure boot code by just write plain text boot code with secure boot enabled mtd driver.
           (e.g. dd if=boot.img of=/dev/mtdblock2  (with secure boot enabled system), then dump the raw data in /dev/mtdblock2)
           The result with "dd" always have AES CBC set 2048 (for 2048bytes page size NAND), or AES CBC set to 256 (for NOR).
         */
#define BOOT2_IMG_AES_CBC_PAGE_SIZE 1024
        // page managment loop
        inner_addr = addr;
        while (datalen > 0)
        {
            if (pf_head->type == FLASH_TYPE_NAND)
            {
                inner_len = (datalen > BOOT2_IMG_AES_CBC_PAGE_SIZE) ? BOOT2_IMG_AES_CBC_PAGE_SIZE : datalen;
            }
            else
            {
#ifdef BOOT_MODE_BOOT2
                inner_len = (datalen> NOR_PAGE_SIZE) ? NOR_PAGE_SIZE : datalen;
#else
                inner_len = (datalen > BOOT2_IMG_AES_CBC_PAGE_SIZE) ? BOOT2_IMG_AES_CBC_PAGE_SIZE : datalen;
#endif
            }

            read_addr = inner_addr + (erase_offset * block_size);

            is_print_progress = 1;
#ifdef CONFIG_PDMA
            rc = pf_head->read_bytes(&(pf_head->arg), read_addr, src, inner_len, PDMA_SWITCH, aes_control);
#else
            rc = pf_head->read_bytes(&(pf_head->arg), read_addr, src, inner_len, 0, aes_control);
#endif
            is_print_progress = 0;

#ifdef BOOT_MODE_BOOT2
            if (is_ecc_cant_handle)
            {
                dbg_log(LOG_INFO, "read_addr=0x%x !!\n"
                       "Interrupt read operation and mark current block as bad block\n");
                is_ecc_cant_handle = 0;
                mark_bad_block(read_addr/block_size);
                return SF_RC_READ_TO;
            }
#endif

//          dbg_log(LOG_INFO, "len = 0x%x\n", len);
//          dbg_log(LOG_INFO, "addr = 0x%x, rc = 0x%x\n", inner_addr + erase_offset * block_size, rc);
//          if (src != 0)
//          {
//              dbg_log(LOG_INFO, "src2 = 0x%x\n", *(u32*)src);
//          }

            datalen -= inner_len;
            len -= inner_len;
            src += inner_len;
            inner_addr = (inner_addr + inner_len);
            // means have read end of page
            if (inner_addr % page_size == 0 && pf_head->type == FLASH_TYPE_NAND)
            {
                inner_addr = (inner_addr + page_size);
            }
        }
        // end of page managment loop

        // move to next block start address in flash
        addr = ((addr / block_size) + 1) * block_size;
    }

#ifndef CONFIG_PDMA
    sf_flush_cache_all();
#endif

    return rc;
}
#endif

int flash_read_bytes_no_check_block(u32 addr, u32 src, u32 len)
{
    return flash_read_bytes(addr, src, len, NO_CHECK_BAD_BLOCK);
}

u32 get_max_block_num(u32 pf_index)
{
    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

    flashdesc *fp = (pf_head + pf_index);
    return fp->blockNum;
}

#ifdef CHECK_AFTER_WRITE
int flash_read_without_pdma(u32 offset, u32 count)
{
    int rc = 0;

    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

    flashdesc *fp;
    fp = pf_head;
    rc = fp->read_bytes(&(fp->arg), offset, 0, count, 0, 0);

    return rc;
}
#endif

#ifdef BOOT_MODE_IPL
void boot_from_flash(void)
{
    dbg_log(LOG_INFO, "Flash_Control: boot_from_flash()\n");
    
    u32 dataLen = BOOT1_SIZE;
    u32 _pc = BOOT1_BASE;
    void (*func) (void);
    u32 addr = BOOT1_PAGE_START;
    u32 src = BOOT1_BASE;

    change_aes_enable_status(ENABLE_SECURE_OTP);
#if defined(IPL)
    if(otp_parse_config(OTP_DISABLE_BAD_BLOCK_CHECK_SHIFT))
        flash_read_bytes(addr, src, dataLen, INCREASE_BAD_BLOCK);
    else
        flash_read_bytes(addr, src, dataLen, CHECK_BAD_BLOCK); 
#else
    flash_read_bytes(addr, src, dataLen, CHECK_BAD_BLOCK); 
#endif

    dbg_log(LOG_INFO, "Jump to boot1 !!!!\n");
    if (_pc)
    {
        func = (void *) _pc;
        HAL_DCACHE_WB_INVALIDATE_ALL();
        HAL_ICACHE_INVALIDATE_ALL();
        if (0 != boot1_check())
            return;
        func();
        asm volatile ("nop;nop;nop;");
    }
}
#endif

#ifdef BOOT_MODE_BOOT1

void load_decrypted_erased_page(void)
{
    u32 dataLen = BLANK_PAGE_SIZE;
    u32 addr = BLANK_PAGE_START_NAND;
    u32 src = ENCRYPTED_BLANK_PAGE_BASE;

    if (!pf_head)
        return;

    if (pf_head->type == FLASH_TYPE_NOR)
        addr = BLANK_PAGE_START_NOR;

    addr += (pf_head->block_size * boot_block_num);

    change_aes_enable_status(ENABLE_SECURE_OTP);

    if(chip_revision>=2)
        flash_read_bytes(addr, src, dataLen, NO_CHECK_BAD_BLOCK);
    else
        flash_read_bytes(addr, src, dataLen, CHECK_BAD_BLOCK);
}

void load_boot2(void)
{
    dbg_log(LOG_INFO, "Flash_Control: load_boot2()\n");

    //int rc = 0;
    u32 dataLen = BOOT2_SIZE;
    u32 addr = BOOT2_PAGE_START_NAND;

    if (!pf_head)
    {
        return;
    }
    
    if (pf_head->type == FLASH_TYPE_NOR)
    {
        addr = BOOT2_PAGE_START_NOR;
    }
    u32 src = BOOT2_BASE;

    addr += (pf_head->block_size * boot_block_num);

    change_aes_enable_status(ENABLE_SECURE_OTP);

    if(chip_revision>=2)
        flash_read_bytes(addr, src, dataLen, NO_CHECK_BAD_BLOCK);
    else
        flash_read_bytes(addr, src, dataLen, CHECK_BAD_BLOCK);
}
#endif

#ifdef BOOT_MODE_BOOT2
char capp_magic[3] = IH_MAGIC;
unsigned long uboot_magic = IH_UBMAGIC;
unsigned long ubi_magic = UBI_EC_HDR_MAGIC;
unsigned long squash_magic = SQUASHFS_MAGIC;

void flash_read_header(u32 addr, u32 size, u32 check_sel)
{
    u32 buf[0x1000 / 4];     // to support max 4k page now
    u32 page_size;

    if (!pf_head)
    {
        return;
    }

    if (is_enable_secure_boot)
    {
        page_size = (pf_head->type == FLASH_TYPE_NAND) ? pf_head->page_size : NOR_PAGE_SIZE;
        change_aes_enable_status(ENABLE_SECURE_OTP);
        flash_read_bytes(addr, (u32)buf, page_size, check_sel);
        change_aes_enable_status(DISABLE_SECURE);

        memcpy(pCtrl_buf, UNCACHED_ADDR(buf), size);
    }
    else
    {
        flash_read_bytes(addr, 0, size, check_sel);
    }
}

u32 get_cmdline_type(ci_header_t *ci_hdr)
{
    u32 type = DEFAULT_CMDLINE;
    u32 rootfs_magic;

    if(!ci_hdr)
        return type;

    // if not combined-image type
    if(ci_hdr->magic != cpu_to_be32(CI_MAGIC))
        return type;

    rootfs_magic = ci_hdr->rootfs_signature;

    if(UBI_EC_HDR_MAGIC == cpu_to_be32(rootfs_magic))
    {
        dbg_log(LOG_VERBOSE, "rootfs: UBI\n");
        type = UBI_CMDLINE;
    }
    else
    {
        dbg_log(LOG_VERBOSE, "rootfs: none UBI\n");
    }

    return type;
}

void write_boot_cmdline(u32 img_type, u32 img_size, ci_header_t *ci_hdr)
{
    int i;
    int count = 0;
    int bad_count = 0;
    int is_have_bad_blk = 0;
    u32 cmdline_type = DEFAULT_CMDLINE;
    char boot_cmdline[COMMAND_LINE_SIZE];
    char mem_setting[16];
    char *p_cmdline = boot_cmdline;
    u32 block_size = pf_head->block_size;
    u32 kernel_blk_num;
    u32 total_num;
    u32 ctrl_ratio = 1;;

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        ctrl_ratio = 2;
    }
    kernel_blk_num = (img_size - 1)/(pf_head->block_size / ctrl_ratio) + 1;
    total_num = 4 + kernel_blk_num;     // 1 block for boot.img, 1 block for bootvar,
                                        // 1 block for cdb, 1 block for combined-image header and the others for kernel
    dbg_log(LOG_VERBOSE, "total block num = %d\n", total_num);

    /*================ write necessary command line arguments =================*/
    memset(boot_cmdline, 0, COMMAND_LINE_SIZE);

    if (img_type == CI_IMAGE_TYPE)
    {
        cmdline_type = get_cmdline_type(ci_hdr);
    }

#ifdef CONFIG_FPGA
    sprintf(mem_setting, " mem=256M@0");
#else
    sprintf(mem_setting, " mem=%dM@0", *(volatile unsigned int*)DDR_SIZE_INFO_ADDR);
#endif
    switch (cmdline_type)
    {
        case UBI_CMDLINE:
            strcat(p_cmdline, "ubi.mtd=6 ubi.block=0,0 root=/dev/ubiblock0_0");
            strcat(p_cmdline, mem_setting);
            strcat(p_cmdline, " console=ttyS0 kgdboc=ttyS0 loglevel=8 rootfstype=squashfs,ubifs noinitrd");
            break;
        default:
            strcat(p_cmdline, "root=/dev/mtdblock6 rw");
            strcat(p_cmdline, mem_setting);
            strcat(p_cmdline, " console=ttyS0 kgdboc=ttyS0 loglevel=8 rootfstype=squashfs,jffs2 noinitrd");
    }

#ifdef CONFIG_FPGA
    strcat(p_cmdline, " lpj=375000");
#else
    strcat(p_cmdline, " lpj=2750000");
#endif

    if(bootvars.quiet)
    {
        strcat(p_cmdline, " quiet");
    }

    p_cmdline += strlen(boot_cmdline);


    /*================= write advance command line arguments ==================*/
    p_cmdline += sprintf(p_cmdline, " swcfg=%s", bootvars.swcfg);
    p_cmdline += sprintf(p_cmdline, " ai=%s", bootvars.ai);
    p_cmdline += sprintf(p_cmdline, " img_size=%08x", img_size);

    if (pf_head->page_size == BASE_PAGE_SIZE * 2)
    {
        p_cmdline += sprintf(p_cmdline, " CI_BLKSZ=262144 ");
    }
    else
    {
        p_cmdline += sprintf(p_cmdline, " CI_BLKSZ=131072 ");
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        for (i = 0; i < pf_head->blockNum; i++)
        {
            if (nand_is_bad_block(i) == DETECT_GOOD_BLOCK)
            {
                count++;
            }
            else
            {
                if (!is_have_bad_blk)
                {
                    is_have_bad_blk = 1;
                    p_cmdline += sprintf(p_cmdline, " bad_list=");
                }
    
                bad_count++;
                if (bad_count > MAX_BAD_BLOCK_NUM)
                {
                    dbg_log(LOG_INFO, "Too much bad block, can't handle with this NAND flash\n");
                    break;
                }
    
                p_cmdline += sprintf(p_cmdline, "%d,", i);
            }
    
            if (count == total_num)
            {
                if (is_have_bad_blk)
                {
                    strcat(p_cmdline, "end");
                }
                break;
            }
        }
    }

    memcpy(BOOT_CMDLINE_DRAM_ADDR, boot_cmdline, COMMAND_LINE_SIZE);
    dbg_log(LOG_VERBOSE, "boot_cmdline = %s\n", boot_cmdline);
}

u32 load_app_image(u32 *h)
{
    dbg_log(LOG_VERBOSE, "load_app_image()\n");

    int rc = 0;
    u32 img_size = 0;
    u32 boot_load_addr;
    u32 dest_addr;
    u32 img_type = UNKNOWN_IMAGE_TYPE;
    u32 ubmagic;
    u32 time, interval;
    image_header_t *img_hdr;
    ci_header_t ci_hdr;
    struct img_head *ih;

    if (!pf_head)
    {
        return img_type;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        boot_load_addr = pf_head->block_size * BOOT_MID_BLOCK_NUM;
    }
    else
    {
        boot_load_addr = pf_head->block_size * BOOT_MID_BLOCK_NUM_NOR;
    }


    //rc = flash_read_bytes(boot_load_addr, 0, sizeof(struct img_head), CHECK_BAD_BLOCK);
    rc = flash_read_bytes(boot_load_addr, 0, CTRL_BUF_SIZE, CHECK_BAD_BLOCK);

    ih = (struct img_head *) pCtrl_buf;
    if (!memcmp(&ih->magic[0], &capp_magic[0], 3))
    {
        // old image
        dbg_log(LOG_INFO, "Image type: CAPP\n");
        *h = dest_addr = CONFIG_UNDIR_FLASH_BASE;
        img_type = CAPP_IMAGE_TYPE; 
        img_size = ih->size;
        img_size += sizeof(struct img_head);
    }
    else
    {
        memcpy(&ci_hdr, (ci_header_t *) pCtrl_buf, sizeof(ci_header_t));
        ci_header_t *ci_hdr = (ci_header_t *) pCtrl_buf;
        ubmagic = ci_hdr->magic;

        if(ubmagic == cpu_to_be32(CI_MAGIC))
        {
            // combined image
            dbg_log(LOG_VERBOSE, "Image type: Combined Image\n");

            img_type = CI_IMAGE_TYPE; 
            if (pf_head->type == FLASH_TYPE_NAND)
            {
                boot_load_addr += pf_head->block_size;
            }
            else
            {
                boot_load_addr += (pf_head->block_size * 2);
            }
        }
        else
        {
            img_hdr = (image_header_t *) pCtrl_buf;
            ubmagic = cpu_to_be32(img_hdr->ih_magic);

            if (!memcmp(&ubmagic, &uboot_magic, 4))
            {
                // uBoot Image
                dbg_log(LOG_INFO, "Image type: UBOOT\n");
                img_type = UBOOT_IMAGE_TYPE;
            }
        }

        if (img_type == CI_IMAGE_TYPE)
        {
            flash_read_header(boot_load_addr, sizeof(image_header_t), CHECK_BAD_BLOCK);
            img_hdr = (image_header_t *) pCtrl_buf;
        }

        if (img_type == CI_IMAGE_TYPE || img_type == UBOOT_IMAGE_TYPE)
        {
            *h = cpu_to_be32(img_hdr->ih_load);
            dest_addr = (*h - sizeof(image_header_t));
            img_size = cpu_to_be32(img_hdr->ih_size) + sizeof(image_header_t);

            dbg_log(LOG_VERBOSE, "ih_load = 0x%x\n", cpu_to_be32(img_hdr->ih_load));
            dbg_log(LOG_VERBOSE, "ih_ep = 0x%x\n", cpu_to_be32(img_hdr->ih_ep));
        }        
        else
        {
            dbg_log(LOG_INFO, "Image type: UNKNOWN\n");
            return img_type;
        }
    }

    dbg_log(LOG_VERBOSE, "img_size = 0x%x\n", img_size);
    if (img_size == 0)
    {
        dbg_log(LOG_INFO, "img_size shouldn't be empty !!\n");
        return img_type;
    }

    // write boot cmdline for linux
    write_boot_cmdline(img_type, img_size, &ci_hdr);

    time = clock_get();
    dbg_log(LOG_VERBOSE, "dest_addr = 0x%x\n", dest_addr);

    // reset memory that kernel needed to avoid the problem,
    // go flash may get failed because wrong dcrc value after tftp the same image
    sf_flush_cache_all();
    change_aes_enable_status(ENABLE_SECURE_OTP);

    // extend length img_size for secure boot
//  if (is_enable_secure_boot)
//  {
        img_size = ((img_size + pf_head->page_size - 1) / pf_head->page_size) * pf_head->page_size;
//  }

    rc = flash_read_bytes(boot_load_addr, dest_addr, img_size, CHECK_BAD_BLOCK);
    change_aes_enable_status(DISABLE_SECURE);

    //dbg_log(LOG_VERBOSE, "\n");
    interval = clock_get();
    interval = ((interval >= time) ? (interval - time) : (~0UL - time + interval));
    dbg_log(LOG_VERBOSE, "Flash read time = %d ms\n", interval);

    return img_type;
}

int is_ubi_image(unsigned char *buf_addr)
{
    u32 cmdline_type;

    if(!buf_addr)
        return 0;

    cmdline_type = get_cmdline_type((ci_header_t *) buf_addr);

    // if firmware is not UBI type
    if(cmdline_type != UBI_CMDLINE)
        return 0;

    return 1;
}

void flash_erase_appdata(void)
{
    u32 boot_load_addr;
    u32 erase_len;

    if (!pf_head)
    {
        return;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        boot_load_addr = pf_head->block_size * BOOT_MID_BLOCK_NUM;
    }
    else
    {
        boot_load_addr = pf_head->block_size * BOOT_MID_BLOCK_NUM_NOR;
    }

    erase_len = (pf_head->size - boot_load_addr);
    if (pf_head->type == FLASH_TYPE_NAND)
        erase_len /= 2;

    //printf("=>> erase %x %x %x\n", boot_load_addr, pf_head->size, erase_len);
    flash_erase_skip_bad_block(boot_load_addr, erase_len);
}

int usr_cmd_flash_read_bytes(u32 addr, u32 src, int len, u32 check_sel)
{
    //u32 ctrl_ratio = 1;

    if (!pf_head)
    {
        return;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr *= 2;
    }

    return flash_read_bytes(addr, src, len, check_sel);
}

int usr_cmd_flash_erase(u32 addr, u32 len, u32 check_sel)
{
    u32 ctrl_ratio = 1;

    if (!pf_head)
    {
        return;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr *= 2;
    }

    return flash_erase(addr, len, check_sel);
}

int usr_cmd_flash_write(u32 addr, u32 from, u32 len, u32 check_sel)
{
    if (!pf_head)
    {
        return;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr *= 2;
    }

    return flash_write(addr, from, len, check_sel);
}

int burn_boot1_into_flash(u32 offset)
{
    return flash_write(offset + BOOT1_PAGE_START, (u32)BOOT1_BURN_BUF, BOOT1_SIZE, CHECK_BAD_BLOCK);
}

int burn_boot2_into_flash(u32 offset)
{
    return flash_write(offset + pf_head->block_size * BOOT_MID_BLOCK_NUM, (u32)BOOT2_BURN_BUF, BOOT2_SIZE, CHECK_BAD_BLOCK);
}

#ifdef CONFIG_UART
#define XMODEM_DEST_SIZE    0xa00000     // 16 m
void burn_linux_into_flash(void)
{
    u32 addr;

    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr = pf_head->block_size * BOOT_MID_BLOCK_NUM;
    }
    else
    {
        addr = pf_head->block_size * BOOT_MID_BLOCK_NUM_NOR;
    }

    flash_erase(addr, XMODEM_DEST_SIZE, CHECK_BAD_BLOCK);
    flash_write(addr, (u32)BOOT_BURN_BASE, XMODEM_DEST_SIZE, CHECK_BAD_BLOCK);
}

void burn_linux_from_xmodem(void)
{
    xmodem_rx((void *) BOOT_BURN_BASE, XMODEM_DEST_SIZE);
    burn_linux_into_flash();
}
#endif

int is_full_empty_page(u8 *addr, u32 page_size)
{
    int i;

    for (i = 0; i < page_size; i++)
    {
        if (*addr != 0xff)
        {
            return 0;
        }
    }

    return 1;
}

int read_setting_block(u32 src)
{
    int i = 0;
    u32 block_size;
    u32 page_size;
    flashsec *sd;
    u32 addr = 0;
    u32 len = 0;

    if (!pf_head)
    {
        return FL_ERR_NOT_INIT;
    }

    page_size = pf_head->page_size;
    block_size = pf_head->block_size;
    len = block_size;
    addr = BOOT_SETTING_INDEX_NOR * block_size;

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr = BOOT_SETTING_INDEX * block_size;
        len = block_size / 2;
    }

    if (is_enable_secure_boot)
    {
        // because caller need to know whether bootvar block is full empty block or not,
        // so we need to translate full empty block for caller
        flash_read_bytes(addr, src + block_size, len, CHECK_BAD_BLOCK);
        change_aes_enable_status(ENABLE_SECURE_OTP);
        flash_read_bytes(addr, src + (block_size * 2), len, CHECK_BAD_BLOCK);
        change_aes_enable_status(DISABLE_SECURE);

        for (i = 0; i < len; i += page_size)
        {
            if (is_full_empty_page((u8 *)(src + block_size + i), page_size))
            {
                memcpy((src + i), (src + block_size + i), page_size);
            }
            else
            {
                memcpy((src + i), (src + (block_size * 2) + i), page_size);
            }
        }
    }
    else
    {
        flash_read_bytes(addr, src, len, CHECK_BAD_BLOCK);
    }

    dbg_log(LOG_VERBOSE, "read setting block success, block size = 0x%x\n", block_size);
    return block_size;
}

int erase_setting_block()
{
    int rc = 0;
    u32 block_size;
    flashsec *sd;
    u32 addr = 0;
    u32 len = 0;

    if (!pf_head)
    {
        return FL_ERR_NOT_INIT;
    }

    block_size = pf_head->block_size;
    len = block_size;
    addr = BOOT_SETTING_INDEX_NOR * block_size;
    
    if (pf_head->type == FLASH_TYPE_NAND)
    {
        addr = BOOT_SETTING_INDEX * block_size;
        len = block_size / 2;
    }

    rc = flash_erase(addr, len, CHECK_BAD_BLOCK);
    dbg_log(LOG_INFO, "erase setting block, addr = 0x%x\n", addr);

    return rc;
}

int program_setting_page(u32 page_index, u32 src)
{
    int rc = 0;
    u32 block_size;
    u32 page_size;
    u32 addr = 0;
    u32 page_interval;

    if (!pf_head)
    {
        return FL_ERR_NOT_INIT;
    }

    block_size = pf_head->block_size;
    page_size = pf_head->page_size;
    page_interval = page_size;

    // because NOR flash page not have spare area, so need to divide 2
    addr = (BOOT_SETTING_INDEX_NOR * block_size) + (page_interval * page_index / 2);

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        page_interval = page_size * 2;
        addr = (BOOT_SETTING_INDEX * block_size) + (page_interval * page_index);
    }

    change_aes_enable_status(ENABLE_SECURE_OTP);
    if (pf_head->type == FLASH_TYPE_NAND)
    {
        rc = flash_write(addr, src, page_size, CHECK_BAD_BLOCK);
    }
    else
    {
        rc = flash_write(addr, src, (page_size / 2), CHECK_BAD_BLOCK);
    }
    change_aes_enable_status(DISABLE_SECURE);
    dbg_log(LOG_INFO, "program setting page, addr = 0x%x, page_size = 0x%x\n", addr, page_size);

    return rc;
}

u32 get_page_size()
{
    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

    if (pf_head->type == FLASH_TYPE_NAND)
    {
        return pf_head->page_size;
    }
    else
    {
        return (pf_head->page_size/2);
    }
}

u32 get_block_size()
{
    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

    return pf_head->block_size;
}

u32 get_linux_block_index()
{
    if (!pf_head)
    {
        dbg_log(LOG_INFO, "OH! pf_head is null\n");
        return FL_ERR_NOT_INIT;
    }

    return (pf_head->type == FLASH_TYPE_NAND) ? BOOT_MID_BLOCK_NUM : BOOT_MID_BLOCK_NUM_NOR;
}

int is_boot_image(unsigned long src)
{
    unsigned long *ptr = (unsigned long *)(src + 0x10);

    if ((ptr[0] == 0xac800000UL) && (ptr[1] == 0xac800004UL)
        && (ptr[2] == 0xac800008UL) && (ptr[3] == 0xac80000cUL))
    {
#if defined(BIG_ENDIAN)
        return BOOT_IMAGE_BE;
#else
        return BOOT_IMAGE_LE;
#endif
    }

    if ((ptr[0] == 0x000080acUL) && (ptr[1] == 0x040080acUL)
        && (ptr[2] == 0x080080acUL) && (ptr[3] == 0x0c0080acUL))
    {
#if defined(BIG_ENDIAN)
        return BOOT_IMAGE_LE;
#else
        return BOOT_IMAGE_BE;
#endif
    }

    return 0;
}
#endif

/*!
 * function: init_secure_settings
 *
 *  \brief Initial secure setting by some parameters
 */
u32 init_secure_settings(u32 action)
{
    u32 aes_control = 0;
    PDMAREG(PDMA_AES_CTRL) = (PDMA_AES_MODE_CBC | PDMA_AES_KEYLEN_256);

    if (!is_enable_secure_boot)
    {
        aes_enable_status = DISABLE_SECURE;
    }

    switch (aes_enable_status)
    {
        case DISABLE_SECURE:
            break;
        case ENABLE_SECURE_REG:
            if (action == ACTION_WRITE)
            {
                aes_control = (AES_ENABLE | AES_OP_ENCRYPT | AES_KEYSEL_REG);
            }
            else
            {
                aes_control = (AES_ENABLE | AES_OP_DECRYPT | AES_KEYSEL_REG);
            }
            break;
        case ENABLE_SECURE_OTP:
            // need to call read otp function to let hardware fetch otp key
            read_otp_key();

            if (action == ACTION_WRITE)
            {
                aes_control = (AES_ENABLE | AES_OP_ENCRYPT | AES_KEYSEL_OTP);
            }
            else
            {
                aes_control = (AES_ENABLE | AES_OP_DECRYPT | AES_KEYSEL_OTP);
            }
            break;
        default:
            break;
    }

    return aes_control;
}

extern int efuse_read(u8 *bufp, u32 index_start, u32 element_num);
void read_otp_key()
{
    u8 otpkey_r[32];
    efuse_read(otpkey_r, 0, 32);
}

/*!
 * function: aes_enable_status
 *
 *  \brief Change aes status of secure boot
 *  \param key_source: where is the source to get AES key
 */
void change_aes_enable_status(u32 aes_status)
{
    aes_enable_status = aes_status;
}

