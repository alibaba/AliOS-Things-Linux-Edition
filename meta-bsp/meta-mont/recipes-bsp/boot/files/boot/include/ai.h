/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file ai.h
*   \brief Audio Inteface API
*   \author Montage
*/
#ifndef _AI_H
#define _AI_H

#define CFG             (0x00)
#define CH_TXBASE       (0x04)
#define CH0_RXBASE      (0x08)
#define CH1_RXBASE      (0x0c)
#define CONF_FSSYNC     (0x10)
#define CH0_CONF        (0x14)
#define CH1_CONF        (0x18)
#define TX_DUMMY        (0x1c)
#define NOTIFY          (0x20)
#define STATE_MACH      (0x24)
#define INTR_STATUS     (0x28)
#define INTR_MASK       (0x2C)
#define INTR_CLR        (0x30)
#define SPDIF_CONF      (0x34)
#define SPDIF_CH_STA    (0x38)
#define SPDIF_CH_STA_1  (0x3C)
#define AUDIO_DMA_ASYNC (0x40)
#define DAC_CONF_REG    (0x44)
#define ADC_CONF_REG    (0x48)
#define TEST_MODE_REG   (0x4C)

#define DES_OWN         (1<<31) //Owner bit 1:SW 0:HW
#define DES_EOR         (1<<30) //End of Ring
#define DES_SKP         (1<<29) //Skip compensated data
#define DES_HWD         (1<<28) //Hw haneld
#define BUF_SIZE_SHIFT  16      //buffer size shift
#define LENG_MASK       (0x07ff0000)    //Mask for buffer length
#define CNT_MASK        (0x0000ffff)    //Mask for UNRUN_COUNT/OVRUN_COUNT

#define NOTIFY_TX_BIT(ch) (ch * 16)     //0*16 or 1*16
#define NOTIFY_RX_BIT(ch) (ch * 16 + 1) //0*16+1 or 1*16+1

#define CH_TX       0
#define CH_RX       1
#define CH_RX_ADC   2
#define NUM_DIR     3           // CH_TX, CH_RX
#define DES_RING_BUFFER_SIZE 16

#define AI_CH0 0
#define AI_CH1 1
#define NUM_CH 2

#define AI_BUF_UNIT         32
#define AI_MAX_DES_BUF      0x000007ff  // 2047
#define AI_MAX_BUFSIZE      (AI_MAX_DES_BUF*AI_BUF_UNIT)        // AI_MAX_DES_BUF * AI_BUF_UNIT
#define AI_TOTAL_BUF_SIZE   (AI_MAX_BUFSIZE*DES_RING_BUFFER_SIZE)

#define IF_PCM      0
#define IF_I2S      1
#define IF_SPDIF    2
#define IF_DAC      3
#define IF_ADC      4
#define IF_NUM      5

#define CHECK_LOOP 10000000

#define DEFAULT_TX_SA_ADDR 0xa0140000
#define DEFAULT_RX_DA_ADDR 0xa0500000
#define DEFAULT_TXRX_LEN   0x10000

#define BUS_8_BIT       0
#define BUS_16_BIT      1
#define BUS_16_BIT_WB   2
#define BUS_24_BIT      3

struct ai_des
{
    unsigned int info;
    unsigned int dptr;
};

struct ai_dev
{
//    int if_mode;        // 0:PCM 1:I2S
/* common register */
    int dma_mode;
    int ctrl_mode;              // 0:Master 1:Slave
/* PCM register */
    int fs_polar;               // 0:Low 1:High Active
    int fs_long_en;             // Long Frame sync enable
    int fs_interval;            // frame sync interval
    int fs_rate;                // 0:8k  1:16k  2:32k
    int slot_mode;              // 0:256KHz 1:512KHz 2:1024KHz 3:2048kHz 4:4096kHz 5:8192kHz
    int pcm_mclk;               // 0~7 perform different clock (256kHz ~ 24.576MHz) 
    int rx_bclk_latch;          // 0:Negative 1:Positive edge
/* I2S register */
    int swapch;                 // 0:Left->Right 1:Right->Left
    int txd_align;              // 0:failling edge 1:rising edge
/* operation information */
    int test_ch;
};

struct ai_ch
{
/* register setting */
    unsigned int loopback_ext;  // external loopback
    unsigned int loopback_int;  // internal loopback
    unsigned int bus_mode;
    unsigned int tristate;      // dtx_float
    unsigned int order;         // bit order
    unsigned int slot;          // slot id
    unsigned int rx_dma_en;     // enable rx dma
    unsigned int tx_dma_en;     // enable tx dma
    unsigned int enable;        // enable channel
/* operation information */
    unsigned int ch_no;         // channel no
};

struct tx_des_info
{
    unsigned int txnext;        // txdes used index
    struct ai_des *txd;
    unsigned int tx_saddr;
    unsigned int tx_len;
    unsigned int tx_ptr;
};

typedef int (*RX_SW_CALLBACK) (void *);

struct rx_des_info
{
    unsigned int rxnext;        // rxdes used index
    unsigned int rxdnext;       // rxdes hw done index
    unsigned int rxhwdes;       // how many des used to hw
    struct ai_des *rxd;
    RX_SW_CALLBACK rx_sw_cb[DES_RING_BUFFER_SIZE];
    unsigned int rx_daddr;
    unsigned int rx_len;
    unsigned int rx_ptr;
};

struct rx_state_info
{
    unsigned char * rx_addr;
    unsigned char * ori_addr;
    unsigned int total_len;
    unsigned int rx_idx;
};

struct spdif_status
{
    unsigned int data_width;
    unsigned int channel_num;
    unsigned int sampling_freq;
    unsigned int clcok_acc;
    unsigned int max_samp_len;
    unsigned int samp_word_len;
    unsigned int org_samp_rate;
    unsigned int pro_con;
    unsigned int audio;
    unsigned int copy_copyright;
    unsigned int pre_emph;
    unsigned int chan_stat_mode;
    unsigned int category_cod;
    unsigned int gen_status;
    unsigned int source_num;
    unsigned int mute;
};

void ai_disable_module();
void ai_init(int if_mode, int buf_size);
void ai_dev_init();

// about setting dev
enum
{
    DEV_CTRL_MODE = 0,
    DEV_FS_POLAR,
    DEV_FS_LOGN_EN,
    DEV_FS_INTERVAL,
    DEV_FS_RATE,
    DEV_SLOT_MODE,
    DEV_DMA_METHOD,
    DEV_BCLK_LATCH,
    DEV_PCM_MCLK,
    // about I2S
    DEV_I2S_CH_SWAP,
    DEV_I2S_TXD_ALIGN,
    DEV_CMD_NUM
};

int aidev_setting(int type, int value);

// about setting ch
enum
{
    CH_TX_DMA_EN = 0,
    CH_RX_DMA_EN,
    CH_SLOT_ID,
    CH_BIT_ORDER,
    CH_DTX_FLOAT,
    CH_BUS_MODE,
    CH_LOOPBACK_INTERNAL,
    CH_LOOPBACK_EXT,
    CH_CMD_NUM
};

int aich_setting(int ch, int type, int value);

int ai_test_internal_loopback(unsigned int ch, unsigned int delay,
                              int bus_mode);
void test_pcm_internal_loopback();

void spdif_enable(int enable);
void spdif_mute();
void set_audio_dma_async(int async);
//void init_spdif(int buf_size);

enum
{
    SPDIF_DATA_WIDTH = 0,
    SPDIF_CH_NUM,
    SPDIF_SAMPLING_FREQ,
    SPDIF_CLCOK_ACC,
    SPDIF_MAX_SAMP_LEN,
    SPDIF_SAMP_WORD_LEN,
    SPDIF_ORG_SAMP_RATE,
    SPDIF_PRO_CON,
    SPDIF_AUDIO,
    SPDIF_COPY_COPYRIGHT,
    SPDIF_PRE_EMPH,
    SPDIF_CH_STAT_MODE,
    SPDIF_CATEGORY_COD,
    SPDIF_GEN_STATUS,
    SPDIF_SOURCE_NUM,
    SPDIF_CMD_NUM
};

/* call it before set_spdif_ch_status */
int spdif_status_setting(int type, int value);

void set_spdif_ch_status();

int ai_test_spdif_tx();         // only dual channel

int ai_test_dac_tx();           // only dual channel

int ai_pcm_test_tx_tone();
int ai_i2s_test_tx_tone();

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#ifndef CONFIG_FPGA
#define PMUENABLEREG 0x4A58 // enable pcm(bit7), spdif (bit8)
#define PMURESETREG  0x4A60 // reset pcm(bit8), spdif(bit9)
#define AI_GPIO_SEL0    0x4A18UL   // set 3 with spdif msk
 #define SPDIF_MSK    0x000F0000UL
#define AI_GPIO_SEL1    0x4A1CUL   // set 5 with each msk
 #define I2S_TXD_MSK  0x00000F00UL
 #define I2S_RXD_MSK  0x0000F000UL
 #define I2S_MCLK_MSK 0x0F000000UL
 #define I2S_BCLK_MSK 0xF0000000UL
#define AI_GPIO_SEL2    0x4A20UL
 #define I2S_LRC_MSK  0x0000000FUL
#endif

#endif
