/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfc_comm.c
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <math.h>
#include <complex.h>
#include <ip301.h>
#include <rfc_comm.h>
#include <bb.h>
#include <panther_debug.h>
#include <os_compat.h>

int printf(char *fmt, ...);

#define RFC_TG_A_I 0x0F //<<4
#define RFC_TG_A_Q 0x0F //<<0

#define RFC_REG_ENABLE 0x1
#define RFC_REG_DISABLE 0x0

#ifdef CONFIG_RFC

/*** DEBUG FUNCTION SECTOR ***/
#ifdef RFC_DEBUG
void dbg_double(int level, double x)
{
	if(level < rfc_dbg_level)
		return;
	if(isnan(x))
	{
		DBG_PRINTF(INFO_NO_PREFIX,  "   NaN   ");
	}
	else if(!finite(x))
	{
		DBG_PRINTF(INFO_NO_PREFIX,  "   Inf   ");
	}
	else if(x>=0)
	{
		DBG_PRINTF(INFO_NO_PREFIX, "%d.%08d", (int) x, (int) ((x - ((int) x)) * 100000000));
	}
    else
    {
        x = -1 * x;
        DBG_PRINTF(INFO_NO_PREFIX, "-%d.%08d", (int) x, (int) ((x - ((int) x)) * 100000000));
    }
}

void print_complex(int level, complex x)
{
	if(level < rfc_dbg_level)
		return;
    dbg_double(level, x.real); DBG_PRINTF(INFO_NO_PREFIX, "+"); 
    dbg_double(level, x.imag); DBG_PRINTF(INFO_NO_PREFIX, "i");
}
#endif // RFC_DEBUG

/*** MATH/Algorithm FUNCTION SECTOR ***/
int isnormal(double x)
{
    if(isnan(x) || (!finite(x)))
        return 0;
    else
        return 1;
}

int ipow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

double angle_diff(double ang1, double ang2)
{
	double diff=0;

	if(ang1 < 0)
		ang1 += 360;
	if(ang2 < 0)
		ang2 += 360;

	diff = ang1 - ang2;

	RFC_DBG(RFC_DBG_INFO, "!!!!!!!!!!!!!!!!!! angle diff = ");
    RFC_DBG_DOUBLE(RFC_DBG_INFO, diff);
	RFC_DBG(RFC_DBG_INFO, "\n");

	return diff;
}

int angle_phase(double ang)
{
	int ret=0;

	if(ang < 0)
		ang += 360;

	if((ang <= 45) || (ang > 315))
		ret = ANGLE_0;
	else if(ang <= 135)
		ret = ANGLE_90;
	else if(ang <= 225)
		ret = ANGLE_180;
	else
		ret = ANGLE_270;
	
	return ret;
}

double calc_angle(double real, double image)
{
	double atan_table[10] = {26.565051177077990, 14.036243467926477, 7.125016348901798, 3.576334374997352, 1.789910608246069,  0.895173710211074, 0.447614170860553, 0.223810500368538,  0.111905677066207, 0.055952891893804};
	double temp_x, temp_y, x_arry[10], y_arry[10];
	double ph=0;
	int quadrant=0, xy_switch=0, i=0;

	temp_x = x_arry[0] = real;
	temp_y = y_arry[0] = image;

	//---------ALL 4 quadrant to 1st quadrant-------------------
	if((x_arry[0] > 0) && (y_arry[0] < 0))
	{
		quadrant=4;     
		y_arry[0] = -temp_y;      
	}		
	else if((x_arry[0] < 0) && (y_arry[0] < 0))
	{
		quadrant=3;
		y_arry[0] = -temp_y;
		x_arry[0] = -temp_x;
	}		
	else if((x_arry[0] < 0) && (y_arry[0] > 0))
	{
		quadrant = 2;
		x_arry[0] = -temp_x;
	}		
	else // x>0 && y>0
	{
		quadrant = 1;
	}		

	//%---Now,x and y are all positive (in 1st quadrant)---
	//%--compare who's bigger , x will be the bigger one, y is smaller one
	temp_x = x_arry[0];
	temp_y = y_arry[0];
	if(y_arry[0] > x_arry[0])
	{
		y_arry[0] = temp_x;
		x_arry[0] = temp_y;
		xy_switch = 1;
	}
	else
	{
		xy_switch = 0;
	}

	//%------------cordic loop---------------
	for(i=0; i < 10; i++)
	{
		if(y_arry[i] > 0)
		{
			if(i < 9)
			{
				x_arry[i+1] = x_arry[i] + (y_arry[i] * pow(2, -(i+1)));
				y_arry[i+1] = y_arry[i] - (x_arry[i] * pow(2, -(i+1)));
			}
			ph = ph + atan_table[i];
		}
		else //%y<0
		{
			if(i < 9)
			{
				x_arry[i+1] = x_arry[i] - (y_arry[i] * pow(2, -(i+1)));
				y_arry[i+1] = y_arry[i] + (x_arry[i] * pow(2, -(i+1)));
			}
			ph=ph-atan_table[i];
		}
	}

	//%--------postcordic----------
	if(xy_switch == 1)
		ph = 90 - ph;

	if(quadrant == 3)
		ph=180+ph;
	else if(quadrant == 2)
		ph=180-ph;
	else if(quadrant == 4)
		ph=-ph;

	//%-----------------Output------------------------------------------------
	return ph;
}

double calc_phase_shifter_angle(double alpha, double f0 , double fs)
{
	double A = tan(PI*f0/fs)*(1-alpha)/(1+alpha);
	double real_part = (1 - A*A);
	double imag_part = -2*A;
	
	return (calc_angle(real_part, imag_part)*PI/180);
}

double calc_rolloff_gain(double a, double f0 , double fs)
{
	double response;
	double real, imag;
	
	real = a + (1-2*a)*cos(2*PI*f0/fs) + a*cos(4*PI*f0/fs);
	imag = (1-2*a)*sin(2*PI*f0/fs) + a*sin(4*PI*f0/fs);
	response = real*real + imag*imag;

	RFC_DBG(RFC_DBG_INFO, "%s(): response = ", __FUNCTION__); RFC_DBG_DOUBLE(RFC_DBG_INFO, response);
	RFC_DBG(RFC_DBG_INFO, ", ");RFC_DBG_DOUBLE(RFC_DBG_INFO, (log10(response)*20));RFC_DBG(RFC_DBG_INFO, "\n");
	
	return response;
}

double ctl_coe_calc(int m, double f, double *val)
{
	*val = m * f;
	
	return *val;
}

int calc_abs_txlo(int a, complex *res_f0, complex *res_2f0, double reduce_factor)
{
	int dc_norm_amp;

	dc_norm_amp = (0.25*a*(abs_complex(res_f0))*reduce_factor)/(abs_complex(res_2f0));

	return dc_norm_amp;
}

void calc_data2iq(complex *LPF1, complex *LPF2, unsigned char *data)
{
    u32 LPF1_I, LPF1_Q, LPF2_I, LPF2_Q;
	
	LPF1_I = (data[0]&0xFF)<<16 | (data[1]&0xFF)<<8 | (data[2]&0xff) ;
	LPF1_Q = (data[3]&0xFF)<<16 | (data[4]&0xFF)<<8 | (data[5]&0xff) ;
	LPF2_I = (data[6]&0xFF)<<16 | (data[7]&0xFF)<<8 | (data[8]&0xff) ;
	LPF2_Q = (data[9]&0xFF)<<16 | (data[10]&0xFF)<<8 | (data[11]&0xff) ;
	// 16777216 = 2^24
	LPF1->real = hex2flt(LPF1_I, 24)/(16777216);
	LPF1->imag = hex2flt(LPF1_Q, 24)/(16777216);
	LPF2->real = hex2flt(LPF2_I, 24)/(16777216);
	LPF2->imag = hex2flt(LPF2_Q, 24)/(16777216);
}

double diff_phy(double *array_a, int array_num)
{
	double ret=0, max=array_a[0], min=array_a[0];
	int i;
	
	for(i=1; i<array_num; i++)
	{
		if(array_a[i] > max)
			max = array_a[i];
		else if(array_a[i] < min)
			min = array_a[i];
	}

	ret = max - min;

	return ret;
}

u8 translate_freq_reg2b(double ctl_coe, int sel)
{
	double tmp;
	u8 ret = 0;

	if(bb_register_read(0, 0x0) < 0x80)	// Cheetah chip
		tmp = (ctl_coe * 10 * 10 * 10)/312.5;
	else	//	OWL chip
		tmp = (ctl_coe * 10 * 10 * 10)/(312.5*2);

#ifdef CONFIG_MONTE_CARLO
	if(!sel)
		tmp = tmp * 2;
#endif

	ret = (u8) tmp;
	
	return ret;
}

#ifdef TONE_MASK_ENABLE
int check_parm_with_mask(struct bal_parm *tx_bal, struct bal_parm *rx_bal, int tone_mask, int tone_start, int quantity)
{
	int i=0, j=quantity;
	int tone_end = tone_start + quantity;
	
	if(((1 << tone_end) - 1) > tone_mask)
	{
		j = 0;
		for(i=0; i<quantity; i++)
		{
			if((1 << (i + tone_start)) & tone_mask)
			{
				tx_bal[j].gain.real = tx_bal[i].gain.real;
				tx_bal[j].gain.imag = tx_bal[i].gain.imag;
				tx_bal[j].phi = tx_bal[i].phi;
				rx_bal[j].gain.real = rx_bal[i].gain.real;
				rx_bal[j].gain.imag = rx_bal[i].gain.imag;
				rx_bal[j].phi = rx_bal[i].phi;
				j++;
			}
		}
	}

	return j;
}
#endif	// TONE_MASK_ENABLE

/* FIXME: PANTHER tonelist is not equal to the cheetah */
int cmp_phase_shifter_coe_coarse(struct bal_parm *parm, int bw, int tone_num, double *tone_list)
{
	//int tone_num=0;
	int edge_loc=0;	//fs=0;
	int l=0, tmp1=0;
	int alpha_coarse;
	//double tone_list_20[FREQ_QUANTITY_20MHZ] = RFC_TONE_LIST_20;
	//double tone_list_40[FREQ_QUANTITY_40MHZ] = RFC_TONE_LIST_40;
	//double *tone_list;
	double slope=0, slope_avg=0, abs_slope_avg=0;
	double cross_corr=0, auto_corr=0;

	if(bw)
	{
		//tone_num = FREQ_QUANTITY_40MHZ;
		//tone_list = tone_list_40;
		// coarse est 
		edge_loc = 20; 		// pi=20M
		//fs = 80;    
	}
	else
	{
		//tone_num = FREQ_QUANTITY_20MHZ;
		//tone_list = tone_list_20;
		// coarse est 
		edge_loc = 10;    	// pi=10M
		//fs = 40;
	}
	 
	/* 	phase_diff is in radian representation , not degree
	 	slope_avg is represent in  radian/MHz 					*/
	for(l=0; l < tone_num; l++)
	{
		cross_corr += tone_list[l] * parm[l].phi;
		auto_corr += tone_list[l] * tone_list[l];
	}
	slope_avg = cross_corr / auto_corr;
						 
	/* slope_avg will make the decision which channel will pass through all pass filter and finetune process */
	abs_slope_avg = fabs(slope_avg);
						 
	SIGN(slope_avg, tmp1);

	slope = abs_slope_avg * edge_loc;
	alpha_coarse = ((2 - slope)/(2 + slope))*256;
	alpha_coarse = alpha_coarse * tmp1;
	
	return alpha_coarse;
}

int cmp_phase_shifter_coe_fine(int alpha_coarse, struct bal_parm *parm, int bw, int tone_num, double *tone_list)
{
	// decide alpha tunning range, coarse+0.05 ~  coarse-0.05
	int alpha_sign;
	//int	tone_num = 0;
	int l=0;
	int search_range=2;
	int alpha_step=2;
	int alpha_coarse_up_lim;
	int alpha_coarse_down_lim;
	int alpha_idx=0;
	int fine_alpha=0;
	double sum_temp=0;
	double min_err_cost=100.0, err_cost=0;
	double ph_response, phase_mean;
	double err=0, err_abs;
	//double tone_list_20[FREQ_QUANTITY_20MHZ] = RFC_TONE_LIST_20;
	//double tone_list_40[FREQ_QUANTITY_40MHZ] = RFC_TONE_LIST_40;
	//double *tone_list;
	double f0=0, fs=0;
	double max_tone_err=0;
	double max_tone_err_ori=0;
	double err_square=0;

	SIGN(alpha_coarse, alpha_sign);
	alpha_coarse = fabs(alpha_coarse);
	alpha_coarse_up_lim = alpha_coarse + search_range;
	alpha_coarse_down_lim = alpha_coarse - search_range;

	//Restriction
	if(alpha_coarse_up_lim > 255)
		alpha_coarse_up_lim	= 255;
	
	if(alpha_coarse_down_lim < 0)
		alpha_coarse_down_lim = 0;

	if(bw==0)   //BW=20MHz
	{
		fs=40;
		//tone_list = tone_list_20;
		//tone_num = FREQ_QUANTITY_20MHZ;
	}
	else  		//BW=40MHz
	{
		fs=80;    
		//tone_list = tone_list_40;
		//tone_num = FREQ_QUANTITY_40MHZ;
	}

	/*	calc mean of bal_parm.phase, so the code can be replaced by any other
		function. Just find out the mean					*/
		
	for(l=0; l < tone_num; l++)
		sum_temp = sum_temp + parm[l].phi;
	
	phase_mean = sum_temp/tone_num;

	/*	apply min(max) 1st, LS maybe later
		error is defined as 
		phi_mean=mean(mismatch_info(1:end).phi);
		if slope_avg>0  : error=mismatch_info.phi - phi_mean + ph_response
		elseif slope_avg<0  : error=mismatch_info.phi - phi_mean - ph_response
		[ph_response]=calc_phase_shifter_angle(alpha, f0 ,fs ) */
		
	/*	--------alpha fine tune begin-------------------	*/
	for(alpha_idx=alpha_coarse_down_lim; alpha_idx<=alpha_coarse_up_lim; alpha_idx+=alpha_step)
	{
		max_tone_err = 0;
		err_square = 0;
		
		for(l=0; l < tone_num; l++)
		{
			f0 = tone_list[l];
			ph_response = calc_phase_shifter_angle(((double)alpha_idx/256), f0 , fs);    
			
			err = parm[l].phi - phase_mean + (alpha_sign * ph_response);
			err_abs = fabs(err);
			err_square += err*err;
			if(err_abs > max_tone_err)
			{
				max_tone_err = err_abs;
			}
		}

		//%Find out the min err alpha
		err_cost = err_square;
		if(err_cost < min_err_cost)
		{
			min_err_cost = err_cost;
			fine_alpha = alpha_idx;
			max_tone_err_ori = max_tone_err;
		}
	}																			

	//%restore sign of fine_alpha
	fine_alpha = fine_alpha * alpha_sign;

	/*	%it must be better compared with the no-filter condition
		%find max(bal_parm(l).phase - phase_mean), it can be replaced by any other function 
		%which has the same move 															*/
	max_tone_err=0;
	for(l = 0; l < tone_num; l++)
	{
		err = parm[l].phi - phase_mean;
		err_abs = fabs(err);
		if(err_abs > max_tone_err)
			max_tone_err = err_abs;            
	}
	
	if(max_tone_err < max_tone_err_ori)
	{
		fine_alpha=256;
	}

	return fine_alpha;
}

int cmp_rolloff_filter_coe_coarse(struct bal_parm *parm, int bw)
{
	double gain_th=1.035; // %0.3dB, make it positive gain
	double gain_th_inv, gain_ratio;
	double max_gain, min_gain, center_gain;
	double neg_bandedge_gain, pos_bandedge_gain, neg_bandedge2_gain, pos_bandedge2_gain;
	double edge_gain_mul=0, edge_gain=0, coarse_a=0;
	int neg_edge_rise, neg_edge_fall, pos_edge_rise, pos_edge_fall;
	int goup=0, godown=0;
	int tone_num, tone_center;
	int l, ret;

	gain_th_inv = 1/gain_th;
	
	if(bw == 0)
		tone_num = FREQ_QUANTITY_20MHZ;
	else
		tone_num = FREQ_QUANTITY_40MHZ;
	tone_center = tone_num/2;

	/* 	Do we need turn on roll off filter?  
		if max(gain)/min(gain)<th then we just ignore this filter
		find max/min */
	max_gain = parm[0].gain.imag;
	min_gain = parm[0].gain.imag;
	for(l=1; l < tone_num; l++)
	{
		if(parm[l].gain.imag > max_gain)
			max_gain = parm[l].gain.imag;
						
		if(parm[l].gain.imag < min_gain)
			min_gain = parm[l].gain.imag;
	}
	gain_ratio = max_gain/min_gain;

	/* make sure it's really roll-off */
	if(gain_ratio >= gain_th)
	{
		/* start to calc possible filter coe. 1~8 or 1~16 */
		center_gain = 0.5*(parm[tone_center-1].gain.imag + parm[tone_center].gain.imag);

		/* edge1/center */
		neg_bandedge_gain = parm[0].gain.imag/center_gain;
		pos_bandedge_gain = parm[tone_num-1].gain.imag/center_gain;

		/* edge2/center */
		neg_bandedge2_gain = parm[1].gain.imag/center_gain;
		pos_bandedge2_gain = parm[tone_num-2].gain.imag/center_gain;

		/* Does negative edge rise or fall? */
		neg_edge_rise = (neg_bandedge_gain > gain_th)  || (neg_bandedge2_gain > gain_th);
		neg_edge_fall = (neg_bandedge_gain < gain_th_inv) || (neg_bandedge2_gain < gain_th_inv);

		/* Does positive edge rise or fall? */
		pos_edge_rise = (pos_bandedge_gain > gain_th) || (pos_bandedge2_gain > gain_th);
		pos_edge_fall = (pos_bandedge_gain < gain_th_inv) || (pos_bandedge2_gain < gain_th_inv);
					
		/* Both pos/neg edge are leaving center. We have to know which dir and two side situation. */
		if((neg_edge_rise || neg_edge_fall) && (pos_edge_rise || pos_edge_fall))
		{
			/* make sure two side is on the same dir */
			if(neg_edge_rise && pos_edge_rise)
				goup = 1;
			if(neg_edge_fall && pos_edge_fall)
				godown = 1;
		
			/* make a avg of 4 tones result, prepare for coarse estimation */
			edge_gain = 0.25*(neg_bandedge_gain + pos_bandedge_gain + neg_bandedge2_gain + pos_bandedge2_gain);
				
			if(goup && godown) // go up  and down @ the same time...
			{
				/* find dominant gain up or down */
				edge_gain_mul = neg_bandedge_gain * pos_bandedge_gain * neg_bandedge2_gain * pos_bandedge2_gain;
				if(edge_gain_mul > 1)
					godown = 0;
				else if(edge_gain_mul < 1)
					goup = 0;
				else
					godown = goup = 0;
			}

			if(edge_gain > 1)
				edge_gain = 1/edge_gain;

			/* find appropriate coarse_a */
			coarse_a = (-0.5)*(edge_gain/1.03 - 1);
			if(coarse_a < 0)
				coarse_a = 0;
		}
	}
				
	/* just calc one time? */
	if((!goup && !godown) || (goup && godown))
		coarse_a = 0;

	ret = (int) floor(coarse_a*128);
	return ret;
}

void cmp_iq_mismatch(complex dn, complex vn, double *result)
{
    complex temp, Pdv, K1K2;
    double Pdplusv;

    Pdv = complex_multi(dn,vn);

    //Pdplusv = abs(dn+conj(vn))^2
    temp = complex_add(dn,complex_conj(vn));
    Pdplusv = (temp.real*temp.real)+(temp.imag*temp.imag);

    //complex Pdplusv = complex_conj(vn);
    K1K2.real = Pdv.real/Pdplusv;
    K1K2.imag = Pdv.imag/Pdplusv;

    *result = sqrt(1 - 4*K1K2.real); //g
    *(result+1) = asin(-1*(2/(*result))*K1K2.imag); //phi
}

double gain_estimator(complex y_LPF, double gi, int type)
{
    double A, B, ret;

    //Estimate A (Amplitude of x)
    B = y_LPF.real*y_LPF.real + y_LPF.imag*y_LPF.imag;
    A = 2*sqrt(B);

    if (0 == type)
    {
		//Estimate g (A = g^2/4)
		ret = 2*sqrt(A);
    }
    else //(1 == type)
    {
        //Estimate phi |-sin(phi)/2| = |A|; phi = asin(2*|A|))
        ret = asin(2*fabs(A/(gi*gi)));
        //data in phi.real
    }

	return ret;
}

void bal_parm_mean(struct bal_parm *tx_bal, struct bal_parm *rx_bal, struct rfc_cal_parm *rfc_ret, int freq_quantity)
{
	complex tx_gain, rx_gain;
	double tx_phi, rx_phi;
	int i;

	tx_gain.real = tx_gain.imag = rx_gain.real = rx_gain.imag = tx_phi = rx_phi = 0;

	for(i=0; i<freq_quantity; i++)
	{
		tx_gain.real += tx_bal[i].gain.real;
		tx_gain.imag += tx_bal[i].gain.imag;
		tx_phi += tx_bal[i].phi;
		rx_gain.real += rx_bal[i].gain.real;
		rx_gain.imag += rx_bal[i].gain.imag;
		rx_phi += rx_bal[i].phi;
	}

	rfc_ret->tx_avg.gain.real 	= tx_gain.real/freq_quantity;
	rfc_ret->tx_avg.gain.imag 	= tx_gain.imag/freq_quantity;
	rfc_ret->tx_avg.phi 		= tx_phi/freq_quantity;
	rfc_ret->rx_avg.gain.real 	= rx_gain.real/freq_quantity;
	rfc_ret->rx_avg.gain.imag 	= rx_gain.imag/freq_quantity;
	rfc_ret->rx_avg.phi 		= rx_phi/freq_quantity;
}

void tx_mux_regs(unsigned char mux, unsigned char tg_a_i, unsigned char tg_a_q, double ctl_coe)
{
    tx_mux_regs_new(mux, tg_a_i, tg_a_q, ctl_coe, DEFAULT_PWRDROPDB);
}

void tx_mux_regs_new(unsigned char mux, unsigned char tg_a_i, unsigned char tg_a_q, double ctl_coe, int pwr_drop_db)
{
    unsigned char value20;
    u8 reg_val;

    //Reserved TX-balancer bypass mode and RX-balancer bypass mode
    value20 = bb_register_read(0, 0x2f);

    if (0 > pwr_drop_db)
    {
        DBG_PRINTF(WARN, "WARN: pwrDropdB is less than 0.\n");
        pwr_drop_db = 0;
    }
    else if (5 < pwr_drop_db)
    {
        DBG_PRINTF(WARN, "WARN: pwrDropdB is bigger than 5.\n");
        pwr_drop_db = 5;
    }
    reg_val = bb_register_read(0, 0x32);
    reg_val = (reg_val & 0xf1) | ((pwr_drop_db << 1) & 0x0eUL);
    bb_register_write(0, 0x32, reg_val);

    //Program tx MUX mode and bypass bits
	//Set TX MUX mode 0~15
	if (mux > 15) //BR2F[3:0]
	{
		DBG_PRINTF(ERROR, "ERROR: Too large mux value.\n");
		return;
	}
	reg_val = (value20&RFC_TX_BYPASS) | (value20&RFC_RX_BYPASS) | (mux&RFC_MUX) ;

    //Write TX MUX setting
    bb_register_write(0, 0x2f, reg_val);

    if (mux == 7) //If in tonegen mode, setting tone generator component
    {
        //Convert tone amplitude and program it
        reg_val = (tg_a_i&RFC_TG_A_I)<<4 | (tg_a_q&RFC_TG_A_Q);
        bb_register_write(0, 0x2c, reg_val);

        //Convert tone frequency (tone frequency unit:MHz) and program it
        reg_val = translate_freq_reg2b(ctl_coe, (bb_register_read(0, 0x32) & 0x1));
        bb_register_write(0, 0x2b, reg_val);
    }
}

void rx_demod_regs(unsigned char rx_cal_mode_sel, double ctl_coe)
{
    char reg_val;
	int sel=0;

    //Convert tone frequency (tone frequency unit:MHz) and program it
	if((bb_register_read(0, 0x32) & 0x1))
		sel = 1;
	reg_val = translate_freq_reg2b(ctl_coe, sel);

//#ifdef CONFIG_MONTE_CARLO
	if(bb_register_read(0, 0x0) >= 0x80)	// OWL chip
		reg_val = reg_val * 2;
//#endif

    bb_register_write(0, 0x2d, reg_val);
    
    //Program Demod and LPF register
    reg_val = (rx_cal_mode_sel&RFC_DEMOD_MUTE)<<2;
    bb_register_write(0, 0x2e, reg_val);
}

void toggle_reg2e(unsigned char msk, unsigned char en)
{
    unsigned char value2e, reg_val;
    //Reserved RX demod configuration
    value2e = bb_register_read(0, 0x2e);

    reg_val = (value2e&(0xFF - msk)) | en;
    bb_register_write(0, 0x2e, reg_val);
}

int read_LPF(complex *LPF1, complex *LPF2, int reset_en, int delay_sel)
{
	/*	Reg.0x2E[0] = latch enable
		if bit.0 = 1 => latch the lpf data
		else => reflect the realtime lpf data	*/
	u8 val = (bb_register_read(0, 0x2e) & (~(RFC_LPF_ALPHA << 4)));
    u8 lpf_alpha1=0;
	u8 data[12];
	unsigned long delay1=0;
	complex lpf1[READ_LPF_CIRCLE], lpf2[READ_LPF_CIRCLE];
	int i, j;

	switch(delay_sel)
	{
		case 0:	// 20 MHz
			lpf_alpha1 = 13;
			delay1 = 208;
			//delay1 = 104;
			break;
		case 1:	// 40 MHz
			lpf_alpha1 = 11;
			delay1 = 26;
			break;
		case 2:	// 20 MHz
			lpf_alpha1 = 10;
			delay1 = 26;
			break;
		case 3:	// 40 MHz
			lpf_alpha1 = 10;
			delay1 = 13;
			break;
		case 4:	// 20 MHz
			lpf_alpha1 = 9;
			delay1 = 13;
			break;
		case 5:	// 40 MHz
			lpf_alpha1 = 9;
			delay1 = 7;
			break;
		case 6:	// 20 MHz
			lpf_alpha1 = 8;
			delay1 = 7;
			break;
		case 7:	// 40 MHz
			lpf_alpha1 = 8;
			delay1 = 4;
			break;
		case 8:	// 20 MHz
			lpf_alpha1 = 7;
			delay1 = 4;
			break;
		case 9:	// 40 MHz
			lpf_alpha1 = 7;
			delay1 = 2;
			break;
		default:
			/* for error handle, if the delay_sel value is not expected */
			RFC_DBG(RFC_DBG_INFO, "! read_LPF(): delay_sel=%d\n", delay_sel);
			lpf_alpha1 = 10;
			delay1 = 100; 
			break;
	}
			
	bb_register_write(0, 0x2e, val | (lpf_alpha1 << 4));
	
	/* do reset & enable */
	if(reset_en)
		toggle_reg2e(0x3, 0x2);
	
	for(i=0; i < READ_LPF_CIRCLE; i++)
	{
		toggle_reg2e(0x3, RFC_REG_ENABLE);
		udelay(delay1);
		toggle_reg2e(0x1, RFC_REG_DISABLE);

		for(j=0; j<12; j++)
		{
			data[j] = bb_register_read(0, 0x40 + j);
		}

		calc_data2iq(&lpf1[i], &lpf2[i], data);
	}

	if(LPF1)
		*LPF1 = complex_mean(lpf1, READ_LPF_CIRCLE);
	if(LPF2)
		*LPF2 = complex_mean(lpf2, READ_LPF_CIRCLE);

	return 0;
}

void rx_dc_offset_comp(int degree, int read_write,  double *rxdc_i, double *rxdc_q)
{
	double ctl_coe=0;
	double rx_dc_i=0, rx_dc_q=0;
	unsigned char cal_mode_sel=0x1;
	complex i_signal_f0, q_signal_f0;
	char val_i, val_q;
	int lpf_sel=0;

	/*  read_write: 1 = only write, 2 = only read, 3 = read& write*/

	if(read_write & 0x1)
	{
		bb_register_write(0, 0x2a, 0);
		bb_register_write(0, 0x31, 0);
	}
	
	rx_demod_regs(cal_mode_sel, ctl_coe);

	if(degree == 0)
		degree = 10;

	if(degree == 14)
		lpf_sel = 2;
	else if(degree == 13)
		lpf_sel = 4;
	else if(degree == 12)
		lpf_sel = 6;
	else if(degree == 11)
		lpf_sel = 8;
	else
		lpf_sel = 2;
		
	//RFC_DBG(RFC_DBG_INFO, "%s(): degree=%d, read_write=%d\n", __FUNCTION__, degree, read_write);

	if((read_write & 0x1) && 
		(!(read_LPF(&i_signal_f0, &q_signal_f0, LPF_RESET, lpf_sel))))
	{
		rx_dc_i = i_signal_f0.real * ipow(2, degree);
		rx_dc_q = i_signal_f0.imag * ipow(2, degree);

		//Transform rx_dc_I, rx_dc_Q to signed char
		val_i = rx_dc_i;
		val_q = rx_dc_q;
#if 0
		RFC_DBG(RFC_DBG_INFO, "Set val_i = %d (", val_i);
		RFC_DBG_DOUBLE(RFC_DBG_INFO, rx_dc_i); RFC_DBG(RFC_DBG_INFO, ")\n");
		RFC_DBG(RFC_DBG_INFO, "Set val_q = %d (", val_q);
		RFC_DBG_DOUBLE(RFC_DBG_INFO, rx_dc_q); RFC_DBG(RFC_DBG_INFO, ")\n");
#endif	
		//write to reg
		bb_register_write(0, 0x2a, val_i);
		bb_register_write(0, 0x31, val_q);
	}

	if((read_write & 0x2) && 
		!(read_LPF(&i_signal_f0, &q_signal_f0, LPF_RESET, lpf_sel)))
	{
		rx_dc_i = i_signal_f0.real * ipow(2, degree);
		rx_dc_q = i_signal_f0.imag * ipow(2, degree);

		//Transform rx_dc_I, rx_dc_Q to signed char
		val_i = rx_dc_i;
		val_q = rx_dc_q;

#if 0
		RFC_DBG(RFC_DBG_INFO, "Double check val_i = %d (", val_i);
		RFC_DBG_DOUBLE(RFC_DBG_INFO, rx_dc_i); RFC_DBG(RFC_DBG_INFO, ")\n");
		RFC_DBG(RFC_DBG_INFO, "Double check val_q = %d (", val_q);
		RFC_DBG_DOUBLE(RFC_DBG_INFO, rx_dc_q); RFC_DBG(RFC_DBG_INFO, ")\n");
#endif
	}

	if(rxdc_i)
		*rxdc_i = rx_dc_i;
	if(rxdc_q)
		*rxdc_q = rx_dc_q;
}

int txlo_pos_neg(int dc_pos, int dc_neg, complex *pos_f0, complex *neg_f0, complex *f0)
{
	int dc_est;
	double abs_f0_pos = abs_complex(pos_f0);
	double abs_f0_neg = abs_complex(neg_f0);
	double abs_f0_new;

	if(abs_f0_pos > abs_f0_neg)
	{
		dc_est = dc_neg;
		abs_f0_new = abs_f0_neg;
	}
	else
	{
		dc_est = dc_pos;
		abs_f0_new = abs_f0_pos;
	}

	if(abs_f0_new > abs_complex(f0))
		dc_est = (dc_pos+dc_neg)/2;

	return dc_est;
}

void bb_rfc_reset(void)
{
    bb_register_write(0, 0x20, 0x00);       
    //bb_register_write(0, 0x21, 0x7F);
    //bb_register_write(0, 0x22, 0x00);
    bb_register_write(0, 0x23, 0x00);
    bb_register_write(0, 0x24, 0x7F);
    //bb_register_write(0x25, 0x00);
    //bb_register_write(0, 0x26, 0x7F);
    //bb_register_write(0, 0x27, 0x00);
    bb_register_write(0, 0x28, 0x00); 
    bb_register_write(0, 0x29, 0x7F);
    bb_register_write(0, 0x2a, 0x00);        
    bb_register_write(0, 0x2b, 0x00);      
    bb_register_write(0, 0x2c, 0x00);       
    bb_register_write(0, 0x2d, 0x00);       
    bb_register_write(0, 0x2e, 0x00);
    bb_register_write(0, 0x2f, 0x00);
    bb_register_write(0, 0x31, 0x00);
    bb_register_write(0, 0x33, 0x33);
    bb_register_write(0, 0x5a, 0x00);
    bb_register_write(0, 0x5b, 0x00);
    bb_register_write(0, 0x5c, 0x00);
    bb_register_write(0, 0x5d, 0x00);
    bb_register_write(0, 0x5e, 0x00);
}


/*** RESULT PRINT/CHECK FUNCTION SECTOR ***/
#ifdef RFC_DEBUG
void check_statistics(int samples, struct result_arry *record)
{
    int i;
    int j;
    double curr_max[2];
    double curr_min[2];
    double accmulate;
    double average[2];
    double variance[2];

	/* calc the max/min/avg/dev value */
	for(j=0;j<2;j++)
    {
        curr_max[j] = record->rec[0][j];
        curr_min[j] = record->rec[0][j];
        accmulate = 0;
        
        for(i=0;i<samples;i++)
        {
            if( record->rec[i][j] > curr_max[j])
                curr_max[j] = record->rec[i][j];
			else if( record->rec[i][j] < curr_min[j])
                curr_min[j] = record->rec[i][j];
            
            accmulate += record->rec[i][j];
        }
        
        average[j] = accmulate/samples;

        accmulate = 0;
		for(i=0;i<samples;i++)
        {
            accmulate += ((record->rec[i][j] - average[j]) * 
						  (record->rec[i][j] - average[j]));
        }
        
        variance[j] = sqrt((accmulate / samples));
    }

	/* print the calculated value */
	for(i=0;i<samples;i++)
    {
        RFC_DBG(RFC_DBG_INFO, " %04d      ", i);
        RFC_DBG_DOUBLE(RFC_DBG_INFO, record->rec[i][0]); RFC_DBG(RFC_DBG_INFO, " db  ");
        RFC_DBG_DOUBLE(RFC_DBG_INFO, record->rec[i][1]); RFC_DBG(RFC_DBG_INFO, " db  ");
        RFC_DBG(RFC_DBG_INFO, "\n");
    }

    RFC_DBG(RFC_DBG_INFO, "-------------------------------------------------------------------\n");
    RFC_DBG(RFC_DBG_INFO, " MAX       ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, curr_max[0]); RFC_DBG(RFC_DBG_INFO, " db  ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, curr_max[1]); RFC_DBG(RFC_DBG_INFO, " db  ");
    RFC_DBG(RFC_DBG_INFO, "\n");

    RFC_DBG(RFC_DBG_INFO, " MIN       ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, curr_min[0]); RFC_DBG(RFC_DBG_INFO, " db  ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, curr_min[1]); RFC_DBG(RFC_DBG_INFO, " db  ");
    RFC_DBG(RFC_DBG_INFO, "\n");

    RFC_DBG(RFC_DBG_INFO, " AVG       ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, average[0]); RFC_DBG(RFC_DBG_INFO, " db  ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, average[1]); RFC_DBG(RFC_DBG_INFO, " db  ");
    RFC_DBG(RFC_DBG_INFO, "\n");

    RFC_DBG(RFC_DBG_INFO, " DEV         ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, variance[0]); RFC_DBG(RFC_DBG_INFO, "     ");
	RFC_DBG_DOUBLE(RFC_DBG_INFO, variance[1]); RFC_DBG(RFC_DBG_INFO, "     ");
    RFC_DBG(RFC_DBG_INFO, "\n");

    RFC_DBG(RFC_DBG_INFO, "-------------------------------------------------------------------\n");

}

void output_rx_iq_mismatch_check_statistics_ht(int samples, struct result_arry *rx_iq_mismatch)
{
    RFC_DBG(RFC_DBG_INFO, "---------RX IQ mismatch check -------------------------------------\n");
    RFC_DBG(RFC_DBG_INFO, "  NO.           -f0               f0        \n");
    RFC_DBG(RFC_DBG_INFO, "-------------------------------------------------------------------\n");
	
	check_statistics(samples, rx_iq_mismatch);
}

void output_tx_iq_mismatch_check_statistics_ht(int samples, struct result_arry *tx_iq_mismatch)
{
    RFC_DBG(RFC_DBG_INFO, "---------TX IQ mismatch check -------------------------------------\n");
    RFC_DBG(RFC_DBG_INFO, "  NO.           f0               2f0         \n");
    RFC_DBG(RFC_DBG_INFO, "-------------------------------------------------------------------\n");

	check_statistics(samples, tx_iq_mismatch);
}

int check_max_min_avg(double *val_max, double *val_min, double *sum, double *now)
{
	if(!isnormal(*now))
		return 1;
	
	if(*val_max == 0 || *val_min == 0)
	{
		*val_max = *now;
		*val_min = *now;
	}
	else if(*val_max <= *now)
	{
		*val_max = *now;
	}
	else if(*val_min >= *now)
	{
		*val_min = *now;
	}

	*sum = *sum + *now;
	
	return 0;
}

void check_deviation(double val, double *accumlate, double average)
{
	if(!isnormal(val))
		return;

	*accumlate += ((val - average) * (val - average));
}

void print_rfc_test_result(struct rfc_test_record *record, 
						   struct mismatch_check *mismatch,
						   int samples,
						   int bw)
{
	int 	i, num[4];
	double 	accumlate[4], max[4], min[4], dev[4], avg[4];

	if(bw)
		RFC_DBG(RFC_DBG_ERR, "========== 40MHz Mode Test Result ==========\n");
	else
		RFC_DBG(RFC_DBG_ERR, "========== 20MHz Mode Test Result ==========\n");

	/* Setup parameter */
	for(i=0; i<4; i++)
		accumlate[i] = max[i] = min[i] = num[i] = dev[i] = 0;

	RFC_DBG(RFC_DBG_ERR, "-------------------------------------------------------------------\n");
	RFC_DBG(RFC_DBG_ERR, "  NO.        TX gain       TX phase        RX gain       RX phase     TX_DC_I   TX_DC_Q\n");
	RFC_DBG(RFC_DBG_ERR, "-------------------------------------------------------------------\n");
	for(i=0;i<samples;i++)
	{
		RFC_DBG(RFC_DBG_ERR, " %04d      ", i);
		RFC_DBG_DOUBLE(RFC_DBG_ERR, record[i].parm[bw].tx_gain); RFC_DBG(RFC_DBG_ERR, "     ");
		RFC_DBG_DOUBLE(RFC_DBG_ERR, record[i].parm[bw].tx_phase); RFC_DBG(RFC_DBG_ERR, "     ");
		RFC_DBG_DOUBLE(RFC_DBG_ERR, record[i].parm[bw].rx_gain); RFC_DBG(RFC_DBG_ERR, "     ");
		RFC_DBG_DOUBLE(RFC_DBG_ERR, record[i].parm[bw].rx_phase); RFC_DBG(RFC_DBG_ERR, "     ");
		RFC_DBG(RFC_DBG_ERR, "0x%x     0x%x\n", record[i].parm[bw].tx_dc_i, record[i].parm[bw].tx_dc_q);
		
		if(check_max_min_avg(&max[0], &min[0], &accumlate[0], &record[i].parm[bw].tx_gain) == 0)
			num[0] += 1;
		if(check_max_min_avg(&max[1], &min[1], &accumlate[1], &record[i].parm[bw].tx_phase) == 0)
			num[1] += 1;
		if(check_max_min_avg(&max[2], &min[2], &accumlate[2], &record[i].parm[bw].rx_gain) == 0)
			num[2] += 1;
		if(check_max_min_avg(&max[3], &min[3], &accumlate[3], &record[i].parm[bw].rx_phase) == 0)
			num[3] += 1;
	}

	for(i=0; i<4; i++)
	{
		avg[i] = accumlate[i]/num[i];
		accumlate[i] = 0;
	}

	for(i=0; i<samples; i++)
	{
		check_deviation(record[i].parm[bw].tx_gain, &accumlate[0], avg[0]);
		check_deviation(record[i].parm[bw].tx_phase, &accumlate[1], avg[1]);
		check_deviation(record[i].parm[bw].rx_gain, &accumlate[2], avg[2]);
		check_deviation(record[i].parm[bw].rx_phase, &accumlate[3], avg[3]);
	}
	for(i=0; i<4; i++)
	{
		dev[i] = sqrt((accumlate[i] / samples));
	}

	RFC_DBG(RFC_DBG_ERR, "-------------------------------------------------------------------\n");
	RFC_DBG(RFC_DBG_ERR, " MAX       ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, max[0]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, max[1]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, max[2]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, max[3]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG(RFC_DBG_ERR, "\n");
	RFC_DBG(RFC_DBG_ERR, " MIN       ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, min[0]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, min[1]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, min[2]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, min[3]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG(RFC_DBG_ERR, "\n");
	RFC_DBG(RFC_DBG_ERR, " AVG       ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, avg[0]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, avg[1]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, avg[2]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, avg[3]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG(RFC_DBG_ERR, "\n");
	RFC_DBG(RFC_DBG_ERR, " DEV       ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, dev[0]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, dev[1]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG_DOUBLE(RFC_DBG_ERR, dev[2]); RFC_DBG(RFC_DBG_ERR, "     "); RFC_DBG_DOUBLE(RFC_DBG_ERR, dev[3]); RFC_DBG(RFC_DBG_ERR, "     ");
	RFC_DBG(RFC_DBG_ERR, "\n");
	RFC_DBG(RFC_DBG_ERR, "-------------------------------------------------------------------\n");

#if 0
	if(mismatch)
	{
		output_rx_iq_mismatch_check_statistics_ht(samples, &mismatch->rx_iq_mismatch[bw]);
		output_tx_iq_mismatch_check_statistics_ht(samples, &mismatch->tx_iq_mismatch[bw]);
	}
#endif
	RFC_DBG(RFC_DBG_ERR, "----------------------------------------------------------------------------------------------\n");

}
#endif // RFC_DEBUG


#endif // CONFIG_RFC
