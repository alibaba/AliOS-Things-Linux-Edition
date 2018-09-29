/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file cm_mac.h
*   \brief Ethernet MAC API
*   \author Montage
*/

#ifndef CM_MAC_H
#define CM_MAC_H

#define FASTPATH_BUFFER_HEADER_POOL_SIZE		(62)
#define DEF_FASTPATH_BUF_SIZE					(0x100)
#define PHYSICAL_ADDR(va)						(((unsigned int)va)&0x1fffffff)

#define SWREG_READ32(x)  (*(volatile unsigned int*)(SW_BASE+(x)))
#define SWREG_WRITE32(x,val) (*(volatile unsigned int*)(SW_BASE+(x)) = (unsigned int)(val))
#define SWREG_UPDATE32(x,val,mask) do {           \
    unsigned int newval;                                        \
    newval = *(volatile unsigned int*) (SW_BASE+(x));     \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));\
    *(volatile unsigned int*)(SW_BASE+(x)) = newval;      \
} while(0)
//#define UMAC_REG_BASE         MAC_BASE
#define MACREG_READ32(x)  (*(volatile unsigned int*)(UMAC_REG_BASE+(x)))
#define MACREG_WRITE32(x,val) (*(volatile unsigned int*)(UMAC_REG_BASE+(x)) = (unsigned int)(val))
#define MACREG_UPDATE32(x,val,mask) do {           \
    unsigned int newval;                                        \
    newval = *(volatile unsigned int*) (UMAC_REG_BASE+(x));     \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));\
    *(volatile unsigned int*)(UMAC_REG_BASE+(x)) = newval;      \
} while(0)

#define MPHY_TR     (1<<31)
#define MPHY_WR     (1<<30)
#define MPHY_RD     (0<<30)
#define MPHY_EXT    (1<<5)
#define MPHY_PA_S   24
#define MPHY_RA_S   16

#define MCR_LB      (1<<0)      //loopback
#define MCR_RE      (1<<1)      //rx enable
#define MCR_FC      (1<<2)      //flow control
#define MCR_LU      (1<<3)      //Link up
#define MCR_RDE     (1<<4)      //RxDMA enable
#define MCR_HN      (1<<5)      //h/w NAT enable
#define MCR_TC      (1<<6)      //all packet to CPU
#define MCR_BU      (1<<7)      //bypass Update
#define MCR_TE      (1<<8)      //tx enable
#define MCR_TFC     (1<<9)      //tx flow control
#define MCR_BL      (1<<10)     //bypass lookup
#define MCR_RLB     (1<<11)     //Rout Loopback
#define MCR_TDE     (1<<12)     //TXDMA en
#define MCR_TRT     (1<<13)     //tx retry
#define MCR_NES     (1<<14)     //nat entry size, 1:32B, 0:24B
#define MCR_HPFD    (1<<15)     //prefetch disable
#define MCR_REV_IDX     (1<<16) //reverse index generate
#define MCR_HEN     (1<<17)     //hmu enable
#define MCR_HCE     (1<<18)     //hmu cached entry
#define MCR_HHFM    (1<<19)     //hash function mode, 1: CRC10, 0:direct
#define MCR_BWC0    (1<<20)     //bandwidth control, drop packet
#define MCR_BWC1    (1<<21)     //bandwidth control, trigger flow-control
#define MCR_HTM     (1<<22)     //hmu test mode
#define MCR_EXTD    (1<<23)     //external descriptor
#define MCR_SMI_S   24
/* In ASIC mode, maximum MDC of ether PHY up to 2.5M,
   we keep MDC to 2M as FPGA */
#ifdef CONFIG_FPGA
#define MCR_SMI_PERIOD	31
#else
#define MCR_SMI_PERIOD	80
#endif

#define DS0_R       (1<<31)     //own by rx
#define DS0_T       (1<<30)     //own by tx
#define DS0_EOR     (1<<29)     //end of des ring
#define DS0_F       (1<<28)     //free
#define DS0_D       (1<<27)     //drop
#define DS0_LEN_S   16
//packet info
#define DS0_PT_S    13          //rx packet type
#define DS0_PT_B    3           //rx packet type, 3bits
#define DS0_IPF     12          //ip fragment
#define DS0_VL      (1<<11)     //vlan valid
#define DS0_VID_S   8           //vlan index
#define DS0_INB     (1<<7)      //inbond direction
#define DS0_TRSF    (1<<6)      //tcp RST/SYN/FIN flag
#define DS0_NATH    (1<<5)      //nat table lookup hit
#define DS0_ALG_S   (1<<0)      //ALG 5bits
//if DES_D (drop is set)
#define DS0_FOR     (1<<15)     //fifo overrun
#define DS0_L3LE    (1<<14)     //layer 3 length error(runt packet/over length/length unmatch)
#define DS0_PPPOEUM (1<<13)     //pppoe unmatch
#define DS0_DAUM    (1<<12)     //destination unmatch
#define DS0_VL      (1<<11)     //vlan valid
#define DS0_VID_S   8           //vlan index
#define DS0_INB     (1<<7)      //inbond direction
#define DS0_L2LE    (1<<6)      //layer 2 length error(runt packet/over length)
#define DS0_NATH    (1<<5)      //nat table lookup hit
#define DS0_ETTL    (1<<4)      //ip's ttl <2
#define DS0_ECRC    (1<<3)      //crc error
#define DS0_EL4CS   (1<<2)      //tcp checksum error
#define DS0_EIPCS   (1<<1)      //ip checksum error
#define DS0_ETSYC   (1<<0)      //tcp sync over

#define DS1_POE     (1<<31)
#define DS1_PSI_S   28
#define DS1_BUF_M   ((1<<28)-1)
//for tx only
#define DS0_DF      (1<<27)     //tx raw packet
#define DS0_TST_S   13          //tx status
#define DS0_TST_B   3           //tx status, 3bits
#define DS0_TST_M   (((1 << DS0_TST_B)-1 )<< DS0_TST_S)
#define DS0_M       (1<<12)     //more fragment in current packet
#define DS0_DMI_S   0           //dest mac index
//s/w mark
#define DES_ER      (1<<12)

#define     MPHY    0x00
#define     MCR 0x04

/// FIXME
#define     MIE_HMU (1<<0)
#define     MIE_TX  (1<<1)
#define     MIE_SQE (1<<2)
#define     MIE_HQE (1<<3)
#define     MIE_BE  (1<<4)
#define     MIE_RX  (1<<5)

#define     MIS_HMU (1<<0)
#define     MIS_TX  (1<<1)
#define     MIS_SQE (1<<2)
#define     MIS_HQE (1<<3)
#define     MIS_BE  (1<<4)
#define     MIS_RX  (1<<5)

#define     MIE_ENABLE  (0x3f)
#define     MIE 0x84
#define     MIM 0x84
#define     MIC 0x88
#define     MIS 0x8C

#define     MSWDB   0x08
#define     MHWDB   0x0c
#define     MBUFB   0x10
#define     MRXCR   0x14
#define     MPOET   0x18
#define     MVL01   0x1c
#define     MVL23   0x20
#define     MVL45   0x24
#define     MVL67   0x28
#define     MTSR    0x2c
#define     MDROPS  0x30
#define     MPKTSZ  0x34
#define     MFP 0x38
#define     MSA00   0x3c
#define     MSA01   0x40
#define     MSA10   0x44
#define     MSA11   0x48
#define         MAUTO   0x4c
#define     MFC1    0x54
#define     MDBUG   0x58
#define     MSRST   0x5c
#define     MMEMB   0x60
#define     MTXDB   0x64
// 0x30 ~
#define     MTT 0xC0
#define     MCT 0xC4
#define     MTTH    0xC8
#define     MTW 0xCC
#define     MARPB   0xD0
#define     MLBUF   0xD4

#define     NHTOBB  0x180
#define     NHTIBB  0x184
#define     NTB 0x188
#define     NSIP    0x194

#define     IDX(a)      (a>>2)

#define virt_to_bus(a) (((unsigned int)a)&0xfffffff)

#define IP_OFFSET				64
#define INTERFRAME_GAP			22
#define MAX_FORWARD_LEN			1536
#define SW_DESC_NUMBER			204
#define PREAMBLE_BYTE_SIZE		7

#define RMII_MII				0x203C
#define P2_PHYMODE_CLK			0x30
#define P2_PHYMODE_MII_100M		0x00000000UL
#define P2_PHYMODE_HALF_SYS		0x00000010UL
#define P2_PHYMODE_RMII			0x00000030UL
#define P0_ADAPTER_MODE			0x40
#define P0_ADAPTER_MII			0x00000000UL
#define P0_ADAPTER_RMII			0x00000040UL
#define P1_ADAPTER_MODE			0x80
#define P1_ADAPTER_MII			0x00000000UL
#define P1_ADAPTER_RMII			0x00000080UL
#define P0_PHYMODE_CLK			0x300
#define P0_PHYMODE_MII_10M		0x00000000UL
#define P0_PHYMODE_MII_100M		0x00000100UL
#define P0_PHYMODE_RMII			0x00000300UL
#define P1_PHYMODE_CLK			0xC00
#define P1_PHYMODE_MII_10M		0x00000000UL
#define P1_PHYMODE_MII_100M		0x00000400UL
#define P1_PHYMODE_RMII			0x00000C00UL
#define P0_RMII_OUT_REFCLK		0x1000
#define P1_RMII_OUT_REFCLK		0x2000
#define P0_MII_MODE				0x4000
#define P0_MII_MAC				0x00000000UL
#define P0_MII_PHY				0x00004000UL
#define P1_MII_MODE				0x8000
#define P1_MII_MAC				0x00000000UL
#define P1_MII_PHY				0x00008000UL

#if !defined(CONFIG_PANTHER)
#define HW_RESET				0x20A0
#define HW_RESET_WMAC				0x00000001UL
#define HW_RESET_HNAT				0x00000002UL
#define HW_RESET_USB				0x00000004UL
#define HW_RESET_SWITCH				0x00000008UL
#define HW_RESET_BB					0x00000010UL
#define HW_RESET_SW_P0				0x00000040UL
#define HW_RESET_SW_P1				0x00000080UL
#define HW_RESET_WIFI_PAUSE			0x00000100UL
#define HW_RESET_HNAT_PAUSE			0x00000200UL
#define HW_RESET_USB_PAUSE			0x00000400UL
#define HW_RESET_SWITCH_PAUSE		0x00000800UL
#define HW_RESET_BB_PAUSE			0x00001000UL
#define HW_RESET_RMII_P0			0x00010000UL
#define HW_RESET_RMII_P1			0x00020000UL
#define HW_RESET_DCM_BB2MAC			0x80000000UL    /* FPGA only */
#endif

#define REVISION_REG			0x20dc

#define EMAC_CFG				0x00
#define ETH_FLOWCTRL			0x40000000UL
#define CRS_SCHEME				0x02000000UL
#define BITRATE_SEL				0x00C00000UL
#define BITRATE_10M				0x00000000UL
#define BITRATE_100M			0x00400000UL
#define HALF_DUPLEX				0x00200000UL
#define BUF_OFS					0x0001F000UL
#define TXCLK_INV				0x00000008UL
#define RXCLK_INV				0x00000004UL
#define TXEN					0x00000002UL
#define RXEN					0x00000001UL

#define EMAC_RX_CFG				0x14
#define RX_USE_LU_VLAN			0x00002000UL
#define TIDMAP_SRC				0x00000060UL
#define TIDMAP_TOS				0x00000000UL
#define TIDMAP_VLAN				0x00000020UL
#define TIDMAP_HDESC			0x00000040UL
#define SECOND_VLAN				0x00000010UL
#define RM_VLAN					0x00000004UL

#define EMAC_TX_CFG				0x18
#define MIN_IFG					0x003F0000UL
#define PREAMBLE_SIZE			0x00007000UL
#define TX_DEBUG				0x00000800UL
#define PAYLOAD_FLOWCTRL		0x00000080UL
#define USE_DESC_VID			0x00000040UL
#define INS_VLAN				0x00000008UL
#define AUTO_RETRY				0x00000002UL
#define COLIISION_DROP16		0x00000001UL

#define EMAC_STATE				0x1C
#define E2C_DESC_CNT			0xFF000000UL
#define E2W_DESC_CNT			0x00FF0000UL
#define BUF_RUNOUT				0x00000020UL
#define NO_E2W_HWDESC			0x00000010UL
#define NO_E2W_DWDESC			0x00000008UL
#define LAN_BUSY				0x00000004UL
#define RXRDY					0x00000002UL
#define TXRDY					0x00000001UL

#define EMAC_TID_TRANS_TBL_0_3	0x20
#define EMAC_TID_TRANS_TBL_4_7	0x24

#define EMAC_VLAN_ID_TBL_0_1	0x28
#define	VLAN_ID_TBL_0			0x00000FFFUL
#define VLAN_ID_TBL_1			0x0FFF0000UL
#define EMAC_VLAN_ID_TBL_2_3	0x2C
#define	VLAN_ID_TBL_2			0x00000FFFUL
#define VLAN_ID_TBL_3			0x0FFF0000UL
#define EMAC_VLAN_ID_TBL_4_5	0x30
#define	VLAN_ID_TBL_4			0x00000FFFUL
#define VLAN_ID_TBL_5			0x0FFF0000UL
#define EMAC_VLAN_ID_TBL_6_7	0x34
#define	VLAN_ID_TBL_6			0x00000FFFUL
#define VLAN_ID_TBL_7			0x0FFF0000UL

#define EMAC_TXRX_MISC			0x38

#define EMAC_IFG				0xA0
#define IFG_UPPER_BOUND			0xFF000000UL
#define IFG_100M_CRS			0x00FF0000UL
#define IFG_10M_CRS				0x0000FF00UL
#define IFG_TXEN				0x000000FFUL

#define SW_PORT_CFG				0x300
#define PORT_EN					0x000000FFUL
#define TX_INS_VLAN				0x0000FF00UL

#define SW_CPU_PORT				0x304

#define SW_VLAN_GROUP0			0x308
#define SW_VLAN_GROUP1			0x30C
#define SW_VLAN_GROUP2			0x310
#define SW_VLAN_GROUP3			0x314
#define SW_VLAN_GROUP4			0x318
#define SW_VLAN_GROUP5			0x31C
#define SW_VLAN_GROUP6			0x320
#define SW_VLAN_GROUP7			0x324

#define SW_BSSID2VLAN0_1		0x328
#define SW_BSSID2VLAN2_3		0x32C
#define SW_BSSID2VLAN4_5		0x330
#define SW_BSSID2VLAN6_7		0x334

#define SW_PORT_VID0_1			0x338
#define SW_PORT_VID2_3			0x33C

#define SW_DESC_INIT_DONE		0x380
#define SW_DESC_BADDR			0x384
#define SW_ADD_DESC_REQ			0x388
#define SW_ADD_DESC_HT			0x38C
#define SW_ADD_DESC_AMOUNT		0x390

#define SW_DESC_CUR_HT			0x394
#define SW_DESC_INFO0			0x398
#define SW_DESC_INFO1			0x39C

#define SW_HWBUF_HDR_BADDR		0x3A0
#define SW_HWBUF_HDR_HT			0x3A4
#define SW_HWBUF_SIZE			0x3A8
#define SW_HWBUF_POOL_CTRL		0x3AC
#define SW_HWBUF_INIT_DONE		0x00000001UL
#define SW_P3DMA_ENABLE			0x00000002UL

#define SW_HWBUF_FC1			0x3B0
#define SW_HWBUF_FC2			0x3B4
#define SW_TIMER				0x3BC

#define SW_FC_CTRL				0x3C0
#define SW_FC_ENABLE			0x00000001UL
#define SW_FC_TOGGLE_RANGE		0xFFFF0000UL
#define SW_FC_PUBLIC			0x3C4
#define SW_FC_THD_REJ			0x0000FFFFUL
#define SW_FC_THD_DROP			0xFFFF0000UL
#define SW_FC_PRIVATE0_1		0x3C8
#define SW_FC_PRIVATE2_3		0x3CC

/* UMAC regs */
#define SWBL_BADDR					0x80C   /* based address of buffer header (data_frame_header) arrays (SW path pool) */
#define HWBL_BADDR					0x810   /* based address of buffer header (data_frame_header) arrays (fastpath pool) */
#define SRQ_BADDR					0x814   /* BA of TX done return queue (SW path) */
#define FRQ_BADDR					0x818   /* BA of TX done return queue (fastpath) */
#define HWB_HT						0x81C   /* HW link list tail (data_frame_header link list for HW fastpath Tx/Rx) */
#define SWB_HT						0x820   /* SW RX link list tail (data_frame_header link list for software Rx) */
#define BEACON_BADDR				0x824   /* beacon queue base address */

#define BUFF_POOL_CNTL				0x840   /* buffer pool control */
#define HWBUFF_CURR_HT				0x844   /* HW buffer pool current head and tail */
#define SWBUFF_CURR_HT				0x848   /* SW buffer pool current head and tail */
#define MAC_BSSID0					0x8a8   /* BSSID bitmap & BSSID[47:32] */
#define MAC_BSSID1					0x8ac   /* BSSID[31:0] */

#define STA_DS_TABLE_CFG			0x87C
#define STA_DS_TABLE_CFG_DONE		0x80000000UL
#define STA_TABLE_MAX_SEARCH		0x000001FFUL
#define DS_TABLE_MAX_SEACH			0x00003E00UL
#define STA_TABLE_HASH_MODE			0x00008000UL
#if defined(CONFIG_ARTHUR)
#define STA_TABLE_HASH_MASK			0x00FF0000UL
#else
#define STA_TABLE_HASH_MASK			0x000F0000UL
#endif
#define STA_DS_TABLE_CACHE_CFG		0x1F000000UL
#define STA_TABLE_CLR				0x20000000UL
#define DS_TABLE_CLR				0x40000000UL

#define MAC_BSSID_FUNC_MAP			0x8b0

/* RX Register set */
#define BA_LIFE_TIME_REG			0xB00
#define RX_BLOCK_SIZE				0xB04
#define SUB_BLK_SIZE				0xB08
#define LNK_BLK_SIZE				0xB0C   /* max dequeue MSDU size */

enum
{
#if defined(CONFIG_P0_AS_LAN)
    PHY_ETH1_IDX = 1,
    PORT_ETH1_IDX = 0,
    PORT_ETH0_IDX,
#else
#if defined(CONFIG_FPGA)
    PHY_ETH1_IDX = 0,
#else
    PHY_ETH1_IDX = 2,
#endif
    PORT_ETH0_IDX = 0,
    PORT_ETH1_IDX,
#endif
    PORT_HNAT_IDX,
    PORT_WLAN_IDX,
};

enum
{
    PHY_CAP_AN = (1 << 14),
    PHY_CAP_PAUSE = (1 << 10),
    PHY_CAP_100F = (1 << 8),
    PHY_CAP_100H = (1 << 7),
    PHY_CAP_10F = (1 << 6),
    PHY_CAP_10H = (1 << 5),

    PHY_AN = (1 << 6),
    PHY_ANC = (1 << 5),
    PHY_LINK = (1 << 4),
    PHY_100M = (1 << 1),
    PHY_FDX = (1 << 0),

    PHYR_ST_ANC = (1 << 5),
    PHYR_ST_AN = (1 << 3),
    PHYR_ST_LINK = (1 << 2),

    PHYR_CTL_AN = (1 << 12),
    PHYR_CTL_DUPLEX = (1 << 8),
    PHYR_CTL_SPEED = (1 << 13),
};

typedef struct
{
#if defined(BIG_ENDIAN)
    unsigned int offset:8;
    unsigned int next_index:11;
    unsigned int len:13;

    unsigned int ep:1;
    unsigned int reserved:2;
    unsigned int dptr:29;
#else
    unsigned int len:13;
    unsigned int next_index:11;
    unsigned int offset:8;

    unsigned int dptr:29;
    unsigned int reserved:2;
    unsigned int ep:1;
#endif
} buf_header;

typedef struct desc
{
    unsigned int w0;
    unsigned int w1;
} DES;

typedef struct bdesc
{
    void (*free) (void *);
    void *key;
} BDS;

typedef struct cmd
{
    DES *txd;
    DES *rxd;
    DES *hwd;
    BDS *txbd;
    unsigned long txbusy;
    unsigned long txnext;
    unsigned long rxbusy;
    unsigned long rxnext;
    unsigned long pktbuf;

    unsigned int rx_pkt;

    unsigned short txdnum;
    unsigned short rxdnum;
    unsigned short hwdnum;
    unsigned short bufnum;
} cmdev;

#ifdef CONFIG_ETH_CALIBRATION
void cm_phy_loopback(unsigned char p, unsigned int action);
int cm_cal_get(void);
void cm_cal_set(unsigned char idx, unsigned char txclk, unsigned char rxclk);
#endif
void cm_switch_port_attach(unsigned char idx, unsigned char ins_vlan,
                           unsigned char calibration);
int cm_probe();
void cm_reset();
void cm_wan_enable();
void cm_mdio_wr(unsigned short pa, unsigned short ra, unsigned short val);
unsigned short cm_mdio_rd(unsigned short pa, unsigned short ra);

#endif                          //CM_MAC_H
