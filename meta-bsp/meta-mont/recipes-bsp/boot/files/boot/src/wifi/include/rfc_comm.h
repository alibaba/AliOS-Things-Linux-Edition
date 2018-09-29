/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfc_comm.h
*   \brief
*   \author Montage
*/

#ifndef __RFC_COMM_H__
#define __RFC_COMM_H__

#include <complex.h>
//#include <mac_ctrl.h>
#include <panther_debug.h>
#include <lib_autoconf.h>
#include <os_compat.h>

#define LPF_RESET 1
#define MAX_RFC_ITERATIONS  	1000
#define FREQ_QUANTITY_40MHZ 	16
#define FREQ_QUANTITY_20MHZ 	8
#define FREQ_TONE_START_20MHZ 	(FREQ_QUANTITY_40MHZ - FREQ_QUANTITY_20MHZ)/2

/* Config the read loop num in the read_LPF() */
#define READ_LPF_CIRCLE 	5
/* Config the OVTH in the rxvga_adjust() */
#define RXVGA_OVTH			110
#define RXVGA_OVTH_TX		110
#define RXVGA_OVTH_RX		115

/* Config BB Reg.0x1D for debug mode */
#define REG1D_VAL			0x96	// original value = 0x46

#define TXLOOP		0
#define RXLOOP		1
#define NON_LOOP	2

#define RXDC_ULTRA_LOW  0
#define RXDC_LOW        1
#define RXDC_MIDDLE     2
#define RXDC_HIGH       3

#define ANGLE_0		0
#define ANGLE_90	1
#define ANGLE_180	2
#define ANGLE_270	3

#define MUX_BASEBAND 0
#define MUX_TONEGEN 7

#ifndef PI
#define PI 3.1415926
#endif

#define RXLOOP 1
//#define INTERACTIVE_MODE

#ifdef CONFIG_RFC_ANALYST
#define TONE_MASK_ENABLE
#endif	// CONFIG_RFC_ANALYST

//#define WLA_GETS					uart_gets
#define WLA_GETS 1
//#define RFC_ATE 1
#ifdef CONFIG_RFC_DEBUG
#define RFC_DEBUG	1
//#define RFC_LOOP_STATISTIC   1
#endif

enum rfc_db_level {
	RFC_DBG_INFO = 0,
	RFC_DBG_WARN,
	RFC_DBG_ERR,
	/* must put RFC_DBG_TRUE to the last, for ATE case */
	RFC_DBG_TRUE,	
};

#ifndef RFC_DBG_LEVEL	
#if defined(CONFIG_ATE_DUT) || defined(CONFIG_MP_TEST)
#define RFC_DBG_LEVEL	RFC_DBG_TRUE
#else
#define RFC_DBG_LEVEL	RFC_DBG_INFO
#endif
#endif

#if defined(RFC_DEBUG)
extern int rfc_dbg_level;
#endif

#if defined(RFC_DEBUG) && !defined(RFC_LOOP_STATISTIC)
#ifndef RFC_DBG
#define RFC_DBG(__lvl, _str...)	do {			\
		if(__lvl >= 0)				\
			DBG_PRINTF(INFO_NO_PREFIX, _str);	\
	} while(0)
#endif
#define RFC_DBG_DOUBLE(__lvl, x)	dbg_double(__lvl, x)
#define BBDBG(x)	DBG_PRINTF(INFO, "Read BB %x %x\n", x, bb_register_read(x))
#define RFC_DBG_COMPLEX(__lvl, x)  		print_complex(__lvl, x)
#else
#define RFC_DBG(...)
#define RFC_DBG_DOUBLE(...)
#define BBDBG(...)
#define RFC_DBG_COMPLEX(...)
#endif

#if 1//defined(CONFIG_OWL_BB)
#define RFC_TONE_LIST_20 {-7.5, -4.375, -2.5, -1.25, 1.25, 2.5, 4.375, 7.5}
#define RFC_TONE_LIST_40 {-16.25, -13.75, -12.5, -11.25, -7.5, -4.375, -2.5, -1.25, 1.25, 2.5, 4.375, 7.5, 11.25, 12.5, 13.75, 16.25}
#else
#define RFC_TONE_LIST_20 {-7.5, -4.0625, -2.5, -1.25, 1.25, 2.5, 4.0625, 7.5}
#define RFC_TONE_LIST_40 {-16.25, -13.75, -12.5, -11.25, -7.5, -4.0625, -2.5, -1.25, 1.25, 2.5, 4.0625, 7.5, 11.25, 12.5, 13.75, 16.25}
#endif

#define READ_LPF_DELAY_SEL 	0
/* the bb version that supports rx ADC sampling rate 80M */
#define RX_80M_VER 0x4b
/* TX/RX Calibration Table*/
#if 1
#define TXCAL_VGA_20MHZ {{1, 9, 7}, {1, 7, 7}, {1, 5, 7}, {1, 4, 7}, \
                                                {1, 4, 7}, {1, 5, 7}, {1, 7, 7}, {1, 9, 7}}
#define RXCAL_VGA_20MHZ        {{2, 2, 7}, {2, 2, 7}, {2, 2, 7}, {2, 2, 7}, \
                                                {2, 2, 7}, {2, 2, 7}, {2, 2, 7}, {2, 2, 7}}
#define TXCAL_VGA_40MHZ        {{2, 9, 6}, {2, 11, 6}, {2, 12, 6}, {2, 12, 6},\
                                                {2, 12, 6}, {2, 10, 6}, {2, 8, 6}, {2, 7, 6}, \
                                                {2, 7, 6}, {2, 8, 6}, {2, 10, 6}, {2, 12, 6}, \
                                                {2, 12, 6}, {2, 12, 6}, {2, 11, 6}, {2, 9, 6}}
#define RXCAL_VGA_40MHZ        {{2, 8, 6}, {2, 8, 6}, {2, 8, 6}, {2, 8, 6}, \
                                                {2, 5, 8}, {2, 4, 8}, {2, 4, 8}, {2, 4, 8}, \
                                                {2, 4, 8}, {2, 4, 8}, {2, 4, 8}, {2, 5, 8}, \
                                                {2, 8, 6}, {2, 8, 6}, {2, 8, 6}, {2, 8, 6}}
#else

#define TXCAL_VGA_20MHZ {{2, 9, 7}, {2, 7, 7}, {2, 5, 7}, {2, 4, 7}, \
						 {2, 4, 7}, {2, 5, 7}, {2, 7, 7}, {2, 9, 7}}
#define RXCAL_VGA_20MHZ	{{2, 2, 7}, {2, 2, 7}, {2, 2, 7}, {2, 2, 7}, \
						 {2, 2, 7}, {2, 2, 7}, {2, 2, 7}, {2, 2, 7}}
#define TXCAL_VGA_40MHZ	{{1, 9, 6}, {1, 11, 6}, {1, 12, 6}, {1, 12, 6},\
						 {2, 12, 6}, {2, 10, 6}, {2, 8, 6}, {2, 7, 6}, \
						 {2, 7, 6}, {2, 8, 6}, {2, 10, 6}, {2, 12, 6}, \
						 {1, 12, 6}, {1, 12, 6}, {1, 11, 6}, {1, 9, 6}}
#define RXCAL_VGA_40MHZ	{{2, 8, 6}, {2, 8, 6}, {2, 8, 6}, {2, 8, 6}, \
						 {2, 5, 8}, {2, 4, 8}, {2, 4, 8}, {2, 4, 8}, \
						 {2, 4, 8}, {2, 4, 8}, {2, 4, 8}, {2, 5, 8}, \
						 {2, 8, 6}, {2, 8, 6}, {2, 8, 6}, {2, 8, 6}}
#endif

#define TX_BAL 0
#define RX_BAL 1

#define RFC_LPF_ALPHA 0x0F
#define RFC_DEMOD_MUTE 0x03
#define RFC_MUX 0x0F
#define RFC_TX_BYPASS 1 <<5
#define RFC_RX_BYPASS 1 <<4

#define DEFAULT_PWRDROPDB   (0)

enum 
{
	I_Q_NONE = 0,
	Q_PATH = 1,
	I_PATH = 2,
};

struct vga_entry {
	short bb_scale;
	short rxvga;
	short txvga;
    short bb_scale_fine;
};

struct vga_tbl {
	struct vga_entry txcal_20mhz[FREQ_QUANTITY_20MHZ];
	struct vga_entry rxcal_20mhz[FREQ_QUANTITY_20MHZ];
	struct vga_entry txcal_40mhz[FREQ_QUANTITY_40MHZ];
	struct vga_entry rxcal_40mhz[FREQ_QUANTITY_40MHZ];
};

typedef struct bal_parm {
    complex gain;
    double phi;
    char a11, a12, a21, a22, n, m;
	char a21_ext;
	char a22_ext;
} mismatch_info;

typedef struct mismatch_curve_data{
	u8 	phase_shifter_ctl;
	double 	phase_shifter_coe;
	u8 	rolloff_filter_ctl;
	double 	rolloff_filter_coe;
} mismatch_curve;

struct rfc_cal_parm {
	mismatch_curve tx_curve;
	mismatch_curve rx_curve;
	mismatch_info tx_avg;
	mismatch_info rx_avg;
};

struct rfc_record_parm {
	double tx_gain;
	double tx_phase;
	double rx_gain;
	double rx_phase;
	unsigned char tx_dc_i;
	unsigned char tx_dc_q;
	//complex iq_signal[4];
	//struct test_reports abnormal;
};

struct result_arry {
	double rec[MAX_RFC_ITERATIONS][2];
};

struct mismatch_check {
	/* result_arry[0] : 20MHz mode, result_arry[1] : 40MHz mode */
	struct result_arry tx_iq_mismatch[2];	/* rec[0]: f0, rec[1]: 2f0 */ 
	struct result_arry rx_iq_mismatch[2];	/* rec[0]: -f0, rec[1]: f0 */ 
};

struct rfc_test_record {
	/* parm[0] : 20MHz mode, parm[1]: 40MHz mode*/
	struct rfc_record_parm parm[2];	
};

#define READ_RXVGA(x)								\
	do {											\
		x = (rf_read(0x9) & 0x1f8) >> 4;		\
	} while(0)

#define COMPLEX_DIV(COMPLEX, DIV)			\
	do{										\
		COMPLEX.real = COMPLEX.real/DIV;	\
		COMPLEX.imag = COMPLEX.imag/DIV;	\
	} while(0)

#define SIGN(x, y)			\
	do{						\
		if(x >= 0) 	y = 1;	\
		else	y = -1;		\
	} while(0)

#define MAX_NUM(arry_a, num, val, ret)		\
	do{										\
		ret = arry_a[0];					\
		for(val = 1; val < num; val++)		\
		{									\
			if(arry_a[val] > ret) 			\
				ret = arry_a[val];			\
		}									\
	} while(0)

#define MIN_NUM(arry_a, num, val, ret)		\
	do{										\
		ret = arry_a[0];					\
		for(val = 1; val < num; val++)		\
		{									\
			if(arry_a[val] < ret) 			\
				ret = arry_a[val];			\
		}									\
	} while(0)


/*** DEBUG FUNCTION ***/
#ifdef RFC_DEBUG
void dbg_double(int level, double x);
void print_complex(int level, complex x);
#else
#define dbg_double(...)
#endif

/*** MATH/Algorithm FUNCTION ***/
int isnormal(double x);
int ipow(int base, int exp);
double angle_diff(double ang1, double ang2);
int angle_phase(double ang);
double calc_angle(double real, double image);
double calc_phase_shifter_angle(double alpha, double f0 , double fs);
double calc_rolloff_gain(double a, double f0 , double fs);
double ctl_coe_calc(int m, double f, double *val);
int calc_abs_txlo(int a, complex *res_f0, complex *res_2f0, double reduce_factor);
void calc_data2iq(complex *LPF1, complex *LPF2, unsigned char *data);
u8 translate_freq_reg2b(double ctl_coe, int sel);
int cmp_phase_shifter_coe_coarse(struct bal_parm *parm, int bw, int tone_num, double *tone_list);
int cmp_phase_shifter_coe_fine(int alpha_coarse, struct bal_parm *parm, int bw, int tone_num, double *tone_list);
#if 0
int cmp_rolloff_filter_coe_coarse(struct bal_parm *parm, int bw);
#endif
void cmp_iq_mismatch(complex dn, complex vn, double *result);
double gain_estimator(complex y_LPF, double gi, int type);
void bal_parm_mean(struct bal_parm *tx_bal, struct bal_parm *rx_bal, struct rfc_cal_parm *rfc_ret, int freq_quantity);
void tx_mux_regs(unsigned char mux, unsigned char tg_a_i, unsigned char tg_a_q, double ctl_coe);
void tx_mux_regs_new(unsigned char mux, unsigned char tg_a_i, unsigned char tg_a_q, double ctl_coe, int pwr_drop_db);
void rx_demod_regs(unsigned char rx_cal_mode_sel, double ctl_coe);
void toggle_reg2e(unsigned char msk, unsigned char en);
int read_LPF(complex *LPF1, complex *LPF2, int reset_en, int delay_sel);
void rx_dc_offset_comp(int degree, int read_write,  double *rxdc_i, double *rxdc_q);
int txlo_pos_neg(int dc_pos, int dc_neg, complex *pos_f0, complex *neg_f0, complex *f0);
void bb_rfc_reset(void);
//void tx_balancer_reverse(mismatch_info *mismatch);

/*** RESULT PRINT/CHECK FUNCTION ***/
void check_statistics(int samples, struct result_arry *record);
void output_rx_iq_mismatch_check_statistics_ht(int samples, struct result_arry *rx_iq_mismatch);
void output_tx_iq_mismatch_check_statistics_ht(int samples, struct result_arry *tx_iq_mismatch);
int check_max_min_avg(double *val_max, double *val_min, double *sum, double *now);
void check_deviation(double val, double *accumlate, double average);
void print_rfc_test_result(struct rfc_test_record *record, struct mismatch_check *mismatch, int samples, int bw);

#ifdef TONE_MASK_ENABLE
int check_parm_with_mask(struct bal_parm *tx_bal, struct bal_parm *rx_bal, int tone_mask, int tone_start, int quantity);
#endif	// TONE_MASK_ENABLE

double diff_phy(double *array_a, int array_num);

#endif // __RFC_COMM_H__
