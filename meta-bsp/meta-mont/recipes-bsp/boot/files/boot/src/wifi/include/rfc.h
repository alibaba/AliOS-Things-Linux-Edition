/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfc.h
*   \brief
*   \author Montage
*/

#ifndef __RFC_H__
#define __RFC_H__

#include <complex.h>
#include <rfc_comm.h>
#include <panther_rf.h>


/*============================ 1x1 BB (ver < 0x50) ============================*/
void modtest_iq_balancer_new(mismatch_info *param, int is_tx);
double tx_cal_LPFout_new(unsigned char reset, complex y_LPF, mismatch_info *X_tx_iq_balancer, unsigned char tx_cal_state);
void compute_balancer_new(struct bal_parm *tx_bal, struct bal_parm *rx_bal, struct rfc_cal_parm *result, int freq_quantity, double *tone_list, int bw);
void transfer_twofilter_regs_new(struct rfc_cal_parm *cal_parm, struct rfc_cal_reg *result);
void bal_regs_new(mismatch_info param, unsigned char mode);
int rfc_ht_new(int bw, struct vga_tbl *vga_table, struct rfc_record_parm *parm_record , int test_case_no, int tone_mask, int debug_en);
int tx_fine_tune_new(mismatch_info *tx_result, double phase_ref, int loop_max, int bw, int freq, unsigned short tg, unsigned short txvga, double phase_step, double gain_step, int debug_en);
void rfc_process_new(int op);
void config_rfc_parm_new(int bw);
void tx_loopback_report_new(int bw, int freq, unsigned short tg_a_i, unsigned short tg_a_q, unsigned short txvga, unsigned short rxvga, unsigned short tx_i_dc, unsigned short tx_q_dc, unsigned short tx_nm, unsigned short br23_phase, unsigned short br24_gain, int agc_en, complex *read_f0, complex *read_2f0, int lpf_reset, int lpf_sel);
void rx_loopback_report_new(int bw, int freq, unsigned short tg_a,  unsigned short txvga, unsigned short rxvga, unsigned short rx_i_dc, unsigned short rx_q_dc, unsigned short rx_nm, unsigned short br28_phase, unsigned short br29_gain, int agc_en, complex *neg_f0, complex *pos_f0, int lpf_reset, int lpf_sel);
void txlo_cal_new(void);

#ifdef CONFIG_RFC_ANALYST
//int rfc_ht_cmd_new(int argc, char* argv[]);
#endif	// CONFIG_RFC_ANALYST

#endif // __RFC_H__
