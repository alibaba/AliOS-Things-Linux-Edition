/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file ai.c
*   \brief Audio Inteface Driver
*   \author Montage
*/
#ifdef CONFIG_AI
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <string.h>
#include <arch/chip.h>
#include <arch/irq.h>
#include <common.h>
#include <ai.h>
#ifndef CONFIG_ATE
#include <ai_tone.h>
#endif
#include "str/str.h"

// for dump memory to a file in usb0
#include "fatfs/ff.h"
#include <part.h>
#ifdef CONFIG_SCHED
#include <sched.h>
#if defined(CONFIG_SCHED)
#define AI_TEST_THREAD_STACK_SIZE  (128*1024)
unsigned char ai_test_thread_stack[AI_TEST_THREAD_STACK_SIZE];
volatile static int i2sdac = 0;
volatile static int adci2s = 0;
#endif
#endif

extern int hextoul(char *str, void *v);
extern int tstc(void);
extern int sprintf(char *buf, char *fmt, ...);
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#if defined(VERIFICATION_FRAMEWORK)
#define IN_QRAM  __attribute__((section(".phyram")))
#define IN_SRAM  __attribute__((section(".phyram")))
#else
#define IN_QRAM
#define IN_SRAM
#endif

#define PHYSICAL_ADDR(va)   (((unsigned int)va)&0x1fffffff)
#define UNCACHED_ADDR(pa)   (((unsigned int)pa)|0xa0000000)
#define CACHED_ADDR(pa)     (((unsigned int)pa)|0x80000000)

#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

#define INTR_TX         BIT1
#define INTR_RX         BIT0
#define INTR_RX_ADC     BIT2

#define AI_DUMMY_TONE   0xEEEE

#define CFGREG(val, mask) tmp = ((tmp&~mask)|val)

#define AI_DEBUG 1

#if AI_DEBUG
/* show more information to debug */
#define dp(format, args...) \
do { \
    printf("[%s:%d](%s)"format"\n", __FILE__, __LINE__, __func__, ##args); \
} while (0)
#else
#define dp(format, args...)
#endif

#define AI_CLOSE_IF()                     \
do {                                      \
    AIREG(SPDIF_CONF)   = 0x00000000UL;   \
    AIREG(CH0_CONF)     = 0x00000400UL;   \
    AIREG(CH1_CONF)     = 0x00000400UL;   \
    AIREG(CFG)          = 0x002800e4UL;   \
    AIREG(DAC_CONF_REG) = 0x0001f9f4UL;   \
    AIREG(ADC_CONF_REG) = 0x00000000UL;   \
} while(0)

#define AI_RESET_HW()       \
do {                        \
    __REG_UPDATE32(PMURESETREG, 0, BIT8);     \
    __REG_UPDATE32(PMURESETREG, BIT8, BIT8);  \
    __REG_UPDATE32(PMURESETREG, 0, BIT9);     \
    __REG_UPDATE32(PMURESETREG, BIT9, BIT9);  \
} while(0)

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

static struct tx_des_info tx_des_state = {
  tx_saddr:DEFAULT_TX_SA_ADDR,
  tx_len:DEFAULT_TXRX_LEN,
};

static struct rx_des_info rx_des_state = {
  rx_daddr:DEFAULT_RX_DA_ADDR,
  rx_len:(DEFAULT_TXRX_LEN * 8),
};

static struct rx_state_info rx_des_state_totx;

static struct rx_des_info adc_rx_des_state = {
  rx_daddr:DEFAULT_RX_DA_ADDR,
  rx_len:(DEFAULT_TXRX_LEN * 8),
};

static struct ai_des aides[NUM_DIR][DES_RING_BUFFER_SIZE]
IN_QRAM __attribute__ ((aligned(0x4)));
static unsigned char BUF[NUM_DIR][DES_RING_BUFFER_SIZE][AI_MAX_BUFSIZE]
IN_QRAM __attribute__ ((aligned(0x4)));
static struct ai_dev *aidev;

static int des_bufsize = 1;     // 1 unit is 32byte
static int real_bufsize;        // des_bufsize * AI_BUF_UNIT

// test data
static int test_if_mode;
static int test_bus_mode;

#define TEST_UNIT 20
#define test_raw_len (AI_BUF_UNIT * TEST_UNIT)
unsigned char test_raw[test_raw_len] = { 0 };

/*=============================================================================+
| PCM module define                                                            |
+=============================================================================*/
struct ai_dev pcmdev = {
//    if_mode      : IF_PCM,//PCM
  dma_mode:0,
  slot_mode:3,
  ctrl_mode:0,
  fs_interval:1,
  fs_rate:2,
  fs_long_en:1,
  fs_polar:1,
  pcm_mclk:5,
  rx_bclk_latch:0,
};

/* default I2S Module setting */
struct ai_dev i2sdev = {
//    if_mode      : IF_I2S,//I2S
  dma_mode:0,
  ctrl_mode:0,
  swapch:0,
  txd_align:0,
};

/* default PCM channel setting */
struct ai_ch all_ch[IF_NUM][NUM_CH] = {
    {                           /* PCM */
     {
          loopback_ext:0,
          loopback_int:0,
          bus_mode:0,          //8bit
          tristate:0,
          order:1,             //MSB
          slot:0,
          rx_dma_en:1,
          tx_dma_en:1,
          enable:1,
          ch_no:0,             //channel 0
      },
     {
          loopback_ext:0,
          loopback_int:0,
          bus_mode:0,          //8bit
          tristate:0,
          order:1,             //MSB
          slot:2,
          rx_dma_en:1,
          tx_dma_en:1,
          enable:1,
          ch_no:1,             //channel 1
      }
     },
    {                           /* I2S */
     {
          loopback_ext:0,
          loopback_int:0,
          bus_mode:1,          //16bit
          tristate:0,
          order:1,             //MSB
          slot:0,
          rx_dma_en:1,
          tx_dma_en:1,
          enable:1,
          ch_no:0,             //channel 0
      },
     {
          loopback_ext:0,
          loopback_int:0,
          bus_mode:1,          //16bit
          tristate:0,
          order:1,             //MSB
          slot:0,
          rx_dma_en:1,
          tx_dma_en:1,
          enable:1,
          ch_no:1,             //channel 1
      },
     },
    {                           /* SPDIF */
     {
          rx_dma_en:1,
          tx_dma_en:1,
          enable:1,
          ch_no:0,             //channel 0
      },
     {
          rx_dma_en:1,
          tx_dma_en:1,
          enable:1,
          ch_no:1,             //channel 1
      },
     },
    {                           /* DAC only tx */
     {
          rx_dma_en:0,
          tx_dma_en:1,         // must enable this
          enable:1,
          ch_no:0,             //channel 0
      },
     {
          rx_dma_en:0,
          tx_dma_en:1,         // must enable this
          enable:1,
          ch_no:1,             //channel 1
      },
     },
    {                           /* ADC only rx */
     {
          rx_dma_en:0,
          tx_dma_en:1,         // enable this for adda
          enable:1,
          ch_no:0,             //channel 0
      },
     {
          rx_dma_en:0,
          tx_dma_en:1,         // enable this for adda
          enable:1,
          ch_no:1,             //channel 1
      },
     }
};

struct spdif_status spdif_dev = {
  data_width:0,
  channel_num:0,
  sampling_freq:8,
  clcok_acc:0,
  max_samp_len:0,
  samp_word_len:0,
  org_samp_rate:0,
  pro_con:0,
  audio:0,
  copy_copyright:0,
  pre_emph:0,
  chan_stat_mode:0,
  category_cod:0,
  gen_status:0,
  source_num:0,
  mute:0,
};

static int wait_isr = 0;

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void spdif_enable(int enable);
void loopback_dacadc();
void enable_dac(int enable);
void enable_adc(int enable);
void dump_data(int bus_mode, unsigned int size, void *addr);

inline void ai_pmu_set()
{
#ifndef CONFIG_FPGA
    //__REG_WRITE32(0x00004c80, 0x00008d07);
    //__REG_WRITE32(0x00004c84, 0x0db6eaa1);
    //__REG_WRITE32(0x00004c7c, 0xbe4477d0);
    dp("ai_pmu_set");
    // initial pcm hareware
    //__REG_UPDATE32(PMUENABLEREG, 1 << 7, BIT7);   // enable pcm
    __REG_UPDATE32(AI_GPIO_SEL0, 3 << 16, SPDIF_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 8, I2S_TXD_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 12, I2S_RXD_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 24, I2S_MCLK_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 28, I2S_BCLK_MSK);
    __REG_UPDATE32(AI_GPIO_SEL2, 5, I2S_LRC_MSK);
#endif
}

inline void ai_pmu_dacadc(void)
{
#ifndef CONFIG_FPGA
    // initial pcm hareware
    __REG_UPDATE32(PMUENABLEREG, 1 << 7, BIT7);   // enable pcm
    __REG_UPDATE32(AI_GPIO_SEL0, 3 << 16, SPDIF_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 8, I2S_TXD_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 12, I2S_RXD_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 24, I2S_MCLK_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 28, I2S_BCLK_MSK);
    __REG_UPDATE32(AI_GPIO_SEL2, 5, I2S_LRC_MSK);

    // initial dac for 48k
    //__REG_WRITE32(0x48F8, 0x00000001);
    __REG_WRITE32(0x4C94, 0x00000020);
    __REG_WRITE32(0x4C98, 0x55428E00);
    __REG_WRITE32(0x4C9C, 0x03040200);
    __REG_WRITE32(0x4CA0, 0x01B2000F);
    __REG_WRITE32(0x4C7C, 0xBE4247D0);
    __REG_WRITE32(0x4C98, 0x5540A555);
#endif
}

inline void ai_pmu_enable()
{
    __REG_UPDATE32(PMUENABLEREG, 1 << 7, BIT7);   // enable pcm
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 8,  I2S_TXD_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 12, I2S_RXD_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 24, I2S_MCLK_MSK);
    __REG_UPDATE32(AI_GPIO_SEL1, 5 << 28, I2S_BCLK_MSK);
    __REG_UPDATE32(AI_GPIO_SEL2, 5, I2S_LRC_MSK);
}

inline void ai_pmu_enable_dac()
{
    if(chip_revision >= 2)
    {
        __REG_UPDATE32(0x4a5c, BIT8, BIT8);
    }
    else
    {
        __REG_UPDATE32(0x48f8, 1, 1);
    }
}

inline void ai_pmu_disable_dac()
{
    if(chip_revision >= 2)
    {
        __REG_UPDATE32(0x4a5c, 0, BIT8);
    }
    else
    {
        __REG_UPDATE32(0x48f8, 0, 1);
    }
}

inline void ai_pmu_enable_adc()
{
    if(chip_revision >= 2)
    {
        __REG_UPDATE32(0x4a5c, BIT7, BIT7);
    }
    else
    {
        __REG_UPDATE32(0x48f8, 1, 1);
    }
}

inline void ai_pmu_disable_adc()
{
    if(chip_revision >= 2)
    {
        __REG_UPDATE32(0x4a5c, 0, BIT7);
    }
    else
    {
        __REG_UPDATE32(0x48f8, 0, 1);
    }
}

inline void ai_des_no_inc(unsigned int *no)
{
    (*no)++;
    if (DES_RING_BUFFER_SIZE == (*no))
        (*no) = 0;
}

static void intr_handle_rx(int is_adc)
{
    //dp("intr_handle_rx %d\n", is_adc);

    struct ai_des *rx_des, *rx_base_des;
    struct rx_des_info *current_rx_state;
    if (1 == is_adc)
    {
        current_rx_state = &adc_rx_des_state;
    }
    else
    {
        current_rx_state = &rx_des_state;
    }
    rx_base_des = (struct ai_des *) UNCACHED_ADDR(current_rx_state->rxd);
    int current_rxd = current_rx_state->rxdnext;
    int deal_count = 0;
    while(deal_count < current_rx_state->rxhwdes)
    {
        rx_des = rx_base_des + current_rxd;
        if (rx_des->info & DES_OWN)
        {
            current_rx_state->rx_sw_cb[current_rxd] ((void *)
                                           UNCACHED_ADDR(rx_des->dptr));
            ai_des_no_inc(&current_rx_state->rxdnext);
        } else {
            break;
        }
        current_rxd = current_rx_state->rxdnext;
        deal_count++;
    }
    current_rx_state->rxhwdes -= deal_count;

    #if 0
    rx_des = rx_base_des + current_rxd;
    if (rx_des->info & DES_OWN)
    {
        current_rx_state->rx_sw_cb[current_rxd] ((void *)
                                           UNCACHED_ADDR(rx_des->dptr));
        ai_des_no_inc(&current_rx_state->rxdnext);
        current_rx_state->rxhwdes -= 1;
    }
    #endif

}

static void intr_handle_tx()
{
    // no need to handle tx
    //printf("tx\n");
}

void ai_intr_handler(void)
{
    unsigned int status;
    status = AIREG(INTR_STATUS);
    //printf("interrupt status = %d\n", status);
    //dp("interrupt status = %d", status);
    //dp("clear = %d", AIREG(INTR_CLR));
    AIREG(INTR_CLR) = status;   // clear status
    if (status & INTR_TX)
    {
        // tx finished, change DES_OWN to SW
        intr_handle_tx();
    }
    int is_adc = 0;
    if (status & INTR_RX)
    {
        // rx finished, change DES_OWN to SW
        intr_handle_rx(is_adc);
    }
    if (status & INTR_RX_ADC)
    {
        is_adc = 1;
        intr_handle_rx(is_adc);
    }

    if(1 == wait_isr)
    {
        wait_isr = 0;
    }
}

void ai_disable_module()
{
    free_irq(IRQ_PCM, (void *) IRQ_PCM);
    AI_CLOSE_IF();
    AI_RESET_HW();
    ai_pmu_disable_adc();
}

void ai_des_init()
{
    //dp("<Reset Audio Interface Descriptor>");
    int no, dir;
    struct ai_des *des;
    for (dir = 0; dir < NUM_DIR; dir++)
    {
        /* init descriptors */
        for (no = 0; no < DES_RING_BUFFER_SIZE; no++)
        {
            des = (struct ai_des *) UNCACHED_ADDR(&aides[dir][no]);
            /* des own bit setting */
            des->info = DES_OWN;
            //dp("des_bufsize %d", des_bufsize);
            /* des buf setting */
            des->info |= (LENG_MASK & (des_bufsize << BUF_SIZE_SHIFT));
            des->dptr = PHYSICAL_ADDR(&BUF[dir][no]);
            //dp("des->dptr %x", des->dptr);
        }
        /* last descriptor is the end of ring descriptor */
        des->info |= DES_EOR;
        //dp("des->info %x", des->info);
        printf("des->info %x\n", des->info);
        /* init descriptor base register */
        AIREG(CH_TXBASE + dir * 4) = PHYSICAL_ADDR(&aides[dir]);
        //dp("AIREG(CH_TXBASE+%d) %x\n", dir, AIREG(CH_TXBASE+dir*4));
        printf("AIREG(CH_TXBASE+%d) %x\n", dir, AIREG(CH_TXBASE + dir * 4));
    }
}

// for ai_main_init
void ai_des_link()
{
    printf("ai_des_link\n");
    // tx
    tx_des_state.txd = (struct ai_des *) PHYSICAL_ADDR(&aides[CH_TX]);
    tx_des_state.txnext = 0;
    // rx
    rx_des_state.rxd = (struct ai_des *) PHYSICAL_ADDR(&aides[CH_RX]);
    rx_des_state.rxnext = 0;
    rx_des_state.rxdnext = 0;
    rx_des_state.rxhwdes = 0;
    // rx adc
    adc_rx_des_state.rxd = (struct ai_des *) PHYSICAL_ADDR(&aides[CH_RX_ADC]);
    adc_rx_des_state.rxnext = 0;
    adc_rx_des_state.rxdnext = 0;
    adc_rx_des_state.rxhwdes = 0;
    printf("ai_des_link done\n");
}

void ai_dev_init()
{
    ai_des_link();              // initialize descriptors

    int tmp = 0;                // for CFGREG
    struct ai_dev *dev = aidev;

    /* common configuration */
    CFGREG((dev->dma_mode << 14), BIT14);
    CFGREG((dev->ctrl_mode << 1), BIT1);
    /* PCM configuration */
    if (test_if_mode == IF_PCM)
    {
        dp("<PCM configuration>\n");
        dp("slot_mode %d, fs_rate %d, pcm_mclk %d", dev->slot_mode,
           dev->fs_rate, dev->pcm_mclk);

        CFGREG((dev->pcm_mclk << 19), (BIT21 | BIT20 | BIT19));
        CFGREG((dev->rx_bclk_latch << 15), BIT15);
        CFGREG((dev->slot_mode << 11), (BIT13 | BIT12 | BIT11));
        CFGREG((dev->fs_rate << 9), (BIT10 | BIT9));
        CFGREG((dev->fs_interval << 5), (BIT8 | BIT7 | BIT6 | BIT5));
        CFGREG((dev->fs_long_en << 3), BIT3);
        CFGREG((dev->fs_polar << 2), BIT2);
        CFGREG((1 << 0), BIT0); //enable module
    }
    /* I2S configuration */
    else if (test_if_mode == IF_I2S)
    {
        dp("I2S configuration\n");

        CFGREG((dev->txd_align << 18), BIT18);
        CFGREG((dev->swapch << 17), BIT17);
        CFGREG((1 << 16), BIT16);       //enable I2S module
    }
    AIREG(CFG) = tmp;
    dp("<Enable Audio Interface Module> %x", tmp);
}

void ai_init(int if_mode, int buf_size)
{
    if (if_mode >= IF_NUM)
    {
        if_mode = IF_PCM;
    }

    if (buf_size > AI_MAX_DES_BUF)
    {
        buf_size = AI_MAX_DES_BUF;
    }

    /* only set dev while IF_I2S and IF_PCM */
    if (IF_I2S == if_mode)
        aidev = &i2sdev;
    else if (IF_PCM == if_mode)
        aidev = &pcmdev;

    test_if_mode = if_mode;
    int tmp = 0;
    AIREG(INTR_MASK) = tmp;     // unmask for getting interrupt RX, TX, ADC_RX

#if defined(SIM)
    int_add(PCM_INTR_NUM, ai_intr_handler);
#else
    request_irq(IRQ_PCM, &ai_intr_handler, (void *) IRQ_PCM);
#endif

    /* reset audio interface module */
    des_bufsize = buf_size;
    real_bufsize = des_bufsize * AI_BUF_UNIT;

    ai_des_init();
}

void spdif_enable(int enable)
{
    if (enable)
    {
        AIREG(SPDIF_CONF) |= BIT0;
    }
    else
    {
        AIREG(SPDIF_CONF) &= 0xFFFFFFFD;
    }
}

void set_spdif_ch_status()
{
    int tmp = 0;
    /* SPDIF Channel status */
    CFGREG((spdif_dev.data_width << 18), (BIT19 | BIT18));
    CFGREG((spdif_dev.channel_num << 14), (BIT17 | BIT16 | BIT15 | BIT14));
    CFGREG((spdif_dev.sampling_freq << 10), (BIT13 | BIT12 | BIT11 | BIT10));
    CFGREG((spdif_dev.clcok_acc << 8), (BIT9 | BIT8));
    CFGREG((spdif_dev.max_samp_len << 7), BIT7);
    CFGREG((spdif_dev.samp_word_len << 4), (BIT6 | BIT5 | BIT4));
    CFGREG((spdif_dev.org_samp_rate), (BIT3 | BIT2 | BIT1 | BIT0));
    AIREG(SPDIF_CH_STA) = tmp;

    /* SPDIF Channel status 1 */
    tmp = 0;
    CFGREG((spdif_dev.pro_con << 19), BIT19);
    CFGREG((spdif_dev.audio << 18), BIT18);
    CFGREG((spdif_dev.copy_copyright << 17), BIT17);
    CFGREG((spdif_dev.pre_emph << 14), (BIT16 | BIT15 | BIT14));
    CFGREG((spdif_dev.chan_stat_mode << 12), (BIT13 | BIT12));
    CFGREG((spdif_dev.category_cod << 5),
           (BIT11 | BIT10 | BIT9 | BIT8 | BIT7 | BIT6 | BIT5));
    CFGREG((spdif_dev.gen_status << 4), BIT4);
    CFGREG((spdif_dev.source_num), (BIT3 | BIT2 | BIT1 | BIT0));
    AIREG(SPDIF_CH_STA_1) = tmp;
}

void loopback_dacadc()
{
    AIREG(TEST_MODE_REG) |= BIT1;
}

void enable_dac(int enable)
{
    if (enable)
    {
        AIREG(DAC_CONF_REG) |= BIT0;
    }
    else
    {
        AIREG(DAC_CONF_REG) &= 0xFFFFFFFE;
    }
}

void enable_adc(int enable)
{
    if (enable)
    {
        AIREG(ADC_CONF_REG) |= BIT0;
    }
    else
    {
        AIREG(ADC_CONF_REG) &= 0xFFFFFFFE;
    }
}

/*=============================================================================+
| Functions for command                                                        |
+=============================================================================*/
void ai_ch_init(struct ai_ch *ch)
{
    int tmp = 0;
    CFGREG((ch->loopback_ext << 15), BIT15);
    CFGREG((ch->loopback_int << 14), BIT14);
    CFGREG((ch->bus_mode << 12), (BIT13 | BIT12));
    CFGREG((ch->tristate << 11), BIT11);
    CFGREG((ch->order << 10), BIT10);
    CFGREG((ch->slot << 3), (BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3));

    dp("bus_mode %d, rx_dma_en %d, tx_dma_en %d, enable %d", ch->bus_mode,
       ch->rx_dma_en, ch->tx_dma_en, ch->enable);
    AIREG(CH0_CONF + (ch->ch_no * 4)) = tmp;
    dp("<Channel %d Setting> %x", ch->ch_no, tmp);
}

inline void ai_ch_enable(struct ai_ch *ch)
{
    int tmp = 0;
    CFGREG((ch->rx_dma_en << 2), BIT2);
    CFGREG((ch->tx_dma_en << 1), BIT1);
    CFGREG((ch->enable << 0), BIT0);    //enable channel
    AIREG(CH0_CONF + (ch->ch_no * 4)) |= tmp;
    dp("<Channel %d enabled> %x", ch->ch_no, AIREG(CH0_CONF + (ch->ch_no * 4)));
}

inline void notify_hw_tx(int ch)
{
    //dp("notify %d", ch);
    if (ch < NUM_CH)
    {
        AIREG(NOTIFY) |= 1 << (NOTIFY_TX_BIT(ch));
        return;
    }

    int i;
    for (i = 0; i < NUM_CH; i++)
    {
        AIREG(NOTIFY) |= 1 << (NOTIFY_TX_BIT(i));
    }
}

inline void notify_hw_rx(int ch)
{
    //dp("notify %d", ch);
    if (ch < NUM_CH)
    {
        AIREG(NOTIFY) |= 1 << (NOTIFY_RX_BIT(ch));
        return;
    }

    int i;
    for (i = 0; i < NUM_CH; i++)
    {
        AIREG(NOTIFY) |= 1 << (NOTIFY_RX_BIT(i));
    }
}

int ai_rx_poll(RX_SW_CALLBACK callback)
{
    int no = rx_des_state.rxnext;
    //dp("rx_des_state.rxnext = %d", no);

    struct ai_des *rx_des = (struct ai_des *) UNCACHED_ADDR(rx_des_state.rxd);
    rx_des = rx_des + no;

    if (rx_des_state.rxhwdes == DES_RING_BUFFER_SIZE)
    {
        //dp("rx_des_state.rxhwdes %d", rx_des_state.rxhwdes);
        return ERR_MEM;
    }

    /* check descriptor */
    if (rx_des->info & DES_OWN)
    {
        ai_des_no_inc(&rx_des_state.rxnext);    // number increase
        rx_des_state.rxhwdes++;
        //dp("rx_des_state.rxhwdes %d", rx_des_state.rxhwdes);
    }
    else
    {
        return ERR_MEM;
    }

    /* record upper buf address & callback function */
    rx_des_state.rx_sw_cb[no] = callback;

    /* switch own bit to hw */
    rx_des->info &= ~DES_OWN;
    /* trigger hw */
    if (rx_des->info & DES_OWN)
    {
        return ERR_MEM;
    }
    return ERR_OK;
}

int prepare_rx(int ch, RX_SW_CALLBACK callback)
{
    int rc = ai_rx_poll(callback);
    if (ERR_OK != rc)
    {
        //dp("failed to set rx descriptor");
        return rc;
    }
    notify_hw_rx(ch);
    return ERR_OK;
}

int ai_adc_rx_poll(RX_SW_CALLBACK callback)
{
    int no = adc_rx_des_state.rxnext;
    //dp("rx_des_state.rxnext = %d", no);

    struct ai_des *rx_des =
        (struct ai_des *) UNCACHED_ADDR(adc_rx_des_state.rxd);
    rx_des = rx_des + no;

    if (adc_rx_des_state.rxhwdes == DES_RING_BUFFER_SIZE)
    {
        //dp("rx_des_state.rxhwdes %d", rx_des_state.rxhwdes);
        return ERR_MEM;
    }

    /* check descriptor */
    if (rx_des->info & DES_OWN)
    {
        ai_des_no_inc(&adc_rx_des_state.rxnext);        // number increase
        adc_rx_des_state.rxhwdes++;
        //dp("rx_des_state.rxhwdes %d", rx_des_state.rxhwdes);
    }
    else
    {
        return ERR_MEM;
    }

    /* record upper buf address & callback function */
    adc_rx_des_state.rx_sw_cb[no] = callback;

    /* switch own bit to hw */
    rx_des->info &= ~DES_OWN;
    /* trigger hw */
    if (rx_des->info & DES_OWN)
    {
        return ERR_MEM;
    }
    return ERR_OK;
}

int prepare_adc_rx(int ch, RX_SW_CALLBACK callback)
{
    int rc = ai_adc_rx_poll(callback);
    if (ERR_OK != rc)
    {
        dp("failed to set rx descriptor");
        return rc;
    }
    notify_hw_rx(ch);
    return ERR_OK;
}
int prepare_tx(int ch, void *addr, unsigned int size);

int ai_rx_cb_memcpy_to_tx(void *addr)
{
    memcpy((void *) rx_des_state_totx.rx_addr, addr, real_bufsize);

    prepare_tx(2, rx_des_state_totx.rx_addr, real_bufsize);
    rx_des_state_totx.rx_addr += real_bufsize;
    rx_des_state_totx.rx_idx += real_bufsize;
    if (rx_des_state_totx.rx_idx >= rx_des_state_totx.total_len)
    {
        rx_des_state_totx.rx_addr = rx_des_state_totx.ori_addr;
        rx_des_state_totx.rx_idx = 0;
    }
    prepare_rx(2, ai_rx_cb_memcpy_to_tx);
    return 0;
}

int ai_adc_cb_memcpy_to_tx(void *addr)
{
    memcpy((void *) rx_des_state_totx.rx_addr, addr, real_bufsize);

    prepare_tx(2, rx_des_state_totx.rx_addr, real_bufsize);
    rx_des_state_totx.rx_addr += real_bufsize;
    rx_des_state_totx.rx_idx += real_bufsize;
    if (rx_des_state_totx.rx_idx >= rx_des_state_totx.total_len)
    {
        rx_des_state_totx.rx_addr = rx_des_state_totx.ori_addr;
        rx_des_state_totx.rx_idx = 0;
    }
    prepare_adc_rx(2, ai_adc_cb_memcpy_to_tx);
    return 0;
}

int ai_adc_rx_cb_memcpy(void *addr)
{
    dp("rx des addr %x", addr);
    memcpy((void *) adc_rx_des_state.rx_ptr, addr, real_bufsize);
#if 1
    printf("=========rx data========\n");
    dump_data(test_bus_mode, real_bufsize, (void *) adc_rx_des_state.rx_ptr);
    printf("\n");
#endif
    adc_rx_des_state.rx_ptr += real_bufsize;
    adc_rx_des_state.rx_len += real_bufsize;

    return 0;
}

int ai_tx(void *addr, unsigned int size)
{
    int no = tx_des_state.txnext;
    void *buf;
    struct ai_des *tx_des = (struct ai_des *) UNCACHED_ADDR(tx_des_state.txd);
    tx_des = tx_des + no;

    /* check descriptor */
    if (tx_des->info & DES_OWN)
    {
        // software own it
        buf = (void *) UNCACHED_ADDR(tx_des->dptr);
        ai_des_no_inc(&tx_des_state.txnext);
    }
    else
    {
        //dp("tx_des->info & DES_OWN == 0 (cannot use in software yet)");
        return ERR_MEM;
    }
    /* copy upper layer buffer to local aligned buffer and copy to uncached buffer address */
    memcpy(buf, addr, size);
#if 0
    printf("=========tx data========\n");
    dump_data(test_bus_mode, size, buf);
    printf("\n");
#endif

    /* switch own bit to hw */
    tx_des->info &= ~DES_OWN;

    /* trigger hw */
    if (tx_des->info & DES_OWN)
    {
        return ERR_MEM;
    }
    return ERR_OK;
}

int prepare_tx(int ch, void *addr, unsigned int size)
{
    int rc = ai_tx(addr, size);
    if (ERR_OK != rc)
    {
        //dp("failed to set tx descriptor");
        return rc;
    }
    notify_hw_tx(ch);
    return ERR_OK;
}

inline void ai_dummy(unsigned short tone)
{
    AIREG(TX_DUMMY) = tone;
}

void gen_16bit_data(unsigned char *addr, int start_val, int size)
{
    unsigned short *pAddr = (unsigned short *) addr;
    int half_size = size / 2;
    int i;
    for (i = 0; i < half_size; i++)
    {
        pAddr[i] = (start_val + i) & 0x0000ffff;
    }
}

/**********************************************************************************************/

#define PARSING_CMD_DEV(VAR, INPUT, MAX, MSG)   \
do {                                            \
    dp(MSG);                                    \
    if (INPUT > MAX)                            \
        goto err_arg;                           \
    VAR = INPUT;                                \
} while(0)

int aidev_setting(int type, int value)
{
    switch (type)
    {
        case DEV_CTRL_MODE:
            PARSING_CMD_DEV(aidev->ctrl_mode, value, 1,
                            "Set pcm master or slave");
            break;
        case DEV_FS_POLAR:
            PARSING_CMD_DEV(aidev->fs_polar, value, 1,
                            "Set frame sync. polarity");
            break;
        case DEV_FS_LOGN_EN:
            PARSING_CMD_DEV(aidev->fs_long_en, value, 1,
                            "Set long frame sync. (enable)");
            break;
        case DEV_FS_INTERVAL:
            PARSING_CMD_DEV(aidev->fs_interval, value, 15,
                            "Set frame sync interval");
            break;
        case DEV_FS_RATE:
            PARSING_CMD_DEV(aidev->fs_rate, value, 2, "Set sample rate");
            break;
        case DEV_SLOT_MODE:
            PARSING_CMD_DEV(aidev->slot_mode, value, 5, "Set slot mode");
            break;
        case DEV_DMA_METHOD:
            PARSING_CMD_DEV(aidev->dma_mode, value, 1,
                            "Set dma method(Dual channel, FIFO)");
            break;
        case DEV_BCLK_LATCH:
            PARSING_CMD_DEV(aidev->rx_bclk_latch, value, 1,
                            "Set rx bclk latch");
            break;
        case DEV_PCM_MCLK:
            PARSING_CMD_DEV(aidev->pcm_mclk, value, 7, "Set pcm mclk");
            break;
        case DEV_I2S_CH_SWAP:
            PARSING_CMD_DEV(aidev->swapch, value, 1, "Set i2s ch swap");
            break;
        case DEV_I2S_TXD_ALIGN:
            PARSING_CMD_DEV(aidev->txd_align, value, 1, "Set i2s txd align");
            break;
        default:
            goto err_arg;
    }

    return ERR_OK;
  err_arg:
    dp("err_arg");
    return ERR_PARM;
}

#define PARSING_CMD_CH(VAR,INPUT,MAX,MSG)   \
do {                                        \
    dp(MSG);                                \
    if (INPUT > MAX) {                      \
        goto err_arg;                       \
    }                                       \
    VAR = INPUT;                            \
    ch_no++;                                \
    if (ch == 2 && ch_no < 2) {             \
        goto set_ch_loop;                   \
    }                                       \
} while(0)

int aich_setting(int ch, int type, int value)
{
    int ch_no = 0;
    struct ai_ch *aich;
    if (ch < 2)
    {
        ch_no = ch;
    }
  set_ch_loop:
    dp("ch_no %d", ch_no);
    aich = &all_ch[test_if_mode][ch_no];
    switch (type)
    {
        case CH_TX_DMA_EN:
            PARSING_CMD_CH(aich->tx_dma_en, value, 1, "Enable/disable tx_dma");
            break;
        case CH_RX_DMA_EN:
            PARSING_CMD_CH(aich->rx_dma_en, value, 1, "Enable/disable rx_dma");
            break;
        case CH_SLOT_ID:
            PARSING_CMD_CH(aich->slot, value, 0x7f, "Set slot id");
            break;
        case CH_BIT_ORDER:
            PARSING_CMD_CH(aich->order, value, 1, "Set bit order");
            break;
        case CH_DTX_FLOAT:
            PARSING_CMD_CH(aich->tristate, value, 1, "Set dtx float");
            break;
        case CH_BUS_MODE:
            PARSING_CMD_CH(aich->bus_mode, value, 3, "Set bus mode");
            break;
        case CH_LOOPBACK_INTERNAL:
            PARSING_CMD_CH(aich->loopback_int, value, 1,
                           "Enable/Disable loopback internal");
            break;
        case CH_LOOPBACK_EXT:
            PARSING_CMD_CH(aich->loopback_ext, value, 1,
                           "Enable/Disable loopback external");
            break;
        default:
            goto err_arg;
    }

    return ERR_OK;
  err_arg:
    dp("err_arg");
    return ERR_PARM;
}

int spdif_status_setting(int type, int value)
{
    switch (type)
    {
        case SPDIF_DATA_WIDTH:
            PARSING_CMD_DEV(spdif_dev.data_width, value, 2,
                            "Set spdif data width");
            break;
        case SPDIF_CH_NUM:
            PARSING_CMD_DEV(spdif_dev.channel_num, value, 0xF,
                            "Set spdif channel num");
            break;
        case SPDIF_SAMPLING_FREQ:
            PARSING_CMD_DEV(spdif_dev.sampling_freq, value, 0xF,
                            "Set spdif sampling freq");
            break;
        case SPDIF_CLCOK_ACC:
            PARSING_CMD_DEV(spdif_dev.clcok_acc, value, 3,
                            "Set spdif clcok acc");
            break;
        case SPDIF_MAX_SAMP_LEN:
            PARSING_CMD_DEV(spdif_dev.max_samp_len, value, 1,
                            "Set spdif max sample len");
            break;
        case SPDIF_SAMP_WORD_LEN:
            PARSING_CMD_DEV(spdif_dev.samp_word_len, value, 5,
                            "Set spdif sample word len");
            break;
        case SPDIF_ORG_SAMP_RATE:
            PARSING_CMD_DEV(spdif_dev.org_samp_rate, value, 0xF,
                            "Set spdif original sampling freq");
            break;
        case SPDIF_PRO_CON:
            PARSING_CMD_DEV(spdif_dev.pro_con, value, 1, "Set spdif pro con");
            break;
        case SPDIF_AUDIO:
            PARSING_CMD_DEV(spdif_dev.audio, value, 1, "Set spdif audio");
            break;
        case SPDIF_COPY_COPYRIGHT:
            PARSING_CMD_DEV(spdif_dev.copy_copyright, value, 1,
                            "Set spdif copyright");
            break;
        case SPDIF_PRE_EMPH:
            PARSING_CMD_DEV(spdif_dev.pre_emph, value, 3, "Set spdif pre emph");
            break;
        case SPDIF_CH_STAT_MODE:
            PARSING_CMD_DEV(spdif_dev.chan_stat_mode, value, 0,
                            "Set spdif channel status mode, always 0");
            break;
        case SPDIF_CATEGORY_COD:
            PARSING_CMD_DEV(spdif_dev.category_cod, value, 0x7F,
                            "Set spdif category code");
            break;
        case SPDIF_GEN_STATUS:
            PARSING_CMD_DEV(spdif_dev.gen_status, value, 1,
                            "Set spdif generation status");
            break;
        case SPDIF_SOURCE_NUM:
            PARSING_CMD_DEV(spdif_dev.source_num, value, 0xF,
                            "Set spdif source num");
            break;
        default:
            goto err_arg;
    }

    return ERR_OK;
  err_arg:
    dp("err_arg");
    return ERR_PARM;
}

int ai_test_spdif()
{
    int i;
    unsigned int bs = real_bufsize;
    struct ai_ch *aich;
    ai_pmu_set();
    ai_init(IF_SPDIF, 400);
    ai_des_link();
    set_spdif_ch_status();

    /* dummy data setting */
    ai_dummy(AI_DUMMY_TONE);

    /* enable channel internal loopback */
    dp("=== if_mode %d ===", test_if_mode);
    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[IF_SPDIF][i];
        //dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_enable(aich);
    }
    test_bus_mode = BUS_16_BIT;
    /* generate test data by bus_mode */
    static signed short sinewave_raw[32000] = { 0 };

    for (i = 0; i < 32000; ++i)
    {
        if ((i / 32) % 2 == 0)
        {
            sinewave_raw[i] = 0x8888;   //0x8888
        }
        else
        {
            sinewave_raw[i] = 0x0000;   //0x0000
        }
    }

    signed short *pTx_addr = &sinewave_raw[0];

    spdif_enable(1);
    while (!tstc())
    {
        prepare_tx(NUM_CH, (void *) pTx_addr, bs);
        idelay(100000);
    }

    dp("leave!!!");
    ai_disable_module();
    return ERR_OK;
}

int ai_i2slb()
{
    int ch = NUM_CH, i;
    int test_if = IF_I2S;
    struct ai_ch *aich;
    ai_pmu_set();
    ai_init(test_if, 2047);
    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    aich_setting(ch, CH_LOOPBACK_EXT, 1);
    test_bus_mode = BUS_16_BIT;
    ai_dummy(AI_DUMMY_TONE);
    ai_dev_init();

    #if 0
    static unsigned char rx_buf[AI_TOTAL_BUF_SIZE] = {0};
    rx_des_state_totx.rx_addr = rx_buf;
    rx_des_state_totx.ori_addr = rx_buf;
    rx_des_state_totx.total_len = AI_TOTAL_BUF_SIZE;
    rx_des_state_totx.rx_idx = 0;

    prepare_rx(ch, ai_rx_cb_memcpy_to_tx);
    prepare_rx(ch, ai_rx_cb_memcpy_to_tx);
    prepare_rx(ch, ai_rx_cb_memcpy_to_tx);
    #endif

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[test_if][i];
        dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_init(aich);
        ai_ch_enable(aich);
    }

    while (!tstc())
    {
        idelay(100000);
    }
    ai_disable_module();
    return ERR_OK;
}

int ai_test_ladda()
{
    int i;
    int test_if = IF_ADC;
    struct ai_ch *aich;
    // reset module
    ai_disable_module();
    ai_pmu_disable_adc();

    ai_pmu_dacadc();
    /* set same clk 62.5k for adc/dac */
    __REG_UPDATE32(0x4c84, 0x00000780, 0x00000fff);
    __REG_UPDATE32(0x4c84, 0x00100000, 0x00180000);
    __REG_UPDATE32(0x4c7c, 0x00002000, 0x00002000);
    __REG_UPDATE32(0x4c7c, 0x00000800, 0x00000fff);
    __REG_UPDATE32(0x704c, 0x00000001, 0x00000001);

#if 0
    ai_init(test_if, 2047);
    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_des_link();
    test_bus_mode = BUS_16_BIT;

    static unsigned char rx_buf[AI_TOTAL_BUF_SIZE] = {0};
    rx_des_state_totx.rx_addr = rx_buf;
    rx_des_state_totx.ori_addr = rx_buf;
    rx_des_state_totx.total_len = AI_TOTAL_BUF_SIZE;
    rx_des_state_totx.rx_idx = 0;

    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
#endif
    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[test_if][i];
        //dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_init(aich);
        ai_ch_enable(aich);
    }

    enable_adc(1);
    enable_dac(1);
    ai_pmu_enable_adc();
    ai_pmu_enable_dac();

#ifndef MASS_PRODUCTION_TEST
    while (!tstc())
    {
        idelay(100000);
    }
#endif
    return ERR_OK;
}

int ai_test_adda()
{
    int ch = NUM_CH, i;
    int test_if = IF_ADC;
    struct ai_ch *aich;

    // reset module
    ai_disable_module();
    ai_pmu_disable_adc();

    ai_pmu_dacadc();
    /* set same clk 62.5k for adc/dac */
    __REG_UPDATE32(0x4c84, 0x00000780, 0x00000fff);
    __REG_UPDATE32(0x4c84, 0x00100000, 0x00180000);
    __REG_UPDATE32(0x4c7c, 0x00002000, 0x00002000);
    __REG_UPDATE32(0x4c7c, 0x00000800, 0x00000fff);

    ai_init(test_if, 368);
    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_des_link();
    test_bus_mode = BUS_16_BIT;

    static unsigned char rx_buf[AI_TOTAL_BUF_SIZE] = {0};
    rx_des_state_totx.rx_addr = rx_buf;
    rx_des_state_totx.ori_addr = rx_buf;
    rx_des_state_totx.total_len = AI_TOTAL_BUF_SIZE;
    rx_des_state_totx.rx_idx = 0;

    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[test_if][i];
        //dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_init(aich);
        ai_ch_enable(aich);
    }
    enable_adc(1);
    enable_dac(1);
    ai_pmu_enable_adc();
    ai_pmu_enable_dac();

#ifndef MASS_PRODUCTION_TEST
    while (!tstc())
    {
        idelay(100000);
    }
#endif
    return ERR_OK;
}

int ai_test_adc(char *pfname)
{
    int ch = NUM_CH, i, rlen;
    int test_if = IF_ADC;
    struct ai_ch *aich;

    FRESULT result;
    FATFS FatFs;
    FIL Fil;

    wait_isr = 1;
    ai_pmu_set();
    ai_init(test_if, 1000);
    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_des_link();

    test_bus_mode = BUS_16_BIT;
    adc_rx_des_state.rx_ptr = adc_rx_des_state.rx_daddr;
    adc_rx_des_state.rx_len = 0;

    prepare_adc_rx(ch, ai_adc_rx_cb_memcpy);
    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[test_if][i];
        //dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_init(aich);
        ai_ch_enable(aich);
    }
    enable_adc(1);
    ai_pmu_enable_adc();
    while (!tstc() && wait_isr == 1)
    {
        idelay(100000);
    }

    // write dump data to file
    printf("try to write file %s\n", pfname);
    static char printout[80000] = {0};
    char *pUnchachePT = (char*)UNCACHED_ADDR(printout);
    int p_offset = 0;
    u16 *p16 = (u16 *) adc_rx_des_state.rx_daddr;
    for(i = 0; i < real_bufsize/2; )
    {
        sprintf(pUnchachePT + p_offset, "%04x", p16[i++]);
        p_offset += 4;
        if (i != 0 && i % 16 != 0)
        {
            sprintf(pUnchachePT + p_offset, " ");
            p_offset += 1;
        }

        if (i % 16 == 0)
        {
            sprintf(pUnchachePT + p_offset, "\n", p16[i]);
            p_offset += 1;
        }
    }

#ifndef CONFIG_ATE
    f_mount(&FatFs, "", 0);
    result = f_open(&Fil, pfname, FA_WRITE | FA_CREATE_ALWAYS);
    if(result==FR_OK)
    {
        result = f_write(&Fil, (const void*)printout, 80000, (UINT*)&rlen);
        f_close(&Fil);
    }
    else
    {
        printf("f_open err 0x%x\n", result);
    }
    f_mount(0, "", 0);
#endif
    return ERR_OK;
}

int ai_init_pcm()
{
    int buf_size = 320;
    ai_pmu_set();
    //int buf_size = 1;
    ai_init(IF_PCM, buf_size);

    int ch = 2, i;
    struct ai_ch *aich;
    test_bus_mode = BUS_16_BIT;

    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_dev_init();

    ai_dummy(AI_DUMMY_TONE);

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[IF_PCM][i];
        ai_ch_init(aich);
        //ai_ch_enable(aich);
    }
    return 0;
}

int ai_init_i2s()
{
    int ch = 2, i;
    struct ai_ch *aich;
    int buf_size = 256;
    ai_pmu_set();
    //int buf_size = 1;
    ai_init(IF_I2S, buf_size);

    test_bus_mode = BUS_16_BIT;

    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_dev_init();

    ai_dummy(AI_DUMMY_TONE);

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[IF_I2S][i];
        ai_ch_init(aich);
        //ai_ch_enable(aich);
    }
    return 0;
}

int ai_init_spdif()
{
    ai_pmu_set();
    ai_init(IF_SPDIF, 400);
    ai_des_link();
    set_spdif_ch_status();

    /* dummy data setting */
    ai_dummy(AI_DUMMY_TONE);

    /* enable channel internal loopback */
    dp("=== if_mode %d ===", test_if_mode);
    test_bus_mode = BUS_16_BIT;
    return ERR_OK;
}

int ai_init_adc()
{
    int ch = NUM_CH, i;
    int test_if = IF_ADC;
    struct ai_ch *aich;

    ai_pmu_set();
    ai_init(test_if, 1000);
    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_des_link();

    test_bus_mode = BUS_16_BIT;
    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[test_if][i];
        //dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_init(aich);
        ai_ch_enable(aich);
    }
    enable_adc(1);
    __REG_UPDATE32(0x4C90, BIT2|BIT3, BIT2|BIT3);
    ai_pmu_enable_adc();
    return ERR_OK;
}

#ifndef CONFIG_ATE
int ai_test_dac()
{
    int i;
    int ch = NUM_CH;
    int buf_size = 1000;
    struct ai_ch *aich;
    ai_pmu_dacadc();
    ai_init(IF_DAC, buf_size);
    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_des_link();

    test_bus_mode = BUS_16_BIT;

    /* dummy data setting */
    ai_dummy(AI_DUMMY_TONE);

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[IF_DAC][i];
        //dp("ch%d loopback %d", aich->ch_no, aich->loopback_int);
        ai_ch_enable(aich);
    }

    unsigned short *pTx_addr = &tone48k_raw[0];
    int total_time = (sizeof(tone48k_raw)/sizeof(unsigned short)) / (real_bufsize/2);
    printf("real_bufsize %d\n", real_bufsize);
    printf("size %d\n", sizeof(tone48k_raw)/sizeof(unsigned short));
    enable_dac(1);
    ai_pmu_enable_dac();
    while (!tstc())
    {
        for(i = 0; i < total_time; )
        {
            if(ERR_OK == prepare_tx(NUM_CH, (void *) pTx_addr, real_bufsize)) {
                pTx_addr += real_bufsize/2;
                ++i;
            }
        }
        pTx_addr = &tone48k_raw[0];

        //idelay(100000);
    }
    ai_pmu_disable_adc();
    ai_disable_module();
    return ERR_OK;
}

int ai_tx_tone(int if_mode)
{
    int buf_size = 1000;
    int ch = 2, rc = 0, i = 0;
    struct ai_ch *aich;

    ai_pmu_set();
    ai_init(if_mode, buf_size);
    test_bus_mode = BUS_16_BIT;

    aich_setting(ch, CH_BUS_MODE, BUS_16_BIT);
    ai_dev_init();

    /* dummy data setting */
    ai_dummy(AI_DUMMY_TONE);

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[if_mode][i];
        ai_ch_init(aich);
        ai_ch_enable(aich);
    }

    unsigned short *pTx_addr = &tone48k_raw[0];
    int total_time = (sizeof(tone48k_raw)/sizeof(unsigned short)) / (real_bufsize/2);
    printf("real_bufsize %d\n", real_bufsize);
    printf("size %d\n", sizeof(tone48k_raw)/sizeof(unsigned short));
    while (!tstc())
    {
        for(i = 0; i < total_time; )
        {
            if(ERR_OK == prepare_tx(NUM_CH, (void *) pTx_addr, real_bufsize)) {
                pTx_addr += real_bufsize/2;
                ++i;
            }
        }
        pTx_addr = &tone48k_raw[0];

        //idelay(100000);
    }

    ai_disable_module();
    printf("exit ai_test_tone\n");
    return rc;
}
#endif

void dump_data(int bus_mode, unsigned int size, void *addr)
{
    int i;
    u8 *p8 = (u8 *) addr;
    u16 *p16 = (u16 *) addr;
    u32 *p24 = (u32 *) addr;

    if (bus_mode == 1 || bus_mode == 2)
    {
        /* 16bit and 16bit wideband */
        size = size / 2;
    }
    else if (bus_mode == 3)
    {
        /* 24 bit */
        size = size / 4;
    }

    for (i = 0; i < size;)
    {
        switch (bus_mode)
        {
            case 0:
                printf("%02x ", p8[i++]);
                if (i % 32 == 0)
                {
                    printf("\n");
                }
                break;
            case 1:
            case 2:
                printf("%04x ", p16[i++]);
                if (i % 16 == 0)
                {
                    printf("\n");
                }
                break;
            case 3:
                printf("%08x ", p24[i++]);
                if (i % 8 == 0)
                {
                    printf("\n");
                }
                break;
        }
    }
}

#ifndef CONFIG_ATE
int ai_test_i2s()
{
    ai_tx_tone(IF_I2S);
    return ERR_OK;
}

int ai_test_pcm()
{
    ai_tx_tone(IF_PCM);
    return ERR_OK;
}

int ai_test_tone(int argc, char *argv[])
{
    if (argc == 2 && (!strcmp("adc", argv[0])))
    {
        ai_test_adc(argv[1]);
        return ERR_OK;
    }

    if (argc != 1)              // none argument
        goto help;

    if (!strcmp("i2s", argv[0]))
    {
        ai_test_i2s();
    }
    else if (!strcmp("pcm", argv[0]))
    {
        ai_test_pcm();
    }
    else if (!strcmp("spdif", argv[0]))
    {
        ai_test_spdif();
    }
    else if (!strcmp("dac", argv[0]))
    {
        ai_test_dac();
    }
    else
    {
        goto err_arg;
    }

    return ERR_OK;

  help:
    return ERR_HELP;
  err_arg:
    return ERR_PARM;
}

void pmu_update_i2s_clk(int domain_sel, int bypass_en, unsigned long clk_div_sel, unsigned long ndiv_sel)
{
    u32 xdiv_reg_0_31_val = 0;
    u32 xdiv_reg_32_47_val = 0;
    u32 xdiv_reg2_0_31_val = 0;

#define PMU_CPLL_XDIV_REG_0_31   (0x4C7C)
#define PMU_CPLL_XDIV_REG_32_47  (0x4C80)
#define PMU_CPLL_XDIV_REG2_0_31  (0x4C84)
#define I2S_NDIV_SEL_SHIFT    15
#define I2S_DOMAIN_SEL        0x00000008
#define I2S_BYPASS_EN         0x01000000
#define I2S_NDIV_SEL_MASK     0x000F8000
#define I2S_CLK_SEL_MASK      0x00030000
#define I2S_CLK_SEL_SHIFT     16

    if(domain_sel)
    {
        xdiv_reg_32_47_val |= I2S_DOMAIN_SEL;
    }

    if(bypass_en)
    {
        xdiv_reg2_0_31_val |= I2S_BYPASS_EN;
    }

    xdiv_reg2_0_31_val |= ((clk_div_sel << I2S_CLK_SEL_SHIFT) & I2S_CLK_SEL_MASK);
    xdiv_reg_0_31_val |= ((ndiv_sel << I2S_NDIV_SEL_SHIFT) & I2S_NDIV_SEL_MASK);

    printf("a [%x] [%x] [%x]\n",PMU_CPLL_XDIV_REG_0_31, PMU_CPLL_XDIV_REG_32_47, PMU_CPLL_XDIV_REG2_0_31);
    printf("v [%x] [%x] [%x]\n",xdiv_reg_0_31_val, xdiv_reg_32_47_val, xdiv_reg2_0_31_val);
   __REG_UPDATE32(PMU_CPLL_XDIV_REG_0_31, xdiv_reg_0_31_val, I2S_NDIV_SEL_MASK);
   __REG_UPDATE32(PMU_CPLL_XDIV_REG_32_47, xdiv_reg_32_47_val, I2S_DOMAIN_SEL);
   __REG_UPDATE32(PMU_CPLL_XDIV_REG2_0_31, xdiv_reg2_0_31_val, I2S_CLK_SEL_MASK|I2S_BYPASS_EN);

}

int ai_test_adci2s()
{
    int ch = NUM_CH, i;
    struct ai_ch *aich;

    ai_init_adc();
    ai_init_i2s();

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[IF_I2S][i];
        ai_ch_enable(aich);
    }

    // initial adc for 64k
    //
    pmu_update_i2s_clk(0,0,0,3);

    static unsigned char rx_buf[AI_TOTAL_BUF_SIZE] = {0};
    rx_des_state_totx.rx_addr = rx_buf;
    rx_des_state_totx.ori_addr = rx_buf;
    rx_des_state_totx.total_len = AI_TOTAL_BUF_SIZE;
    rx_des_state_totx.rx_idx = 0;

    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);
    prepare_adc_rx(ch, ai_adc_cb_memcpy_to_tx);

#if defined(CONFIG_SCHED)
    while(adci2s)
    {
        idelay(100000);
    }
#else
    while (!tstc())
    {
        idelay(100000);
    }
#endif
    enable_adc(0);
    ai_disable_module();

#if defined(CONFIG_SCHED)
    thread_exit();
#endif
    return ERR_OK;
}

int ai_test_i2sdac()
{
    int ch = NUM_CH, i;
    struct ai_ch *aich;

    ai_init_i2s();
    // initial dac for 48k
    __REG_WRITE32(0x4C94, 0x00000020);
    __REG_WRITE32(0x4C98, 0x55428E00);
    __REG_WRITE32(0x4C9C, 0x03040200);
    __REG_WRITE32(0x4CA0, 0x01B2000F);
    __REG_WRITE32(0x4C7C, 0xBE4247D0);
    __REG_WRITE32(0x4C98, 0x5540A555);

    //i2s 48k
    pmu_update_i2s_clk(0,1,2,8);

    static unsigned char rx_buf[AI_TOTAL_BUF_SIZE] = {0};
    rx_des_state_totx.rx_addr = rx_buf;
    rx_des_state_totx.ori_addr = rx_buf;
    rx_des_state_totx.total_len = AI_TOTAL_BUF_SIZE;
    rx_des_state_totx.rx_idx = 0;

    prepare_rx(ch, ai_rx_cb_memcpy_to_tx);
    prepare_rx(ch, ai_rx_cb_memcpy_to_tx);
    prepare_rx(ch, ai_rx_cb_memcpy_to_tx);

    for (i = 0; i < NUM_CH; i++)
    {
        aich = &all_ch[IF_I2S][i];
        ai_ch_enable(aich);
    }

    enable_adc(0);
    ai_pmu_enable_dac();

#if defined(CONFIG_SCHED)
    while(i2sdac)
    {
        idelay(100000);
    }
#else
    while (!tstc())
    {
        idelay(100000);
    }
#endif
    enable_dac(0);
    ai_disable_module();

#if defined(CONFIG_SCHED)
    thread_exit();
#endif
    return ERR_OK;
}

#endif

int ai_cmd(int argc, char *argv[])
{
    if (argc == 0)              // none argument
        goto help;

    if (argc == 1)              // one argument
    {
        if (!strcmp("dis", argv[0]))
        {
            ai_disable_module(aidev);
        }
        else if (!strcmp("i2slb", argv[0]))
        {
            ai_i2slb();
        }
        else if (!strcmp("adda", argv[0]))
        {
            ai_test_adda();
        }
        else if (!strcmp("ladda", argv[0]))
        {
            ai_test_ladda();
        }
        else if (!strcmp("i2sdac", argv[0]))
        {
            printf("Build date:%s\n",__DATE__);
#ifndef CONFIG_ATE
#if defined(CONFIG_SCHED)
            if(!i2sdac)
            {
                i2sdac = 1;
                thread_create(ai_test_i2sdac, 0,
                    &ai_test_thread_stack[AI_TEST_THREAD_STACK_SIZE], AI_TEST_THREAD_STACK_SIZE);
            }
            else
                i2sdac = 0;
#else
                ai_test_i2sdac();
#endif
#endif
        }
        else if (!strcmp("adci2s", argv[0]))
        {
            printf("Build date:%s\n",__DATE__);
#ifndef CONFIG_ATE
#if defined(CONFIG_SCHED)
            if(!adci2s)
            {
                adci2s = 1;
                thread_create(ai_test_adci2s, 0,
                    &ai_test_thread_stack[AI_TEST_THREAD_STACK_SIZE], AI_TEST_THREAD_STACK_SIZE);
            }
            else
                adci2s = 0;
#else
                ai_test_adci2s();
#endif
#endif
        }
        else
            goto err_arg;
    }
    else if (argc == 2)
    {
        if (!strcmp("adc", argv[0]))
        {
            ai_test_adc(argv[1]);
        }
    }
    else
    {
        goto err_arg;
    }

    return ERR_OK;

  help:
    return ERR_HELP;
  err_arg:
    return ERR_PARM;
}

int ai_init_cmd(int argc, char *argv[]) {
    if (argc != 1)              // none argument
        goto help;

    ai_disable_module();
    if (!strcmp("i2s", argv[0]))
    {
        ai_init_i2s();
    }
    else if (!strcmp("pcm", argv[0]))
    {
        ai_init_pcm();
    }
    else if (!strcmp("spdif", argv[0]))
    {
        ai_init_spdif();
    }
    else if (!strcmp("adc", argv[0]))
    {
        ai_init_adc();
    }
    return ERR_OK;

  help:
    return ERR_HELP;
}

int ai_set_cmd(int argc, char *argv[]) {
    int hex;

    if (argc != 2)              // none argument
        goto help;

    if (!strcmp("mlk", argv[0]))
    {
        if (!hextoul(argv[1], &hex))
            goto err_arg;
        aidev_setting(DEV_PCM_MCLK, hex);
    }
    else if (!strcmp("slot", argv[0]))
    {
        if (!hextoul(argv[1], &hex))
            goto err_arg;
        aidev_setting(DEV_SLOT_MODE, hex);
    }
    else if (!strcmp("pol", argv[0]))
    {
        if (!hextoul(argv[1], &hex))
            goto err_arg;
        aidev_setting(DEV_FS_POLAR, hex);
    }
    else if (!strcmp("rate", argv[0]))
    {
        if (!hextoul(argv[1], &hex))
            goto err_arg;
        aidev_setting(DEV_FS_RATE, hex);
    }
    else if (!strcmp("rx_bclk", argv[0]))
    {
        if (!hextoul(argv[1], &hex))
            goto err_arg;
        aidev_setting(DEV_BCLK_LATCH, hex);
    }
    else if (!strcmp("spd_mute", argv[0]))
    {
        if (!hextoul(argv[1], &hex))
            goto err_arg;
        spdif_dev.mute = hex;
    }
    else
    {
        goto err_arg;
    }

    return ERR_OK;

  help:
    return ERR_HELP;
  err_arg:
    return ERR_PARM;
}

#ifndef CONFIG_ATE
cmdt cmdt_ai_test __attribute__ ((section("cmdt"))) =
{
    "ai_test", ai_test_tone,
        "ai_test i2s\n"
        "ai_test pcm\n"
        "ai_test spdif\n"
        "ai_test adc\n"
        "ai_test dac\n"};
cmdt cmdt_ai_set __attribute__ ((section("cmdt"))) =
    {
    "ai_set", ai_set_cmd,
        "ai_set mlk <no>\n"
        "ai_set slot <no>\n"
        "ai_set pol <no>\n"
        "ai_set rate <no>\n"
        "ai_set rx_bclk <no>\n"};

cmdt cmdt_ai_init __attribute__ ((section("cmdt"))) =
    {
    "ai_init", ai_init_cmd,
        "ai_init i2s\n"
        "ai_init pcm\n"
        "ai_init spdif\n"
        "ai_init adc\n"};

cmdt cmdt_ai __attribute__ ((section("cmdt"))) =
    {
    "ai", ai_cmd,
        "ai dis\n"
        "ai_set\n"
        "ai_init\n"
        "ai_test\n"
        "ai adda\n"
        "ai ladda\n"
        "ai adc\n"
        "ai i2slb\n"
        "ai i2sdac\n"
        "ai adci2s\n"
        };
#endif
#endif                          /* CONFIG_AI */
