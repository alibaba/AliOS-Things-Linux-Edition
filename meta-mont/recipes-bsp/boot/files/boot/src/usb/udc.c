/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file udc.c
*   \brief USB Device Controller Driver
*   \author Montage
*/
#ifdef CONFIG_UDC
//#define UDC_LOOPBACK
//#define FORCE_FULL_SPEED
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#ifdef __ECOS
#include <string.h>
#include <os_api.h>
#include <cyg/hal/hal_cache.h>  //HAL_FLUSH function
#include <stdio.h>              //sscanf
#include <cli_api.h>
#else
#include <common.h>
#include <arch/chip.h>
#endif
#include <udc.h>
#include <mt_udc.h>
#include <udc_api.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
enum usb_device_state
{
    /* NOTATTACHED isn't in the USB spec, and this state acts
     * the same as ATTACHED ... but it's clearer this way.
     */
    USB_STATE_NOTATTACHED = 0,

    /* chapter 9 and authentication (wireless) device states */
    USB_STATE_ATTACHED,
    USB_STATE_POWERED,          /* wired */
    USB_STATE_RECONNECTING,     /* auth */
    USB_STATE_UNAUTHENTICATED,  /* auth */
    USB_STATE_DEFAULT,          /* limited function */
    USB_STATE_ADDRESS,
    USB_STATE_CONFIGURED,       /* most functions */

    USB_STATE_SUSPENDED
};
enum ep0_state
{
    EP0_NONE,
    //EP0_SETUP,
    EP0_DATA,
    EP0_STATUS
};
enum ubuf_state
{
    UB_FREE = 0xadad,
    UB_ALLOC = 0xacac,
    UB_RELEASE = 0xabab,
    UB_OK = 0,
    UB_ERR_ID = 3,
    UB_HEAD_SZ = 0x40,
};
#define UBFLAG(ub) (ub->flag)
//#define UDC_DBG

#ifdef __ECOS
#define printf  diag_printf
#define dump_hex(a, b, c) { printd(a"\n"); diag_dump_buf_32bit(b ,c); }
#define dcache_flush_range(addr, len)  HAL_DCACHE_FLUSH((u32)addr, len)
#define dcache_inv_range    HAL_DCACHE_INVALIDATE
#define ubufq_lock() os_mutex_lock(ubuf_cfg.mutex)
#define ubufq_unlock() os_mutex_unlock(ubuf_cfg.mutex)
#else
#define ubufq_lock() ubuf_cfg.flag = irq_save()
#define ubufq_unlock() irq_restore(ubuf_cfg.flag)
#endif
#ifndef NULL
#define NULL ((void*)0)
#endif
#undef SWRST
#define SWRST         0xa0
#define UsbSoftRstSet()  (GPREG(SWRST) &= ~(1<<2))
#define UsbSoftRstClr()  (GPREG(SWRST) |= (1<<2))
#define mt_udc_start() (USBREG(USBCMD) |= 1)
#define uncached_addr(a) (((unsigned int)a)|0xa0000000)
#define cached_addr(a) (((unsigned int)a)&0x8fffffff)
#define usbdelay() {int i; for(i=0;i<100000;i++); }
#define TD_NEXT_PTR(td) ((void *)(le32_to_cpu(td->next)&~DTD_NEXT_TERMINATE))
#define TD_NEXT_ADD_TERM(td) (td->next|cpu_to_le32(DTD_NEXT_TERMINATE))
#define TD_NEXT_DEL_TERM(td) (td->next&~cpu_to_le32(DTD_NEXT_TERMINATE))
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct udc udc;
struct mt_dcr *dr_regs = (void *) USB_BASE;
struct usb_device_request setup_cmd;    //address align 4

#define UDC_QH_NUM 8
#define UDC_TD_NUM 106
struct udc_qh s_qh[UDC_QH_NUM] __attribute__ ((aligned(0x800)));
struct mt_td_ref s_td_ref[UDC_QH_NUM];
#define EP0_DES_NUM 1           //contorl pipe
#define EP1_DES_NUM 32          //bulk pipe
#define EP2_DES_NUM 6           //interrupt pipe
#define EP3_DES_NUM 2           //reserved
#if (2*EP0_DES_NUM + 2*EP1_DES_NUM + EP2_DES_NUM > UDC_TD_NUM)
#error "total descriptor number isn't enough"
#endif
struct ubuf_cfg ubuf_cfg;
struct ubuf ubuf_base[UDC_TD_NUM];
u32 udc_enable_intr = USB_STS_INT | USB_STS_ERR | USB_STS_PORT_CHANGE |
    USB_STS_SYS_ERR | USB_STS_RESET | USB_STS_SUSPEND;
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
int ubuf_enqueue(udc_queue * qp, struct ubuf *ub);
void ubuf_dequeue(udc_queue * qp, struct ubuf **rpp);
int udc_ep_rx(int ep_num, void *src, int len, void (*cb) (struct udc_cb_api *),
              void *arg);
int udc_ep_tx(int ep_num, void *src, int len, void (*cb) (struct udc_cb_api *),
              void *arg);
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
/*!
 * function: ubuf_get
 *
 *  \brief
 *  \param
 *  \return
 */
void *ubuf_get(void)
{
    struct ubuf *ub;

    ubufq_lock();
    ubuf_dequeue(&ubuf_cfg.queue, &ub);
    ubufq_unlock();

    if (ub == 0)
    {
        printf("%s:empty\n", __func__);
        goto out;
    }

    if (UB_FREE != UBFLAG(ub))
    {
        printf("%s:ub=0x%08x, err flag:%x\n", __func__, ub, UBFLAG(ub));
        goto out;
    }

    ub->next = 0;
    UBFLAG(ub) = UB_ALLOC;

  out:
    return ub;
}

/*!
 * function: ubuf_put
 *
 *  \brief
 *  \param
 *  \return
 */
int ubuf_put(void *desc)
{
    struct ubuf *ub = desc;

    if (!ub ||
        ((UB_ALLOC != UBFLAG(ub)) &&
         (UB_RELEASE != UBFLAG(ub)) && (UB_FREE != UBFLAG(ub))))
    {
        printf("%s:ub=%x id=%x error\n", __func__, ub, (ub ? UBFLAG(ub) : 0));
        goto err;
    }
    if (UB_FREE == UBFLAG(ub))
    {
        printf("%s %x free twice\n", __func__, ub);
        goto out;
    }

    UBFLAG(ub) = UB_FREE;

    ubufq_lock();
    ubuf_enqueue(&ubuf_cfg.queue, ub);
    ubufq_unlock();
  out:
    return UB_OK;
  err:
    return -UB_ERR_ID;
}

/*!
 * function: ubuf_init
 *
 *  \brief
 *  \param
 *  \return
 */
int ubuf_init(struct udc_api *uapi, void *base, int num)
{
    struct ubuf *ub;
    short n;

    ubuf_cfg.queue.max = num;
    ubuf_cfg.queue.count = 0;
    ubuf_cfg.num = num;
    ubuf_cfg.size = sizeof (struct ubuf);

    ub = ubuf_cfg.base = base;
    for (n = 0; n < num; n++)
    {
#ifdef UDC_DBG
        printf("ub:%x\n", ub);
#endif
        UBFLAG(ub) = UB_RELEASE;
        ubuf_put(ub);
        ub = (struct ubuf *) ((unsigned int) ub + ubuf_cfg.size);
    }

    for (n = 0; n < USB_MAX_EP_NUM; n++)
    {
        s_td_ref[2 * n + MT_EP_OUT].des_used = 0;
        s_td_ref[2 * n + MT_EP_IN].des_used = 0;
    }

    return 0;
}

/*!
 * function: ubuf_enqueue
 *
 *  \brief
 *  \param
 *  \return
 */
int ubuf_enqueue(udc_queue * qp, struct ubuf *ub)
{
    if (!qp->count)
        qp->head = qp->tail = ub;
    else
    {
        if (qp->max > qp->count)
        {
            qp->tail->next = (struct ubuf *) ub;
            qp->tail = ub;
        }
        else
            return -1;
    }

    ub->next = 0;
    qp->count++;

    return 0;
}

/*!
 * function: ubuf_dequeue
 *
 *  \brief
 *  \param
 *  \return
 */
void ubuf_dequeue(udc_queue * qp, struct ubuf **rpp)
{
    if (0 >= qp->count)
        *rpp = 0;
    else
    {
        *rpp = qp->head;
        qp->head = (struct ubuf *) qp->head->next;
        (*rpp)->next = 0;
        qp->count--;
    }
}

/*!
 * function: ubuf_state
 *
 *  \brief
 *  \param
 *  \return
 */
int ubuf_state(void)
{
    struct ubuf *ub;
    int i, counter[3];
    int flag;

    memset(&counter[0], 0, sizeof (counter));

    for (i = 0, ub = ubuf_cfg.base; i < ubuf_cfg.num;
         i++, ub = (struct ubuf *) ((char *) ub + ubuf_cfg.size))
    {
        flag = UBFLAG(ub);
        if (UB_FREE == flag)
            counter[0]++;
        else if (UB_ALLOC == flag)
            counter[1]++;
        else
        {
            counter[2]++;
            printf("ubuf:%x unknown flag=%04x", (unsigned int) ub, flag);
        }
    }
    printf("[Total %d ubufs]\n", ubuf_cfg.num);
    printf("Free: %d , Used: %d , Others: %d \n", counter[0], counter[1],
           counter[2]);
    printf("idle_queue: %d\n", ubuf_cfg.queue.count);
    return 0;
}

/*!
 * function: mt_udc_link_td
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_link_td(int ep_offset)
{
    struct udc_qh *qh;
    struct udc_td *pre_td;
    struct udc_td *fst_td;
    struct udc_td *td;
    struct mt_td_ref *r = &s_td_ref[ep_offset];
    int i;

    qh = (void *) uncached_addr(&s_qh[ep_offset]);
    // first TD
    td = (void *) uncached_addr(ubuf_get());
    qh->next = td->next = cpu_to_le32((u32) td | DTD_NEXT_TERMINATE);
    r->td_head = r->td_tail = td;
    // insert others TD
    fst_td = td;
    for (i = 1; i < r->des_num; i++)
    {
        pre_td = td;
        td = (void *) uncached_addr(ubuf_get());
        pre_td->next = cpu_to_le32((u32) td | DTD_NEXT_TERMINATE);
    }
    td->next = cpu_to_le32((u32) fst_td | DTD_NEXT_TERMINATE);
}

/*!
 * function: mt_udc_alloc_td
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_alloc_td(struct udc_api *uapi)
{
    struct enpt_list *eplist = uapi->enptlist;
    struct usb_endpoint_descriptor *epdesc;
    u32 ep, dir;

    /* ep0 */
    mt_udc_link_td(2 * 0 + MT_EP_OUT);
    mt_udc_link_td(2 * 0 + MT_EP_IN);
    /* other eps */
    while (eplist)
    {
        epdesc = eplist->desc;
        ep = epdesc->bEndpointAddress & (USB_MAX_EP_NUM - 1);
        dir = (epdesc->bEndpointAddress & USB_DIR_IN) ? MT_EP_IN : MT_EP_OUT;
        mt_udc_link_td(2 * ep + dir);
        eplist = eplist->next;
    }
}

/*!
 * function: mt_udc_queue_td
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_queue_td(int ep_offset, struct udc_td *td, u32 bitmask)
{
    u32 temp, tmp_stat;
    struct udc_qh *qh = (void *) uncached_addr(&s_qh[ep_offset]);
    struct mt_td_ref *r = &s_td_ref[ep_offset];

    /* check if the pipe is empty */
    if (r->des_used)
    {
        //usbdbg("r->td_tail=0x%08x", r->td_tail);
        /* Add td to the end */
        r->td_tail->next = TD_NEXT_DEL_TERM(r->td_tail);        //remove terminate bit
        /* move tail pointer */
        r->td_tail = TD_NEXT_PTR(r->td_tail);
        /* Read prime bit, if 1 goto done */
        if (udc_readl(&dr_regs->endptprime) & bitmask)
            goto out;

        do
        {
            /* Set ATDTW bit in USBCMD */
            temp = udc_readl(&dr_regs->usbcmd);
            udc_writel(&dr_regs->usbcmd, temp | USB_CMD_ATDTW);

            /* Read correct status bit */
            tmp_stat = udc_readl(&dr_regs->endptstatus) & bitmask;

        }
        while (!(udc_readl(&dr_regs->usbcmd) & USB_CMD_ATDTW));

        /* Write ATDTW bit to 0 */
        temp = udc_readl(&dr_regs->usbcmd);
        udc_writel(&dr_regs->usbcmd, temp & ~USB_CMD_ATDTW);

        if (tmp_stat)
            goto out;
    }

    /* clear active and halt flag */
    qh->token &= cpu_to_le32(~(EP_QUEUE_HEAD_STATUS_ACTIVE
                               | EP_QUEUE_HEAD_STATUS_HALT));

    /* link td */
    qh->next = cpu_to_le32(uncached_addr(td));

    /* Prime endpoint by writing 1 to ENDPTPRIME */
    udc_writel(&dr_regs->endptprime, bitmask);
  out:
    return;
}

/*!
 * function: mt_udc_reclaim_td
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_reclaim_td(int ep_offset)
{
    int len;
    struct udc_td *td;
    struct udc_cb_api *cbi;
    struct mt_td_ref *r = &s_td_ref[ep_offset];
    struct udc_td *end = TD_NEXT_PTR(r->td_tail);

    td = r->td_head;
    //success : active(7) halted(6) transaction_error(5) data_buffer_error(3) are both zeros
    //usbdbg("ep_offset=%d", ep_offset);
    while (r->des_used && ((le32_to_cpu(td->token) & 0xe8) == 0))
    {
        cbi = (void *) (&((struct ubuf *) td)->rxbuf);
        /* calculate Rx length */
        if ((0 == ep_offset % 2) && (ep_offset > 1))
        {                       // Rx EP1 ~ MAX
            len = ((le32_to_cpu(td->token) & 0x7FFF0000) >> 16);
            //usbdbg("td=0x%08x\n", td);
            //usbdbg("td->rxbuflen=%d\n", cbi->rxbuflen);
            cbi->rxbuflen -= len;
            if (cbi->rxbuflen == 0)
                usberr("%08x %08x", td->token,
                       !(td->token & cpu_to_le32(0xe8)));
            //usbdbg("td->rxbuflen=%d\n", cbi->rxbuflen);
        }
        /* reclaim td */
        td->next = TD_NEXT_ADD_TERM(td);
        //usbdbg("td->next=0x%08x\n", td->next);
        r->des_used--;

        /* execute callback function */
        if (cbi->cb)
        {
            //usbdbg("cb=0x%08x\n", cbi->cb);
            ((int (*)(void *)) cbi->cb) (cbi);
        }

        td = TD_NEXT_PTR(td);
        //usbdbg("c ");
        if (td == end)
            break;
    }
    /* move head pointer */
    if (r->des_used)
        r->td_head = td;
    else
        r->td_head = r->td_tail;
}

/*!
 * function: mt_udc_free_ep_td
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_free_ep_td(int ep_offset)
{
    int i;
    struct mt_td_ref *r = &s_td_ref[ep_offset];
    struct udc_td *td = r->td_head;
    struct udc_cb_api *cbi;

    for (i = 0; i < r->des_used; i++)
    {
        cbi = (void *) (&((struct ubuf *) td)->rxbuf);

        /* avoid upper layer to parse data */
        cbi->rxbuflen = 0;

        /* execute callback function */
        if (ep_offset > 1)      //non ep0 need call upper callback
            if (cbi->cb)
            {
                cbi->status = UDC_TRS_CANCEL;
                ((int (*)(void *)) cbi->cb) (cbi);
            }

        /* reclaim td */
        td->next = TD_NEXT_ADD_TERM(td);

        /* next td */
        td = TD_NEXT_PTR(td);
    }
    r->td_head = r->td_tail;
    r->des_used = 0;
}

/*!
 * function: mt_udc_free_tds
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_free_tds(struct udc_api *uapi)
{
    struct enpt_list *eplist = uapi->enptlist;
    struct usb_endpoint_descriptor *epdesc;
    u32 ep, dir;

    /* ep0 */
    mt_udc_free_ep_td(2 * 0 + MT_EP_OUT);
    mt_udc_free_ep_td(2 * 0 + MT_EP_IN);
    /* other eps */
    while (eplist)
    {
        epdesc = eplist->desc;
        ep = epdesc->bEndpointAddress & (USB_MAX_EP_NUM - 1);
        dir = (epdesc->bEndpointAddress & USB_DIR_IN) ? MT_EP_IN : MT_EP_OUT;
        mt_udc_free_ep_td(2 * ep + dir);
        eplist = eplist->next;
    }
}

/*!
 * function: mt_udc_init_qhs
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_init_qhs(void)
{
    int i;
    struct udc_qh *qh;
    for (i = 0; i < UDC_QH_NUM; i++)
    {
        qh = &s_qh[i];
        memset(qh, 0, MT_QH_SIZE);
        qh->next = cpu_to_le32(DTD_NEXT_TERMINATE);
        dcache_flush_range(qh, MT_TD_SIZE);
    }
}

/*!
 * function: mt_udc_qh_setup
 *
 *  \brief
 *  \param 1.epnum 2.dir 3.ep type 4.max_pkt 5.zlt 6.multi
 *  \return
 */
void mt_udc_qh_setup(char ep_num, unsigned char dir, unsigned char ep_type,
                     unsigned int max_pkt_len, unsigned int zlt,
                     unsigned char mult)
{
    int tmp;
    struct udc_qh *qh = (void *) uncached_addr(&s_qh[2 * ep_num + dir]);

    /* set the Endpoint Capabilites in QH */
    switch (ep_type)
    {
        case USB_ENDPOINT_XFER_CONTROL:
            /* Interrupt On Setup (IOS). for control ep  */
            tmp = (max_pkt_len << QH_MAX_PKT_LEN_POS)
                | (1 << QH_INTR_ON_SETUP_POS);
            break;
        case USB_ENDPOINT_XFER_ISOC:
            tmp = (max_pkt_len << QH_MAX_PKT_LEN_POS) | (mult << QH_MULT_POS);
            break;
        case USB_ENDPOINT_XFER_BULK:
        case USB_ENDPOINT_XFER_INT:
            tmp = (max_pkt_len << QH_MAX_PKT_LEN_POS);
            break;
        default:
            usberr("error ep type is %d", ep_type);
            return;
    }

    if (zlt)
        tmp |= (1 << QH_ZLT_POS);

    //usbdbg("tmp=0x%x\n",tmp);
    qh->setup = cpu_to_le32(tmp);       //non-cached address
}

/*!
 * function: mt_udc_setup_intr
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_setup_intr(void)
{
    /* enable controller interrupt */
    udc_writel(&dr_regs->usbintr, udc_enable_intr);
}

/*!
 * function: mt_udc_port_change
 *
 *  \brief
 *  \param
 *  \return
 */
/* Process a port change interrupt */
static void mt_udc_port_change(struct udc *udc)
{
    u32 speed;

    /* Bus resetting is finished */
    if (!(udc_readl(&dr_regs->portsc) & PORTSCX_PORT_RESET))
    {
        /* Get the speed */
        speed = (udc_readl(&dr_regs->portsc) & PORTSCX_PORT_SPEED_MASK);
        switch (speed)
        {
            case PORTSCX_PORT_SPEED_HIGH:
                udc->speed = USB_SPEED_HIGH;
                break;
            case PORTSCX_PORT_SPEED_FULL:
                udc->speed = USB_SPEED_FULL;
                break;
            case PORTSCX_PORT_SPEED_LOW:
                udc->speed = USB_SPEED_LOW;
                break;
            default:
                udc->speed = USB_SPEED_RESERVED;
                break;
        }
    }

#if 0
    /* Update USB state */
    if (!udc->resume_state)
        udc->usb_state = USB_STATE_DEFAULT;
#endif
}

/*!
 * function: mt_udc_ep_status
 *
 *  \brief
 *  \param
 *  \return
 */
static int mt_udc_ep_status(unsigned int windex)
{
    u32 epctrl;
    u8 dir = windex & USB_ENDPOINT_DIR_MASK;
    u32 ep_num = ((windex & USB_ENDPOINT_NUMBER_MASK) << 1) + (dir ? 1 : 0);

    epctrl = udc_readl(&dr_regs->endptctrl[ep_num]);
    if (dir)
        return (epctrl & EPCTRL_TX_EP_STALL) ? 1 : 0;
    else
        return (epctrl & EPCTRL_RX_EP_STALL) ? 1 : 0;
}

/*!
 * function: mt_udc_ep0_stall
 *
 *  \brief Set protocol stall on ep0, protocol stall will automatically be
 *         cleared on new transaction
 *  \param
 *  \return
 */
static void mt_udc_ep0_stall(void)
{
    u32 tmp_epctrl;

    /* must set tx and rx to stall at the same time */
    tmp_epctrl = udc_readl(&dr_regs->endptctrl[0]);
    tmp_epctrl |= (EPCTRL_TX_EP_STALL | EPCTRL_RX_EP_STALL);
    udc_writel(&dr_regs->endptctrl[0], tmp_epctrl);
}

/*!
 * function: mt_udc_ep_stall
 *
 *  \brief
 *  \param
 *  \return
 */
static void mt_udc_ep_stall(unsigned char ep_num, unsigned char dir,
                            unsigned char set)
{
    unsigned int tmp_epctrl;

    tmp_epctrl = udc_readl(&dr_regs->endptctrl[ep_num]);
    if (set)
    {
        if (dir)                //IN endpoint
            tmp_epctrl |= EPCTRL_TX_EP_STALL;
        else                    //OUT endpoint
            tmp_epctrl |= EPCTRL_RX_EP_STALL;
    }
    else
    {
        if (dir)
        {                       //IN endpoint
            tmp_epctrl |= EPCTRL_TX_DATA_TOGGLE_RST;
            tmp_epctrl &= ~EPCTRL_TX_EP_STALL;
        }
        else
        {                       //OUT endpoint
            tmp_epctrl |= EPCTRL_RX_DATA_TOGGLE_RST;
            tmp_epctrl &= ~EPCTRL_RX_EP_STALL;
        }
    }
    udc_writel(&dr_regs->endptctrl[ep_num], tmp_epctrl);
}

/*!
 * function: mt_udc_ep_setup
 *
 *  \brief
 *  \param
 *  \return
 */
static void mt_udc_ep_setup(unsigned char ep_num, unsigned char dir,
                            unsigned char ep_type)
{
    unsigned int tmp_epctrl;

    tmp_epctrl = udc_readl(&dr_regs->endptctrl[ep_num]);
    if (dir)
    {                           //IN endpoint
        if (ep_num)
            tmp_epctrl |= EPCTRL_TX_DATA_TOGGLE_RST;
        tmp_epctrl |= EPCTRL_TX_ENABLE;
        tmp_epctrl |= ((unsigned int) (ep_type) << EPCTRL_TX_EP_TYPE_SHIFT);
    }
    else
    {                           //OUT endpoint
        if (ep_num)
            tmp_epctrl |= EPCTRL_RX_DATA_TOGGLE_RST;
        tmp_epctrl |= EPCTRL_RX_ENABLE;
        tmp_epctrl |= ((unsigned int) (ep_type) << EPCTRL_RX_EP_TYPE_SHIFT);
    }
    udc_writel(&dr_regs->endptctrl[ep_num], tmp_epctrl);
}

/*!
 * function: mt_udc_init
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_init(void)
{
    u32 tmp;

    /* Stop USB controller */
    tmp = udc_readl(&dr_regs->usbcmd);
    tmp &= ~USB_CMD_RUN_STOP;
    udc_writel(&dr_regs->usbcmd, tmp);

    /* Reset USB controller */
    tmp = udc_readl(&dr_regs->usbcmd);
    tmp |= USB_CMD_CTRL_RESET;
    udc_writel(&dr_regs->usbcmd, tmp);

    /* Wait for reset to complete */
    while (udc_readl(&dr_regs->usbcmd) & USB_CMD_CTRL_RESET) ;

    /* Set the controller as device mode */
    tmp = udc_readl(&dr_regs->usbmode);
    tmp |= USB_MODE_CTRL_MODE_DEVICE;

    /* Disable Setup Lockout */
    tmp |= USB_MODE_SETUP_LOCK_OFF;
    udc_writel(&dr_regs->usbmode, tmp);

    /* force Full Speed */
#ifdef FORCE_FULL_SPEED
    unsigned int tmp_epctrl;
    tmp_epctrl = udc_readl(&dr_regs->portsc);
    tmp_epctrl |= PORTSCX_PORT_FORCE_FULL_SPEED;
    udc_writel(&dr_regs->portsc, tmp_epctrl);
#endif

    /* Clear the setup status */
    udc_writel(&dr_regs->usbsts, 0);

    /* assign qh */
    udc_writel(&dr_regs->endptlistaddr, s_qh);

    usbinf("[qh_base = 0x%8x] reg is 0x%8x",
           s_qh, udc_readl(&dr_regs->endptlistaddr));
}

/*!
 * function: mt_udc_reset_queue
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_reset_queue(void)
{
}

/*!
 * function: mt_udc_set_addr_done
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_set_addr_done(struct udc_cb_api *cbi)
{
    u32 addr = (u32) cbi->arg;
    usbdbg("addr = %d\n", addr);
    if (udc.ep0_state != EP0_STATUS)
        usberr("error ep0 status");

    if (udc.usb_state == USB_STATE_ADDRESS)
    {
        udc_writel(&dr_regs->devaddr, addr << USB_DEVICE_ADDRESS_BIT_POS);
        udc.usb_state = USB_STATE_CONFIGURED;
    }
    udc.ep0_state = EP0_NONE;
}

/*!
 * function: mt_udc_reset
 *
 *  \brief
 *  \param
 *  \return
 */
void mt_udc_reset(struct udc_api *uapi)
{
    u32 tmp;

    /* clear the device address */
    tmp = udc_readl(&dr_regs->devaddr);
    udc_writel(&dr_regs->devaddr, tmp & ~USB_DEVICE_ADDRESS_MASK);

    /* Clear all the setup token semaphores */
    tmp = udc_readl(&dr_regs->endptsetupstat);
    udc_writel(&dr_regs->endptsetupstat, tmp);

    /* Clear all the endpoint complete status bits */
    tmp = udc_readl(&dr_regs->endptcomplete);
    udc_writel(&dr_regs->endptcomplete, tmp);

    /* wait prime status */
    while (udc_readl(&dr_regs->endptprime)) ;

    /* Write 1s to the flush register */
    udc_writel(&dr_regs->endptflush, 0xffffffff);

    if (udc_readl(&dr_regs->portsc) & PORTSCX_PORT_RESET)
    {
        /* reclaim used descriptor */
        mt_udc_free_tds(uapi);
        //usbdbg("USB bus reset");
    }
    else
    {
        //usbdbg("Controller reset");
        /* Stop USB controller */
        tmp = udc_readl(&dr_regs->usbcmd);
        tmp &= ~USB_CMD_RUN_STOP;
        udc_writel(&dr_regs->usbcmd, tmp);

        /* Reset USB controller */
        tmp = udc_readl(&dr_regs->usbcmd);
        tmp |= USB_CMD_CTRL_RESET;
        udc_writel(&dr_regs->usbcmd, tmp);

        /* Wait for reset to complete */
        while (udc_readl(&dr_regs->usbcmd) & USB_CMD_CTRL_RESET) ;

        /* qhs initialization */
        mt_udc_init_qhs();

        /* ep0 qh setup */
        mt_udc_qh_setup(0, MT_EP_IN, USB_ENDPOINT_XFER_CONTROL,
                        CTRL_MAXPKTSIZE, 1, 0);
        mt_udc_qh_setup(0, MT_EP_OUT, USB_ENDPOINT_XFER_CONTROL,
                        CTRL_MAXPKTSIZE, 1, 0);

        /* init montage USB device controller */
        mt_udc_init();

#if 1
        /* init ubuf */
        ubuf_init(uapi, (void *) ubuf_base, UDC_TD_NUM);
#else
        /* reclaim used descriptor */
        mt_udc_free_tds(uapi);
#endif

        /* init descriptors */
        mt_udc_alloc_td(uapi);

        /* setup USB interrupt */
        mt_udc_setup_intr();

        /* Enable controller */
        mt_udc_start();
    }

    /* change state: return to default */
    udc.ep0_state = EP0_NONE;
    udc.usb_state = USB_STATE_NOTATTACHED;
}

/*!
 * function: udc_ep_rx
 *
 *  \brief
 *  \param
 *  \return
 */
int udc_ep_rx(int ep_num, void *src, int len, void (*cb) (struct udc_cb_api *),
              void *arg)
{
    u32 addr;
    u32 ep_offset = 2 * ep_num + MT_EP_OUT;
    struct mt_td_ref *r = &s_td_ref[ep_offset];
    struct udc_td *td;

    //usbdbg("%dr ",udc.ep0_state);
    //usbdbg("ep_num=%d, *src=0x%08x, len=%d qh=0x%08x cb=0x%08x arg=0x%08x",ep_num, src, len, qh, cb, arg);

    /* no free descriptor */
    if (r->des_used >= r->des_num)
    {
        usberr("ep%d no descriptor", ep_num);
        return -1;
    }

    /* find a TD */
    if (r->des_used == 0)
        td = r->td_tail;
    else
        td = TD_NEXT_PTR(r->td_tail);
    //usbdbg("td=0x%08x\n",td);

    /* build a TD */
    ((struct ubuf *) td)->cb = cb;
    ((struct ubuf *) td)->arg = arg;
    ((struct ubuf *) td)->status = UDC_TRS_SUCCESS;
    ((struct ubuf *) td)->rxbuf = (u32) src;
    ((struct ubuf *) td)->rxbuflen = len;
    //td->next = TD_NEXT_ADD_TERM(td); //set terminate bit to 1
    td->token =
        cpu_to_le32(len << DTD_LENGTH_BIT_POS | DTD_IOC | DTD_STATUS_ACTIVE);
    addr = (u32) src;
    td->buffer[0] = cpu_to_le32(addr);
    addr &= (~0xFFF);
    td->buffer[1] = cpu_to_le32(addr + 0x1000);
    td->buffer[2] = cpu_to_le32(addr + 0x2000);
    td->buffer[3] = cpu_to_le32(addr + 0x3000);
    td->buffer[4] = cpu_to_le32(addr + 0x4000);

    /* invalidate data buffer */
    if (len)
        dcache_inv_range(src, len);

    /* flush td */
    dcache_flush_range(td, MT_TD_SIZE);

    /* link td */
    mt_udc_queue_td(ep_offset, td, 1 << (ep_num));

    r->des_used++;
    return 0;
}

/*!
 * function: udc_ep_tx
 *
 *  \brief tramsit data in some endpoint
 *  \param
 *  \return
 */
int udc_ep_tx(int ep_num, void *src, int len, void (*cb) (struct udc_cb_api *),
              void *arg)
{
    u32 addr;
    u32 ep_offset = 2 * ep_num + MT_EP_IN;
    struct mt_td_ref *r = &s_td_ref[ep_offset];
    struct udc_td *td;

    //usbdbg("%dt ",udc.ep0_state);
    //usbdbg("ep_num=%d, *src=0x%08x, len=%d qh=0x%08x cb=0x%08x arg=0x%08x",ep_num, src, len, qh, cb, arg);

    /* no free descriptor */
    if (r->des_used >= r->des_num)
    {
        usberr("ep%d no descriptor", ep_num);
        return -1;
    }

    /* find a TD */
    if (r->des_used == 0)
        td = r->td_tail;
    else
        td = TD_NEXT_PTR(r->td_tail);
    //usbdbg("td=0x%08x\n",td);

    /* build a TD */
    ((struct ubuf *) td)->cb = cb;
    ((struct ubuf *) td)->arg = arg;
    ((struct ubuf *) td)->status = UDC_TRS_SUCCESS;
    ((struct ubuf *) td)->rxbuf = (u32) src;
    ((struct ubuf *) td)->rxbuflen = len;
    //td->next = TD_NEXT_ADD_TERM(td); //set terminate bit to 1
    td->token =
        cpu_to_le32(len << DTD_LENGTH_BIT_POS | DTD_IOC | DTD_STATUS_ACTIVE);
    addr = (u32) src;
    td->buffer[0] = cpu_to_le32(addr);
    addr &= (~0xFFF);
    td->buffer[1] = cpu_to_le32(addr + 0x1000);
    td->buffer[2] = cpu_to_le32(addr + 0x2000);
    td->buffer[3] = cpu_to_le32(addr + 0x3000);
    td->buffer[4] = cpu_to_le32(addr + 0x4000);

    /* flush data buffer */
    if (len)
        dcache_flush_range(src, len);

    /* flush td */
    dcache_flush_range(td, MT_TD_SIZE);

    /* link td */
    mt_udc_queue_td(ep_offset, td, 1 << (ep_num + 16));

    r->des_used++;
    return 0;
}

/*!
 * function: ep0_status_done
 *
 *  \brief
 *  \param
 *  \return
 */
static void ep0_status_done(struct udc_cb_api *cbi)
{
    if (udc.ep0_state != EP0_STATUS)
        usberr("");

    udc.ep0_state = EP0_NONE;
}

/*!
 * function: ep0_rx_done
 *
 *  \brief
 *  \param
 *  \return
 */
static void ep0_rx_done(void)
{
    /* change state */
    udc.ep0_state = EP0_STATUS;

    /* send ZLP in status stage */
    udc_ep_tx(0, 0, 0, ep0_status_done, NULL);
}

/*!
 * function: ep0_tx_done
 *
 *  \brief
 *  \param
 *  \return
 */
static void ep0_tx_done(void)
{
    //usbdbg("");
    /* change state */
    udc.ep0_state = EP0_STATUS;

    /* receive ZLP in status stage */
    udc_ep_rx(0, 0, 0, ep0_status_done, NULL);
}

/*!
 * function: udc_complete_handler
 *
 *  \brief
 *  \param
 *  \return
 */
static void udc_complete_handler(void)
{
    u32 i, bitmask, com = udc_readl(&dr_regs->endptcomplete);

    //usbdbg("com=0x%08x\n", com);
    if (!com)
        return;

    /* Clear the bits in the register */
    udc_writel(&dr_regs->endptcomplete, com);

    /* scan endpt */
    for (i = 0; i < USB_MAX_EP_NUM; i++)
    {
        bitmask = 1 << i;
        if (i == 0)
        {                       //ep0
            if (com & bitmask)
            {                   //Rx
                if (udc.ep0_state == EP0_DATA)
                    ep0_rx_done();
                mt_udc_reclaim_td(2 * i + MT_EP_OUT);
            }
            else if (com & (bitmask << 16))
            {                   //Tx
                if (udc.ep0_state == EP0_DATA)
                    ep0_tx_done();
                mt_udc_reclaim_td(2 * i + MT_EP_IN);
            }
        }
        else
        {
            if (com & bitmask)  //Rx
                mt_udc_reclaim_td(2 * i + MT_EP_OUT);
            if (com & (bitmask << 16))  //Tx
                mt_udc_reclaim_td(2 * i + MT_EP_IN);
        }
    }
}

/*!
 * function: udc_ep0_get_wrong_setup
 *
 *  \brief
 *  \param
 *  \return
 */
void udc_ep0_get_wrong_setup(struct usb_device_request *setup)
{
    u8 *usbcmd = (u8 *) setup;
    usberr("not support this setup cmd");
    usberr("Setup CMD(0x%08x):%02x %02x %02x %02x %02x %02x %02x %02x\n",
           usbcmd,
           usbcmd[0], usbcmd[1], usbcmd[2], usbcmd[3],
           usbcmd[4], usbcmd[5], usbcmd[6], usbcmd[7]);
}

/*!
 * function: udc_get_status
 *
 *  \brief
 *  \param
 *  \return
 */
static int udc_get_status(struct udc_api *uapi,
                          struct usb_device_request *usbcmd)
{
    unsigned char request_type = usbcmd->bmRequestType;
    unsigned int tmp = 0;
    unsigned int idx = le16_to_cpu(usbcmd->wIndex);

    switch (request_type & USB_REQ_RECIPIENT_MASK)
    {
        case USB_RECIP_DEVICE:
            /* Get device status */
            // always bus powered
        case USB_RECIP_INTERFACE:
            /* Get interface status */
            // no remote wake-up support
            break;
        case USB_RECIP_ENDPOINT:
            /* Get endpoint status */
            if (mt_udc_ep_status(idx))
                tmp = 1;
            break;
        default:
            goto failed;
            break;
    }
    tmp = cpu_to_le16(tmp);
    udc_ep_tx(0, &tmp, 2, NULL, NULL);
    return 0;
  failed:
    udc_ep0_get_wrong_setup(usbcmd);
    mt_udc_ep0_stall();
    return -1;
}

/*!
 * function: udc_get_descriptor
 *
 *  \brief
 *  \param
 *  \return
 */
static int udc_get_descriptor(struct udc_api *uapi,
                              struct usb_device_request *usbcmd)
{
    void *addr;
    int idx, len = le16_to_cpu(usbcmd->wLength);
    struct total_string_desc *str;
    struct udc *udc;

    switch (usbcmd->wValue & 0x0f)
    {
        case USB_DT_DEVICE:
            len = min(len, sizeof (struct usb_device_descriptor));
            addr = uapi->dev_desc;
            break;
        case USB_DT_DEVICE_QUALIFIER:
            len = min(len, sizeof (struct usb_qualifier_descriptor));
            addr = uapi->dev_qualifier_desc;
            //dump_hex("device qualifier", addr, len);
            break;
        case USB_DT_CONFIGURATION:
            udc = uapi->dev;
            uapi->set_enpt_desc_by_spd((udc->speed == USB_SPEED_HIGH) ? 1 : 0);
            len = min(len, uapi->cfg_len);
            addr = uapi->cfg_desc;
            break;
        case USB_DT_OTHER_SPEED_CONFIGURATION:
            udc = uapi->dev;
            uapi->set_enpt_desc_by_spd((udc->speed == USB_SPEED_HIGH) ? 0 : 1);
            len = min(len, uapi->cfg_len);
            addr = uapi->cfg_desc;
            break;
#if 0
        case USB_DT_INTERFACE:
            len = min(len, sizeof (struct usb_interface_descriptor));
            addr = &boot_dev_desc.interface_desc;
            break;
        case USB_DT_ENDPOINT:
            len = min(len, sizeof (struct usb_endpoint_descriptor));
            addr = &boot_dev_desc.endpoint_desc;
            break;
#endif
        case USB_DT_STRING:
            idx = le16_to_cpu(usbcmd->wValue) & 0xff;
            if (idx == 0xee)    //get Microsoft OS String Descriptor
                goto failed;
            str = (struct total_string_desc *) (uapi->str_desc);
            len = min(len, str[idx].string_desc.bLength);
            addr = &str[idx];
            //usbdbg("idx=%d,size=%d\n",idx, len);
            break;
        default:
            usberr("not support this request");
            goto failed;
    }
    udc_ep_tx(0, addr, len, NULL, NULL);
    return 0;
  failed:
    udc_ep0_get_wrong_setup(usbcmd);
    mt_udc_ep0_stall();
    return -1;
}

/*!
 * function: udc_get_interface
 *
 *  \brief
 *  \param
 *  \return
 */
static int udc_get_interface(struct udc_api *uapi,
                             struct usb_device_request *usbcmd)
{
    unsigned char tmp = 0;
    // no alternate interface setting support
    //tmp = cpu_to_le16(tmp);
    udc_ep_tx(0, &tmp, 1, NULL, NULL);
    return 0;
//failed:
//    udc_ep0_get_wrong_setup(usbcmd);
//    mt_udc_ep0_stall();
//    return -1;
}

/*!
 * function: udc_ep0setup
 *
 *  \brief
 *  \param
 *  \return
 */
static void udc_ep0setup(struct udc_api *uapi)
{
    u32 tmp;
    u32 usb_address;
    u32 ep, dir, type, maxpkt;
    struct usb_device_request *usbcmd;
    struct udc_qh *qh;
    struct udc *udc = uapi->dev;
    struct enpt_list *eplist;
    struct usb_endpoint_descriptor *epdesc;

    /* setup stage completed by HW; now go to data stage */
    ifwarn(udc->ep0_state != EP0_NONE, "found wrong ep0 state:%d",
           udc->ep0_state);
    udc->ep0_state = EP0_DATA;

    qh = (void *) uncached_addr(&s_qh[MT_EP_OUT]);      //ep0 rx

    /* Clear bit in ENDPTSETUPSTAT */
    //tmp = udc_readl(&dr_regs->endptsetupstat);
    udc_writel(&dr_regs->endptsetupstat, 1);
    // check bit be cleared
    while ((udc_readl(&dr_regs->endptsetupstat) & 1) == 1) ;

    /* Tripwire mechanism to ensure a setup packet is extracted
     * without being corrupted by another incoming setup packet */

    /* while a hazard exists when setup package arrives */
    do
    {
        /* Set Setup Tripwire */
        tmp = udc_readl(&dr_regs->usbcmd);
        udc_writel(&dr_regs->usbcmd, tmp | USB_CMD_SUTW);

        /* Copy the setup packet to local buffer */
        memcpy(&setup_cmd, qh->setup_buffer, 8);
    }
    while (!(udc_readl(&dr_regs->usbcmd) & USB_CMD_SUTW));

    /* Clear Setup Tripwire */
    tmp = udc_readl(&dr_regs->usbcmd);
    udc_writel(&dr_regs->usbcmd, tmp & ~USB_CMD_SUTW);

    usbcmd = &setup_cmd;
    //usbdbg("+udc_ep0setup()");

    //usbdbg("Setup CMD(0x%08x):%02x %02x %02x %02x %02x %02x %02x %02x\n",
    //        usbcmd,
    //        usbcmd[0],usbcmd[1],usbcmd[2],usbcmd[3],
    //        usbcmd[4],usbcmd[5],usbcmd[6],usbcmd[7]);

    /* reset ep0 queue */
    udc_writel(&dr_regs->endptflush, 0x10001);

    if ((usbcmd->bmRequestType & USB_REQ_TYPE_MASK) == USB_TYPE_STANDARD)
    {                           //standard request
        /* 2. parse SETUP packet and enter DATA stage */
        /* 2.1 check D2H/H2D command */
        if (usbcmd->bmRequestType & USB_REQ_DIRECTION_MASK)     //D2H
        {
            switch (usbcmd->bRequest)
            {
                case USB_REQ_GET_STATUS:
                    //usb request failed then ep0 stall and ep0_state return to initial state
                    if (udc_get_status(uapi, usbcmd))
                        udc->ep0_state = EP0_NONE;
                    break;
                case USB_REQ_GET_DESCRIPTOR:
                    //usb request failed then ep0 stall and ep0_state return to initial state
                    if (udc_get_descriptor(uapi, usbcmd))
                        udc->ep0_state = EP0_NONE;
                    break;
                case USB_REQ_GET_INTERFACE:
                    //usb request failed then ep0 stall and ep0_state return to initial state
                    if (udc_get_interface(uapi, usbcmd))
                        udc->ep0_state = EP0_NONE;
                    break;
                default:
                    udc_ep0_get_wrong_setup(usbcmd);
                    mt_udc_ep0_stall();
                    udc->ep0_state = EP0_NONE;
                    break;
            }
        }
        else                    //H2D
        {
            switch (usbcmd->bRequest)
            {
                case USB_REQ_SET_ADDRESS:
                    usb_address = le16_to_cpu(usbcmd->wValue);
                    //usbinf("device address = %d\n", usb_address);

                    /* change state */
                    udc->ep0_state = EP0_STATUS;
                    udc->usb_state = USB_STATE_ADDRESS;

                    /* send ZLP in status stage */
                    udc_ep_tx(0, 0, 0, mt_udc_set_addr_done,
                              (void *) usb_address);
                    break;

                case USB_REQ_SET_CONFIGURATION:
                    //usbdbg("set configuration");
                    /* assume only one configuration */

                    /* set endpoint in correct speed setting */
                    uapi->set_enpt_desc_by_spd((udc->speed ==
                                                USB_SPEED_HIGH) ? 1 : 0);

                    /* QH setup for data path */
                    eplist = uapi->enptlist;
                    while (eplist)
                    {
                        epdesc = eplist->desc;
                        ep = epdesc->bEndpointAddress & (USB_MAX_EP_NUM - 1);
                        dir =
                            (epdesc->bEndpointAddress & USB_DIR_IN) ? MT_EP_IN :
                            MT_EP_OUT;
                        type =
                            epdesc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
                        maxpkt = le16_to_cpu(epdesc->wMaxPacketSize);

                        if (type == USB_ENDPOINT_XFER_BULK)
                            mt_udc_qh_setup(ep, dir, type, maxpkt, 0, 0);
                        else
                            mt_udc_qh_setup(ep, dir, type, maxpkt, 1, 0);

                        /* enable EP */
                        mt_udc_ep_setup(ep, dir, type);

                        eplist = eplist->next;
                    }
                    // prepare Rx descriptor and link callback function
                    uapi->start_rx(uapi->upper_dev);
                    ep0_rx_done();
                    break;
                case USB_REQ_CLEAR_FEATURE:
                case USB_REQ_SET_FEATURE:
                    if ((usbcmd->bmRequestType & USB_REQ_RECIPIENT_MASK) ==
                        USB_RECIP_ENDPOINT)
                    {
                        tmp = le16_to_cpu(usbcmd->wIndex);
                        ep = tmp & USB_ENDPOINT_NUMBER_MASK;
                        dir = tmp & USB_ENDPOINT_DIR_MASK;
                        type =
                            (usbcmd->bRequest == USB_REQ_SET_FEATURE) ? 1 : 0;
                        usbdbg("%s endpoint%d %s\n", type ? "set" : "clear", ep,
                               dir ? "IN" : "OUT");
                        mt_udc_ep_stall(ep, dir, type);
                        ep0_rx_done();
                    }
                    else
                        goto unsupport;
                    break;
                default:
                  unsupport:
                    udc_ep0_get_wrong_setup(usbcmd);
                    mt_udc_ep0_stall();
                    udc->ep0_state = EP0_NONE;
                    break;
            }
        }
    }
#ifdef __ECOS
    else if ((usbcmd->bmRequestType & USB_REQ_TYPE_MASK) == USB_TYPE_CLASS)
    {
        /* trigger upper layer to handle */
        uapi->setup((void *) usbcmd);
    }
#endif
    else
    {
        udc_ep0_get_wrong_setup(usbcmd);
        mt_udc_ep0_stall();
        udc->ep0_state = EP0_NONE;
    }
    return;
}

/*!
 * function: udc_thread
 *
 *  \brief
 *  \param
 *  \return
 */
void udc_irqhandler(void *arg)
{
    u32 irq_src;
    struct udc_api *uapi = arg;

    /* Disable all interrupt condition to avoid 
     * interrupt happend and handled in the same interrupt handle.
     * It should avoid no interrupt cause case */
    udc_writel(&dr_regs->usbintr, 0x0);

    irq_src = udc_readl(&dr_regs->usbsts) & udc_enable_intr;

    /* Clear notification bits */
    udc_writel(&dr_regs->usbsts, irq_src);

    //if (!irq_src) {
    //    //usbdbg("no status about interrupt!!!\n");
    //    udc_writel(&dr_regs->usbintr, udc_enable_intr);
    //    return;
    //}
    //usbdbg("+%s()", __func__);
    //usbdbg("irq_src: 0x%08x", irq_src);

    /* Resume */
    if (udc.usb_state == USB_STATE_SUSPENDED)
        if ((udc_readl(&dr_regs->portsc) & PORTSCX_PORT_SUSPEND) == 0)
        {
            udc.usb_state = udc.resume_state;
        }

    /* USB Interrupt */
    if (irq_src & USB_STS_INT)
    {
        //usbdbg("Packet int\n");
        if (udc.ep0_state == EP0_NONE)
            /* Setup package, we only support ep0 as control ep */
            if (udc_readl(&dr_regs->endptsetupstat) & EP_SETUP_STATUS_EP0)
            {
                udc_ep0setup(uapi);
            }

        /* completion of td */
        if (udc_readl(&dr_regs->endptcomplete))
        {
            udc_complete_handler();
        }

        if (udc.ep0_state == EP0_NONE)
            /* Setup package, we only support ep0 as control ep */
            if (udc_readl(&dr_regs->endptsetupstat) & EP_SETUP_STATUS_EP0)
            {
                udc_ep0setup(uapi);
            }
    }

    /* SOF (for ISO transfer) */
    //if (irq_src & USB_STS_SOF) {
    //    usbdbg("SOF\n");
    //}

    /* Port Change */
    if (irq_src & USB_STS_PORT_CHANGE)
    {
        //usbdbg("Port Change\n");
        mt_udc_port_change(uapi->dev);
    }

    /* Reset Received */
    if (irq_src & USB_STS_RESET)
    {
        mt_udc_reset(uapi);
        //usbdbg("Reset Received 0x%08x\n", udc_readl(&dr_regs->portsc));
#ifdef __ECOS
        if (uapi->disable)
            uapi->disable(uapi->upper_dev);
        if (uapi->enable)
            uapi->enable(uapi->upper_dev);
#endif
    }

    /* Suspend */
    if (irq_src & USB_STS_SUSPEND)
    {
        udc.resume_state = udc.usb_state;
        udc.usb_state = USB_STATE_SUSPENDED;
    }

    if (irq_src & (USB_STS_ERR | USB_STS_SYS_ERR))
    {
        usbinf("Error IRQ %x\n", irq_src);
    }

    /* re-enable related interrupt trigger condition */
    udc_writel(&dr_regs->usbintr, udc_enable_intr);
}

/*!
 * function: usb_phy_patch
 *
 *  \brief
 *  \param
 *  \return
 */
void usb_phy_patch(void)
{
    //tune USB PHY
#ifndef CONFIG_EPHY_GLITCH_PATCH
    USBREG(USB_PHY_PLL) = 0x6c1080fe;
#else
    USBREG(USB_PHY_PLL) = 0x5ca083fe;
#endif
    USBREG(PHY_DIG_CTRL) |= RECOVERY_CLK_INV;
    USBREG(USB_REG0) |= SQ_DELAY;
}

/*!
 * function: udc_init
 *
 *  \brief
 *  \param
 *  \return
 */
int udc_init(void *ptr)
{
    struct udc_api *uapi = ptr;

    uapi->dev = &udc;
#ifdef __ECOS
    uapi->irqhandler = udc_irqhandler;
    ubuf_cfg.mutex = os_mutex_create();
#endif

    /* init ubuf */
    ubuf_init(uapi, (void *) ubuf_base, UDC_TD_NUM);
    s_td_ref[2 * 0 + MT_EP_OUT].des_num = EP0_DES_NUM;
    s_td_ref[2 * 0 + MT_EP_IN].des_num = EP0_DES_NUM;
    s_td_ref[2 * 1 + MT_EP_OUT].des_num = EP1_DES_NUM;  //rx 32
    s_td_ref[2 * 1 + MT_EP_IN].des_num = EP1_DES_NUM << 1;      //tx 64
    s_td_ref[2 * 2 + MT_EP_OUT].des_num = EP2_DES_NUM;
    s_td_ref[2 * 2 + MT_EP_IN].des_num = EP2_DES_NUM;
    s_td_ref[2 * 3 + MT_EP_OUT].des_num = EP3_DES_NUM;
    s_td_ref[2 * 3 + MT_EP_IN].des_num = EP3_DES_NUM;

#if 0                           //Already reset USB module in board_init(), should needn't redo it
    /* reset USB module */
    UsbSoftRstSet();
    usbdelay();
    UsbSoftRstClr();
#endif

    //MONTAGE_USB
    usb_phy_patch();

#ifdef CONFIG_FPGA
#define USBMOD         0xf8
    /* change USB PHY to device mode */
    GPREG(USBMOD) = 0x1;
#endif

    /* config phy interface - UTMI (fixed) */
    //udc_writel(&dr_regs->portsc, 0x0C000004);

    /* qhs initialization */
    mt_udc_init_qhs();

    /* ep0 qh setup */
    mt_udc_qh_setup(0, MT_EP_IN, USB_ENDPOINT_XFER_CONTROL,
                    CTRL_MAXPKTSIZE, 1, 0);
    mt_udc_qh_setup(0, MT_EP_OUT, USB_ENDPOINT_XFER_CONTROL,
                    CTRL_MAXPKTSIZE, 1, 0);

    /* init montage USB device controller */
    mt_udc_init();

    /* init descriptors */
    mt_udc_alloc_td(uapi);

#if 0                           //ep0 always enable
    /* ep0 enable */
    mt_udc_ep_setup(0, USB_DIR_IN, USB_ENDPOINT_XFER_CONTROL);
    mt_udc_ep_setup(0, USB_DIR_OUT, USB_ENDPOINT_XFER_CONTROL);
#endif

#ifndef __ECOS
    request_irq(IRQ_UDC, &udc_irqhandler, (void *) uapi);
#endif

    /* setup USB interrupt */
    mt_udc_setup_intr();

    /* Enable controller */
    mt_udc_start();

    return 0;
}

/*!
 * function: udc_dump_des
 *
 *  \brief dump descroptor content
 *  \param
 *  \return
 */
int udc_dump_des(int ep)
{
    struct udc_td *td;
    struct udc_td *start;

    //Rx
    td = start = s_td_ref[2 * ep + MT_EP_OUT].td_head;
    if (start)
    {
        do
        {
            dump_hex("Rx", td, MT_TD_SIZE);
            td = TD_NEXT_PTR(td);
        }
        while (td != start);
    }

    //Tx
    td = start = s_td_ref[2 * ep + MT_EP_IN].td_head;
    if (start)
    {
        do
        {
            dump_hex("Tx", td, MT_TD_SIZE);
            td = TD_NEXT_PTR(td);
        }
        while (td != start);
    }

    return 0;
}

/*!
 * function: udcd_info
 *
 *  \brief
 *  \param
 *  \return
 */
int udcd_info(void)
{
    printf("ep0_state=%d\n", udc.ep0_state);
    printf("usb_state=%d\n", udc.usb_state);
    printf("speed=%d\n", udc.speed);
    return 0;
}

/*!
 * function: udc_des_info
 *
 *  \brief
 *  \param
 *  \return
 */
int udc_des_info(void)
{
    int ep;
    for (ep = 0; ep < USB_MAX_EP_NUM; ep++)
    {
        printf("ep%d rx used desnum=%d\n", ep,
               s_td_ref[2 * ep + MT_EP_OUT].des_used);
        printf("ep%d tx used desnum=%d\n", ep,
               s_td_ref[2 * ep + MT_EP_IN].des_used);
    }
    return 0;
}

/*!
 * function: udcd_cmd
 *
 *  \brief USB Device Controller Driver Debug Command
 *  \param
 *  \return
 */
int udcd_cmd(int argc, char *argv[])
{
#ifdef __ECOS
    char *str = argv[1];
    if (2 > argc)
        return 1;
#else
    char *str = argv[0];
    if (1 > argc)
        return 1;
#endif
    if (!strcmp(str, "ubuf"))
        ubuf_state();
    else if (!strcmp(str, "info"))
        udcd_info();
    else if (!strncmp(str, "ep", 2))
    {
        sscanf(&str[2], "%d", &argc);
        printf("ep%d:\n", argc);
        udc_dump_des(argc);
    }
    else if (!strcmp(str, "des"))
        udc_des_info();
    return 0;
}

/* register command */
#ifdef __ECOS
CLI_CMD(udcd, udcd_cmd, "udcd\n"
        "\tdes: enpt des num\n"
        "\tinfo: state\n"
        "\tubuf: ubuf info\n" "\tepx: dump epx descriptor", 0);
#else
cmdt cmdt_udc __attribute__ ((section("cmdt"))) =
{
"udcd", udcd_cmd, "udcd\n"
        "\tdes: enpt des num\n"
        "\tinfo: state\n" "\tubuf: ubuf info\n" "\tepx: dump epx descriptor"};
#endif
#endif                          /* CONFIG_UDC */
