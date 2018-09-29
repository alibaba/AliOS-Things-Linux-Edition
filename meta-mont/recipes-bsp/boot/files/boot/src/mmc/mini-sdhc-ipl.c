/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file mini-sdhc.c
*   \brief Mini SDHC Driver
*   \author Montage
*/
#ifdef CONFIG_MINI_SDHC

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#if defined(IPL)
#include <common/chip.h>
#include <panther/cache.h>
#else
#include <arch/chip.h>
#include <arch/cpu.h>
#include <lib.h>
#endif

//#include <byteorder.h>
#include <common.h>

#ifdef CONFIG_SD_RECOVERY
#include <image.h>
#endif
#if defined(MASS_PRODUCTION_TEST) && defined(CONFIG_SCHED)
#include <sched.h>
#endif

#define CONFIG_SYS_SDIO_MAX_CLK 25000000
#define CONFIG_SYS_SDIO_MIN_CLK 400000

//#define SDHC_DELAY()    {int _www; for (_www=0; _www< 0x200; _www++); }

//#define HS_BOOT
//#define REAL_MINI_BOOT
#if !defined(BOOT_MODE_BOOT1)
//#define CONFIG_MMC_TRACE
#endif

#if defined(MASS_PRODUCTION_TEST)
struct sd_dev {
    volatile u32 init;
    volatile u32 run;
    u32 cnt;        // total test counter
    u32 pcnt;       // pass test counter
    u32 rcnt;       // read error counter
    u32 wcnt;       // write error counter
    u32 ecnt;       // compare error counter
};

struct sd_dev sd_devx;
#endif

#if !defined(CONFIG_MMC_TRACE)
#define printf(A, ...)
#endif

volatile unsigned long temp_variable;
static inline void sd_memcpy(void *dst, void *src, int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
        d[i] = s[i];
}

#ifdef REAL_MINI_BOOT
/*
 * REAL_MINI_BOOT means: no printf and clicmd
 *    mini-sdhc.o needs src/start.S and src/clock.o only
 * modify Makefile and start.S for compile correctly
 *    B2_OBJS = src/start.o src/clock.o src/mmc/mini-sdhc.o
 */
#define printf(A, ...)

void mini_sdhc_init(void);
#ifdef CONFIG_SD_RECOVERY
void sdrc(void);
#endif
void cmain()
{
    clock_init();
    mini_sdhc_init();
}

void memcpy(void *dst, void *src, int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
        d[i] = s[i];
}

int memcmp(void *dst, void *src, int len)
{
    register int i;
    register char *s = (char *) src;
    register char *d = (char *) dst;

    for (i = 0; i < len; i++)
    {
        if (*d++ != *s++)
            break;
    }
    return (len - i);
}
#endif

//#include <mt_types.h>           //for using u8, etc..
#define NULL               ((void *) 0x0)
#define __be32_to_cpu(x)    (x)

#define SD_VERSION_SD	0x20000
#define SD_VERSION_3	(SD_VERSION_SD | 0x300)
#define SD_VERSION_2	(SD_VERSION_SD | 0x200)
#define SD_VERSION_1_0	(SD_VERSION_SD | 0x100)
#define SD_VERSION_1_10	(SD_VERSION_SD | 0x10a)
#define MMC_VERSION_MMC		0x10000
#define MMC_VERSION_UNKNOWN	(MMC_VERSION_MMC)
#define MMC_VERSION_1_2		(MMC_VERSION_MMC | 0x102)
#define MMC_VERSION_1_4		(MMC_VERSION_MMC | 0x104)
#define MMC_VERSION_2_2		(MMC_VERSION_MMC | 0x202)
#define MMC_VERSION_3		(MMC_VERSION_MMC | 0x300)
#define MMC_VERSION_4		(MMC_VERSION_MMC | 0x400)
#define MMC_VERSION_4_1		(MMC_VERSION_MMC | 0x401)
#define MMC_VERSION_4_2		(MMC_VERSION_MMC | 0x402)
#define MMC_VERSION_4_3		(MMC_VERSION_MMC | 0x403)
#define MMC_VERSION_4_41	(MMC_VERSION_MMC | 0x429)
#define MMC_VERSION_4_5		(MMC_VERSION_MMC | 0x405)

#define MMC_MODE_HS		0x001
#define MMC_MODE_HS_52MHz	0x010
#define MMC_MODE_4BIT		0x100
#define MMC_MODE_8BIT		0x200
#define MMC_MODE_SPI		0x400
#define MMC_MODE_HC		0x800

#define MMC_MODE_MASK_WIDTH_BITS (MMC_MODE_4BIT | MMC_MODE_8BIT)
#define MMC_MODE_WIDTH_BITS_SHIFT 8

#define SD_DATA_4BIT	0x00040000

#define IS_SD(x) (x->version & SD_VERSION_SD)

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2

#define NO_CARD_ERR		-16     /* No SD/MMC card inserted */
#define UNUSABLE_ERR		-17     /* Unusable Card */
#define COMM_ERR		-18     /* Communications Error */
#define TIMEOUT			-19
#define IN_PROGRESS		-20     /* operation is in progress */

#define MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SWITCH			6
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_EXT_CSD		8
#define MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_WRITE_SINGLE_BLOCK	24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START	35
#define MMC_CMD_ERASE_GROUP_END		36
#define MMC_CMD_ERASE			38
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_SPI_READ_OCR		58
#define MMC_CMD_SPI_CRC_ON_OFF		59
#define MMC_CMD_RES_MAN			62

#define MMC_CMD62_ARG1			0xefac62ec
#define MMC_CMD62_ARG2			0xcbaea7

#define SD_CMD_SEND_RELATIVE_ADDR	3
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SEND_IF_COND		8

#define SD_CMD_APP_SET_BUS_WIDTH	6
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_APP_SEND_OP_COND		41
#define SD_CMD_APP_SEND_SCR		51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY	0x00020000
#define SD_HIGHSPEED_SUPPORTED	0x00020000

#define MMC_HS_TIMING		0x00000100
#define MMC_HS_52MHZ		0x2

#define OCR_BUSY		0x80000000
#define OCR_HCS			0x40000000
#define OCR_VOLTAGE_MASK	0x007FFF80
#define OCR_ACCESS_MODE		0x60000000

#define SECURE_ERASE		0x80000000

#define MMC_STATUS_MASK		(~0x0206BF7F)
#define MMC_STATUS_RDY_FOR_DATA (1 << 8)
#define MMC_STATUS_CURR_STATE	(0xf << 9)
#define MMC_STATUS_ERROR	(1 << 19)

#define MMC_STATE_PRG		(7 << 9)

#define MMC_VDD_165_195		0x00000080      /* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21		0x00000100      /* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22		0x00000200      /* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23		0x00000400      /* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24		0x00000800      /* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25		0x00001000      /* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26		0x00002000      /* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27		0x00004000      /* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28		0x00008000      /* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29		0x00010000      /* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30		0x00020000      /* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31		0x00040000      /* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32		0x00080000      /* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33		0x00100000      /* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34		0x00200000      /* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35		0x00400000      /* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36		0x00800000      /* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET		0x00    /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01    /* Set bits in EXT_CSD byte
                                                   addressed by index which are
                                                   1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02    /* Clear bits in EXT_CSD byte
                                                   addressed by index, which are
                                                   1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03    /* Set target byte to value */

#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

/*
 * EXT_CSD fields
 */
#define EXT_CSD_GP_SIZE_MULT		143     /* R/W */
#define EXT_CSD_PARTITIONS_ATTRIBUTE	156     /* R/W */
#define EXT_CSD_PARTITIONING_SUPPORT	160     /* RO */
#define EXT_CSD_RST_N_FUNCTION		162     /* R/W */
#define EXT_CSD_RPMB_MULT		168     /* RO */
#define EXT_CSD_ERASE_GROUP_DEF		175     /* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH		177
#define EXT_CSD_PART_CONF		179     /* R/W */
#define EXT_CSD_BUS_WIDTH		183     /* R/W */
#define EXT_CSD_HS_TIMING		185     /* R/W */
#define EXT_CSD_REV			192     /* RO */
#define EXT_CSD_CARD_TYPE		196     /* RO */
#define EXT_CSD_SEC_CNT			212     /* RO, 4 bytes */
#define EXT_CSD_HC_WP_GRP_SIZE		221     /* RO */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224     /* RO */
#define EXT_CSD_BOOT_MULT		226     /* RO */

/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

#define EXT_CSD_CARD_TYPE_26	(1 << 0)        /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52	(1 << 1)        /* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1	0       /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1       /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2       /* Card is in 8 bit mode */

#define EXT_CSD_BOOT_ACK_ENABLE			(1 << 6)
#define EXT_CSD_BOOT_PARTITION_ENABLE		(1 << 3)
#define EXT_CSD_PARTITION_ACCESS_ENABLE		(1 << 0)
#define EXT_CSD_PARTITION_ACCESS_DISABLE	(0 << 0)

#define EXT_CSD_BOOT_ACK(x)		(x << 6)
#define EXT_CSD_BOOT_PART_NUM(x)	(x << 3)
#define EXT_CSD_PARTITION_ACCESS(x)	(x << 0)

#define EXT_CSD_BOOT_BUS_WIDTH_MODE(x)	(x << 3)
#define EXT_CSD_BOOT_BUS_WIDTH_RESET(x)	(x << 2)
#define EXT_CSD_BOOT_BUS_WIDTH_WIDTH(x)	(x)

#define R1_ILLEGAL_COMMAND		(1 << 22)
#define R1_APP_CMD			(1 << 5)

#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136	(1 << 1)        /* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)        /* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)        /* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)        /* response contains opcode */

#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1b	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE| \
			MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

/* Maximum block size for MMC */
#define MMC_MAX_BLOCK_LEN	512

struct mmc_cid
{
    unsigned long psn;
    unsigned short oid;
    unsigned char mid;
    unsigned char prv;
    unsigned char mdt;
    char pnm[7];
};

struct mmc_cmd
{
    u32 cmdidx;
    u32 resp_type;
    u32 cmdarg;
    u32 response[4];
};

struct mmc_data
{
    union
    {
        char *dest;
        const char *src;        /* src buffers don't get written to */
    };
    u32 flags;
    u32 blocks;
    u32 blocksize;
};

/* forward decl. */
struct mmc;

struct mmc_ops
{
    int (*send_cmd) (struct mmc * mmc,
                     struct mmc_cmd * cmd, struct mmc_data * data);
    void (*set_ios) (struct mmc * mmc);
};

struct mmc_config
{
    //const char *name;
    //const struct mmc_ops *ops;
    u32 host_caps;
    u32 voltages;
#if 0
    u32 f_min;
    u32 f_max;
#endif
    u32 b_max;
};

/* TODO struct mmc should be in mmc_private but it's hard to fix right now */
struct mmc
{
    const struct mmc_config *cfg;       /* provided configuration */
    u32 version;
    void *priv;
    u32 has_init;
    int high_capacity;
    u32 bus_width;
    u32 clock;
    u32 card_caps;
    u32 ocr;
    u32 scr[2];
    //u32 csd[4];
    u32 cid[4];
    u32 rca;
    u32 tran_speed;
    u32 read_bl_len;
#if !defined(BOOT_MODE_BOOT1)
    u32 write_bl_len;
    u32 erase_grp_size;
#endif
    char op_cond_pending;       /* 1 if we are waiting on an op_cond command */
    char init_in_progress;      /* 1 if we have done mmc_start_init() */
    u32 op_cond_response;       /* the response byte from the last op_cond */
    //int (*block_read) (int dev_num, u32 start, u32 blkcnt, void *dst);
};

#ifdef CONFIG_SD_RECOVERY
typedef struct bootvar
{
    int mode;
    unsigned int ver;
    unsigned char mac0[8];
    unsigned char mac1[8];
    unsigned char mac2[8];
    int vlan;

    unsigned int ip;
    unsigned int msk;
    unsigned int gw;
    unsigned int server;
    unsigned int load_sz;
    unsigned int load_addr;
    unsigned int load_src;
    unsigned int load_src2;
    unsigned int log_src;
    unsigned int log_sz;
    unsigned int id;
    unsigned int hver;
    unsigned int pll;
    unsigned int serial;
    char file[32];
    //char cver[16];
    char rfc[128];
    char txvga[128];
    char rxvga[128];
    char pin[16];
    unsigned int freq_ofs;
    char madc_val0[64];
    char madc_val1[64];
    char lna[32];
} bootvar;
extern bootvar bootvars;
#endif

//struct mmc *mmc_create(const struct mmc_config *cfg, void *priv);
static void mmc_init(struct mmc *mmc);
//int mmc_read(struct mmc *mmc, u64 src, u8 * dst, int size);
static void mmc_set_clock(struct mmc *mmc, u32 clock);
static int mmc_start_init(struct mmc *mmc);

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

/*
 * Controller registers
 */

#define SDHCI_DMA_ADDRESS	0x00
#define SDHCI_ARGUMENT2		SDHCI_DMA_ADDRESS

#define SDHCI_BLOCK_SIZE	0x04
#define  SDHCI_MAKE_BLKSZ(dma, blksz) (((dma & 0x7) << 12) | (blksz & 0xFFF))

#define SDHCI_BLOCK_COUNT	0x06

#define SDHCI_ARGUMENT		0x08

#define SDHCI_TRANSFER_MODE	0x0C
#define  SDHCI_TRNS_DMA		0x01
#define  SDHCI_TRNS_BLK_CNT_EN	0x02
#define  SDHCI_TRNS_AUTO_CMD12	0x04
#define  SDHCI_TRNS_AUTO_CMD23	0x08
#define  SDHCI_TRNS_READ	0x10
#define  SDHCI_TRNS_MULTI	0x20

#define SDHCI_COMMAND		0x0E
#define  SDHCI_CMD_RESP_MASK	0x03
#define  SDHCI_CMD_CRC		0x08
#define  SDHCI_CMD_INDEX	0x10
#define  SDHCI_CMD_DATA		0x20
#define  SDHCI_CMD_ABORTCMD	0xC0

#define  SDHCI_CMD_RESP_NONE	0x00
#define  SDHCI_CMD_RESP_LONG	0x01
#define  SDHCI_CMD_RESP_SHORT	0x02
#define  SDHCI_CMD_RESP_SHORT_BUSY 0x03

#define SDHCI_MAKE_CMD(c, f) (((c & 0xff) << 8) | (f & 0xff))
#define SDHCI_GET_CMD(c) ((c>>8) & 0x3f)

#define SDHCI_RESPONSE		0x10

#define SDHCI_BUFFER		0x20

#define SDHCI_PRESENT_STATE	0x24
#define  SDHCI_CMD_INHIBIT	0x00000001
#define  SDHCI_DATA_INHIBIT	0x00000002
#define  SDHCI_DOING_WRITE	0x00000100
#define  SDHCI_DOING_READ	0x00000200
#define  SDHCI_SPACE_AVAILABLE	0x00000400
#define  SDHCI_DATA_AVAILABLE	0x00000800
#define  SDHCI_CARD_PRESENT	0x00010000
#define  SDHCI_CARD_STATE_STABLE    0x00020000
#define  SDHCI_CARD_DETECT_PIN_LEVEL    0x00040000
#define  SDHCI_WRITE_PROTECT	0x00080000
#define  SDHCI_DATA_LVL_MASK	0x00F00000
#define   SDHCI_DATA_LVL_SHIFT	20

#define SDHCI_HOST_CONTROL	0x28
#define  SDHCI_CTRL_LED		0x01
#define  SDHCI_CTRL_4BITBUS	0x02
#define  SDHCI_CTRL_HISPD	0x04
#define  SDHCI_CTRL_DMA_MASK	0x18
#define   SDHCI_CTRL_SDMA	0x00
#define   SDHCI_CTRL_ADMA1	0x08
#define   SDHCI_CTRL_ADMA32	0x10
#define   SDHCI_CTRL_ADMA64	0x18
#define   SDHCI_CTRL_8BITBUS	0x20
#define  SDHCI_CTRL_CD_TEST_INS 0x40
#define  SDHCI_CTRL_CD_TEST 0x80

#define SDHCI_POWER_CONTROL	0x29
#define  SDHCI_POWER_ON		0x01
#define  SDHCI_POWER_180	0x0A
#define  SDHCI_POWER_300	0x0C
#define  SDHCI_POWER_330	0x0E

#define SDHCI_BLOCK_GAP_CONTROL	0x2A

#define SDHCI_WAKE_UP_CONTROL	0x2B
#define  SDHCI_WAKE_ON_INT	0x01
#define  SDHCI_WAKE_ON_INSERT	0x02
#define  SDHCI_WAKE_ON_REMOVE	0x04

#define SDHCI_CLOCK_CONTROL	0x2C
#define  SDHCI_DIVIDER_SHIFT	8
#define  SDHCI_DIVIDER_HI_SHIFT	6
#define  SDHCI_DIV_MASK	0xFF
#define  SDHCI_DIV_MASK_LEN	8
#define  SDHCI_DIV_HI_MASK	0x300
#define  SDHCI_PROG_CLOCK_MODE	0x0020
#define  SDHCI_CLOCK_CARD_EN	0x0004
#define  SDHCI_CLOCK_INT_STABLE	0x0002
#define  SDHCI_CLOCK_INT_EN	0x0001

#define SDHCI_TIMEOUT_CONTROL	0x2E

#define SDHCI_SOFTWARE_RESET	0x2F
#define  SDHCI_RESET_ALL	0x01
#define  SDHCI_RESET_CMD	0x02
#define  SDHCI_RESET_DATA	0x04

#define SDHCI_INT_STATUS	0x30
#define SDHCI_INT_ENABLE	0x34
#define SDHCI_SIGNAL_ENABLE	0x38
#define  SDHCI_INT_RESPONSE	0x00000001
#define  SDHCI_INT_DATA_END	0x00000002
#define  SDHCI_INT_DMA_END	0x00000008
#define  SDHCI_INT_SPACE_AVAIL	0x00000010
#define  SDHCI_INT_DATA_AVAIL	0x00000020
#define  SDHCI_INT_CARD_INSERT	0x00000040
#define  SDHCI_INT_CARD_REMOVE	0x00000080
#define  SDHCI_INT_CARD_INT	0x00000100
#define  SDHCI_INT_ERROR	0x00008000
#define  SDHCI_INT_TIMEOUT	0x00010000
#define  SDHCI_INT_CRC		0x00020000
#define  SDHCI_INT_END_BIT	0x00040000
#define  SDHCI_INT_INDEX	0x00080000
#define  SDHCI_INT_DATA_TIMEOUT	0x00100000
#define  SDHCI_INT_DATA_CRC	0x00200000
#define  SDHCI_INT_DATA_END_BIT	0x00400000
#define  SDHCI_INT_BUS_POWER	0x00800000
#define  SDHCI_INT_ACMD12ERR	0x01000000
#define  SDHCI_INT_ADMA_ERROR	0x02000000

#define  SDHCI_INT_NORMAL_MASK	0x00007FFF
#define  SDHCI_INT_ERROR_MASK	0xFFFF8000

#define  SDHCI_INT_CMD_MASK	(SDHCI_INT_RESPONSE | SDHCI_INT_TIMEOUT | \
		SDHCI_INT_CRC | SDHCI_INT_END_BIT | SDHCI_INT_INDEX)
#define  SDHCI_INT_DATA_MASK	(SDHCI_INT_DATA_END | SDHCI_INT_DMA_END | \
		SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL | \
		SDHCI_INT_DATA_TIMEOUT | SDHCI_INT_DATA_CRC | \
		SDHCI_INT_DATA_END_BIT | SDHCI_INT_ADMA_ERROR)
#define SDHCI_INT_ALL_MASK	((unsigned int)-1)

#define SDHCI_ACMD12_ERR	0x3C

#define SDHCI_HOST_CONTROL2		0x3E
#define  SDHCI_CTRL_UHS_MASK		0x0007
#define   SDHCI_CTRL_UHS_SDR12		0x0000
#define   SDHCI_CTRL_UHS_SDR25		0x0001
#define   SDHCI_CTRL_UHS_SDR50		0x0002
#define   SDHCI_CTRL_UHS_SDR104		0x0003
#define   SDHCI_CTRL_UHS_DDR50		0x0004
#define   SDHCI_CTRL_HS_SDR200		0x0005  /* reserved value in SDIO spec */
#define  SDHCI_CTRL_VDD_180		0x0008
#define  SDHCI_CTRL_DRV_TYPE_MASK	0x0030
#define   SDHCI_CTRL_DRV_TYPE_B		0x0000
#define   SDHCI_CTRL_DRV_TYPE_A		0x0010
#define   SDHCI_CTRL_DRV_TYPE_C		0x0020
#define   SDHCI_CTRL_DRV_TYPE_D		0x0030
#define  SDHCI_CTRL_EXEC_TUNING		0x0040
#define  SDHCI_CTRL_TUNED_CLK		0x0080
#define  SDHCI_CTRL_PRESET_VAL_ENABLE	0x8000

#define SDHCI_CAPABILITIES	0x40
#define  SDHCI_TIMEOUT_CLK_MASK	0x0000003F
#define  SDHCI_TIMEOUT_CLK_SHIFT 0
#define  SDHCI_TIMEOUT_CLK_UNIT	0x00000080
#define  SDHCI_CLOCK_BASE_MASK	0x00003F00
#define  SDHCI_CLOCK_V3_BASE_MASK	0x0000FF00
#define  SDHCI_CLOCK_BASE_SHIFT	8
#define  SDHCI_MAX_BLOCK_MASK	0x00030000
#define  SDHCI_MAX_BLOCK_SHIFT  16
#define  SDHCI_CAN_DO_8BIT	0x00040000
#define  SDHCI_CAN_DO_ADMA2	0x00080000
#define  SDHCI_CAN_DO_ADMA1	0x00100000
#define  SDHCI_CAN_DO_HISPD	0x00200000
#define  SDHCI_CAN_DO_SDMA	0x00400000
#define  SDHCI_CAN_VDD_330	0x01000000
#define  SDHCI_CAN_VDD_300	0x02000000
#define  SDHCI_CAN_VDD_180	0x04000000
#define  SDHCI_CAN_64BIT	0x10000000

#define  SDHCI_SUPPORT_SDR50	0x00000001
#define  SDHCI_SUPPORT_SDR104	0x00000002
#define  SDHCI_SUPPORT_DDR50	0x00000004
#define  SDHCI_DRIVER_TYPE_A	0x00000010
#define  SDHCI_DRIVER_TYPE_C	0x00000020
#define  SDHCI_DRIVER_TYPE_D	0x00000040
#define  SDHCI_RETUNING_TIMER_COUNT_MASK	0x00000F00
#define  SDHCI_RETUNING_TIMER_COUNT_SHIFT	8
#define  SDHCI_USE_SDR50_TUNING			0x00002000
#define  SDHCI_RETUNING_MODE_MASK		0x0000C000
#define  SDHCI_RETUNING_MODE_SHIFT		14
#define  SDHCI_CLOCK_MUL_MASK	0x00FF0000
#define  SDHCI_CLOCK_MUL_SHIFT	16

#define SDHCI_CAPABILITIES_1	0x44

#define SDHCI_MAX_CURRENT		0x48
#define  SDHCI_MAX_CURRENT_LIMIT	0xFF
#define  SDHCI_MAX_CURRENT_330_MASK	0x0000FF
#define  SDHCI_MAX_CURRENT_330_SHIFT	0
#define  SDHCI_MAX_CURRENT_300_MASK	0x00FF00
#define  SDHCI_MAX_CURRENT_300_SHIFT	8
#define  SDHCI_MAX_CURRENT_180_MASK	0xFF0000
#define  SDHCI_MAX_CURRENT_180_SHIFT	16
#define   SDHCI_MAX_CURRENT_MULTIPLIER	4

/* 4C-4F reserved for more max current */

#define SDHCI_SET_ACMD12_ERROR	0x50
#define SDHCI_SET_INT_ERROR	0x52

#define SDHCI_ADMA_ERROR	0x54

/* 55-57 reserved */

#define SDHCI_ADMA_ADDRESS	0x58

/* 60-FB reserved */

#define SDHCI_SLOT_INT_STATUS	0xFC

#define SDHCI_HOST_VERSION	0xFE
#define  SDHCI_VENDOR_VER_MASK	0xFF00
#define  SDHCI_VENDOR_VER_SHIFT	8
#define  SDHCI_SPEC_VER_MASK	0x00FF
#define  SDHCI_SPEC_VER_SHIFT	0
#define   SDHCI_SPEC_100	0
#define   SDHCI_SPEC_200	1
#define   SDHCI_SPEC_300	2

#define SDHCI_GET_VERSION(x) (x->version & SDHCI_SPEC_VER_MASK)

/*
 * End of controller registers.
 */

#define SDHCI_MAX_DIV_SPEC_200	256
#define SDHCI_MAX_DIV_SPEC_300	2046

/*
 * quirks
 */
#define SDHCI_QUIRK_32BIT_DMA_ADDR	(1 << 0)
#define SDHCI_QUIRK_REG32_RW		(1 << 1)
#define SDHCI_QUIRK_BROKEN_R1B		(1 << 2)
#define SDHCI_QUIRK_NO_HISPD_BIT	(1 << 3)
#define SDHCI_QUIRK_BROKEN_VOLTAGE	(1 << 4)
#define SDHCI_QUIRK_NO_CD		(1 << 5)
#define SDHCI_QUIRK_WAIT_SEND_CMD	(1 << 6)
#define SDHCI_QUIRK_NO_SIMULT_VDD_AND_POWER (1 << 7)
#define SDHCI_QUIRK_USE_WIDE8		(1 << 8)

/* to make gcc happy */
struct sdhci_host;

/*
 * Host SDMA buffer boundary. Valid values from 4K to 512K in powers of 2.
 */
#define SDHCI_DEFAULT_BOUNDARY_SIZE	(512 * 1024)
#define SDHCI_DEFAULT_BOUNDARY_ARG	(7)
struct sdhci_ops
{
    void (*platform_reset_enter) (struct sdhci_host * host, u8 mask);
    void (*platform_reset_exit) (struct sdhci_host * host, u8 mask);
};

void *ioaddr = (void *) SDIO_BASE;
struct sdhci_host
{
    //char *name;
    unsigned int quirks;
    unsigned int host_caps;
    unsigned int version;
    unsigned int clock;
    struct mmc *mmc;
    //const struct sdhci_ops *ops;

    struct mmc_config cfg;
};

static inline void sdhci_writel(struct sdhci_host *host, u32 val, int reg)
{
    *(volatile u32 *) (ioaddr + (reg)) = val;
}

static inline void sdhci_writew(struct sdhci_host *host, u16 val, int reg)
{
    *(volatile u16 *) (ioaddr + (reg)) = val;
}

static inline void sdhci_writeb(struct sdhci_host *host, u8 val, int reg)
{
    *(volatile u8 *) (ioaddr + reg) = val;
}

static inline u32 sdhci_readl(struct sdhci_host *host, int reg)
{
    return *(const volatile u32 *) (ioaddr + (reg));
}

static inline u16 sdhci_readw(struct sdhci_host *host, int reg)
{
    return *(const volatile u16 *) (ioaddr + (reg));
}

static inline u8 sdhci_readb(struct sdhci_host *host, int reg)
{
    return *(const volatile u8 *) (ioaddr + reg);
}

int add_sdhci(struct sdhci_host *host); //, u32 max_clk, u32 min_clk);

struct sdhci_host host_dev;
struct sdhci_host *host = &host_dev;
struct mmc mmc_dev;
struct mmc *mmc = &mmc_dev;

int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data);
#if !defined(CONFIG_MMC_TRACE)
#define mmc_send_cmd sdhci_send_command
#else
int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
    int ret;

#ifdef CONFIG_MMC_TRACE
    int i;
    u8 *ptr;

    printf("CMD_SEND:%d\n", cmd->cmdidx);
    printf("\t\tARG\t\t\t 0x%08X\n", cmd->cmdarg);
    ret = sdhci_send_command(mmc, cmd, data);
    //ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
    switch (cmd->resp_type)
    {
        case MMC_RSP_NONE:
            printf("\t\tMMC_RSP_NONE\n");
            break;
        case MMC_RSP_R1:
            printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%08X \n", cmd->response[0]);
            break;
        case MMC_RSP_R1b:
            printf("\t\tMMC_RSP_R1b\t\t 0x%08X \n", cmd->response[0]);
            break;
        case MMC_RSP_R2:
            printf("\t\tMMC_RSP_R2\t\t 0x%08X \n", cmd->response[0]);
            printf("\t\t          \t\t 0x%08X \n", cmd->response[1]);
            printf("\t\t          \t\t 0x%08X \n", cmd->response[2]);
            printf("\t\t          \t\t 0x%08X \n", cmd->response[3]);
            printf("\n");
            printf("\t\t\t\t\tDUMPING DATA\n");
            for (i = 0; i < 4; i++)
            {
                int j;
                printf("\t\t\t\t\t%03d - ", i * 4);
                ptr = (u8 *) & cmd->response[i];
                ptr += 3;
                for (j = 0; j < 4; j++)
                    printf("%02X ", *ptr--);
                printf("\n");
            }
            break;
        case MMC_RSP_R3:
            printf("\t\tMMC_RSP_R3,4\t\t 0x%08X \n", cmd->response[0]);
            break;
        default:
            printf("\t\tERROR MMC rsp not supported\n");
            break;
    }
#else
    ret = sdhci_send_command(mmc, cmd, data);
    //ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
#endif
    return ret;
}
#endif

int mmc_send_status(struct mmc *mmc, int timeout)
{
    struct mmc_cmd cmd;
    int err, retries = 5;
#ifdef CONFIG_MMC_TRACE
    int status;
#endif

    cmd.cmdidx = MMC_CMD_SEND_STATUS;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    do
    {
        err = mmc_send_cmd(mmc, &cmd, NULL);
        if (!err)
        {
            if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
                (cmd.response[0] & MMC_STATUS_CURR_STATE) != MMC_STATE_PRG)
                break;
            else if (cmd.response[0] & MMC_STATUS_MASK)
            {
                printf("Status Error: 0x%08X\n", cmd.response[0]);
                return COMM_ERR;
            }
        }
        else if (--retries < 0)
            return err;

        mdelay(1);

    }
    while (timeout--);

#ifdef CONFIG_MMC_TRACE
    status = (cmd.response[0] & MMC_STATUS_CURR_STATE) >> 9;
    printf("CURR STATE:%d\n", status);
#endif
    if (timeout <= 0)
    {
        printf("Timeout waiting card ready\n");
        return TIMEOUT;
    }

    return 0;
}

static int mmc_set_blocklen(struct mmc *mmc, int len)
{
    struct mmc_cmd cmd;

    cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = len;

    return mmc_send_cmd(mmc, &cmd, NULL);
}

#if 0
struct mmc *get_mmc(void)
{
    struct mmc *mmc = &mmc_dev;
    if (mmc->has_init)
        return mmc;
    else
        return NULL;
}
#endif

static u32 mmc_read_blocks(struct mmc *mmc, void *dst, u32 start, u32 blkcnt)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    if (blkcnt > 1)
        cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
    else
        cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

    if (mmc->high_capacity)
        cmd.cmdarg = start;
    else
        cmd.cmdarg = start * mmc->read_bl_len;

    cmd.resp_type = MMC_RSP_R1;

    data.dest = dst;
    data.blocks = blkcnt;
    data.blocksize = mmc->read_bl_len;
    data.flags = MMC_DATA_READ;

    if (mmc_send_cmd(mmc, &cmd, &data))
        return 0;

    if (blkcnt > 1)
    {
        cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
        cmd.cmdarg = 0;
        cmd.resp_type = MMC_RSP_R1b;
        if (mmc_send_cmd(mmc, &cmd, NULL))
        {
            printf("mmc fail to send stop cmd\n");
            return 0;
        }
    }

    return blkcnt;
}

#if !defined(BOOT_MODE_BOOT1)
#define mmc_host_is_spi(mmc)	0
static u32 mmc_write_blocks(struct mmc *mmc, u32 start,
                            u32 blkcnt, const void *src)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    int timeout = 1000;

    if (blkcnt == 0)
        return 0;
    else if (blkcnt == 1)
        cmd.cmdidx = MMC_CMD_WRITE_SINGLE_BLOCK;
    else
        cmd.cmdidx = MMC_CMD_WRITE_MULTIPLE_BLOCK;

    if (mmc->high_capacity)
        cmd.cmdarg = start;
    else
        cmd.cmdarg = start * mmc->write_bl_len;

    cmd.resp_type = MMC_RSP_R1;

    data.src = src;
    data.blocks = blkcnt;
    data.blocksize = mmc->write_bl_len;
    data.flags = MMC_DATA_WRITE;

    if (mmc_send_cmd(mmc, &cmd, &data))
    {
        printf("mmc write failed\n");
        return 0;
    }

    /* SPI multiblock writes terminate using a special
     * token, not a STOP_TRANSMISSION request.
     */
    if (!mmc_host_is_spi(mmc) && blkcnt > 1)
    {
        cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
        cmd.cmdarg = 0;
        cmd.resp_type = MMC_RSP_R1b;
        if (mmc_send_cmd(mmc, &cmd, NULL))
        {
            printf("mmc fail to send stop cmd\n");
            return 0;
        }
    }

    /* Waiting for the ready status */
    if (mmc_send_status(mmc, timeout))
        return 0;

    return blkcnt;
}

static int mmc_erase_t(struct mmc *mmc, u32 start, u32 blkcnt)
{
    struct mmc_cmd cmd;
    u32 end;
    int err, start_cmd, end_cmd;

    if (mmc->high_capacity)
    {
        end = start + blkcnt - 1;
    }
    else
    {
        end = (start + blkcnt - 1) * mmc->write_bl_len;
        start *= mmc->write_bl_len;
    }

    if (IS_SD(mmc))
    {
        start_cmd = SD_CMD_ERASE_WR_BLK_START;
        end_cmd = SD_CMD_ERASE_WR_BLK_END;
    }
    else
    {
        start_cmd = MMC_CMD_ERASE_GROUP_START;
        end_cmd = MMC_CMD_ERASE_GROUP_END;
    }

    cmd.cmdidx = start_cmd;
    cmd.cmdarg = start;
    cmd.resp_type = MMC_RSP_R1;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    cmd.cmdidx = end_cmd;
    cmd.cmdarg = end;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    cmd.cmdidx = MMC_CMD_ERASE;
    cmd.cmdarg = SECURE_ERASE;
    cmd.resp_type = MMC_RSP_R1b;

    err = mmc_send_cmd(mmc, &cmd, NULL);
    if (err)
        goto err_out;

    return 0;

  err_out:
    puts("mmc erase failed\n");
    return err;
}

int mmc_berase(int dev_num, u32 start, u32 blkcnt)
{
    int err = 0;
    u32 blk = 0, blk_r = 0;
    int timeout = 1000;

    mmc->erase_grp_size = 1;

#if 0
    if ((start % mmc->erase_grp_size) || (blkcnt % mmc->erase_grp_size))
        printf("\n\nCaution! Your devices Erase group is 0x%x\n"
               "The erase range would be change to "
               "0x%llx~0x%llx\n\n",
               mmc->erase_grp_size, start & ~(mmc->erase_grp_size - 1),
               ((start + blkcnt + mmc->erase_grp_size)
                & ~(mmc->erase_grp_size - 1)) - 1);
#endif

    while (blk < blkcnt)
    {
        blk_r = ((blkcnt - blk) > mmc->erase_grp_size) ?
            mmc->erase_grp_size : (blkcnt - blk);
        err = mmc_erase_t(mmc, start + blk, blk_r);
        if (err)
            break;

        blk += blk_r;

        /* Waiting for the ready status */
        if (mmc_send_status(mmc, timeout))
            return -1;
    }

    return 0;
}

int mmc_bwrite(int dev_num, u32 start, u32 blkcnt, const void *src)
{
    u32 cur, blocks_todo = blkcnt;

    //struct mmc *mmc = find_mmc_device(dev_num);
    //if (!mmc)
    //    return -1;

    if (mmc_set_blocklen(mmc, mmc->write_bl_len))
        return -1;

    do
    {
        cur = (blocks_todo > mmc->cfg->b_max) ? mmc->cfg->b_max : blocks_todo;
        if (mmc_write_blocks(mmc, start, cur, src) != cur)
            return -1;
        blocks_todo -= cur;
        start += cur;
        src += cur * mmc->write_bl_len;
    }
    while (blocks_todo > 0);

    return 0;
}
#endif

int mmc_bread(int dev_num, u32 start, u32 blkcnt, void *dst)
{
    u32 cur, blocks_todo = blkcnt;

    if (blkcnt == 0)
        return -1;

    //struct mmc *mmc = get_mmc();
    //if (!mmc)
    //    return -1;

    if (mmc_set_blocklen(mmc, mmc->read_bl_len))
        return -1;

    do
    {
        cur = (blocks_todo > mmc->cfg->b_max) ? mmc->cfg->b_max : blocks_todo;
        if (mmc_read_blocks(mmc, dst, start, cur) != cur)
            return -1;
        blocks_todo -= cur;
        start += cur;
        dst += cur * mmc->read_bl_len;
    }
    while (blocks_todo > 0);

    return 0;
}

static int mmc_go_idle(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err;

    mdelay(1);

    cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
    cmd.cmdarg = 0;
    cmd.resp_type = MMC_RSP_NONE;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    mdelay(2);

    return 0;
}

static int sd_send_op_cond(struct mmc *mmc)
{
    int timeout = 1000;
    int err;
    struct mmc_cmd cmd;

    do
    {
        cmd.cmdidx = MMC_CMD_APP_CMD;
        cmd.resp_type = MMC_RSP_R1;
        cmd.cmdarg = 0;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
        cmd.resp_type = MMC_RSP_R3;

        /*
         * Most cards do not answer if some reserved bits
         * in the ocr are set. However, Some controller
         * can set bit 7 (reserved for low voltages), but
         * how to manage low voltages SD card is not yet
         * specified.
         */
        cmd.cmdarg = (mmc->cfg->voltages & 0xff8000);

        if (mmc->version == SD_VERSION_2)
            cmd.cmdarg |= OCR_HCS;

        err = mmc_send_cmd(mmc, &cmd, NULL);

        if (err)
            return err;

        mdelay(1);
    }
    while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

    if (timeout <= 0)
        return UNUSABLE_ERR;

    if (mmc->version != SD_VERSION_2)
        mmc->version = SD_VERSION_1_0;

    mmc->ocr = cmd.response[0];

    mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
    mmc->rca = 0;

    return 0;
}

/* We pass in the cmd since otherwise the init seems to fail */
static int mmc_send_op_cond_iter(struct mmc *mmc, struct mmc_cmd *cmd,
                                 int use_arg)
{
    int err;

    cmd->cmdidx = MMC_CMD_SEND_OP_COND;
    cmd->resp_type = MMC_RSP_R3;
    cmd->cmdarg = 0;
    if (use_arg)
    {
        cmd->cmdarg =
            (mmc->cfg->voltages &
             (mmc->op_cond_response & OCR_VOLTAGE_MASK)) |
            (mmc->op_cond_response & OCR_ACCESS_MODE);

        if (mmc->cfg->host_caps & MMC_MODE_HC)
            cmd->cmdarg |= OCR_HCS;
    }
    err = mmc_send_cmd(mmc, cmd, NULL);
    if (err)
        return err;
    mmc->op_cond_response = cmd->response[0];
    return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err, i;

    /* Some cards seem to need this */
    mmc_go_idle(mmc);

    /* Asking to the card its capabilities */
    mmc->op_cond_pending = 1;
    /* only query ocr, not to set */
    for (i = 0; i < 1; i++)
    {
        err = mmc_send_op_cond_iter(mmc, &cmd, i != 0);
        if (err)
            return err;

        /* exit if not busy (flag seems to be inverted) */
        if (mmc->op_cond_response & OCR_BUSY)
            return 0;
    }
    return IN_PROGRESS;
}

int mmc_complete_op_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int timeout = 1000;
    u32 start;
    int err;

    mmc->op_cond_pending = 0;
    start = how_long(0);
    do
    {
        err = mmc_send_op_cond_iter(mmc, &cmd, 1);
        if (err)
            return err;
        if (how_long(start) > timeout)
            return UNUSABLE_ERR;
        udelay(100);
    }
    while (!(mmc->op_cond_response & OCR_BUSY));

    mmc->version = MMC_VERSION_UNKNOWN;
    mmc->ocr = cmd.response[0];

    mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
    mmc->rca = 1;

    return 0;
}

#ifdef HS_BOOT
static int mmc_send_ext_csd(struct mmc *mmc, u8 * ext_csd)
{
    struct mmc_cmd cmd;
    struct mmc_data data;
    int err;

    /* Get the Card Status Register */
    cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    data.dest = (char *) ext_csd;
    data.blocks = 1;
    data.blocksize = MMC_MAX_BLOCK_LEN;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd(mmc, &cmd, &data);

    return err;
}

static int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
    struct mmc_cmd cmd;
    int timeout = 1000;
    int ret;

    cmd.cmdidx = MMC_CMD_SWITCH;
    cmd.resp_type = MMC_RSP_R1b;
    cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
        (index << 16) | (value << 8) | set;

    ret = mmc_send_cmd(mmc, &cmd, NULL);

    /* Waiting for the ready status */
    if (!ret)
        ret = mmc_send_status(mmc, timeout);

    return ret;

}

static int mmc_change_freq(struct mmc *mmc)
{
    ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
    char cardtype;
    int err;

    mmc->card_caps = 0;

    /* Only version 4 supports high-speed */
    if (mmc->version < MMC_VERSION_4)
        return 0;

    err = mmc_send_ext_csd(mmc, ext_csd);

    if (err)
        return err;

    cardtype = ext_csd[EXT_CSD_CARD_TYPE] & 0xf;

    err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING, 1);

    if (err)
        return err;

    /* Now check to see that it worked */
    err = mmc_send_ext_csd(mmc, ext_csd);

    if (err)
        return err;

    /* No high-speed support */
    if (!ext_csd[EXT_CSD_HS_TIMING])
        return 0;

    /* High Speed is set, there are two types: 52MHz and 26MHz */
    if (cardtype & MMC_HS_52MHZ)
        mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
    else
        mmc->card_caps |= MMC_MODE_HS;

    return 0;
}

static int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 * resp)
{
    struct mmc_cmd cmd;
    struct mmc_data data;

    /* Switch the frequency */
    cmd.cmdidx = SD_CMD_SWITCH_FUNC;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = (mode << 31) | 0xffffff;
    cmd.cmdarg &= ~(0xf << (group * 4));
    cmd.cmdarg |= value << (group * 4);

    data.dest = (char *) resp;
    data.blocksize = 64;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    return mmc_send_cmd(mmc, &cmd, &data);
}

static int sd_change_freq(struct mmc *mmc)
{
    int err;
    struct mmc_cmd cmd;
    ALLOC_CACHE_ALIGN_BUFFER(u32, scr, 2);
    ALLOC_CACHE_ALIGN_BUFFER(u32, switch_status, 16);
    struct mmc_data data;
    int timeout;

    mmc->card_caps = 0;

    /* Read the SCR to find out if this card supports higher speeds */
    cmd.cmdidx = MMC_CMD_APP_CMD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    cmd.cmdidx = SD_CMD_APP_SEND_SCR;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = 0;

    timeout = 3;

  retry_scr:
    data.dest = (char *) scr;
    data.blocksize = 8;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;

    err = mmc_send_cmd(mmc, &cmd, &data);

    if (err)
    {
        if (timeout--)
            goto retry_scr;

        return err;
    }

    mmc->scr[0] = __be32_to_cpu(scr[0]);
    mmc->scr[1] = __be32_to_cpu(scr[1]);

    switch ((mmc->scr[0] >> 24) & 0xf)
    {
        case 0:
            mmc->version = SD_VERSION_1_0;
            break;
        case 1:
            mmc->version = SD_VERSION_1_10;
            break;
        case 2:
            mmc->version = SD_VERSION_2;
            if ((mmc->scr[0] >> 15) & 0x1)
                mmc->version = SD_VERSION_3;
            break;
        default:
            mmc->version = SD_VERSION_1_0;
            break;
    }

    if (mmc->scr[0] & SD_DATA_4BIT)
        mmc->card_caps |= MMC_MODE_4BIT;

    /* Version 1.0 doesn't support switching */
    if (mmc->version == SD_VERSION_1_0)
        return 0;

    timeout = 4;
    while (timeout--)
    {
        /* between SD_CMD_APP_SEND_SCR and SD_SWITCH_CHECK, to wait for a moment */
        mdelay(1);
        err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1, (u8 *) switch_status);

        if (err)
            return err;

        /* The high-speed function is busy.  Try again */
        if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
            break;
    }

    /* If high-speed isn't supported, we return */
    if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
        return 0;

    /*
     * If the host doesn't support SD_HIGHSPEED, do not switch card to
     * HIGHSPEED mode even if the card support SD_HIGHSPPED.
     * This can avoid furthur problem when the card runs in different
     * mode between the host.
     */
    if (!((mmc->cfg->host_caps & MMC_MODE_HS_52MHz) &&
          (mmc->cfg->host_caps & MMC_MODE_HS)))
        return 0;

    /* between SD_SWITCH_CHECK and SD_SWITCH_SWITCH, to wait for a moment */
    mdelay(1);
    err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *) switch_status);

    if (err)
        return err;

    if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000)
        mmc->card_caps |= MMC_MODE_HS;

    return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
    10000,
    100000,
    1000000,
    10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
    0,                          /* reserved */
    10,
    12,
    13,
    15,
    20,
    25,
    30,
    35,
    40,
    45,
    50,
    55,
    60,
    70,
    80,
};
#endif

void sdhci_set_ios(struct mmc *mmc);
#define mmc_set_ios sdhci_set_ios
#if 0
static void mmc_set_ios(struct mmc *mmc)
{
    if (mmc->cfg->ops->set_ios)
        mmc->cfg->ops->set_ios(mmc);
}
#endif

static void mmc_set_clock(struct mmc *mmc, u32 clock)
{
#if 0
    if (clock > mmc->cfg->f_max)
        clock = mmc->cfg->f_max;

    if (clock < mmc->cfg->f_min)
        clock = mmc->cfg->f_min;
#endif

    mmc->clock = clock;

    mmc_set_ios(mmc);
}

static void mmc_set_bus_width(struct mmc *mmc, u32 width)
{
    mmc->bus_width = width;

    mmc_set_ios(mmc);
}

unsigned long force_sdio_clk_div = 0;
static int mmc_startup(struct mmc *mmc)
{
    int err;
    //u32 mult, freq;
    struct mmc_cmd cmd;
    //ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);
    //ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, MMC_MAX_BLOCK_LEN);
    int timeout = 1000;

    /* Put the Card in Identify Mode */
    cmd.cmdidx = MMC_CMD_ALL_SEND_CID;
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = 0;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    sd_memcpy((void *)mmc->cid, (void *)cmd.response, 16);

    /*
     * For MMC cards, set the Relative Address.
     * For SD cards, get the Relatvie Address.
     * This also puts the cards into Standby State
     */
    cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
    cmd.cmdarg = mmc->rca << 16;
    cmd.resp_type = MMC_RSP_R6;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    if (IS_SD(mmc))
    {
        temp_variable = cmd.response[0];
        mmc->rca = (temp_variable >> 16) & 0xffff;
    }

    /* Get the Card-Specific Data */
    cmd.cmdidx = MMC_CMD_SEND_CSD;
    cmd.resp_type = MMC_RSP_R2;
    cmd.cmdarg = mmc->rca << 16;
    
    err = mmc_send_cmd(mmc, &cmd, NULL);

    /* Waiting for the ready status */
    mmc_send_status(mmc, timeout);

    if (err)
        return err;

#if 0
    mmc->csd[0] = cmd.response[0];
    mmc->csd[1] = cmd.response[1];
    mmc->csd[2] = cmd.response[2];
    mmc->csd[3] = cmd.response[3];
#endif
#ifdef HS_BOOT
    if (mmc->version == MMC_VERSION_UNKNOWN)
    {
        int version = (cmd.response[0] >> 26) & 0xf;

        switch (version)
        {
            case 0:
                mmc->version = MMC_VERSION_1_2;
                break;
            case 1:
                mmc->version = MMC_VERSION_1_4;
                break;
            case 2:
                mmc->version = MMC_VERSION_2_2;
                break;
            case 3:
                mmc->version = MMC_VERSION_3;
                break;
            case 4:
                mmc->version = MMC_VERSION_4;
                break;
            default:
                mmc->version = MMC_VERSION_1_2;
                break;
        }
    }

    /* divide frequency by 10, since the mults are 10x bigger */
    freq = fbase[(cmd.response[0] & 0x7)];
    mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

    mmc->tran_speed = freq * mult;
#endif
    temp_variable = cmd.response[1];
    mmc->read_bl_len = 1 << ((temp_variable >> 16) & 0xf);

    if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
        mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

#if !defined(BOOT_MODE_BOOT1)
    if (IS_SD(mmc))
        mmc->write_bl_len = mmc->read_bl_len;
    else
        mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);

    if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
        mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
#endif

    /* Select the card, and put it into Transfer Mode */
    cmd.cmdidx = MMC_CMD_SELECT_CARD;
    cmd.resp_type = MMC_RSP_R1;
    cmd.cmdarg = mmc->rca << 16;
    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

#if 1
    if( (0==force_sdio_clk_div)
       && (otp_parse_config(OTP_SD_CLK_DIV_SHIFT)>=1)
       && (otp_parse_config(OTP_SD_CLK_DIV_SHIFT)<4)  )
    {
        if (IS_SD(mmc))
        {
            if(1) //if (mmc->card_caps & MMC_MODE_4BIT)
            {
                cmd.cmdidx = MMC_CMD_APP_CMD;
                cmd.resp_type = MMC_RSP_R1;
                cmd.cmdarg = mmc->rca << 16;
    
                err = mmc_send_cmd(mmc, &cmd, NULL);
                if (err)
                    return err;
    
                cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
                cmd.resp_type = MMC_RSP_R1;
                cmd.cmdarg = 2;
                err = mmc_send_cmd(mmc, &cmd, NULL);
                if (err)
                    return err;
    
                mmc_set_bus_width(mmc, 4);
            }
        }
    }
#endif

#ifdef HS_BOOT
    /*
     * For SD, its erase group is always one sector
     */
    if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4))
    {
        /* check  ext_csd version and capacity */
        err = mmc_send_ext_csd(mmc, ext_csd);

        switch (ext_csd[EXT_CSD_REV])
        {
            case 1:
                mmc->version = MMC_VERSION_4_1;
                break;
            case 2:
                mmc->version = MMC_VERSION_4_2;
                break;
            case 3:
                mmc->version = MMC_VERSION_4_3;
                break;
            case 5:
                mmc->version = MMC_VERSION_4_41;
                break;
            case 6:
                mmc->version = MMC_VERSION_4_5;
                break;
        }
    }

    if (IS_SD(mmc))
        err = sd_change_freq(mmc);
    else
        err = mmc_change_freq(mmc);

    if (err)
        return err;

    /* Restrict card's capabilities by what the host can do */
    mmc->card_caps &= mmc->cfg->host_caps;

    if (IS_SD(mmc))
    {
        if (mmc->card_caps & MMC_MODE_4BIT)
        {
            cmd.cmdidx = MMC_CMD_APP_CMD;
            cmd.resp_type = MMC_RSP_R1;
            cmd.cmdarg = mmc->rca << 16;

            err = mmc_send_cmd(mmc, &cmd, NULL);
            if (err)
                return err;

            cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
            cmd.resp_type = MMC_RSP_R1;
            cmd.cmdarg = 2;
            err = mmc_send_cmd(mmc, &cmd, NULL);
            if (err)
                return err;

            mmc_set_bus_width(mmc, 4);
        }

        if (mmc->card_caps & MMC_MODE_HS)
            mmc->tran_speed = 50000000;
        else
            mmc->tran_speed = 25000000;
    }
    else
    {
        int idx;

        /* An array of possible bus widths in order of preference */
        static unsigned ext_csd_bits[] = {
            EXT_CSD_BUS_WIDTH_8,
            EXT_CSD_BUS_WIDTH_4,
            EXT_CSD_BUS_WIDTH_1,
        };

        /* An array to map CSD bus widths to host cap bits */
        static unsigned ext_to_hostcaps[] = {
            [EXT_CSD_BUS_WIDTH_4] = MMC_MODE_4BIT,
            [EXT_CSD_BUS_WIDTH_8] = MMC_MODE_8BIT,
        };

        /* An array to map chosen bus width to an integer */
        static unsigned widths[] = {
            8, 4, 1,
        };

        for (idx = 0; idx < ARRAY_SIZE(ext_csd_bits); idx++)
        {
            unsigned int extw = ext_csd_bits[idx];

            /*
             * Check to make sure the controller supports
             * this bus width, if it's more than 1
             */
            if (extw != EXT_CSD_BUS_WIDTH_1 &&
                !(mmc->cfg->host_caps & ext_to_hostcaps[extw]))
                continue;

            err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
                             EXT_CSD_BUS_WIDTH, extw);

            if (err)
                continue;

            mmc_set_bus_width(mmc, widths[idx]);

            err = mmc_send_ext_csd(mmc, test_csd);
            if (!err && ext_csd[EXT_CSD_PARTITIONING_SUPPORT]
                == test_csd[EXT_CSD_PARTITIONING_SUPPORT]
                && ext_csd[EXT_CSD_ERASE_GROUP_DEF]
                == test_csd[EXT_CSD_ERASE_GROUP_DEF]
                && ext_csd[EXT_CSD_REV]
                == test_csd[EXT_CSD_REV]
                && ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
                == test_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
                && memcmp(&ext_csd[EXT_CSD_SEC_CNT],
                          &test_csd[EXT_CSD_SEC_CNT], 4) == 0)
            {

                mmc->card_caps |= ext_to_hostcaps[extw];
                break;
            }
        }

        if (mmc->card_caps & MMC_MODE_HS)
        {
            if (mmc->card_caps & MMC_MODE_HS_52MHz)
                mmc->tran_speed = 52000000;
            else
                mmc->tran_speed = 26000000;
        }
    }

    mmc_set_clock(mmc, mmc->tran_speed);
#endif
    return 0;
}

static int mmc_send_if_cond(struct mmc *mmc)
{
    struct mmc_cmd cmd;
    int err;

    cmd.cmdidx = SD_CMD_SEND_IF_COND;
    /* We set the bit if the host supports voltages between 2.7 and 3.6 V */
    cmd.cmdarg = ((mmc->cfg->voltages & 0xff8000) != 0) << 8 | 0xaa;
    cmd.resp_type = MMC_RSP_R7;

    err = mmc_send_cmd(mmc, &cmd, NULL);

    if (err)
        return err;

    temp_variable = cmd.response[0];
    if ((temp_variable & 0xff) != 0xaa)
        return UNUSABLE_ERR;
    else
        mmc->version = SD_VERSION_2;

    return 0;
}

static struct mmc *mmc_create(const struct mmc_config *cfg, void *priv)
{
    //struct mmc *mmc = &mmc_dev;
    mmc->cfg = cfg;
    mmc->priv = priv;

    //mmc->block_read = mmc_bread;
    return mmc;
}

static int mmc_start_init(struct mmc *mmc)
{
    int err;

    if (mmc->has_init)
        return 0;

    mmc_set_clock(mmc, 1);
    mmc_set_bus_width(mmc, 1);

    /* Reset the Card */
    err = mmc_go_idle(mmc);

    if (err)
        return err;

    /* Test for SD version 2 */
    err = mmc_send_if_cond(mmc);

    /* Now try to get the SD card's operating condition */
    err = sd_send_op_cond(mmc);

    /* If the command timed out, we check for an MMC card */
    if (err == TIMEOUT)
    {
        err = mmc_send_op_cond(mmc);

        if (err && err != IN_PROGRESS)
        {
            printf("Card did not respond to voltage select!\n");
            return UNUSABLE_ERR;
        }
    }

    if (err == IN_PROGRESS)
        mmc->init_in_progress = 1;

    return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
    int err = 0;

    if (mmc->op_cond_pending)
        err = mmc_complete_op_cond(mmc);

    if (!err)
        err = mmc_startup(mmc);
    if (err)
        mmc->has_init = 0;
    else
    {
        mmc->has_init = 1;
#if 0
        printf("Vendor: Man %06x Snr %04x%04x\n",
               mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
               (mmc->cid[3] >> 16) & 0xffff);
        printf("Product: %c%c%c%c%c%c\n", mmc->cid[0] & 0xff,
               (mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
               (mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
               (mmc->cid[2] >> 24) & 0xff);
        printf("Revision: %d.%d\n", (mmc->cid[2] >> 20) & 0xf,
               (mmc->cid[2] >> 16) & 0xf);
#endif
    }
    mmc->init_in_progress = 0;
    return err;
}

static void mmc_init(struct mmc *mmc)
{
    int cnt = 20;
    int err;
    //unsigned start = how_long(0);

    while (cnt--)
    {
        err = mmc_start_init(mmc);
        if (!err || err == IN_PROGRESS)
            err = mmc_complete_init(mmc);

        if (!err)
            break;
    }
    //printf("%s: %d, time %lu\n", __func__, err, how_long(start));
}

#ifdef CONFIG_MMC_TRACE
#define DRIVER_NAME "mmc"
static void sdhci_dumpregs(struct sdhci_host *host)
{
    printf(DRIVER_NAME ": ============== REGISTER DUMP ==============\n");

    printf(DRIVER_NAME ": Sys addr: 0x%08x | Version:  0x%08x\n",
           sdhci_readl(host, SDHCI_DMA_ADDRESS),
           sdhci_readw(host, SDHCI_HOST_VERSION));
    printf(DRIVER_NAME ": Blk size: 0x%08x | Blk cnt:  0x%08x\n",
           sdhci_readw(host, SDHCI_BLOCK_SIZE),
           sdhci_readw(host, SDHCI_BLOCK_COUNT));
    printf(DRIVER_NAME ": Argument: 0x%08x | Trn mode: 0x%08x\n",
           sdhci_readl(host, SDHCI_ARGUMENT),
           sdhci_readw(host, SDHCI_TRANSFER_MODE));
    printf(DRIVER_NAME ": Present:  0x%08x | Host ctl: 0x%08x\n",
           sdhci_readl(host, SDHCI_PRESENT_STATE),
           sdhci_readb(host, SDHCI_HOST_CONTROL));
    printf(DRIVER_NAME ": Power:    0x%08x | Blk gap:  0x%08x\n",
           sdhci_readb(host, SDHCI_POWER_CONTROL),
           sdhci_readb(host, SDHCI_BLOCK_GAP_CONTROL));
    printf(DRIVER_NAME ": Wake-up:  0x%08x | Clock:    0x%08x\n",
           sdhci_readb(host, SDHCI_WAKE_UP_CONTROL),
           sdhci_readw(host, SDHCI_CLOCK_CONTROL));
    printf(DRIVER_NAME ": Timeout:  0x%08x | Int stat: 0x%08x\n",
           sdhci_readb(host, SDHCI_TIMEOUT_CONTROL),
           sdhci_readl(host, SDHCI_INT_STATUS));
    printf(DRIVER_NAME ": Int enab: 0x%08x | Sig enab: 0x%08x\n",
           sdhci_readl(host, SDHCI_INT_ENABLE),
           sdhci_readl(host, SDHCI_SIGNAL_ENABLE));
    printf(DRIVER_NAME ": AC12 err: 0x%08x | Slot int: 0x%08x\n",
           sdhci_readw(host, SDHCI_ACMD12_ERR),
           sdhci_readw(host, SDHCI_SLOT_INT_STATUS));
    printf(DRIVER_NAME ": Caps:     0x%08x | Caps_1:   0x%08x\n",
           sdhci_readl(host, SDHCI_CAPABILITIES),
           sdhci_readl(host, SDHCI_CAPABILITIES_1));
    printf(DRIVER_NAME ": Cmd:      0x%08x | Max curr: 0x%08x\n",
           sdhci_readw(host, SDHCI_COMMAND),
           sdhci_readl(host, SDHCI_MAX_CURRENT));
    printf(DRIVER_NAME ": Host ctl2: 0x%08x\n",
           sdhci_readw(host, SDHCI_HOST_CONTROL2));

    printf(DRIVER_NAME ": ===========================================\n");
}
#endif

static void sdhci_reset(struct sdhci_host *host, u8 mask)
{
    unsigned long timeout;

    //if (host->ops->platform_reset_enter)
    //    host->ops->platform_reset_enter(host, mask);

    /* Wait max 100 ms */
    timeout = 100;
    sdhci_writeb(host, mask, SDHCI_SOFTWARE_RESET);
    while (sdhci_readb(host, SDHCI_SOFTWARE_RESET) & mask)
    {
        if (timeout == 0)
        {
            printf("%s: Reset 0x%x never completed.\n", __func__, (int) mask);
            break;
        }
        timeout--;
        mdelay(1);
    }

    //if (host->ops->platform_reset_exit)
    //    host->ops->platform_reset_exit(host, mask);
}

static void sdhci_cmd_done(struct sdhci_host *host, struct mmc_cmd *cmd)
{
    int i;
    if (cmd->resp_type & MMC_RSP_136)
    {
        /* CRC is stripped so we need to do some shifting. */
        for (i = 0; i < 4; i++)
        {
            cmd->response[i] = sdhci_readl(host,
                                           SDHCI_RESPONSE + (3 - i) * 4) << 8;
            if (i != 3)
                cmd->response[i] |= sdhci_readb(host,
                                                SDHCI_RESPONSE + (3 - i) * 4 -
                                                1);
        }
    }
    else
    {
        cmd->response[0] = sdhci_readl(host, SDHCI_RESPONSE);
    }
}

#define ___swab32(x) \
     ((unsigned int)( \
         (((unsigned int)(x) & (unsigned int)0x000000ffUL) << 24) | \
         (((unsigned int)(x) & (unsigned int)0x0000ff00UL) <<  8) | \
         (((unsigned int)(x) & (unsigned int)0x00ff0000UL) >>  8) | \
         (((unsigned int)(x) & (unsigned int)0xff000000UL) >> 24) ))

#define PIN_STRAP_REG_ADDR  0xbf004828UL
static void sdhci_transfer_pio(struct sdhci_host *host, struct mmc_data *data)
{
    int i;
    char *offs;
    u32 scratch;
    u32 little_endian;
    
#if defined(IPL)    
    if (0 == (*((volatile unsigned long *) PIN_STRAP_REG_ADDR) & 0x01))
        little_endian = 1;
    else
        little_endian = 0;
#else
#if defined(BIG_ENDIAN)
    little_endian = 0;
#else
    little_endian = 1;
#endif
#endif        
                         
    for (i = 0; i < data->blocksize; i += 4)
    {
        offs = data->dest + i;
        if (data->flags == MMC_DATA_READ)
        {
            scratch = sdhci_readl(host, SDHCI_BUFFER);
            *(u32 *) offs = little_endian ? scratch : ___swab32(scratch);
        }
#if !defined(BOOT_MODE_BOOT1)
        else
        {
            scratch = *(u32 *) offs;
            sdhci_writel(host, (little_endian ? scratch : ___swab32(scratch)), SDHCI_BUFFER);
        }
#endif
    }
}

static int sdhci_transfer_data(struct sdhci_host *host, struct mmc_data *data,
                               unsigned int start_addr)
{
    unsigned int stat, rdy, mask, timeout, block = 0;

    timeout = 10000000;
    rdy = SDHCI_INT_SPACE_AVAIL | SDHCI_INT_DATA_AVAIL;
    mask = SDHCI_DATA_AVAILABLE | SDHCI_SPACE_AVAILABLE;

    while(1)
    {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);
        if (stat & SDHCI_INT_ERROR)
        {
            printf("%s: Error detected in status(0x%X)!\n", __func__, stat);
            return -1;
        }
        if (stat & rdy)
        {
            if (!(sdhci_readl(host, SDHCI_PRESENT_STATE) & mask))
                continue;
            sdhci_writel(host, rdy, SDHCI_INT_STATUS);
            sdhci_transfer_pio(host, data);
            data->dest += data->blocksize;
            if (++block >= data->blocks)
                break;
        }
        if (timeout-- > 0)
        {
            udelay(10);
        }
        else
        {
            printf("%s: Transfer data timeout\n", __func__);
            return -1;
        }
    }
    timeout = 10000000;
    do
    {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);

        if (timeout-- > 0)
        {
            udelay(10);
        }
    }
    while (!(stat & SDHCI_INT_DATA_END));
    return 0;
}

/*
 * No command will be sent by driver if card is busy, so driver must wait
 * for card ready state.
 * Every time when card is busy after timeout then (last) timeout value will be
 * increased twice but only if it doesn't exceed global defined maximum.
 * Each function call will use last timeout value. Max timeout can be redefined
 * in board config file.
 */
#ifndef CONFIG_SDHCI_CMD_MAX_TIMEOUT
#define CONFIG_SDHCI_CMD_MAX_TIMEOUT		3200
#endif
#define CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT	100

int sdhci_send_command(struct mmc *mmc, struct mmc_cmd *cmd,
                       struct mmc_data *data)
{
    struct sdhci_host *host = mmc->priv;
    unsigned int stat = 0;
    int ret = 0;
    u32 mask, flags, mode;
    unsigned int time = 0, start_addr = 0;
    unsigned int retry = 10000;

    /* Timeout unit - ms */
    static unsigned int cmd_timeout = CONFIG_SDHCI_CMD_DEFAULT_TIMEOUT;

    sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
    mask = SDHCI_CMD_INHIBIT | SDHCI_DATA_INHIBIT;

    /* We shouldn't wait for data inihibit for stop commands, even
       though they might use busy signaling */
    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
        mask &= ~SDHCI_DATA_INHIBIT;

    while (sdhci_readl(host, SDHCI_PRESENT_STATE) & mask)
    {
        if (time >= cmd_timeout)
        {
            printf("%s: MMC busy ", __func__);
            if (2 * cmd_timeout <= CONFIG_SDHCI_CMD_MAX_TIMEOUT)
            {
                cmd_timeout += cmd_timeout;
                printf("timeout increasing to: %u ms.\n", cmd_timeout);
            }
            else
            {
                printf("timeout.\n");
                return COMM_ERR;
            }
        }
        time++;
        mdelay(1);
    }

    mask = SDHCI_INT_RESPONSE;
    /* Command Complete is not generated
       by the response of CMD12 or CMD23 */
    if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
        mask &= ~SDHCI_INT_RESPONSE;
    if (!(cmd->resp_type & MMC_RSP_PRESENT))
        flags = SDHCI_CMD_RESP_NONE;
    else if (cmd->resp_type & MMC_RSP_136)
        flags = SDHCI_CMD_RESP_LONG;
    else if (cmd->resp_type & MMC_RSP_BUSY)
    {
        flags = SDHCI_CMD_RESP_SHORT_BUSY;
        /* Command Complete is not generated
           by MMC_RSP_BUSY */
        mask &= ~SDHCI_INT_RESPONSE;
        mask |= SDHCI_INT_DATA_END;
    }
    else
        flags = SDHCI_CMD_RESP_SHORT;

    if (cmd->resp_type & MMC_RSP_CRC)
        flags |= SDHCI_CMD_CRC;
    if (cmd->resp_type & MMC_RSP_OPCODE)
        flags |= SDHCI_CMD_INDEX;
    if (data)
        flags |= SDHCI_CMD_DATA;

    /* Set Transfer mode regarding to data flag */
    if (data != 0)
    {
        sdhci_writeb(host, 0xe, SDHCI_TIMEOUT_CONTROL);
        mode = SDHCI_TRNS_BLK_CNT_EN;
        if (data->blocks > 1)
            mode |= SDHCI_TRNS_MULTI;

        if (data->flags == MMC_DATA_READ)
            mode |= SDHCI_TRNS_READ;

        //sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
        //                                    data->blocksize), SDHCI_BLOCK_SIZE);
        sdhci_writew(host, data->blocks, SDHCI_BLOCK_COUNT);
        sdhci_writew(host, mode, SDHCI_TRANSFER_MODE);
    }

    if (data != 0)
        sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
                                            data->blocksize), SDHCI_BLOCK_SIZE);
    sdhci_writel(host, cmd->cmdarg, SDHCI_ARGUMENT);
    sdhci_writew(host, SDHCI_MAKE_CMD(cmd->cmdidx, flags), SDHCI_COMMAND);
#ifdef CONFIG_MMC_TRACE
    sdhci_dumpregs(host);
#endif
    do
    {
        stat = sdhci_readl(host, SDHCI_INT_STATUS);
        if (stat & SDHCI_INT_ERROR)
            break;
        if (--retry == 0)
            break;
        udelay(10);
    }
    while ((stat & mask) != mask);

    if (retry == 0)
    {
        if (host->quirks & SDHCI_QUIRK_BROKEN_R1B)
            return 0;
        else
        {
            printf("%s: Timeout for status update!\n", __func__);
            return TIMEOUT;
        }
    }
    if ((stat & (SDHCI_INT_ERROR | mask)) == mask)
    {
        sdhci_cmd_done(host, cmd);
        sdhci_writel(host, mask, SDHCI_INT_STATUS);
    }
    else
        ret = -1;

    if (!ret && data)
        ret = sdhci_transfer_data(host, data, start_addr);

    if (host->quirks & SDHCI_QUIRK_WAIT_SEND_CMD)
        mdelay(1);

    stat = sdhci_readl(host, SDHCI_INT_STATUS);
    sdhci_writel(host, SDHCI_INT_ALL_MASK, SDHCI_INT_STATUS);
    if (!ret)
        return 0;

    sdhci_reset(host, SDHCI_RESET_CMD);
    sdhci_reset(host, SDHCI_RESET_DATA);
    if (stat & SDHCI_INT_TIMEOUT)
        return TIMEOUT;
    else
        return COMM_ERR;
}


static int sdhci_set_clock(struct mmc *mmc, unsigned int clock)
{
    struct sdhci_host *host = mmc->priv;
    unsigned int div, clk, timeout;

    sdhci_writew(host, 0, SDHCI_CLOCK_CONTROL);

    if (clock == 0)
        return 0;

#if 1 //defined(IPL)
    div = otp_parse_config(OTP_SD_CLK_DIV_SHIFT);
    if(force_sdio_clk_div)
        div = force_sdio_clk_div;
    else if(div>=4)
        div = (0x01 << div);
    else if(div>=1)
        div = (0x08 >> (3 - div));
    else
        div = 0x08;     // ASIC: 100Mhz/16, FPGA: 50Mhz/16
#else
    if (clock > CONFIG_SYS_SDIO_MAX_CLK)
        clock = CONFIG_SYS_SDIO_MAX_CLK;

    /* Version 3.00 divisors must be a multiple of 2. */
    if (mmc->cfg->f_max <= clock)
        div = 1;
    else
    {
        for (div = 2; div < SDHCI_MAX_DIV_SPEC_300; div += 2)
        {
            if ((mmc->cfg->f_max / div) <= clock)
                break;
        }
    }
    div >>= 1;
#endif

    clk = (div & SDHCI_DIV_MASK) << SDHCI_DIVIDER_SHIFT;
    clk |= ((div & SDHCI_DIV_HI_MASK) >> SDHCI_DIV_MASK_LEN)
        << SDHCI_DIVIDER_HI_SHIFT;
    clk |= SDHCI_CLOCK_INT_EN;
    sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

    /* Wait max 20 ms */
    timeout = 20;
    while (0==(sdhci_readw(host, SDHCI_CLOCK_CONTROL) & SDHCI_CLOCK_INT_STABLE))
    {
        if (timeout == 0)
        {
            printf("%s: Internal clock never stabilised.\n", __func__);
            return -1;
        }
        timeout--;
        mdelay(1);
    }

    clk |= (SDHCI_CLOCK_CARD_EN | SDHCI_CLOCK_INT_STABLE);
    sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

    printf("%s: clock %d\n", __func__, clock);

    return 0;
}

static void sdhci_set_power(struct sdhci_host *host, unsigned short power)
{
    u8 pwr = 0;

    if (power == 1)
        pwr = SDHCI_POWER_330;

    if (pwr == 0)
    {
        sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
        return;
    }

    pwr |= SDHCI_POWER_ON;

    sdhci_writeb(host, pwr, SDHCI_POWER_CONTROL);
}

void sdhci_set_ios(struct mmc *mmc)
{
    u32 ctrl;
    struct sdhci_host *host = mmc->priv;

#if 1
    sdhci_set_clock(mmc, mmc->clock);
#else
    if (mmc->clock != host->clock)
        sdhci_set_clock(mmc, mmc->clock);
#endif

    /* Set bus width */
    ctrl = sdhci_readb(host, SDHCI_HOST_CONTROL);
#if 1
    ctrl &= ~SDHCI_CTRL_8BITBUS;
    if (mmc->bus_width == 4)
        ctrl |= SDHCI_CTRL_4BITBUS;
    else
        ctrl &= ~SDHCI_CTRL_4BITBUS;
#else
    if (mmc->bus_width == 8)
    {
        ctrl &= ~SDHCI_CTRL_4BITBUS;
        ctrl |= SDHCI_CTRL_8BITBUS;
    }
    else
    {
        ctrl &= ~SDHCI_CTRL_8BITBUS;
        if (mmc->bus_width == 4)
            ctrl |= SDHCI_CTRL_4BITBUS;
        else
            ctrl &= ~SDHCI_CTRL_4BITBUS;
    }
#endif

#if 1
    ctrl &= ~SDHCI_CTRL_HISPD;
#else
    if (mmc->card_caps & MMC_MODE_HS)
        ctrl |= SDHCI_CTRL_HISPD;
    else
        ctrl &= ~SDHCI_CTRL_HISPD;
#endif

    sdhci_writeb(host, ctrl, SDHCI_HOST_CONTROL);
}

int sdhci_init(struct mmc *mmc)
{
    struct sdhci_host *host = mmc->priv;

    sdhci_set_power(host, 1);

    /* Enable only interrupts served by the SD controller */
    sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK |
                 SDHCI_INT_CARD_REMOVE | SDHCI_INT_CARD_INSERT,
                 SDHCI_INT_ENABLE);
    /* Mask all sdhci interrupt sources */
    sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

    if (host->quirks & SDHCI_QUIRK_NO_CD)
    {
        unsigned int status;
        int cnt = 10;

        sdhci_writeb(host, SDHCI_CTRL_CD_TEST_INS | SDHCI_CTRL_CD_TEST,
                     SDHCI_HOST_CONTROL);

        status = sdhci_readl(host, SDHCI_PRESENT_STATE);
        while (((!(status & SDHCI_CARD_PRESENT)) ||
                (!(status & SDHCI_CARD_STATE_STABLE)) ||
                (!(status & SDHCI_CARD_DETECT_PIN_LEVEL))) && (cnt-- > 0))
        {
            status = sdhci_readl(host, SDHCI_PRESENT_STATE);
            mdelay(1);
        }
        if (cnt <= 0)
        {
            printf("sorry, don't detect any card...\n");
            return -1;
        }
    }

    return 0;
}

void sdhci_reset_all(struct mmc *mmc)
{
    struct sdhci_host *host = mmc->priv;
    sdhci_reset(host, SDHCI_RESET_ALL);
}

#if 0
static const struct mmc_ops sdhci_ops = {
    .send_cmd = sdhci_send_command,
    .set_ios = sdhci_set_ios,
};
#endif

int add_sdhci(struct sdhci_host *host) //, u32 max_clk, u32 min_clk)
{
    unsigned int caps;

    //host->cfg.name = host->name;
    //host->cfg.ops = &sdhci_ops;

    sdhci_reset(host, SDHCI_RESET_ALL);

#if 1 // force SD CLK INV
    unsigned long regval;

    //if(otp_parse_config(OTP_SD_CLK_INV_SHIFT))
    {
        // set 32bits register 0x70 bit[5] = 1 to inverse SDCLK phase
        regval = sdhci_readl(host, 0x70);
        sdhci_writel(host, (regval | (0x01 << 5)), 0x70);
    }
#else
    {
        unsigned long regval;

        // set 32bits register 0x70 bit[5] = 1 to inverse SDCLK phase
        regval = sdhci_readl(host, 0x70);
        sdhci_writel(host, (regval | (0x01 << 5)), 0x70);
    }
#endif

    host->version = sdhci_readw(host, SDHCI_HOST_VERSION);

    caps = sdhci_readl(host, SDHCI_CAPABILITIES);

#if 1
    //host->cfg.f_max = max_clk;
    //host->cfg.f_min = min_clk;
#else
    if (max_clk)
        host->cfg.f_max = max_clk;
    else
    {
        host->cfg.f_max = (caps & SDHCI_CLOCK_V3_BASE_MASK)
            >> SDHCI_CLOCK_BASE_SHIFT;
        host->cfg.f_max *= 1000000;
    }
    if (min_clk)
        host->cfg.f_min = min_clk;
    else
    {
        host->cfg.f_min = host->cfg.f_max / SDHCI_MAX_DIV_SPEC_300;
    }
#endif

    host->cfg.voltages = 0;
    if (caps & SDHCI_CAN_VDD_330)
        host->cfg.voltages |= MMC_VDD_32_33 | MMC_VDD_33_34;
    if (caps & SDHCI_CAN_VDD_300)
        host->cfg.voltages |= MMC_VDD_29_30 | MMC_VDD_30_31;
#ifdef HS_BOOT
    if (caps & SDHCI_CAN_VDD_180)
        host->cfg.voltages |= MMC_VDD_165_195;

    host->cfg.host_caps =
        MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT | MMC_MODE_8BIT;
#else
    host->cfg.host_caps = MMC_MODE_4BIT;
#endif
    if (host->host_caps)
        host->cfg.host_caps |= host->host_caps;

    host->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

    host->mmc = mmc_create(&host->cfg, host);
#if 0
    if (host->mmc == NULL)
    {
        printf("%s: mmc create fail!\n", __func__);
        return -1;
    }
#endif

    return sdhci_init(host->mmc);
}

#if !defined(CONFIG_PANTHER)
static void sdhci_mt_platform_reset_enter(struct sdhci_host *host, u8 mask)
{
    if (mask & SDHCI_RESET_ALL)
    {
        /* enable SD clock */
        ANAREG(CLKEN) |= SD_CLK_EN;

        /* SD driving adjustment */
        GPREG(DRIVER1) &= ~(DRV_SDIO);

        /*
         *  bit 27, 26, 25, 24 :
         *  bit 23, 22, 21, 20 : MDIO_AUX, SWI2C_AUX,    SWI2C, RF_LEGA
         *  bit 19, 18, 17, 16 : JTAG_AUX,   LED_AUX, UART_AUX, I2C_AUX
         *  bit 15, 14, 13, 12 :  PWM_AUX,      ROM1,     JTAG,     AGC
         *  bit 11, 10,  9,  8 :      PWM,       LED,     UART,    MDIO
         *  bit  7,  6,  5,  4 :     PCM1,      PCM0,     ETH1,    ETH0
         *  bit  3,  2,  1,  0 :     SDIO,        W6,      SIP,     SDR
         *
         *  For PINMUX, Enable SDIO function. Max(SDIO_FNC + ETH0_FNC) <=1
         *  ETH0 must be disabled, when SDIO is enabled
         *  MDIO must be disabled, when SDIO is enabled
         */

//              if (host->mmc->caps3 & MMC_CAP3_CONTROL_VOLTAGE)
//                      ANAREG(VOLCTRL) &= ~(3<<0);
//              else
        ANAREG(VOLCTRL) |= (1 << 1);

        /*
         * SDHC forcely disable mdio pin and eth0 function
         * So if DDR mode(SIP mode) and MDIO is used by external ether phy,
         * ether will lose its function
         * (DDR mode + external ETH0 or ETH1 will be failed)
         * (DDR mode + internal ether will be OK)
         */
        /* disable PINMUX mdio function */
        GPREG(PINMUX) &= ~(EN_MDIO_FNC);
        /* disable PINMUX eth0 function */
        GPREG(PINMUX) &= ~(EN_ETH0_FNC);

        /* enable PINMUX sdio function */
        GPREG(PINMUX) |= (EN_SDIO_FNC);
        /* disable external BB mode */
        GPREG(GDEBUG) &= ~(0x90 << 24);
        /* enable W6 */
        GPREG(GDEBUG) |= (1 << 22);
        /* enable sdio module */
        GPREG(GDEBUG) |= (1 << 21);
        /* turn on sdio gated clock */
        GPREG(SWRST) &= ~(PAUSE_SDIO);
        /* reset sdio module */
        GPREG(SWRST) &= ~(SWRST_SDIO);
        mdelay(1);
        GPREG(SWRST) |= (SWRST_SDIO);
    }
}

static void sdhci_mt_platform_reset_exit(struct sdhci_host *host, u8 mask)
{
    if (mask & SDHCI_RESET_ALL)
    {
        /* system endian setting */
        // AHB bus: little to little, AMB bus: little to big
        GPREG(SDIO) =
            (GPREG(SDIO) & ~(0x3UL << SDIO_ENDIAN_PIN)) | (0x2UL <<
                                                           SDIO_ENDIAN_PIN);
    }
}

static struct sdhci_ops sdhci_mt_ops = {
    .platform_reset_enter = sdhci_mt_platform_reset_enter,
    .platform_reset_exit = sdhci_mt_platform_reset_exit,
};
#endif

void mini_sdhc_init(void)
{
    // select pinmux for SDIO
    *(volatile unsigned long *)0xbf004a30 = 0x55540000;

    //mmc = &mmc_dev;
    //host = &host_dev;
    //host->name = "montage-sdhci";
    //ioaddr = (void *) SDIO_BASE;
    //host->ops = &sdhci_mt_ops;
    host->quirks = SDHCI_QUIRK_NO_CD | SDHCI_QUIRK_WAIT_SEND_CMD;
    host->host_caps = MMC_MODE_HC;

    add_sdhci(host); //, CONFIG_SYS_SDIO_MAX_CLK, CONFIG_SYS_SDIO_MIN_CLK);

    mmc->has_init = 0;
    mmc_init(mmc);
}

#ifdef CONFIG_SD_RECOVERY
int sd_read(int start, int blkcnt)
{
    u32 err = 0;
    u8 *buf = (u8 *) buf_address;
#if 1
    err = mmc_bread(0, start, blkcnt, buf);
#else
    err = host->mmc->block_read(0, start, blkcnt, buf);
#endif
}

//////////////////////////////////////////////////////////////////////
////////////testcode!!!!!
//  load .img file from SD card command!!!
//  cmd:sdup
//  description:this command will run command mmc and chk,that is,
//              download .img file from SD card to memory, and run
//              command chk.
//////////////////////////////////////////////////////////////////////  

void sdrc(void)
{
    unsigned int load_loc = bootvars.load_addr;
    struct img_head *ih1;       // kernel header
    struct img_head *ih2;       // filesystem header
    unsigned h;
    unsigned int kb, block_cnt;
    unsigned long byte_count;
    char *arg[] = { "1" };
    int rc;

    // from first block we can get image header message,
    // so we can use "hlen" and "size" to find the image size.
    // 1. find kernel size:
    sd_read(0, 1);
    ih1 = (struct img_head *) load_loc;
    kb = ((ih1->hlen + ih1->size) / 512) + 1;   // blocks for kernel

    // 2. find filesystem size:
    sd_read(0, kb);

    // after find the kernel size and filesystem size, we can caculate 
    // number of blocks the image need, then do command mmc again.
    h = load_loc + ih1->hlen + ih1->size;
    ih2 = (struct img_head *) h;
    block_cnt = ((ih1->hlen + ih1->size + ih2->hlen + ih2->size) / 512) + 1;    // blocks for kernel & filesystem
    sd_read(0, block_cnt);

    //run command chk!!
    byte_count = block_cnt * 512;
    rc = verify_image(load_loc);
    printf("verify header & checksum,rc = %d\n", rc);

    if (rc < 0)
    {
        printf("error file, please change another file and try again...\n");
    }
    else if (rc == 1)
    {
        printf("image file pass verification..\n");
        //run flash_cmd
        flash_erase(CONFIG_BOOT_LOAD_OFS, byte_count, CHECK_BAD_BLOCK);
        flash_write(CONFIG_BOOT_LOAD_OFS, load_loc, byte_count,
                    CHECK_BAD_BLOCK);
        cmd_rst(1, arg);
    }
}
#endif

#ifdef IPL
unsigned int sdio_read(unsigned long dst)
{
    u32 err = 0;
    u8 *buf = (u8 *) dst;

    err = mmc_bread(0, 8, 32, buf);
    if(!err)
    {
        printf("sd OK\n");
        /* always fallback to slow speed in next read (if any) */
        force_sdio_clk_div = 0x80;
        return dst;
    }
    else
    {
        printf("sd FAIL\n");
        force_sdio_clk_div = 0x80;
        return 0;
    }
}
#else
#ifndef REAL_MINI_BOOT
#if defined(BOOT_MODE_BOOT1)
unsigned int sdio_read(u32 start, u32 blkcnt, unsigned long dst)
{
    u32 err = 0;
    u8 *buf = (u8 *) dst;

    err = mmc_bread(0, start, blkcnt, buf);
    if(!err)
    {
        /* always fallback to slow speed in next read (if any) */
        force_sdio_clk_div = 0x80;
        return dst;
    }
    else
    {
        force_sdio_clk_div = 0x80;
        return 0;
    }
}
#endif

#if defined(BOOT2_TEST_SD)
int mmcr_cmd(int argc, char *argv[])
{
    u32 start, blkcnt;
#ifdef CONFIG_MMC_TRACE
    u32 tstart;
#endif
    u32 err = 0;
    u8 *buf = (u8 *) buf_address;
    switch (argc)
    {
        case 3:
            if (!hextoul(argv[2], &buf))
                goto err;
        case 2:
            if (1 != sscanf(argv[1], "%d", &blkcnt))
                goto err;
            if (1 != sscanf(argv[0], "%d", &start))
                goto err;
            break;
        default:
            goto err;
    }

    printf("input: start=%d, blkcnt=%d, buf=0x%x\n", start, blkcnt, buf);

    mini_sdhc_init();

#ifdef CONFIG_MMC_TRACE
    tstart = how_long(0);
#endif
#if 1
    err = mmc_bread(0, start, blkcnt, buf);
#else
    err = host->mmc->block_read(0, start, blkcnt, buf);
#endif
    if (!err)
    {
        printf("cmd OK(%d ms)\n", how_long(tstart));
    }
    else
    {
        printf("cmd FAIL(%d ms)\n", how_long(tstart));
        force_sdio_clk_div = 0x80;
    }

    return ERR_OK;
  err:
    return ERR_HELP;
}

int mmcw_cmd(int argc, char *argv[])
{
    u32 start, blkcnt;
#ifdef CONFIG_MMC_TRACE
    u32 tstart;
#endif
    u32 err = 0;
    u8 *buf = (u8 *) buf_address;
    switch (argc)
    {
        case 3:
            if (!hextoul(argv[2], &buf))
                goto err;
        case 2:
            if (1 != sscanf(argv[1], "%d", &blkcnt))
                goto err;
            if (1 != sscanf(argv[0], "%d", &start))
                goto err;
            break;
        default:
            goto err;
    }

    printf("input: start=%d, blkcnt=%d, buf=0x%x\n", start, blkcnt, buf);

    mini_sdhc_init();

#ifdef CONFIG_MMC_TRACE
    tstart = how_long(0);
#endif

#if 0
    err = mmc_berase(0, start, blkcnt);
    if (!err)
        printf("cmd OK(%d ms)\n", how_long(tstart));
    else
        printf("cmd FAIL(%d ms)\n", how_long(tstart));
#endif

    err = mmc_bwrite(0, start, blkcnt, buf);
    if (!err)
        printf("cmd OK(%d ms)\n", how_long(tstart));
    else
        printf("cmd FAIL(%d ms)\n", how_long(tstart));

    return ERR_OK;
  err:
    return ERR_HELP;
}

cmdt cmdt_mmcw[] __attribute__ ((section("cmdt"))) =
{
    {
    "mmcw", mmcw_cmd, "mmcw <start> <blkcnt> <src> ;mmc write block"}
,};

cmdt cmdt_mmcr[] __attribute__ ((section("cmdt"))) =
{
    {
    "mmcr", mmcr_cmd, "mmcr <start> <blkcnt> <dst> ;mmc read block"}
,};
#endif

#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
#pragma GCC optimize ("O0")
static int idle_cycle = 0, mode = 0;
#endif
#if defined(MASS_PRODUCTION_TEST)
void st_thread(void *param)
{
    struct sd_dev *dev = (struct sd_dev *)param;
    static u8 rbuf_address[2*MMC_MAX_BLOCK_LEN];
    static u8 wbuf_address[2*MMC_MAX_BLOCK_LEN];
    u8 *rbuf = (u8 *)uncached_addr(rbuf_address);
    u8 *wbuf = (u8 *)uncached_addr(wbuf_address);
    u32 start = 0, blkcnt = 2;
    u32 end = 256;
    u32 err = 0;
    u32 pass = 1;
    u32 i;
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
    int cycle;
#endif

    for(i=0;i<sizeof(wbuf_address);i++) {
        wbuf[i] = rand() % 256;
    }

#if defined(CONFIG_SCHED)
    while (dev->run)
#else
    for(i=0; i<128 ; i++)
#endif
    {
        if((start + blkcnt) >= end)
            start = 0;

        pass = 1;

#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
	if(mode == 1)
	{
		blkcnt = 1;
		memset((char *)rbuf, 0, sizeof(rbuf_address));
		err = mmc_bread(0, start, blkcnt, rbuf);
		if (err) {
			dev->rcnt++;
			pass = 0;
		}

		err = mmc_bread(0, start, blkcnt, rbuf);
		if (err) {
			dev->rcnt++;
			pass = 0;
		}

		err = mmc_bread(0, start, blkcnt, rbuf);
		if (err) {
			dev->rcnt++;
			pass = 0;
		}
		for(cycle=0;cycle<idle_cycle;cycle++)
			err = cycle;
	}
	else if(mode == 2)
	{
		blkcnt = 1;
		err = mmc_bwrite(0, start, blkcnt, wbuf);
		if (err) {
			dev->wcnt++;
			pass = 0;
		}

		err = mmc_bwrite(0, start, blkcnt, wbuf);
		if (err) {
			dev->wcnt++;
			pass = 0;
		}

		err = mmc_bwrite(0, start, blkcnt, wbuf);
		if (err) {
			dev->wcnt++;
			pass = 0;
		}
		for(cycle=0;cycle<idle_cycle;cycle++)
			err = cycle;
	}
	else
	{
		if(mode == 3)
			blkcnt = 1;
		err = mmc_bwrite(0, start, blkcnt, wbuf);
		if (err) {
			dev->wcnt++;
			pass = 0;
		}

		memset((char *)rbuf, 0, sizeof(rbuf_address));
		err = mmc_bread(0, start, blkcnt, rbuf);
		if (err) {
			dev->rcnt++;
			pass = 0;
		}
		if(mode != 3)
		{
			if (memcmp(wbuf, rbuf, sizeof(wbuf_address))) {
				dev->ecnt++;
				pass = 0;
			}
		}
		for(cycle=0;cycle<idle_cycle;cycle++)
			err = cycle;
	}
#else
        err = mmc_bwrite(0, start, blkcnt, wbuf);
        if (err) {
            dev->wcnt++;
            pass = 0;
        }

        memset((char *)rbuf, 0, sizeof(rbuf_address));
        err = mmc_bread(0, start, blkcnt, rbuf);
        if (err) {
            dev->rcnt++;
            pass = 0;
        }

        if (memcmp(wbuf, rbuf, sizeof(wbuf_address))) {
            dev->ecnt++;
            pass = 0;
        }
#endif

        if (pass)
            dev->pcnt++;
        dev->cnt++;

        start += blkcnt;

    }
#if defined(CONFIG_SCHED)
    thread_exit();
#endif
    return;
}

#if defined(CONFIG_SCHED)
#define ST_TEST_THREAD_STACK_SIZE  (128*1024)
unsigned char st_test_thread_stack[ST_TEST_THREAD_STACK_SIZE];
#endif

void st_show_result(struct sd_dev *dev)
{
#undef printf
    printf("Total: %u\n", dev->cnt);
    printf("Pass:  %u\n", dev->pcnt);
    printf("Fail:  %u\n", (dev->cnt - dev->pcnt));
    printf("read    error count: %u\n", dev->rcnt);
    printf("write   error count: %u\n", dev->wcnt);
    printf("compare error count: %u\n", dev->ecnt);
#define printf(A, ...)
}

int st_cmd(int argc, char *argv[])
{
    struct sd_dev *dev = &sd_devx;

    if (argc >= 1)
    {
        if (!strcmp(argv[0], "start")) {
            memset((char *)dev, 0, sizeof(struct sd_dev));;
            if (dev->init)
                goto exit;

            mini_sdhc_init();
            /* create a thread */
            dev->init = 1;
            dev->run = 1;
#if defined(CONFIG_SCHED)
            thread_create(st_thread, (void *)dev,
                   &st_test_thread_stack[ST_TEST_THREAD_STACK_SIZE], ST_TEST_THREAD_STACK_SIZE);
#else
            st_thread((void *)dev);
#endif
            goto exit;
        }
        else if (!strcmp(argv[0], "stop")) {

            if (!dev->init) {
                printf ("please `st start` first\n");
            }
            else {
                dev->run = 0;
                dev->init = 0;
            }
            goto exit;
        }
        else if (!strcmp(argv[0], "stat")) {
            st_show_result(dev);
            goto exit;
        }
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
        else if (!strcmp(argv[0], "ddr_test")) {
            if (argc != 3)
                printf("st ddr_test [r/w/rw] [idle cycle]\n");
            if (strcmp(argv[1], "r") == 0)
            {
                mode = 1;
                idle_cycle = atoi(argv[2]);
                printf("start ddr_test idle_cycle: %d, (mode %d) read pattern\n", idle_cycle, mode);
            }
            else if (strcmp(argv[1], "w") == 0)
            {
                mode = 2;
                idle_cycle = atoi(argv[2]);
                printf("start ddr_test idle_cycle: %d, (mode %d) write pattern\n", idle_cycle, mode);
            }
            else if (strcmp(argv[1], "rw") == 0)
            {
                mode = 3;
                idle_cycle = atoi(argv[2]);
                printf("start ddr_test idle_cycle: %d, (mode %d) read/write pattern\n", idle_cycle, mode);
            }
            else if (strcmp(argv[1], "n") == 0)
            {
                mode = 0;
                idle_cycle = 0;
            }
            return 0;
        }
#endif
    }
    return ERR_HELP;

exit:
    return ERR_OK;
}

cmdt cmdt_st[] __attribute__ ((section("cmdt"))) =
{
    {
        "st", st_cmd, "st; SDHC TEST\n"
                    "\tstart : start to test\n"
                    "\tstop  : stop to test\n"
                    "\tstat  : show result\n"}
,};
#endif

#endif
#endif
#endif
