/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file usb_loopback_dev.c
*   \brief USB LoopBack Device
*   \author Montage
*/
#ifdef CONFIG_USB_LOOPBACK_DEV
#define CONFIG_UDC
#include "udc.c"
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define STR_LANGID       0x0
#define STR_MANUFACTURER 0x1
#define STR_PRODUCT      0x2
#define STR_SERIAL       0x3
#define STR_CONFIG       0x4
#define STR_INTERFACE    0x5
#define STR_COUNT        0x6
#define LANGID "\011\004"       //LANGID:(English)United States
//P.S. encoding by UNICODE
#define MANUFACTURER_STRING "M\0o\0n\0t\0a\0g\0e\0,\0 \0I\0n\0c\0."
#define PRODUCT_NAME_STRING "L\0o\0o\0p\0B\0a\0c\0k"
#define SERIAL_STRING "3\0002\0008\0001"
#define CONFIG_STRING \
        "d\0e\0f\0a\0u\0l\0t\0 \0c\0o\0n\0f\0i\0g\0u\0r\0a\0t\0i\0o\0n"
#define INTERFACE_STRING "d\0e\0f\0a\0u\0l\0t\0 \0i\0n\0t\0e\0r\0f\0a\0c\0e"

#define LB_DES_NUM 4
#define LB_BUF_LEN 1024
#if (LB_DES_NUM > EP1_DES_NUM)
#error "ep1 descriptor number isn't enough"
#endif

#define BULK_IN_EPNO 1
#define BULK_OUT_EPNO 1
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct usb_device_descriptor boot_dev_desc = {
    .bLength = sizeof (struct usb_device_descriptor),
    .bDescriptorType = USB_DT_DEVICE,
    .bcdUSB = cpu_to_le16(USB_BCD_VERSION),
    .bDeviceClass = USB_CLASS_VENDOR_SPEC,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CTRL_MAXPKTSIZE,
    .idVendor = cpu_to_le16(0x0000),
    .idProduct = cpu_to_le16(0x3281),
    .bcdDevice = cpu_to_le16(0x0000),
    .iManufacturer = STR_MANUFACTURER,
    .iProduct = STR_PRODUCT,
    .iSerialNumber = STR_SERIAL,
    .bNumConfigurations = 1,
};

struct boot_total_cfg_desc
{
    struct usb_configuration_descriptor configuration_desc;
    struct usb_interface_descriptor interface_desc;     //control interface
    struct usb_endpoint_descriptor endpoint_desc[2];
} __attribute__ ((packed));
struct boot_total_cfg_desc boot_cfg_descs = {
    .configuration_desc = {
                           .bLength =
                           sizeof (struct usb_configuration_descriptor),
                           .bDescriptorType = USB_DT_CONFIGURATION,
                           .wTotalLength =
                           cpu_to_le16(sizeof (struct boot_total_cfg_desc)),
                           .bNumInterfaces = 1,
                           .bConfigurationValue = 1,
                           .iConfiguration = STR_CONFIG,
                           .bmAttributes = BMATTRIBUTE_RESERVED,
                           .bMaxPower = 0x1     //unit 2mA
                           },
    .interface_desc = {
                       .bLength = sizeof (struct usb_interface_descriptor),
                       .bDescriptorType = USB_DT_INTERFACE,
                       .bInterfaceNumber = 0,
                       .bAlternateSetting = 0,
                       .bNumEndpoints = 2,
                       .bInterfaceClass = USB_CLASS_VENDOR_SPEC,
                       .bInterfaceSubClass = 0x0,
                       .bInterfaceProtocol = 0xff,
                       .iInterface = STR_INTERFACE},
    .endpoint_desc = {
                      {
                       .bLength = sizeof (struct usb_endpoint_descriptor),
                       .bDescriptorType = USB_DT_ENDPOINT,
                       .bEndpointAddress = BULK_IN_EPNO | USB_DIR_IN,
                       .bmAttributes = USB_ENDPOINT_XFER_BULK,
                       .wMaxPacketSize = cpu_to_le16(HS_BULK_MAXPKTSIZE),
                       .bInterval = 0x0,
                       },
                      {
                       .bLength = sizeof (struct usb_endpoint_descriptor),
                       .bDescriptorType = USB_DT_ENDPOINT,
                       .bEndpointAddress = BULK_OUT_EPNO | USB_DIR_OUT,
                       .bmAttributes = USB_ENDPOINT_XFER_BULK,
                       .wMaxPacketSize = cpu_to_le16(HS_BULK_MAXPKTSIZE),
                       .bInterval = 0x0,
                       },
                      },
};

struct total_string_desc boot_str_descs[STR_COUNT] = {
    {
     .string_desc = {
                     .bLength = sizeof (struct usb_string_descriptor) +
                     sizeof (LANGID) - sizeof (""),
                     .bDescriptorType = USB_DT_STRING,
                     }
     ,
     .string = LANGID,
     }
    ,
    {
     .string_desc = {
                     .bLength = sizeof (struct usb_string_descriptor) +
                     sizeof (MANUFACTURER_STRING),
                     .bDescriptorType = USB_DT_STRING,
                     }
     ,
     .string = MANUFACTURER_STRING,
     }
    ,
    {
     .string_desc = {
                     .bLength = sizeof (struct usb_string_descriptor) +
                     sizeof (PRODUCT_NAME_STRING),
                     .bDescriptorType = USB_DT_STRING,
                     }
     ,
     .string = PRODUCT_NAME_STRING,
     }
    ,
    {
     .string_desc = {
                     .bLength = sizeof (struct usb_string_descriptor) +
                     sizeof (SERIAL_STRING),
                     .bDescriptorType = USB_DT_STRING,
                     }
     ,
     .string = SERIAL_STRING,
     }
    ,
    {
     .string_desc = {
                     .bLength = sizeof (struct usb_string_descriptor) +
                     sizeof (CONFIG_STRING),
                     .bDescriptorType = USB_DT_STRING,
                     }
     ,
     .string = CONFIG_STRING,
     }
    ,
    {
     .string_desc = {
                     .bLength = sizeof (struct usb_string_descriptor) +
                     sizeof (INTERFACE_STRING),
                     .bDescriptorType = USB_DT_STRING,
                     }
     ,
     .string = INTERFACE_STRING,
     }
    ,
};

static struct usb_qualifier_descriptor boot_dev_qualifier = {
    .bLength = sizeof (struct usb_qualifier_descriptor),
    .bDescriptorType = USB_DT_DEVICE_QUALIFIER,
    .bcdUSB = cpu_to_le16(USB_BCD_VERSION),
    .bDeviceClass = USB_CLASS_VENDOR_SPEC,
    .bMaxPacketSize0 = CTRL_MAXPKTSIZE,
    .bNumConfigurations = 1
};

struct udc_api boot_uapi;
struct enpt_list boot_eplist[2];
u8 ep1_lb_buf[LB_DES_NUM][LB_BUF_LEN]
    __attribute__ ((aligned(HAL_DCACHE_LINE_SIZE)));
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void udc_loopback_start(void);
void udc_loopback_tx_done(struct udc_cb_api *cbi);
void udc_loopback_rx_done(struct udc_cb_api *cbi);
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
/*!
 * function: boot_enpt_descs_setting
 *
 *  \brief
 *  \param
 *  \return
 */
void boot_enpt_descs_setting(int hs)
{
    u16 maxpktsize;
    if (hs)
        maxpktsize = cpu_to_le16(HS_BULK_MAXPKTSIZE);
    else
        maxpktsize = cpu_to_le16(FS_BULK_MAXPKTSIZE);
    memcpy(&boot_cfg_descs.endpoint_desc[0].wMaxPacketSize, &maxpktsize,
           sizeof (u16));
    memcpy(&boot_cfg_descs.endpoint_desc[1].wMaxPacketSize, &maxpktsize,
           sizeof (u16));
}

/*!
 * function: usb_lb_dev_init
 *
 *  \brief
 *  \param
 *  \return
 */
int usb_lb_dev_init(void)
{
    struct udc_api *uapi = &boot_uapi;

    uapi->start_rx = (void (*)(void *)) udc_loopback_start;
    uapi->dev_desc = &boot_dev_desc;
    uapi->cfg_desc = &boot_cfg_descs;
    uapi->cfg_len = sizeof (struct boot_total_cfg_desc);
    uapi->str_desc = boot_str_descs;
    uapi->dev_qualifier_desc = &boot_dev_qualifier;
    uapi->set_enpt_desc_by_spd = boot_enpt_descs_setting;

    boot_eplist[0].desc = &boot_cfg_descs.endpoint_desc[0];
    boot_eplist[1].desc = &boot_cfg_descs.endpoint_desc[1];
    boot_eplist[0].next = &boot_eplist[1];
    boot_eplist[1].next = NULL;
    uapi->enptlist = &boot_eplist[0];

    udc_init(uapi);
}

/*!
 * function: udc_loopback_rx_done
 *
 *  \brief
 *  \param
 *  \return
 */
void udc_loopback_rx_done(struct udc_cb_api *cbi)
{
    //usbdbg("rxlen=%d\n", cbi->rxbuflen);
    if (!cbi->status)
        udc_ep_tx(BULK_IN_EPNO, (void *) cbi->rxbuf, cbi->rxbuflen,
                  udc_loopback_tx_done, NULL);
}

/*!
 * function: udc_loopback_tx_done
 *
 *  \brief
 *  \param
 *  \return
 */
void udc_loopback_tx_done(struct udc_cb_api *cbi)
{
    //usbdbg("\nresubmit descriptor\n");
    if (!cbi->status)
        udc_ep_rx(BULK_OUT_EPNO, (void *) cbi->rxbuf, LB_BUF_LEN,
                  udc_loopback_rx_done, NULL);
}

/*!
 * function: udc_loopback_start
 *
 *  \brief
 *  \param
 *  \return
 */
void udc_loopback_start(void)
{
    int i;
    printf("%s:%d\n", __func__, __LINE__);
    for (i = 0; i < LB_DES_NUM; i++)
        udc_ep_rx(BULK_OUT_EPNO, (void *) (ep1_lb_buf[i]), LB_BUF_LEN,
                  udc_loopback_rx_done, NULL);
    printf("%s:%d\n", __func__, __LINE__);
}
#endif
