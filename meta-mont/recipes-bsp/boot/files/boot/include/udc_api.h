/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file udc_api.h
*   \brief USB Device Controller Driver API
*   \author Montage
*/
#ifndef __UDC_API_H__
#define __UDC_API_H__
/*-----------------------------------------------------------------------------+
| Define                                                                       |
+-----------------------------------------------------------------------------*/
enum transfer_status
{
    UDC_TRS_CANCEL = -1,
    UDC_TRS_SUCCESS = 0,
};
/*-----------------------------------------------------------------------------+
| Structures                                                                   |
+-----------------------------------------------------------------------------*/
struct enpt_list
{
    void *desc;
    struct enpt_list *next;
};
struct udc_api
{
    void (*irqhandler) (void *arg);
    void (*enable) (void *dev);
    void (*disable) (void *dev);
    void (*setup) (void *usbcmd);
    void (*start_rx) (void *eth);
    void *upper_dev;
    void *dev;
    void *dev_desc;
    void *cfg_desc;
    void *str_desc;
    void *dev_qualifier_desc;
    unsigned int cfg_len;
    void (*set_enpt_desc_by_spd) (int hs);
    struct enpt_list *enptlist;
};
struct udc_cb_api
{
    unsigned int rxbuf;         //Rx buffer address
    unsigned int rxbuflen;      //Rx length
    unsigned int status;        //status; return zero if success, return negative value if fail
    void *cb;                   //callback function
    void *arg;                  //callback function's argument
};
/*-----------------------------------------------------------------------------+
| function prototype                                                           |
+-----------------------------------------------------------------------------*/
#endif                          /* __UDC_API_H__ */
