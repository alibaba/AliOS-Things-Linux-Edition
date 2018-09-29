#ifndef __PANTHER_RF_H__
#define __PANTHER_RF_H__

struct rfc_cal_reg{
	unsigned char is_failed;
	unsigned char balancer_nm;				//bb20;
	unsigned char txlo_msb;					//bb21 in panther
	unsigned char tx_a11;  				        //bb21;
	unsigned char tx_a12; 					//bb22;
	unsigned char tx_a21; 					//bb23;
	unsigned char tx_a22; 					//bb24;
	unsigned char tx_dc_i; 					//bb25;
	unsigned char rx_a21;  					//bb28;
	unsigned char rx_a22; 					//bb29;
	unsigned char rx_dc_i; 					//bb2a;
	unsigned char tx_dc_q; 					//bb30;
	unsigned char rx_dc_q; 					//bb31;
	unsigned char filter_switch; 			//bb5a;
	unsigned char rolloff_rx_coe; 			//bb5b;
	unsigned char phaseshifter_rx_alfa;		//bb5c;
	unsigned char rolloff_tx_coe;			//bb5d;
	unsigned char phaseshifter_tx_alfa;		//bb5e;
	unsigned char a21a22_ext;
};

void lrf_set_40mhz_channel(int channel_num, int channel_type);
void lrf_set_tx_power(unsigned int level);
int lrf_ch2freq(int ch);
unsigned int lrf_get_tx_power(void);
void panther_rf_init(void);
void config_rfc_parm(int parm, int freq);
int lrf_voltage_temperature(int delay_vot, int delay_reg9, int debug);
int lrf_channel_pll_cal(unsigned int flo);
int lrf_set_freq(unsigned int flo);
int lrf_rf_on(int is_tx);
int lrf_set_pll(unsigned int freq);
void set_dcoc_dac_ctrl(int not_manual);
void set_bb_vgagain(unsigned int input);
void set_dcoc_cal_done(int is_done);
#endif	//  __PANTHER_RF_H__

