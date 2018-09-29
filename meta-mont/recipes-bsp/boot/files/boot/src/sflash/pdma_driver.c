/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
 *   \file pdma_driver.c
 *   \brief Peripheral DMA API
 *   \author Montage
 */

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>

#include "include/flash_config.h"
#include "include/pdma_driver.h"

#ifdef BOOT_MODE_IPL
#include <common/chip.h>
#else
#include <arch/chip.h>
#endif

#if defined(BOOT_MODE_BOOT2) && defined(PDMA_INTERRUPT)
#include <arch/irq.h>
#endif

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/    

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
static u32 pdma_intr_index = 0;
#ifdef BOOT_MODE_BOOT2
u32 ch6_descr[4] __attribute__ ((aligned (32)));
#ifdef LCD_PDMA
u32 ch8_descr[4] __attribute__ ((aligned (32)));
u32 ch9_descr[4] __attribute__ ((aligned (32)));
#endif
#endif
u32 ch7_descr[4] __attribute__ ((aligned (32)));

u32 little_endian = 0;

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
#if 0
void pdma_program_aes_key(unsigned char *key, int length)
{
    // length shall be 16 (128bits), 24 (192bits) or 32 (256bits)
    u32 keyval;
    unsigned char *k = key;
    int i;

    for (i = 0; i < length/4; i++)
    {
        keyval = (k[3] << 24) | (k[2] << 16) | (k[1] << 8) | k[0];

        PDMAREG((PDMA_AES_KEY + i*4)) = keyval;
        k += 4;
    }
}
#endif

void translate_pdma_status(u32 ahb_read_data, u32 pdma_rx_status, u32 pdma_tx_status)
{
    if (ahb_read_data == 0)
    {
        // should not happen
        dbg_log(LOG_INFO, "ERROR: Get PDMA INTR status is 0 !!!\n");
    }
    else
    {
        // too many message, set default to comment
        // dbg_log(LOG_INFO, "Get PDMA INTR status: %x\n", ahb_read_data);
    }

    if ((pdma_rx_status >> 1) == 0x1)
    {
        dbg_log(LOG_INFO, "PDMA_CH_SPI_RX timeout !!\n");
    }
    else if ((pdma_tx_status >> 1) == 0x1)
    {
        dbg_log(LOG_INFO, "PDMA_CH_SPI_TX timeout !!\n");
    }

    // clear INTR status
    PDMAREG(PDMA_INTR_STATUS) = ahb_read_data;
}

void pdma_pooling_wait(void)
{
    volatile u32 ahb_read_data;
    volatile u8 pdma_rx_status = 0x00;
    volatile u8 pdma_tx_status = 0x00;
    int count = 0;
    
    while ((pdma_rx_status == 0x00) && (pdma_tx_status == 0x00))
    {
        ahb_read_data = PDMAREG(PDMA_INTR_STATUS);
        pdma_rx_status = (ahb_read_data >> PDMA_CH_SPI_RX * 2) & 0x03;
        pdma_tx_status = (ahb_read_data >> PDMA_CH_SPI_TX * 2) & 0x03;

        count++;
        if (count >= PDMA_POOLING_COUNT)
        {
            dbg_log(LOG_INFO, "ahb_read_data = %x\n", ahb_read_data);
            dbg_log(LOG_INFO, "pdma_rx_status = %x\n", pdma_rx_status);
            dbg_log(LOG_INFO, "pdma_tx_status = %x\n", pdma_tx_status);
#ifdef BOOT_MODE_BOOT2
            dbg_log(LOG_INFO, "CH6_DESC_BASE = %x\n", (*(u32 *)ch6_descr));
#endif
            dbg_log(LOG_INFO, "CH7_DESC_BASE = %x\n", (*(u32 *)ch7_descr));
            break;
        }
    }

    translate_pdma_status(ahb_read_data, pdma_rx_status, pdma_tx_status);
}

#if defined(PDMA_INTERRUPT) && defined(BOOT_MODE_BOOT2)
void pdma_intr_handler(void)
{
//	    dbg_log(LOG_INFO, "PDMA interrupt coming !!!\n");

    volatile u32 ahb_read_data;
    volatile u8 pdma_rx_status = 0x00;
    volatile u8 pdma_tx_status = 0x00;

    ahb_read_data = PDMAREG(PDMA_INTR_STATUS);
    pdma_rx_status = (ahb_read_data >> PDMA_CH_SPI_RX * 2) & 0xff;
    pdma_tx_status = (ahb_read_data >> PDMA_CH_SPI_TX * 2) & 0xff;

    translate_pdma_status(ahb_read_data, pdma_rx_status, pdma_tx_status);
    
    pdma_intr_index = 0;
}

u32 pdma_intr_check(void)
{
    return pdma_intr_index;
}
#endif

void pdma_init(void)
{
    dbg_log(LOG_VERBOSE, "pdma_init()\n");

    if (0 == (*((volatile unsigned long *)PIN_STRAP_REG_ADDR) & 0x01))
    {
        little_endian = 1;
    }
    dbg_log(LOG_VERBOSE, "little_endian = 0x%x\n", little_endian);

    // TODO will add hardware reset for PDMA here

    // clear PDMA register
    PDMAREG(PDMA_INTR_ENABLE) = 0;
    PDMAREG(PDMA_ENABLE) = 0;

#ifdef BOOT_MODE_BOOT2
    // Enable channel 6 & 7 interrupt
    PDMAREG(PDMA_INTR_ENABLE) = (1 << PDMA_CH_SPI_TX);
    PDMAREG(PDMA_INTR_ENABLE) |= (1 << PDMA_CH_SPI_RX);
    // Set channel 6 descriptors
    PDMAREG(LDMA_CH6_DESCR_BADDR) = PHYSICAL_ADDR(ch6_descr);
    // Set channel 7 descriptors
    PDMAREG(LDMA_CH7_DESCR_BADDR) = PHYSICAL_ADDR(ch7_descr);

    // Enable PDMA channel 6 & 7
    PDMAREG(PDMA_ENABLE) = (1 << PDMA_CH_SPI_TX);
    PDMAREG(PDMA_ENABLE) |= (1 << PDMA_CH_SPI_RX);
#ifdef LCD_PDMA
    // Enable channel 8 & 9 interrupt
    PDMAREG(PDMA_INTR_ENABLE) |= (1 << PDMA_CH_LCD_SPI_TX);
    PDMAREG(PDMA_INTR_ENABLE) |= (1 << PDMA_CH_LCD_SPI_RX);
    // Set channel 8 descriptors
    PDMAREG(LDMA_CH8_DESCR_BADDR) = PHYSICAL_ADDR(ch8_descr);
    // Set channel 9 descriptors
    PDMAREG(LDMA_CH9_DESCR_BADDR) = PHYSICAL_ADDR(ch9_descr);

    // Enable PDMA channel 8 & 9
    PDMAREG(PDMA_ENABLE) |= (1 << PDMA_CH_LCD_SPI_TX);
    PDMAREG(PDMA_ENABLE) |= (1 << PDMA_CH_LCD_SPI_RX);
#endif
#else
    // Enable channel 7 interrupt
    PDMAREG(PDMA_INTR_ENABLE) = (1 << PDMA_CH_SPI_RX);
    // Set channel 7 descriptors
    PDMAREG(LDMA_CH7_DESCR_BADDR) = PHYSICAL_ADDR(ch7_descr);
    // Enable PDMA channel 6 & 7
    PDMAREG(PDMA_ENABLE) = (1 << PDMA_CH_SPI_TX);
    PDMAREG(PDMA_ENABLE) |= (1 << PDMA_CH_SPI_RX);
#endif

#if defined(PDMA_INTERRUPT) && defined(BOOT_MODE_BOOT2)
    request_irq(IRQ_PDMA, &pdma_intr_handler, (void *) IRQ_PDMA);
#endif
}

#if 0
int pdma_submit(unsigned int ch, unsigned int src, unsigned int dst,
                unsigned short len, dma_callback callback, void *param)
{
    int ret = -1;

    return ret;
}
#endif

u32 pdma_desc_rdata;
void pdma_desc_set(pdma_descriptor *descriptor)
{
    pdma_intr_index = 1;

    // Set Destination Addr
    (*(u32 *)(descriptor->desc_addr + 4)) = descriptor->dest_addr;

    // Set Source Addr
    (*(u32 *)(descriptor->desc_addr + 8)) = descriptor->src_addr;

    // Set Next descriptor Addr
    (*(u32 *)(descriptor->desc_addr + 12)) = descriptor->next_addr;

    // Set length, size, owner
    (*(u32 *)descriptor->desc_addr) = (little_endian << PDMA_ENDIAN_SHIFT) |
                                      (descriptor->dma_total_len << PDMA_TOTAL_LEN_SHIFT) |
                                      (descriptor->intr_enable << PDMA_INTR_SHIFT) |
                                      (descriptor->src << PDMA_SRC_SHFT) |
                                      (descriptor->dest << PDMA_DEST_SHFT) |
                                      (descriptor->aes_ctrl) |
                                      (descriptor->fifo_size << PDMA_DEVICE_SIZE_SHFT) |
                                      (descriptor->valid);
	pdma_desc_rdata = *((volatile u32 *)descriptor->desc_addr);  
}

