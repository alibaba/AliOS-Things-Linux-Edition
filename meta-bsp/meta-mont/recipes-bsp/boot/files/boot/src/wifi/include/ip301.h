/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file ip301.h
*   \brief 
*   \author Montage
*/

#ifndef __IP301_H__
#define __IP301_H__
//#include <mt_types.h>
#include <os_compat.h>
//#define CONFIG_MONTE_CARLO 1
void mt301_set_40mhz_channel(int channel_num, int mode);
void mt301_set_tx_power(unsigned int level);
unsigned int mt301_get_tx_power(void);
void ip301_set_iqcal(u8 loop_type);
void ip301_set_iqcal_vga(u8 loop_type, u32 rxvga, u32 txvga);
void mt301_set_iqcal_vga(u8 loop_type, u32 rxvga, u32 txvga);
int mt301_init(void);
int mt301_recover(void);

/* record the rf ctrl interface info */
enum {
	RF_CTRL_UNKNOW,
	RF_CTRL_W6,
	RF_CTRL_DIRECT,
	RF_CTRL_I2C,
};

#define RFREG_READ_DIRECT(x)		RFREG(x<<2)
#define RFREG_WRITE_DIRECT(x,val)	(RFREG(x<<2) = (u32)(val))
#define RFREG_UPDATE_DIRECT(x,val,mask) do {           \
    u32 newval = RFREG(x<<2);     \
    newval = (( newval & ~(mask) ) | ( (val) & (mask) ));\
    RFREG(x<<2) = newval;      \
} while(0)

void rf_interface_check(void);
unsigned int mt301_reg_read_w6(unsigned char reg);
void mt301_reg_write_w6(unsigned char reg, unsigned int data);
#endif

