/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file unc_lz.c
*   \brief entry point of boot2
*   \author Montage
*/

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#if defined(CONFIG_FPGA)
#define UART_CLK   (40 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE  115200
#else
#define UART_CLK   (120 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE  115200
#endif
typedef unsigned int u32;

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/cpu.h>
#include <arch/chip.h>
#include <common.h>
#include "sflash/include/flash_config.h"

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
int putchar(int c);
int flash_erase(u32 addr, u32 len, u32 check_sel);
void nand_reset(void);
void nor_reset(void);

#define UART_BASE               0xBF002900UL

#define URBR                    0x00
#define URCS                    0x04
#define URCS_TF                (1<<3)
#define URCS_TB                (1<<0)
#define URCS_BRSHIFT            16
#define URBR_DTSHFT             24

#define REG_WRITE_32(addr, val)  (*(volatile unsigned long *)(addr)) = ((unsigned long)(val))
#define REG_READ_32(addr)       (*((volatile u32 *)(addr)))

int chip_revision;
void chip_revision_detection(void)
{
    if((*(volatile unsigned long *)0xbfc03aa0UL)==0x27bdffd8)
        chip_revision = 1;
    else if((*(volatile unsigned long *)0xbfc03aa0UL)==0xac627ff8)
        chip_revision = 2;
}

void panther_putc(char c)
{
    while (URCS_TF & REG_READ_32(UART_BASE + URCS));

    REG_WRITE_32(UART_BASE + URBR, (((unsigned char)c) << URBR_DTSHFT));
}

void uart_init(void)
{
    REG_WRITE_32(UART_BASE + URCS, ((UART_CLK/UART_TARGET_BAUD_RATE)<<URCS_BRSHIFT));
}

void boot1_clear(void)
{
    unsigned long *ptr = (unsigned long *)0xB0000010UL;

    ptr[0] = 0xFFFFFFFFUL;
    ptr[1] = 0x0UL;
    ptr[2] = 0x5A5A5A5AUL;
    ptr[3] = 0xDEADC0DEUL;
}

/*!
 * function:
 *
 *  \brief      c main entry
 *  \return
 */
#define REG_UPDATE_32(x, val, mask) do {                  \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (x);               \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(x) = newval;                \
} while(0)
int bmain()
{
    REG_UPDATE_32(0xBF004A20UL, 0x00100000, 0x00F00000);
    uart_init();

    chip_revision_detection();

#if !defined(CONFIG_FPGA)
#if defined(CONFIG_USE_DDR3)
    // 1.5V/1.2V Buck register settings
    *((volatile unsigned long *) 0xBF004F38) = 0x0073F108;
    *((volatile unsigned long *) 0xBF004F3C) = 0x0091AFAD;
    *((volatile unsigned long *) 0xBF004F44) = 0x0FC723C3;
    *((volatile unsigned long *) 0xBF004F60) = 0xFC710000;
    *((volatile unsigned long *) 0xBF004F34) = 0x73495BBB;
#else
    // 1.8V/1.2V Buck register settings
    *((volatile unsigned long *) 0xBF004F38) = 0x0073F108;
    *((volatile unsigned long *) 0xBF004F3C) = 0x0091AFAD;
    *((volatile unsigned long *) 0xBF004F44) = 0x2FC723C3;
    *((volatile unsigned long *) 0xBF004F60) = 0xFC710000;
    *((volatile unsigned long *) 0xBF004F34) = 0x73495BBB;
#endif

#define GPIO_DRV1_REG 0xBF004A30UL
#define GPIO_DRV3_REG 0xBF004A38UL
    // change driving for ASIC mode
    REG_UPDATE_32(GPIO_DRV1_REG, 0x0003C000UL, 0x0003C000UL);
    REG_UPDATE_32(GPIO_DRV3_REG, 0x00003FC0UL, 0x00003FC0UL);

#define PMU_CTRL_REG     0xBF004804UL
#define SLP_PD_CTRL_REG  0xBF004808UL
    // we need set 0xbf004804[7] = 1, and 0xbf004F34[7](BUCK_DIG_CTRL) = 1, some registers need to check this bit
    REG_UPDATE_32(PMU_CTRL_REG, 0x00000080UL, 0x00000080UL);

    REG_WRITE_32(SLP_PD_CTRL_REG, 0xFFE5EFFEUL);
#endif

    putchar('N');putchar('A');putchar('N');putchar('D');
    putchar(0xd);putchar(0xa);

    nand_reset();
    if(flash_init(BOOT_FROM_NAND)>=0)
       flash_erase(0, 0x100000, 0);

    putchar('N');putchar('A');putchar('N');putchar('D');
    putchar(0xd);putchar(0xa);

    nand_reset();
    if(flash_init(BOOT_FROM_NAND_WITH_OTP)>=0)
       flash_erase(0, 0x100000, 0);

    putchar('N');putchar('O');putchar('R');
    putchar(0xd);putchar(0xa);

    nor_reset();
    if(flash_init(BOOT_FROM_NOR)>=0)
       flash_erase(0, 0x80000, 0);

    putchar('E');putchar('R');putchar('A');putchar('S');putchar('E');putchar('D');
    putchar(0xd);putchar(0xa);

    boot1_clear();
    //REG_WRITE_32(0xBF004820UL, 0x01000001UL);

    while(1)
      ;

    return 0;
}
