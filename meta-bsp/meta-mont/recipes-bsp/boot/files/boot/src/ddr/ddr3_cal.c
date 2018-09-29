/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*
 *  NOTE:
 *        written by Li changjun Shanghai originally
 *        Modified by Cao Zhi
 *
 */
#include "ddr_config.h"

#ifdef CONFIG_USE_DDR3

#ifdef CONFIG_BOARD_WITH_CHIP

typedef signed char             s8;
typedef unsigned char           u8;
typedef signed short            s16;
typedef unsigned short          u16;
typedef signed long             s32;
typedef unsigned long           u32;

#define SW_TEST_START           0xbf5d0010
#define SW_TEST_COMPLETE        0xbf5d0014
#define SW_TEST_DATA0           0xbf5d0018

#define SDRAM_REG_BASE         (0xBF000000)       // supplied by Alvis Email May 25, 2017
#define	DDR3_CAL_BASE_ADDR     (0xBF000800)       // supplied by Louis Email 2017-5-25 

#ifndef CONFIG_FREQ666
#if 0
#define DDR_FREQ                800
#define DDR_CAS_LATENCY         11    // DDR3 800M = 11; DDR3 666M = 10
#define VERSION                "U-DDR3 800 20170614"
#else
#define DDR_FREQ                528
#define DDR_CAS_LATENCY         10    // DDR3 800M = 11; DDR3 666M = 10; DDR3 528MHz TBD
#define VERSION                "U-DDR3 528 20170614"
#endif
#else
// #define DDR_FREQ                531   // for debug only
#define DDR_FREQ                664
#define DDR_CAS_LATENCY         10    // DDR3 800M = 11; DDR3 666M = 10
#define VERSION                "O-DDR3 664 20170614"
#endif

#define ADDR_PWRUP             (0x6 << 2)
#define ADDR_PRE               (0x0 << 2)
#define ADDR_AR                (0x2 << 2)
#define ADDR_RL_ADD            (0xE << 2)
#define ADDR_TWR               (0x17 << 2)
#define ADDR_ZQCL              (0x3 << 2)
#define ADDR_MODE              (0x8 << 2)
#define ADDR_MRS               (0x1 << 2)


//#define	 DDR3_TCK_PS            (1000*1000 / 450)
//#define	 PS_C_DLY_CELL_STEP     (63)
#define	PS_F_DLY_CELL_STEP     (15) //for test!!!
#define	DQ_HOLD_MIN_PS         (100)
#define	DQ_SETUP_MIN_PS        (100)
#ifndef CONFIG_FREQ666
#define MRS1                   (0x04)   // 0x40:120ohm;0x44:40ohm; 0x04:60ohm
#define MRS3                   (0x00)
#else                          
#define MRS1                   (0x04)   // 0x40:120ohm;0x44:40ohm; 0x04:60ohm
#define MRS3                   (0x00)
#endif
#define RX90_SETUP_MARGIN      (200)
#define RX90_HOLD_MARGIN       (200)

#define T025_MAX               (65)
#define T025_MIN               (55)

#define REG32(addr)            (*((volatile u32 *)(addr)))
#define REG8(addr)             (*((volatile u8 *)(addr)))

#ifndef vc
#define addr_read(addr, data)  (*data = REG32(addr))
#define addr_write(addr, data) (REG32(addr) = (data))
#endif
#ifndef CONFIG_FREQ666
#if 0
#define DQ_ODT                 (3) //#2:120 Ohm // 800
#else
#define DQ_ODT                 (2) //#2:120 Ohm // 528
#endif
#else
#define DQ_ODT                 (2) //#2:120 Ohm
#endif
#ifdef UPG
u32 uart_downlaod_address  = 0xa0008000, jump_address = 0xa0008000, data_size = 0;
#endif

#if 0
#define R_UART0_BAUD            0xbff00000
#define R_UART0_DATA           (R_UART0_BAUD+0x08)
#define R_UART0_STATUS         (R_UART0_BAUD+0x0c)
#define TX_DONE                 0x40
                              
#define UART_BASE_ADDR         (0xbff00000)
#define R_UART_BAUD            (UART_BASE_ADDR + 0x0)
#define R_UART_CONCTL          (UART_BASE_ADDR + 0x4)
#define R_UART_DATA            (UART_BASE_ADDR + 0x8)
#define R_UART_STATUS          (UART_BASE_ADDR + 0xC)
#define M_UART_SR_RXC           0x80
#define M_UART_SR_TXC           0x40
#endif

#if 1   // used for panther, supplied by  Alvis Email May 25, 2017
#define UART_BASE               0xBF002900UL
#define UART_CLK               (120 * 1000 * 1000)
#define UART_TARGET_BAUD_RATE   115200

#define URBR                    0x00
#define URCS                    0x04
#define URCS_TF                (1<<3)
#define URCS_TB                (1<<0)
#define URCS_BRSHIFT            16
#define URBR_DTSHFT             24

#define REG_WRITE_32(addr, val) (*(volatile unsigned long *)(addr)) = ((unsigned long)(val))
#define REG_READ_32(addr)       (*((volatile u32 *)(addr)))

void __attribute__ ((noinline)) panther_putc(char c)
{
    while (URCS_TF & REG_READ_32(UART_BASE + URCS));

    REG_WRITE_32(UART_BASE + URBR, (((unsigned char)c) << URBR_DTSHFT));
}             

void uart_init(void)
{
    REG_WRITE_32(UART_BASE + URCS, ((UART_CLK/UART_TARGET_BAUD_RATE)<<URCS_BRSHIFT));
}
#endif

// #define  DISPLAY_DEBUG

#define DDR_CONDIF_CL   (DDR_CAS_LATENCY)
extern void jump_to_memory(u32);
//extern void jump_return();
u32 PS_C_DLY_CELL_STEP = 0,clk_sync_dq_dly = 0,val000c = 0,tCK_PS, txdqs_earlier[2], tx90_width[4];
u32 tCK_DIV2_PS,tCK_DIV4_PS,tCK_M3DIV4_PS,MRS2,MRS0,wl_all0_result[2],wl_all1_result[2];
u32 freq = DDR_FREQ, ddr_cl = DDR_CONDIF_CL, start_dly, end_dly, txdqs_dly, t025_code_cf = 0, txdq_earlier[4];
u32 tx90_left_boundary[4],tx90_right_boundary[4],rx90_width[4];

u32 g_crystal = 0, g_temp = 0;

static  u32    randx = 1;

char Ver[]=VERSION;

u32 m_rand(void)
{
	randx = randx*0x41C64E6D + 0x6073;
	//randx = randx*0x6C078965 + 0x1;

	return randx;
}

void turn_off_autoref()
{
	addr_write(SDRAM_REG_BASE + 0x0c, (val000c | (1<<8)));
}

#ifndef vc
static void  __attribute__ ((noinline)) Sleep(u32 a)
{
	volatile u32 i = 30 * a;
	while(i--);
}
#endif
#ifdef UPG
static u32 read_char(u8 *p_data, u32 timeout)
{
	u32 time = 0;
	while(1)
	{
		if((REG8((volatile u8 *)R_UART_STATUS) & M_UART_SR_RXC) != 0)
		{
			*p_data = REG8((volatile u8 *)R_UART_DATA);
			return 0;
		}

		if(time++ >= timeout)
			return 1;
		Sleep(1);
	}
	return 1;
}
static u32 uart_down_load()
{
	u32 len = 0;
	u8  ch, data_inf[12];
	u32 ret = 0;
	len = 0;
	while(1)
	{
		ret = read_char(&data_inf[len], 0xfffffffc);
		len++;
		if(len >= 12)
			break;
	}
	data_size = (u32)(data_inf[0]|(data_inf[1]<<8)|(data_inf[2]<<16)|(data_inf[3]<<24));
	uart_downlaod_address = 0xa0000000 | (u32)(data_inf[4]|(data_inf[5]<<8)|(data_inf[6]<<16)|(data_inf[7]<<24));
	jump_address = 0xa0000000 | (u32)(data_inf[8]|(data_inf[9]<<8)|(data_inf[10]<<16)|(data_inf[11]<<24));
	while(1)
	{
		ret = read_char(&ch, 0x20000);
		if (!ret)
		{
			*(u8 *)(uart_downlaod_address++) = ch;
			len++;
		}
		else
		{
			return len;
		}
	}
	return len;
}
#endif
//set serial bits to special value
void  __attribute__ ((noinline)) set_register_bits(u32 addr, u32 bit_offset, u32 bit_num, u32 value)
{
	u32 data = 0;

	addr_read(addr, &data);
	data = (data & (~(((1 << bit_num) - 1) << bit_offset))) | ((value&((1<<bit_num) - 1))<<bit_offset);
	addr_write(addr, data);
}

//just fixed setting for test now!!!!
void  set_clk_sync_sel(u32 i,u32 sel)
{
	if(i == 0)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 4, 2, sel);
	}
	else
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 18, 4, (sel & 0x3)|((sel & 0x3)<<2));
	}
}

u32 code2dly_cf(u32 code)
{
	return (((code >> 2) & 0x1f) * PS_C_DLY_CELL_STEP + (code & 0x03) * PS_F_DLY_CELL_STEP);
}

u32  do_cal_clk()
{
	// MC always send clock ref_clk_1t[1:0] to PHY.\n
	// clk_cal_en = 0 will enable clk_sync_ca to sample ref_clk_1t[1] and give result to clk_cal_result[1]\n
	// and enable clk_sync_dq to sample ref_clk_1t[0] and give result to clk_cal_result[0]\n

	u32 clk_cal_result_remap = 0, clk_cal_result;

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 11, 1, 0);
	Sleep(1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 11, 1, 1);
	#ifndef vc
	Sleep(10);
	#endif
	addr_read(DDR3_CAL_BASE_ADDR + 0x4c, &clk_cal_result);
	clk_cal_result= (clk_cal_result >> 18) & 0x03;
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 11, 1, 0);
	clk_cal_result_remap = ((clk_cal_result >> 1) & 1) + ((clk_cal_result & 1) << 1);

	return clk_cal_result_remap;
}

void set_clk_sync_ca_dly(u32 dly)
{
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 0, 4, dly >> 2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 22, 2, dly & 3);
	Sleep(1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 24, 2, dly & 3);
}

void set_clk_sync_dq_dly(u32 dly)
{
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 10, 4, dly >> 2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 26, 2, dly & 3);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 14, 4, dly >> 2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 28, 2, dly & 3);
}

void set_clk_sync_dly(u32 i,u32 dly)
{
	if(0 == i)
	{
		set_clk_sync_ca_dly(dly);
	}
	else
	{
		set_clk_sync_dq_dly(dly);
	}
}

u32 get_clk_sync_dq_dly()
{
	u32 data0,data1;

	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &data0);
	data0 = ((data0 >> 10) & 0x0f);
	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &data1);
	data1 = ((data1 >> 26) & 0x03);

	return ((data0 << 2) | data1);
}

void  set_clk_leveling(u32 lev)
{
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x44, 0, 12, (lev&0x3F)|((lev&0x3F)<<6));
}

void  default_timing_setting()
{
	//The default values make the following calibration life easy
	u32 cur_clk_sync_sel_dq = 0;
	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &cur_clk_sync_sel_dq);
	cur_clk_sync_sel_dq = (cur_clk_sync_sel_dq >> 18) & 0x03;

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 14, 2, cur_clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 30, 2, cur_clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 14, 2, cur_clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 30, 2, cur_clk_sync_sel_dq);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 14, 2, cur_clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 30, 2, cur_clk_sync_sel_dq);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 14, 4, 0);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 6, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 22, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 6, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 22, 1, 1);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 18, 2, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 6, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 22, 1, 1);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 0, 6, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 16, 6, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 0, 6, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 16, 6, 0);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 8, 6, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 24, 6, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 8, 6, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 24, 6, 0);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 0, 6, 4);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 16, 6, 4);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 8, 6, 4);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 24, 6, 4);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 20, 4, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 4, 3, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 11, 3, 0);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 17, 1, 0);
}

void __attribute__ ((noinline)) send_mrs(u32 i, u32 val)
{
	addr_write(SDRAM_REG_BASE + ADDR_MODE, ((i << 14) | val));
	addr_write(SDRAM_REG_BASE + ADDR_MRS, 1);
}

void  dram_init()
{
	addr_write(SDRAM_REG_BASE + 0x38, 0x70030000);
	set_register_bits(SDRAM_REG_BASE + 0x34, 4, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 0, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 8, 1, 1);

	// assert reset to SDRAM
	// reset go high to SDRAM
	addr_write(SDRAM_REG_BASE + ADDR_PWRUP, 0x10000);
	#ifndef vc
	Sleep(1000);
	#endif
	addr_write(SDRAM_REG_BASE + ADDR_PWRUP, 0x0);
	#ifndef vc
	Sleep(100);
	#endif
	//#issue MRS
	send_mrs(2,MRS2);// #MRS2 = 0x10
	send_mrs(3,MRS3);// #MRS3 = 0x0
	send_mrs(1,MRS1);// #MRS1 = 0x40

	if(freq > 666)
	{
		addr_write(SDRAM_REG_BASE + 0x5c, 0xc0000);
	}
	else
	{
		addr_write(SDRAM_REG_BASE + 0x5c, 0xa0000);
	}
	send_mrs(0,MRS0); //#MRS0 = 0xb60

	// Issue ZQCAL command bit[3] to start ZQ calibration
	// tFAW-2=28
	addr_write(SDRAM_REG_BASE + ADDR_ZQCL, 0x1c000008);

	if(freq > 666)
	{
		if(11 == ddr_cl)
			// addr_write(SDRAM_REG_BASE + 0x24, 0x040b1c0b);
			addr_write(SDRAM_REG_BASE + 0x24, 0x040b1c0b);
		else
			addr_write(SDRAM_REG_BASE + 0x24, 0x040a1c0a);
			
		// addr_write(SDRAM_REG_BASE + 0x28, 0x58000806);
		addr_write(SDRAM_REG_BASE + 0x28, 0x58000806);
		
		if(11 == ddr_cl)
			// addr_write(SDRAM_REG_BASE + 0x2c, 0x2704169e);
			addr_write(SDRAM_REG_BASE + 0x2c, 0x2704169e);
		else
			addr_write(SDRAM_REG_BASE + 0x2c, 0x2604169e);
	}
	else if(freq > 600)
	{
		if(9 == ddr_cl)
			addr_write(SDRAM_REG_BASE + 0x24, 0x03091809); //tRTP-2=5-2=3, tRP=10(9), tRAS=0x18, tRCD=9
		else
			addr_write(SDRAM_REG_BASE + 0x24, 0x030a180a);

		// addr_write(SDRAM_REG_BASE + 0x28, 0x49010705); //tRFC, tRTW, tWTR=6(4), tRRD=5(4)
		addr_write(SDRAM_REG_BASE + 0x28, 0x49020705); // 2015-8-12 lynn modify trtw = 2
		if(9 == ddr_cl)
			addr_write(SDRAM_REG_BASE + 0x2c, 0x21041410); //for 666 tRC=33,tMRD,tREFI
		else
			addr_write(SDRAM_REG_BASE + 0x2c, 0x22041410);
	}
	else
	{
		addr_write(SDRAM_REG_BASE + 0x24, 0x030a1708);
		// addr_write(SDRAM_REG_BASE + 0x28, 0x90010605);
		addr_write(SDRAM_REG_BASE + 0x28, 0x90020605); // 2015-8-12 lynn modify trtw = 2
		addr_write(SDRAM_REG_BASE + 0x2c, 0x21070fe6); //for 522
	}

	//#wait for tDLLK (512 tCK) and tZQinit (512 tCK) completed
	#ifndef vc
	Sleep(1000);
	#endif
	//#power up done
	addr_write(SDRAM_REG_BASE + ADDR_PWRUP, 1);
	turn_off_autoref();
}

void turn_on_autoref()
{
	addr_write(SDRAM_REG_BASE + 0x0c, val000c & (~(1<<8)));
}

void judge_nibble_fail(u32 *expect_data, u32 *real_data, u32 *return_data)
{
	// expect_data: [burst01,burst23,burst45,burst67] burst01: bit[31:16]/bit[15:0] dq[15:0];
	// real_data: [burst01,burst23,burst45,burst67]

	u32 rd_check_fail[4];
	u32 burst, n ;
	for(n = 0; n < 4; n++)
	{
		rd_check_fail[n] = 0;
		//printf("expect %x real %x",expect_data[n],real_data[n]);
	}
	for(burst = 0; burst < 8; burst++)
	{
		for(n = 0; n < 4; n++)
		{
			if(((expect_data[burst/2] >> (n*4)) & 0xf) != ((real_data[burst/2] >> (n*4) ) & 0xf))
				rd_check_fail[n] = 1;
			if(((expect_data[burst/2] >> (n*4+16)) & 0xf) != ((real_data[burst/2] >> (n*4+16) ) & 0xf))
				rd_check_fail[n] = 1;
		}
	}
	for(n = 0; n < 4; n++)
	{
		return_data[n] = rd_check_fail[n];
	}
}

u32 dly2code_cf(u32 dly)
{
	u32 c, f;
	c = dly / PS_C_DLY_CELL_STEP;
	f = (dly % PS_C_DLY_CELL_STEP) / PS_F_DLY_CELL_STEP;
	if(f >= 4)
		f = 3;
	return ((c << 2) | f);
}

void  mpr_rd(u32 *return_data)
{
	u32 data = 0, dq_4t[4];
	u32 expect_data[4];

	for(data = 0; data < 4; data++)
	{
		dq_4t[data] = 0;
		expect_data[data] = 0xffff0000;
	}

	addr_read(SDRAM_REG_BASE + 0x0c, &data);
	data |= (1 << 4);

	addr_write(SDRAM_REG_BASE + 0x0c, data);
	// wait 20 tCK
	#ifndef vc
	Sleep(1);
	#endif
	for(data = 0; data < 4; data++)
	{
		addr_read(SDRAM_REG_BASE + 0x40 + 4 * data, &dq_4t[data]);
	}
	judge_nibble_fail(expect_data, dq_4t, return_data);
}

u32 dly2lev(u32 dly,u32 earlier)
{
	u32 real_dly = 0, i = 0, t05 = 0, nt = 0, t025 = 0, leveling = 0;
	for(i = 0; i < 4; i++)
	{
		nt = i;
		if((dly - tCK_PS * i) >= (clk_sync_dq_dly + earlier + tCK_DIV4_PS) && (dly - tCK_PS * i) < (clk_sync_dq_dly + earlier + tCK_M3DIV4_PS))
		{
			real_dly = dly;
			t05 = 0;
			break;
		}
		else if((dly - tCK_DIV2_PS - tCK_PS * i) >= (clk_sync_dq_dly + earlier + tCK_DIV4_PS ) && (dly - tCK_PS * i - tCK_DIV2_PS) < (clk_sync_dq_dly + earlier + tCK_M3DIV4_PS))
		{
			if(dly > tCK_PS)
				real_dly = dly - tCK_PS;
			else
				real_dly = dly;
			t05 = 1;

			break;
		}
	}
	// if(real_dly == 0)
	//	 printf("error dly2lev for dly=%d, earlier=%d, clk_sync_dq_dly=%d, tCK_DIV4_PS=%d",dly,earlier,clk_sync_dq_dly,tCK_DIV4_PS);

	t025 = real_dly / tCK_DIV4_PS;
	leveling = dly2code_cf(real_dly - t025 * tCK_DIV4_PS);
	// printf("\nclk_sync_dq_dly = %d earlier = %d dly = %d real_dly = %d nt = %d t05 = %d t025 = %d leveling = %x",clk_sync_dq_dly,earlier,dly,real_dly,nt,t05,t025 & 3,leveling);

	return ((nt << 9) | (t05 << 8) | ((t025 & 0x03) << 6) | (leveling & 0x3f));
}

void set_txdqs_dly(u32 b,u32 txdqs_dly)
{
	u32 data,txdqs_leveling_frac,txi_en_nt_sel_dqs,txi_en_05t_sel_dqs,clk_txdqs_sel;

	data = dly2lev(txdqs_dly,code2dly_cf(txdqs_earlier[b]));
	txi_en_nt_sel_dqs = (data >> 9) & 0x01;
	txi_en_05t_sel_dqs = (data >> 8) & 0x01;
	clk_txdqs_sel = (data >> 6) & 0x03;
	txdqs_leveling_frac = data & 0x3f;

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 6+16*b, 1, txi_en_05t_sel_dqs);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 18+b, 1, txi_en_nt_sel_dqs);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 14+16*b, 2, clk_txdqs_sel);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 0+16*b, 6, txdqs_leveling_frac);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 8+16*b, 6, txdqs_leveling_frac);
}

void calc_period(u32 freq)
{
	//printf("freq = %d",freq);
	tCK_PS = 1000000/freq;
	tCK_DIV2_PS = 500000/freq;
	tCK_DIV4_PS = 250000/freq;
	tCK_M3DIV4_PS = 750000/freq;
	if(freq > 666)
	{
		MRS2 = 0x18;   // CAS write Latency (CWL)
		MRS0 = 0xd70;  // CL=11  // WR
		val000c = 0x1e000000;
	}
	else
	{
		MRS2 = 0x10;
		if(9 == ddr_cl)
		{
			MRS0 = 0xb50;
		}
		else
		{
			MRS0 = 0xb60;
		}
		val000c = 0x1c000000;
	}
}

void cal_wl_step()
{
	u32 i = 0, dq_raw = 0;

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 12, 1, 1);
	Sleep(1);
	for(i = 0; i < 2; i++)
	{
		wl_all0_result[i] = 1;
		wl_all1_result[i] = 1;
	}

	for(i=0; i<4; i++)
	{
		// wait 10 tCK
		Sleep(1);

		addr_read(DDR3_CAL_BASE_ADDR + 0x24, &dq_raw);
		//#dq0 = (dq_raw >> 7) & 1
		//#dq8 = (dq_raw >> 14) & 1
		//#logger.debug("DQ0 = %d, DQ8=%d"%(dq0,dq8))
		//printf("DQ=%x",dq_raw);
		//#dq_raw[7] is DDR3_DQ0

		if((dq_raw&0x80) != 0x00)
		{
			wl_all0_result[0] = 0;
		}

		if((dq_raw&0x80) != 0x80)
		{
			wl_all1_result[0] = 0;
		}

		// dq_raw[14] is DDR3_DQ8
		if(((dq_raw>>8)&0x40) != 0x00)
		{
			wl_all0_result[1] = 0;
		}

		if(((dq_raw>>8)&0x40) != 0x40)
		{
			wl_all1_result[1] = 0;
		}
	}
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 12, 1, 0);
}

void set_txdq_dly(u32 n, u32 txdq_dly)
{
	u32 txi_en_nt_sel,txi_en_05t_sel,clk_txdq_sel,txdq_leveling_frac, data;

	data = dly2lev(txdq_dly,code2dly_cf(txdq_earlier[n]));
	txi_en_nt_sel = (data >> 9) & 0x01;
	txi_en_05t_sel = (data >> 8) & 1;
	clk_txdq_sel = (data >> 6) & 0x03;
	txdq_leveling_frac = data & 0x3f;
	//printf("set_txdq_dly %x,%x,%x,%x",txi_en_nt_sel, txi_en_05t_sel,clk_txdq_sel,txdq_leveling_frac);

	if(n < 4)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 14+n, 1, txi_en_nt_sel);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+(n/2)*4, 14+(n%2)*16, 2, clk_txdq_sel);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+(n/2)*4, 6+(n%2)*16, 1, txi_en_05t_sel);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+(n/2)*4, 0+(n%2)*16, 6, txdq_leveling_frac);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+(n/2)*4, 8+(n%2)*16, 6, txdq_leveling_frac);
	}
}

void set_all_txdq_dly(u32 txdq_dly)
{
	u32 i = 0;
	for(i = 0; i < 4; i++)
	{
		set_txdq_dly(i,txdq_dly);
	}
	#ifndef vc
	Sleep(1);
	#endif
}
#if 0 // symphony
void cal_tx90_step(u32 *tx90_fail_2)
{
	u32 i,j;
	u32 data,addr,temp,fail_v;

	static  u32 randx = 1;

	fail_v = 0;
	for(j=0; j<32; j++)
	{
		addr = 0xa1000000+ j * 4;
		data = m_rand() ;
		addr_write(addr, data);
		addr_read(addr, &temp);

		for(i = 0; i < 4; i++)
		{
			if((data&(0xF<<(4*i)))!=(temp&(0xF<<(4*i)))||(data&(0xF<<(4*(i+4))))!=(temp&(0xF<<(4*(i+4)))))
			{
				fail_v = fail_v | (1 << i);
			}
			else
				fail_v = fail_v | (0 << i);
		}
		(*tx90_fail_2) = fail_v;
	}

	//printf("tx90_fail_2=%x",*tx90_fail_2);
}
#endif
#if 1 // tango
void cal_tx90_step(u32 *tx90_fail_2)
{
	u32 old_bit0, old_bit3, old_bit4,old_bit7, i,j;
	u32 new_bit11, new_12bits, new_LSB4, new_LSB8, data,addr,temp,fail_v;

	static u32 old_12bits = 0x5fa5a8a5;
	fail_v = 0;
	for(j=0; j<16; j++)
	{
		old_bit0 = old_12bits & 0x1;
		old_bit3 = (old_12bits & 0x8) >>3;
		old_bit4 = (old_12bits & 0x10)>>4;
		old_bit7 = (old_12bits & 0x80)>>7;
		new_bit11 = old_bit0 ^ old_bit3 ^ old_bit4 ^ old_bit7;
		new_12bits = (old_12bits>>1) + (new_bit11<<11);
		new_LSB4 = 0xF & new_12bits;
		new_LSB8 = 0xFF & new_12bits;
		addr = 0xa1000000+(((new_LSB4<<24)+(new_12bits<<12)+(new_12bits & 0xFFC))&0xFFFFFF) / 4 * 4;
		data = (new_LSB8<<24)+(new_12bits<<12)+new_12bits;
		addr_write(addr, data);
		addr_read(addr, &temp);

		for(i = 0; i < 4; i++)
		{
			if((data&(0xF<<(4*i)))!=(temp&(0xF<<(4*i)))||(data&(0xF<<(4*(i+4))))!=(temp&(0xF<<(4*(i+4)))))
			{
				fail_v = fail_v | (1 << i);
			}
			else
				fail_v = fail_v | (0 << i);
		}
		(*tx90_fail_2) = fail_v;
		old_12bits = new_12bits;
	}

	//printf("tx90_fail_2=%x",*tx90_fail_2);
}
#endif
void get_max_tx90_window(u32 *_txdq_dly, u32 *tx90_failed, u32 num)
{
	u32 cur_left_boundary[4],left_boundary_found[4], i, n;

	for(i=0; i< 4; i++)
	{
		tx90_left_boundary[i] = 0;
		cur_left_boundary[i] = 0;
		tx90_right_boundary[i] = 0;
		left_boundary_found[i] = 0;
		tx90_width[i] = 0;
	}

	for(i =0; i< num; i++)
	{
		for(n = 0; n < 4; n++)
		{
			if(((tx90_failed[i]>>n)&1) == 0 && left_boundary_found[n] == 0)
			{
				left_boundary_found[n] = 1;
				cur_left_boundary[n] = _txdq_dly[i];
				
				#ifdef vc
				printf("Cycle=%-2d nibble=%d ,find L boundary=%d\n",i,n,_txdq_dly[i]);
				#endif
			}
			if(((tx90_failed[i]>>n)&1) == 1 && left_boundary_found[n] == 1)
			{
				left_boundary_found[n] = 0;
				
				#ifdef vc
				printf("Cycle=%-2d nibble=%d ,find R boundary=%d\n",i,n,_txdq_dly[i]);
				#endif
				
				if(_txdq_dly[i] - cur_left_boundary[n] > tx90_width[n])
				{
					#ifdef vc
					// printf("rx90_width[%d]=%d i[0]=%d left_boundary[%d] = %d\n",n,rx90_width[n],i,n,tx90_left_boundary[n]);
					#endif
					
					tx90_left_boundary[n] = cur_left_boundary[n];
					tx90_right_boundary[n] = _txdq_dly[i];
					tx90_width[n] = tx90_right_boundary[n] - tx90_left_boundary[n];
				}
			}
		}
	}
	#ifdef vc
	for(i =0; i< 4; i++)
		printf("-tx90[%d]:-%d-----%d---\n", i, tx90_left_boundary[i], tx90_right_boundary[i]);
	#endif
}

void cal_tx90()
{
	// Sweep all range of tx90 and then calculate the best point

	u32 txdq_dly[1][150];
	u32 tx90_fail[1][150];
	u32 i, nt, n, data;

	turn_off_autoref();
	
	#ifdef vc
	printf("======================================\n");
	printf("======================================\n");
	printf("TX90 calibration start......\n");
	#endif
	
	data = txdq_earlier[0] > txdq_earlier[1] ? txdq_earlier[0] : txdq_earlier[1];
	data = data > txdq_earlier[2] ? data : txdq_earlier[2];
	data = data > txdq_earlier[3] ? data : txdq_earlier[3];
	start_dly = clk_sync_dq_dly + code2dly_cf(data) + tCK_DIV4_PS;

	end_dly = start_dly + tCK_PS;

	#ifdef vc
	printf("(dq)start_dly=%d,(dq)end_dly=%d,data=%d\n",start_dly, end_dly,data);
	#endif
	
	// just config the single wr-rd. do redo only later
	#if 1
	for(i = 0; i<150; i++)
	{
		txdq_dly[0][i] = 0;
		tx90_fail[0][i] = 1;
	}
	#endif

	for(nt = 0; nt < 1; nt++)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 18, 2, (nt&0x1) | ((nt&0x1) << 1));
		i = 0;
		
		for(data = start_dly; data < end_dly; data = data + PS_F_DLY_CELL_STEP)
		{
			set_all_txdq_dly(data);
			txdq_dly[nt][i] = data;
			cal_tx90_step(&(tx90_fail[nt][i]));
			
			#ifdef vc
			if(tx90_fail[nt][i])
				printf("the cycle:%3d, find different data=0x%x\n",i,tx90_fail[nt][i]);
			#endif	
			
			i++;
		}
		
		get_max_tx90_window(txdq_dly[nt], tx90_fail[nt], i);

		if(tx90_width[0] > 150 && tx90_width[1] > 150 && tx90_width[2] > 150 && tx90_width[3] > 150)
			break;
	}

	for(n=0; n < 4; n++)
	{
		#ifdef vc
		printf("==============================\n");
		#endif
		
		for(nt=0; nt < 1; nt++)
		{
			// get_max_tx90_window(txdq_dly[nt], tx90_fail[nt], i);

			if(tx90_right_boundary[n]-tx90_left_boundary[n] > 150)
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 18 + (n/2), 1,nt);
				break;
			}
		}
		if(tx90_width[n] == 0)
		{
			panther_putc('G');
			panther_putc(0x30+n);
			#ifdef vc
			printf("nibble %d tx90 calibration fail\n",n);
			#endif
		}
		else
		{
			set_txdq_dly(n,(tx90_right_boundary[n]+tx90_left_boundary[n])/2);
			
			#ifdef vc
			printf("nibble %d tx90 left = %d pS right = %d pS width = %d pS\n",n,tx90_left_boundary[n],tx90_right_boundary[n],tx90_right_boundary[n]-tx90_left_boundary[n]);
			#endif
		}
	}
	turn_on_autoref();
}

u32 cal_t025()
{
	u32 t025_code_cfs[16], ttu_flag = 0, i = 0, j = 0, data = 0;

	for(i = 0; i < 16; i++)
	{
		t025_code_cfs[i] = 0;
	}

	// put b0n0 and b0n1 T/4 later than b1n1 and b1n0
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 0, 1, 1);
	for(j = 0; j < 4; j++)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 14, 2, j+1);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 30, 2, j+1);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 14, 2, j);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 30, 2, j);

		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 0, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 16, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 0, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 16, 6, 0);

		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 8, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 24, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 8, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 24, 6, 0);
		// sweep the code of b1n1 and b1n0 (including coarse and fine codes) to let b0n0 and b0n1 sample delayed b1n1 and b1n0 to find not 1
		// mt_console_printf("b0n0 and b0n1 sample b1n1 and b1n0 to find not 1\n");
		#ifndef vc
		Sleep(1);
		#endif
		for(i = 0; i < 0x3f; i++)
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 0, 6, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 16, 6, i);

			set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 8, 6, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 24, 6, i);
			#ifndef vc
			Sleep(1);
			#endif

			addr_read(DDR3_CAL_BASE_ADDR + 0x28, &ttu_flag);
			ttu_flag = (ttu_flag >> 1) & 0x3ff; //bit 1--10
			if((((ttu_flag>>3)&1) != 1) && (t025_code_cfs[j*4+1] == 0))
				t025_code_cfs[j*4+1] = i;
			if((((ttu_flag>>8)&1) != 1) && (t025_code_cfs[j*4+3] == 0))
				t025_code_cfs[j*4+3] = i;
		}

		// #put b0n0 and b0n1 T/4 earlier than b1n1 and b1n0
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 0, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 16, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 0, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 16, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 8, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 24, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 8, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 24, 6, 0);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 14, 2, j);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 30, 2, j);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 14, 2, j+1);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 30, 2, j+1);

		//#sweep the code of b0n0 and b0n1 (including coarse and fine codes) to let b0n0 and b0n1 sample delayed b1n1 and b1n0 to find 1
		//printf("b0n0 and b0n1 sample b1n1 and b1n0 to find 1\n");
		for(i = 0; i < 0x3f; i++)
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 0, 6, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 8, 6, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 16, 6, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 24, 6, i);
			#ifndef vc
			Sleep(1);
			#endif

			addr_read(DDR3_CAL_BASE_ADDR + 0x28, &ttu_flag);
			ttu_flag = (ttu_flag >> 1) & 0x3ff; //bit 1--10
			if(((ttu_flag >> 3) & 1) == 1 && t025_code_cfs[j*4+2] == 0)
				t025_code_cfs[j*4+2] = i;
			if(((ttu_flag >> 8) & 1) == 1 && t025_code_cfs[j*4+0] == 0)
				t025_code_cfs[j*4+0] = i;
		}
	}

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 0, 1, 0);

	// calculate the t025 value
	data = 0;
	for(i=0; i < 16;i++)
	{
		data += t025_code_cfs[i];
	}
	t025_code_cf = data/16;

	//#calculate the coarse dly and fine dly
	//PS_C_DLY_CELL_STEP = tCK_PS * 4 / (sum(t025_code_cfs) >> 2)
	if(data>>2)
	{
		PS_C_DLY_CELL_STEP = (tCK_PS*4)/ (data >> 2);
	}
	else
	{
		PS_C_DLY_CELL_STEP = 100;
	}

	#ifdef vc
	printf("t025_code_cf(dec) = %d,PS_C_DLY_CELL_STEP(dec) = %d \n" ,t025_code_cf,PS_C_DLY_CELL_STEP);
	#else
	#ifdef DISPLAY_DEBUG
	sram_printk1("t025_code_cf(dec) = %d,PS_C_DLY_CELL_STEP(dec) = %d \n" ,t025_code_cf,PS_C_DLY_CELL_STEP);
	#endif
	#endif

	//#PS_F_DLY_CELL_STEP = ( tCK_DIV4_PS % (t025_code_cf >> 2) ) / (t025_code_cf & 3)
	//#Just for code debug
	//PS_F_DLY_CELL_STEP = PS_F_DLY_CELL_STEP;//15;
	//printf("PS_F_DLY_CELL_STEP = 0x%x", PS_C_DLY_CELL_STEP);
	//printf("T025 calibration complete successfully!\n");

	return PS_C_DLY_CELL_STEP;
}

void set_ddr_freq(u32 freq)
{
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 4, 7, (freq / (40 / 5))&0x7f);
}

void cal_regu_w_t025()
{
	u32 tCK_PS_temp,j=0;
	s32 i=0;
	// cal regulator with t025 calibration for CONCERTO
	// ps_coarse locate in [60,70] is good voltage
	
	tCK_PS_temp = tCK_PS;
	set_ddr_freq(664);
	#ifndef vc
	Sleep(10000);
	#endif
	tCK_PS = 1500;

	for(i = 0; i < 4; i++)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x58, 0, 2, i);
		
		#ifndef vc
		Sleep(10000);
		#endif
		
		#ifdef vc
		printf(" i=%d   ",i);
		#endif
		
		cal_t025();
		if(PS_C_DLY_CELL_STEP < T025_MIN)
		{
			break;
		}
	}

	for(j = 4; ; j--)
	{
		if(j>12)
			j=12;
		if(j==6)
			break;
		
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x50, 5, 4, j);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x50, 1, 4, j);
		
		#ifndef vc
		Sleep(10000);
		#endif
		
		#ifdef vc
		printf(" i=%d j=%d   ",i,j);
		#endif
		
		cal_t025();
		if(PS_C_DLY_CELL_STEP > T025_MIN)
		{
			break;
		}
	}
	tCK_PS = tCK_PS_temp;
}

void ddr3_init_chip_temp()
{
	u32 n,i,j,k,data = 0,temp = 0, rd_check_fail[4], ttu_flag = 0, crystal = 40;
	u32 cur_clk_sync_sel, clk_cal_result, clk_sync_ca_dly = 0;
	u32 clk_sync_sel_dq, init_clk_sync_dly, txdqs_earlier_found, cur_clk_txdqs_dly, txdq_earlier_found;
	u32 cur_clk_sync_sel_dq, rl_0_found, rl_1_found, rl_first_1_found, rl_0_cnt, cur_rxi_en_nt_sel;
	u32 cur_rxi_en_05t_sel, cur_clk_rx_sel,rl_result_high,rl_result_low, cur_clk_txdq_dly;
	u32 wr_rd_fail[32], left_boundary_found[4],left_boundary_coarse[4],left_boundary_fine[4];
	u32 right_boundary_fine[4],cur_left_boundary_coarse[4],right_boundary_coarse[4], right_boundary_found[4];
	u32 wl_complete[2],wl_all0_found[2],wl_all0_cnt[2],wl_start_found[2],edge_start[2],edge_end[2];

	for(i = 0; i < 4; i++)
	{
		rd_check_fail[i] = 0;
		txdq_earlier[i] = 0;
	}
	for(i = 0; i < 2; i++)
	{
		txdqs_earlier[i] = 0;
	}
	// disp_status();
	#ifdef DISPLAY_DEBUG
	sram_printk1("\nVer:%s\n",Ver);
	#else
	panther_putc(Ver[0]);
	#endif


	set_register_bits(DDR3_CAL_BASE_ADDR + 0x58, 0, 2, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x50, 5, 4, 4);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x50, 1, 4, 4);
	

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 20, (1+1), 1 | (1 << 1));
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 12, (1+1+5), 0 | (0<<1) | (0<<2));
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 0, (3+1), 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 11, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 27, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x58, 4, 2, 3);
	
	set_ddr_freq(freq);
	calc_period(freq);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x04, 5, 1, 1);
	Sleep(1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x04, 6, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x2c, 0, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x04, 1, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x04, 0, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x04, 3, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x04, 2, 1, 1);

	//ungate_txclk()
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x2c, 1, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x2c, 2, 1, 1);

	//default_setting()
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 29, 2, 2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 21, 2, 2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 13, 2, 2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 5, 2, 2);
	if(freq>666)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 8, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 0, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 8, 5, 0x8);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 0, 5, 0x8);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 24, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 16, 5, 0x10);
	}
	else
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 8, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 0, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 8, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 0, 5, 0x10);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 24, 5, 0x08);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x48, 16, 5, 0x08);
	}
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 24, 5, DQ_ODT);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 16, 5, DQ_ODT);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 13, 2, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 5, 2, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 14, 1, 1);

	addr_read(DDR3_CAL_BASE_ADDR + 0x28, &data);

	//////////////modify 2014-03-31  start ////////////////
	// hold_low_n = 0 will force DDR3_RESETN, DDR3_CKE(DDR2_ODT) and DDR3_RASN(DDR2_CKE) as 0

	cal_regu_w_t025();
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x5c, 4, 7, (freq / (crystal / 5))&0x7f);
	#ifndef vc
	Sleep(10000);
	#endif
	cal_t025();


	//////////////modify 2014-03-31  end  ////////////////

	/*
	The delay line contains nT/4 selector and fine tune delay cells \n
	delay = nT/4 + coarse_delay_p_step * coarse_code + fine_delay_p_step * fine_code \n
	\n
	MC send all ca signals and ref_clk_1t[1] with same phase to PHY \n
	and send all data signals and ref_clk_1t[0] with same phase to PHY \n
	PHY use clk_sync_ca to sample ca signals and clk_sync_dq to sample data signals \n
	clk_sync_sel[0] and clk_sync_dly[0] control the phase of clk_sync_ca \n
	clk_sync_sel[1] and clk_sync_dly[1] control the phase of clk_sync_ca \n
	clk_sync_res[1] present the clk_sync_ca sample ref_clk_1t[1] result \n
	clk_sync_res[0] present the clk_sync_dq sample ref_clk_1t[0] result \n
	\n
	This function will put the clk_sync_ca rising edge to T/2 after the rising edge of ref_clk_1t[1] \n
	and put the clk_sync_dq rising edge to T/2 after the rising edge of ref_clk_1t[0] \n
	*/
	#ifdef vc
	printf("======================================\n");
	printf("======================================\n");
	printf("clk_sync calibration start......\n");
	#endif
	set_clk_sync_ca_dly(0);
	set_clk_sync_dq_dly(0);
	for(n=0; n < 2; n++)
	{
		// Sweep nT/4 to find the sample result change from 0 to 1
		k = 0;
		data = 0;
		temp = 0;

		for(j = 0; j < 8; j++)
		{
			cur_clk_sync_sel = j&0x3;
			set_clk_sync_sel(n,cur_clk_sync_sel);
			for(i = 0; i < t025_code_cf; i++)
			{
				set_clk_sync_dly(n,i);
				#ifndef vc
				Sleep(1);
				#endif
				clk_cal_result = do_cal_clk();
				#ifdef vc
				printf("clk_sync_dly[%d] = %d,clk_cal_result=%d\n",n,i,clk_cal_result);
				#endif

				if(k == 0 && ((clk_cal_result >> n) & 1) == 0)
				{
					if(temp > 3)
					{
						k = 1;
						temp = 0;
					}
					temp += 1;
				}
				if(k == 1 && ((clk_cal_result >> n) & 1) == 1)
				{
					if(temp > (n==0?3:3))
					{
						data = 1;
						temp = 0;
						#ifdef DISPLAY_DEBUG 
						sram_printk1(">>sync_dly %d %d\n", i, cur_clk_sync_sel);
						#endif
						#ifdef vc
						printf("clk_sync_sel[%d] = %d found 0 and then 1 \n", n,i);
						#endif
						break;
					}
					temp += 1;
				}
				if(k == 0 && ((clk_cal_result >> n) & 1) == 1)
				{
					temp = 0;
				}
			}

			if(data)
			{
				break;
			}
		}
		if(!data)
		{
			// printf("clk_sync_cal fail for %d",n);
			panther_putc('B');
			panther_putc(0x30+n);
			#if 0
			// sweep the delay code to find the exact rising edge of ref_clk_1t
			data = 0;
			cur_clk_sync_sel -= 1;
			set_clk_sync_sel(n,cur_clk_sync_sel);
			for(i = 0; i < t025_code_cf; i++)
			{
				set_clk_sync_dly(n,i);
				clk_cal_result = do_cal_clk();
				if(((clk_cal_result >> n) & 1) == 1)
				{
					data = 1;
					printf("clk_sync_dly[%d] = %d found 1 \n", n,i);
					break;
				}
			}
			if(!data)
			{
				printf("clk_sync_%d_cal code sweep fail\n", n);
				break;
			}
			if(data)
			{
				printf("clk_sync_cal fail for %d",n);
			}
			#endif
		}
		else
		{
			// put the clk_sync to the T/2 after the rising edge of ref_clk_1t
			cur_clk_sync_sel += 2;
			set_clk_sync_sel(n,cur_clk_sync_sel);
		}
	}

	//#put the clk_ca T/2 after the clk_sync
	addr_read(DDR3_CAL_BASE_ADDR + 0x4c, &data);
	data = (data >> 4) & 0x03;
	if(freq>666)
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 6, 2, (data + 1)&0x3);
	else
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 6, 2, (data + 1)&0x3);
	//printf("clk_sync calibration complete \n");

	//calc_clk_sync_dq_dly()
	data = get_clk_sync_dq_dly();
	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &temp);
	temp = (temp >> 18) & 0x3;
	clk_sync_dq_dly = temp * tCK_DIV4_PS + code2dly_cf(data);

	addr_read(DDR3_CAL_BASE_ADDR + 0x4c, &data);
	data = data & 0xF; //(0--3)
	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &temp);
	temp = (temp >> 22) & 0x3;
	clk_sync_ca_dly = (data << 2) | temp;

	if(freq>666)
	{
		temp=10;
		temp=(temp+clk_sync_ca_dly>63)?63:(temp+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 0,  6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 6,  6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 12, 6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 18, 6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 24, 6, temp);

		data=16;
		data=(data+clk_sync_ca_dly>63)?63:(data+clk_sync_ca_dly);
		set_clk_leveling(data);
		#ifdef DISPLAY_DEBUG 
		sram_printk1("clk_%d,ca_%d\n",data-clk_sync_ca_dly,temp-clk_sync_ca_dly);
		#endif
		// set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 0, 2, 3);
	}
	else if(freq>600)
	{
		temp=10;
		temp=(temp+clk_sync_ca_dly>63)?63:(temp+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 0,  6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 6,  6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 12, 6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 18, 6, temp);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 24, 6, temp);

		data=16;
		data=(data+clk_sync_ca_dly>63)?63:(data+clk_sync_ca_dly);
		set_clk_leveling(data);
		#ifdef DISPLAY_DEBUG 
		sram_printk1("clk_%d,ca_%d\n",data-clk_sync_ca_dly,temp-clk_sync_ca_dly);
		#endif
		// set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 0, 2, 3);
	}
	else
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 0,  6, 10+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 6,  6, 10+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 12, 6, 10+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 18, 6, 10+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 24, 6, 10+clk_sync_ca_dly);
		set_clk_leveling(0+clk_sync_ca_dly);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 0, 2, 3);
	}

	default_timing_setting();
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x2c, 0, 1, 0);
	#ifndef vc
	Sleep(10);
	#endif
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x2c, 0, 1, 1);
	#ifndef vc
	Sleep(10);
	#endif

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 17, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 4, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 5, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 0, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 8, 1, 1);
	set_register_bits(SDRAM_REG_BASE + 0x34, 12, 1, 0); // panther modified by lynn email 6/30/2017 10:26:13 AM
	set_register_bits(SDRAM_REG_BASE + 0x34, 17, 1, 1); // panther modified by qinghai email Thu 5/11/2017
	dram_init();
	if(freq > 666)
	{
		set_register_bits(SDRAM_REG_BASE + 0x38, 16, 3, 6);
	}
	else
	{
		set_register_bits(SDRAM_REG_BASE + 0x38, 16, 3, 5);
	}

	//cal_ecode
	/*
	ecode: earlier code. \n
	PHY use txclk_dq and txclk_dqs to sample WR_EN but layout can't balance txclk and WR_EN well. \n
	The real delay of WR_EN is larger than txclk\n
	When adjust the phase of txclk, in order to avoid setup/hold issue, we need to know the phase \n
	relationship of WR_EN and txclk.\n
	\n
	This function can get the relationship with ttu.\n
	*/

	#ifdef vc
	printf("======================================\n");
	printf("======================================\n");
	printf("Earlier code calibration start......\n");
	#endif

	//#Set the phases of clk_txdq and clk_txdqs before as clk_sync_dq
	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &temp);
	clk_sync_sel_dq = (temp >> 18) & 0x3;

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 14, 2, clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x08, 30, 2, clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 14, 2, clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x0c, 30, 2, clk_sync_sel_dq);

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 14, 2, clk_sync_sel_dq);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 30, 2, clk_sync_sel_dq);

	//#ungate_txclk()
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 0, 1, 1);
	init_clk_sync_dly = get_clk_sync_dq_dly();
	for(i = 0; i < 2; i++)
	{
		txdqs_earlier_found = 0;
		for(cur_clk_txdqs_dly=init_clk_sync_dly; cur_clk_txdqs_dly<init_clk_sync_dly+t025_code_cf*3;cur_clk_txdqs_dly++)
		{
			// printf("cur_clk_txdqs_dly = %d",cur_clk_txdqs_dly);
			// delay cells code reach the maximum value
			if(cur_clk_txdqs_dly > 0x3f)
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 0+i*16, 6, cur_clk_txdqs_dly - t025_code_cf);
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 8+i*16, 6, cur_clk_txdqs_dly - t025_code_cf);
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 14+i*16, 2, clk_sync_sel_dq + 1);
			}
			else
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 0+i*16, 6, cur_clk_txdqs_dly);
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x10, 8+i*16, 6, cur_clk_txdqs_dly);
			}

			addr_write(0xa0000000, 0x55aa55aa);
			// wait 5 tCK
			#ifndef vc
			Sleep(1);
			#endif

			addr_read(DDR3_CAL_BASE_ADDR + 0x28, &ttu_flag);
			//printf("leveling =%d ttu_flag bit3 = %d, bit8 = %d", i, (ttu_flag>>3)&1, (ttu_flag>>8)&1);
			ttu_flag = (ttu_flag >> 1) & 0x3ff; //bit 1--10
			if(0 == ((ttu_flag >> (i*5+4)) & 1))
			{
				txdqs_earlier[i] = cur_clk_txdqs_dly - init_clk_sync_dly;
				txdqs_earlier_found = 1;
				#ifdef vc
				printf("txdqs_earlier_b%d = %d\n",i,cur_clk_txdqs_dly-init_clk_sync_dly);
				#endif
				break;
			}
		}
		if(!txdqs_earlier_found)
		{
			panther_putc('C');
			panther_putc(0x30+i);
			#ifdef vc
			printf("txdqs_earlier_code[%d] not found\n", i);
			#endif
		}
		

		for(n = 0; n < 2; n++)
		{
			txdq_earlier_found = 0;
			for(cur_clk_txdq_dly = init_clk_sync_dly; init_clk_sync_dly < init_clk_sync_dly + t025_code_cf * 2; cur_clk_txdq_dly++)
			{
				// printf("cur_clk_txdq_dly = %d", cur_clk_txdq_dly);
				// delay cells code reach the maximum value
				if(cur_clk_txdq_dly > 0x3f)
				{
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+((i*2+n)/2)*4, 0+((i*2+n)%2)*16, 6, cur_clk_txdq_dly - t025_code_cf);
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+((i*2+n)/2)*4, 8+((i*2+n)%2)*16, 6, cur_clk_txdq_dly - t025_code_cf);
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+((i*2+n)/2)*4, 14+((i*2+n)%2)*16, 2, clk_sync_sel_dq + 1);
				}
				else
				{
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+((i*2+n)/2)*4, 0+((i*2+n)%2)*16, 6, cur_clk_txdq_dly);
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x08+((i*2+n)/2)*4, 8+((i*2+n)%2)*16, 6, cur_clk_txdq_dly);
				}
				addr_write(0xa0000000, 0x55aa55aa);
				// wait 5 tCK
				#ifndef vc
				Sleep(1);
				#endif

				addr_read(DDR3_CAL_BASE_ADDR + 0x28, &ttu_flag);
				//printf("leveling2 =%d ttu_flag bit3 = %d, bit8 = %d", i, (ttu_flag>>3)&1, (ttu_flag>>8)&1);
				ttu_flag = (ttu_flag >> 1) & 0x3ff;
				if(0 == ((ttu_flag >> (i*5+n)) & 1))
				{
					txdq_earlier[i*2+n] = cur_clk_txdq_dly - init_clk_sync_dly;
					txdq_earlier_found = 1;
					#ifdef vc
					printf("txdq_earlier_b%dn%d = %d\n",i,n,cur_clk_txdq_dly-init_clk_sync_dly);
					#endif
					break;
				}
			}
			if(!txdq_earlier_found)
			{
				panther_putc('C');
				panther_putc(0x32+i*2+n);
				#ifdef vc
				printf("txdq_earlier_codeb%dn%d not found\n", i,n);
				#endif
			}
		}
	}
	// gate_txclk()
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 0, 0, 0);

	// This function put the rising edge of rd_en to the T/2 before the first dqs rising edge
	// If do dq_loopback test, set dq_loopback_test as 1

	#ifdef vc
	printf("======================================\n");
	printf("======================================\n");
	printf("Read leveling calibration start......\n");
	#endif

	turn_off_autoref();
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 16, 5, 0x2);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 24, 5, 0x04);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 2, 1, 1);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 31, 1, 1);
	send_mrs(3,MRS3|0x4); //MRS3 = 0x00

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 5, 2, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 12, 2, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 4, 1, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 11, 1, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 20, 2, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 22, 2, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 0, 4, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 7, 4, 0);

	addr_read(DDR3_CAL_BASE_ADDR + 0x40, &data);
	cur_clk_sync_sel_dq = (data >> 18) & 0x3;
	get_clk_sync_dq_dly();

	// sweep rd_en phase
	for(n = 0; n < 2; n++)
	{
		rl_0_found = 0;
		rl_1_found = 0;
		rl_first_1_found = 0;
		rl_0_cnt = 0;
		cur_rxi_en_nt_sel = 0;

		if(0 == n)
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 5, 2, cur_rxi_en_nt_sel);
		}
		else
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 12, 2, cur_rxi_en_nt_sel);
		}

		cur_rxi_en_05t_sel = 0 ;
		// for(i = 0; i < 12; i++)
		for(i = 3; i < 15; i++)
		{
			cur_clk_rx_sel = cur_clk_sync_sel_dq + i ;
			if(0 == n)
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 20, 2, cur_clk_rx_sel);
			}
			else
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 22, 2, cur_clk_rx_sel);
			}
			if(i%4 >= 1 && i%4 <= 2)
			{
				cur_rxi_en_05t_sel = 1;
			}
			else
			{
				if(cur_rxi_en_05t_sel == 1)
				{
					cur_rxi_en_nt_sel += 1;
					if(0 == n)
					{
						set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 5, 2, cur_rxi_en_nt_sel);
					}
					else
					{
						set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 12, 2, cur_rxi_en_nt_sel);
					}
				}
				cur_rxi_en_05t_sel = 0;
			}

			if(0 == n)
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 4, 1, cur_rxi_en_05t_sel);
			}
			else
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 11, 1, cur_rxi_en_05t_sel);
			}

			for(j = 0; j < (t025_code_cf >> 2)+1; j++)
			{
				if(0 == n)
				{
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 0, 4, j);
				}
				else
				{
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 7, 4, j);
				}
				#ifndef vc
				Sleep(1);
				#endif
				rl_result_high = 1;
				rl_result_low = 0;
				#ifdef vc
				printf("\nrl[%d] nt=%d 05t=%d 025t=%d leveling = %d   :",n,cur_rxi_en_nt_sel,cur_rxi_en_05t_sel & 1,i&0x3,j);
				#endif
				for(k=0; k<4; k++)
				{
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 13, 1, 1);
					#ifndef vc
					Sleep(1);
					#endif
					mpr_rd(rd_check_fail);

					// wait 20 tCK
					#ifndef vc
					Sleep(1);
					#endif
					addr_read(DDR3_CAL_BASE_ADDR + 0x4C, &data);
					data = (((data >> 20) & 0x03) >> n) & 1;
					//printf("rl_result[%d]=%d", n,data);
					rl_result_high &= data ;
					rl_result_low |= data;
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 13, 1, 0);
					#ifndef vc
					Sleep(1);
					#endif
				}
				if(rl_result_high == 1 && rl_first_1_found == 0)
				{
					rl_first_1_found = 1;
				}
				if(rl_result_low == 0 && rl_first_1_found == 1)
				{
					// go to the next step to avoid jitter error
					if(0 == n)
						set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 0, 4, j+2);
					else
						set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 7, 4, j+2);

					set_register_bits(DDR3_CAL_BASE_ADDR + 0x4c, 13, 1, 1);
					mpr_rd(rd_check_fail);
					// wait 20 tCK
					#ifndef vc
					Sleep(1);
					#endif
					addr_read(DDR3_CAL_BASE_ADDR + 0x4C, &data);
					data = (((data >> 20) & 0x03) >> n) & 1;
					if(data == 0 && rl_0_cnt > 6)
					{
						#ifdef vc
						printf("rl_0_found for b%d", n);
						#endif
						rl_0_found = 1;
					}
					#ifdef vc
					else
					{
						printf("rl_0_found counter = %d", rl_0_cnt);
					}
					#endif
					rl_0_cnt += 1;
				}
				if(rl_result_low == 1 && rl_0_found == 0 && rl_first_1_found == 1)
				{
					#ifdef vc
					printf("result at least one  1!!! ");
					#endif
					rl_0_cnt = 0;
				}
				if(rl_result_high == 1 && rl_0_found == 1 && rl_first_1_found == 1)
				{
					#ifdef vc
					printf("rl_1_found for b%d\n",n);
					#endif
					rl_1_found = 1;
					temp = j;
					break;
				}
			}
			if(rl_1_found)
				break;
		}
		if(!rl_1_found)
		{
			// printf("b%d rl calibration fail", n);
			panther_putc('D');
			panther_putc(0x30+n);
		}
		else
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 0+n*7, 4, temp);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 20+n*2, 2, cur_clk_rx_sel + 2);

			if(cur_rxi_en_05t_sel == 0)
			{
				if(cur_rxi_en_nt_sel == 0)
				{
					set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 17, 1, 1);
				}
				else
				{
					cur_rxi_en_nt_sel -= 1;
					if(0 == n)
					{
						set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 5, 2, cur_rxi_en_nt_sel);
					}
					else
					{
						set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 12, 2, cur_rxi_en_nt_sel);
					}
				}
			}
			if(0 == n)
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 4, 1, 1 - cur_rxi_en_05t_sel);
			}
			else
			{
				set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 11, 1, 1 - cur_rxi_en_05t_sel);
			}
			#ifdef vc
			printf("\nrl[%d] nt=%d 05t=%d 025t=%d leveling = %d\n",n,cur_rxi_en_nt_sel,(1-(cur_rxi_en_05t_sel & 1)),((cur_clk_rx_sel + 2)&0x3),temp);
			#endif
		}
	}

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 31, 1, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x40, 2, 1, 0);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 16, 5, DQ_ODT);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x20, 24, 5, DQ_ODT);

	send_mrs(3,MRS3&0xfffb);
	turn_on_autoref();

	// This function calibrate the phase relationship between DQS and DQ of read.
	// The target is to put DQS in the middle of DQ pass window.

	#ifdef vc
	printf("======================================\n");
	printf("======================================\n");
	printf("Read 90 degree calibration start......\n");
	#endif
	turn_off_autoref();
	send_mrs(3,MRS3|0x4);
	// sweep all the code and record the pass_fail result

	for(i=0; i< 32; i++)
	{
		wr_rd_fail[i] = 0;
	}
	for(i=0; i< 4; i++)
	{
		left_boundary_found[i] = 0;
		left_boundary_coarse[i] = 0;
		left_boundary_fine[i] = 0;
		right_boundary_fine[i] = 0;
		cur_left_boundary_coarse[i] = 0;
		right_boundary_coarse[i] = 0;
		right_boundary_found[i] = 0;
	}

	for(i=0; i<0x20; i++)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x18, 0, 7, (i<<2));
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x18, 16, 7, (i<<2));
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x1c, 0, 7, (i<<2));
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x1c, 16, 7, (i<<2));

		set_register_bits(DDR3_CAL_BASE_ADDR + 0x18, 8, 7, (i<<2));
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x18, 24, 7, (i<<2));
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x1c, 8, 7, (i<<2));
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x1c, 24, 7, (i<<2));
		Sleep(1);
		mpr_rd(rd_check_fail);
		wr_rd_fail[i] = rd_check_fail[0] | (rd_check_fail[1] << 1) | (rd_check_fail[2] << 2) | (rd_check_fail[3] << 3);
		//printf("rxdqs_dly90=%x rd_check_fail=%x ",i<<2,wr_rd_fail[i]);
	}

	// analyse the pass_fail result
	for(n = 0; n < 4; n++)
	{
		for(i = 0; i < 32; i++)
		{
			if(((wr_rd_fail[i] >> n)& 1) == 0 && left_boundary_found[n] == 0)
			{
				left_boundary_found[n] = 1;
				cur_left_boundary_coarse[n] = i;
				
				#ifdef vc
				printf("nibble=%d,cycle=%-2d,find L boundary\n",n,i);
				#endif
			}
			else if(((wr_rd_fail[i] >> n)& 1) == 1 && left_boundary_found[n] == 1)
			{
				left_boundary_found[n] = 0;
				
				#ifdef vc
				printf("nibble=%d,cycle=%-2d,find R boundary\n",n,i);
				#endif
				
				if(i - cur_left_boundary_coarse[n] > right_boundary_coarse[n] - left_boundary_coarse[n])
				{
					left_boundary_coarse[n] = cur_left_boundary_coarse[n];
					right_boundary_coarse[n] = i;
					
					#ifdef vc
					printf("boundary_coarse[%d] left=%d right=%d\n",n,cur_left_boundary_coarse[n],i);
					#endif
				}
			}
		}
		// the left_boundary found but right boundary not found
		if(left_boundary_found[n] && 0x20 - cur_left_boundary_coarse[n] > right_boundary_coarse[n] - left_boundary_coarse[n])
		{
			left_boundary_coarse[n] = cur_left_boundary_coarse[n];
			right_boundary_coarse[n] = 0x1f;
			left_boundary_found[n] = 0;
			
			#ifdef vc
			printf("boundary_coarse[%d] left=%d\n",n,cur_left_boundary_coarse[n]);
			#endif
		}
		
		#ifdef vc
		printf("-----------boundary_coarse[%d] left=%d right=%d\n",n,left_boundary_coarse[n],right_boundary_coarse[n]);
		#endif
	}

	#if 0
	//the for loop make the bin size out of size limit. Just use coarse value as fine value
	for(i = 0; i < 4; i++)
	{
		left_boundary_fine[i] = (left_boundary_coarse[i] << 2);
	}
	#else
	for(i = 0; i < 4; i++)
	{
		left_boundary_found[i] = 0;
	}

	// find the left boundary fine
	for(n=0; n < 4; n++)
	{
		//printf("to find the left boundary fine");
		for(i = (left_boundary_coarse[n] + 2) << 2 ; i != 0; i = i - 1)
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x18+(n/2)*4, 0+(n%2)*16, 7, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x18+(n/2)*4, 8+(n%2)*16, 7, i);
			// printf("rxdqs90_dly[%d]=%x",n,i);

			for(j=0;j<4;j++)
			{
				mpr_rd(rd_check_fail);

				// printf("wr_rd_fail[%d]=%d",n,rd_check_fail[n]);
				if(rd_check_fail[n])
				{
					left_boundary_found[n] = 1;
					break;
				}
			}
			if(left_boundary_found[n])
			{
				left_boundary_fine[n] = i;
				#ifdef vc
				printf("nibble%d left boundary fine found. It is %d\n",n,i);
				#endif
				break;
			}
		}
		// 0 still pass
		if(!left_boundary_found[n])
		{
			left_boundary_fine[n] = 0;
			#ifdef vc
			printf("nibble%d left boundary fine is 0\n",n);
			#endif
		}
	}
	#endif

	// find the right boundary fine
	#ifdef vc
	printf("to find the right boundary fine\n");
	#endif

	for(n = 0; n < 4; n++)
	{
		#ifdef vc
		printf("------------------------to find the right boundary fine nibble %d----------\n",n);
		#endif

		for(i =(right_boundary_coarse[n] - 2) << 2; i < 0x7f; i++)
		{
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x18+(n/2)*4, 0+(n%2)*16, 7, i);
			set_register_bits(DDR3_CAL_BASE_ADDR + 0x18+(n/2)*4, 8+(n%2)*16, 7, i);
			// printf("rxdqs90_dly[%d]=%d",n,i);
			for(j=0;j< 4;j++)
			{
				mpr_rd(rd_check_fail);
				// printf("wr_rd_fail[%d]=%d",n,rd_check_fail[n]);
				if(rd_check_fail[n])
				{
					right_boundary_found[n] = 1;
					break;
				}
			}
			if(right_boundary_found[n])
			{
				right_boundary_fine[n] = i;

				#ifdef vc
				printf("nibble%d right boundary fine found. It is %d\n",n,i);
				#endif

				break;
			}
		}

		// 0x7f still pass
		if (!right_boundary_found[n])
		{
			right_boundary_fine[n] = 0x7f;
			#ifdef vc
			printf("nibble%d right boundary fine is 0x7f\n",n);
			#endif
		}
	}

	// get the middle value
	for(n=0; n < 4;n++)
	{
		#ifdef vc
		printf("============================\n");
		#endif

		data = dly2code_cf( (code2dly_cf(left_boundary_fine[n]) + code2dly_cf(right_boundary_fine[n]))/2);
		temp = code2dly_cf(right_boundary_fine[n]) - code2dly_cf(left_boundary_fine[n]);

		set_register_bits(DDR3_CAL_BASE_ADDR + 0x18+(n/2)*4, 0+(n%2)*16, 7, data);
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x18+(n/2)*4, 8+(n%2)*16, 7, data);

		#ifdef vc
		printf("rx90[%d] left = %d right = %d middle=%d, width = %d ps\n",n,left_boundary_fine[n],right_boundary_fine[n],data,temp);
		#endif

		rx90_width[n] = temp;
	}
	//rx90_left_boundary = left_boundary_fine;  // for cycling only
	//rx90_right_boundary = right_boundary_fine; // for cycling only
	send_mrs(3,MRS3&0xfffb);
	turn_on_autoref();

	// This function align the DQS and Dram clock for write.
	#ifdef vc
	printf("======================================\n");
	printf("======================================\n");
	printf("Write Leveling calibration start......\n");
	#endif
	for(i=0;i<2;i++)
	{
		wl_complete[i] = 0;
		wl_all0_found[i] = 0;
		wl_all0_cnt[i] = 0;
		wl_start_found[i] = 0;
		edge_start[i] = 0;
		edge_end[i] = 0;
	}
	// step = PS_F_DLY_CELL_STEP
	// ungate_txclk()
	send_mrs(1,MRS1|0x80);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 14, 1, 1);
	if(txdqs_earlier[0] >= txdqs_earlier[1])
		data = txdqs_earlier[0];
	else
		data = txdqs_earlier[1];

	start_dly = clk_sync_dq_dly + tCK_DIV4_PS + code2dly_cf(data);
	end_dly = start_dly + tCK_PS * 2;

	#ifdef vc
	printf("start_dly=%d, end_dly=%d\n", start_dly, end_dly );
	#endif
	txdqs_dly = start_dly;

	turn_off_autoref();

	set_register_bits(DDR3_CAL_BASE_ADDR + 0x14, 18, 2, 0);

	addr_read(SDRAM_REG_BASE + 0x0c, &data);
	data |= (1<<16);
	addr_write(SDRAM_REG_BASE + 0x0c, data);

	for(txdqs_dly = start_dly; txdqs_dly < end_dly; txdqs_dly=txdqs_dly+PS_F_DLY_CELL_STEP)
	{
		if(!wl_complete[0])
		{
			//printf("txdqs_dly[0]=%d",txdqs_dly);
			set_txdqs_dly(0,txdqs_dly);
		}
		if(!wl_complete[1])
		{
			//printf("txdqs_dly[1]=%d",txdqs_dly);
			set_txdqs_dly(1,txdqs_dly);
		}
		
		cal_wl_step(wl_all1_result);
		for(j = 0; j < 2; j++)
		{
			if(wl_complete[j])
				continue;

			if(wl_all0_result[j] && !wl_all0_found[j])
			{
				wl_all0_cnt[j] += 1;
				if(wl_all0_cnt[j] > 10)
				{
					#ifdef vc
					printf("wl[%d] has found 0. It is %d\n",j,txdqs_dly);
					#endif
					wl_all0_found[j] = 1;
				}
			}
			if(wl_all0_found[j] && !wl_all0_result[j])
			{
				wl_start_found[j] = 1;
				edge_start[j] = txdqs_dly;
				#ifdef vc
				printf("wl[%d] start found. It is %d\n",j,txdqs_dly);
				#endif
			}
			if(wl_all1_result[j] && wl_start_found[j] && !wl_complete[j])
			{
				edge_end[j] = txdqs_dly;
				set_txdqs_dly(j,(edge_start[j] + edge_end[j])/2);
				wl_complete[j] = 1;
				#ifdef vc
				printf("wl[%d] end found. It is %d\n",j,txdqs_dly);
				printf("wl[%d] Middle found. It is %d\n",j,(edge_start[j] + edge_end[j])/2);
				#endif
			}
		}
		if(1 == wl_complete[0] && 1 == wl_complete[1])
			break;
	}
	if(!wl_complete[0])
	{
		panther_putc('F');
		panther_putc(0x30);
	}
	if(!wl_complete[1])
	{
		panther_putc('F');
		panther_putc(0x31);
	}

	send_mrs(1,MRS1&0xFF7F);
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x28, 14, 1, 0);
	//printf("Write Leveling calibration complete");

	addr_read(SDRAM_REG_BASE + 0x0c, &data);
	data &= (0xfffeffff);
	addr_write(SDRAM_REG_BASE + 0x0c, data);
	turn_on_autoref();

	#ifdef DISPLAY_DEBUG
	addr_read(DDR3_CAL_BASE_ADDR + 0x10, &data);
	sram_printk1("\n0x10=%x", data);
	addr_read(DDR3_CAL_BASE_ADDR + 0x14, &data);
	sram_printk1("\n0x14=%x", data);
	#endif

	cal_tx90();

	#ifdef DISPLAY_DEBUG
	addr_read(DDR3_CAL_BASE_ADDR + 0x0C, &data);
	sram_printk1("\n0x0C=%x", data);
	addr_read(DDR3_CAL_BASE_ADDR + 0x08, &data);
	sram_printk1("\n0x08=%x", data);
	addr_read(DDR3_CAL_BASE_ADDR + 0x14, &data);
	sram_printk1("\n0x14=%x", data);
	#endif

	//printf("");
	//printf("=========================== Calibration Summary");
	//printf("------------RX90 Width");

	#if 0
	for (i=0; i<1000;i++)
	{
		addr_write(0x80000000+i*4, i);
	}
	for (i=0; i<1000;i++)
	{
		addr_read (0x80000000+i*4,&temp);
		if(temp != i)
				sram_printk1 ("x%d %d\n", i,temp);
	}
	sram_printk1 ("1 done\n");
	#endif

	#ifdef vc
	for(i=0; i<4; i++)
	{
		printf("nibble %0d rx90_width = %d ps\n", i,rx90_width[i]);
	}
	for(i=0; i<4; i++)
	{
		printf("nibble %0d tx90_width = %d ps\n",i,tx90_width[i]);
	}
	#endif

	//gate_txclk
	set_register_bits(DDR3_CAL_BASE_ADDR + 0x2c, 1, 1, 0);
	#ifndef vc
	Sleep(100);
	#endif

	// addr_write(SDRAM_REG_BASE + 0x204, 0x201); // only for SOC test 2015-8-13

	#ifdef DISPLAY_DEBUG
	sram_printk1("\nR %d,%d,%d,%d", rx90_width[0], rx90_width[1], rx90_width[2], rx90_width[3]);
	sram_printk1("\nT %d,%d,%d,%d", tx90_width[0], tx90_width[1], tx90_width[2], tx90_width[3]);
	#else
	#if 1
	for(n = 0; n < 4; n++)
	{
		rx90_width[n] = rx90_width[n] / 100;

		if(rx90_width[n] > 9)
			 rx90_width[n] = rx90_width[n] - 9 + 0x41; //a,b,c...
		else
			rx90_width[n] = rx90_width[n] + 0x30; //0,1,2...

		panther_putc(rx90_width[n]);
	}
	for(n = 0; n < 4; n++)
	{
		tx90_width[n] = tx90_width[n] / 100;

		if(tx90_width[n] > 9)
			tx90_width[n] = tx90_width[n] - 9 + 0x41; //a,b,c...
		else
			tx90_width[n] = tx90_width[n] + 0x30; //0,1,2...

		panther_putc(tx90_width[n]);
	}
	#endif
	#endif
	
	#if 1
	addr_write(0xbd000000,512);
	Sleep(2);
	addr_write(0xad000000,256);
	Sleep(2);
	addr_write(0xa5000000,128);
	Sleep(2);
	addr_write(0xa1000000,64);
	Sleep(2);
	addr_read(0xbd000000,&temp);
	if(temp == 512)
	{
		g_crystal = 512;
		panther_putc('X');
		if(freq>666)
			// g_temp = 0xe0000806;
			g_temp = 0xf0020806;
		else
			g_temp = 0xc6020705;
	}
	else
	{
		addr_read(0xad000000,&temp);
		if(temp == 256)
		{
			g_crystal = 256;
			panther_putc('W');
			if(freq>666)
				// g_temp = 0x78000806;
				g_temp = 0x80000806;
			else
				g_temp = 0x6a010705;
		}
		else
		{
			addr_read(0xa5000000,&temp);
			if(temp == 128)
			{
				g_crystal = 128;
				panther_putc('V');
				// default size
				if(freq>666)
					// g_temp = 0x90020806;
					g_temp = 0x58000806;
				else
					g_temp = 0x49020705;
			}
			else
			{
				g_crystal = 64;
				panther_putc('U');
				if(freq>666)
					g_temp = 0x43000806;
				else
					g_temp = 0x3c010705;
			}
		}
		set_register_bits(SDRAM_REG_BASE + 0xc, 8, 1, 1);
		Sleep(10);
		addr_write(SDRAM_REG_BASE + 0x28, g_temp);
		Sleep(10);
		set_register_bits(SDRAM_REG_BASE + 0xc, 8, 1, 0);
	}

	#ifdef DISPLAY_DEBUG
	sram_printk1("\nfreq %dMHz;CL %d;Size %dMB\n", freq, ddr_cl,g_crystal);
	#else
	panther_putc('\n');
	#endif
	#endif

	//store DDR size from calibraton into GDMA register
	*(volatile u32*)DDR_SIZE_INFO_ADDR = g_crystal;

	#if 0
	for (i=0; i<100000;i++)
	{
		addr_write(0x80000000+i*4, i);
	}
	for (i=0; i<100000;i++)
	{
		addr_read (0x80000000+i*4,&temp);
		if(temp != i)
			sram_printk1 ("x%d %d\n", i,temp);
	}
	sram_printk1 ("2 done\n");
	#endif

	
	#ifdef UPG
	#if 1
	//printf("SEND_UPG");
	panther_putc('S');
	panther_putc('E');
	panther_putc('N');
	panther_putc('D');
	panther_putc('_');
	panther_putc('U');
	panther_putc('P');
	panther_putc('G');
	uart_down_load();
        
	jump_to_memory(jump_address);
	#else
	//just for test:
	//printf("start cp:");
	for(i = 0; i < 0x40000/4; i++)
	{
		addr_read(0xb000c100+4*i, &data);
		addr_write(0x81000000+4*i, data);
		addr_read(0x81000000+4*i, &temp);
		//if(temp!=data)
		//  printf("i=%d,exp=%x,read=%x",i,data,temp);
	}
	printf(" cp end");
	jump_to_memory(0x81000000);
	#endif
	while(1);
	#endif

}

#ifdef DISPLAY_DEBUG
void mtest_ca_rand(u32 start, u32 end, u32 grp)
{
	u32 addr, readback,bk_ca_grp_offset;
	u32 val, dly, randx_cp;
	int fail = 0,width=0;
	int th1 = -1, th=-1;
	int th2 = -1;

	bk_ca_grp_offset = reg32(DDR3_CAL_BASE_ADDR + 0x3c)>>grp&0x3f;

	for(dly=0;dly<64;dly++)
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, grp, 6, dly);
		
		sram_printk1(".");
		randx_cp = randx;
		for (addr=start; addr<end; addr=addr+4)
		{
			val = m_rand();
			reg32(addr) = val;
		}

		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, grp, 6, bk_ca_grp_offset);

		fail = 0;
		randx = randx_cp;
		for (addr=start; addr<end; addr=addr+4)
		{
			readback = reg32(addr);
			val = m_rand();
			if (readback != val)
			{
				//sram_printk1 ("CA dly 0x%d: tgt %08lX, src %08lX\n",dly, readback, val);
				fail = 1;
				break;
			}
		}
		if(fail == 0 && th1 ==-1) 
		{
			th1 = dly;
			sram_printk1 ("S");
		}
		if(fail == 0 && th1 !=-1) 
		{
			th2 = dly;
			sram_printk1 ("E");
			if(th2-th1>width)
			{
				width = th2-th1;
				th = (th1+th2)/2;
			}
		}
		if(fail == 1 && th2 != -1) 
		{
			// break;  // if not find the 2nd window, enable this line; if find, disable this line
			th1 =-1;
			th2 =-1;
		}
	}

	if(th !=-1)
	{
		// set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, grp, 6, th);  // for test only
		sram_printk1 (" CA dly %d, width %d\n", th, width);
		//if(grp==6)
		//{
		//	set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, 12, 6, th);
		//}
	}
	else
	{
		set_register_bits(DDR3_CAL_BASE_ADDR + 0x3c, grp, 6, bk_ca_grp_offset);
		sram_printk1 (" CA def %d\n", bk_ca_grp_offset);
	}
}
#endif

#ifdef DISPLAY_DEBUG
void ddr3_ca_cal(void)
{
	sram_printk1 ("0x3C = 0x%X\n", reg32(DDR3_CAL_BASE_ADDR + 0x3c));
	mtest_ca_rand(0x80000000, 0x80100000, 0);
	mtest_ca_rand(0x80000000, 0x80100000, 6);
	mtest_ca_rand(0x80000000, 0x80100000, 18);
	mtest_ca_rand(0x80000000, 0x80100000, 24);
	sram_printk1 ("0x3C = 0x%X\n", reg32(DDR3_CAL_BASE_ADDR + 0x3c));
}
#endif

#if 0
void sw_test() 
{
	u32 fail,i = 0,data;
	
	addr_write(SW_TEST_START, 0);
	
	while(1)
	{
		addr_read(SW_TEST_START ,&data);
		if(data == 0)
			continue;
		
		addr_write(SW_TEST_COMPLETE ,0);
		for(i=0; i<1000; i++)
		{
			cal_tx90_step(&fail);
			if( fail )
			{
				break;
			}
		}
		addr_write(SW_TEST_DATA0,fail);
		
		addr_write(SW_TEST_COMPLETE ,0x1);
		
		addr_write(SW_TEST_START, 0);

		if(fail)
			sram_printk1("*");
		else
			sram_printk1(".");
	}
}
#endif

#if 0
void mtestfortemp(unsigned int start, unsigned int end)
{
	unsigned int addr = 0, readback, fail_cnt =0;
	int pattern = 0, incr = 1;
        unsigned int val = 0,i;

	for(;;)
	{
		#if 1
		sram_printk1 ("\nP %08lX  W... ",pattern);
		for (addr=start,val=pattern; addr<end; addr=addr+4)
		{
			reg32(addr) = val;
			val  += incr;
		}
		#endif

		#if 1
		sram_printk1 ("R...%d", fail_cnt);

		for (addr=start,val=pattern; addr<end; addr=addr+4)
		{
			readback = reg32(addr);
			#if 0
			if (readback != val)
			{
				fail_cnt++;
				sram_printk1 ("\nMem error @ 0x%08X: found %08lX, expected %08lX,  %d\n",addr, readback, val, fail_cnt);

			}
			val  += incr;
			#endif
		}

		if(pattern & 0x80000000)
		{
			pattern = -pattern;          /* complement & increment */
		}
		else
		{
			pattern = ~pattern;
		}
		incr = -incr;
		#endif
	}
}
#endif

void ddr3_init()
{
	uart_init();
	ddr3_init_chip_temp();
	#ifdef DISPLAY_DEBUG 
	ddr3_ca_cal();
	#endif
	// mtestfortemp(0xa2000000, 0xa3000000);
	// sram_printk1("into_sw_test ");
	// sw_test();

	//sram_printk("wait for init,write 0xbf0e0008, 0x5a init, 0xa5 no init ");
	//addr_write(0xbf0e0008,0x0);
	//while(1){
	// addr_read(0xbf0e0008,&start_dly);
	//if(start_dly == 0x5a) {
	//   ddr3_init_chip_temp();
	//}else if(start_dly == 0xa5) break;
	//}
	//sram_printk("init ok");

	#ifdef DDR3_DBG_PRINT
	//sram_printk("\n\nDDR3 clk freq is %d\n", freq);
	//if (freq > 540)
	//  sram_printk("\nNOTE: DDR3 clk-freq larger than 540MHz, the HARDWARE should be modified.\n");
	#endif

	//  printf("DDR FREQ %d\n",freq);
	//  jump_return();
}


#endif /* CONFIG_BOARD_WITH_CHIP */

#endif /* CONFIG_USE_DDR3 */
