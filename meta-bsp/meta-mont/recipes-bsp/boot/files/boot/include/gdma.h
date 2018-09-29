/*!
*   \file gdma.h
*   \brief Generic DMA API
*   \author Montage
*/
#ifndef __CHEETAH_GDMA_H__
#define __CHEETAH_GDMA_H__

#include <mt_types.h>

#define UNCACHED_ADDR(x) (((u32) (x) & 0x0fffffffUL) | 0xa0000000UL)
#define PHYSICAL_ADDR(x) ((u32) (x) & 0x0fffffffUL)

#define GDMA_DESCR_ALIGN_SIZE   0x20
#define DMADBG(args...)     printk(args)
#define DMAPANIC(args...)   do { panic(args);  while(1); } while(0);

#define DMA_ENABLE          0x00
#define DMA_KICK            0x04
#define DMA_INTR_STATUS     0x08
#define DMA_INTR_STATUS_COUNT       0x00000001UL
#define DMA_INTR_STATUS_TIME        0x00000002UL
#define DMA_INTR_MASK       0x0c
#define DMA_INTR_MASK_COUNT         0x00000001UL
#define DMA_INTR_MASK_TIME          0x00000002UL
#define DMA_INTR_THRESHOLD  0x10
#define DMA_INTR_THRESHOLD_TIME     0xFFFF0000UL
#define DMA_INTR_THRESHOLD_COUNT    0x0000FFFFUL
#define DMA_DESCR_BADDR     0x14
#define DMA_READ_DESCR_ADDR 0x18

typedef void (*dma_callback) (void *priv);
typedef struct
{
    union
    {
#ifdef BIG_ENDIAN
        struct
        {
            volatile u16 ctrl:2;
            u16 eor:1;
            volatile u16 sw_inuse:1;    // software only controlled bit (indicate the descr. is used/hold by someone)
             u16:9;
            u16 cksum_init:1;
            u16 operation:2;

            volatile u16 cksum_result;

            u32 dest_addr;
            u32 src_addr;

            u16 cksum_offset;
            u16 cksum_length;

            u16 operation_length;
            u16 cksum_initval;

            u32 sw_callback;
            u32 sw_priv;
            u32 sw_priv2;
        };

        struct
        {
            volatile u16 ctrl_word0;

            volatile u16 cksum_result_t;

            u32 dest_addr_t;
            u32 src_addr_t;

            u16 cksum_offset_t;
            u16 cksum_length_t;

            u16 operation_length_t;
            u16 cksum_initval_t;

            u32 sw_callback_t;
            u32 sw_priv_t;
            u32 sw_priv2_t;
        };
#else //Little endian
        struct
        {
            volatile u16 cksum_result;

            u16 operation:2;
            u16 cksum_init:1;
            u16:9;
            volatile u16 sw_inuse:1;    // software only controlled bit (indicate the descr. is used/hold by someone)
            u16 eor:1;
            volatile u16 ctrl:2;

            u32 dest_addr;
            u32 src_addr;

            u16 cksum_length;
            u16 cksum_offset;

            u16 cksum_initval;
            u16 operation_length;

            u32 sw_callback;
            u32 sw_priv;
            u32 sw_priv2;
        };

        struct
        {

            volatile u16 cksum_result_t;
            volatile u16 ctrl_word0;

            u32 dest_addr_t;
            u32 src_addr_t;

            u16 cksum_length_t;
            u16 cksum_offset_t;

            u16 cksum_initval_t;
            u16 operation_length_t;

            u32 sw_callback_t;
            u32 sw_priv_t;
            u32 sw_priv2_t;
        };
#endif
    };
} __attribute__ ((aligned(GDMA_DESCR_ALIGN_SIZE), __packed__)) dma_descriptor;

#define GDMA_CTRL_GO            0x3
#define GDMA_CTRL_GO_POLL       0x2
#define GDMA_CTRL_DONE_SKIP     0x1
#define GDMA_CTRL_DONE_STOP     0x0

#define GDMA_OP_COPY        0x0
#define GDMA_OP_COPY_CSUM   0x1
#define GDMA_OP_CSUM        0x2

int gdma_submit(unsigned int op, unsigned int src, unsigned int dst,
                unsigned short len, dma_callback callback, void *param);

#endif                          // __CHEETAH_GDMA_H__
