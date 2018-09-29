/*!
*   \file tsi.c
*   \brief Transport Stream Input interface
*   \author Montage
*/
#ifdef CONFIG_TSI
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <arch/irq.h>
#include <lib.h>
#include "tsi.h"

void delay(unsigned int time);
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct tsi_dev g_tsi_dev;
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void tsi_intr_init(struct tsi_dev *dev) __attribute__ ((section(".minit")));
void tsi_init(void) __attribute__ ((section(".minit")));
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
static void tsi_irqhandler(void *data)
{
    struct tsi_dev *dev = (struct tsi_dev *) data;
    unsigned int stat = TSIREG(TSI_INTR_STAT);

    /* mask interrupt */
    disable_irq(IRQ_TSI);
    //printf("%s\n",__func__);

    /* hold and clear status latch */
    TSIREG(TSI_INTR_CLR) = stat;

    /* statistics */
    dev->irq_count++;
    if (stat & TSI_OFLW_I)
        dev->irq_stat_unrun++;
    if (stat & TSI_SPKT_DONE_I)
        dev->irq_stat_pdone++;
    if (stat & TSI_DST_CHG_I)
        dev->irq_stat_dstch++;

    /* call handler */
    if (dev->func)
        dev->func(dev, stat);

    /* release status latch */
    TSIREG(TSI_INTR_CLR) = 0;

    /* interrupt is handled and umask interrupt */
    enable_irq(IRQ_TSI);
}

void tsi_intr_init(struct tsi_dev *dev)
{
    request_irq(IRQ_TSI, &tsi_irqhandler, (void *) dev);
}

static int tsi_mux_sel = 1;
void tsi_show_mux(void)
{
    if(tsi_mux_sel==1)
    {
        printf("TS_D5: GPIO14, TS_D6: GPIO15, TS_D7: GPIO16\n");
    }
    else
    {
        printf("TS_D5: GPIO6, TS_D6: GPIO7, TS_D7: GPIO8\n");
    }
}

void tsi_asic_init(void)
{
#if defined(CONFIG_FPGA)
    return;
#endif

    if(tsi_mux_sel==1)
    {
        /*   Pinmux type 1
             GPIO0: sel 7, TS_S_CLK
             GPIO1: sel 7, TS_S_VALID
             GPIO2: sel 7, TS_S_SYNC
             GPIO3: sel 7, TS_S_ERR
             GPIO4: sel 7, TS_D0
             GPIO9: sel 7, TS_D1
             GPIO10: sel 7, TS_D2
             GPIO11: sel 7, TS_D3
             GPIO12: sel 7, TS_D4
             GPIO14: sel 7, TS_D5
             GPIO15: sel 7, TS_D6
             GPIO16: sel 7, TS_D7
         */
        *(volatile unsigned long *)0xbf004a18 = 0x99977777UL;
        *(volatile unsigned long *)0xbf004a1c = 0x77077779UL;
        *(volatile unsigned long *)0xbf004a20 = 0x01100007UL;
    }
    else
    {
        /*   Pinmux type 0
             GPIO0: sel 7, TS_S_CLK
             GPIO1: sel 7, TS_S_VALID
             GPIO2: sel 7, TS_S_SYNC
             GPIO3: sel 7, TS_S_ERR
             GPIO4: sel 7, TS_D0
             GPIO6: sel 7, TS_D5
             GPIO7: sel 7, TS_D6
             GPIO8: sel 7, TS_D7
             GPIO9: sel 7, TS_D1
             GPIO10: sel 7, TS_D2
             GPIO11: sel 7, TS_D3
             GPIO12: sel 7, TS_D4
         */
        *(volatile unsigned long *)0xbf004a18 = 0x77977777UL;
        *(volatile unsigned long *)0xbf004a1c = 0x00077777UL;
        *(volatile unsigned long *)0xbf004a20 = 0x01100000UL;
    }
}

int tsi_mux(int argc, char *argv[])
{
    unsigned int mux_sel;

    if (!hextoul(argv[1], &mux_sel))
        goto err_arg;

    tsi_mux_sel = mux_sel;
    tsi_show_mux();

    return ERR_OK;
err_arg:
    return ERR_PARM;
}

void tsi_start(struct tsi_dev *dev)
{
    int i;

#if !defined(CONFIG_FPGA)
    tsi_asic_init();
#endif

    /* check setting */
    if ((dev->size0) % (TSI_BURSTSIZE + 1))
    {
        printf("illegal buffer0 size\n");
        return;
    }
    if ((dev->size1) % (TSI_BURSTSIZE + 1))
    {
        printf("illegal buffer1 size\n");
        return;
    }

    /* initialize the destination address for ping-pong buffer */
    TSIREG(TSI_BUF0_ADDR) = dev->addr0;
    TSIREG(TSI_BUF0_SIZE) = dev->size0;
    TSIREG(TSI_BUF1_ADDR) = dev->addr1;
    TSIREG(TSI_BUF1_SIZE) = dev->size1;

    /* initialize the transfer mode */
    if (dev->flags & TSI_FLAG_BOUNDARY_STOP)
        TSIREG(TSI_CTRL) |= TSI_BNDR_STOP;
    else
        TSIREG(TSI_CTRL) &= ~TSI_BNDR_STOP;

    /* initialize the filter setting */
    for (i = 0; i < TSI_PFLTR_NUM; i += 2)
        TSIREG(TSI_FLTR_BASE + (i * 2)) =
            (dev->filter_id[i + 1] << TSI_FLTR_OFT) | dev->filter_id[i];
    TSIREG(TSI_CSIZE) &= ~(TSI_FLTR_BMAP);
    TSIREG(TSI_CSIZE) |= dev->filter_number;

    /* TS interrupt enable setting */
    TSIREG(TSI_CTRL) &= ((1 << TSI_SPKTNUM_OFT) - 1);
    TSIREG(TSI_INTR_EN) = 0;
    if (dev->func)
    {
        TSIREG(TSI_CTRL) |= (dev->intr_pktnum_th << TSI_SPKTNUM_OFT);
        TSIREG(TSI_INTR_EN) = dev->intr_status;
    }

    /* enable TS interface */
    TSIREG(TSI_CTRL) |= (TSI_EN);
    //TSIREG(TSI_CTRL);
}

void tsi_stop(struct tsi_dev *dev)
{
    /* disable TS interface */
    TSIREG(TSI_CTRL) &= ~(TSI_EN);

    /* clear TSI module */
    TSIREG(TSI_CTRL) |= TSI_STALL;
    while (!(TSIREG(TSI_ADC_INFO) & TSI_AXIM_IDLE)) ;
    TSIREG(TSI_CTRL) |= TSI_CLR_MODULE;
    while (!
           ((TSIREG(TSI_ADC_INFO) & (TSI_CORE_IDLE | TSI_FIFO_IDLE)) ==
            (TSI_CORE_IDLE | TSI_FIFO_IDLE))) ;
    delay(1);
    //printf("IDLE: 0x%08x\n",TSIREG(TSI_ADC_INFO));
    TSIREG(TSI_CTRL) &= ~(TSI_CLR_MODULE | TSI_STALL);

    /* clear status latch */
    TSIREG(TSI_INTR_CLR) = ~0;
    delay(1);
    TSIREG(TSI_INTR_CLR) = 0;
    //TSIREG(TSI_INTR_CLR);
}

void tsi_stat(void)
{
    unsigned int regval;

    tsi_show_mux();
    printf("current address: 0x%08x\n", TSIREG(TSI_CUR_ADDR));
    printf("irq count: %d\n", g_tsi_dev.irq_count);
    printf("irq stat unrun: %d\n", g_tsi_dev.irq_stat_unrun);
    printf("irq stat pdone: %d\n", g_tsi_dev.irq_stat_pdone);
    printf("irq stat dstch: %d\n", g_tsi_dev.irq_stat_dstch);
    regval = TSIREG(TSI_INTR_STAT);
    printf("irq stat reg: 0x%08x\n", regval);
    //INTR_STAT
    if (regval & TSI_OFLW_DROP_I)
        printf("\t[8]packet drop because overflow\n");
    if (regval & TSI_SYN_DROP_I)
        printf("\t[7]packet drop because sync err\n");
    if (regval & TSI_DST_CHG)
        printf("\t[5]change destination buffer\n");
    if (regval & TSI_SPKT_DONE)
        printf("\t[4]rx specific packet number\n");
    if (regval & TSI_REV_PKT)
        printf("\t[3]rx packet\n");
    if (regval & TSI_SIZEERR)
        printf("\t[2]Size Error\n");
    if (regval & TSI_SYNCERR)
        printf("\t[1]Sync Error\n");
    if (regval & TSI_OFLW)
        printf("\t[0]FIFO overflow\n");
    //
    regval = TSIREG(TSI_ADC_INFO);
    printf("reg(%02x): 0x%08x\n", TSI_ADC_INFO, regval);
    printf("\t[31-16]FIFO count: %d bytes\n", (regval >> TSI_FIFO_CNT) << 2);
    if (regval & TSI_FIFO_FULL)
        printf("\t[5]FIFO full\n");
    printf("\t[4]dst ptr: %d\n", (regval & TSI_CURR_DST) ? 1 : 0);
    if (regval & TSI_AXIS_IDLE)
        printf("\t[3]AXI Slave idle\n");
    if (regval & TSI_AXIM_IDLE)
        printf("\t[2]AXI Master idle\n");
    if (regval & TSI_CORE_IDLE)
        printf("\t[1]core idle\n");
    if (regval & TSI_FIFO_IDLE)
        printf("\t[0]FIFO idle\n");
    //
    regval = TSIREG(TSI_AXI_INFO);
    printf("reg(%02x): 0x%08x\n", TSI_AXI_INFO, regval);
    printf("\t[31-16]event count: %d\n", regval >> TSI_EVENT_CNT);
    if (regval & TSI_AXIS_RDNRDY)
        printf("\t[7]Read-Data No Ready\n");
    if (regval & TSI_AXIS_RCNRDY)
        printf("\t[6]Read-Command No Ready\n");
    if (regval & TSI_AXIM_WDST)
        printf("\t[4]master write to buf\n");
    if (regval & TSI_AXIM_RDNRDY)
        printf("\t[4]Read-Data No Ready\n");
    if (regval & TSI_AXIM_WDNRDY)
        printf("\t[3]Write-Data No Ready\n");
    if (regval & TSI_AXIM_WRNRDY)
        printf("\t[2]Write-Response No Ready\n");
    if (regval & TSI_AXIM_RCNRDY)
        printf("\t[1]Read-Commnad No Ready\n");
    if (regval & TSI_AXIM_WCNRDY)
        printf("\t[0]Write-Commnad No Ready\n");
}

void tsi_init(void)
{
    struct tsi_dev *dev = &g_tsi_dev;

    /* initialize device structure */
    dev->flags = 0;
    dev->filter_number = 0;
    dev->irq_count = 0;
    dev->underrun_count = 0;
    dev->unsync_count = 0;
    dev->func = 0;
    dev->intr_status = 0;

    /* initialize interrupt setting */
    tsi_intr_init(dev);

    // enable this function in boot code. kernel driver doesn't do this!!
    // DM6000 also can inverse the clock!!
    // tsi_ext_inv, bit[4] TSI external clock select inverse clock
    *(volatile unsigned long *)0xbf004a5c = 0x010UL;

    /* enable TSI controller */
#if defined(BIG_ENDIAN)
    TSIREG(TSI_CTRL) =
        ((TSI_BURSTSIZE << TSI_BRSTSZE_OFT) | TSI_BENDIAN | TSI_M_DIRECT |
         TSI_M_DMA);
#else
    TSIREG(TSI_CTRL) =
        ((TSI_BURSTSIZE << TSI_BRSTSZE_OFT) | TSI_M_DIRECT | TSI_M_DMA);
#endif
}

int tsi_cmd(int argc, char *argv[])
{
    if (!strcmp(argv[0], "show"))
        tsi_stat();
    else if (!strcmp(argv[0], "start"))
        tsi_start(&g_tsi_dev);
    else if (!strcmp(argv[0], "stop"))
        tsi_stop(&g_tsi_dev);
    else if (!strcmp(argv[0], "mux"))
        return tsi_mux(argc, argv);
    else
        return ERR_PARM;

    return ERR_OK;
}

cmdt cmdt_tsi __attribute__ ((section("cmdt"))) =
{
"tsi", tsi_cmd, "tsi <show, start, stop, mux>"};
#endif
