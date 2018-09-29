#ifndef __STR_H__
#define __STR_H__

#define DEBUG_COMMAND
//#define CUT_THROUGH_TRIAL
//#define TEST_MEMORY
#define CSUM_MEMORY

#define UART_BASE  0xBF002900UL

#if defined(CONFIG_FPGA)
#define UART_CLK   (40 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE  115200
#else
#define UART_CLK   (120 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE  115200
#endif

#define URBR	0x00
#define  URBR_RDY         (1<<23)
#define URCS	0x04
#define URCS_TF (1<<3)
#define URCS_TB (1<<0)
#define URCS_BRSHIFT 16
#define URBR_DTSHFT  24

#define REG_WRITE32(addr, val)  (*(volatile unsigned long *)(addr)) = ((unsigned long)(val))
#define REG_READ32(addr)        (*(volatile unsigned long *)(addr))
#define REG_UPDATE32(x, val, mask) do {                  \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (x);               \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(x) = newval;                \
} while(0)

#endif // __STR_H__

