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
#define _LZMA_FIXED_INT_DATA
#define BOOT2_START_BLOCK   72
#define BOOT2_BLOCK_CNT     192         // 96 k
#define XMODEM_DEST_SIZE    0x20000     // 128 k
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
#include <otp.h>
#include "sflash/include/flash_config.h"

#if defined(BIG_ENDIAN)
#include "lzo/unlzo_be.c"
#else
#include "lzo/unlzo_le.c"
#endif

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
int xmodem_rx(unsigned char *dest, int destsz);
void ddr2_init(void);
void ddr3_init(void);
void sf_flush_cache_all(void);
int flash_erase(u32 addr, u32 len, u32 check_sel);
void switch_pdma_interrupt(u32 status);
void flush_cache(void);
void load_boot2(void);
int flash_init(int boot_type);
int putchar(int c);
void clock_init(void);
void mini_sdhc_init(void);
unsigned int sdio_read(u32 start, u32 blkcnt, unsigned long dst);
int otp_parse_config(int shift_val);
void load_decrypted_erased_page(void);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern void setup_cache(void);

extern unsigned long otp_config;

int chip_revision;
void chip_revision_detection(void)
{
    if((*(volatile unsigned long *)0xbfc03aa0UL)==0x27bdffd8)
        chip_revision = 1;
    else if((*(volatile unsigned long *)0xbfc03aa0UL)==0xac627ff8)
        chip_revision = 2;
}

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
#if defined(CONFIG_BOOT1_MINI_SDHC)
void load_boot2_from_sd(void)
{
    int ret;
    clock_init();

    while(1)
    {
        mini_sdhc_init();
        ret =  sdio_read(BOOT2_START_BLOCK, BOOT2_BLOCK_CNT, BOOT2_BASE);
        if(ret)
            break;
    }
}
#endif

#if defined(CONFIG_BOOT1_UART)
void uart_controller_rx_init(void)
{
    int i;

    URREG(URCS) = (URCS_REN | ((UART_CLK/UART_TARGET_BAUD_RATE)<<URCS_BRSHFT));

    /* cleanup RX FIFO, if any */
    for (i = 0; i < 32; i++)
    {
        URREG(URBR);
    }
}

void load_boot2_from_uart(void)
{
    uart_controller_rx_init();
    xmodem_rx((void *) BOOT2_BASE, XMODEM_DEST_SIZE);
}
#endif

void *memcpy(void *__to, __const__ void *__from, size_t __n)
{
    int i;
    unsigned char *dst = (unsigned char *) __to;
    unsigned char *src = (unsigned char *) __from;

    for(i=0;i<__n;i++)
        dst[i] = src[i];

    return dst;
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
#define REG_WRITE_32(x, val) do {               \
    *(volatile unsigned int*)(x) = val;         \
} while(0)
int bmain()
{
    int rc = 0;
#if !defined(CONFIG_BOOT1_MINI_SDHC)
    int retry_count = 0;
#endif
#if defined(CONFIG_FPGA)
    int ret = 0;
#endif
    char *src;
    unsigned int boot_type;
    void (*jump) (void) = (void (*)(void)) B2_TEXT;

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

    // initialize DDR memory
#if defined(CONFIG_FPGA)
    ddr3_calibration_c();
    ret = ReadWriteCheck_Ok();
    putchar('0' + ret);
#else

#define GPIO_SEL2_REG  0xBF004A20UL
#define CLK_ENABLE_REG 0xBF004A58UL
    REG_UPDATE_32(GPIO_SEL2_REG, 0x01100000UL, 0x0FF00000UL); // UART pinmux
    REG_UPDATE_32(CLK_ENABLE_REG, 0xFFBFFFFFUL, 0xFFBFFFFFUL); // disable all gated-clock except EJTAG

#if defined(CONFIG_USE_DDR3)
    ddr3_init();
#else
    ddr2_init();
#endif
#endif

#ifdef CONFIG_ATE
    while(__REG_READ32(0x4AFC) & 0x8)
        ;
#endif

#if !defined(CONFIG_FPGA)
#if defined(DEBRICK)
    *(volatile unsigned long *)0xbf005520UL = 0;  // cleanup REG GDMA_DESCR_BADDR
    putchar('*');
    while(1)
    {
            unsigned long _pc;
            void (*func) (void);

#if defined(CONFIG_FPGA)
            //putchar('*');
#endif

            _pc = *(volatile unsigned long *)0xbf005520UL;  //use REG GDMA_DESCR_BADDR for PC

            if(_pc)
            {
                  *(volatile unsigned long *)0xbf005520UL = 0;  // cleanup REG GDMA_DESCR_BADDR
                  *(volatile unsigned long *)0xbf005524UL = 0;  // cleanup boot type
                  func = (void *) _pc;
                  sf_flush_cache_all();
                  func();
                  asm volatile ("nop;nop;nop;");
            }
    }
#endif

    if(chip_revision>=2)
    {
        // Panther A2 and above get boot_type from GDMA DEBUG REG1
        otp_read_config();
        boot_type = otp_get_boot_type();
    }
    else
    {
        // following code are move from IPL to here, work-aound for Panther A0

    #define PMU_SOFTWARE_GPREG  0x48FC
        unsigned long pmu_software_gpreg;

        pmu_software_gpreg = __REG_READ32(PMU_SOFTWARE_GPREG);

        if (pmu_software_gpreg & 0x01)
        {
            /* allow usage of pmu_software_gpreg to over-write otp_config */
            otp_config = (pmu_software_gpreg >> 8) & 0x00ffffffUL;
        }
        else
        {
            otp_read_config();
        }

        boot_type = otp_get_boot_type();
        if (pmu_software_gpreg & 0x02)
        {
            /* allow usage of pmu_software_gpreg to over-write boot_type */
            boot_type = (pmu_software_gpreg >> 4) & 0x000fUL;
        }

        *((volatile unsigned long *)0xbf005520UL) = (0x80000000 | get_otp_config());
        *((volatile unsigned long *)0xbf005524UL) = boot_type;
    }

#else

    // FPGA part
    // get boot_type from GDMA DEBUG REG1
    boot_type = otp_get_boot_type();
    otp_read_config();
    
#endif

#if !defined(CONFIG_BOOT1_MINI_SDHC)
Retry:
#endif

#if defined(CONFIG_BOOT1_MINI_SDHC)
    if (boot_type == BOOT_FROM_SD)
    {
        load_boot2_from_sd();
        goto load_finish;
    }
#endif

#if defined(CONFIG_BOOT1_UART)
    if (boot_type == BOOT_FROM_UART)
    {
        load_boot2_from_uart();
        goto load_finish;
    }
#endif

#if !defined(CONFIG_BOOT1_MINI_SDHC)

#ifdef CONFIG_PDMA
    switch_pdma_interrupt(PDMA_POLLING);
#endif

    flash_init(boot_type);

    if(otp_parse_config(OTP_ENABLE_SECURE_SHIFT))
    {
        flush_cache();
        load_decrypted_erased_page();
    }

    // need to flush cache for savety read
    flush_cache();

    load_boot2();
#endif

#if defined(CONFIG_BOOT1_MINI_SDHC)||defined(CONFIG_BOOT1_UART)
load_finish:
#endif
    putchar('X');
    // read compressed boot2 with LZMA from flash
    src = (char *) (BOOT2_BASE);
    rc = unlzo((u8 *) src, BOOT2_SIZE, 0, 0,(u8 *) B2_TEXT, 0, 0);
    while (rc != 0)
    {
        // should not enter this while statement...
        putchar('G');
        //putchar('G');

#if defined(CONFIG_BOOT1_MINI_SDHC)
        if (boot_type == BOOT_FROM_SD)
            load_boot2_from_sd();
#else
#ifndef SERIAL_FLASH_TEST
        if(boot_type == BOOT_FROM_NAND_WITH_OTP)
        {
            if((otp_config & 0x0F00)==0x0F00)
                otp_config &= 0xFFFFF0FFUL;
            else
                otp_config += 0x0100;
        }

#if defined(CONFIG_BOOT1_UART)
        if(retry_count>100)
            boot_type = BOOT_FROM_UART;
#endif
        retry_count++;

        goto Retry;
#endif
#endif
    }
    putchar('S');

    flush_cache();

    //putchar('U');
    jump();

    while (1)
    {
        putchar('Z');
    }

    return rc;
}
