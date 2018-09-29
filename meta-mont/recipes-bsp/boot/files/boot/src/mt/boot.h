/*
 * boot.h
 *
*/
/*
Copyright (c) 2015, Imagination Technologies Limited and/or its affiliated group companies

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list
of conditions and the following disclaimer in the documentation and/or other materials
provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be
used to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/



#define LEAF(name)\
    .##text;\
    .##globl    name;\
    .##ent  name;\
name:

#define END(name)\
    .##size name,.-name;\
    .##end  name
#ifdef EVA // use direct mapping
	#define GCR_CONFIG_ADDR     0x1fbf8000  // Boot address of the GCR registers
	#define GCR_CONFIG_ADDR_PB  0xbfbf8000  // Post Boot address of the GCR registers
	#define GIC_P_BASE_ADDR		0x1bdc0000  // physical address of the GIC
	#define GIC_BASE_ADDR		0x1bdc0000  // Boot address address of the GIC
	#define GIC_BASE_ADDR_PB	0xbbdc0000	// Post Boot address address of the GIC
	#define CPC_P_BASE_ADDR 	0x1bde0001  // physical address of the CPC including enable bit
	#define CPC_BASE_ADDR		0x1bde0000  // Boot address address of the CPC
	#define CPC_BASE_ADDR_PB	0xbbde0000  // Post Boot address address of the CPC

	#define DENALI_CTL_SECTION  0x1bc00000
	#define MALTA_DISP_ADDR     0x1f000410
	#define MALTA_DISP_CLEAR	0x1f00
    #define STACK_BASE_ADDR     0x02000000  /* Change: Base on memory size. */
#elif defined MPU
  //Sherlock Change the VA address = PA under MPU enable
	#define GCR_CONFIG_ADDR     0x1fbf8000  // MPU Direct map address of the GCR registers
	#define GCR_CONFIG_ADDR_PB  0x1fbf8000  // Post Boot address of the GCR registers
	#define GIC_P_BASE_ADDR		0x1bdc0000  // physical address of the GIC
	#define GIC_BASE_ADDR		0x1bdc0000  // MPU Direct address address of the GIC
	#define GIC_BASE_ADDR_PB	0x1bdc0000	// Post Boot address address of the GIC
	#define CPC_P_BASE_ADDR 	0x1bde0001  // physical address of the CPC
	#define CPC_BASE_ADDR		0x1bde0000  // MPU Direct address address of the CPC
	#define CPC_BASE_ADDR_PB	0x1bde0000  // Post Boot address address of the CPC

	#define DENALI_CTL_SECTION  0x1bc00000
	#define MALTA_DISP_ADDR     0x1f000410
	#define MALTA_DISP_CLEAR	0x1f00
	#define CORE_FPGA6_MIG		0x1bc00004
	#define STACK_BASE_ADDR     0x02000000  /* Change: Base on memory size. */

	//Sherlock add CDMM address & MPU offset define
	#define CDMM_P_BASE_ADDR		0x1fc10000  // physical address of the CDMM Regsiter
	#define MPU_CDMM_OFFSET     (64*3)
	#define MPU_ACSR            0
	#define MPU_Config          0x8
	#define MPU_SegmentControl0 0x10
	#define MPU_SegmentControl1 0x14
	#define MPU_SegmentControl2 0x18
	#define MPU_SegmentControl3 0x1c
#else
	#define GCR_CONFIG_ADDR     0xbfbf8000  // KSEG1 address of the GCR registers
	#define GCR_CONFIG_ADDR_PB  0xbfbf8000  // Post Boot address of the GCR registers
	#define GIC_P_BASE_ADDR		0x1bdc0000  // physical address of the GIC
	#define GIC_BASE_ADDR		0xbbdc0000  // KSEG1 address address of the GIC
	#define GIC_BASE_ADDR_PB	0xbbdc0000	// Post Boot address address of the GIC
	#define CPC_P_BASE_ADDR 	0x1bde0001  // physical address of the CPC including enable bit
	#define CPC_BASE_ADDR		0xbbde0000  // KSEG1 address address of the CPC
	#define CPC_BASE_ADDR_PB	0xbbde0000  // Post Boot address address of the CPC

	#define MALTA_DISP_ADDR     0xbf000410
	#define MALTA_DISP_CLEAR	0xbf00

    #define STACK_BASE_ADDR     0x82000000  /* Change: Base on memory size. */
#endif


#define STACK_SIZE_LOG2     22          /* 4Mbytes each */


/**************************************************************************************
 Register use while executing in this file: ("GLOBAL" denotes a common value.)
**************************************************************************************/

#define r1_all_ones     $1   /* at Will hold 0xffffffff to simplify bit insertion of 1's. GLOBAL! */

// $2 - $7 (v0, v1 a0 - a3) reserved for program use

#define r8_core_num    $8  /* t0 Core number. Only core 0 is active after reset. */
#define r9_vpe_num     $9  /* t1 MT ASE VPE number that this TC is bound to (0 if non-MT.) */
#define r10_has_mt_ase  $10   /* t2 Core implements the MT ASE. */
#define r11_is_cps      $11   /* t3 Core is part of a Coherent Processing System. */

// $12 - $15 (t4 - t7 o32 or t0 - t3 n32/64)  are free to use
// $16, $17 (s0 and s1) are free to use

#define r18_tc_num      $18  /* s2 MT ASE TC number (0 if non-MT.) */
#define r19_more_cores  $19  /* s3 Number of cores in CPS in addition to core 0. GLOBAL! */
#define r20_more_vpes   $20  /* s4 Number of vpes in this core in addition to vpe 0. */
#define r21_more_tcs    $21  /* s5 Number of tcs in vpe in addition to the first. */
#define r22_gcr_addr    $22  /* s6 Uncached (kseg1) base address of the Global Config Registers. */
#define r23_cpu_num     $23  /* s7 Unique per vpe "cpu" identifier (CP0 EBase[CPUNUM]). */
#define r24_malta_word  $24  /* t8 Uncached (kseg1) base address of Malta ascii display. GLOBAL! */
#define r25_coreid      $25  /* t9 Copy of cp0 PRiD GLOBAL! */
#define r26_int_addr    $26  /* k0 Interrupt handler scratch address. */
#define r27_int_data    $27  /* k1 Interrupt handler scratch data. */
// $28 gp and $29 sp
#define r30_cpc_addr    $30  /* s8 Address of CPC register block after cpc_init. 0 indicates no CPC. */
// $31 ra




