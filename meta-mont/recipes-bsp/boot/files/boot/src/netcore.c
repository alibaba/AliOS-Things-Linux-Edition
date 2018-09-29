/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file netcore.c
*   \brief Network Core Driver
*   \author Montage
*/

#include <arch/chip.h>
#include <arch/irq.h>
#include <common.h>
#include <netprot.h>
#include <netdev.h>
#include <cm_mac.h>
#include <lib.h>

int netdev_num;
struct net_device devlist[4];
struct net_device *dev;
struct nbuf_cfg nbuf_cfg;
queue netif_queue;
void (*net_func) (struct nbuf * nbuf);
void net_poll(struct nbuf *nbuf);
void brif_rx(struct nbuf *nbuf);
int nbuf_enqueue(queue * qp, struct nbuf *nb);
void nbuf_dequeue(queue * qp, struct nbuf **rpp);

// these calibration value are set default value with 3
// becuase Tx: 11'b, Rx: 11'b
unsigned int tclk = 3;
unsigned int rclk = 3;

extern char boot3_submode;
#define NBFLAG(nb) (nb->flag)

/*!
 * function:
 *
 *  \brief
 *  \param base
 *  \param size
 *  \param num
 *  \return
 */

int nbuf_init(void *base, int size, int num)
{
    struct nbuf *nb;
    short n;

    nbuf_cfg.queue.max = num;

    nb = nbuf_cfg.base = (struct nbuf *) ALIGN_TO(base, 16);

/*
    nb = nbuf_cfg.base = (struct nbuf *) ALIGN_TO(0xa1ff0000,16);
    #define SDRAM_BASE_ADDR  (MAC_BASE + 0x60)
            *( volatile unsigned int *) SDRAM_BASE_ADDR = 0x1000000;  //between 16~32MB
    printf("nb:%x\n", nb);
*/

    for (n = 0; n < num; n++)
    {
//      printf("nb:%x\n", nb);
        NBFLAG(nb) = NB_RELEASE;
        nbuf_put(nb);
        nb = (struct nbuf *) ((unsigned int) nb + size);
    }

    nbuf_cfg.num = num;
    nbuf_cfg.size = size;
    return NB_OK;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

struct nbuf *nbuf_get(void)
{
    struct nbuf *nb;
    int flag;

    flag = irq_save();

    nbuf_dequeue(&nbuf_cfg.queue, &nb);

    if (nb == 0)
    {
#if NBDEBUG > 0
        printf("%s:empty\n", __func__);
#endif
        goto out;
    }

    if (NB_FREE != NBFLAG(nb))
    {
#if NBDEBUG > 0
        printf("%s:err flag:%x\n", __func__, NBFLAG(nb));
#endif
        goto out;
    }

    nb->tail = nb->data = nb->head = (unsigned char *) nb + NB_HEAD_SZ;
    nb->end = (unsigned char *) nb + (nbuf_cfg.size - 1);
    nb->len = 0;
    NBFLAG(nb) = NB_ALLOC;
    nb->next = 0;
  out:
    irq_restore(flag);
    return nb;
}

/*!
 * function:
 *
 *  \brief
 *  \param nb
 *  \return
 */

int nbuf_put(struct nbuf *nb)
{
    int flag;

    if(!nb)
        return 0;

    flag = irq_save();

    if (!nb
        ||
        ((NB_ALLOC != NBFLAG(nb)) &&
         (NB_RELEASE != NBFLAG(nb)) && (NB_FREE != NBFLAG(nb))))
    {
#if NBDEBUG > 0
        printf("%s:nb=%x id=%x error\n", __func__, nb, (nb ? NBFLAG(nb) : 0));
#endif
        irq_restore(flag);
        return -NB_ERR_ID;
    }
    if (NB_FREE == NBFLAG(nb))
    {
        printf("%s %x free twice\n", __func__, nb);
        irq_restore(flag);
        return NB_OK;
    }
    else if (NB_STALL == NBFLAG(nb))
    {
        irq_restore(flag);
        return NB_OK;
    }


    NBFLAG(nb) = NB_FREE;

    nbuf_enqueue(&nbuf_cfg.queue, nb);

    irq_restore(flag);
    return NB_OK;
}

/*!
 * function:
 *
 *  \brief
 *  \param qp
 *  \param nb
 *  \return
 */

int nbuf_enqueue(queue * qp, struct nbuf *nb)
{
    if (!qp->count)
        qp->head = qp->tail = nb;
    else
    {
#if 1  /* disable queue depth check */
        qp->tail->next = (struct nbuf *) nb;
        qp->tail = nb;
#else
        if (qp->max > qp->count)
        {
            qp->tail->next = (struct nbuf *) nb;
            qp->tail = nb;
        }
        else
            return -1;
#endif
    }

    nb->next = 0;
    qp->count++;

    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param qp
 *  \param rpp
 *  \return
 */

void nbuf_dequeue(queue * qp, struct nbuf **rpp)
{
    if (0 >= qp->count)
        *rpp = 0;
    else
    {
        *rpp = qp->head;
        qp->head = (struct nbuf *) qp->head->next;
        (*rpp)->next = 0;
        qp->count--;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

int nbuf_state()
{
    struct nbuf *nb;
    int i, counter[3];
    int flag;

    memset((char *)&counter[0], 0, sizeof (counter));

    for (i = 0, nb = nbuf_cfg.base; i < nbuf_cfg.num;
         i++, nb = (struct nbuf *) ((char *) nb + nbuf_cfg.size))
    {
        flag = NBFLAG(nb);
        if (NB_FREE == flag)
            counter[0]++;
        else if (NB_ALLOC == flag)
            counter[1]++;
        else
        {
            counter[2]++;
#if NBDEBUG > 0
            printf("nbuf:%x unknown flag=%04x", (unsigned int) nb, flag);
#endif
        }
    }
    printf("Total =%d\n", nbuf_cfg.num);
    printf("Free: %d , Used: %d , Others: %d \n", counter[0], counter[1],
           counter[2]);
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

struct net_device *netdev_alloc()
{
    struct net_device *dev;
    dev = &devlist[netdev_num++];
    sprintf(dev->name, "eth%d", netdev_num);
    return dev;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void netdev_init(void)
{
    netdev_num = 0;
    memset((void *) &netif_queue, 0, sizeof (queue));
    netif_queue.max = (CONFIG_NETBUF_NUM / 5) ? (CONFIG_NETBUF_NUM / 4) : 1;
    net_func = net_poll;
}

int eth_unmatch_da(unsigned char *packet, short brcst);
/*!
 * function:
 *
 *  \brief
 *  \param nbuf
 *  \return
 */

void net_poll(struct nbuf *nbuf)
{
    int flag;
    if (eth_unmatch_da(nbuf->data, 1))
        goto free;

    flag = irq_save();
    if (nbuf_enqueue(&netif_queue, nbuf))
    {
        irq_restore(flag);
        nbuf_put(nbuf);
        return;
    }

    irq_restore(flag);
    return;
  free:
    nbuf_put(nbuf);
}

/*!
 * function:
 *
 *  \brief
 *  \param nbuf
 *  \return
 */

int netif_rx(struct nbuf *nbuf)
{
    struct net_device *dev;

    if(NULL==nbuf)
        return 0;

    dev = nbuf->dev;
    if (dev)
    {
        if (net_func)
            net_func(nbuf);
        else
            nbuf_put(nbuf);

        return 0;
    }
    else
    {
        nbuf_put(nbuf);

        return 0;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void eth_reset(void)
{
    cm_wan_enable();
    cheetah_phy_up(1, 0);
    cm_reset();
}

#if !defined(CONFIG_PANTHER)
/*!
 * function:
 *
 *  \brief
 *  \return
 */
void eth_init(void)
{
    int i;
    unsigned int val = 0;

    /* P2 MII clk doesn't use 1/2 system clk */
    MACREG_UPDATE32(RMII_MII, 0, P2_PHYMODE_CLK);
    /* Clear P0/P1 clock mode */
    MACREG_UPDATE32(RMII_MII, 0,
                    P1_MII_MODE | P1_RMII_OUT_REFCLK | P1_PHYMODE_CLK |
                    P1_ADAPTER_MODE | P0_MII_MODE | P0_RMII_OUT_REFCLK |
                    P0_PHYMODE_CLK | P0_ADAPTER_MODE);
#if defined(CONFIG_P0_AS_LAN)
#if defined(CONFIG_P0_RMII)
    GPREG(PINMUX) |= EN_ETH0_FNC;       //Turn on P0
#endif
#ifndef CONFIG_P0_EXT_PHY
    GPREG(PINMUX) &= ~EN_ETH0_FNC;      //Turn off P0
#endif
#if defined(CONFIG_P0_RMII_CLKOUT)
    val = P0_RMII_OUT_REFCLK | P0_PHYMODE_RMII | P0_ADAPTER_RMII;
#elif defined(CONFIG_P0_RMII_CLKIN)
    val = P0_PHYMODE_RMII | P0_ADAPTER_RMII;
#elif defined(CONFIG_P0_MII_PHY)
    val =
        P0_MII_PHY | P0_RMII_OUT_REFCLK | P0_PHYMODE_MII_100M | P0_ADAPTER_MII;
#elif defined(CONFIG_P0_MII_MAC)
    val = P0_MII_MAC | P0_ADAPTER_MII;
#endif
    MACREG_UPDATE32(RMII_MII, val, val);
#else
#if defined(CONFIG_P1_RMII)
    GPREG(PINMUX) |= EN_ETH1_FNC;       //Turn on P1
#endif
#ifndef CONFIG_P1_EXT_PHY
    GPREG(PINMUX) &= ~EN_ETH1_FNC;      //Turn on P1 as internal phy
#endif
#if defined(CONFIG_P1_RMII_CLKOUT)
    val = P1_RMII_OUT_REFCLK | P1_PHYMODE_RMII | P1_ADAPTER_RMII;
#elif defined(CONFIG_P1_RMII_CLKIN)
    val = P1_PHYMODE_RMII | P1_ADAPTER_RMII;
#elif defined(CONFIG_P1_MII_PHY)
    val =
        P1_MII_PHY | P1_RMII_OUT_REFCLK | P1_PHYMODE_MII_100M | P1_ADAPTER_MII;
#elif defined(CONFIG_P1_MII_MAC)
    val = P1_MII_MAC | P1_ADAPTER_MII;
#endif
    MACREG_UPDATE32(RMII_MII, val, val);
#endif
    GPREG(SWRST) &= ~(PAUSE_SW | PAUSE_HNAT | PAUSE_WIFI);      //Enalbe gating clock of switch, hnat and wifi module
    for (i = 0; i < 200; i++) ;
    GPREG(SWRST) &= ~(SWRST_SW | SWRST_HNAT);   //Soft reset of switch and hnat
    for (i = 0; i < 200; i++) ;
    GPREG(SWRST) |= (SWRST_SW | SWRST_HNAT);    //Release soft reset
}
#endif

int eth_open(int intf);
/*!
 * function:
 *
 *  \brief
 *  \return
 */
int eth_probe()
{
    static int registered = 0;

#if !defined(CONFIG_PANTHER)
    eth_init();
#endif
    if (registered)
    {
        return (!eth_open(0));
    }

    netdev_init();

    if ((dev != 0) && (dev->start))
        return 1;

    cm_probe();
    if (bootvars.network)
    {
        eth_open(0);
    }
    return 1;
}

#ifdef CONFIG_ETH_CALIBRATION
/*!
 * function:
 *
 *  \brief
 *  \param idx
 *  \param txclk
 *  \param rxclk
 *  \return
 */
int eth_cal_txrx(unsigned char idx, int txclk, int rxclk)
{
    int rx_cnt, i, tx_cnt = 3;
    struct arprequest arpreq;
    unsigned char bcast[] = {[0 ... 5] = 0xff };

    arpreq.protolen = 4;
    cm_cal_set(idx, txclk, rxclk);
    rx_cnt = cm_cal_get();
    for (i = 0; i < tx_cnt; i++)
    {
        eth_tx((const char*)bcast, ARP, sizeof (arpreq), &arpreq);
    }
    mdelay(50);
    rx_cnt = cm_cal_get() - rx_cnt;
    return (rx_cnt == tx_cnt);
}

/*!
 * function:
 *
 *  \brief
 *  \param idx
 *  \return
 */
void eth_cal_start(unsigned char idx)
{
    cm_phy_loopback(PHY_ETH1_IDX, 1);
    mdelay(3000);

    /* FIXED ME: Sometimes calibration will fail if inverse RX clock,
       do TX calibration before RX calibration.
     */
#ifndef CONFIG_FPGA
    if (eth_cal_txrx(idx, 0, 0))
    {
        tclk = 0;
        rclk = 0;
        printf("ETH calibration p(%d) TX(%d) RX(%d)\n", idx, 0, 0);
    }
    else
#endif
    if (eth_cal_txrx(idx, 0, 1))
    {
        tclk = 0;
        rclk = 3;
        printf("ETH calibration p(%d) TX(%d) RX(%d)\n", idx, 0, 1);
    }
    else if (eth_cal_txrx(idx, 0, 0))
    {
        tclk = 0;
        rclk = 0;
        printf("ETH calibration p(%d) TX(%d) RX(%d)\n", idx, 0, 0);
    }
    else if (eth_cal_txrx(idx, 1, 0))
    {
        tclk = 3;
        rclk = 0;
        printf("ETH calibration p(%d) TX(%d) RX(%d)\n", idx, 1, 0);
    }
    else if (eth_cal_txrx(idx, 1, 1))
    {
        tclk = 3;
        rclk = 3;
        printf("ETH calibration p(%d) TX(%d) RX(%d)\n", idx, 1, 1);
    }
    else
    {
        printf("ETH calibration fail!!!\n");
    }

    cm_phy_loopback(PHY_ETH1_IDX, 0);
    mdelay(3000);
    cm_switch_port_attach(idx, 0, 0);
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void eth_calibration(void)
{
#if defined(CONFIG_P0_AS_LAN)
    eth_cal_start(0);
#else
    eth_cal_start(1);
#endif
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \param intf
 *  \return
 */

int eth_open(int intf)
{
    int result;

    dev = &devlist[intf];

    if (!dev)
        return -1;

    if (dev->start)
    {
        return 0;
    }

    memcpy((void *)(&dev->dev_addr[0]), (void *)(&(bootvars.mac0[0])), 6);
    result = dev->open(dev);

    if (result)
    {
        printf("rc=%d\n", result);
        return result;
    }
#ifdef CONFIG_ETH_CALIBRATION
    if (bootvars.autocal)
        eth_calibration();
#endif

    return (0);
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void *eth_rx()
{
    struct nbuf *pkt;
    unsigned int flags;
#ifdef CONFIG_ETH_POLLING
    dev->poll((void *) dev);
#endif

    flags = irq_save();
    nbuf_dequeue(&netif_queue, &pkt);
    irq_restore(flags);
    return pkt;
}

extern void wifi_tx(const char *dest, unsigned int type, unsigned int size, const void *pkt);
void eth_tx(const char *d, unsigned int t, unsigned int s, const void *p)
{
    struct nbuf *pkt;
    int result;
    register unsigned char *data;

    // whether to transmit wifi data frame through ether path
#if defined(CONFIG_WIFI)
    if(boot3_submode == RECOVER_SUBMODE)
        wifi_tx(d, t, s, p);
    else
#endif
    {
        pkt = nbuf_get();
        if (pkt == 0)
        {
            printf("tx no buf\n");
            goto err1;
        }

        data = pkt->data = pkt->head + 0x40;

        if (bootvars.vlan)
            memcpy((void *)(data + 14 + 4), (void *)p, s);
        else
            memcpy((void *)(data + 14), (void *)p, s);

        memcpy((void *)data, (void *)d, 6);
        memcpy((void *)(data + 6), (void *)bootvars.mac0, 6);
        if (bootvars.vlan)
        {
            data[12] = (VLAN >> 8) & 0xff;
            data[13] = (VLAN & 0xff);
            data[14] = 0;
            data[15] = bootvars.vlan;
            data[16] = (t >> 8) & 0xff;
            data[17] = t & 0xff;
            pkt->len = s + 14 + 4;
        }
        else
        {
            data[12] = (t >> 8) & 0xff;
            data[13] = t & 0xff;
            pkt->len = s + 14;
        }

#ifdef CONFIG_ETH_PAD64
        if (pkt->len < 60)
        {
            memset((char *)&data[pkt->len], 0, (int)(60 - pkt->len));
            pkt->len = 60;
        }
#endif

#if 0 // enable this to test TCP TX packet lost recovery
    {
        static int drop = 0;
        if(pkt->len > 60) //if(drop++>=2)
        {
            drop = 0;
            nbuf_put(pkt);
            return;
        }
    }
#endif

        result = dev->hard_start_xmit(pkt, dev);

        if (result < 0)
            nbuf_put(pkt);
    }
err1:
    return;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void eth_disable(void)
{
    if (dev)
        dev->stop(dev);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param argv
 *  \return
 */

int eth_ioctl(int cmd, void *argv)
{
    return ((dev->do_ioctl) ? dev->do_ioctl(dev, argv, cmd) : 0);
}

#ifdef CONFIG_CMD_BR            //bridge
/*!
 * function: eth_bridge
 *
 *  \brief  just forward packet
 *  \param nbuf
 *  \return
 */
static int br_debug;
void eth_bridge(struct nbuf *nbuf)
{
    int result;

#ifdef CONFIG_ETH_PAD64
    if (nbuf->len < 60)
    {
        memset(&nbuf->data[nbuf->len], 0, 60 - nbuf->len);
        nbuf->len = 60;
    }
#endif

    if (br_debug)
        dump_hex("br:", nbuf->data, 16);
    // for ic+ debug 175D QOS , swap vlan id from 3 to 4, 4 to 3
    if ((VLAN >> 8) == nbuf->data[12])
    {
        if (3 == nbuf->data[15])
            nbuf->data[15] = 4;
        else if (4 == nbuf->data[15])
            nbuf->data[15] = 3;
        // if 2nd tag, replace 1st tag's prio with 2nd's
        if ((VLAN >> 8) == nbuf->data[16])
        {
            nbuf->data[14] &= 0x1f;     //mask b5~7
            nbuf->data[14] |= (nbuf->data[18] & 0xe0);  //take b5~7 (prio of 2nd vlan tag)
        }
    }
    result = dev->hard_start_xmit(nbuf, nbuf->dev);
    if (result < 0)
        nbuf_put(nbuf);
    return;
}

void icplus_175d_vlan()
{
    //20.5=0016 (X_EN enable, MAC_X_EN disable)
    cm_mdio_wr(20, 5, 0x0016);
    //20.13=0021 (Enable learning constrain)
    cm_mdio_wr(20, 13, 0x0021);

    //22.0=07ff (tag base VLAN, port0~4 PVID mode, port5 VID mode)
    cm_mdio_wr(22, 0, 0x07ff);

    //22.2=0020 (port5 keep tag)
    cm_mdio_wr(22, 2, 0x0020);

    //22.4=0004 (port0 PVID)
    cm_mdio_wr(22, 4, 0x0004);

    //22.5=0003 (port1 PVID)
    cm_mdio_wr(22, 5, 0x0003);

    //22.6=0003 (port2 PVID)
    cm_mdio_wr(22, 6, 0x0003);

    //22.7=0003 (port3 PVID)
    cm_mdio_wr(22, 7, 0x0003);

    //22.8=0003 (port4 PVID)
    cm_mdio_wr(22, 8, 0x0003);

    //22.10=0003 (Enable VLAN entry 0&1)
    cm_mdio_wr(22, 10, 0x0003);

    //22.14=1003 (VLAN entry 0)
    cm_mdio_wr(22, 14, 0x1003);

    //22.15=2004 (VLAN entry 1)
    cm_mdio_wr(22, 15, 0x2004);

    //23.0=213e (VLAN entry 0 member, VLAN entry 1 member)
    cm_mdio_wr(23, 0, 0x213e);

    //23.8=2020 (VLAN entry 0 add tag port, VLAN entry 1 add tag port)
    cm_mdio_wr(23, 8, 0x2020);

    //23.16=011e (VLAN entry 0 remove tag port, VLAN entry 1 remove tag port)
    cm_mdio_wr(23, 16, 0x011e);

    //25.0=2000 (port5 COS enable)
    cm_mdio_wr(25, 0, 0x2000);

    //25.2=e400 (VLAN priority 7--> queue3, VLAN priority 6 --> queue2, VLAN priority 5 ---> queue1, VLAN priority 1 ---> queue0)
    cm_mdio_wr(25, 2, 0xe400);

    //25.22=0003 (port0 strictlt priority)
    cm_mdio_wr(25, 22, 0x0003);
}

#include <common.h>
/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int eth_br_cmd(int argc, char *argv[])
{
    if (argc < 1)
        goto show;
    if ('d' == (argv[0][0]))
        br_debug = br_debug ^ 1;
    else if ('o' == (argv[0][0]))
        switch (argv[0][1])
        {
            case 'n':          // switch on
                // set 175d vlan
                icplus_175d_vlan();
                net_func = &eth_bridge;
                break;
            case 'f':          // off
                // To add reverse code
                net_func = &net_poll;
                break;
            default:
                goto show;
        }
  show:
    printf("bridge mode: %s\n", (net_func == &eth_bridge) ? "on" : "off");
    printf("      debug: %s\n", (br_debug) ? "on" : "off");
    return ERR_OK;
}

cmdt cmdt_br[] __attribute__ ((section("cmdt"))) =
{
    {
    "br", eth_br_cmd, "br  <on/off/debug>; bridge at cpu port "}
,};
#endif
