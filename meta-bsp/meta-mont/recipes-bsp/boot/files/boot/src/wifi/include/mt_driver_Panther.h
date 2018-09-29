/*****************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd                                    */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2015 Montage Technology Group Limited. All Rights Reserved. */
/*****************************************************************************/
/* ---------------------------------------------------------------------------
 *
 * File:               mt_driver_Panther.h
 *
 * Current version:    0.00.01
 *
 * Description:        Header file of Panther driver.
 *
 *
 * Log:  Description             Version     Date            Author
 *        --------------------------------------------------------------------
 *        Create                 0.00.00     2015.11.11      YZ.Huang
 *        Modify                 0.00.01     2015.11.11      YZ.Huang
 *****************************************************************************/


#ifndef  _MT_FE_PANTHER_DRIVER_H
#define  _MT_FE_PANTHER_DRIVER_H


#ifdef __cplusplus
extern "C" {
#endif



#if 1
#define	U8	unsigned char								/* 8bit unsigned	*/
#define	S8	signed char									/* 8bit unsigned	*/
#define	U16	unsigned short								/* 16bit unsigned	*/
#define	S16	signed short								/* 16bit unsigned	*/
#define	U32	unsigned int								/* 32bit unsigned	*/
#define	S32	signed int									/* 16bit unsigned	*/
#define	DB	double
#else
typedef	unsigned char		U8;							/* 8bit unsigned	*/
typedef	unsigned char		S8;							/* 8bit unsigned	*/
typedef	unsigned short		U16;						/* 16bit unsigned	*/
typedef	signed short		S16;						/* 16bit unsigned	*/
typedef	unsigned int		U32;						/* 32bit unsigned	*/
typedef	signed int			S32;						/* 16bit unsigned	*/
typedef	double				DB;
#endif



#ifndef NULL
#define NULL    0
#endif

#ifndef BOOL
#define BOOL    int
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE    0
#endif



#define PANTHER_I2C_DEV_ADDR		0x68



typedef struct _MT_Device_Settings_Panther
{
	U8		bInitOK;			// Initialize status
	U8		ucDevAddr;			// I2C device address

	U32		ulLoFreqMHz;		// LO frequency
	U32		ulFVCOFreqMHz;		// FVCO frequency

	U8		iPLLMode;			// RF PLL mode
	U8		iTSMuxMode;			// TS MUX mode

	U8		bManualEn;			// Manual enable
	U8		bLoBpfBypassEn;		// LO BPF bypass enable

	U32		iVersionNumber;		// Version number
	U32		iVersionTime;		// Version time
} MT_DEVICE_SETTINGS_PANTHER, *MT_Panther_Handle;



BOOL DABUS_Get_Reg(U16 addr, U32 *data);
BOOL DABUS_Set_Reg(U16 addr, U32 data);
BOOL RBUS_Get_Reg(U32 addr, U32 *data);
BOOL RBUS_Set_Reg(U32 addr, U32 data);
BOOL PBUS_Get_Reg(U32 addr, U32 *data);
BOOL PBUS_Set_Reg(U32 addr, U32 data);



BOOL Panther_Get_Reg(MT_Panther_Handle handle, U32 addr, U32 *data);
BOOL Panther_Set_Reg(MT_Panther_Handle handle, U32 addr, U32 data);
U32 SetSeveralBits2Data(U32 target_data, U32 source_bits, U8 bit_high, U8 bit_low);
BOOL Panther_Get_RF_Reg(MT_Panther_Handle handle, U8 index, U32 *data);
BOOL Panther_Set_RF_Reg(MT_Panther_Handle handle, U8 index, U32 data);
BOOL Panther_Set_RF_Reg_Bits(MT_Panther_Handle handle, U8 index, U32 data, U8 bit_high, U8 bit_low);


BOOL mt_Panther_Init(MT_Panther_Handle handle);
BOOL mt_Panther_RF_set_pll(MT_Panther_Handle handle, U32 ulLoFreq);
void mt_Panther_Set_TX(MT_Panther_Handle handle);
void mt_Panther_Set_RX(MT_Panther_Handle handle);
void mt_Panther_set_TS_Mux_mode(MT_Panther_Handle handle, U8 iTsMuxIndex);
void mt_Panther_set_ABB(MT_Panther_Handle handle);


#ifdef __cplusplus
};
#endif

#endif

