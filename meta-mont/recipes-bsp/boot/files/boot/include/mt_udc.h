/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file mt_udc.h
*   \brief Montage USB Device Controller Descriptor Definition
*   \author Montage
*/
#ifndef __MT_UDC_H__
#define __MT_UDC_H__

#define USB_MAX_EP_NUM      (1<<2)      //should be powers of 2

#define mt_writel(val32, addr)  writel(val32, addr)
/*-----------------------------------------------------------------------------+
| Macros                                                                       |
+-----------------------------------------------------------------------------*/
#define MT_EP_OUT               0
#define MT_EP_IN                1

/* ep0 transfer state */
#define WAIT_FOR_SETUP          0
#define DATA_STATE_XMIT         1
#define DATA_STATE_NEED_ZLP     2
#define WAIT_FOR_OUT_STATUS     3
#define DATA_STATE_RECV         4

/* qh setup offset */
#define QH_MULT_POS             30
#define QH_ZLT_POS              29
#define QH_MAX_PKT_LEN_POS      16
#define QH_INTR_ON_SETUP_POS    15

/* Device Controller Capability Parameter register */
#define DCCPARAMS_DC                0x00000080
#define DCCPARAMS_DEN_MASK          0x0000001f

/* Frame Index Register Bit Masks */
#define USB_FRINDEX_MASKS           0x3fff

/* USB CMD  Register Bit Masks */
#define USB_CMD_RUN_STOP                     0x00000001
#define USB_CMD_CTRL_RESET                   0x00000002
#define USB_CMD_PERIODIC_SCHEDULE_EN         0x00000010
#define USB_CMD_ASYNC_SCHEDULE_EN            0x00000020
#define USB_CMD_INT_AA_DOORBELL              0x00000040
#define USB_CMD_ASP                          0x00000300
#define USB_CMD_ASYNC_SCH_PARK_EN            0x00000800
#define USB_CMD_SUTW                         0x00002000
#define USB_CMD_ATDTW                        0x00004000
#define USB_CMD_ITC                          0x00FF0000

/* bit 15,3,2 are frame list size */
#define USB_CMD_FRAME_SIZE_1024              0x00000000
#define USB_CMD_FRAME_SIZE_512               0x00000004
#define USB_CMD_FRAME_SIZE_256               0x00000008
#define USB_CMD_FRAME_SIZE_128               0x0000000C
#define USB_CMD_FRAME_SIZE_64                0x00008000
#define USB_CMD_FRAME_SIZE_32                0x00008004
#define USB_CMD_FRAME_SIZE_16                0x00008008
#define USB_CMD_FRAME_SIZE_8                 0x0000800C

/* bit 9-8 are async schedule park mode count */
#define USB_CMD_ASP_00                       0x00000000
#define USB_CMD_ASP_01                       0x00000100
#define USB_CMD_ASP_10                       0x00000200
#define USB_CMD_ASP_11                       0x00000300
#define USB_CMD_ASP_BIT_POS                  8

/* bit 23-16 are interrupt threshold control */
#define USB_CMD_ITC_NO_THRESHOLD             0x00000000
#define USB_CMD_ITC_1_MICRO_FRM              0x00010000
#define USB_CMD_ITC_2_MICRO_FRM              0x00020000
#define USB_CMD_ITC_4_MICRO_FRM              0x00040000
#define USB_CMD_ITC_8_MICRO_FRM              0x00080000
#define USB_CMD_ITC_16_MICRO_FRM             0x00100000
#define USB_CMD_ITC_32_MICRO_FRM             0x00200000
#define USB_CMD_ITC_64_MICRO_FRM             0x00400000
#define USB_CMD_ITC_BIT_POS                  16

/* USB STS Register Bit Masks */
#define USB_STS_INT                          0x00000001
#define USB_STS_ERR                          0x00000002
#define USB_STS_PORT_CHANGE                  0x00000004
#define USB_STS_FRM_LST_ROLL                 0x00000008
#define USB_STS_SYS_ERR                      0x00000010
#define USB_STS_IAA                          0x00000020
#define USB_STS_RESET                        0x00000040
#define USB_STS_SOF                          0x00000080
#define USB_STS_SUSPEND                      0x00000100
#define USB_STS_HC_HALTED                    0x00001000
#define USB_STS_RCL                          0x00002000
#define USB_STS_PERIODIC_SCHEDULE            0x00004000
#define USB_STS_ASYNC_SCHEDULE               0x00008000

/* USB INTR Register Bit Masks */
#define USB_INTR_INT_EN                      0x00000001
#define USB_INTR_ERR_INT_EN                  0x00000002
#define USB_INTR_PTC_DETECT_EN               0x00000004
#define USB_INTR_FRM_LST_ROLL_EN             0x00000008
#define USB_INTR_SYS_ERR_EN                  0x00000010
#define USB_INTR_ASYN_ADV_EN                 0x00000020
#define USB_INTR_RESET_EN                    0x00000040
#define USB_INTR_SOF_EN                      0x00000080
#define USB_INTR_DEVICE_SUSPEND              0x00000100

/* Device Address bit masks */
#define USB_DEVICE_ADDRESS_MASK              0xFE000000
#define USB_DEVICE_ADDRESS_BIT_POS           25

/* endpoint list address bit masks */
#define USB_EP_LIST_ADDRESS_MASK              0xfffff800

/* PORTSCX  Register Bit Masks */
#define PORTSCX_CURRENT_CONNECT_STATUS       0x00000001
#define PORTSCX_CONNECT_STATUS_CHANGE        0x00000002
#define PORTSCX_PORT_ENABLE                  0x00000004
#define PORTSCX_PORT_EN_DIS_CHANGE           0x00000008
#define PORTSCX_OVER_CURRENT_ACT             0x00000010
#define PORTSCX_OVER_CURRENT_CHG             0x00000020
#define PORTSCX_PORT_FORCE_RESUME            0x00000040
#define PORTSCX_PORT_SUSPEND                 0x00000080
#define PORTSCX_PORT_RESET                   0x00000100
#define PORTSCX_LINE_STATUS_BITS             0x00000C00
#define PORTSCX_PORT_POWER                   0x00001000
#define PORTSCX_PORT_INDICTOR_CTRL           0x0000C000
#define PORTSCX_PORT_TEST_CTRL               0x000F0000
#define PORTSCX_WAKE_ON_CONNECT_EN           0x00100000
#define PORTSCX_WAKE_ON_CONNECT_DIS          0x00200000
#define PORTSCX_WAKE_ON_OVER_CURRENT         0x00400000
#define PORTSCX_PHY_LOW_POWER_SPD            0x00800000
#define PORTSCX_PORT_FORCE_FULL_SPEED        0x01000000
#define PORTSCX_PORT_SPEED_MASK              0x0C000000
#define PORTSCX_PORT_WIDTH                   0x10000000
#define PORTSCX_PHY_TYPE_SEL                 0xC0000000

/* bit 11-10 are line status */
#define PORTSCX_LINE_STATUS_SE0              0x00000000
#define PORTSCX_LINE_STATUS_JSTATE           0x00000400
#define PORTSCX_LINE_STATUS_KSTATE           0x00000800
#define PORTSCX_LINE_STATUS_UNDEF            0x00000C00
#define PORTSCX_LINE_STATUS_BIT_POS          10

/* bit 15-14 are port indicator control */
#define PORTSCX_PIC_OFF                      0x00000000
#define PORTSCX_PIC_AMBER                    0x00004000
#define PORTSCX_PIC_GREEN                    0x00008000
#define PORTSCX_PIC_UNDEF                    0x0000C000
#define PORTSCX_PIC_BIT_POS                  14

/* bit 19-16 are port test control */
#define PORTSCX_PTC_DISABLE                  0x00000000
#define PORTSCX_PTC_JSTATE                   0x00010000
#define PORTSCX_PTC_KSTATE                   0x00020000
#define PORTSCX_PTC_SEQNAK                   0x00030000
#define PORTSCX_PTC_PACKET                   0x00040000
#define PORTSCX_PTC_FORCE_EN                 0x00050000
#define PORTSCX_PTC_BIT_POS                  16

/* bit 27-26 are port speed */
#define PORTSCX_PORT_SPEED_FULL              0x00000000
#define PORTSCX_PORT_SPEED_LOW               0x04000000
#define PORTSCX_PORT_SPEED_HIGH              0x08000000
#define PORTSCX_PORT_SPEED_UNDEF             0x0C000000
#define PORTSCX_SPEED_BIT_POS                26

/* bit 28 is parallel transceiver width for UTMI interface */
#define PORTSCX_PTW                          0x10000000
#define PORTSCX_PTW_8BIT                     0x00000000
#define PORTSCX_PTW_16BIT                    0x10000000

/* bit 31-30 are port transceiver select */
#define PORTSCX_PTS_UTMI                     0x00000000
#define PORTSCX_PTS_ULPI                     0x80000000
#define PORTSCX_PTS_FSLS                     0xC0000000
#define PORTSCX_PTS_BIT_POS                  30

/* otgsc Register Bit Masks */
#define OTGSC_CTRL_VUSB_DISCHARGE            0x00000001
#define OTGSC_CTRL_VUSB_CHARGE               0x00000002
#define OTGSC_CTRL_OTG_TERM                  0x00000008
#define OTGSC_CTRL_DATA_PULSING              0x00000010
#define OTGSC_STS_USB_ID                     0x00000100
#define OTGSC_STS_A_VBUS_VALID               0x00000200
#define OTGSC_STS_A_SESSION_VALID            0x00000400
#define OTGSC_STS_B_SESSION_VALID            0x00000800
#define OTGSC_STS_B_SESSION_END              0x00001000
#define OTGSC_STS_1MS_TOGGLE                 0x00002000
#define OTGSC_STS_DATA_PULSING               0x00004000
#define OTGSC_INTSTS_USB_ID                  0x00010000
#define OTGSC_INTSTS_A_VBUS_VALID            0x00020000
#define OTGSC_INTSTS_A_SESSION_VALID         0x00040000
#define OTGSC_INTSTS_B_SESSION_VALID         0x00080000
#define OTGSC_INTSTS_B_SESSION_END           0x00100000
#define OTGSC_INTSTS_1MS                     0x00200000
#define OTGSC_INTSTS_DATA_PULSING            0x00400000
#define OTGSC_INTR_USB_ID                    0x01000000
#define OTGSC_INTR_A_VBUS_VALID              0x02000000
#define OTGSC_INTR_A_SESSION_VALID           0x04000000
#define OTGSC_INTR_B_SESSION_VALID           0x08000000
#define OTGSC_INTR_B_SESSION_END             0x10000000
#define OTGSC_INTR_1MS_TIMER                 0x20000000
#define OTGSC_INTR_DATA_PULSING              0x40000000

/* USB MODE Register Bit Masks */
#define USB_MODE_CTRL_MODE_IDLE              0x00000000
#define USB_MODE_CTRL_MODE_DEVICE            0x00000002
#define USB_MODE_CTRL_MODE_HOST              0x00000003
#define USB_MODE_CTRL_MODE_RSV               0x00000001
#define USB_MODE_SETUP_LOCK_OFF              0x00000008
#define USB_MODE_STREAM_DISABLE              0x00000010

/* Endpoint Setup Status bit masks */
#define EP_SETUP_STATUS_EP0                  0x00000001

/* ENDPOINTCTRLx  Register Bit Masks */
#define EPCTRL_TX_ENABLE                     0x00800000
#define EPCTRL_TX_DATA_TOGGLE_RST            0x00400000 /* Not EP0 */
#define EPCTRL_TX_DATA_TOGGLE_INH            0x00200000 /* Not EP0 */
#define EPCTRL_TX_TYPE                       0x000C0000
#define EPCTRL_TX_DATA_SOURCE                0x00020000 /* Not EP0 */
#define EPCTRL_TX_EP_STALL                   0x00010000
#define EPCTRL_RX_ENABLE                     0x00000080
#define EPCTRL_RX_DATA_TOGGLE_RST            0x00000040 /* Not EP0 */
#define EPCTRL_RX_DATA_TOGGLE_INH            0x00000020 /* Not EP0 */
#define EPCTRL_RX_TYPE                       0x0000000C
#define EPCTRL_RX_DATA_SINK                  0x00000002 /* Not EP0 */
#define EPCTRL_RX_EP_STALL                   0x00000001

/* bit 19-18 and 3-2 are endpoint type */
#define EPCTRL_EP_TYPE_CONTROL               0
#define EPCTRL_EP_TYPE_ISO                   1
#define EPCTRL_EP_TYPE_BULK                  2
#define EPCTRL_EP_TYPE_INTERRUPT             3
#define EPCTRL_TX_EP_TYPE_SHIFT              18
#define EPCTRL_RX_EP_TYPE_SHIFT              2

/* register offset */
#define USBCMD         0x40
#define USBSTS         0x44
#define USBINTR        0x48
#define FINDEX         0x4C
#define DEVADDR        0x54
#define LISTADDR       0x58
#define ENDPTNAK       0x78
#define ENDPTNAKEN     0x7C
#define PORTSC         0x84
#define USBMODE        0xa8
#define ENDPTSETUPSTAT 0xac
#define ENDPTPRIME     0xb0
#define ENDPTFLUSH     0xb4
#define ENDPTSTATUS    0xb8
#define ENDPTCOMPLETE  0xbc
#define ENDPTCTRL0     0xc0

/* Endpoint Queue Head Bit Masks */
#define EP_QUEUE_HEAD_MULT_POS               30
#define EP_QUEUE_HEAD_ZLT_SEL                0x20000000
#define EP_QUEUE_HEAD_MAX_PKT_LEN_POS        16
#define EP_QUEUE_HEAD_MAX_PKT_LEN(ep_info)   (((ep_info)>>16)&0x07ff)
#define EP_QUEUE_HEAD_IOS                    0x00008000
#define EP_QUEUE_HEAD_NEXT_TERMINATE         0x00000001
#define EP_QUEUE_HEAD_IOC                    0x00008000
#define EP_QUEUE_HEAD_MULTO                  0x00000C00
#define EP_QUEUE_HEAD_STATUS_HALT            0x00000040
#define EP_QUEUE_HEAD_STATUS_ACTIVE          0x00000080
#define EP_QUEUE_CURRENT_OFFSET_MASK         0x00000FFF
#define EP_QUEUE_HEAD_NEXT_POINTER_MASK      0xFFFFFFE0
#define EP_QUEUE_FRINDEX_MASK                0x000007FF
#define EP_MAX_LENGTH_TRANSFER               0x4000

/* Endpoint Transfer Descriptor bit Masks */
#define DTD_NEXT_TERMINATE                   0x00000001
#define DTD_IOC                              0x00008000
#define DTD_STATUS_ACTIVE                    0x00000080
#define DTD_STATUS_HALTED                    0x00000040
#define DTD_STATUS_DATA_BUFF_ERR             0x00000020
#define DTD_STATUS_TRANSACTION_ERR           0x00000008
#define DTD_RESERVED_FIELDS                  0x80007300
#define DTD_ADDR_MASK                        0xFFFFFFE0
#define DTD_PACKET_SIZE                      0x7FFF0000
#define DTD_LENGTH_BIT_POS                   16
#define DTD_ERROR_MASK                       (DTD_STATUS_HALTED | \
                                              DTD_STATUS_DATA_BUFF_ERR | \
                                              DTD_STATUS_TRANSACTION_ERR)

#define MT_QH_SIZE      0x30
#define MT_TD_SIZE      0x20

#define MT_QH_ALIGNMENT 0x40
#define MT_TD_ALIGNMENT 0x20    //next pointer limit
/*-----------------------------------------------------------------------------+
| Typedef                                                                      |
+-----------------------------------------------------------------------------*/
#include <mt_types.h>           //for using u8, etc..
/*-----------------------------------------------------------------------------+
| Structures                                                                   |
+-----------------------------------------------------------------------------*/
/* Montage USB device controller registers (Little Endian) */
struct mt_dcr
{
    /* Capability register */
    /* 0x00 */ u8 res1[0x24];
    /* 0x24 */ u32 dccparams;
    /* Device Controller Capability Parameters */
    /* 0x28 */ u8 res2[0x18];

    /* Operation register */
    /* 0x40 */ u32 usbcmd;
    /* USB Command Register */
    /* 0x44 */ u32 usbsts;
    /* USB Status Register */
    /* 0x48 */ u32 usbintr;
    /* USB Interrupt Enable Register */
    /* 0x4c */ u32 frindex;
    /* Frame Index Register */
    /* 0x50 */ u8 res3[0x4];
    /* 0x54 */ u32 devaddr;
    /* Device Address */
    /* 0x58 */ u32 endptlistaddr;
    /* Endpoint List Address Register */
    /* 0x5c */ u8 res4[0x4];
    /* 0x60 */ u32 burstsize;
    /* Master Interface Data Burst Size Register */
    /* 0x64 */ u32 txttfilltuning;
    /* Transmit FIFO Tuning Controls Register */
    /* 0x68 */ u8 res5[0x10];
    /* 0x78 */ u32 endptnak;
    /* Endpoint NAK Register */
    /* 0x7c */ u32 endptnaken;
    /* Endpoint NAK Enable Register */
    /* 0x80 */ u32 configflag;
    /* Configure Flag Register */
    /* 0x84 */ u32 portsc;
    /* Port Status and Control Register */
    /* 0x88 */ u8 res6[0x1c];
    /* 0xa4 */ u32 otgsc;
    /* On-The-Go Status and Control */
    /* 0xa8 */ u32 usbmode;
    /* USB Mode Register */
    /* 0xac */ u32 endptsetupstat;
    /* Endpoint Setup Status Register */
    /* 0xb0 */ u32 endptprime;
    /* Endpoint Initialization Register */
    /* 0xb4 */ u32 endptflush;
    /* Endpoint Flush Register */
    /* 0xb8 */ u32 endptstatus;
    /* Endpoint Status Register */
    /* 0xbc */ u32 endptcomplete;
    /* Endpoint Complete Register */
    /* 0xc0 */ u32 endptctrl[USB_MAX_EP_NUM];
    /* Endpoint Control Registers */
};

struct udc_qh
{
    /* 0x00 */ u32 setup;
    /* Mult(31-30), Zlt(29), Max Pkt len and IOS(15) */
    /* 0x04 */ u32 curr;
    /* Current dTD Pointer(31-5) */
    /* 0x08 */ u32 next;
    /* overlay td */
    /* 0x0c */ u32 token;
    /* 0x10 */ u32 buffer[5];
    /* 0x24 */ u32 reserved;
    /* 0x28 */ u8 setup_buffer[8];
    /* Setup data 8 bytes */
} __attribute__ ((packed, aligned(MT_QH_ALIGNMENT)));

struct udc_td
{
    /* 0x00 */ u32 next;
    /* 0x04 */ u32 token;
    /* 0x08 */ u32 buffer[5];
} __attribute__ ((packed, aligned(MT_TD_ALIGNMENT)));

/*-----------------------------------------------------------------------------+
| ubuf definition                                                              |
+-----------------------------------------------------------------------------*/
struct ubuf
{
    /* USB controller access */
    struct udc_td des;
    /* software part *//* struct udc_cb_api */
    unsigned int rxbuf;         //Rx buffer address
    unsigned int rxbuflen;      //Rx length
    unsigned int status;        //status; return zero if success, return negative value if fail
    void *cb;                   //callback function
    void *arg;                  //callback function's argument
    /* about queue */
    struct ubuf *next;
    unsigned int flag;
} __attribute__ ((aligned(MT_TD_ALIGNMENT)));
// this structure include TD, alignment is as same as TD

typedef struct udc_queue
{
    struct ubuf *head, *tail;
    int count;
    int max;
} udc_queue;

struct ubuf_cfg
{
    udc_queue queue;
    unsigned int size;
    unsigned int num;
#ifdef __ECOS
    void *mutex;
#else
    unsigned int flag;
#endif
    void *base;
};

struct mt_td_ref
{
    u32 des_num;
    u32 des_used;
    struct udc_td *td_head;
    struct udc_td *td_tail;
};
#endif                          /* __MT_UDC_H__ */
