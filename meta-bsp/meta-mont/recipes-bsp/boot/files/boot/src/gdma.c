/*!
*   \file gdma.c
*   \brief Generic DMA driver.
*   \author Montage
*/
#ifdef CONFIG_GDMA
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#ifdef __ECOS
#include <os_api.h>             //include interrupt API
#include <cyg/hal/drv_api.h>    //header include interrupt function
#include <string.h>
#else
#include <lib.h>
#include <common.h>
#include <arch/irq.h>
#include <arch/chip.h>
#endif
#include <gdma.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define GDMA_DESCR_COUNT 20
#ifdef __ECOS
#define GDMA_LOCK()   os_mutex_lock(dev->mutex)
#define GDMA_UNLOCK() os_mutex_unlock(dev->mutex)
#else
#define GDMA_LOCK()
#define GDMA_UNLOCK()
#endif
#define GDMAREG(addr) (*(volatile unsigned int*)(GDMA_BASE+addr))
#define writel(a, b)  (*((volatile u32 *)(a)) = ((volatile u32)b))
#define readl(a)      (*((volatile u32 *)(a)))
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct gdma_regs
{
    u32 enable;
    u32 kick;
    u32 status;
    u32 mask;
    u32 threshold;
    u32 baseaddr;
    u32 lastraddr;
};
struct gdma_dev
{
    unsigned short irq;         /* GDMA IRQ number */
#ifdef __ECOS
    cyg_handle_t intr_handle;
    cyg_interrupt intr_obj;
    OS_MUTEX_ID mutex;          /* GDMA mutex */
#endif
    struct gdma_regs *reg;      /* GDMA register pointer */
    u16 ridx;                   /* GDMA last freed descriptor index */
    u16 widx;                   /* GDMA last alllocated descriptor index */
    u16 rcount;                 /* GDMA last freed descriptor count */
    u16 wcount;                 /* GDMA last alllocated descriptor count */
    dma_descriptor desc[GDMA_DESCR_COUNT];      /* GDMA descriptor array */
};
struct gdma_dev gdma_devx = {.reg = (void *) GDMA_BASE };

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
#ifdef __ECOS
static unsigned int gdma_isr(cyg_vector_t vec, cyg_addrword_t data)
    __attribute__ ((section(".iram")));
#endif
void gdma_intr_init(struct gdma_dev *dev) __attribute__ ((section(".minit")));
void gdma_init(void) __attribute__ ((section(".minit")));
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static inline void gdma_intr_mask(unsigned int *addr)
{
    writel(addr, (DMA_INTR_MASK_COUNT | DMA_INTR_MASK_TIME));
}

static inline void gdma_intr_unmask(unsigned int *addr)
{
    writel(addr, DMA_INTR_MASK_TIME);
}

static inline int get_dma_hw_descr_rindex(struct gdma_dev *dev)
{
    return ((readl(&dev->reg->lastraddr) -
             PHYSICAL_ADDR((u32) dev->desc)) / sizeof (dma_descriptor));
}

inline void gdma_des_handle(void *param)
{
    int dma_index;
    dma_callback cb;
    struct gdma_dev *dev = param;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);

    dma_index = get_dma_hw_descr_rindex(dev);

    //printd("[%s:%d]dma_index=%d dev->ridx=%d\n",__func__,__LINE__,dma_index,dev->ridx);
    while (dev->ridx != dma_index)
    {
        if (des[dev->ridx].sw_inuse)
        {
            if (des[dev->ridx].sw_callback)
            {
                cb = (dma_callback) des[dev->ridx].sw_callback;
                //printd("[%s:%d]cb=%08x p=%08x\n",__func__,__LINE__,cb,des[dev->ridx].sw_priv);
                cb((void *) des[dev->ridx].sw_priv);
            }

            dev->rcount++;
            des[dev->ridx].sw_inuse = 0;
        }
        else
        {
            //printd("GDMA bug: descr. %d not done\n", dev->ridx);
            break;
        }

        dev->ridx = (dev->ridx + 1) % GDMA_DESCR_COUNT;
    }
}

#ifdef __ECOS
static unsigned int gdma_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    /* mask interrupt */
    cyg_drv_interrupt_mask(vec);

    /* clear latch interrupt */
    cyg_drv_interrupt_acknowledge(vec);

    return CYG_ISR_CALL_DSR;
}

static void gdma_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    struct gdma_dev *dev = (struct gdma_dev *) data;
    struct gdma_regs *reg = dev->reg;
    int status;

    gdma_intr_mask(&dev->reg->mask);

    status = readl(&reg->status);       //only support count interrupt
    writel(&reg->status, status);

    gdma_des_handle(dev);

    gdma_intr_unmask(&reg->mask);
    /* interrupt is handled and umask interrupt */
    cyg_drv_interrupt_unmask(vec);
}
#else
static void gdma_irqhandler(void *param)
{
    struct gdma_dev *dev = param;
    struct gdma_regs *reg = dev->reg;
    int status;

    disable_irq(dev->irq);
    gdma_intr_mask(&dev->reg->mask);

    status = readl(&reg->status);       //only support count interrupt
    writel(&reg->status, status);

    gdma_des_handle(dev);

    gdma_intr_unmask(&reg->mask);
    enable_irq(dev->irq);
}
#endif

void gdma_intr_init(struct gdma_dev *dev)
{
    dev->irq = IRQ_GDMA;

#ifdef __ECOS
    /* mask interrupt */
    cyg_drv_interrupt_mask(dev->irq);

    /* Interrupt and Interrupt Handler Configuration */
    cyg_drv_interrupt_create(dev->irq, 0, (CYG_ADDRWORD) dev,
                             gdma_isr, gdma_dsr, &dev->intr_handle,
                             &dev->intr_obj);

    /* hook handler */
    cyg_drv_interrupt_attach(dev->intr_handle);

    /* IRQ number and trigger Configuration */
    cyg_drv_interrupt_configure(dev->irq, 1, 1);

    /* umask interrupt and enable interrupt work */
    cyg_drv_interrupt_unmask(dev->irq);
#else
    request_irq(dev->irq, &gdma_irqhandler, (void *) dev);
#endif
}

void gdma_init(void)
{
    struct gdma_dev *dev = &gdma_devx;
    struct gdma_regs *reg = dev->reg;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);

    /* initialize descriptors */
    memset((char *)des, 0, sizeof (dma_descriptor) * GDMA_DESCR_COUNT);
    des[GDMA_DESCR_COUNT - 1].eor = 1;
    dev->ridx = dev->widx = 0;
    dev->rcount = dev->wcount = 0;

    /* initialize registers */
    writel(&reg->threshold, 0);
    writel(&reg->baseaddr, &dev->desc);
    gdma_intr_unmask(&reg->mask);
    writel(&reg->enable, 1);

#ifdef __ECOS
    /* create a thread as interrupt handler */
    dev->mutex = os_mutex_create();
#endif

    /* initialize interrupt settting */
    gdma_intr_init(dev);
}

void gdma_finish(void)
{
    struct gdma_dev *dev = &gdma_devx;
    struct gdma_regs *reg = dev->reg;

    writel(&reg->enable, 0);
    gdma_intr_mask(&reg->mask);
}

#if defined(CONFIG_MPEGTS_ASYNCFIFO)
void asyncfifo_gdma_submit(unsigned int dst)
{
    short idx;
    struct gdma_dev *dev = (struct gdma_dev *) &gdma_devx;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);

    idx = dev->widx;

    des[idx].ctrl = GDMA_CTRL_GO;
    des[idx].dest_addr = dst;
    des[idx].sw_inuse = 1;
    dev->widx = (idx + 1) % GDMA_DESCR_COUNT;
    dev->wcount++;
    /* kick */
    writel(&dev->reg->kick, 1);
}

void asyncfifo_gdma_preinit(unsigned int src, unsigned short len,
                            dma_callback callback, void *param)
{
    short idx;
    struct gdma_dev *dev = (struct gdma_dev *) &gdma_devx;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);

    for (idx = 0; idx < GDMA_DESCR_COUNT; idx++)
    {
        des[idx].operation = GDMA_OP_COPY;
        des[idx].cksum_length = len;
        des[idx].operation_length = len;
        des[idx].src_addr = src;
        des[idx].sw_callback = (u32) callback;
        des[idx].sw_priv = (u32) param;
    }
}
#elif defined(CONFIG_LCD_ST7789V)
#define LCD_USE_GDMA_DES_NUM 4
#if (GDMA_DESCR_COUNT%LCD_USE_GDMA_DES_NUM)
#error "LCD ST7789V assume GDMA_DESCR_COUNT is 4x"
#endif
void lcd_gdma_submit(unsigned int src)
{
    short idx;
    struct gdma_dev *dev = (struct gdma_dev *) &gdma_devx;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);
    unsigned short len, i;

    idx = dev->widx;

    len = des[idx].operation_length;
    for (i = 0; i < LCD_USE_GDMA_DES_NUM; i++)
    {
        des[idx + i].ctrl = GDMA_CTRL_GO;
        des[idx + i].src_addr = src + i * len;
        des[idx + i].sw_inuse = 1;
    }

    dev->widx = (idx + LCD_USE_GDMA_DES_NUM) % GDMA_DESCR_COUNT;
    dev->wcount += LCD_USE_GDMA_DES_NUM;
    /* kick */
    writel(&dev->reg->kick, 1);
}

void lcd_gdma_preinit(unsigned int dst, unsigned short len,
                      dma_callback callback)
{
    short idx;
    struct gdma_dev *dev = (struct gdma_dev *) &gdma_devx;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);

    for (idx = 0; idx < GDMA_DESCR_COUNT; idx++)
    {
        des[idx].operation = GDMA_OP_COPY;
        des[idx].operation_length = len;
        des[idx].dest_addr = dst;
        if ((LCD_USE_GDMA_DES_NUM - 1) == (idx % LCD_USE_GDMA_DES_NUM))
            des[idx].sw_callback = (u32) callback;
    }
}
#else
int gdma_submit(unsigned int op, unsigned int src, unsigned int dst,
                unsigned short len, dma_callback callback, void *param)
{
    int ret = -1;
    short idx;
    struct gdma_dev *dev = (struct gdma_dev *) &gdma_devx;
    dma_descriptor *des = (void *) UNCACHED_ADDR(dev->desc);

    GDMA_LOCK();

    idx = dev->widx;

    if (des[idx].ctrl == GDMA_CTRL_DONE_STOP)
    {
        if (des[idx].sw_inuse == 0)
        {
            des[idx].ctrl = GDMA_CTRL_GO;
            des[idx].operation = op;
            des[idx].cksum_length = len;
            des[idx].dest_addr = dst;
            des[idx].operation_length = len;
            des[idx].src_addr = src;
            des[idx].sw_callback = (u32) callback;
            des[idx].sw_priv = (u32) param;
            des[idx].sw_inuse = 1;
            dev->widx = (idx + 1) % GDMA_DESCR_COUNT;
            dev->wcount++;
            /* kick */
            writel(&dev->reg->kick, 1);
            ret = 0;
        }
    }

    GDMA_UNLOCK();

    return ret;
}
#endif
#endif
