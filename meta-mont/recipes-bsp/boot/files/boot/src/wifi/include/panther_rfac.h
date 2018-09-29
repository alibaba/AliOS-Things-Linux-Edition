#ifndef __PANTHER_RFAC_H__
#define __PANTHER_RFAC_H__

int read_txcal_txvga(void);
int set_txcal_txvga(int gain);
int set_txcal_rxvga(int gain);
void panther_set_iqcal_vga(int loop_type, int rxvga, int txvga, int bw, int freq, int iqswap);
int lrf_k_pll_set_ch(int freq, int bw);
int set_rxcal_rxvga(int gain);
int read_txcal_rx_gain(void);
#endif	//  __PANTHER_RFAC_H__

