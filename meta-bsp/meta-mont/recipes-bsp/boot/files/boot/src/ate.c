/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file ate.c
*   \brief
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
#include <pmu.h>
#include <reg.h>
#include <lib.h>
#include <cm_mac.h>
#include "ate.h"
#include "ddr_config.h"
#include <sched.h>
#ifdef WT_TEST
#include "wtest.h"
#endif

#ifdef PHYSICAL_ADDR
#undef PHYSICAL_ADDR
#endif
#include "gdma.h"


#ifdef CONFIG_ATE
#define REG32(addr)            (*((volatile u32 *)(addr)))
#define addr_read(addr, data)  (*data = REG32(addr))
#define addr_write(addr, data) (REG32(addr) = (data))

int gpio_ids[] = {2,3,4,5,6,7,8,
                  9,10,11,12,13,14,15,16,
                  17,18,23,24,25,26,27,
                  31,-1};
unsigned long gpio_funcs[] = {0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,
                              3,3,3,3,0,0,0,
                              1,-1};
unsigned int ddr2_size;

unsigned int dtmp;
#ifdef WT_TEST
#define WLA_TEST_THREAD_STACK_SIZE  (1024*1024)
unsigned char ate_test_thread_stack[WLA_TEST_THREAD_STACK_SIZE];
extern wla_test_cfg *acfg_p;
extern void wla_test(void);
unsigned int ate_rssi;
unsigned int ate_cnt;
#endif
unsigned int gpio_func_0_7 = 0;
unsigned int gpio_func_8_15 = 0;
unsigned int gpio_func_16_23 = 0;
unsigned int gpio_func_24_31 = 0;


extern void ate_set_chan(unsigned char chan, unsigned char band);

volatile char polling = -1;
static u8 bb_register_read(int group,u32 bb_reg)
{
    u8 value=0;
    u32 data, dst;

    if(group == 0)
    {
        dst = (bb_reg/4) * 4;
    }
    else if(group == 1)
    {
        dst = ((bb_reg/4) * 4) + 0x100;
    }
    else if(group ==2)
    {
        dst = ((bb_reg/4) * 4) + 0x200;
    }
    else if(group==3)
    {
        dst = ((bb_reg/4) * 4) + 0x300;
    }
    else
    {
        printf("XXX: UnKonwn group %d\n", group);
        return 0;
    }
    data = BBREG_READ32(dst);

    value = (data >> 8*(bb_reg % 4)) & 0xffUL;

    //printf("\t\t\t\tread %d\t%08x\t%02x\n", group, bb_reg, value);
    return value;
}
static void bb_register_write(int group, u32 bb_reg, u8 value)
{
    u32 data, dst, mask=0;

    if(group==0)
    {
        dst = (bb_reg/4) * 4;
    }
    else if(group==1)
    {
        dst = ((bb_reg/4) * 4) + 0x100;
    }
    else if(group==2)
    {
        dst = ((bb_reg/4) * 4) + 0x200;
    }
    else if(group==3)
    {
        dst = ((bb_reg/4) * 4) + 0x300;
    }
    else
    {
        printf("XXX: UnKonwn group %d\n", group);
        return;
    }
    data =BBREG_READ32(dst);

//    printf("read %08x\t%08x ", dst, data);
    if(bb_reg%4==0)
    {
        mask |= 0x000000ffUL;
    }
    else if(bb_reg%4==1)
    {
        mask |= 0x0000ff00UL;
    }
    else if(bb_reg%4==2)
    {
        mask |= 0x00ff0000UL;
    }
    else if(bb_reg%4==3)
    {
        mask |= 0xff000000UL;
    }
    data = ( (data & ~(mask)) | ((value << 8*(bb_reg % 4)) & (mask)) );
    // reg0 will reset BB by write any value except zero
    // write the reg1,2,3 with the LSByte==0, so the BB won't be reset
    if(((dst==0) && (group==0)) && (bb_reg!=0))
    {
        mask = 0xffffff00UL;
        data = data & mask;
    }

//    printf("write %08x\t%08x\n", dst ,data);

    BBREG_WRITE32(dst, data);
}
/*-------------- Utils ---------------*/
void restore_pin_func(void)
{
    REG_WRITE32(PMU_GPIO_FUNC_0_7, gpio_func_0_7);
    REG_WRITE32(PMU_GPIO_FUNC_8_15, gpio_func_8_15);
    REG_WRITE32(PMU_GPIO_FUNC_16_23, gpio_func_16_23);
    REG_WRITE32(PMU_GPIO_FUNC_24_31, gpio_func_24_31);
}

void backup_pin_func(void)
{
    gpio_func_0_7   = REG_READ32(PMU_GPIO_FUNC_0_7);
    gpio_func_8_15  = REG_READ32(PMU_GPIO_FUNC_8_15);
    gpio_func_16_23 = REG_READ32(PMU_GPIO_FUNC_16_23);
    gpio_func_24_31 = REG_READ32(PMU_GPIO_FUNC_24_31);
}

void pinmux_set_pin_func_gpio(void)
{
    pmu_set_gpio_function(gpio_ids, gpio_funcs);
    return;
}

void gpio_func_direction_val(int input, int high)
{
    if(input)
    {
        PMUREG_WRITE32(GPIO_FUNC_EN, 0xff87ffff);   //skip gpio 19/20/21/22
        PMUREG_WRITE32(GPIO_OEN, 0xff87ffff);       //skip gpio 19/20/21/22
    }
    else
    {
        PMUREG_WRITE32(GPIO_FUNC_EN, 0xff87ffff);   //skip gpio 19/20/21/22
        PMUREG_WRITE32(GPIO_OEN, 0x0);              //skip gpio 19/20/21/22
        if(high)
            PMUREG_WRITE32(GPIO_OUTPUT_DATA, 0xff87ffff);
        else
            PMUREG_WRITE32(GPIO_OUTPUT_DATA, 0x0);
    }
}

/*-------------- Test Item ---------------*/
static void pin_input_init(void)
{
    backup_pin_func();
    pinmux_set_pin_func_gpio();
    gpio_func_direction_val(1,0);
}

static void dc_tst(void)
{
    printf("%s\n",__func__);
}

static void ip_tst(void)
{
    printf("%s\n",__func__);
}

static void wrt(void)
{
    REG_WRITE32(RESULT3, PMUREG(GPVAL));
}

static void vol(void)
{
    /*Set goip output direction and value low*/
    gpio_func_direction_val(0, 0);
}

static void voh(void)
{
    /*Set goip output direction and value high*/
    gpio_func_direction_val(0, 1);
}

static void ts_md(void)
{
    printf("%s\n",__func__);
}

/******************************************************
 * USB / EPHY / DDR
 *
 *
*******************************************************/
#ifdef DDR_TEST
static unsigned int verify_dma(void *dtmp)
{
    u32 val = *(u32 *)dtmp;

    u32 dma_busy;
    u32 dma_data0;
    u32 dma_data1;
    u32 dma_data2;
    u32 dma_data3;
    u32 dma_data4;
    u32 dma_data5;
    u32 dma_data6;
    u32 dma_data7;
    u32 dma_data8;
    u32 dma_data9;
    u32 dma_data10;
    u32 dma_data11;
    u32 dma_data12;
    u32 dma_data13;
    u32 dma_data14;
    u32 dma_data15;

    addr_read(0xa0000100, &dma_data0);
    addr_read(0xa0000104, &dma_data1);
    addr_read(0xa0000108, &dma_data2);
    addr_read(0xa000010c, &dma_data3);
    addr_read(0xa0000110, &dma_data4);
    addr_read(0xa0000114, &dma_data5);
    addr_read(0xa0000118, &dma_data6);
    addr_read(0xa000011c, &dma_data7);
    addr_read(0xa0000120, &dma_data8);
    addr_read(0xa0000124, &dma_data9);
    addr_read(0xa0000128, &dma_data10);
    addr_read(0xa000012c, &dma_data11);
    addr_read(0xa0000130, &dma_data12);
    addr_read(0xa0000134, &dma_data13);
    addr_read(0xa0000138, &dma_data14);
    addr_read(0xa000013c, &dma_data15);

    if(  (0xffffffff != dma_data0 ) | (0xffffffff != dma_data1 )
       | (0xffffffff != dma_data2 ) | (0xffffffff != dma_data3 )
       | (0x0        != dma_data4 ) | (0x0        != dma_data5 )
       | (0x0        != dma_data6 ) | (0x0        != dma_data7 )
       | (0xaaaa5555 != dma_data8 ) | (0xaaaa5555 != dma_data9 )
       | (0xaaaa5555 != dma_data10) | (0xaaaa5555 != dma_data11)
       | (0x5555aaaa != dma_data12) | (0x5555aaaa != dma_data13)
       | (0x5555aaaa != dma_data14) | (0x5555aaaa != dma_data15))
    {
        printf("DMA FAIL\n");
        val |= (1<<21);
    }
    else
        printf("DMA Pass\n");

    REG_WRITE32(RESULT5, val);
    return 0;
}

static void ddr_test(void)
{
    u32 i, data, addr;
    u32 pin_version;
    u32 flag_512m, data_len=0;
    u32 sdram_size;

    REG_WRITE32(RESULT5, 0);
    /*Support DDR2 only ?*/
    addr_read(DDR_SIZE_INFO_ADDR,&sdram_size);
    //#ifndef    __USE_DDR2
    //sdram_size=g_crystal;
    //#endif

    /*
    Sonata ATE Test Pattern for DDR Interface
    Jan. 10, 2014

    Objective: At-speed (667MHz) test on DDR3 interface

    Premise:
    A 16-bit 2Gb DDR3 device and a SPI NOR flash are needed on the load board.
    GPIO pins are detectable by the tester.
    Boot-up sequence is properly configured by the strap pins.
    (Starting from ROM and then jumping to NOR flash)

    Test Patterns:
    1.	Drive Test_Done = GPIO[12] = 0;
    2.	DDR3 Initialization and Calibration (Provided by Clarke)
    Assert test_fail bit if the calibrated range is too small.
    3.	Test stuck-at errors on address and DQ, DQM bits one by one (non-cacheable addressing)
    */
    //First, write to some specific addresses.
    /*
    @ 0xa000_0000 = 0x5555_aaaa;
    @ 0xa000_0004 = 0xaaaa_5555;
    @ 0xafff_fff8   = 0xffff_0000;
    @ 0xafff_fffc   = 0x000_ffff;
    */
    addr_write(0xa0000000, 0x5555aaaa);
    addr_write(0xa0000004, 0xaaaa5555);
    addr_write(0xaffffff8, 0xffff0000);
    addr_write(0xaffffffc, 0x000ffff);

    // BA[0] = Addr[11]=1
    //@ 0xa0000800 = 0x5555BA00;
    addr_write(0xa0000800, 0x5555BA00);
    // BA[1] = Addr[12]=1
    //@ 0xa0001000 = 0x5555BA01;
    addr_write(0xa0001000, 0x5555BA01);
    // BA[2] = Addr[13]=1
    //@ 0xa000_2000 = 0x5555_BA02;
    addr_write(0xa0002000, 0x5555BA02);
    // Row[0] = Addr[14]=1
    //@ 0xa000_4000 = 0xaaaa_5500;
    addr_write(0xa0004000, 0xaaaa5500);
    // Row[1] = Addr[15]=1
    //@ 0xa000_8000 = 0xaaaa_5501;
    addr_write(0xa0008000, 0xaaaa5501);
    // Row[2] = Addr[16]=1
    //@ 0xa001_0000 = 0xaaaa_5502;
    addr_write(0xa0010000, 0xaaaa5502);
    // Row[10] = Addr[24]=1
    //@ 0xa100_0000 = 0xaaaa_550A;
    addr_write(0xa1000000, 0xaaaa550A);
    // Row[11] = Addr[25]=1
    //@ 0xa2000000 = 0xaaaa550B;
    addr_write(0xa2000000, 0xaaaa550B);
    // Row[12] = Addr[26]=1
    //@ 0xa4000000 = 0xaaaa550C;
    if(sdram_size>=128)
    {
        addr_write(0xa4000000, 0xaaaa550C);
    }
    // Row[13] = Addr[27]=1
    //@ 0xa8000000 = 0xaaaa550D;
    if(sdram_size>=256)
    {
        addr_write(0xa8000000, 0xaaaa550D);
    }
    #if 0
    // Row[14] = Addr[28]=1
    //@ 0xb0000000 = 0xaaaa550E;
    if(sdram_size>=512)
    {
        addr_write(0xB0000000, 0xaaaa550E);
    }
    #endif
    // Col[3] = Addr[4]=1
    //@ 0xa0000010 = 0x555500C3;
    addr_write(0xa0000010, 0x555500C3);
    // Col[4] = Addr[5]=1
    //@ 0xa0000020 = 0x555500C4;
    addr_write(0xa0000020, 0x555500C4);
    // Col[5] = Addr[6]=1
    //@ 0xa0000040 = 0x555500C5;
    addr_write(0xa0000040, 0x555500C5);
    // Col[6] = Addr[7]=1
    //@ 0xa0000080 = 0x555500C6;
    addr_write(0xa0000080, 0x555500C6);
    // Col[7] = Addr[8]=1
    //@ 0xa0000100 = 0x555500C7;
    addr_write(0xa0000100, 0x555500C7);
    // Col[8] = Addr[9]=1
    //@ 0xa0000200 = 0x555500C8;
    addr_write(0xa0000200, 0x555500C8);
    // Col[9] = Addr[10]=1
    //@ 0xa0000400 = 0x555500C9;
    addr_write(0xa0000400, 0x555500C9);

    //Second, read from the above addresses and check the data.
    dtmp = 0;
    //Read(0xa0000000) = 0x5555aaaa;
    addr_read(0xa0000000, &data);
    if(0x5555aaaa != data)
    {
        dtmp |= 1 << 0;
    }
    //Read(0xa0000004) = 0xaaaa5555;
    addr_read(0xa0000004, &data);
    if(0xaaaa5555 != data)
    {
        dtmp |= 1 << 1;
    }
    //Read(0xaffffff8)   = 0xffff0000;
    addr_read(0xaffffff8, &data);
    if(0xffff0000 != data)
    {
        dtmp |= 1 << 2;
    }
    //Read(0xaffffffc)   = 0x000ffff;
    addr_read(0xaffffffc, &data);
    if(0x000ffff != data)
    {
        dtmp |= 1 << 3;
    }
    //Read(0xa0000800) = 0x5555BA00;
    addr_read(0xa0000800, &data);
    if(0x5555BA00 != data)
    {
        dtmp |= 1 << 4;
    }
    //Read(0xa0001000) = 0x5555BA01;
    addr_read(0xa0001000, &data);
    if(0x5555BA01 != data)
    {
        dtmp |= 1 << 5;
    }
    //Read(0xa0002000) = 0x5555BA02;
    addr_read(0xa0002000, &data);
    if(0x5555BA02 != data)
    {
        dtmp |= 1 << 6;
    }
    //Read(0xa0004000) = 0xaaaa5500;
    addr_read(0xa0004000, &data);
    if(0xaaaa5500 != data)
    {
        dtmp |= 1 << 7;
    }
    //Read(0xa0008000) = 0xaaaa5501;
    addr_read(0xa0008000, &data);
    if(0xaaaa5501 != data)
    {
        dtmp |= 1 << 8;
    }
    //Read(0xa0010000) = 0xaaaa5502;
    addr_read(0xa0010000, &data);
    if(0xaaaa5502 != data)
    {
        dtmp |= 1 << 9;
    }
    //Read(0xa1000000) = 0xaaaa550A;
    addr_read(0xa1000000, &data);
    if(0xaaaa550A != data)
    {
        dtmp |= 1 << 10;
    }
    //Read(0xa2000000) = 0xaaaa550B;
    addr_read(0xa2000000, &data);
    if(0xaaaa550B != data)
    {
        dtmp |= 1 << 11;
    }
    //Read(0xa4000000) = 0xaaaa550C;

    if(sdram_size>=128)
    {
        data_len = 1;

        addr_read(0xa4000000, &data);
        if(0xaaaa550C != data)
        {
            dtmp |= 1 << 12;
        }
    }
    //Read(0xa8000000) = 0xaaaa550D;
    if(sdram_size>=256)
    {
        data_len = 2;

        addr_read(0xa8000000, &data);
        if(0xaaaa550D != data)
        {
            dtmp |= 1 << 13;
        }
    }
    #if 0
    //Read(0xb0000000) = 0xaaaa550E;
    if(sdram_size>=512)
    {
	    addr_read(0xb0000000, &data);
	    if(0xaaaa550E != data)
        {
            dtmp |= 1 << 22;
        }
    }
    #endif
    //Read(0xa0000010) = 0x555500C3;
    addr_read(0xa0000010, &data);
    if(0x555500C3 != data)
    {
        dtmp |= 1 << 14;
    }
    //Read(0xa0000020) = 0x555500C4;
    addr_read(0xa0000020, &data);
    if(0x555500C4 != data)
    {
        dtmp |= 1 << 15;
    }
    //Read(0xa0000040) = 0x555500C5;
    addr_read(0xa0000040, &data);
    if(0x555500C5 != data)
    {
        dtmp |= 1 << 16;
    }
    //Read(0xa0000080) = 0x555500C6;
    addr_read(0xa0000080, &data);
    if(0x555500C6 != data)
    {
        dtmp |= 1 << 17;
    }
    //Read(0xa0000100) = 0x555500C7;
    addr_read(0xa0000100, &data);
    if(0x555500C7 != data)
    {
        dtmp |= 1 << 18;
    }
    //Read(0xa0000200) = 0x555500C8;
    addr_read(0xa0000200, &data);
    if(0x555500C8 != data)
    {
        dtmp |= 1 << 19;
    }
    //Read(0xa0000400) = 0x555500C9;
    addr_read(0xa0000400, &data);
    if(0x555500C9 != data)
    {
        dtmp |= 1 << 20;
    }
#if 0
    addr = (1<<9) + (1<<10);
    for(i=0;i<16;i++)
    {
        addr_write(addr,i|i<<4|i<<8|i<<12|i<<16|i<<20|i<<24|i<<28);
        addr=addr<<1;
    }
    addr = (1<<9) + (1<<10);
    for(i=0;i<16;i++)
    {
        addr_read(addr,&data);
        if(data!=(i|i<<4|i<<8|i<<12|i<<16|i<<20|i<<24|i<<28))
        {
            dtmp |= 1 << 22;
        }
        addr=addr<<1;
    }
#endif

    /* DMA Pattern */

	//CPU write memory
    addr_write(0xa0000000, 0xffffffff);
    addr_write(0xa0000004, 0xffffffff);
    addr_write(0xa0000008, 0xffffffff);
    addr_write(0xa000000c, 0xffffffff);
    addr_write(0xa0000010, 0x0);
    addr_write(0xa0000014, 0x0);
    addr_write(0xa0000018, 0x0);
    addr_write(0xa000001c, 0x0);
    addr_write(0xa0000020, 0xaaaa5555);
    addr_write(0xa0000024, 0xaaaa5555);
    addr_write(0xa0000028, 0xaaaa5555);
    addr_write(0xa000002c, 0xaaaa5555);
    addr_write(0xa0000030, 0x5555aaaa);
    addr_write(0xa0000034, 0x5555aaaa);
    addr_write(0xa0000038, 0x5555aaaa);
    addr_write(0xa000003c, 0x5555aaaa);

    addr_write(0xa0000100, 0x0);
    addr_write(0xa0000104, 0x0);
    addr_write(0xa0000108, 0x0);
    addr_write(0xa000010c, 0x0);
    addr_write(0xa0000110, 0x0);
    addr_write(0xa0000114, 0x0);
    addr_write(0xa0000118, 0x0);
    addr_write(0xa000011c, 0x0);
    addr_write(0xa0000120, 0x0);
    addr_write(0xa0000124, 0x0);
    addr_write(0xa0000128, 0x0);
    addr_write(0xa000012c, 0x0);
    addr_write(0xa0000130, 0x0);
    addr_write(0xa0000134, 0x0);
    addr_write(0xa0000138, 0x0);
    addr_write(0xa000013c, 0x0);
	//DMA operation
    gdma_submit(GDMA_OP_COPY, 0x0, 0x100, 0x40, (void *)verify_dma, (void *)&dtmp);

    /*
    4.	Assert Test Status Bits (test_done should be asserted later than test_fail.)
    //First, assert test status bit.
    Test_Fail = GPIO[15] = 0 for passed or 1 for failed.
    //Second, assert test done bit.
    Test_Done = GPIO[12] = 1;
    */

    //addr_write(0xbf200000, 0x06 );	//write software version
    //addr_read(0xbf200000);
    //u32 dtmp = 0;
    //addr_read(0xbf200000, &dtmp);
    //*(volatile unsigned long *)(0xbf540100) = 'V';
    //*(volatile unsigned long *)(0xbf540100) = 'E';
    //*(volatile unsigned long *)(0xbf540100) = 'R';
    //*(volatile unsigned long *)(0xbf540100) = ':';

    //uart_print_data(dtmp);
    /*
    if(((dtmp>>4)&0xf)>9)
    {
        *(volatile unsigned long *)(0xbf540100) = ((dtmp>>4)&0xf)+'A'-10;
    }
    else
    {
        *(volatile unsigned long *)(0xbf540100) = ((dtmp>>4)&0xf)+'0';
    }

    if((dtmp&0xf)	>9 )
    {
        *(volatile unsigned long *)(0xbf540100) = (dtmp&0xf)+'A'-10;
    }
    else
    {
        *(volatile unsigned long *)(0xbf540100) = (dtmp&0xf)+'0';
    }
    */

    //*(volatile unsigned long *)(0xbf540100) = 0x88;
    #if 0
    while((*(volatile unsigned short *)(0xbf540010) & 0x7f) >= 64);
    *(volatile unsigned long *)(0xbf540100) = 0x88;
    while((*(volatile unsigned short *)(0xbf540010) & 0x7f) >= 64);
    *(volatile unsigned long *)(0xbf540100) = 0x66;
    while((*(volatile unsigned short *)(0xbf540010) & 0x7f) >= 64);
    *(volatile unsigned long *)(0xbf540100) = (dtmp>>24&0xff);
    while((*(volatile unsigned short *)(0xbf540010) & 0x7f) >= 64);
    *(volatile unsigned long *)(0xbf540100) = (dtmp>>16&0xff);
    while((*(volatile unsigned short *)(0xbf540010) & 0x7f) >= 64);
    *(volatile unsigned long *)(0xbf540100) = (dtmp>>8&0xff);
    while((*(volatile unsigned short *)(0xbf540010) & 0x7f) >= 64);
    *(volatile unsigned long *)(0xbf540100) = (dtmp>>0&0xff);
    #endif
}
#endif

#ifdef USB_TEST
#if 0
static void std_pkt_usb(void)
{
    int i;

    //only loopback mode test
    //operation
    USB0REGW(USB_TEST_MODE_REG, 0x6);
    USB1REGW(USB_TEST_MODE_REG, 0x6);

    for(i=HIGH_SPEED; i<=LOW_SPEED; i++)
    {
        //set HIGH/FULL/LOW mode bit[5:4]
        USB0_UPDATE32(USB_TEST_PKT_CFG_REG, (i<<4), 0x30);
        USB1_UPDATE32(USB_TEST_PKT_CFG_REG, (i<<4), 0x30);
        //case4
        USB0_UPDATE32(USB_PHY_DIG_CFG, 0x1000, 0x1000);
        USB1_UPDATE32(USB_PHY_DIG_CFG, 0x1000, 0x1000);
        //LoopBack
        USB0_UPDATE32(USB_TEST_PKT_CFG_REG, 0x1, 0x1);
        USB1_UPDATE32(USB_TEST_PKT_CFG_REG, 0x1, 0x1);

        //check loopback complete bit[0]
        while(!(USB0REGR(USB_TEST_PKT_STS_REG)&0x01) &&
                !(USB1REGR(USB_TEST_PKT_STS_REG)&0x01))
            ;
        //check status bit[2]
        printf("USB0 Status:%x\n",USB0REGR(USB_TEST_PKT_STS_REG)&0x4);
        printf("USB1 Status:%x\n",USB1REGR(USB_TEST_PKT_STS_REG)&0x4);
    }
}
#endif
static void psudo_case4(unsigned char mode, char ebit)
{
    unsigned int status;

    //USB0
    //loopback bit[12]
    //USB0_UPDATE32(USB_PHY_DIG_CFG, 0x1000, 0x1000);
    USB0REGW(USB_PHY_DIG_CFG, 0x1F30CC);

    //set HIGH/FULL/LOW speed mode bit[6:5], and enable/disable error bit bit[4], RX/TX bit[1:0]
    USB0_UPDATE32(USB_BIST_CFG_REG, (mode<<5)|(ebit<<4)|0x3, 0x73);

    udelay(1000);
    status = USB0REGR(USB_BIST_BIT_ERR_DETECTED_REG)&0x1;

    //save result
    if(!ebit)
        REG_UPDATE32(RESULT2, REG_READ32(RESULT2)|(status<<(mode*2)), 0x3F);
    else
        REG_UPDATE32(RESULT2, REG_READ32(RESULT2)|(status<<(mode*2+1)), 0x3F);
/*
    if(ebit && status)
        printf("USB0 Pass: Psudo ERR Dected:%x ebit:%d\n",status, ebit);
    else if (!ebit && !status)
        printf("USB0 Pass: Psudo ERR Dected:%x ebit:%d\n",status, ebit);
    else
        printf("USB0 Fail: Psudo ERR Dected:%x ebit:%d\n",status, ebit);
*/
    USB0_UPDATE32(USB_PHY_DIG_CFG, 0x0, 0x1000);

    //USB1
    //loopback bit[12]
    //USB1_UPDATE32(USB_PHY_DIG_CFG, 0x1000, 0x1000);
    USB1REGW(USB_PHY_DIG_CFG, 0x1F30CC);

    //set HIGH/FULL/LOW speed mode bit[6:5], and enable/disable error bit bit[4], RX/TX bit[1:0]
    USB1_UPDATE32(USB_BIST_CFG_REG, (mode<<5)|(ebit<<4)|0x3, 0x73);

    udelay(1000);
    status = USB1REGR(USB_BIST_BIT_ERR_DETECTED_REG)&0x1;

    //save result
    if(!ebit)
        REG_UPDATE32(RESULT2, REG_READ32(RESULT2)|(status<<(mode*2))<<8, 0x3F00);
    else
        REG_UPDATE32(RESULT2, REG_READ32(RESULT2)|(status<<(mode*2+1))<<8, 0x3F00);
/*
    if(ebit && status)
        printf("USB1 Pass: Psudo ERR Dected:%x ebit:%d\n",status, ebit);
    else if (!ebit && !status)
        printf("USB1 Pass: Psudo ERR Dected:%x ebit:%d\n",status, ebit);
    else
        printf("USB1 Fail: Psudo ERR Dected:%x ebit:%d\n",status, ebit);
*/
    USB1_UPDATE32(USB_PHY_DIG_CFG, 0x0, 0x1000);
}

static void psudo_pkt_usb(int start_mode)
{
    int i;
    int start, end;

    if(start_mode == HIGH_SPEED)
    {
        start = HIGH_SPEED;
        end = HIGH_SPEED;
    }
    else
    {
        start = FULL_SPEED;
        end = LOW_SPEED;
    }

    for(i=start; i<=end; i++)
    {
        USB0REGW(USB_TEST_MODE_REG, 0x0);
        USB1REGW(USB_TEST_MODE_REG, 0x0);

        USB0REGW(USB_BIST_CFG_REG, 0x0);
        USB0REGW(USB_BIST_PKT_LEN_REG, 0x40);
        USB0REGW(USB_BIST_CFG_REG, 0x2);

        USB1REGW(USB_BIST_CFG_REG, 0x0);
        USB1REGW(USB_BIST_PKT_LEN_REG, 0x40);
        USB1REGW(USB_BIST_CFG_REG, 0x2);

        USB0REGW(USB_BIST_CFG_REG, 0x0);
        USB0REGW(USB_BIST_PKT_LEN_REG, 0x40);
        USB0REGW(USB_BIST_CFG_REG, 0x1);

        USB1REGW(USB_BIST_CFG_REG, 0x0);
        USB1REGW(USB_BIST_PKT_LEN_REG, 0x40);
        USB1REGW(USB_BIST_CFG_REG, 0x1);

        USB0REGW(USB_BIST_CFG_REG, 0x0);
        USB1REGW(USB_BIST_CFG_REG, 0x0);

        USB0REGW(USB_TEST_MODE_REG, 0x7);
        USB1REGW(USB_TEST_MODE_REG, 0x7);
        udelay(100000);

        //disable error bit
        psudo_case4(i,0);

        //enable error bit
        psudo_case4(i,1);
    }
    USB0REGW(USB_TEST_MODE_REG, 0x0);
    USB0REGW(USB_BIST_CFG_REG, 0x0);
    USB1REGW(USB_TEST_MODE_REG, 0x0);
    USB1REGW(USB_BIST_CFG_REG, 0x0);
}

static void usb_test_low(void)
{
    //Set to normal operation
    USB0REGW(USB_TEST_MODE_REG, 0x0);
    USB1REGW(USB_TEST_MODE_REG, 0x0);

    //reset RESULT2 for USB0/USB1
    REG_UPDATE32(RESULT2, 0, 0x2000);

    //Start test
    //std_pkt_usb();
    psudo_pkt_usb(FULL_SPEED);
}

static void usb_test_high(void)
{
    //Set to normal operation
    USB0REGW(USB_TEST_MODE_REG, 0x0);
    USB1REGW(USB_TEST_MODE_REG, 0x0);

    //Start test
    //std_pkt_usb();
    psudo_pkt_usb(HIGH_SPEED);
}
#endif

#ifdef EPHY_TEST
#define ETHPHY 2
static void eth_test(void)
{
#define UDELAY 1000
    int val1, val2;

    REG_WRITE32(RESULT1, 0);
    REG_WRITE32(0xbf004a58, 0x13f98047);

    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x14, 0x6000);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1f, 0x2);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x11, 0x8059);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x12, 0x8975);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x13, 0x6a60);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1f, 0x4);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x12, 0x5a40);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1b, 0xc0);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x10, 0xb5a0);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x11, 0xa528);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x12, 0x0f90);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x17, 0x6bbd);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x17, 0x6bbc);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x18, 0xf400);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x13, 0xa4d8);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x14, 0x3780);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x15, 0xb600);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x14, 0xb100);
    udelay(UDELAY);

    //ETH bist
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x13, 0xa000);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x19, 0x400);    //analog front end loopback
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x0, 0xa000);
    udelay(UDELAY);

    cm_mdio_wr(ETHPHY, 0x1f, 0x4);
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x10, 0x4);      //AFE loopback
    //cm_mdio_wr(ETHPHY, 0x10, 0x1);    //RJ45 loopback
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x11, 0x200); //Start bist
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x11, 0x8000);
    udelay(UDELAY);

    //verify
    val1 = cm_mdio_rd(ETHPHY, 0x11) & 0xffff;
    REG_WRITE32(RESULT1, val1);

    //force error
    cm_mdio_wr(ETHPHY, 0x11, 0xa00); //Force bist error and start
    udelay(UDELAY);
    cm_mdio_wr(ETHPHY, 0x11, 0x8000);
    udelay(UDELAY);

    //verify
    val2 = cm_mdio_rd(ETHPHY, 0x11) & 0xffff;
    REG_WRITE32(RESULT1, val1|val2<<16);
}
#endif

static void ip_bk(void)
{
#ifdef EPHY_TEST
    eth_test();
#endif

#ifdef DDR_TEST
    ddr_test();
#endif
}

static void usb_lf(void)
{
#ifdef USB_TEST
    usb_test_low();
#endif
}

static void usb_hi(void)
{
#ifdef USB_TEST
    usb_test_high();
#endif
}

static void rf_tst(char flag)
{
#ifdef WT_TEST
    if(acfg_p)
    {
        //tx
        if(flag == WIFI_TX)
        {
            if(acfg_p->start)
            {
                printf("Already start\n");
                return;
            }
            acfg_p->rx_echo = 0;

            acfg_p->tx_repeat = -1;
            acfg_p->start = 1;
            // clear counter
            bb_register_write(0, 0x80, 0xc0);
            // enable counter
            bb_register_write(0, 0x80, 0x80);

            thread_create(wla_test, (void *) 0, &ate_test_thread_stack[WLA_TEST_THREAD_STACK_SIZE], WLA_TEST_THREAD_STACK_SIZE);
            printf("TX Start\n");
        }
        //rx
        else if(flag == WIFI_RX)
        {
            if(acfg_p->start)
            {
                printf("Already start\n");
                return;
            }
            REG_WRITE32(RESULT4, 0);
            acfg_p->tx_repeat = 0;
            acfg_p->iteration = 0;

            acfg_p->rx_echo = 0;
            acfg_p->rx_drop = 1;
            acfg_p->start = 1;
            acfg_p->expect_count = 100;
            // clear counter
            bb_register_write(0, 0x80, 0xc0);
            // enable counter
            bb_register_write(0, 0x80, 0x80);
            thread_create(wla_test, (void *) 0, &ate_test_thread_stack[WLA_TEST_THREAD_STACK_SIZE], WLA_TEST_THREAD_STACK_SIZE);
            printf("RX Start\n",__func__);
        }
        else if(flag == WIFI_STOP)
        {
            unsigned long value;

            // disable counter
            bb_register_write(0, 0x80, 0x0);

            acfg_p->start = 0;
            acfg_p->indicate_stop = 1;

            // read ok counter high byte
            value = (bb_register_read(0, 0x89) << 8);
            // read ok counter low byte
            value |= bb_register_read(0, 0x8a);

            if(ate_cnt == 0)
                printf("No pkt recv\n");
            else
                REG_WRITE32(RESULT4, ((ate_rssi/ate_cnt)<<16)|ate_cnt);
            ate_rssi=0;
            ate_cnt=0;
        }
    }
#endif
}

extern int ai_test_ladda(void);
static void aud_tst(void)
{
#ifdef AUD_TEST
    ai_test_ladda();
#if 0
    //CH0_OP8
    REG_WRITE32(CLK_EN_CTRL, 0xffffffff);
    REG_WRITE32(ANA_TEST_CTRL, 0x15);
    REG_WRITE32(CPLL_REG, 0x30000f70);
    REG_WRITE32(AUDIO_ADC, 0x40000);
    REG_WRITE32(0xBF00704C, 0xc);

    //CH1_OP8
    REG_WRITE32(CLK_EN_CTRL, 0xffffffff);
    REG_WRITE32(ANA_TEST_CTRL, 0x15);
    REG_WRITE32(CPLL_REG, 0x30000f70);
    REG_WRITE32(AUDIO_ADC, 0x80000);
    REG_WRITE32(0xBF00704C, 0xc);

    //CH0_OP2
    REG_WRITE32(CLK_EN_CTRL, 0xffffffff);
    REG_WRITE32(ANA_TEST_CTRL, 0x15);
    REG_WRITE32(CPLL_REG, 0x30000f70);
    REG_WRITE32(AUDIO_ADC, 0x40010);
    REG_WRITE32(0xBF00704C, 0xc);

    //CH1_OP2
    REG_WRITE32(CLK_EN_CTRL, 0xffffffff);
    REG_WRITE32(ANA_TEST_CTRL, 0x15);
    REG_WRITE32(CPLL_REG, 0x30000f70);
    REG_WRITE32(AUDIO_ADC, 0x81000);
    REG_WRITE32(0xBF00704C, 0xc);
#endif
#endif
}

#define PKG_MODE_CTRL_REG 0xBF004AFC
#define ATE_ADC_CTRL_REG 0xBF004C2C
static void mad_tst_chani(void)
{
    //Chan I
    REG_WRITE32(PKG_MODE_CTRL_REG, 6<<4);
    REG_WRITE32(ATE_ADC_CTRL_REG, 1<<3);
}
static void mad_tst_chanq(void)
{
    //Chan Q
    REG_WRITE32(PKG_MODE_CTRL_REG, 6<<4);
    REG_WRITE32(ATE_ADC_CTRL_REG, 0<<3);
}

#if 0
static void reg_tst(void)
{
    unsigned int reg_offset[]=
    {
        0xbf0048FC,
        0xbf005510,
        0xbf005514,
        0xbf005520,
        0xbf005524,
        /*
        0x18,
        0x1c,
        0x24,
        0x28,
        0x2c,
        */
    };
    int i, j;
    for(i=0; i<sizeof(reg_offset)/4; i++)
    {
        REG_WRITE32(reg_offset[i],0);
        for(j=0; j<32; j++)
        {
            unsigned int val;
            REG_UPDATE32(reg_offset[i],1<<j,1<<j);
            //verify
            val = REG_READ32(reg_offset[i]);
            if((1<<j)&val)
                printf("Success: val:%x reg_offset:%x\n",val,reg_offset[i]);
            else {
                printf("Fail: val:%x reg_offset:%x j:%d\n",val,reg_offset[i],j);
                goto fail;
            }
        }
    }
    for(i=0; i<sizeof(reg_offset)/4; i++)
    {
        for(j=0; j<32; j++)
        {
            unsigned int val;
            REG_UPDATE32(reg_offset[i],0,1<<j);
            //verify
            val = REG_READ32(reg_offset[i]);
            if(!((1<<j)&val))
                printf("Success: val:%x reg_offset:%x\n",val,reg_offset[i]);
            else {
                printf("Fail: val:%x reg_offset:%x j:%d\n",val,reg_offset[i],j);
                goto fail;
            }
        }
    }
fail:
    return;
}
#endif

/*EPHY_DC*/
static void prep_eth_dc(void)
{
    REG_UPDATE32(0xBF004A58, 1<<19, 1<<19);
    REG_UPDATE32(0xBF004A58, 1<<24, 1<<24);
    REG_WRITE32(0xBF004C28, 0x13);
    REG_UPDATE32(0xBF004C88, 1<<11, 0x3800);
}

static void rx100_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x19, 0x9800);
    cm_mdio_wr(ETHPHY, 0x1b, 0x0);
    cm_mdio_wr(ETHPHY, 0x16, 0xc910);
}

static void vcm_ref_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    cm_mdio_wr(ETHPHY, 0x1b, 0xc0);
    cm_mdio_wr(ETHPHY, 0x19, 0xa000);
    cm_mdio_wr(ETHPHY, 0x11, 0x538);
}

static void vcm_tx_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    cm_mdio_wr(ETHPHY, 0x14, 0x4000);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    cm_mdio_wr(ETHPHY, 0x19, 0xb000);
    cm_mdio_wr(ETHPHY, 0x11, 0x538);
}

static void vbn_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    cm_mdio_wr(ETHPHY, 0x14, 0x4000);
    cm_mdio_wr(ETHPHY, 0x0, 0x2000);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    cm_mdio_wr(ETHPHY, 0x19, 0xb800);
    cm_mdio_wr(ETHPHY, 0x10, 0xf400);
}

static void vbnc_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    cm_mdio_wr(ETHPHY, 0x14, 0x4000);
    cm_mdio_wr(ETHPHY, 0x0, 0x2000);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    cm_mdio_wr(ETHPHY, 0x19, 0xc000);
    cm_mdio_wr(ETHPHY, 0x10, 0xf400);
}

static void vbp_cmfb_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    cm_mdio_wr(ETHPHY, 0x14, 0x4000);
    cm_mdio_wr(ETHPHY, 0x0, 0x2000);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    cm_mdio_wr(ETHPHY, 0x19, 0xc800);
    cm_mdio_wr(ETHPHY, 0x10, 0xf400);
}

static void vbp_tst(void)
{
    prep_eth_dc();
    cm_mdio_wr(ETHPHY, 0x1f, 0x0);
    cm_mdio_wr(ETHPHY, 0x14, 0x4000);
    cm_mdio_wr(ETHPHY, 0x0, 0x2000);
    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    cm_mdio_wr(ETHPHY, 0x19, 0xd000);
    cm_mdio_wr(ETHPHY, 0x10, 0xf400);
}
/*END EPHY_DC*/

/*DDR_DC*/
static void ddr_dc(void)
{
    unsigned int temp,temp_read_reg,init_read_reg;
    unsigned int cycle_ibias_gen, cycle_testcase;

    temp = ANAREG_READ32(CPLL_REG);
    temp = temp&~(7<<11);
    ANAREG_WRITE32(CPLL_REG, temp|1<<11);

    temp = ANAREG_READ32(ANA_TEST_CTRL);
    temp = temp&~(0x1f<<0);
    ANAREG_WRITE32(ANA_TEST_CTRL, temp|0x11);

    //REG_WRITE32(0xbf000858, 0x0);
    REG_WRITE32(0xbf000854, 0x0); //ok

    /*Normal Temp*/
    init_read_reg = REG_READ32(0xbf000858);
    temp_read_reg = init_read_reg & ~(1<<3);
    REG_WRITE32(0xbf000858, temp_read_reg);

    for(cycle_testcase = 0; cycle_testcase < 16; cycle_testcase++)
    {
        for(cycle_ibias_gen = 0; cycle_ibias_gen < 0x1; cycle_ibias_gen++)
        {
            if(cycle_testcase == 8)
            {
                while(polling)
                    udelay(100000);
                continue;
            }

            REG_WRITE32(0xbf000850, cycle_ibias_gen<<9);
            REG_WRITE32(0xbf000854, cycle_testcase<<2|1<<0);
        }
        if(cycle_testcase == 15)
            break;
        while(polling)
            udelay(100000);
        polling = 1;
    }
    polling = -1;
    thread_exit();
}
/*END DDR_DC*/

static void pll_clk(void)
{
    unsigned short rd;

    REG_UPDATE32(0xBF004C28, 0, 1<<0);
    REG_UPDATE32(0xBF004C88, 6<<11, 7<<11);
    REG_UPDATE32(0xBF004F0C, 0, 1<<29);
    REG_UPDATE32(0xBF004F04, 1<<7, 7<<7);
    REG_UPDATE32(0xBF004C88, 4<<20, 7<<20);

    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    rd = cm_mdio_rd(ETHPHY, 0x1b);
    rd =rd&~(0x20);
    cm_mdio_wr(ETHPHY, 0x1b, rd);

}

static void pll_ephy(void)
{
    unsigned short rd;

    REG_UPDATE32(0xBF004C28, 0, 1<<0);
    REG_UPDATE32(0xBF004C88, 4<<20, 7<<20);
    REG_UPDATE32(0xBF004C88, 6<<11, 7<<11);
    REG_UPDATE32(0xBF004F0C, 0, 1<<29);
    REG_UPDATE32(0xBF004F04, 0, 7<<7);

    cm_mdio_wr(ETHPHY, 0x1f, 0x1);
    rd = cm_mdio_rd(ETHPHY, 0x1b);
    rd = rd|(7<<3);
    cm_mdio_wr(ETHPHY, 0x1b, rd);

}

#define REG1_MIN 0xBF00485C
#define REG1_MAX 0xBF004864
#define REG2_MIN 0xBF00487C
#define REG2_MAX 0xBF0048A4
#define REG3_MIN 0xBF0048EC
#define REG3_MAX 0xBF004900
#define REG4_MIN 0xBF004F04
#define REG4_MAX 0xBF004F68

#define TEST_PATTERN1 0x55555555
#define TEST_PATTERN2 0xAAAAAAAA

#define SKIP_REG1 0xBF0048F0
#define SKIP_REG2 0xBF004F34

#define REG_REGION "0xBF00_485C <= address < 0xBF00_4864\n" \
                   "0xBF00_487C <= address < 0xBF00_48A4\n" \
                   "0xBF00_48EC <= address < 0xBF00_4900\n" \
                   "0xBF00_4F04 <= address < 0xBF00_4F68\n" \
                   "Skip 0xBF0048F0 and 0xBf004F34\n"

static int reg_test(reg)
{
    unsigned int reg_val;
   
    //printf("0x%x\n",reg); 
    REG_WRITE32(reg, TEST_PATTERN1);
    reg_val=REG_READ32(reg);
    if(reg_val != TEST_PATTERN1)
        goto fail_case1;

    REG_WRITE32(reg, TEST_PATTERN2);
    reg_val=REG_READ32(reg);
    if(reg_val != TEST_PATTERN2)
        goto fail_case2;
    
    return 0;

fail_case1:
    printf("Test Pattern %x Fail on reg:0x%x\n",TEST_PATTERN1, reg);
    return 1;
fail_case2:
    printf("Test Pattern %x Fail on reg:0x%x\n",TEST_PATTERN2, reg);
    return 1;
}

static void reg_test_all(void)
{
    unsigned int reg=0;
    int ret=0;

    /*Reset result*/
    REG_UPDATE32(RESULT2,0,0xF0000);
    
    for(reg=REG1_MIN; reg<REG1_MAX; reg+=4)
    {
        if((ret=reg_test(reg)))
        {
            REG_UPDATE32(RESULT2,0x10000,0x10000);
            break;
        }
    }
    
    for(reg=REG2_MIN; reg<REG2_MAX; reg+=4)
    {
        if((ret=reg_test(reg)))
        {
            REG_UPDATE32(RESULT2,0x20000,0x20000);
            break;
        }
    }
    
    for(reg=REG3_MIN; reg<REG3_MAX; reg+=4)
    {
        if(reg==SKIP_REG1)
            continue;
        if((ret=reg_test(reg)))
        {
            REG_UPDATE32(RESULT2,0x40000,0x40000);
            break;
        }
    }
    
    for(reg=REG4_MIN; reg<REG4_MAX; reg+=4)
    {
        if(reg==SKIP_REG2)
            continue;
        if((ret=reg_test(reg)))
        {
            REG_UPDATE32(RESULT2, 0x80000, 0x80000);
            break;
        }
    }

    return;
}

static void ate_scan_fail(unsigned int reg)
{
    if(reg < REG1_MIN)
        goto reg_fail;
    else if((reg >= REG1_MAX) && (reg < REG2_MIN))
        goto reg_fail;
    else if ((reg >= REG2_MAX) && (reg < REG3_MIN))
        goto reg_fail;
    else if((reg >= REG3_MAX) && (reg < REG4_MIN))
        goto reg_fail;
    else if(reg >= REG4_MAX)
        goto reg_fail;
    if((reg == SKIP_REG1) || (reg == SKIP_REG2))
        goto reg_fail;

    reg_test(reg);
    return ;
reg_fail:
    printf("Register is out of test range.\n");
    printf("%s\n",REG_REGION);
    return;
}

int ate_cmd(int argc, char *argv[])
{
    int pin;
#define CLK_ENABLE_CTRL_REG 0xBF004A58UL
#define GDMA_DISABLE_MASK   0x00000008UL
    REG_UPDATE32(CLK_ENABLE_CTRL_REG, GDMA_DISABLE_MASK, GDMA_DISABLE_MASK);

    addr_read(DDR_SIZE_INFO_ADDR,&ddr2_size);
    //printf("SRAM SIZE:%d\n",ddr2_size);
    if(argc >= 1)
    {
        if(!strcmp("prep", argv[0])) {
            pin_input_init();
            return ERR_OK;
        }
        else if(!strcmp("recover", argv[0])) {
            if(gpio_func_0_7 != 0)
                restore_pin_func();
            return ERR_OK;
        }

        /*Test Items*/
        if(!strcmp("dc_tst", argv[0]))
            dc_tst();
        else if(!strcmp("ip_tst", argv[0]))
            ip_tst();
        else if(!strcmp("wrt", argv[0]))
            wrt();
        else if(!strcmp("vol", argv[0]))
            vol();
        else if(!strcmp("voh", argv[0]))
            voh();
        else if(!strcmp("ts_md", argv[0]))
            ts_md();
        else if(!strcmp("ip_bk", argv[0]))
            ip_bk();
        else if(!strcmp("usb_lf", argv[0]))
            usb_lf();
        else if(!strcmp("usb_hi", argv[0]))
            usb_hi();
        else if(!strcmp("rf_tst", argv[0]))
        {
            if(argc >= 2)
            {
                if(!strcmp("tx", argv[1]))
                {
                    if(argc >= 3)
                    {
                        unsigned char chan = 1;
                        unsigned char s_chan = 0; //20
                        unsigned char tx_rate = 5;//b,g,n
                        chan = atoi(argv[2]);
                        if(argc >= 4)
                        {
                            s_chan = atoi(argv[3]);
                            if(argc == 5)
                                acfg_p->tx_rate = atoi(argv[4]);
                        }

                        printf("ch:%d s-chan:%d rate:%d\n",chan, s_chan, acfg_p->tx_rate);
                        ate_set_chan(chan, s_chan);
                    }
                    rf_tst(WIFI_TX);
                }
                else if(!strcmp("rx", argv[1]))
                {
                    if(argc >= 3)
                    {
                        unsigned char chan = 1;
                        unsigned char s_chan = 0; //20
                        chan = atoi(argv[2]);

                        if(argc == 4)
                            s_chan = atoi(argv[3]);
                        printf("ch:%d s_chan:%d\n",chan,s_chan);
                        ate_set_chan(chan, s_chan);
                    }
                    rf_tst(WIFI_RX);
                }
                else if(!strcmp("stop", argv[1]))
                    rf_tst(WIFI_STOP);
            }
        }
        else if(!strcmp("aud_tst", argv[0]))
            aud_tst();
        else if(!strcmp("mad_tst", argv[0]))
        {
            if(argc == 2)
            {
                if(!strcmp("0", argv[1]))
                    mad_tst_chani();
                else if(!strcmp("1", argv[1]))
                    mad_tst_chanq();
                else
                    printf("paremeter must be 0/1\n");
            }
        }
#if 0 //for test register write
        else if(!strcmp("reg", argv[0])) //test 10 regs
            reg_tst();
#endif
        else if(!strcmp("rx100_tst", argv[0]))
            rx100_tst();
        else if(!strcmp("vcm_ref_tst", argv[0]))
            vcm_ref_tst();
        else if(!strcmp("vcm_tx_tst", argv[0]))
            vcm_tx_tst();
        else if(!strcmp("vbn_tst", argv[0]))
            vbn_tst();
        else if(!strcmp("vbnc_tst", argv[0]))
            vbnc_tst();
        else if(!strcmp("vbp_cmfb_tst", argv[0]))
            vbp_cmfb_tst();
        else if(!strcmp("vbp_tst", argv[0]))
            vbp_tst();
        else if(!strcmp("ddr_dc", argv[0]))
        {
            if(polling == -1)
            {
                polling = 1;
                thread_create(ddr_dc, (void *) 0, &ate_test_thread_stack[WLA_TEST_THREAD_STACK_SIZE], WLA_TEST_THREAD_STACK_SIZE);
            }
            else
                polling = 0;
        }
        else if(!strcmp("pll", argv[0]))
        {
            if(argc == 2)
            {
                if(!strcmp("clk", argv[1]))
                    pll_clk();
                else if(!strcmp("ephy", argv[1]))
                    pll_ephy();
                else
                    printf("paremeter must be clk/ephy\n");
            }
        }
        else if(!strcmp("regt", argv[0]))
        {
            if(argc == 2)
            {
                unsigned int reg;
                sscanf(argv[1], "%x", &reg);
                ate_scan_fail(reg);
            }
            else
                reg_test_all();
        }
        else
            return ERR_HELP;
        return ERR_OK;
    }
    else
        return ERR_HELP;
}

cmdt cmdt_ate[] __attribute__ ((section("cmdt"))) =
{
    {
    "ate", ate_cmd, "Build date:" __DATE__" "__TIME__ ".\n"
            "ate: ATE Item\n"
            "\tate prep\n"
            "\tate dc_tst\n"
            "\tate ip_tst\n"
            "\tate wrt reg0\n"
            "\tate vol\n"
            "\tate voh\n"
            "\tate ts_md\n"
            "\tate ip_bk\n"
            "\tate usb_lf\n"
            "\tate usb_hi\n"
            "\tate rf_tst [tx/rx]\n"
            "\tate aud_tst\n"
            "\tate mad_tst\n"
            "\tate rx100_tst\n"
            "\tate vcm_ref_tst\n"
            "\tate vcm_tx_tst\n"
            "\tate vbn_tst\n"
            "\tate vbnc_tst\n"
            "\tate vbp_cmfb_tst\n"
            "\tate vbp_tst\n"
            "\tate ddr_dc\n"
            "\tate pll [clk/ephy]\n"
            "\tate regt\n"
            }
,};
#endif
