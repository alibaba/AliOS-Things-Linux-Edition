#ifndef __RFC_PANTHER_H__
#define __RFC_PANTHER_H__
int old_panther_rfc_process(void);
int panther_rfc_process(void);
int search_dcoc_lsb_and_track(int vga, int qn_swap_mode, int *dcoc_dac_lsb, int *dcoc_dac_lsb_track);
int detect_dco_dac(int vga, int dcoc_dac_lsb, int dcoc_dac_lsb_track, int qn_swap_mode, int *dcoc_sm_i, int *dcoc_sm_q, double *final_dc_i, double *final_dc_q);
#endif
