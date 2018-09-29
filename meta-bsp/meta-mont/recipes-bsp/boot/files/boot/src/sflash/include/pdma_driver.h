/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
 *   \file pdma_driver.h
 *   \brief Peripheral DMA API
 *   \author Montage
 */

#ifndef __PDMA_DRIVER_H__
#define __PDMA_DRIVER_H__

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#if defined(SIM)
#include "mac.h"
#else
#include "mt_types.h"
#endif

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define UNCACHED_ADDR(x) (((u32) (x) & 0x1fffffffUL) | 0xa0000000UL)    // mapping physical memory
#define PHYSICAL_ADDR(x) ((u32) (x) & 0x1fffffffUL)     // mapping physical memory

#define PDMA_DESCR_ALIGN_SIZE   0x20

#define PDMA_POOLING_COUNT 1000

#define PDMA_AHB_BUS   (0 << 1)
#define PDMA_AXI_BUS   (1 << 1)
#define PDMA_KEEP_ADDR (0 << 0)
#define PDMA_INC_ADDR  (1 << 0)

#define PDMA_TOTAL_LEN_SHIFT  (16)
#define PDMA_AES_CTRL_SHIFT   (12)
#define PDMA_ENDIAN_SHIFT     (11)
#define PDMA_INTR_SHIFT       (10)
#define PDMA_SRC_SHFT         (8)
#define PDMA_DEST_SHFT        (6)
#define PDMA_DEVICE_SIZE_SHFT (1)
#define PDMA_VALID     1

#define PDMA_CH_UART0_TX 0
#define PDMA_CH_UART0_RX 1
#define PDMA_CH_UART1_TX 2
#define PDMA_CH_UART1_RX 3
#define PDMA_CH_UART2_TX 4
#define PDMA_CH_UART2_RX 5
#define PDMA_CH_SPI_TX   6
#define PDMA_CH_SPI_RX   7
#define PDMA_CH_LCD_SPI_TX   8
#define PDMA_CH_LCD_SPI_RX   9

#define PDMA_ENABLE            0x00
#define PDMA_CTRL              0x04
#define PDMA_INTR_ENABLE       0x08
#define PDMA_INTR_STATUS       0x0c
#define PDMA_CHANNEL_PRIORITY  0x10

#define LDMA_CH0_DESCR_BADDR   0x14
#define LDMA_CH0_CURRENT_ADDR  0x18
#define LDMA_CH0_STATUS        0x1c

#define LDMA_CH1_DESCR_BADDR   0x20
#define LDMA_CH1_CURRENT_ADDR  0x24
#define LDMA_CH1_STATUS        0x28

#define LDMA_CH2_DESCR_BADDR   0x2c
#define LDMA_CH2_CURRENT_ADDR  0x30
#define LDMA_CH2_STATUS        0x34

#define LDMA_CH3_DESCR_BADDR   0x38
#define LDMA_CH3_CURRENT_ADDR  0x3c
#define LDMA_CH3_STATUS        0x40

#define LDMA_CH4_DESCR_BADDR   0x44
#define LDMA_CH4_CURRENT_ADDR  0x48
#define LDMA_CH4_STATUS        0x4c

#define LDMA_CH5_DESCR_BADDR   0x50
#define LDMA_CH5_CURRENT_ADDR  0x54
#define LDMA_CH5_STATUS        0x58

#define LDMA_CH6_DESCR_BADDR   0x5c
#define LDMA_CH6_CURRENT_ADDR  0x60
#define LDMA_CH6_STATUS        0x64

#define LDMA_CH7_DESCR_BADDR   0x68
#define LDMA_CH7_CURRENT_ADDR  0x6c
#define LDMA_CH7_STATUS        0x70

#define LDMA_CH8_DESCR_BADDR   0x74
#define LDMA_CH8_CURRENT_ADDR  0x78
#define LDMA_CH8_STATUS        0x7c

#define LDMA_CH9_DESCR_BADDR   0x80
#define LDMA_CH9_CURRENT_ADDR  0x84
#define LDMA_CH9_STATUS        0x88

#define PDMA_DEBUG0            0x8c
#define PDMA_DEBUG1            0x90
#define LDMA_DEBUG_OUT_LOW     0x94
#define LDMA_DEBUG_OUT_HIGH    0x98

#define CH6_SLV_ADDR  SPI_BASE   // SF Write
#define CH7_SLV_ADDR  SPI_BASE   // SF Read
#define CH8_SLV_ADDR  GSPI_BASE   // LCD Write
#define CH9_SLV_ADDR  GSPI_BASE   // LCD Read

#define PDMA_MAX_LENGTH            0x00000FFE0 //65535-32 bytes

#define PDMA_AES_CTRL          0x9C
    #define PDMA_AES_REVERSE   0x0001
    #define PDMA_AES_MODE      0x001E
        #define PDMA_AES_MODE_ECB    0x0000
        #define PDMA_AES_MODE_CBC    0x0002
    #define PDMA_AES_KEYLEN    0x0060
        #define PDMA_AES_KEYLEN_128  0x0000
        #define PDMA_AES_KEYLEN_192  0x0020
        #define PDMA_AES_KEYLEN_256  0x0040

#define PDMA_AES_CTRL2         0xA0
    #define PDMA_AES_OTPKEY_ENC_DISABLE 0x00000001

#define PDMA_AES_KEY           0xA4    // 8 x 32-bits registers

#define AES_ENABLE      (0x01 << 12)
#define AES_OP_ENCRYPT  0x00
#define AES_OP_DECRYPT  (0x01 << 13)
#define AES_KEYSEL_OTP  0x00
#define AES_KEYSEL_REG  (0x01 << 14)

typedef void (*dma_callback) (void *priv);
typedef struct
{
    u32 channel;
    u32 desc_addr;
    u32 next_addr;
    u32 src_addr;
    u32 dest_addr;
    u32 dma_total_len;
    u32 aes_ctrl;
    u32 endian;
    u32 intr_enable;
    u32 src;
    u32 dest;
    u32 fifo_size;
    u32 valid;
} __attribute__ ((aligned(PDMA_DESCR_ALIGN_SIZE), __packed__)) pdma_descriptor;

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void pdma_init(void);

int pdma_submit(unsigned int ch, unsigned int src, unsigned int dst,
                unsigned short len, dma_callback callback, void *param);

void pdma_desc_set(pdma_descriptor *descriptor);

void pdma_program_aes_key(unsigned char *key, int length);

u32 pdma_intr_check(void);

void pdma_pooling_wait(void);

#endif             // __PDMA_DRIVER_H__
