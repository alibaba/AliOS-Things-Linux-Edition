/*!
*   \file asyncfifo.c
*   \brief Receiving MEPG-TS by HPI appended a asynchronous FIFO
*   \author Montage
*/
#ifdef CONFIG_MPEGTS_ASYNCFIFO
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <gdma.h>
#ifdef __ECOS
#include <os_api.h>             //include interrupt API
#include <cyg/hal/drv_api.h>    //header include interrupt function
#include <cyg/hal/hal_cache.h>  //cache line information
#include <gpio_api.h>
#include <cli_api.h>
#include <delay.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../serv/ffmpeg/include/da3100_stb.h"
#else
#include <common.h>
#include <arch/irq.h>
#include <arch/chip.h>
#endif
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
//#define TIMER0_EMU
//#define COUNT_FRAME_LENGTH //count length or packet

#ifdef __ECOS
#define GET_TICK() os_current_time()
#else
#define GET_TICK() clock_get()
#endif

#ifdef  TIMER0_EMU
#define TCN_INI  (1<<2)|(1<<1)|(1<<0)   /* b2: go, b1: ie, b0: auto-reload */
#define PRESCALE 4
#define T0PERIOD (40000000/PRESCALE/2000)       /* around 10700000/188/11 */

#define  tmr0_init() \
do { \
    volatile unsigned int *pt = (unsigned int *) TM_BASE; \
    pt[T0LR] = T0PERIOD -1 ; \
    pt[T0PR] = PRESCALE -1 ; \
    pt[T0CN] = TCN_INI; \
} while (0);
#endif

#ifdef CONFIG_FPGA
#define FIFO_EMPTY    27
#define FIFO_HALF     26
#define FIFO_FULL     25
#define FIFO_BIT8     24
#define FIFO_RESET    28
#else
#define FIFO_EMPTY    14
#define FIFO_HALF     15
#define FIFO_FULL     16
#define FIFO_BIT8     17
#define FIFO_RESET    21
//#define DEBUG_PIN     18
#endif
#define DEFAULT_FILTER_NUM 0
#define FORWARD_LEN (MPEGTS_FRAME_LEN*7)
#define ASYNCFIFO_LEN (MPEGTS_FRAME_LEN*10)
#define ASYNCFIFO_BUFLEN (2000*ASYNCFIFO_LEN)
#define ASYNCFIFO_BUFLEN_NOT_SHARE_CACHELINE ((ASYNCFIFO_BUFLEN&~(HAL_DCACHE_LINE_SIZE-1))+HAL_DCACHE_LINE_SIZE)
#define ASYNCFIFO_SRC 0xbc000000
#define MPEGTS_SYMBOL 0x47
#ifndef MPEGTS_FRAME_LEN
#define MPEGTS_FRAME_LEN 188
#endif
#define MPEGTS_FRAME_DATALEN 184
//#define HPI_TIMMING_MAGIC_VALUE 0x0610a472 //Bus:120MHz (fake MPEG-TS)
#define HPI_TIMMING_MAGIC_VALUE 0x02100040      //Bus:150MHz (real MPEGT-TS)
#define CACHED_ADDR(a) (((unsigned int)a)&0x8fffffff)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct asyncfifo_dev
{
    unsigned short irq;         /* IRQ number */
#ifdef __ECOS
    cyg_handle_t intr_handle;
    cyg_interrupt intr_obj;
    OS_MUTEX_ID mutex;          /* mutex for dev->pending */
    OS_SEM_ID thread_sem;       /* semaphore for iptv thread */
    OS_THREAD_ID thread;
#endif

    unsigned int filter_number; /* filter_number */
    unsigned int off;           /* interrupt switch */
    unsigned char *buf;         /* buffer pointer */
    unsigned char *buf_top;     /* top of buffer */
    unsigned char *gdma_dst;    /* GDMA destination pointer */
    unsigned char *pending_src; /* buffer pointer for pending data */
    unsigned int pending;       /* pending byte count */
    unsigned int max_pending;   /* maximum pending byte */
    unsigned int out_count;     /* async thread dequeue */
    unsigned int irq_count;     /* irq count */
    unsigned int underrun_count;        /* underrun count */
    unsigned int start_tick;    /* start tick */
    unsigned int unsync_count;
    unsigned int resync_count;
    unsigned int frame_count;
    unsigned int rehandled_count;
    unsigned int wait_gdma_complete;
    unsigned int full;
};
unsigned char asyncfifo_buf[ASYNCFIFO_BUFLEN_NOT_SHARE_CACHELINE]
    __attribute__ ((aligned(HAL_DCACHE_LINE_SIZE)));
struct asyncfifo_dev asyncfifo_devx;
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
#ifdef __ECOS
static unsigned int asyncfifo_isr(cyg_vector_t vec, cyg_addrword_t data)
    __attribute__ ((section(".iram")));
#endif
void asyncfifo_intr_init(struct asyncfifo_dev *dev)
    __attribute__ ((section(".minit")));
void asyncfifo_init(void) __attribute__ ((section(".minit")));
void asyncfifo_reset(void);
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern void breakpoint(void);
extern void asyncfifo_gdma_submit(unsigned int dst);
extern void asyncfifo_gdma_preinit(unsigned int src, unsigned short len,
                                   dma_callback callback, void *param);
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void asyncfifo_callback(void *param)
{
    struct asyncfifo_dev *dev = param;

    //check if interrupt again before callback_func is done
    //if ((*(volatile unsigned int *)(0xaf004000)) &(1<<dev->irq))
    //  printd("[!]");

    /* interrupt is handled and umask interrupt */
#ifdef __ECOS
    if (!dev->off)
        cyg_drv_interrupt_unmask(dev->irq);
#else
    enable_irq(dev->irq);
#endif
}

#ifdef __ECOS
void data_callback(void *param)
{
    struct asyncfifo_dev *dev = param;

#ifdef DEBUG_PIN
    gpio_set(DEBUG_PIN);
#endif
#ifdef COUNT_FRAME_LENGTH
    os_mutex_lock(dev->mutex);
    dev->pending += ASYNCFIFO_LEN;
    os_mutex_unlock(dev->mutex);
#else
    dev->pending += (ASYNCFIFO_LEN / MPEGTS_FRAME_LEN);
#endif
    dev->wait_gdma_complete = 0;

    os_semaphore_post(dev->thread_sem);

    dev->gdma_dst += ASYNCFIFO_LEN;
    if (dev->gdma_dst >= dev->buf_top)
        dev->gdma_dst = dev->buf;
    //printd("dev->gdma_dst=%x\n",dev->gdma_dst);

    if (!dev->off)
        cyg_drv_interrupt_unmask(dev->irq);
}

inline void mpegts_resync(struct asyncfifo_dev *dev, unsigned char *src)
{
    register volatile unsigned char *cp = src + 1;
    int max_ofs = (MPEGTS_FRAME_LEN - 1);
    for (; max_ofs > 0; max_ofs--)      // find the 0x47 sync byte in this packet
    {
        (void) *((volatile unsigned char *) ASYNCFIFO_SRC);
        if (MPEGTS_SYMBOL == *cp++)
        {                       // has read the 1st byte, to read out the remainding bytes
            for (max_ofs = (MPEGTS_FRAME_LEN - 1); max_ofs > 0; max_ofs--)
                (void) *((volatile unsigned char *) ASYNCFIFO_SRC);
            dev->resync_count++;
            break;
        }
    }
}

#ifdef COUNT_FRAME_LENGTH
void iptv_thread(void *param)
{
    unsigned int j = 0, total_len =
        0, selected, end, synced, pending, over, handled_byte;
    unsigned char *src, *dst, *rbuf, *pending_src;
    struct asyncfifo_dev *dev = param;

    //init
    src = dst = rbuf = NULL;
    pending_src = asyncfifo_buf;
    over = 0;

    while (true)
    {
        os_semaphore_wait(dev->thread_sem);

        pending = dev->pending;
        if (!pending)
            continue;

        if (pending > dev->max_pending)
            dev->max_pending = pending;

        handled_byte = pending;
        //`pending` is un-handled length

      rehandle:
        /* initialize source address */
        src = pending_src;

        /* check whether over buffer end boundary */
        if ((src + pending) >=
            ((unsigned char *) asyncfifo_buf + ASYNCFIFO_BUFLEN))
        {
            over =
                (src + pending) - ((unsigned char *) asyncfifo_buf +
                                   ASYNCFIFO_BUFLEN);
            pending -= over;
            end = 1;
        }
        else
        {
            over = 0;
            pending_src += pending;
            end = 0;
        }

        HAL_DCACHE_INVALIDATE(src, pending);

        while (pending >= MPEGTS_FRAME_LEN)
        {
            /* check buffer */
            if (!rbuf)
            {
                rbuf = iptv_alloc();
                dst = (char *) CACHED_ADDR(rbuf);
            }

            /* check sync byte */
            synced = (*src == MPEGTS_SYMBOL);
            if (!synced)
            {
                dev->unsync_count++;
                if (MPEGTS_FRAME_LEN == pending)        // latest packet
                    mpegts_resync(dev, src);
            }

            /* check PID */
            if (0 == dev->filter_number)
            {
                selected = iptv_discard(AV_RB16(src + 1) & 0x1fff);
            }
            else
            {
                if ((selected = (++j >= dev->filter_number)))
                    j = 0;
            }

            /* copy selected frame to wbuf */
            if (selected)
            {
                memcpy(dst, src, MPEGTS_FRAME_LEN);
                dst += MPEGTS_FRAME_LEN;
                total_len += MPEGTS_FRAME_LEN;
            }

            /* forward mpegts data */
            if ((total_len == FORWARD_LEN) || (selected == 2))
            {
                HAL_DCACHE_FLUSH(CACHED_ADDR(rbuf), total_len);
                iptv_forward_stream(rbuf, total_len);
                total_len = 0;
                rbuf = NULL;
            }

            /* update pending */
            pending -= MPEGTS_FRAME_LEN;
            src += MPEGTS_FRAME_LEN;
            dev->frame_count++;
        }

        if (end)
        {
            //update pending_src
            pending_src = asyncfifo_buf;

            //handle data over end boundary
            if (over)
            {
                pending = over;
                dev->rehandled_count++;
                goto rehandle;
            }
        }

        os_mutex_lock(dev->mutex);
        dev->pending -= handled_byte;
        os_mutex_unlock(dev->mutex);

        if ((!dev->wait_gdma_complete) && dev->full)
        {
            dev->full = 0;
            cyg_drv_interrupt_unmask(dev->irq);
        }
#if 0
        //check the full flag of AsyncFIFO
        if (!(GPREG(GPVAL) & (1 << FIFO_FULL)))
        {
            timeout_lo((OS_FUNCPTR) printf, "AsyncFIFO is full!\n", 0);
        }
#endif
    }
}
#else
void iptv_thread(void *param)
{
    unsigned int j = 0, total_len = 0, synced, selected;
    unsigned char *src, *dst, *rbuf;
    struct asyncfifo_dev *dev = param;
    int to_do;

    src = dst = rbuf = NULL;

    dev->pending_src = dev->buf;
    while (true)
    {
        os_semaphore_wait(dev->thread_sem);

        src = dev->pending_src;
        to_do = (int) dev->pending - (int) dev->out_count;
        //printf("%s:to_do=%d\n", __func__, to_do);
        if (to_do > dev->max_pending)
            dev->max_pending = to_do;

        for (; to_do-- > 0;)
        {
            /* check buffer */
            if (!rbuf)
            {
                rbuf = iptv_alloc();
                dst = (char *) CACHED_ADDR(rbuf);
            }
            dev->out_count++;

            HAL_DCACHE_INVALIDATE(src, MPEGTS_FRAME_LEN + 16);
            /* check sync byte */
            synced = (*src == MPEGTS_SYMBOL);
            if (!synced)
            {
                dev->unsync_count++;
                if (2 > to_do)  // latest packet
                    mpegts_resync(dev, src);
            }
            else
            {

                /* check PID */
                if (0 == dev->filter_number)
                {
                    selected = iptv_discard(AV_RB16(src + 1) & 0x1fff);
                }
                else
                {
                    if ((selected = (++j >= dev->filter_number)))
                        j = 0;
                }

                /* copy selected frame to wbuf */
                if (selected)
                {
                    memcpy(dst, src, MPEGTS_FRAME_LEN);
                    dst += MPEGTS_FRAME_LEN;
                    total_len += MPEGTS_FRAME_LEN;
                }

                /* forward mpegts data */
                if ((total_len == FORWARD_LEN) || (selected == 2))
                {
                    HAL_DCACHE_FLUSH(CACHED_ADDR(rbuf), total_len);
                    iptv_forward_stream(rbuf, total_len);
                    total_len = 0;
                    rbuf = NULL;
                }
                dev->frame_count++;
            }
            src += MPEGTS_FRAME_LEN;

            /* update local variable back to dev structure */
            if (src < dev->buf_top)
                dev->pending_src = src;
            else
            {
                dev->pending_src = dev->buf;
                src = dev->buf;
            }
        }                       //for

        if ((!dev->wait_gdma_complete) && dev->full)
        {
            dev->full = 0;
            cyg_drv_interrupt_unmask(dev->irq);
        }

    }
}
#endif

static void asyncfifo_full(struct asyncfifo_dev *dev)
{
    printf("buff is full !!!\n");
}

static unsigned int asyncfifo_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    /* mask interrupt */
    cyg_drv_interrupt_mask(vec);

    /* clear latch interrupt */
    cyg_drv_interrupt_acknowledge(vec);
#ifdef  TIMER0_EMU
    {
        volatile unsigned int *pt = (unsigned int *) TM_BASE;
        pt[T0IS] = 1;
        pt[T0CR] = 0;
    }
#endif
    return CYG_ISR_CALL_DSR;
}

static void asyncfifo_dsr(cyg_vector_t vec, cyg_ucount32 count,
                          cyg_addrword_t data)
{
    struct asyncfifo_dev *dev = (struct asyncfifo_dev *) data;

    dev->irq_count++;
#ifdef COUNT_FRAME_LENGTH
    if (dev->pending > (ASYNCFIFO_BUFLEN - ASYNCFIFO_LEN))
    {
#else
    if (((int) dev->pending - (int) dev->out_count) >
        ((ASYNCFIFO_BUFLEN - ASYNCFIFO_LEN) / MPEGTS_FRAME_LEN))
    {
#endif
        dev->full = 1;
        timeout_lo((OS_FUNCPTR) asyncfifo_full, dev, 0);
    }
    else
    {
        dev->wait_gdma_complete = 1;
#ifdef DEBUG_PIN
        gpio_clr(DEBUG_PIN);
#endif
        asyncfifo_gdma_submit((unsigned int) dev->gdma_dst);
    }
}
#else
static void asyncfifo_irqhandler(void *data)
{
    struct asyncfifo_dev *dev = (struct asyncfifo_dev *) data;

    /* mask interrupt */
    disable_irq(dev->irq);

    asyncfifo_gdma_submit((unsigned int) dev->buf);
}
#endif

void asyncfifo_intr_init(struct asyncfifo_dev *dev)
{
    /* assign interrupt source */
#ifdef TIMER0_EMU
    dev->irq = IRQ_TMR0;
    tmr0_init();
#else                           /* Assign GPIO_X */
    dev->irq = IRQ_GPIOX;
#endif

    /* configure interrupt */
#ifdef __ECOS
    /* mask interrupt */
    cyg_drv_interrupt_mask(dev->irq);

    /* Interrupt and Interrupt Handler Configuration */
    cyg_drv_interrupt_create(dev->irq, 0, (CYG_ADDRWORD) dev,
                             asyncfifo_isr, asyncfifo_dsr, &dev->intr_handle,
                             &dev->intr_obj);

    /* hook handler */
    cyg_drv_interrupt_attach(dev->intr_handle);

    /* IRQ number and trigger Configuration */
    cyg_drv_interrupt_configure(dev->irq, 1, 0);        //level(1) low(0) trigger

    /* umask interrupt and enable interrupt work */
    if (!dev->off)
        cyg_drv_interrupt_unmask(dev->irq);
#else
    /* Interrupt trigger level setting */
    ICREG(IPLR) &= ~(1 << dev->irq);    //active low

    request_irq(dev->irq, &asyncfifo_irqhandler, (void *) dev);
#endif
}

void asyncfifo_reset(void)
{
#ifdef __ECOS
    gpio_set(FIFO_RESET);
    udelay(100);
    gpio_clr(FIFO_RESET);
    udelay(100);
    gpio_set(FIFO_RESET);
    (*(volatile unsigned int *) (0xa0000000));  //readback for cleaning write buffer
#else
    GPREG(GPSET) = (1 << FIFO_RESET);
    GPREG(GPSET);
    idelay(100);
    GPREG(GPCLR) = (1 << FIFO_RESET);
    GPREG(GPCLR);
    idelay(100);
    GPREG(GPSET) = (1 << FIFO_RESET);
    GPREG(GPSET);
#endif
}

void asyncfifo_init(void)
{
    struct asyncfifo_dev *dev = &asyncfifo_devx;

#ifdef __ECOS
    /* pre-initialize GDMA descriptor */
    asyncfifo_gdma_preinit(ASYNCFIFO_SRC, ASYNCFIFO_LEN, data_callback, dev);
#else
    asyncfifo_gdma_preinit(ASYNCFIFO_SRC, ASYNCFIFO_LEN, asyncfifo_callback,
                           dev);
#endif

    /* initialize device structure */
    dev->buf = asyncfifo_buf;
    dev->buf_top = asyncfifo_buf + ASYNCFIFO_BUFLEN;
    dev->gdma_dst = asyncfifo_buf;
    dev->off = 1;
    dev->pending = 0;
    dev->max_pending = 0;
    dev->filter_number = DEFAULT_FILTER_NUM;
    dev->irq_count = 0;
    dev->underrun_count = 0;
    dev->unsync_count = 0;
    dev->frame_count = 0;
    dev->rehandled_count = 0;
    dev->wait_gdma_complete = 0;
    dev->full = 0;

    /* PINMUX setting */
    GPREG(PINMUX) |= EN_ROM1_FNC;
    GPREG(PINMUX) &= (~EN_ETH1_FNC);

    /* HPI timming setting */
    MIREG(MR2C) = HPI_TIMMING_MAGIC_VALUE;

    /* initialize GPIO */
#ifdef __ECOS
    gpio_dir_input(FIFO_EMPTY);
    gpio_dir_input(FIFO_HALF);
    gpio_dir_input(FIFO_FULL);
    gpio_dir_input(FIFO_BIT8);
    gpio_dir_output(FIFO_RESET);
#ifdef DEBUG_PIN
    gpio_dir_output(DEBUG_PIN);
    gpio_set(DEBUG_PIN);
#endif
#else
    GPREG(GPSEL) |=
        ((1 << FIFO_EMPTY) | (1 << FIFO_HALF) | (1 << FIFO_FULL) |
         (1 << FIFO_BIT8) | (1 << FIFO_RESET));
    GPREG(GPDIR) |= ((1 << FIFO_RESET));
#endif

    /* reset AsyncFIFO */
    asyncfifo_reset();

#ifdef __ECOS
    /* create a thread as interrupt handler */
    dev->thread_sem = os_semaphore_create(0);
    dev->thread =
        os_thread_create(iptv_thread, dev, "iptv", 7,
                         CYGNUM_HAL_STACK_SIZE_TYPICAL);
    dev->mutex = os_mutex_create();
#endif

    /* initialize interrupt settting */
    asyncfifo_intr_init(dev);
}

#ifdef __ECOS
#define NUM_IN_ONE_LINE 16
void asyncfifo_dumpbuf(struct asyncfifo_dev *dev)
{
    u32 *saddr = (void *) dev->buf;
    u32 *eaddr = saddr + ASYNCFIFO_LEN;
    u32 *addr = saddr;

    while (addr < eaddr)
    {
        if (!((u32) addr % NUM_IN_ONE_LINE) || (addr == saddr))
            printf("\n%08x: ", addr);
        printf("%08x ", *(volatile u32 *) addr);
        addr++;
    }
}

void asyncfifo_statistic(struct asyncfifo_dev *dev)
{
    int period_tick = GET_TICK() - dev->start_tick;
    printf("buf:%08x\n", dev->buf);
    printf("max:%u\n", dev->max_pending);
    printf("p:%u\n", dev->pending);
    printf("w:%u\n", dev->wait_gdma_complete);
    printf("f:%u\n", dev->full);
#ifndef COUNT_FRAME_LENGTH
    printf("out:%u\n", dev->out_count);
#endif
    printf("filter_number:%d\n", dev->filter_number);
    printf("irq_off:%u\n", dev->off);
    printf("irq_count:%u\n", dev->irq_count);
    printf("period:%d\n", period_tick);
    printf("irq_count/period:%d\n", dev->irq_count / period_tick);
    printf("unsync_count:%u\n", dev->unsync_count);
    printf("resync_count:%u\n", dev->resync_count);
    printf("frame_count:%u\n", dev->frame_count);
    printf("rehandled_count:%u\n", dev->rehandled_count);
    printf("underrun_count:%u\n", dev->underrun_count);
    //reset irq_count and start_tick
    dev->irq_count = 0;
    dev->start_tick = GET_TICK();
}

int asyncfifo_cmd(int argc, char *argv[])
{
    struct asyncfifo_dev *dev = &asyncfifo_devx;

    if (argc < 2)
        goto help;

    if (strncmp(argv[1], "stat", sizeof ("stat")) == 0)
    {
        timeout_lo((OS_FUNCPTR) asyncfifo_statistic, dev, 0);
    }
    else if (strncmp(argv[1], "buf", sizeof ("buf")) == 0)
    {
        timeout_lo((OS_FUNCPTR) asyncfifo_dumpbuf, dev, 0);
    }
    else if (strncmp(argv[1], "on", sizeof ("on")) == 0)
    {
        /* clear latch interrupt */
        cyg_drv_interrupt_acknowledge(dev->irq);

        asyncfifo_reset();
        dev->off = 0;
        dev->start_tick = GET_TICK();
        cyg_drv_interrupt_unmask(dev->irq);
    }
    else if (strncmp(argv[1], "off", sizeof ("off")) == 0)
    {
        dev->off = 1;
        cyg_drv_interrupt_mask(dev->irq);
    }
    else if (strncmp(argv[1], "num", sizeof ("num")) == 0)
    {
        if (argc != 3)
            goto help;
        dev->filter_number = strtol(argv[2], NULL, 10);
    }
    else if (strncmp(argv[1], "reset", sizeof ("reset")) == 0)
    {
        asyncfifo_reset();
    }

    return CLI_OK;
  help:
    return CLI_SHOW_USAGE;
}

CLI_CMD(ts, asyncfifo_cmd,
        "ts; receiving simulated MPEG-TS stream with a Asynchronous FIFO\n"
        "\tbuf : show buffer content\n" "\tstat : show statistic\n"
        "\ton : switch on interrupt\n" "\toff : switch off interrupt\n"
        "\tnum : filter rate\n" "\treset : reset fifo\n", 0);
#endif
#endif
