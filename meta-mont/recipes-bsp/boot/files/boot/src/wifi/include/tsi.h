#ifndef __TSI_H__
#define __TSI_H__

#include <arch/chip.h>

#define TSI_ADCDUMP_CTRL            (TSI_BASE + 0x00)
   #define TSI_BURST_SIZE              0x00007000  /* bit[14:12], default 0x7 (64 bytes)  */
      #define TSI_DEFAULT_BURST_SIZE 64
   #define TSI_BYTE_SWAP               0x00000100  /* enable byte-swap, for bit-endian mode */
   #define TSI_DIRECT_TRANSFER         0x00000080
   #define TSI_TRANSFER_MODE           0x00000040
   #define TSI_OP_MODE                 0x00000004
   #define TSI_ENABLE                  0x00000001
#define TSI_PKT_SIZE_RD_TH          (TSI_BASE + 0x04)
   #define TSI_BUF_MAX_BURST_NUM       0x00ff0000
#define ADC_DUMP_BUF_START_ADDR     (TSI_BASE + 0x08)
#define ADC_DUMP_BUF_SIZE           (TSI_BASE + 0x0c)          /* size in 8bytes unit */
#define ADC_DUMP_DST_BASE_ADDR_0    (TSI_BASE + 0x10)
#define ADC_DUMP_DST_MEM_SIZE_0     (TSI_BASE + 0x14)
#define ADC_DUMP_DST_BASE_ADDR_1    (TSI_BASE + 0x18)
#define ADC_DUMP_DST_MEM_SIZE_1     (TSI_BASE + 0x1c)

#define TSI_INTR_ENABLE             (TSI_BASE + 0x40)
#define TSI_INTR_CLEAR              (TSI_BASE + 0x44)
#define TSI_INTR_STATUS             (TSI_BASE + 0x48)
#define TSI_STATUS                  (TSI_BASE + 0x4C)
    #define TSI_CORE_IDLE              0x00000002
    #define TSI_AXI_MASTER_IDLE        0x00000004
#define TSI_ADC_DUMP_DST_CURR_ADDR  (TSI_BASE + 0x50)

#endif // __TSI_H__
