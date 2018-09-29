/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file rfac_rfc_patch..c
*   \brief
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <ip301.h>
#include <rf.h>
#include <rfc_comm.h>
#include <panther_rf.h>
#include <rfac_rfc_patch.h>
#include <panther_dev.h>
static u8 patch_func_map=0;

void rfac_rfc_patch_check(void)
{
	u8 rf_ver = ldev->rf.chip_ver;
	
	patch_func_map |= PATCH_TRANS_2FILTER_REGS;

	if(rf_ver == RFCHIP_IP301_E)
		patch_func_map |= PATCH_RF_NOISE_TONE;
}
	
/* patch for rf noise tone effect, for IP301_G & IP301_E */
u32 rf_noise_tone_patch(int sel, u32 rf_reg6)
{
 	if(patch_func_map & PATCH_RF_NOISE_TONE)
	{
		if(sel == PATCH_START)
		{
			rf_reg6 = rf_read(0x6);
			rf_write(0x6, rf_reg6 | 0x40);
		}
		else // sel == PATCH_END
		{
			/* restore reg.6 */
			rf_write(0x6, rf_reg6);
		}
	}
	return rf_reg6;
}

/* patch the issue : 5e = 0 & 5a = 1 => rfc will get wrong result */
void trans_2filter_regs_patch(struct rfc_cal_reg *parm)
{
 	if(patch_func_map & PATCH_TRANS_2FILTER_REGS)
	{
		if(parm->phaseshifter_rx_alfa == 0)
			parm->filter_switch &= 0xfc;
		if(parm->phaseshifter_tx_alfa == 0)
			parm->filter_switch &= 0xcf;
	}
}

