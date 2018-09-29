#include <lib.h>
#include <clock.h>
#include <ip301.h>
#include <bb.h>
#include <rf.h>
#include <mac_ctrl.h>
#include <panther_rf.h>
#include <panther_dev.h>
#include <arch/chip.h>
#include <netprot.h>
#include <otp.h>
#include <pmu.h>
#include <rfc_panther.h>
#include "performance.h"

#define DEVICE_ID_WIFIMAC 2531
void pmu_reset_wifi_mac(void)
{
    unsigned long reset_device_ids[] = {  DEVICE_ID_WIFIMAC, 0 };

    pmu_reset_devices(reset_device_ids);
}

#ifndef CONFIG_MONTE_CARLO
extern unsigned char txvga_gain[14];
extern unsigned char txvga_gain_save[14];
extern unsigned char fofs;
extern unsigned char fofs_save;
extern unsigned char bg_txp_diff;
extern unsigned char ng_txp_diff;
extern unsigned char bg_txp_gap;
extern char fem_product_id[8];
extern int fem_en;
/* get tx config 1.otp 2.flash 3.default */
extern bootvar bootvars;
int decimal_vga_converter(unsigned char c)
{
    int val;
    switch(c)
    {
        case 0xa:
            val = 44;
            break;
        case 0xb:
            val = 45;
            break;
        case 0xc:
            val = 46;
            break;
        case 0xd:
            val = 47;
            break;
        case 0xe:
            val = 48;
            break;
        case 0xf:
            val = 49;
            break;
        case 0x0:
            val = 50;
            break;
        case 0x1:
            val = 51;
            break;
        case 0x2:
            val = 52;
            break;
        case 0x3:
            val = 53;
            break;
        case 0x4:
            val = 54;
            break;
        case 0x5:
            val = 55;
            break;
        case 0x6:
            val = 56;
            break;
        default:
            val = 50;
            break;
    }
    return val;
}
unsigned char string_vga_converter(char c)
{
    unsigned char val;
    switch(c)
    {
        case 'a':
            val = 0xa;
            break;
        case 'b':
            val = 0xb;
            break;
        case 'c':
            val = 0xc;
            break;
        case 'd':
            val = 0xd;
            break;
        case 'e':
            val = 0xe;
            break;
        case 'f':
            val = 0xf;
            break;
        case '0':
            val = 0x0;
            break;
        case '1':
            val = 0x1;
            break;
        case '2':
            val = 0x2;
            break;
        case '3':
            val = 0x3;
            break;
        case '4':
            val = 0x4;
            break;
        case '5':
            val = 0x5;
            break;
        case '6':
            val = 0x6;
            break;
        default:
            val = 0x0;
            break;
    }
    return val;
}
unsigned char default_mac_addr[6];
void panther_get_tx_config(void)
{
    char c;
    unsigned char val;
    unsigned char buf[10];
    int idx, size, otp_ready = 0;

    if(0 == otp_load())
        otp_ready = 1;
    /* tx power level */
    if(otp_ready && (OTP_LEN_TXVGA == (size = otp_read(buf, OTP_TXVGA))))
    {
        for(idx = 0; idx < OTP_LEN_TXVGA; idx++)
        {
            txvga_gain[idx*2] = buf[idx] >> 4;
            txvga_gain_save[idx*2] =  txvga_gain[idx*2];

            txvga_gain[idx*2+1] = buf[idx] & 0xf;
            txvga_gain_save[idx*2+1] =  txvga_gain[idx*2+1];
        }
    }
    else if(bootvars.txvga)
    {
        for(idx = 0; idx < sizeof(txvga_gain); idx++)
        {
            c = bootvars.txvga[idx];
            val = string_vga_converter(c);
            txvga_gain[idx] = val & 0xf;
            txvga_gain_save[idx] =  txvga_gain[idx];
        }
    }
    else
    {
        memset((void *) txvga_gain, 0, sizeof(txvga_gain));
        memcpy(txvga_gain_save, txvga_gain, sizeof(txvga_gain_save));
    }

    /* tx frequency offset */
    if(otp_ready && (OTP_LEN_FOFS == (size = otp_read(buf, OTP_FOFS))))
    {
        fofs = buf[0];
        fofs_save = fofs;
    }
    else if(bootvars.freq_ofs)
    {
        fofs = bootvars.freq_ofs;
        fofs_save = fofs;
    }
    else
    {
        fofs = 16;
        fofs_save = fofs;
    }

    /* tx power difference */
    if(otp_ready && (OTP_LEN_TXP_DIFF == (size = otp_read(buf, OTP_TXP_DIFF))))
    {
        bg_txp_diff = buf[0] >> 4;
        ng_txp_diff = buf[0] & 0xf;
        bg_txp_gap = buf[1];
    }
    else if(bootvars.txp_diff)
    {
        c = bootvars.txp_diff[0];
        if(c >= '0' && c <= '6')
            bg_txp_diff = c - '0';
        else
            bg_txp_diff = 4;

        c = bootvars.txp_diff[1];
        if(c >= '0' && c <= '6')
            ng_txp_diff = c - '0';
         else
            ng_txp_diff = 0;

        c = bootvars.txp_diff[2];
        if(c == '3' || c == '6')
            bg_txp_gap = c - '0';
        else
            bg_txp_gap = 6;
    }
    else
    {
        bg_txp_diff = 4;
        ng_txp_diff = 0;
        bg_txp_gap = 6;
    }

    if(bootvars.fem_product_id)
        sscanf(bootvars.fem_product_id, "%s", fem_product_id);

    fem_en = bootvars.fem_en;

    if(otp_ready && (OTP_LEN_MAC == (size = otp_read(buf, OTP_MAC_ADDR))))
    {
        memcpy(default_mac_addr, buf, OTP_LEN_MAC);
    }
    else if(bootvars.mac0)
    {
        memcpy(default_mac_addr, bootvars.mac0, OTP_LEN_MAC);
    }
    else
    {
        default_mac_addr[0] = 0x00;
        default_mac_addr[1] = 0x12;
        default_mac_addr[2] = 0x34;
        default_mac_addr[3] = 0x56;
        default_mac_addr[4] = 0x78;
        default_mac_addr[5] = 0x90;
    }

/*
    for(idx=0;idx<14;idx++)
        printf("ch%d %02x\n", idx+1, txvga_gain[idx]);
    printf("fofs %d\n", fofs);
    printf("txp_diff %d %d %d\n", bg_txp_diff, ng_txp_diff, bg_txp_gap);
*/
}
#endif
extern int wifi_init(void);
void dratini_start(void)
{
    rf_init();
    bb_init();

#ifndef CONFIG_MONTE_CARLO
    panther_get_tx_config();

    panther_rfc_process();

    pmu_reset_wifi_mac();   // reset mac before mac_init
#endif
    wifi_init();

    if(fem_en)
        panther_fem_init(fem_en);
    else
        panther_without_fem_init();
}
