#ifndef __RF_H__
#define __RF_H__

#include <os_compat.h>

unsigned int rf_read(char reg);
void rf_write(char reg, int val);
void rf_update(char reg, int val, unsigned int mask);
void rf_set_40mhz_channel(int primary_ch_freq, int channel_type);
void rf_set_channel(int channel_freq, int channel_num);
int rf_init(void);
void rf_set_tx_gain(unsigned long gain);

//#define RFC_I2C_TEST 1

enum {
	RFCHIP_MT301_A1 = 1,	
	RFCHIP_IP301_B = 11,
	RFCHIP_IP301_E = 14,
	RFCHIP_IP301_G = 16,
	RFCHIP_MT301_A0 = 19,
	RFCHIP_IP301_L = 21,
	RFCHIP_IP301_GX2 = (16+0x0100),
	RFCHIP_PANTHER = 100,
};

#define APPLY_PANTHER_RFC_COMMON	// use this define to split declarations for non-panther and panther in rfc_comm.c/rfc_comm.h
//#define APPLY_LYNX_TXVGA_TABLE
//#define APPLY_LYNX_TX_LOOPBACK

#define IP301_NORMAL_MODE		0x3F954
#define MAX_RF_REG_NUM		10

#define RF_SPI_CLK			0x3
#define RF_SPI_BANK			0x1
#define RF_INTERNAL_LDO		true
	
struct rf_regs {
	u8 num;
	u32 val;
};

extern struct rf_regs rf_tbl[MAX_RF_REG_NUM];

struct rf_dev{
	unsigned short chip_ver; /* RF chip version */
	unsigned char internal_ldo;
	unsigned char spi_idx;
	unsigned char spi_clk;
	int (*init)(void);
	void (*set_channel)(int channel_freq, int channel_num);
	void (*set_40mhz_channel)(int primary_ch_freq, int channel_type);
	unsigned int (*read)(unsigned char reg);
	void (*write)(unsigned char reg, unsigned int val);
	void (*set_tx_gain)(unsigned int gain);
};

struct rf_init_tbl{
	int (*init)(void);
};

extern struct rf_dev *my_rf_dev;

#endif // __RF_H__

