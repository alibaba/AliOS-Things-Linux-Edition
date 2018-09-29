/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file chip.h
*   \brief Registers definition
*   \author Montage
*/

#ifndef CHIP_H
#define CHIP_H

/*==============================================================+
| define this to avoid using some unnecessary code,             |
| maybe these code will use in the future.                      |
+==============================================================*/
//      #define CONFIG_TODO

/* interrupt number */
#define IRQ_PDMA    0
#define IRQ_TMR2    5
#define IRQ_TMR1    6
#define IRQ_TMR0    7
#define IRQ_GDMA   13
#define IRQ_WIFI   14
#define IRQ_UDC    16
#define IRQ_SDHC   18
#define IRQ_MAC    20
#define IRQ_PCM    21
#define IRQ_TSI    27

#define IRQ_GPIOX   5

/* register base */
#define MI_BASE         0xbf000000
#define DDR_BASE        (MI_BASE+0x0000)
#define TM_BASE         (MI_BASE+0x1000)
#define SPI_BASE        (MI_BASE+0x2000)
#define GSPI_BASE       (MI_BASE+0x2E00)
#define SMI_BASE        (MI_BASE+0x2200)
#define I2C_BASE        (MI_BASE+0x2400)
#define UR_BASE         (MI_BASE+0x2900)
#define PDMA_BASE       (MI_BASE+0x2C00)
#define UMAC_REG_BASE   (MI_BASE+0x3000)
#define BASEBAND_REG_BASE (MI_BASE + 0x04000)
#define PMU_BASE        (MI_BASE+0x4800)
#define IC_BASE         (MI_BASE+0x4900)
#define GPIO_BASE       (MI_BASE+0x4A00)
#define ANA_BASE        (MI_BASE+0x4C00)
#define GDMA_BASE       (MI_BASE+0x5500)
#define AI_BASE         (MI_BASE+0x7000)
#define SW_BASE         (MI_BASE+0x8000)
#define MAC_BASE        (MI_BASE+0x8800)
#define TSI_BASE        (MI_BASE+0x9800)
#define SDIO_BASE       (MI_BASE+0xB000)
#define USB_BASE        (MI_BASE+0xC000)
#define USB_OTG_BASE    (MI_BASE+0xD000)

#define PLL_BASE	    (0x000C0A00)
#define RF_BASE         (MI_BASE+0x4F00)
#define BBP_REG_BASE    (MI_BASE+0x4000)

/* macro for accessing related register */
#define REG(base,offset) (*(volatile unsigned int*)(base+(offset)))
#define MIREG(offset)    REG(MI_BASE,offset)
#define DDRREG(offset)   REG(DDR_BASE,offset)
#define TMREG(offset)    REG(TM_BASE,offset)
#define SPIREG(offset)   REG(SPI_BASE,offset)
#define GSPIREG(offset)  REG(GSPI_BASE,offset)
#define SMIREG(offset)   REG(SMI_BASE,offset)
#define PMUREG(offset)   REG(PMU_BASE,offset)
#define ANAREG(offset)   REG(ANA_BASE,offset)
#define I2CREG(offset)   REG(I2C_BASE,offset)
#define URREG(offset)    REG(UR_BASE+0x100*CONFIG_UART_PORT,offset)
#define PDMAREG(offset)  REG(PDMA_BASE,offset)
#define ICREG(offset)    REG(IC_BASE,offset)
#if defined(CONFIG_FPGA)
#define GPREG(offset)    __check_gpio_reg_access__()    //REG(GPIO_BASE,offset)
#else
#define GPREG(offset)    REG(GPIO_BASE,offset)
#endif
//#define ANAREG(offset)   __check_gpio_reg_access__()    //REG(ANA_BASE,offset)
#define AIREG(offset)    REG(AI_BASE,offset)
#define SWREG(offset)    REG(SW_BASE,offset)
#define TSIREG(offset)   REG(TSI_BASE,offset)
#define USBREG(offset)   REG(USB_BASE,offset)
#define PLLREG(offset)   REG(PLL_BASE,offset)
#define RFREG(offset)    REG(RF_BASE,offset)
#define TMREG(offset)    REG(TM_BASE,offset)
#define BBREG(x)         REG(BBP_REG_BASE,offset)

// CAUTION: __REG_READ32/__REG_WRITE32/__REG_UPDATE32 are MI_BASE (0xBF000000) based access
#define __REG_READ32(offset)         REG(MI_BASE, offset)
#define __REG_WRITE32(offset, val)   REG(MI_BASE, offset) = (unsigned int)(val)
#define __REG_UPDATE32(x, val, mask) do {                   \
    unsigned int newval;                                  \
    newval = *(volatile unsigned int*) (MI_BASE + (x));   \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) )); \
    *(volatile unsigned int*)(MI_BASE + (x)) = newval;    \
} while(0)

// for base-band
#define BBREG_READ32(x) (*(volatile u32*)(BBP_REG_BASE+(x)))
#define BBREG_WRITE32(x, val) (*(volatile u32*)(BBP_REG_BASE+(x)) = (u32)(val))
#define BBREG_UPDATE32(x,val,mask) do {           \
    unsigned long newval = BBREG(x);     \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));\
    BBREG(x) = newval;      \
} while(0)

/* OTP configuration */
#define OTP_CONFIG_BASE 32   // bytes
#define OTP_CONFIG_LEN 4     // bytes
#define OTP_ENABLE_SECURE_SHIFT        0
#define OTP_DISABLE_KEY_WRITE_SHIFT    1
#define OTP_KEY_ENCRYPT_SHIFT          2
#define OTP_ENABLE_FLASH_SHIFT         3
#define OTP_FLASH_BOOT_SELECT_SHIFT    4
#define OTP_UART_SD_BOOT_SEL_SHIFT     5
#define OTP_WATCHDOG_SHIFT             6
#define OTP_JTAG_SHIFT                 7
#define OTP_READ_CMD_DUMMY_SHIFT       8
#define OTP_READ_DATA_DUMMY_SHIFT      9
#define OTP_PAGE_SIZE_SHIFT           10
#define OTP_CHECK_BAD_BLOCK_SHIFT     12
#define OTP_UART_SD_BOOT_DISABLE_SHIFT 13
#define OTP_SD_CLK_INV_SHIFT          14
#define OTP_BOOT1_IMAGE_CHECK_SHIFT   15
#define OTP_DISABLE_BAD_BLOCK_CHECK_SHIFT  16
#define OTP_SD_CLK_DIV_SHIFT               17
#define OTP_FLASH_CLK_DIV_SHIFT            20
#define OTP_ONE_SECOND_WATCHDOG_SHIFT      23
#define OTP_ANYBOOT_ENABLE_SHIFT           24
#define OTP_ANYBOOT_DISABLE_SHIFT          25
#define OTP_BOOT1_CKSUM_SHIFT              26
#define OTP_NOR_RESET_SHIFT                27
#define OTP_NOR_4ADDR_SHIFT                28
#define OTP_UART_BOOT_DETECT_SHIFT         29
#define OTP_BOOT_FAILURE_DETECT_SHIFT      30

#define BOOT_FROM_UART   0
#define BOOT_FROM_NOR    1
#define BOOT_FROM_NAND   2
#define BOOT_FROM_NAND_WITH_OTP 3
#define BOOT_FROM_SD     4

/* ROM bank register */
#define MR1C           0x00
#define MR2C           0x04
#define SROMEN         0x08
#define  CS2_PFLASH       (1<<2)
#define  CS1_PFLASH       (1<<1)
#define  CS0_PFLASH       (1<<0)

/* DDR2/DDR3 SDRAM bank register */
#define DDR_PRECHG      0x000
#define DDR_CTRLMRS     0x004
#define  DDR_KCKMRS       (1<<0)        /* send Mode Register Set command */
#define DDR_AUTOREF     0x008
#define DDR_SLFREF      0x00c
#define  DDR_KCKZQCL      (1<<3)        /* send ZQ Calibration Long command */
#define DDR_CFGZQC      0x010
#define DDR_SWCMDEN     0x014
#define DDR_INITCTRL    0x018   /* initialization control */
#define  DDR_W500US       (1<<16)       /* wait 500us */
#define  DDR_PWRUP        (1<<0)        /* power up */
#define DDR_LBL32       0x01c
#define DDR_CTRLMODE    0x020
#define  DDR_BASHFT       14    /* shift for BA pin field */
#define DDR_LBL32       0x01c
#define DDR_PARA0       0x024
#define DDR_PARA1       0x028
#define  DDR_WTRSHFT       8    /* shift for internal wirte to read delay field */
#define DDR_PARA2       0x02c
#define DDR_DRAMTYPE    0x034
#define  DDR_LRGCL        (1<<8)
#define  DDR_DDR3         (1<<0)        /* 1: for ddr3, 0: for ddr2 */
#define DDR_CTRL0       0x038
#define  DDR_DQSSHFT      28    /* shift for DQS delay field */
#define  DDR_ARLSHFT      16    /* shift for additive read latency to CL field */
#define DDR_CTRL1       0x03c
#define DDR_PHYRD00     0x040
#define DDR_PHYRD32     0x044
#define DDR_PHYRD64     0x048
#define DDR_PHYRD96     0x04c
#define DDR_PLBCHCK     0x050   /* phy loopback check */
#define DDR_PLBMODE     0x054   /* phy loopback mode */
#define DDR_WR_RGN      0x05c
#define  DDR_TWRSHFT      16    /* shift for the time of Write Recovery field */
#define DDR_DCLKDLY     0xc00
#define DDR_RDCLKDLY    0xc04
#define DDR_DCLK        0xc08
#define DDR_RDCLK       0xc0c

/* timer bank register */
#define T0CR           0x00     /* current value */
#define T0LR           0x04     /* reload value */
#define T0PR           0x08     /* prescaler */
#define T0CN           0x0c     /* control */
#define T0IS           0x10     /* interrupt status */
#define T1CR           0x14     /* current value */
#define T1LR           0x18     /* reload value */
#define T1PR           0x1c     /* prescaler */
#define T1CN           0x20     /* control */
#define T1IS           0x24     /* interrupt status */
#define T2CR           0x28     /* current value */
#define T2LR           0x2c     /* reload value */
#define T2PR           0x30     /* prescaler */
#define T2CN           0x34     /* control */
#define T2IS           0x38     /* interrupt status */

/* SPI bank register */
#define DFIFO          0x000
#define CH0_BAUD       0x100
#define  SPI_DLYSHFT      28    /* shift for delay field */
#define CH0_MODE_CFG   0x104
#define  SPI_DUMYH        (1<<31)       /* high impedance for dummy */
#define  SPI_CSRTRN       (1<<30)       /* cs retrun to default automatically */
#define  SPI_BYTEPKG      (1<<29)       /* pack data unit at byte */
#define  SPI_WTHSHFT      24    /* shift for data width field */
#define  SPI_CLDSHFT      18    /* shift for CLK delay field */
#define  SPI_CSDSHFT      16    /* shift for CS delay field */
#define  SPI_DBYTE        (1<<29)       /* data unit: byte */
#define  SPI_LSB          (1<<12)       /* least significant bit first */
#define  SPI_DPACK        (1<<10)       /* enable data package */
#define  SPI_HBYTE        (1<<9)        /* high byte first */
#define  SPI_FSHMODE      (1<<8)        /* change to flash mode */
#define CH1_BAUD       0x108
#define CH1_MODE_CFG   0x10C
#define CH2_BAUD       0x110
#define CH2_MODE_CFG   0x114
#define CH3_BAUD       0x118
#define CH3_MODE_CFG   0x11C
#define SPI_TC         0x120
#define SPI_CTRL       0x124
#define  SPI_C1NSHFT      30    /* shift for cmd1 io num field */
#define  SPI_C1LSHFT      24    /* shift for cmd1 len field */
#define  SPI_C0NSHFT      22    /* shift for cmd0 io num field */
#define  SPI_C0LSHFT      16    /* shift for cmd0 len field */
#define  SPI_DNTSHFT      14    /* shift for data io num field */
#define  SPI_DULSHFT      3     /* shift for dummy len field */
#define  SPI_DIRSHFT      1     /* shift for direction field */
#define  SPI_TRIGGER      (1<<0)        /* kick */
#define FIFO_CFG       0x128
#define SMPL_MODE      0x12C
#define PIN_MODE       0x130
#define  SPI_IO3SHFT       6    /* shift for io3 direction field */
#define  SPI_IO2SHFT       4    /* shift for io2 direction field */
#define  SPI_IO1SHFT       2    /* shift for io1 direction field */
#define  SPI_IO0SHFT       0    /* shift for io0 direction field */
#define  SPI_PININ         0    /* in direction */
#define  SPI_PINOUT        1    /* out direction */
#define  SPI_PINDIN        2    /* in direction at duplex mode, in/out direction at half-duplex mode */
#define  SPI_PINDOUT       3    /* out direction at duplex mode, in/out direction at half-duplex mode */
#define PIN_CTRL       0x134
#define  SPI_3WMODE       (1<<16)       /* 3 wire mode */
#define  SPI_03SHFT        6    /* shift for MOSI3 setting field */
#define  SPI_02SHFT        4    /* shift for MOSI2 setting field */
#define  SPI_01SHFT        2    /* shift for MOSI1 setting field */
#define  SPI_00SHFT        0    /* shift for MOSI0 setting field */
#define  SPI_OUTLOW        1    /* output low level */
#define  SPI_OUTHGH        2    /* output high level */
#define CH_MUX         0x138
#define  SPI_IO1CS3       (1<<2)        /* use io1 as channel3's cs */
#define  SPI_IO2CS2       (1<<1)        /* use io2 as channel2's cs */
#define  SPI_IO3CS1       (1<<1)        /* use io3 as channel1's cs */
#define STA            0x140
#define  SPI_BUSY         (1<<16)       /* spi busy state */
#define  SPI_TCSHFT        8    /* shift for TXD FIFO count state field */
#define  SPI_RCSHFT        0    /* shift for RXD FIFO count state field */
#define INT_STA        0x144
#define CMD_FIFO       0x148

/* SMI bank register */
#define SMI_DATA          0x00
#define SMI_CLK_RST       0x04  /* [15:0]:read_data */

/* UART bank register */
#define URBR           0x00     /* tx/rx buffer register */
#define  URBR_DTSHFT      24
#define  URBR_RDY         (1<<23)       /* rx ready */
#define URCS           0x04     /* control/status */
#define  URCS_REN         (1<<31)       /* rx enable */
#define  URCS_BRSHFT      16
#define  URCS_TIE         (1<<15)       /* tx interrupt enable */
#define  URCS_RIE         (1<<14)       /* rx interrupt enable */
#define  URCS_PE          (1<<13)       /* parity bit enable */
#define  URCS_EVEN        (1<<12)       /* even/odd parity */
#define  URCS_SP2         (1<<11)       /* stop bit setting */
#define  URCS_LB          (1<<10)       /* internal loop */
#define  URCS_P           (1<<9)        /* Rx byte parity */
#define  URCS_PER         (1<<8)        /* parity error */
#define  URCS_FE          (1<<7)        /* framing error */
#define  URCS_RTHRSHFT    4
#define  URCS_TF          (1<<3)        /* tx full */
#define  URCS_TE          (1<<2)        /* tx empty */
#define  URCS_RR          (1<<1)        /* rx ready */
#define  URCS_TB          (1<<0)        /* tx busy */

/* PDMA register */
#define PDMA_ENABLE          0x00
#define PDMA_CTRL            0x04
#define PDMA_INTR_ENABLE     0x08
#define PDMA_INTR_STATUS     0x0c
#define PDMA_CH_PRIORITY     0x10
#define PDMA_CH0_DESCR_BADDR 0x14
#define PDMA_CH0_CURR_ADDR   0x18
#define PDMA_CH0_STATUS      0x1c
#define PDMA_CH_OFFSET       0xc

/* interrupt bank register */
#define ISTS           0x00
#define IPLR           0x04
#define IMSK           0x08

/* GPIO bank register */
#define GPVAL          0x58
#define GPSET          0x30
#define GPCLR          0x34
#define GPDIR          0x38
#define GPSEL          0x2C
#define IO_T           0x1c
#define DRIVER1        0x20
#define  DRV_SDIO         (0x0000000CUL)
#define DRIVER2        0x24
#define TMIICLK        0x3c
#define GP2VAL         0x40
#define GP2SET         0x44
#define GP2CLR         0x48
#define GP2DIR         0x4c
#define GP2SEL         0x50
#define CS3SEL         0x84
#define GDEBUG         0x90
#define  FUNC_MODE_EN     (1 << 31)
#define  EXT_SW_HNAT      (1 << 30)
#define  EXT_SW_WIFI      (1 << 29)
#define  EXT_BB           (1 << 28)
#define  DEBUG_ASIC       (1 << 4)
#define  DEBUG_FPGA       (0 << 4)
#define  DEBUG_WMAC       0
#define  DEBUG_HNAT       1
#define  DEBUG_BB         2
#define  DEBUG_USB        3
#define  DEBUG_MI         4
#define GPIO_SEL0      0x18
#define GPIO_SEL1      0x1C
#define GPIO_SEL2      0x20
#define GPIO_SEL3      0x24
#define GPIO_SEL4      0x28
#define SWRST          0xa0
#define  PAUSE_AI1        (1 << 19)
#define  SWRST_AI1        (1 << 18)
#define  PAUSE_AI0        (1 << 15)
#define  SWRST_AI0        (1 << 14)
#define  PAUSE_SDIO       (1 << 13)
#define  PAUSE_BB         (1 << 12)
#define  PAUSE_SW         (1 << 11)
#define  PAUSE_USB        (1 << 10)
#define  PAUSE_HNAT       (1 << 9)
#define  PAUSE_WIFI       (1 << 8)
#define  SWRST_SDIO       (1 << 5)
#define  SWRST_BB         (1 << 4)
#define  SWRST_SW         (1 << 3)
#define  SWRST_USB        (1 << 2)
#define  SWRST_HNAT       (1 << 1)
#define  SWRST_WMAC       (1 << 0)
#define SDIO           0xa4
#define  SDIO_ENDIAN_PIN  (4)
#define SMI_CTRL0      0xb4
#define SMI_CTRL1      0xb8
#define SRM_LBND       0xc8
#define SRM_UBND       0xcc
#define SVN_NUM        0xdc
#define USBMOD         0xf8
#define MADCSAMP       0x430
#define PWM            0x480
#define ARBITER        0x484

/* analog bank register */
#define DDR_REGU       0x00
#define  REG_CP_HFRQ      (1<<0)
#define CLKGEN         0x0C
#define CLKEN          0x1C
#define  RMII_CLK_EN      (1<<8)
#define  PCM_CLK_EN       (1<<7)
#define  I2S_CLK_EN       (1<<6)
#define  WIFI_CLK_EN      (1<<3)
#define  SD_CLK_EN        (1<<2)
#define CLKDIV         0x20
#define  CLKDIV_CPUFFST   16
#define  CLKDIV_CPUSHFT   8
#define  CLKDIV_POSTDIV_NUM 8
#define  CPU_CLK_UPDATE   (1<<31)
#define  CPU_CLK_NOGATE   (1<<30)
#define  CPU_CLK_PREDIV   (0x00FF0000UL)
#define  CPU_CLK_POSTDIV  (0x07000000UL)
#define  SYS_CLK_UPDATE  (1<<15)
#define  SYS_CLK_NOGATE  (1<<14)
#define  SYS_CLK_PREDIV  (0x000000FFUL)
#define  SYS_CLK_POSTDIV (0x00000700UL)
#define MADCCTL        0x28
#define CHIPID         0x34
#define  CHIP_ID_A0       (0x6000)
#define  CHIP_ID_A1       (0x6010)
#define CLKDIV2        0x3C
#define  I2S_CLK_UPDATE   (1<<31)
#define  I2S_CLK_NOGATE   (1<<30)
#define  I2S_CLK_PREDIV   (0x000FFF00UL)
#define  I2S_CLK_POSTDIV  (0x0000007FUL)
#define CLKDIV3        0x40
#define  PCM_CLK_UPDATE   (1<<31)
#define  PCM_CLK_NOGATE   (1<<30)
#define  PCM_CLK_PREDIV   (0x0000FFF0UL)
#define  PCM_CLK_POSTDIV  (0x00000007UL)
#define RMIIADDR       0x50
#define VOLCTRL        0x60
#define DDR_V1P2_CTR_L 0x78
#define DDR_CTR_L      0x7c
#define DDR_V1P2_CTR_R 0x88
#define DDR_CTR_R      0x8c

/* TSI register */
#define TSI_CTRL       0x00
#define  TSI_SPKTNUM_OFT 20
#define  TSI_BRSTSZE_OFT 12
#define  TSI_BNDR_STOP   (1<<10)
#define  TSI_STALL       (1<<9)
#define  TSI_BENDIAN     (1<<8)
#define  TSI_M_DIRECT    (1<<7)
#define  TSI_M_DMA       (1<<6)
#define  TSI_CLR_MODULE  (1<<1)
#define  TSI_EN          (1<<0)
#define TSI_CSIZE      0x04
#define  TSI_FLTR_BMAP   0x1f
#define TSI_BUF0_ADDR  0x10
#define TSI_BUF0_SIZE  0x14
#define TSI_BUF1_ADDR  0x18
#define TSI_BUF1_SIZE  0x1c
#define TSI_FLTR_BASE  0x20
#define  TSI_FLTR_OFT    16
#define TSI_INTR_EN    0x40
#define TSI_INTR_CLR   0x44
#define TSI_INTR_STAT  0x48
#define  TSI_OFLW_DROP   (1<<24)
#define  TSI_SYN_DROP    (1<<23)
#define  TSI_DST_CHG     (1<<21)
#define  TSI_SPKT_DONE   (1<<20)
#define  TSI_REV_PKT     (1<<19)
#define  TSI_SIZEERR     (1<<18)
#define  TSI_SYNCERR     (1<<17)
#define  TSI_OFLW        (1<<16)
#define  TSI_OFLW_DROP_I (1<<8)
#define  TSI_SYN_DROP_I  (1<<7)
#define  TSI_DST_CHG_I   (1<<5)
#define  TSI_SPKT_DONE_I (1<<4)
#define  TSI_REV_PKT_I   (1<<3)
#define  TSI_SIZERRR_I   (1<<2)
#define  TSI_SYNCERR_I   (1<<1)
#define  TSI_OFLW_I      (1<<0)
#define TSI_ADC_INFO   0x4c
#define  TSI_FIFO_CNT    16
#define  TSI_FIFO_FULL   (1<<5)
#define  TSI_CURR_DST    (1<<4)
#define  TSI_AXIS_IDLE   (1<<3)
#define  TSI_AXIM_IDLE   (1<<2)
#define  TSI_CORE_IDLE   (1<<1)
#define  TSI_FIFO_IDLE   (1<<0)
#define TSI_CUR_ADDR   0x50
#define TSI_AXI_INFO   0x54
#define  TSI_EVENT_CNT   16
#define  TSI_AXIS_RDNRDY (1<<7)
#define  TSI_AXIS_RCNRDY (1<<6)
#define  TSI_AXIM_WDST   (1<<5)
#define  TSI_AXIM_RDNRDY (1<<4)
#define  TSI_AXIM_WDNRDY (1<<3)
#define  TSI_AXIM_WRNRDY (1<<2)
#define  TSI_AXIM_RCNRDY (1<<1)
#define  TSI_AXIM_WCNRDY (1<<0)

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
    #define TSI_AXI_MASTER_IDLE        0x00000004
#define TSI_ADC_DUMP_DST_CURR_ADDR  (TSI_BASE + 0x50)

/* USB bank register */
#define OTGSC          0x0a4
#define  AVV              (1<<9)
#define  AVVIS            (1<<17)
#define  AVVIE            (1<<25)
#define PHY_DIG_CTRL   0x308
#define  VBUS_DETECTION   (1<<12)
#define  RECOVERY_CLK_INV (1<<4)
#define USB_REG0       0x320
#define  SQ_DELAY         (1<<31)
#define USB_PHY_PLL    0x328
#define USB_A0_REG     0x32c
#define  USB_DOWN         (1<<0)

/* PMU bank register */
#define PMUREG_READ32(x)            (*(volatile u32*)(PMU_BASE+(x)))
#define PMUREG_WRITE32(x, val)      (*(volatile u32*)(PMU_BASE+(x)) = (u32)(val))
#define PMUREG_UPDATE32(x,val,mask) do {              \
    u32 newval = PMUREG_READ32(x);                    \
    newval = ((newval & ~(mask)) | ((val) & (mask))); \
    PMUREG_WRITE32(x, newval);                        \
} while(0)

// GPIO registers in PMU
#define GPIO_FUNC_EN      (0x2C)
#define GPIO_ODS          (0x30)
#define GPIO_ODC          (0x34)
#define GPIO_OEN          (0x38)
#define GPIO_OUTPUT_DATA  (0x3C)
#define GPIO_INT_RAISING  (0x40)
#define GPIO_INT_FALLING  (0x44)
#define GPIO_INT_HIGH     (0x48)
#define GPIO_INT_LOW      (0x4C)
#define GPIO_INT_MASK     (0x50)
#define GPIO_INT_STATUS   (0x54)
#define GPIO_ID           (0x58)

#endif
