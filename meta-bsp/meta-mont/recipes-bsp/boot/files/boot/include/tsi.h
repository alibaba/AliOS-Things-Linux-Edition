/*!
*   \file tsi.h
*   \brief Transport Stream Input API
*   \author Montage
*/
#ifndef __TSI_H__
#define __TSI_H__

#include <arch/chip.h>

#define MPEGTS_SYMBOL 0x47
#define MPEGTS_FRAME_LEN 188

#define TSI_BURSTSIZE 7         // (x+1)*8, x=1~7
#define TSI_PFLTR_NUM 16
#define TSI_FLAG_BOUNDARY_STOP 1<<0

struct tsi_dev
{
    unsigned short filter_id[TSI_PFLTR_NUM];    /* filter_number */
    unsigned int filter_number; /* filter_number */
    unsigned int flags;         /* interrupt switch */
    unsigned int addr0;         /* destination 0 base address */
    unsigned int addr1;         /* destination 1 base address */
    unsigned int size0;         /* destination 0 size */
    unsigned int size1;         /* destination 1 size */
    unsigned int intr_pktnum_th;        /* interrupt packet number threshold */
    unsigned int intr_status;   /* interrupt condition */
    unsigned int irq_count;     /* irq count */
    unsigned int irq_stat_unrun;
    unsigned int irq_stat_pdone;
    unsigned int irq_stat_dstch;
    unsigned int underrun_count;        /* underrun count */
    unsigned int unsync_count;
    unsigned int resync_count;
    void (*func) (void *dev, unsigned int status);      /* hook interrupt handler */
};

#endif                          // __TSI_H__
