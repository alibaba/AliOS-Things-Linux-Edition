/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file bb.h
*   \brief  API definition for BB
*   \author Montage
*/

#ifndef __BB_H__
#define __BB_H__

#include <os_compat.h>
//#include <mt_types.h>

/* bb rx counter ctrl */
enum {
	BB_RX_CNT_RESET,
	BB_RX_CNT_ENABLE,
	BB_RX_CNT_DISABLE,
};

#define	BB_RX_CHANNEL_CNT_ADDR	0x20
#define	BB_RX_BUSY_CNT_ADDR		0x23

void bb_init(void);
void bb_reset(void);
void rf_bb_check(void);
u8 bb_register_read(int group, u32 bb_reg);
void bb_register_write(int  group, u32 bb_reg, u8 value);
u8 bb_set_cca_level(u8 level);
#ifndef CONFIG_MONTE_CARLO
void bb_set_tx_gain(u8 txpwr);
#else
void bb_set_tx_gain(u8 level);
#endif
u8 bb_get_tx_gain(void);
u8 bb_rssi_decode(u8 val, int rssi_offset, u8 *lna_gain_tble);

void bb_set_40mhz_mode(int channel_type, int freq);
void bb_set_20mhz_mode(int freq);
/* for debug */
void bb_dump_rx_auto_rfc(void);
void bb_dump_adc_dc(void);
void bb_enable_dump_adc_dc(void);

void bb_control_init(void);
void bb_txpe_off(void);
void bb_rxpe_off(void);
void bb_rxpe_on(void);
void bb_txpe_on(void);

void bb_rx_counter_ctrl(u32 op);
u32 bb_rx_counter_read(u8 addr);
u8 bb_read_noise_floor(void);

#define MAX_RFC_REG_NUM		50

struct bb_regs {
	u8 num;
	u8 val;
};

struct bb_regs_ahb {
        int group;
        u8 num;
        u8 val;
};
struct bb_dev {
	void (*set_20mhz_mode)(void);
	void (*set_40mhz_mode)(int channel_type);
};

extern struct bb_regs rfc_tbl[MAX_RFC_REG_NUM];
extern struct bb_dev *my_bb_dev;
#endif
