#include <stdint.h>

/*  Macro:
				#define DDR2 or not

		api:
        wd_write
        			wd_write(addr,data);
        wd_read
        			wd_read(addr,&data);
        PHY_INFO
        			print  //do{..}while(0)
		
		para:
				global_ddr_freq;  // set current ddr2 freq: 400 528   or current ddr3 freq: 528 664
				global_ddr_size;  // set current ddr2 size(MByte)  64  128   or current ddr3 size:  128  256
*/

#include <ddr_config.h>
void udelay(unsigned int us);

#ifdef CONFIG_USE_DDR3
#ifdef CONFIG_FREQ666
#define global_ddr_freq 664
#else
#define global_ddr_freq 528
#endif
#endif

#ifdef CONFIG_USE_DDR2
#define DDR2
#ifdef CONFIG_FREQ396
#define global_ddr_freq 400
#else
#define global_ddr_freq 528
#endif
#endif

#define PHY_INFO(...)
//#define PHY_INFO  printf

void wd_write(unsigned long addr, unsigned long data)
{
    *(volatile unsigned long *)addr = data;
}

void wd_read(unsigned long addr, unsigned long *data)
{
    *(volatile unsigned long *)data = *(volatile unsigned long *)addr;
}

#define SDRAM_REG_BASE       0xBF000000
#define DDR3_CAL_BASE_ADDR   0xBF000800

#define  ADDR_PWRUP         (0x6 << 2)
#define  ADDR_PRE           (0x0 << 2)
#define  ADDR_AR            (0x2 << 2)
#define  ADDR_RL_ADD        (0xE << 2)
#define  ADDR_TWR           (0x17 << 2)
#define  ADDR_ZQCL          (0x3 << 2)
#define  ADDR_MODE          (0x8 << 2)
#define  ADDR_MRS           (0x1 << 2)


#define PMU_BASE             0xbf004800
#define PMU_PWR_CTRL         PMU_BASE
#define PMU_GPIO_CTRL        PMU_BASE + 0x200
#define PMU_RF_CTRL          PMU_BASE + 0x300
#define PMU_CTRL_REG         PMU_BASE + 0x4
#define PMU_SLP_PD_REG       PMU_BASE + 0x8
#define PMU_STDBY_PD_REG     PMU_BASE + 0xc
#define PMU_INT_REG          PMU_BASE + 0x10
#define SLP_TMR_REG          PMU_BASE + 0x14
                            
#define RF_CTRL_REG          0xbf004f00
#define RF_CTRL_34_REG       RF_CTRL_REG + 0x34
#define RF_CTRL_4C_REG       RF_CTRL_REG + 0x4c
#define RF_CTRL_50_REG       RF_CTRL_REG + 0x50
#define RF_CTRL_54_REG       RF_CTRL_REG + 0x54
#define RF_CTRL_58_REG       RF_CTRL_REG + 0x58
#define RF_CTRL_5C_REG       RF_CTRL_REG + 0x5c
                            
#define GCI0_BASE            PMU_BASE + 0x100
// 0:  pdma
// 1:  uart0
// 2:  uart1
// 3:  uart2
// 4:  sflash
// 5:  timer0
// 6:  timer1
// 7:  timer2
// 13: gdma
// 14: wifi_mac
// 15: css
// 25: wakeup
#define GCI0_STATUS          GCI0_BASE   
#define GCI0_POLAR           GCI0_BASE + 0x4
#define GCI0_MASK            GCI0_BASE + 0x8
                            
#define GCI1_BASE            PMU_BASE + 0x110
#define GCI1_STATUS          GCI1_BASE
#define GCI1_POLAR           GCI1_BASE + 0x4
#define GCI1_MASK            GCI1_BASE + 0x8
                            
#define SYS_MEM              0xa0000000

#if 0
static unsigned long get_global_ddr_freq(void)
{
	unsigned long current_freq;
	wd_read(DDR3_CAL_BASE_ADDR + 0x5c,&current_freq);
	current_freq = (current_freq>>4&0xff)*(40/5);
	
	return current_freq;
}
#endif
#if 0
static void send_mrs_2(unsigned long i, unsigned long val)
{
	wd_write(SDRAM_REG_BASE + 0x20, ((i << 14) | val));
	wd_write(SDRAM_REG_BASE + 0x04, 1);
}
#endif

void srf_enter(void)
{
	unsigned long data;
	// wd_write(0xBF002900, ('X' << 24));
	#ifndef DDR2
	#if 0
	send_mrs_2(1,0x00);
	#endif
	#endif

	#if 0
	unsigned long clr_msk, set_msk;
	// Mask off all interrupt except wakeup intr
	data = ~(1<<25); /* data[25] = 0 */
	wd_write(GCI0_MASK ,data);
	wd_write(GCI1_MASK ,data);

	set_msk = (1<<25);
	wd_read(GCI0_STATUS,&data);
	if (data & set_msk) 
		PHY_INFO("!!! ERROR: GCI0_STATUS %0h Wake up status is active",data);
	wd_read(GCI1_STATUS,&data);
	if (data & set_msk) 
		PHY_INFO("!!! ERROR: GCI1_STATUS %0h Wake up status is active",data);

	// Enable BUCK control by PMU register
	wd_read(RF_CTRL_34_REG, &data);
	data |= (1<<7); /* data[7] = 1; */
	wd_write(RF_CTRL_34_REG, data);

	// Set Power down control in stndby mode
	wd_read(PMU_STDBY_PD_REG, &data);
	/*
	data[26:22] = 60x01e; data[18:17] = 20x02;
	data[12] = 1; data[10] = 1;
	data[5:0] = 60x021; */
	// turn on psw_ddr
	clr_msk = ~((0x3f<<22) & (0x3<<17) & (0x3f));
	set_msk =  (0x1e << 22)| (0x2<<17) | (1<<12) | (1<<10) | (0x21);
	data = (data & clr_msk) | set_msk;
	wd_write(PMU_STDBY_PD_REG, data);

	// Set Power down control in SLP mode
	wd_read(PMU_SLP_PD_REG, &data);
	data = (data & (~0x3f)) | 0x20; /* data[5:0] = 60x020; // turn on psw_ddr */
	wd_write(PMU_SLP_PD_REG, data);

	/*  ??? TODO
			// Set SLP timer
			data = 0x01000040;
			wd_write(SLP_TMR_REG, data);
			//$display ("Setting SLP_TMR_REG Enable and 0x40");
	*/
	// ??? TODO
	data = 0x00001e00;
	wd_write(PMU_INT_REG, data);
	//PHY_INFO(PMU_INT_REG, data);
	//PHY_INFO ("read dta: %8h",data) ;
	//PHY_INFO ("Setting PMU_INT_REG - Disabling Non Sleep Timer INT function");
	#endif

	wd_write(SDRAM_REG_BASE + 0x00, 1);

	// MC enter DRAM Self refresh
	wd_read(SDRAM_REG_BASE + 0x0c, &data);
	data = data | 0x1 | (1<<8); /* data[0] = 1; data [8] = 1; */
	wd_write(SDRAM_REG_BASE + 0x0c, data);
	//~ PHY_INFO ("Set MC to make DRAM into Self refresh mode");

	#if 1
	// DDR PHY Power Down
	wd_write(DDR3_CAL_BASE_ADDR + 0x04, 0x00000050);
	#else
	// DDR PHY goes to power off
	wd_read(DDR3_CAL_BASE_ADDR + 0x04, &data);
	clr_msk = ~(0x0f | 0x20); /* data[3:0] = 0; data[5] = 0; */
	data = data & clr_msk;
	wd_write(DDR3_CAL_BASE_ADDR + 0x04, data);
	#endif

	#if 0
	// Enter power saving
	// Set Bit6 DDR PHY REG Reset Disable
	data = 0x00000045;
	wd_write(PMU_CTRL_REG, data);
	//wd_read(PMU_CTRL_REG, &data);
	//$display ("read dta: %8h",data) ;
	//PHY_INFO("Setting PMU_CTRL_REG - Set SLP_ON_REG to enter Sleep Mode");
	#endif
	// wd_write(0xBF002900, ('Y' << 24));
}

void srf_exit(unsigned long global_ddr_size) 
{
	unsigned long data, clr_msk;
	#ifdef DDR2
	unsigned long tREFI = 0;
	#endif
	// wd_write(0xBF002900, ('A' << 24));

	#if 0
	// Clear interrupt source
	wd_read(PMU_INT_REG, &data);
	wd_write(PMU_INT_REG, data);
	#endif

	#if 1
	// Power Up DDR
	wd_write(DDR3_CAL_BASE_ADDR + 0x04, 0x00000060);
	udelay(500);
	#else
	// DDR PHY goes to power on
	wd_read(DDR3_CAL_BASE_ADDR + 0x04, &data);
	data |= (1<<5); /*  data[5] = 1; */
	wd_write(DDR3_CAL_BASE_ADDR + 0x04, data);
	#endif

	// printf("pause2\n");
	// while(1);
	// ddr_freq = get_global_ddr_freq();
	
	#ifdef DDR2
	tREFI = 7800000/(1000000/global_ddr_freq) - 20;
	wd_write((SDRAM_REG_BASE + 0x38),0x72050000);
	wd_read(SDRAM_REG_BASE + 0x34,&data);
	if(global_ddr_size == 64)
	{
		//set cfg_rd_en_earlier and cfg_wr_en_earlier
		//ddr_ctrl SDRAM type bit[0]: 0-DDR2; 1-DDR3.
		//LARGE_CL bit[8]: 0-DDR2; 1-DDR3.
		//PAGE_SIZE bit[17:16]: 2KB.
		data=(data&(~(1<<0))&(~(3<<4))&(~(1<<8))&(~(1<<12))&(~(1<<17))) \
								|(0<<0)|(3<<4)|(0<<8)|(1<<12)|(1<<17);
		wd_write((SDRAM_REG_BASE + 0x34), data);
	}
	else
	{
		//set cfg_rd_en_earlier and cfg_wr_en_earlier
		//ddr_ctrl SDRAM type bit[0]: 0-DDR2; 1-DDR3.
		//LARGE_CL bit[8]: 0-DDR2; 1-DDR3.
		//PAGE_SIZE bit[17:16]: 2KB.
		data=(data&(~(1<<0))&(~(3<<4))&(~(1<<8))&(~(1<<12))&(~(1<<17))) \
								|(0<<0)|(3<<4)|(0<<8)|(0<<12)|(1<<17);
		wd_write((SDRAM_REG_BASE + 0x34), data);
	}
	if(global_ddr_freq <= 400)
	{
		
		//CL=6({A6,A5,A4}=6); tWR=6({A11,A10,A9}=5); BL=4
		wd_write((SDRAM_REG_BASE+0x20), 0x4006);
		wd_write((SDRAM_REG_BASE+0x20), 0x0a62);
		
		//tWR = 6.
		wd_write((SDRAM_REG_BASE + ADDR_TWR),0x60000);
		//tFAW-2 = 16
		wd_write((SDRAM_REG_BASE + ADDR_ZQCL),0x16000000);
		//tRTP-2 = 1, tRCD=6, tRP=6, tRAS=18.
		wd_write((SDRAM_REG_BASE + 0x24),0x01061206);
		//tRC = 24, tMRD=2, tREFI=0xc00.
		wd_write((SDRAM_REG_BASE + 0x2c),(0x18020c00 & 0xffff0000) | tREFI);
		
		if(global_ddr_size == 64)
		{
			//tRFC = 51, tRTW_add=2, tWTR=5, tRRD=4.
			wd_write((SDRAM_REG_BASE + 0x28),0x2a020504);
		}
		else
		{
			//tRFC = 51, tRTW_add=2, tWTR=5, tRRD=4.
			wd_write((SDRAM_REG_BASE + 0x28),0x33020504);
		}
			
	}
	else
	{
		
		//CL=6({A6,A5,A4}=6); tWR=6({A11,A10,A9}=5); BL=4
		wd_write((SDRAM_REG_BASE+0x20), 0x4006);
		wd_write((SDRAM_REG_BASE+0x20), 0x0e72);
		
		//tWR = 6.
		wd_write((SDRAM_REG_BASE + ADDR_TWR),0x80000);
		//tFAW-2 = 16
		wd_write((SDRAM_REG_BASE + ADDR_ZQCL),0x16000000);
		//tRTP-2 = 1, tRCD=6, tRP=6, tRAS=18.
		wd_write((SDRAM_REG_BASE + 0x24),0x02081808);
		//tRC = 24, tMRD=2, tREFI=0xc00.
		wd_write((SDRAM_REG_BASE + 0x2c),(0x1f021020 & 0xffff0000) | tREFI);
		
		if(global_ddr_size == 64)
		{
			//tRFC = 51, tRTW_add=2, tWTR=5, tRRD=4.
			wd_write((SDRAM_REG_BASE + 0x28),0x38050906);
		}
		else
		{
			//tRFC = 51, tRTW_add=2, tWTR=5, tRRD=4.
			wd_write((SDRAM_REG_BASE + 0x28),0x44050906);
		}
			
	}
	#else
	wd_write((SDRAM_REG_BASE + 0x38),0x70050000);
	//ddr_ctrl SDRAM type bit[0]: 0-DDR2; 1-DDR3.
	wd_write((SDRAM_REG_BASE + 0x34),0x01020131);
	if(global_ddr_freq > 600)
	{

		//CWL
		wd_write((SDRAM_REG_BASE+0x20), 0x8010);
		wd_write((SDRAM_REG_BASE+0x20), 0x4004);
		wd_write((SDRAM_REG_BASE+0x20), 0x0b60);

		wd_write((SDRAM_REG_BASE+ADDR_TWR), 0x000a0000);	//bit[19:16] should be set same as MR0 2*bit[11:9].
		//tFAW-2=25
		wd_write((SDRAM_REG_BASE+ADDR_ZQCL), 0x1c000000);
		//RCD=bit[3:0]=8, RAS=bit[12:8]=20, RP=bit[19:16]=8, RTP_minus2=bit[27:24]=2
		wd_write((SDRAM_REG_BASE+0x24), 0x030a180a);
		//tREFI=bit[12:0]=4158~=4096, tMRD=bit[18:16]=4, tRC=bit[29:24]=20
		wd_write((SDRAM_REG_BASE+0x2C), 0x22041410);
		if(global_ddr_size == 128)
		{
			//tRRD=bit[2:0]=6, tWTR_origin=bit[11:8]=5, tRTW_add=bit[17:16],tRFC=[31:24]=186
			wd_write((SDRAM_REG_BASE+0x28), 0x49020705);
		}
		else  // 256
		{
			//tRRD=bit[2:0]=6, tWTR_origin=bit[11:8]=5, tRTW_add=bit[17:16],tRFC=[31:24]=186
			wd_write((SDRAM_REG_BASE+0x28), 0x6a010705);
		}
	}
	else
	{

		//CWL
		wd_write((SDRAM_REG_BASE+0x20), 0x8010);
		wd_write((SDRAM_REG_BASE+0x20), 0x4004);
		wd_write((SDRAM_REG_BASE+0x20), 0x0b60);
		wd_write((SDRAM_REG_BASE+ADDR_TWR), 0x000a0000);	//bit[19:16] should be set same as MR0 2*bit[11:9].
		//tFAW-2=25
		wd_write((SDRAM_REG_BASE+ADDR_ZQCL), 0x1c000000);
		//RCD=bit[3:0]=8, RAS=bit[12:8]=20, RP=bit[19:16]=8, RTP_minus2=bit[27:24]=2
		wd_write((SDRAM_REG_BASE+0x24), 0x030a1708);
		//tREFI=bit[12:0]=4158~=4096, tMRD=bit[18:16]=4, tRC=bit[29:24]=20
		wd_write((SDRAM_REG_BASE+0x2C), 0x21070fe6);
		if(global_ddr_size == 128)
		{
			//tRRD=bit[2:0]=6, tWTR_origin=bit[11:8]=5, tRTW_add=bit[17:16],tRFC=[31:24]=186
			wd_write((SDRAM_REG_BASE+0x28), 0x49020705);
		}
		else
		{
			//tRRD=bit[2:0]=6, tWTR_origin=bit[11:8]=5, tRTW_add=bit[17:16],tRFC=[31:24]=186
			wd_write((SDRAM_REG_BASE+0x28), 0x6a010705);
		}
	}
	
	#endif

	//de-assert wait_200us (CKE) on bit[8] and wait_500us on bit[16] (RESETN for DDR3)
	wd_write((SDRAM_REG_BASE+ ADDR_PWRUP), 0x0);
	// udelay(500);

	//Power-up done
	wd_write(( SDRAM_REG_BASE+ ADDR_PWRUP), 0x1);
	// udelay(500);

	#if 1
	// MC exit DRAM self refresh
	wd_read(SDRAM_REG_BASE + 0x0c, &data);
	clr_msk = ~(0x01 | (1<<8));  // data[0] = 0; data [8] = 0;
	data = data & clr_msk;
	wd_write(SDRAM_REG_BASE + 0xc, data);
	//$display ("Set MC to make DRAM exit Self refresh mode @time = %0t",$time);
	udelay(500);
	#endif
	#if 0
	send_mrs_2(0,0xb60);
	send_mrs_2(1,0x04);
	send_mrs_2(2,0x10);
	send_mrs_2(3,0x00);
	#endif
	#if 1
	wd_write(DDR3_CAL_BASE_ADDR + 0x4, 0x61);
	udelay(500);
	wd_write(DDR3_CAL_BASE_ADDR + 0x4, 0x6f);
	udelay(500);
	#else
	// DDR PHY goes to power on
	wd_read(DDR3_CAL_BASE_ADDR + 0x4, &data);
	data |= 0x1 ; //data[3:0] = 0xf;
	wd_write(DDR3_CAL_BASE_ADDR + 0x4, data);
	data |= 0xf ; //data[3:0] = 0xf;
	wd_write(DDR3_CAL_BASE_ADDR + 0x4, data);

	udelay(500);
	#endif

	#if 0
	// MC exit DRAM self refresh
	wd_read(SDRAM_REG_BASE + 0x0c, &data);
	clr_msk = ~(0x01 | (1<<8));  // data[0] = 0; data [8] = 0;
	data = data & clr_msk;
	wd_write(SDRAM_REG_BASE + 0xc, data);
	//$display ("Set MC to make DRAM exit Self refresh mode @time = %0t",$time);
	udelay(500);
	#endif

	#ifndef DDR2
	#if 0
	send_mrs_2(1,0x00);
	#endif
	#endif
	// wd_write(0xBF002900, ('B' << 24));
}

