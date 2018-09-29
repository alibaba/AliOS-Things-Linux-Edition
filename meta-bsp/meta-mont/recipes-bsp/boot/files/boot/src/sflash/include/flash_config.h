/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file flash_config.h
*   \brief Configuration file
*   \author Montage
*/

#ifndef __FLASH_CONFIG_H__
#define __FLASH_CONFIG_H__

/*=============================================================================+
| Main Functions                                                               |
+=============================================================================*/
//	#define BOOT_MODE_IPL
#ifdef BOOT_MODE_IPL
//	    #define IPL_DEBUG_ENABLE
#endif

#if defined(BOOT_MODE_BOOT2)
    #define CHECK_AFTER_WRITE
#if defined(VERBOSE_DEBUG)
    #define BOOT_DEBUG_ENABLE
#endif
#endif

#define CONFIG_PDMA
#ifdef CONFIG_PDMA
    #define PDMA_SWITCH 1
    #define PDMA_POLLING 0x0
    #define PDMA_INTERRUPT 0x1
#endif

#if defined(BOOT_MODE_BOOT2)
#define CONFIG_WRITE_IO_MODE 4   // 1 or 4
#define CONFIG_READ_IO_MODE  4   // 1, 2 or 4 bit
#else
#define CONFIG_WRITE_IO_MODE 1   // always set 1 here
#define CONFIG_READ_IO_MODE  1   // always set 1 here
#endif

/*=============================================================================+
| Miscellaneous Functions                                                      |
+=============================================================================*/
        //  #define CONFIG_FLASH_SW_LOCK
#define CONFIG_SFLASH
#define CONFIG_FLASH_BANK_NO 1
// only NAND flash need to do bad block management
#define BAD_BLOCK_MANAGE
// NAND flash
//	#define DECLINE_WRITE_OOB

/*=============================================================================+
| About flash command                                                          |
+=============================================================================*/
#define SF_CMD_READ_STATUS      0x05    /** read status register */
#define SF_CMD_READ_ID          0x9F    /** read id */
#define SF_CMD_READ             0x03    /** read */
#define SF_CMD_4READ            0x13    /** read with 4-bytes address */
#define SF_CMD_FAST_READ        0x0B    /** fast read */
#define SF_CMD_DUAL_FAST_READ   0x3B    /** dual output fast read */
#define SF_CMD_DUAL_4FAST_READ  0x3C    /** dual output fast read with 4-bytes address */
#define SF_CMD_QUAD_FAST_READ   0x6B    /** quad output fast read */
#define SF_CMD_QUAD_4FAST_READ  0x6C    /** quad output fast read with 4-bytes address */
#define SF_CMD_WRITE_ENABLE     0x06    /** enable write */
#define SF_CMD_WRITE_DISABLE    0x04    /** disable write */
#define SF_CMD_SEC_ERASE        0xD8    /** erase sector */
#define SF_CMD_SEC_4ERASE       0xDC    /** erase sector with 4-bytes address */
#define SF_CMD_SEC2_ERASE       0x52    /** erase 32K sector */
#define SF_CMD_BULK_ERASE       0xC7    /** erase chip */
#define SF_CMD_BULK2_ERASE      0x60
#define SF_CMD_SSEC_ERASE       0x20    /** erase 4K sector */
#define SF_CMD_WRITE            0x02    /** page program */
#define SF_CMD_4WRITE           0x12    /** page program with 4-bytes address */
#define SF_CMD_QUAD_WRITE       0x32    /** Quad Page Program */
#define SF_CMD_QUAD_4WRITE      0x34    /** Quad Page Program with 4-bytes address */

// NAND_FLASH
#define SF_CMD_EXECUTE          0x10    /** page Execute */
#define SF_CMD_READ2CACHE       0x13    /** page Read to Cache */
#define SF_CMD_GET_FEATURE      0x0F    /** get Feature */
#define SF_CMD_SET_FEATURE      0x1F    /** set Feature */
#define SF_CMD_RESET            0xFF    /** reset */
#define SF_PROTECT_REG          0xA0    /** protection register */
#define SF_FEATURE_REG          0xB0    /** feature register */
#define SF_STATUS_REG           0xC0    /** status register */
// NOR FLASH
#define SF_CMD_RST_EN           0x66    /** enable flash reset      */
#define SF_CMD_RST_MEM          0x99    /** reset flash memory      */
#define SF_CMD_WRITE_STA_REG1   0x01    /** write status register-1 */
#define SF_CMD_WRITE_STA_REG2   0x31    /** write status register-2 */
#define SF_CMD_WRITE_STA_REG3   0x11    /** write status register-3 */
#define SF_CMD_READ_STA_REG1    0x05    /**  read status register-1 */
#define SF_CMD_READ_STA_REG2    0x35    /**  read status register-2 */
#define SF_CMD_READ_STA_REG3    0x15    /**  read status register-3 */


/*=============================================================================+
| Constant                                                                     |
+=============================================================================*/
#define PAGES_PER_BLOCK 64

// because block size of NOR is 64k and out boot code is 128k now,
// so NOR and NAND have different block mapping configuration
#define BOOT_SETTING_INDEX     1     // bootvar block index (nand)
#define BOOT_SETTING_INDEX_NOR 2     // bootvar block index (nor)

#define FLASH_BLOCK_SIZE 0x40000 // for basic NAND flash, 64 pages, (FLASH_PAGE_SIZE * PAGES_PER_BLOCK * 2)
#define BASE_PAGE_SIZE   0x800    // for basic NAND flash
#define NOR_BLOCK_SIZE   0x10000
#define NOR_PAGE_SIZE    0x100      // for NOR flash

#define BOOT_BURN_BASE   0x81000000UL

#define BOOT_IMG_SIZE    0x20000
#define BOOT1_BURN_BUF   BOOT_BURN_BASE
#define BOOT1_BASE       0x90000000UL
#define BOOT1_PAGE_START 0x0000
#define BOOT1_SIZE   0x4000     // 8 or 12 pages, max 16k or 24k in SRAM,
                                // IPL can't use over this size, so please don't change
#define BOOT1_SIZE_24KBYTES 0x6000

#define ENCRYPTED_BLANK_PAGE_BASE 0x82ff0000UL
#define BLANK_PAGE_START_NAND 0x0C000
#define BLANK_PAGE_START_NOR  0x06000
#define BLANK_PAGE_ADDR   0x6000
#define BLANK_PAGE_SIZE   0x2000

#define BOOT2_BURN_BUF (BOOT1_BURN_BUF + BOOT1_SIZE)
#define BOOT2_BASE   0x83000000UL
#define BOOT2_PAGE_START_NAND 0x10000
#define BOOT2_PAGE_START_NOR 0x8000

#if defined(CONFIG_ATE)
#define BOOT2_SIZE   0x38000    // 224K bytes boot2 for ATE
#else
#define BOOT2_SIZE   0x18000    // maximum size(96Kbytes) = 128Kbytes - boot1(16Kbytes) - boot1a(16Kbytes)
#endif

#define BOOT_RECOVERY_SIZE 0x40000  // 256k

#define FLASH_BASE_CS0  0xb8000000
#define SPI_INT_DLY   3
#define SPI_TXD_DELAY (1 << 1)

// about Pin-strapping setting
#define PIN_STRAP_REG_ADDR  0xbf004828UL

/*! status bit */
#define SF_SR_BUSY  (1<<0)  /** status bit, b0 : busy */

#define SF_CMD_S    24  /** CR command field shift **/

#ifdef  CONFIG_SFI_CLK_DIV
    #define SPI_CLK_DIV   CONFIG_SFI_CLK_DIV
#else
    #define SPI_CLK_DIV   4
#endif

#ifdef CHECK_AFTER_WRITE
    #define CHECK_WRITE_SUCCESS 0
    #define CHECK_WRITE_FAIL 1
#endif

// about BAD_BLOCK_MANAGE
#define DETECT_GOOD_BLOCK  0
#define DETECT_BAD_BLOCK   1
#define DETECT_OUT_RANGE  -1
#define RET_TYPE_INDEX   0
#define RET_TYPE_OFFSET  1

#define NO_CHECK_BAD_BLOCK 0
#define CHECK_BAD_BLOCK    1
#define INCREASE_BAD_BLOCK 2

#define SF_RC_ERASE_TO  -2  /** erase timeout */
#define SF_RC_PROG_TO   -3  /** program timeout */
#define SF_RC_PROG_FAIL -4  /** program failed */
#define SF_RC_READ_TO   -5  /** read timeout */

#define SF_PROG_TO  100000  /** wait loop before program timeout, 1.5 ~5 ms */
#define SF_ERASE_TO 5000000 /** wait loop before sector erase timeout, 0.8~ 2 sec*/

#define SF_ERASE_SEC    0   /** erase section */
//#define SF_ERASE_BULK   1   /** erase chip */

#define DISABLE_ECC 0
#define ENABLE_ECC  1

// about secure boot
#define DISABLE_SECURE 0
#define ENABLE_SECURE_REG  1
#define ENABLE_SECURE_OTP  2
#define ACTION_WRITE 0
#define ACTION_READ  1

#define OTP_REG      0x00001810
    // The register should be set with 0x4B0 with PBUS clock 120MHz.
    #define OTP_CLK_PLUSE_WIDTH   (0x4B0 << 21)
    #define OTP_CLK_PLUSE_GEN     0x00100000
    #define OTP_READ_MODE_EN      0x00080000
    #define OTP_ADDR_VALID_BIT    0x000007ff
    #define OTP_DATA_VALID_BIT    0x000000ff

#define OTP_AVDD_REG 0x00001814
    #define OTP_AES_KEY_WRITE_PROTECT 0x00000010
    #define OTP_PROGRAM_MODE_EN       0x00000004
    #define OTP_CLK_PLUSE_WIDTH_BIT11 0x00000002
    #define OTP_AVDD_EN               0x00000001

enum linux_image_type
{
    CAPP_IMAGE_TYPE,
    UBOOT_IMAGE_TYPE,
    CI_IMAGE_TYPE,      // CI, combined image
    UNKNOWN_IMAGE_TYPE
};

enum cmdline_type
{
    DEFAULT_CMDLINE,
    UBI_CMDLINE,
	SD_CMDLINE,
};

#endif                          // FLASH_CONFIG_H
