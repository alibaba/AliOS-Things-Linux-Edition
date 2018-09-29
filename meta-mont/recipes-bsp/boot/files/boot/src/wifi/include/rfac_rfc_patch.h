/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfac_rfc_patch.h
*   \brief
*   \author Montage
*/

#ifndef __RFAC_RFC_PATCH_H__
#define __RFAC_RFC_PATCH_H__

#define PATCH_START 0
#define PATCH_END 	1

#define PATCH_RF_NOISE_TONE 		BIT(0)
#define PATCH_IP301G_TX_FILTER 		BIT(1)
#define PATCH_TRANS_2FILTER_REGS	BIT(2)
#define PATCH_CLOCK_ISSUE 			BIT(3)

void rfac_rfc_patch_check(void);
u32 rf_noise_tone_patch(int sel, u32 rf_reg6);
void ip301g_tx_filter_patch(void);
void trans_2filter_regs_patch(struct rfc_cal_reg *parm);
void clock_issue_patch(int bw, u8 *filter_switch);

#endif // __RFAC_RFC_PATCH_H__
