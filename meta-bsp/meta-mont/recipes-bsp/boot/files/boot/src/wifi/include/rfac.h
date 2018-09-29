/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfac.h
*   \brief
*   \author Montage
*/

#ifndef __RFAC_H__
#define __RFAC_H__


#define CONFIG_RFAC

/* setup the used channel number in the rfac*/
#define RF_CAL_CH 	255

#define RF_DC_LEVEL	4

void ip301_power_on(void);
void ip301_power_off(void);

int vco_calibration(int internal_ldo);
int rx_filter_calibration(u8 bw, u8 filter_code);
int tx_filter_calibration(u8 bw, u8 filter_code);
int tx_rx_filter_calibration(u8 bw, u8 tx_filter_code, u8 rx_filter_code);

int txlo_calibration(int channel);
int rxdc_calibration_old(u8 gain);
int rxdc_calibration(u8 gain);
int rf_calibration(int channel, int internal_ldo);
int mt301_vco_calibration(int internal_ldo);
int mt301_rx_filter_calibration(u8 bw, u8 filter_code);
//int mt301_tx_filter_calibration(u8 bw, u8 filter_code);
int mt301_txlo_calibration(int channel);
int mt301_rxdc_calibration(u8 gain);
int mt301_calibration(int channel, int internal_ldo);


#ifdef RFAC_ANALYSIS
int tx_filter_calibration_check(void);
int rx_filter_calibration_check(void);
void output_vco_curves_statistics(int samples);
void output_rx_filter_statistics(int samples);
void output_tx_filter_statistics(int samples);
void output_txlo_statistics(int samples);
void output_rxdc_statistics(int samples);
int rfac_cmd(int argc, char* argv[]);
#endif // RFAC_ANALYSIS

#endif // __RFAC_H__
