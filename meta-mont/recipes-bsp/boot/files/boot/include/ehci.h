/*!
*   \file ehci.h
*   \brief USB EHCI API
*   \author
*/
/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef USB_EHCI_H
#define USB_EHCI_H

#if !defined(CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS)
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS  1
#endif

/* (shifted) direction/type/recipient from the USB 2.0 spec, table 9.2 */
#define DeviceRequest \
    ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)

#define DeviceOutRequest \
    ((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE) << 8)

#define InterfaceRequest \
    ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

#define EndpointRequest \
    ((USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

#define EndpointOutRequest \
    ((USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_INTERFACE) << 8)

/*
 * Register Space.
 */
struct ehci_hccr
{
    unsigned int cr_capbase;
#define HC_LENGTH(p)        (((p) >> 0) & 0x00ff)
#define HC_VERSION(p)       (((p) >> 16) & 0xffff)
    unsigned int cr_hcsparams;
#define HCS_PPC(p)      ((p) & (1 << 4))
#define HCS_INDICATOR(p)    ((p) & (1 << 16))   /* Port indicators */
#define HCS_N_PORTS(p)      (((p) >> 0) & 0xf)
    unsigned int cr_hccparams;
//  unsigned char cr_hcsp_portrt[8];
} __attribute__ ((packed));

struct ehci_hcor
{
    unsigned int or_usbcmd;     //0x10 or 0x40
#define CMD_PARK    (1 << 11)   /* enable "park" */
#define CMD_PARK_CNT(c) (((c) >> 8) & 3)        /* how many transfers to park */
#define CMD_LRESET  (1 << 7)    /* partial reset */
#define CMD_IAAD    (1 << 6)    /* "doorbell" interrupt */
#define CMD_ASE     (1 << 5)    /* async schedule enable */
#define CMD_PSE     (1 << 4)    /* periodic schedule enable */
#define CMD_RESET   (1 << 1)    /* reset HC not bus */
#define CMD_RUN     (1 << 0)    /* start/stop HC */
    unsigned int or_usbsts;     //0x14 or 0x44
#define STD_ASS     (1 << 15)   /* Async Schedule Status */
#define STD_PSS     (1 << 14)   /* Periodic Schedule Status */
#define STS_HALT    (1 << 12)   /* Not running (any reason) */
    unsigned int or_usbintr;    //0x18 or 0x48
    unsigned int or_frindex;    //0x1c or 0x4c
    unsigned int or_ctrldssegment;      //0x20 or 0x50
    unsigned int or_periodiclistbase;   //0x24 or 0x54
    unsigned int or_asynclistaddr;      //0x28 or 0x58
    unsigned int _reserved_1;   //0x5C
    unsigned int burstsize;     //0x60
    unsigned int txfill;        //0x64
    unsigned int _reserved_2[7];        //0x68
    unsigned int or_portsc[CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS]; //0x30 or 0x84
#define PORT_RESET  (1 << 8)
#define PORT_ENABLE (1 << 2)
#define CONN_STS    (1 << 0)
    unsigned int or_padding[8]; //0x88
    unsigned int or_usbmode;    //0xa8
} __attribute__ ((packed));

/* Interface descriptor */
struct usb_linux_interface_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bInterfaceNumber;
    unsigned char bAlternateSetting;
    unsigned char bNumEndpoints;
    unsigned char bInterfaceClass;
    unsigned char bInterfaceSubClass;
    unsigned char bInterfaceProtocol;
    unsigned char iInterface;
} __attribute__ ((packed));

/* Configuration descriptor information.. */
struct usb_linux_config_descriptor
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned short wTotalLength;
    unsigned char bNumInterfaces;
    unsigned char bConfigurationValue;
    unsigned char iConfiguration;
    unsigned char bmAttributes;
    unsigned char MaxPower;
} __attribute__ ((packed));

#if 1                           //defined CONFIG_EHCI_DESC_BIG_ENDIAN
#define ehci_readl(x)       (*((volatile u32 *)(x)))
#define ehci_writel(a, b)   (*((volatile u32 *)(a)) = ((volatile u32)b))
#else
#define ehci_readl(x)       cpu_to_le32((*((volatile u32 *)(x))))
#define ehci_writel(a, b)   (*((volatile u32 *)(a)) = \
                    cpu_to_le32(((volatile u32)b)))
#endif

#if 0                           //defined CONFIG_EHCI_MMIO_BIG_ENDIAN
#define hc32_to_cpu(x)      be32_to_cpu((x))
#define cpu_to_hc32(x)      cpu_to_be32((x))
#else
#define hc32_to_cpu(x)      le32_to_cpu((x))
#define cpu_to_hc32(x)      cpu_to_le32((x))
#endif

#define EHCI_PS_WKOC_E      (1 << 22)   /* RW wake on over current */
#define EHCI_PS_WKDSCNNT_E  (1 << 21)   /* RW wake on disconnect */
#define EHCI_PS_WKCNNT_E    (1 << 20)   /* RW wake on connect */
#define EHCI_PS_PO      (1 << 13)       /* RW port owner */
#define EHCI_PS_PP      (1 << 12)       /* RW,RO port power */
#define EHCI_PS_LS      (3 << 10)       /* RO line status */
#define EHCI_PS_PR      (1 << 8)        /* RW port reset */
#define EHCI_PS_SUSP        (1 << 7)    /* RW suspend */
#define EHCI_PS_FPR     (1 << 6)        /* RW force port resume */
#define EHCI_PS_OCC     (1 << 5)        /* RWC over current change */
#define EHCI_PS_OCA     (1 << 4)        /* RO over current active */
#define EHCI_PS_PEC     (1 << 3)        /* RWC port enable change */
#define EHCI_PS_PE      (1 << 2)        /* RW port enable */
#define EHCI_PS_CSC     (1 << 1)        /* RWC connect status change */
#define EHCI_PS_CS      (1 << 0)        /* RO connect status */
#define EHCI_PS_CLEAR       (EHCI_PS_OCC | EHCI_PS_PEC | EHCI_PS_CSC)

#define EHCI_PS_IS_LOWSPEED(x)  (((x) & EHCI_PS_LS) == (1 << 10))

/*
 * Schedule Interface Space.
 *
 * IMPORTANT: Software must ensure that no interface data structure
 * reachable by the EHCI host controller spans a 4K page boundary!
 *
 * Periodic transfers (i.e. isochronous and interrupt transfers) are
 * not supported.
 */

/* Periodic Frame List */
struct periodic_frame_list
{
    unsigned int periodic_entry[1024];
} __attribute__ ((packed, aligned(4096)));

/* Queue Element Transfer Descriptor (qTD). */
struct qTD
{
    unsigned int qt_next;
#define QT_NEXT_TERMINATE   1
    unsigned int qt_altnext;
    unsigned int qt_token;
#define STAT_ACTIVE     (1<<7)
#define STAT_HALTED     (1<<6)
#define STAT_BUFERR     (1<<5)
#define STAT_BABLE      (1<<4)
#define STAT_XACTERR    (1<<3)
#define STAT_MISS       (1<<2)
#define STAT_SPLIT      (1<<1)
#define STAT_PING       (1<<1)
    unsigned int qt_buffer[5];
} __attribute__ ((packed, aligned(32)));

/* Queue Head (QH). */
struct QH
{
    unsigned int qh_link;
#define QH_LINK_TERMINATE   1
#define QH_LINK_TYPE_ITD    0
#define QH_LINK_TYPE_QH     2
#define QH_LINK_TYPE_SITD   4
#define QH_LINK_TYPE_FSTN   6
    unsigned int qh_endpt1;
    unsigned int qh_endpt2;
    unsigned int qh_curtd;
    struct qTD qh_overlay;
    /*
     * Add dummy fill value to make the size of this struct
     * aligned to 32 bytes
     */
    unsigned char fill[16];
} __attribute__ ((packed, aligned(32)));

/* Isochronous Transfer Descriptor (iTD). */
struct ehci_itd
{
    /* first part defined by EHCI spec */
    unsigned int hw_next;       /* see EHCI 3.3.1 */
    unsigned int hw_transaction[8];     /* see EHCI 3.3.2 */
#define EHCI_ISOC_ACTIVE        (1<<31) /* activate transfer this slot */
#define EHCI_ISOC_BUF_ERR       (1<<30) /* Data buffer error */
#define EHCI_ISOC_BABBLE        (1<<29) /* babble detected */
#define EHCI_ISOC_XACTERR       (1<<28) /* XactErr - transaction error */
#define EHCI_ITD_LENGTH(tok)    (((tok)>>16) & 0x0fff)
#define EHCI_ITD_IOC        (1 << 15)   /* interrupt on complete */

#define ITD_ACTIVE(ehci)    cpu_to_hc32(ehci, EHCI_ISOC_ACTIVE)

    unsigned int hw_bufp[7];    /* see EHCI 3.3.3 */
//  unsigned int            hw_bufp_hi [7]; /* Appendix B */

    /* the rest is HCD-private */
//  dma_addr_t      itd_dma;    /* for this itd */
//  union ehci_shadow   itd_next;   /* ptr to periodic q entry */
//
//  struct urb      *urb;
//  struct ehci_iso_stream  *stream;    /* endpoint's queue */
//  struct list_head    itd_list;   /* list of stream's itds */
//
//  /* any/all hw_transactions here may be used by that urb */
//  unsigned        frame;      /* where scheduled */
//  unsigned        pg;
//  unsigned        index[8];   /* in urb->iso_frame_desc */
} __attribute__ ((packed, aligned(32)));

/* Low level init functions */
int ehci_hcd_init(void);
int ehci_hcd_stop(void);
#endif                          /* USB_EHCI_H */
