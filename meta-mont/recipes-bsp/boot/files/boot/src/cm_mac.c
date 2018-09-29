/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cm_mac.c
*   \brief Ethernet MAC Driver
*   \author Montage
*/

/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <lib.h>
#include <arch/chip.h>
#include <arch/irq.h>
#include <netdev.h>
#include <cm_mac.h>
#include <netprot.h>

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

#define TXD_NUM     8
#define RXD_NUM     8
#define HWD_NUM     2
#define DES_NUM     (TXD_NUM+RXD_NUM+HWD_NUM)
#define BUF_NUM     (RXD_NUM+1)
#define DEFAULT_MAC { 0x00,0x11,0x22,0x33,0x44,0x55 }

#define ENODEV      -2
#define EBUSY       -3
#define DEBUGINFO(format, args...)      //printf(format, ##args)

#define MCR_INIT    ((MCR_SMI_PERIOD<<MCR_SMI_S) | MCR_BWC1|MCR_BWC0| \
            /*MCR_HEN | MCR_HPFD | MCR_NES | MCR_HHFM| */ \
            MCR_BL| /* bypass lookup */ \
            MCR_TRT|MCR_TFC|MCR_HN|MCR_LU|MCR_FC)

#define MIM_INIT    0x33
#define MRXCR_INIT  0x8201004   //defulat=0x8321004
#define MAUTO_INIT  0x81ff0030

#define NBUF_NET    0           // rx, to net
#define NBUF_HW     1           // initiail to hw
#define NBUF_RXSET  2           // set by rx routine
#define NBUF_TXDONE 4           // free at txdone
#define NBUF_HWFREE 5           // force free hw
#define NBUF_TXFREE 6           // force free at tx
#define NBUF_RXFREE 7           // force free at rx
#define NBUF_SWFREE 8           // free at sw initial

#define MREG(ofs)   (((volatile unsigned int*)g_cm_base)[ofs>>2])

static unsigned short cycle[3][2] = { {48, 480}, {52, 512}, {20, 200} };

#if defined(CONFIG_ETH_CALIBRATION) || defined(CONFIG_FPGA)
static int tx_clk = 0, rx_clk = 0;
#endif
static unsigned short an_status[2] = { 0, 0 };
static unsigned int phyid[2] = { 0, 0 };

#define clkmode 0x7
/* phy id */
#define PHYID_ICPLUS_IP101G		0x02430c54
#define PHYID_MONT_EPHY			0x00177c01
typedef struct
{
    cmdev *cm;
    short vid;                  //vlan id
} priv;

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void cm_preset_175d();
void cm_set_cpu_port();
void cm_phy_init(unsigned char p);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#define SWITCH_BUF_INSRAM
unsigned char netbuf[CONFIG_NETBUF_SZ * CONFIG_NETBUF_NUM]
    __attribute__ ((aligned(0x800)));
#if !defined(SWITCH_BUF_INSRAM)
unsigned char fastpath_buf[DEF_FASTPATH_BUF_SIZE *
                           FASTPATH_BUFFER_HEADER_POOL_SIZE]
    __attribute__ ((aligned(0x800)));
unsigned char hw_buf_headers[sizeof (buf_header) *
                             FASTPATH_BUFFER_HEADER_POOL_SIZE]
    __attribute__ ((aligned(0x800)));
#endif
DES g_desc[DES_NUM]  __attribute__ ((aligned(0x20)));
BDS g_txbds[TXD_NUM];

cmdev g_cmdev;
unsigned long g_cm_base = MAC_BASE;
static unsigned char g_mac_addr[6] = DEFAULT_MAC;
priv g_priv[2];
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

/*!
 * function:
 *
 *  \brief
 *  \param id
 *  \return
 */

inline static struct nbuf *NBUF_GET(short id)
{
    struct nbuf *nb = nbuf_get();
    if (nb)
        nb->id2 = id;
    return nb;
}

/*!
 * function:
 *
 *  \brief
 *  \param nb
 *  \param id
 *  \return
 */

inline static void NBUF_PUT(struct nbuf *nb, short id)
{
    if (nb)
    {
        nb->id2 = id;
        nbuf_put(nb);
    }
}

static void cm_init_desc(DES * desc, unsigned long last)
{
    desc->w0 = last ? DS0_EOR : 0;
    desc->w1 = 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param desc
 *  \param last
 *  \return void
 */

static void cm_init_rx_desc(DES * desc, unsigned long last)
{
    desc->w0 = last ? DS0_EOR | DS0_R : DS0_R;
    desc->w1 = 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param desc
 *  \return
 */

static inline unsigned long cm_desc_is_last(DES * desc)
{
    return (DS0_EOR) & *(unsigned long *) desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param desc
 *  \return
 */

static inline unsigned long cm_desc_is_empty(DES * desc)
{
    return (*(unsigned long *) desc & ~(DS0_EOR)) == 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param desc
 *  \return
 */

static inline unsigned long cm_desc_owned_by_dma(DES * desc)
{
    return ((DS0_R | DS0_T)) & *(unsigned long *) desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param desc
 *  \return
 */

static inline unsigned long cm_desc_owned_by_txdma(DES * desc)
{
    return (DS0_T) & *(unsigned long *) desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param desc
 *  \return
 */
static void cm_desc_take(DES * desc)
{
    if (cm_desc_owned_by_dma(desc))     /* own by h/w? */
    {
        desc->w0 &= ~(DS0_T | DS0_R);
        desc->w0 |= DS0_D;
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */

void cm_free_buf(cmdev * cmd)
{
// free all allocated buffer
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cm_smi_init(void)
{
#if defined(CONFIG_PANTHER)
    SMIREG(SMI_CLK_RST) = (MCR_SMI_PERIOD << MCR_SMI_S);
#else
    GPREG(SMI_CTRL1) = (MCR_SMI_PERIOD << MCR_SMI_S);
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param base
 *  \return
 */
int cm_init(cmdev * cmd, unsigned long base)
{
    memset((void *) cmd, 0, sizeof (*cmd));
    cmd->txdnum = TXD_NUM;
    cmd->hwdnum = HWD_NUM;
    cmd->rxdnum = RXD_NUM;
    cmd->bufnum = BUF_NUM;
    cmd->txbd = g_txbds;
    return 0;
}

#if !defined(CONFIG_PANTHER)
/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cm_switch_reset(void)
{
    int i;
    /* Port Reset */
    for (i = 0; i < PORT_HNAT_IDX; i++)
    {
        unsigned int val, adapter;
        val = ((i) ? HW_RESET_SW_P1 : HW_RESET_SW_P0);
        adapter = ((i) ? HW_RESET_RMII_P1 : HW_RESET_RMII_P0);

        /* After RMII2MII adapter reset, also need to reset MII */
        if (adapter)
        {
            MACREG_UPDATE32(HW_RESET, 0, adapter);
            MACREG_UPDATE32(HW_RESET, adapter, adapter);
        }
        MACREG_UPDATE32(HW_RESET, 0, val);
        MACREG_UPDATE32(HW_RESET, val, val);
    }
    /* Core Reset */
    MACREG_UPDATE32(HW_RESET, 0, HW_RESET_SWITCH);
    MACREG_UPDATE32(HW_RESET, HW_RESET_SWITCH, HW_RESET_SWITCH);
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cm_reset()
{
    int i;
    MREG(MSRST) = 0x7f;         //reset
    for (i = 0; i < 1000; i++) ;
    MREG(MSRST) = 0;
    MREG(MIM) = ~0;
    MREG(MCR) = 0;
    MREG(MRXCR) = 0;

#if !defined(CONFIG_PANTHER)
    cm_switch_reset();
#endif
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param m
 *  \return
 */
int cm_mac_setup(cmdev * cmd, unsigned char m[6])
{
    unsigned int mac1, mac2;

    mac1 = (m[0] << 8) | (m[1]);
    mac2 = (m[2] << 24) | (m[3] << 16) | (m[4] << 8) | m[5];
    MREG(MSA00) = mac1;
    MREG(MSA01) = mac2;

#if !defined(CONFIG_PANTHER)
    /* Reset WIFI */
    MACREG_UPDATE32(HW_RESET, 0, HW_RESET_WIFI_PAUSE | HW_RESET_BB_PAUSE);
    MACREG_UPDATE32(HW_RESET, 0,
                    HW_RESET_WMAC | HW_RESET_BB | HW_RESET_DCM_BB2MAC);
    MACREG_UPDATE32(HW_RESET, -1,
                    HW_RESET_WMAC | HW_RESET_BB | HW_RESET_DCM_BB2MAC);
#endif
    /* Configure MAC to BSSID table here */
    MACREG_WRITE32(MAC_BSSID1, mac2);
    /* Enable bitmap */
    MACREG_WRITE32(MAC_BSSID0, 0x00010000UL | mac1);
    /* HNAT bitmap */
    MACREG_WRITE32(MAC_BSSID_FUNC_MAP, 0x01000000UL);
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param phy
 *  \param reg
 *  \param pval
 *  \param wr
 *  \return
 */
void cm_mdio(short phy, short reg, unsigned short *pval, int wr)
{
    unsigned long v;
    int internal = 0;

#ifdef CONFIG_P0_EXT_PHY
    if (phy == 0)
        internal = 0;
    else
#endif
#ifdef CONFIG_P1_EXT_PHY
    if (phy == 1)
        internal = 0;
    else
#endif
        internal = 1;
#ifdef CONFIG_P0_AS_ADDR2
    /* Ephy will listen address 0 as broadcast address,
       revise address avoid violation */
    if (phy == 0)
        phy = 2;
#endif
    // arthur add bit29, but it's harmless to cheetah
    if (internal)
        phy ^= MPHY_EXT;
    v = MPHY_TR | (phy << MPHY_PA_S) | (reg << MPHY_RA_S);
    if (wr)
        v |= MPHY_WR | *pval;
#if defined(CONFIG_PANTHER)
    SMIREG(SMI_DATA) = v;
#else
    GPREG(SMI_CTRL0) = v;
#endif
    for (v = 0; v < 100; v++) ;
    for (;;)
    {
#if defined(CONFIG_PANTHER)
        if (MPHY_TR & (v = SMIREG(SMI_DATA)))
#else
        if (MPHY_TR & (v = GPREG(SMI_CTRL0)))
#endif
        {
            break;
        }
    }
#if defined(CONFIG_PANTHER)
    v = SMIREG(SMI_CLK_RST);
#endif
    if (!wr)
        *pval = (v & 0xffff);
}

/*!
 * function:
 *
 *  \brief
 *  \param pa
 *  \param ra
 *  \return
 */
unsigned short cm_mdio_rd(unsigned short pa, unsigned short ra)
{
    unsigned short rd;
    cm_mdio(pa, ra, &rd, 0);
    return rd;
}

#if 0
int cmd_mdio(int argc, char **argv)
{
    int phy, reg;
    for (phy = 0; phy < 32; phy++)
    {
        printf("phy%d:\n", phy);
        for (reg = 0; reg < 32; reg++)
        {
            printf(" %04x", cm_mdio_rd(phy, reg));
            if ((reg % 8) == 7)
            {
                printf("\n");
            }
        }
    }
    return ERR_OK;
}

cmdt cmdt_mdio __attribute__ ((section("cmdt"))) =
{
"mdio", cmd_mdio, "mdio"};
#endif

/*!
 * function:
 *
 *  \brief
 *  \param pa
 *  \param ra
 *  \param val
 *  \return
 */
void cm_mdio_wr(unsigned short pa, unsigned short ra, unsigned short val)
{
    unsigned short rd;
    cm_mdio(pa, ra, &val, 1);
    rd = cm_mdio_rd(pa, ra);
    if (rd != val)
    {
        DEBUGINFO("==> not match, rd:%x, wr:%x\n\n", (unsigned int) rd,
                  (unsigned int) val);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param desc
 *  \return
 */
int cm_hw_setup(cmdev * cmd, DES * desc)
{
    cmd->rxd = desc;
    cmd->hwd = cmd->rxd + cmd->rxdnum;
    cmd->txd = cmd->hwd + cmd->hwdnum;

    cmd->txnext = 0;
    cmd->txbusy = 0;
    cmd->rxnext = 0;
    cmd->rxbusy = 0;

    cm_smi_init();
    MREG(MCR) = (MCR_INIT & 0xff000000);
#ifndef CONFIG_FPGA
    cm_preset_175d();
#endif
    cm_reset(cmd);
#if 1
    DEBUGINFO("rwd=%x\n", cmd->rxd);
    DEBUGINFO("hwd=%x\n", cmd->hwd);
    DEBUGINFO("txd=%x\n", cmd->txd);
#endif

    MREG(MHWDB) = virt_to_phy(cmd->hwd);
    MREG(MSWDB) = virt_to_phy(cmd->rxd);
    MREG(MTXDB) = virt_to_phy(cmd->txd);
    MREG(MCR) = MCR_INIT;
    MREG(MRXCR) = MRXCR_INIT;
    MREG(MAUTO) = MAUTO_INIT;
//printf("set MAUTO=%x\n", MREG(MAUTO));
    //disable TCP sync rate control
    MREG(MTSR) = 0xa0003ff;
#ifdef CONFIG_FPGA
    MREG(MPKTSZ) = 0x00fa05fe;  //look up serch time, unit:clk.
    //MREG(MDBUG) = 0x20000050;     //debug checksum error
    //MREG(MDBUG) = 0x30000600;     //debug packet drop
    MREG(MDBUG) = 0x60000040;   //debug rxdma 1009
#endif

    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param buf
 *  \return
 */
short cm_rxd_set(cmdev * cmd, unsigned long buf)
{
    short desc = cmd->rxnext;
    DES *rxd = cmd->rxd + desc;
    if (!cm_desc_is_empty(rxd))
        return -1;

    rxd->w1 = buf;
    if (cm_desc_is_last(rxd))
    {
        rxd->w0 = DS0_R | DS0_EOR;
        cmd->rxnext = 0;
    }
    else
    {
        rxd->w0 = DS0_R;
        cmd->rxnext++;
    }
    return desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param status
 *  \param pbuf
 *  \param plen
 *  \param pdata
 *  \return
 */
short cm_rxd_get(cmdev * cmd, unsigned long *status, unsigned long *pbuf,
                 unsigned long *plen, void **pdata)
{
    short desc = cmd->rxbusy;
    DES *rxd = cmd->rxd + desc;

    if (cm_desc_owned_by_dma(rxd))
        return -1;
    if (cm_desc_is_empty(rxd))
        return -1;
    DEBUGINFO("rxd: %x %x\n", rxd->w0, rxd->w1);
    *status = rxd->w0;
    *plen = (rxd->w0 << 5) >> (DS0_LEN_S + 5);
    *pbuf = phy_to_virt((rxd->w1 << 4) >> 4);
#ifdef  CONFIG_BEFORE_01162007
    *pdata = (void *) (((unsigned long) *pbuf - 4) & ~0x7f);
#else
    {
        *pdata = (void *) (((unsigned long) *pbuf) & ~0x7f);
    }
#endif

    cm_init_desc(rxd, cm_desc_is_last(rxd));
    cmd->rxbusy = cm_desc_is_last(rxd) ? 0 : desc + 1;

    return desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param buf
 *  \param len
 *  \param key
 *  \return
 */
short cm_txd_set(cmdev * cmd, unsigned long buf, unsigned long len, void *key)
{
    unsigned long w0;
    short desc = cmd->txnext;
    DES *tx = cmd->txd + desc;
    BDS *bd = cmd->txbd + desc;

    if (!cm_desc_is_empty(tx))
        return -1;

    tx->w1 = (buf << 4) >> 4;
    w0 = (DS0_T | DS0_DF) | (len << DS0_LEN_S) |
        (cm_desc_is_last(tx) ? DS0_EOR : 0);

    DEBUGINFO("txd set: w0=%x %x\n", w0, tx->w1);
    tx->w0 = w0;
    bd->key = key;

    cmd->txnext = cm_desc_is_last(tx) ? 0 : desc + 1;

    return desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param status
 *  \param pbuf
 *  \param plen
 *  \param key
 *  \return
 */
short cm_txd_get(cmdev * cmd, unsigned long *status, unsigned long *pbuf,
                 unsigned long *plen, void **key)
{
    short desc = cmd->txbusy;

    DES *tx = cmd->txd + desc;
    if (cm_desc_owned_by_txdma(tx))
        return -1;
    if (cm_desc_is_empty(tx))
        return -1;

    *status = tx->w0;
    *pbuf = (tx->w1 << 4) >> 4;
    *plen = (tx->w0 << 5) >> (DS0_LEN_S + 5);
    *key = cmd->txbd[desc].key;
    cmd->txbd[desc].key = 0;

#ifdef  DEBUG_DESC
    memcpy(tx + NUM_DESC * 2, tx, 8);
#endif

    cm_init_desc(tx, cm_desc_is_last(tx));
    cmd->txbusy = cm_desc_is_last(tx) ? 0 : desc + 1;

    return desc;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
int cm_buf_setup(cmdev * cmd)
{
    struct nbuf *first, *cb, *nb;
    unsigned long t;
    int i;

    if (!(cb = first = NBUF_GET(NBUF_HW)))
    {
        DEBUGINFO("no head buf \n");
        goto no_buf;
    }
    DEBUGINFO("head buf=%x\n", first->head);

    for (i = 0; i < cmd->bufnum; i++)
    {
        if (!(nb = NBUF_GET(NBUF_HW)))
        {
            DEBUGINFO(" no buf \n");
            goto free;
        }
        /* current buf's head point to next buf's head */
        *(unsigned long *) cb->head = virt_to_phy(nb->head);
        cb = nb;
    }
    DEBUGINFO("last buf=%x\n", cb->head);
    *(unsigned long *) cb->head = 0;    //last
    cmd->pktbuf = MREG(MBUFB) = virt_to_phy(first->head);
    MREG(MLBUF) = virt_to_phy(cb->head);
    return 0;

  free:
    cb = first;
    for (i = 0; !cb && i < cmd->bufnum; i++)
    {
        t = phy_to_virt((*(unsigned long *) (first->head)));
        NBUF_PUT(cb, NBUF_SWFREE);
        if (0 == t)             //last
            break;
        cb = (struct nbuf *) (t - NB_HEAD_SZ);
    }
  no_buf:
    return -1;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_hwd_setup(cmdev * cmd)
{
    short i;
    for (i = 0; i < cmd->hwdnum - 1; i++)
        cm_init_rx_desc(cmd->hwd + i, 0);
    cm_init_rx_desc(cmd->hwd + i, 1);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_rxd_setup(cmdev * cmd)
{
    short i;
    for (i = 0; i < cmd->rxdnum - 1; i++)
        cm_init_rx_desc(cmd->rxd + i, 0);
    cm_init_rx_desc(cmd->rxd + i, 1);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_txd_setup(cmdev * cmd)
{
    short i;
    for (i = 0; i < cmd->txdnum - 1; i++)
        cm_init_desc(cmd->txd + i, 0);
    cm_init_desc(cmd->txd + i, 1);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
inline void cm_rx_start(cmdev * cmd)
{
    MREG(MCR) |= (MCR_RDE | MCR_RE);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_rx_stop(cmdev * cmd)
{
    int i;
    MREG(MCR) &= ~(MCR_RDE | MCR_RE);
    for (i = 0; i < cmd->rxdnum; i++)
        cm_desc_take(cmd->rxd + i);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_tx_start(cmdev * cmd)
{
    MREG(MCR) |= (MCR_TDE | MCR_TE);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_tx_stop(cmdev * cmd)
{
    int i;
    MREG(MCR) &= ~(MCR_TDE | MCR_TE);
    for (i = 0; i < cmd->txdnum; i++)
        cm_desc_take(cmd->txd + i);
}

/*!
 * function:
 *
 *  \brief
 *  \param status
 *  \return
 */
int cm_rxd_valid(unsigned long status)
{
    if (!(status & DS0_D))      /* not drop */
    {
        return 1;
    }
    else
    {
        if (!(status & DS0_ECRC))       /* not crc */
            return 1;
    }
// error
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param status
 *  \return
 */
inline int cm_txd_valid(unsigned long status)
{
    return (!(status & DS0_TST_M));
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
inline void cm_int_enable(cmdev * cmd)
{
    MREG(MIM) &= ~MIM_INIT;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
inline void cm_int_disable(cmdev * cmd)
{
    MREG(MIM) |= MIM_INIT;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
inline void cm_int_clr(cmdev * cmd)
{
    MREG(MIC) = MREG(MIS);
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
inline void cm_tx_trig(cmdev * cmd)
{
    MREG(MTT) = 1;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
unsigned long cm_int_get(cmdev * cmd)
{
    unsigned long status = MREG(MIS);
    MREG(MIC) = status;
    return status;
}

int cm_probed = 0;

/*!
 * function:
 *
 *  \brief
 *  \param dev
 *  \return
 */
static void cm_rx(struct net_device *dev)
{
    cmdev *cmd;
    unsigned long status, len, buf;
    struct nbuf *skb, *nskb;
    short max = 16;
    short rc;

    DEBUGINFO("%s\n", dev->name);
    cmd = ((priv *) dev->priv)->cm;

    for (; max-- > 0;)
    {
        rc = cm_rxd_get(cmd, &status, &buf, &len, (void **) &skb);
        if (rc < 0)
            break;
        cmd->rx_pkt++;
        DEBUGINFO("%s - get rxd%d %08x %08x\n", dev->name, rc, status, buf);
        if (cm_rxd_valid(status))
        {
            if (!(nskb = NBUF_GET(NBUF_RXSET)))
                goto set_rxd;   //reuse if no buf

            skb->len = len;
            skb->data = (unsigned char *) buf;
            skb->tail = skb->head + len;
            skb->dev = dev;
            skb->id2 = NBUF_NET;
#if 0                           // debug dump of RX SKB
            {
                int i;

                printf("\n======= SKB %08x %d\n", skb->data, skb->len);
                for (i = 0; i < skb->len; i++)
                {
                    printf("%02x ", skb->data[i]);
                    if ((i % 16) == 15)
                        printf("\n");
                }
                printf("\n=======\n");
            }
#endif
            netif_rx(skb);
            skb = nskb;
        }
      set_rxd:
        //dcache_flush();
        rc = cm_rxd_set(cmd, virt_to_phy(skb->head));
        if (rc < 0)
        {
            DEBUGINFO("err: set rx des\n", dev->name);
            NBUF_PUT(skb, NBUF_NET);
        }
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param dev
 *  \return
 */
static void cm_txdone(struct net_device *dev)
{
    cmdev *cmd;
    struct nbuf *key = NULL;
    unsigned long status, buf, len;
    int i;

    DEBUGINFO("txdone(%s)\n", dev->name);
    cmd = ((priv *) dev->priv)->cm;

    for (;;)
    {
        i = cm_txd_get(cmd, &status, &buf, &len, (void **) &key);
        if (i < 0)
            break;
        DEBUGINFO("%s - get txd%d %08x %08x\n", dev->name, i, status, buf);
        NBUF_PUT(key, NBUF_TXDONE);
    }

}

/*!
 * function:
 *
 *  \brief
 *  \param dev_id
 *  \return
 */
static void cm_isr(void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    cmdev *cmd;
    unsigned long irq;
    int loop = 4;

    cmd = ((priv *) dev->priv)->cm;

    while (0 != (irq = cm_int_get(cmd)))
    {
//      DEBUGINFO("irq = %x\n", irq);
        if (irq & MIS_RX)
        {
            DEBUGINFO("%s - rx\n", dev->name);
            cm_rx(dev);
        }

        if (irq & MIS_TX)
        {
            DEBUGINFO("%s - tx\n", dev->name);
            cm_txdone(dev);
        }

        if (--loop <= 0)
            break;
    }

    cm_int_enable(cmd);
//  DEBUGINFO ("%s - return\n", dev->name);
}

/*!
 * function:
 *
 *  \brief
 *  \param dev
 *  \return
 */
static int cm_open(struct net_device *dev)
{
    cmdev *cmd;

    DEBUGINFO("%s\n", dev->name);

    cmd = ((priv *) dev->priv)->cm;

    if (cm_hw_setup(cmd, (DES *) uncached_addr(&g_desc[0])) != 0)
    {
        DEBUGINFO("err hw init\n");
        return -ENODEV;
    }

    if (cm_mac_setup(cmd, dev->dev_addr) != 0)
    {
        DEBUGINFO("err init MAC\n");
        return -ENODEV;
    }

    if (cm_buf_setup(cmd))
        goto err;
    cm_phy_init(PHY_ETH1_IDX);
    cm_hwd_setup(cmd);          //h/w descriptors init
    cm_rxd_setup(cmd);
    cm_txd_setup(cmd);
    dcache_flush();
#ifndef CONFIG_ETH_POLLING
    request_irq(dev->irq, &cm_isr, dev);
#endif
    cm_int_clr(cmd);
    cm_int_enable(cmd);

    cm_rx_start(cmd);
    cm_tx_start(cmd);
    /* turn on switch to cpu port */
    cm_set_cpu_port();
    dev->start = 1;

    return 0;

  err:
    return -1;
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_free_hwd(cmdev * cmd)
{
    short i;
    struct nbuf *key;
    unsigned long status, dummy;
    for (;;)
    {
        i = cm_rxd_get(cmd, &status, &dummy, &dummy, (void **) &key);
        if (i < 0 || key == 0)
            break;
        DEBUGINFO("hwd%d for skb %08x\n", i, key);
        NBUF_PUT(key, NBUF_HWFREE);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_free_rxd(cmdev * cmd)
{
    short i;
    struct nbuf *key = NULL;
    unsigned long status, dummy;
    for (;;)
    {
        i = cm_rxd_get(cmd, &status, &dummy, &dummy, (void **) &key);
        if (i < 0 || key == 0)
            break;
        DEBUGINFO("rxd%d key: %08x\n", i, key);
        NBUF_PUT(key, NBUF_RXFREE);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \return
 */
void cm_free_txd(cmdev * cmd)
{
    short i;
    struct nbuf *key = NULL;
    unsigned long status, dummy;
    for (;;)
    {
        i = cm_txd_get(cmd, &status, &dummy, &dummy, (void **) &key);
        if (i < 0 || key == 0)
            break;
        DEBUGINFO("txd%d key: %08x\n", i, key);
        NBUF_PUT(key, NBUF_TXFREE);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \param dev
 *  \return
 */
static int cm_stop(struct net_device *dev)
{
    cmdev *cmd;

    DEBUGINFO("%s\n", dev->name);

    cmd = ((priv *) dev->priv)->cm;
    dev->start = 0;

    cm_int_disable(cmd);
    cm_rx_stop(cmd);
    cm_tx_stop(cmd);

    free_irq(dev->irq, dev);

    cm_free_hwd(cmd);
    cm_free_rxd(cmd);
    cm_free_txd(cmd);
    cm_free_buf(cmd);

    return ERR_OK;
}

/*!
 * function:
 *
 *  \brief
 *  \param skb
 *  \param dev
 *  \return
 */
static int cm_tx(struct nbuf *skb, struct net_device *dev)
{
    cmdev *cmd;
    short rc;

    DEBUGINFO("%s\n", dev->name);
    cmd = ((priv *) dev->priv)->cm;

    dcache_flush();
    rc = cm_txd_set(cmd, virt_to_phy(skb->data), skb->len, skb);
    if (rc < 0)
    {
        DEBUGINFO("%s - no txdes\n", dev->name);
        return -EBUSY;
    }
    DEBUGINFO("%s - set txd%d skb %08x\n", dev->name, (int) rc, skb);
    cm_tx_trig(cmd);
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param dev
 *  \param data
 *  \param command
 *  \return
 */
static int cm_ioctl(struct net_device *dev, void *data, int command)
{
    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
int cm_probe()
{
    struct net_device *dev;
    priv *pr;

    dev = (struct net_device *) netdev_alloc();

    dev->addr_len = 6;
    dev->base_addr = MAC_BASE;
    dev->priv = pr = &g_priv[0];
    dev->irq = IRQ_MAC;
    memset((void *) pr, 0, sizeof (priv));
    pr->cm = &g_cmdev;
    pr->vid = 1;

    DEBUGINFO("pr->cm= %08lx\n", &pr->cm);
    cm_init(pr->cm, dev->base_addr);
    memcpy(dev->dev_addr, g_mac_addr, dev->addr_len);
    dev->dev_addr[5] = cm_probed + 1;

    DEBUGINFO("addr = %08lx, irq=%d\n", dev->base_addr, dev->irq);

    dev->open = &cm_open;
    dev->hard_start_xmit = &cm_tx;
    dev->stop = &cm_stop;
    dev->do_ioctl = &cm_ioctl;
    dev->poll = &cm_isr;

    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
int cm_unprobe()
{
    cm_probed = 0;
    return ERR_OK;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cm_wan_enable()
{
    cm_mdio_wr(20, 6, 0x3f3f);
    DEBUGINFO("undo WAN port isolation\n");
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cm_preset_175d()
{
    //printf("Isolated CPU,WAN port\n");
    // WAN is port 4
//  cm_mdio_wr(20, 6, 0x0f3f);
}

/*!
 * function:
 *
 *  \brief
 *  \param phyno
 *  \return
 */
short cm_phy_status(short phyno)
{
    short ret = 0;
    short phy, ctl, sts;
    int retry = 20;

#ifdef CONFIG_I2CS
    extern short i2cs_phy_status(short phyno);
    if (i2cs_phy_status(phyno))
        return PHY_FDX;         // 10M FULL
#endif

    /* dummy read to release link down latch */
    sts = cm_mdio_rd(phyno, 1);
    sts = cm_mdio_rd(phyno, 1);

    if (sts & PHYR_ST_AN)
        ret |= PHY_AN;

    if (sts & PHYR_ST_LINK)
        ret |= PHY_LINK;
    else
    {
        /* Once link partner disable AN, 
           PHY need more delay time for link up */
        if ((cm_mdio_rd(phyno, 0) & 0x1000))
        {
Retry:
            /* Delay for link partner disable AN */
            mdelay(500);
            sts = cm_mdio_rd(phyno, 1);
            printf(".");
            if (sts & PHYR_ST_LINK)
            {
                ret |= PHY_LINK;
                goto get_cap;
            }
            else if(retry-- > 0)
            {
                goto Retry;
            }
            else
            {
                printf(" timeout");
            }
        }
        goto ret_;
    }

  get_cap:
    if (sts & PHYR_ST_ANC)      // AN Complete
    {
        ret |= PHY_ANC;
        phy = cm_mdio_rd(phyno, 4);     // mine
        ctl = cm_mdio_rd(phyno, 5);     // link parter
        if ((phyid[PORT_ETH1_IDX] == PHYID_MONT_EPHY) && (ctl == 0))
        {
            sts = cm_mdio_rd(phyno, 31);
            sts &= 0x7;
            /* Swap to page 0 */
            cm_mdio_wr(phyno, 31, 0);
            ctl = cm_mdio_rd(phyno, 19);        // result of parallel detection
            cm_mdio_wr(phyno, 31, sts);
            if (ctl & 0x10)
                ret |= PHY_100M;
            if (ctl & 0x8)
                ret |= PHY_FDX;
        }
        else
        {
            phy = 0x3e0 & (phy & ctl);  // link parter
            if (PHY_CAP_100F & phy)
                ret |= (PHY_100M | PHY_FDX);
            else if (PHY_CAP_100H & phy)
                ret |= (PHY_100M);
            else if (PHY_CAP_10F & phy)
                ret |= (PHY_FDX);
        }
    }
    else                        // force
    {
        ctl = cm_mdio_rd(phyno, 0);
        if (ctl & PHYR_CTL_SPEED)
            ret |= PHY_100M;
        if (ctl & PHYR_CTL_DUPLEX)
            ret |= PHY_FDX;
    }
  ret_:
    return ret;
}

/*!
 * function:
 *
 *  \brief
 *  \param idx
 *  \param ins_vlan
 *  \param calibration
 *  \return
 */
void cm_switch_port_attach(unsigned char idx, unsigned char ins_vlan,
                           unsigned char calibration)
{
    unsigned int reg_off, val;
    unsigned int clk;

    //printk("cm_switch_port_attach %d\n", idx);
    if (idx == PORT_WLAN_IDX)
        goto p_enable;

    reg_off = idx * 0x100;

    /* letency = (system clock / MII clock) * 8 */
#ifdef CONFIG_FPGA
    clk = 2;
#else
    clk = (bootvars.hver >> 16) & 0xf;
    clk = (clk) ? 1 : 0;
#endif
    SWREG_WRITE32(reg_off | EMAC_TXRX_MISC,
                  ((MAX_FORWARD_LEN << 16) | cycle[clk][0]) - 1);
    SWREG_WRITE32(reg_off | EMAC_RX_CFG, RX_USE_LU_VLAN | RM_VLAN | TIDMAP_TOS);
    /* Not insert VLAN */
    val =
        (INTERFRAME_GAP << 16) | (PREAMBLE_BYTE_SIZE << 12) | PAYLOAD_FLOWCTRL |
        USE_DESC_VID | AUTO_RETRY;
    if (idx != PORT_HNAT_IDX)
    {
        printk("Waiting PHY linkup ");
        an_status[idx] = cm_phy_status(PHY_ETH1_IDX);
        //printk("an_status[%d]: %x\n", idx, an_status[idx]);
        printk(" (%x)\n", an_status[idx]);
        if (!calibration)
        {
            val |= (an_status[idx] & PHY_FDX) ? 0 : COLIISION_DROP16;
        }
    }
#ifndef CONFIG_FPGA
    val |= TX_DEBUG;
#endif
    SWREG_WRITE32(reg_off | EMAC_TX_CFG, val);

    val = ((IP_OFFSET >> 2) << 12) | TXEN | RXEN;

    if (idx != PORT_HNAT_IDX)
    {
        if (calibration)
            val |= BITRATE_100M;
        else
        {
            val |= (an_status[idx] & PHY_100M) ? BITRATE_100M : BITRATE_10M;
            val |= (an_status[idx] & PHY_FDX) ? 0 : HALF_DUPLEX;
        }
#ifdef CONFIG_FPGA
        val |= ((tx_clk) ? TXCLK_INV : 0);
        val |= ((rx_clk) ? RXCLK_INV : 0);
#else
        /* FIXME: on board external phy must invert tx/rx clock ??? */
        if ((phyid[idx] == PHYID_ICPLUS_IP101G) && (idx == 1))
            val |= (TXCLK_INV | RXCLK_INV);
        else if (phyid[idx] == PHYID_MONT_EPHY)
            val |= RXCLK_INV;
#endif

        /* New CRS scheme */
        /* bit[29:24]:  upper_bound
           bit[21:16]:  100M CRS initial value
           bit[13:8]:   10M CRS initial value
           bit[7:0]:    txen initial value */
        val |= CRS_SCHEME;
        if (phyid[idx] == PHYID_MONT_EPHY)
        {
            /* FIXME: txen=920ns */
            SWREG_WRITE32(reg_off | EMAC_IFG, 0x18060104);
        }
        else
        {
#ifdef CONFIG_FPGA
            SWREG_WRITE32(reg_off | EMAC_IFG, 0x16030101);
#else
            /* FIXME: txen=920ns */
            SWREG_WRITE32(reg_off | EMAC_IFG, 0x16030102);
#endif
        }
    }
    SWREG_WRITE32(reg_off | EMAC_CFG, val);
  p_enable:
    val = (1 << idx);
    val = (val << 16) | ((ins_vlan << idx) << 8) | val;
    SWREG_UPDATE32(SW_PORT_CFG, val, val);
}

/*!
 * function:
 *
 *  \brief
 *  \param p
 *  \return
 */
void cm_phy_init(unsigned char p)
{
    unsigned int id;
    unsigned short id1, id2;

    //printk("cm_phy_init %d\n", p);
    id1 = cm_mdio_rd(p, 2);
    id2 = cm_mdio_rd(p, 3);
    id = (id1 << 16) | id2;

#ifdef CONFIG_I2CS
    extern int i2cs_phy_int(unsigned char p, unsigned int id);
    if (i2cs_phy_int(p, PHYID_MONT_EPHY))
    {
        phyid[PORT_ETH1_IDX] = id;
        return;
    }
#endif

    if (id == PHYID_ICPLUS_IP101G)
    {
        /* Config IP101G to version 1.2 */
        cm_mdio_wr(p, 20, 16);
        cm_mdio_wr(p, 16, 0x1006);
#if defined(CONFIG_FPGA)
        /* Config IP101G RXC driving current = 12.96mA */
        cm_mdio_wr(p, 20, 4);
        cm_mdio_wr(p, 22, 0xa000);
        /* Config IP101G TXC driving current = 12.96mA */
        cm_mdio_wr(p, 20, 16);
        cm_mdio_wr(p, 27, 0x0015);
        /* Config IP101G RXD driving current = 8.10mA */
        cm_mdio_wr(p, 20, 16);
        cm_mdio_wr(p, 26, 0x5b6d);
#endif
        /* Fallback to 100M, Full duplex mode */
        cm_mdio_wr(p, 4, 0x05e1);
        cm_mdio_wr(p, 0, 0xb100);
    }
    else if (id == PHYID_MONT_EPHY)
    {
#if defined(CONFIG_FPGA)
        /* Swap to page 1 */
        cm_mdio_wr(p, 31, 1);
        cm_mdio_wr(p, 16, 0xb5a0);
        /* AFE tx control: enable termination impedance calibration */
        cm_mdio_wr(p, 17, 0xa528);
#if (CONFIG_REF_CLK==25000000)
        cm_mdio_wr(p, 18, (cm_mdio_rd(p, 18) & 0x07ff));
        cm_mdio_wr(p, 18, (cm_mdio_rd(p, 18) | 0x1800));
#endif
        cm_mdio_wr(p, 19, 0xa4d8);
        /* AFE rx control */
        cm_mdio_wr(p, 20, 0x3780);
        /* ADC VCM = 1.00 */
        cm_mdio_wr(p, 22, (cm_mdio_rd(p, 22) | 0x6000));
#if (CONFIG_REF_CLK==25000000)
        cm_mdio_wr(p, 23, (cm_mdio_rd(p, 23) & 0xe01f));
        cm_mdio_wr(p, 23, (cm_mdio_rd(p, 23) | 0x1500));
#endif
        /* Swap to page 2 */
        cm_mdio_wr(p, 31, 2);
        /* AGC thresholds, org=0x4030 */
        cm_mdio_wr(p, 17, 0x8059);
        /* DSP initial val */
        cm_mdio_wr(p, 18, 0x8975);
        cm_mdio_wr(p, 19, 0xba60);
        /* Swap to page 4 */
        cm_mdio_wr(p, 31, 4);
        /* 10T signal detection time control, org=0x5aa0 */
        cm_mdio_wr(p, 18, 0x5a40);
        /* Swap to page 0 */
        cm_mdio_wr(p, 31, 0);
        /* RMII V1.2 */
        cm_mdio_wr(p, 19, (cm_mdio_rd(p, 19) | 0x0040));
        /* Enable MDIX */
        cm_mdio_wr(p, 20, (cm_mdio_rd(p, 20) | 0x3000));
#else

        //Digital Settings
        //Page 0
        cm_mdio_wr(p, 0x1f, 0x0);
        //Enable Polarity reverse, Auto-MDIX enable
        cm_mdio_wr(p, 0x14, 0x6000);

        //Page 2
        cm_mdio_wr(p, 0x1f, 0x2);
        //agc pulse upper/lower threshold;
        cm_mdio_wr(p, 0x11, 0x8059);
        //100BTX receiver control register 3, TRL loop filter gain
        cm_mdio_wr(p, 0x12, 0x8975);
        //100BTX receiver control register 4, equlizer step 0,
        cm_mdio_wr(p, 0x13, 0x6a60);

        //Page 4
        cm_mdio_wr(p, 0x1f, 0x4);
        //singal detection time control
        cm_mdio_wr(p, 0x12, 0x5a40);

        //Analog Settings
        //Page 1
        cm_mdio_wr(p, 0x1f, 0x1);
        //TX Swing and Low Power Mode,lpmode_100, txamp_100
        cm_mdio_wr(p, 0x10, 0xb5a0);
        //TX Common Mode, txvcm, rterm_ext, using calibration results
        cm_mdio_wr(p, 0x11, 0xa528);
        //Set PGA_test_en[1:0] to 1 for correct working mode
        cm_mdio_wr(p, 0x1b, 0x00c0);
        //AFE Clock gen control register 2
        cm_mdio_wr(p, 0x18, 0xf400);
        //RX controll regster 2, sd_level, unsd_level
        cm_mdio_wr(p, 0x13, 0xa4d8);
        //RX controll regster 3, lpf_ctune_cal, rx_lpf
        cm_mdio_wr(p, 0x14, 0x3780);
        //RX controll regster 4, lpf_ctune_ext_en, using external value
        cm_mdio_wr(p, 0x15, 0xb600);
        //RX controll regster 5, RX Common mode
        cm_mdio_wr(p, 0x16, 0xf900);

        //Page 0
        cm_mdio_wr(p, 0x1f, 0x0);
        // set bit[15]=1 to reset, bit[13]=1 set 100Mb/s, bit[12]=1 enable auto-negotiation process.
        cm_mdio_wr(p, 0x00, 0xb100);
#endif
    }
    else
    {
        printf("Unknow phy id:%x\n", id);
    }
    phyid[PORT_ETH1_IDX] = id;
    /* Delay for reset */
    //mdelay(2000);
}

#ifdef CONFIG_ETH_CALIBRATION
/*!
 * function:
 *
 *  \brief
 *  \param p
 *  \param action
 *  \return
 */
void cm_phy_loopback(unsigned char p, unsigned int action)
{
    unsigned short val;

    val = cm_mdio_rd(p, 0);
    if (action)
    {
        /* Enable loopback, disable AN, force 100M full mode */
        val |= 0x6100;
        val &= ~0x1000;
        cm_mdio_wr(p, 0, val);
    }
    else
    {
        /* Disable loopback, enable and restart AN */
        val &= ~0x4000;
        val |= 0x1200;
        cm_mdio_wr(p, 0, val);
    }
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */
int cm_cal_get(void)
{
    cmdev *cmd = &g_cmdev;
    return cmd->rx_pkt;
}

/*!
 * function:
 *
 *  \brief
 *  \param idx
 *  \param txclk
 *  \param rxclk
 *  \return
 */
void cm_cal_set(unsigned char idx, unsigned char txclk, unsigned char rxclk)
{
    tx_clk = txclk;
    rx_clk = rxclk;
    cm_switch_port_attach(idx, 0, 1);
}
#endif

/*!
 * function:
 *
 *  \brief
 *  \return
 */
void cm_set_cpu_port()
{
    volatile short i;
#if !defined(SWITCH_BUF_INSRAM)
    buf_header *hw_bhdr = (buf_header *) uncached_addr(hw_buf_headers);
#else
    //buf_header *hw_bhdr = (buf_header *) uncached_addr(hw_buf_headers);
    buf_header *hw_bhdr = (buf_header *) (0xb0007e00UL);
    unsigned char *buf = (unsigned char *) (0xb0004000UL);
#endif
    unsigned int clk;

    /* wait switch ready */
    DEBUGINFO("Reset switch \n");
#if defined(SWITCH_BUF_INSRAM)
    DEBUGINFO("hw_bhdr %p, buf %p\n", hw_bhdr, buf);
#else
    DEBUGINFO("hw_bhdr %p, fastpath_buf %p\n", hw_bhdr, fastpath_buf);
#endif
    /*
       Do not change this reset sequence
     */
    /* Provide clk to HNAT and SWITCH module */
#if !defined(CONFIG_PANTHER)
    MACREG_UPDATE32(HW_RESET, 0, HW_RESET_HNAT_PAUSE | HW_RESET_SWITCH_PAUSE);
    cm_switch_reset();
#endif

#if !defined(CONFIG_PANTHER)
    DEBUGINFO("Within 5s, Wait SW ready..\n");
    while (tmp--)
    {
        if ((MACREG_READ32(REVISION_REG) >> 16) == 0x3281)
            break;
        mdelay(50);
    }
    if (tmp == 0)
        printf("!!!Error SW isn't ready..\n");
#endif

    memset((void *) hw_bhdr, 0, sizeof (buf_header) * FASTPATH_BUFFER_HEADER_POOL_SIZE);
#if defined(SWITCH_BUF_INSRAM)
    memset((void *) buf, 0, DEF_FASTPATH_BUF_SIZE * FASTPATH_BUFFER_HEADER_POOL_SIZE);
#endif
    /*
       chain buffer headers from element[0] to element[info->hw_buf_headers_count-1]
       this linklist is used for HW as buffer to store & forward frame in fastpath
     */
    for (i = 0; i < FASTPATH_BUFFER_HEADER_POOL_SIZE; i++)
    {
        hw_bhdr[i].next_index = i + 1;
        hw_bhdr[i].offset = 0;
        hw_bhdr[i].ep = 0;
#if !defined(SWITCH_BUF_INSRAM)
        hw_bhdr[i].dptr =
            (unsigned int)
            virt_to_phy(&fastpath_buf[i * DEF_FASTPATH_BUF_SIZE]);
#else
        hw_bhdr[i].dptr =
            (unsigned int) virt_to_phy(&buf[i * DEF_FASTPATH_BUF_SIZE]);
#endif
    }
    hw_bhdr[i - 1].next_index = 0;
    hw_bhdr[i - 1].ep = 1;
#if !defined(CONFIG_PANTHER)
    /* FIXME: Enalbe Wlan lookup table avoid switch DA lookup fail once wifi not bring up. */
    MACREG_WRITE32(STA_DS_TABLE_CFG,
                   STA_DS_TABLE_CFG_DONE | STA_DS_TABLE_CACHE_CFG |
                   STA_TABLE_HASH_MASK);
#endif
    /*
       Enable buffer header process
     */
    SWREG_WRITE32(SW_HWBUF_HDR_BADDR, PHYSICAL_ADDR(hw_bhdr));
    /* the link list head & tail index are simply ( 0  , total element of macframe headers - 1 ) */
    SWREG_WRITE32(SW_HWBUF_HDR_HT, (FASTPATH_BUFFER_HEADER_POOL_SIZE - 1));
    /* Set the buffer sizes of HW pool */
    SWREG_WRITE32(SW_HWBUF_SIZE, DEF_FASTPATH_BUF_SIZE);
    SWREG_WRITE32(SW_HWBUF_POOL_CTRL, 1);
    /*
       Enable buffer flow control
     */
    SWREG_WRITE32(SW_HWBUF_FC1, 0x00100010);
    SWREG_WRITE32(SW_HWBUF_FC2, 0x00000000);
    /* Config timer for 1ms period */
#ifdef CONFIG_FPGA
    clk = 0xf423;
#else
    /* 1ms in sysclk 160MHz is 160k ticks */
    clk = 160000;
#endif
    SWREG_WRITE32(SW_TIMER, clk);

#if defined(CONFIG_P0_AS_LAN)
    /* LAN group hnat, and port 0 */
    SWREG_WRITE32(SW_VLAN_GROUP0, 0x00050001);
#else
    /* LAN group hnat, and port 1 */
    SWREG_WRITE32(SW_VLAN_GROUP0, 0x00060001);
#endif

    /* default port VID */
    SWREG_WRITE32(SW_PORT_VID0_1, 0x00010001);
    SWREG_WRITE32(SW_PORT_VID2_3, 0x00010001);

    /* Turn on HNAT port and P0 */
    cm_switch_port_attach(PORT_HNAT_IDX, 0, 0);
    cm_switch_port_attach(PORT_ETH1_IDX, 0, 0);

    DEBUGINFO("Forward CPU port\n");
}

#ifdef CONFIG_CMD_ET
/*!
 * function:
 *
 *  \brief
 *  \param cmd
 *  \param len
 *  \param pattern
 *  \param loop
 *  \param mode
 *  \return
 */
int cm_tx_test(cmdev * cmd, unsigned long len, int pattern, unsigned int loop,
               short mode)
{
    unsigned long buf = 0xa0100000;
    DES *tx = (DES *) 0xa0110000;
    int num = 512;
    int i, to;
    int w0 = (DS0_T | DS0_DF) | (len << DS0_LEN_S);
    cm_rx_stop(cmd);
    cm_tx_stop(cmd);
    unsigned char *pbuf = (unsigned char *) buf;

    MREG(MTXDB) = virt_to_phy(tx);
    switch (mode)
    {
        case 1:                //incremental
            for (i = 0; i < 2048; i++)
                pbuf[i] = (unsigned char) i;
            break;
        case 2:                //random
            for (i = 0; i < 8192; i++)
                pbuf[i] = (unsigned char) rand();
            break;
        default:
            memset((void *) buf, (unsigned char) pattern, len);
            break;
    }

    for (i = 0; i < num - 1; i++)
    {
        tx[i].w1 = (buf << 4) >> 4;
        tx[i].w0 = w0;
    }
    tx[i].w0 = w0 | DS0_EOR;

    printf("Init all tx desc w0=%x,w1=%x\n", tx[0].w0, tx[1].w1);
    cm_tx_start(cmd);
    cm_tx_trig(cmd);

    while (loop--)
    {
        to = 5000;
        while (to--)
        {
            if (tx[i].w0 & DS0_T)
            {                   // owned by hw
                mdelay(100);
                continue;
            }
            printf("t");
            goto done;
            break;
        }
        printf("w");
      done:
        for (i = 0; i < num - 1; i++)
        {
            tx[i].w0 = w0;
            if (2 == mode)      //random
                tx[i].w1 = ((buf + (rand() & 0x3ff)) << 4) >> 4;
        }
        tx[i].w0 = w0 | DS0_EOR;
        cm_tx_trig(cmd);
        if (serial_poll())
        {
            if (27 == serial_getc())
            {
                while (to--)
                {
                    if (tx[i].w0 & DS0_T)
                    {           // owned by hw
                        mdelay(100);
                        continue;
                    }
                    printf("t");
                    break;
                }
                break;
            }
        }
    }
    cm_tx_stop(cmd);

    return 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int eth_open(int intf);
int eth_tx_cmd(int argc, char *argv[])
{
    unsigned long len, val, loop;
    len = 1514;
    val = 0x55;
    loop = -1;
    short mode = 0;

    if (argc > 0 && !hextoul(argv[0], &len))
        goto err1;
    if (argc > 1)
    {
        switch (argv[1][0])
        {
            case 'i':
                mode = 1;
                break;
            case 'r':
                mode = 2;
                break;
            default:
                if (!hextoul(argv[1], &val))
                    goto err1;
        }
    }
    if (argc > 2 && !hextoul(argv[2], &loop))
        goto err1;
    eth_open(0);
    while (1)
    {
#if !defined(CONFIG_FPGA)
        // set GPIO27 to GPIO input mode
        GPREG(0x24) &= ~(0x0fUL << 12);
        PMUREG(0x38) |= (1 << 27);
        PMUREG(0x2C) |= (1 << 27);
#endif
        //printk("=== GPIO27 %d %d\n", !!(PMUREG(0x58) & (1 << 27)), !!(GPREG(0x0) & (1 << 27)));
        if(PMUREG(0x58) & (1 << 27))
        {
            cm_tx_test(&g_cmdev, len, val, loop, mode);
        }
        if (serial_poll())
        {
            if (27 == serial_getc())
            {
                break;
            }
        }
        //mdelay(100);
    }
    return ERR_OK;

  err1:
    return ERR_PARM;

}

cmdt cmdt_etx[] __attribute__ ((section("cmdt"))) =
{
    {
    "et", eth_tx_cmd, "et  <len> <pattern> <loop>;ether tx test"}
,};
#endif

#ifdef CONFIG_CMD_SWITCH
#include <netprot.h>
/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */

int acl_cmd(int argc, char *argv[])
{
    unsigned long val;
    unsigned short msb, lsb, sp = 0, prot = 0, dip_en = 0;
    unsigned int dip = bootvars.server, i;

    if (0 < argc)
    {
        if (!hextoul(argv[0], &val) || (0 > val) || (6 < val))
        {
            printf("error phy sp[0-6]\n");
            goto err;
        }
        sp = 0x8000 | ((1 << val) << 8);
    }
    if (1 < argc)
    {
        if (i = inet_aton(argv[1], &dip))
        {
            printf("error ip dip\n");
            goto err;
        }
        dip_en = 0x003f;
    }
    if (2 < argc)
    {
        if (!hextoul(argv[2], &val) || (0 > val) || (255 < val))
        {
            printf("error prot num[0-255]\n");
            goto err;
        }
        prot = 0x0100 | val;
    }
    cm_mdio_wr(26, 22, 0x0100);
    lsb = cm_mdio_rd(26, 23);
    msb = cm_mdio_rd(26, 24);
    val = (msb << 16) | lsb;
    /* Enable ACL */
    cm_mdio_wr(25, 1, 0x8080);
    /* Reset ACL */
    cm_mdio_wr(26, 0, 0xc000);
    /* ACL entry */
    cm_mdio_wr(26, 3, dip_en);
    cm_mdio_wr(26, 7, (dip & 0xffff));
    cm_mdio_wr(26, 8, ((dip >> 16) & 0xffff));
    cm_mdio_wr(26, 11, prot);
    cm_mdio_wr(26, 15, sp);
    cm_mdio_wr(26, 20, 0x0001);
    cm_mdio_wr(26, 22, 0x00e0);

    printf("cnt=(%d) bytes\n", val);
    return ERR_OK;

  err:
    return ERR_PARM;

}

cmdt cmdt_acl[] __attribute__ ((section("cmdt"))) =
{
    {
    "acl", acl_cmd, "acl <sp> <ip> <prot>;acl"}
,};

char *desc[4][13] = {
    {
     "-RX_PACKETS",
     "-RX_BYTES",
     "-RX_UNICAST",
     "-RX_BROAD",
     "-RX_MULTI",
     "-RX_ERROR_PACKETS",
     "-RX_CRC",
     "-RX_PAUSE",
     "-RX_DROP",
     "-RX_OVERSIZE",
     "-RX_UNDERSIZE",
     "-RX_JABBER",
     "-RX_FRAGMENTS",
     },
    {
     "-RX_64",
     "-RX_65TO127",
     "-RX_128TO255",
     "-RX_256TO511",
     "-RX_512TO1023",
     "-RX_1024TO1518",
     "-RX_1519TO1792",
     "-RX_Q0",
     "-RX_Q1",
     "-RX_Q2",
     "-RX_Q3",
     },
    {
     "-TX_PACKETS",
     "-TX_BYTES",
     "-TX_UNICAST",
     "-TX_BROAD",
     "-TX_MULTI",
     "-TX_COL",
     "-TX_PAUSE",
     "-TX_OVERSIZE",
     "-TX_ABORT_COL",
     },
    {
     "-TX_64",
     "-TX_65TO127",
     "-TX_128TO255",
     "-TX_256TO511",
     "-TX_512TO1023",
     "-TX_1024TO1518",
     "-TX_1519TO1792",
     "-TX_Q0",
     "-TX_Q1",
     "-TX_Q2",
     "-TX_Q3",
     },
};

/*!
 * function:
 *
 *  \brief
 *  \param port
 *  \param bank
 *  \param reg
 *  \return
 */
void net_mib(int port, int bank, int reg)
{
    unsigned int val, tmp, mib_cnt;
    unsigned short msb, lsb;

    val = 0x8010 | (port << 11) | (bank << 9);
    printf("\nport%d bank%d:\n\n", port, bank);
    for (tmp = 0; tmp <= reg; tmp++)
    {
        cm_mdio_wr(27, 18, (val | (tmp << 5)));
        msb = cm_mdio_rd(27, 20);
        lsb = cm_mdio_rd(27, 21);
        mib_cnt = (msb << 16) | lsb;
        printf("%s - %d\n", desc[bank][tmp], mib_cnt);
    }
    printf("\n");
}

/*!
 * function:
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return
 */
int mib_cmd(int argc, char *argv[])
{
    unsigned long val;
    int port = 0, port_end = 4, bank = 0, bank_end = 3, reg;

    if (0 < argc)
    {
        if (!hextoul(argv[0], &val) || (0 > val) || (4 < val))
        {
            printf("error port num[0-4]\n");
            goto err;
        }
        port = port_end = val;
    }
    if (1 < argc)
    {
        if (!hextoul(argv[1], &val) || (0 > val) || (3 < val))
        {
            printf("error bank num[0-3]\n");
            goto err;
        }
        bank = bank_end = val;
    }
    for (port; port <= port_end; port++)
    {
        for (bank; bank <= bank_end; bank++)
        {
            if (bank == 0)
                reg = 12;
            else if (bank == 1)
                reg = 10;
            else if (bank == 2)
                reg = 8;
            else if (bank == 3)
                reg = 10;
            net_mib(port, bank, reg);
        }
    }
    return ERR_OK;

  err:
    return ERR_PARM;
}

cmdt cmdt_mib[] __attribute__ ((section("cmdt"))) =
{
    {
    "mib", mib_cmd, "mib <port> <bank>;mib"}
,};
#endif
