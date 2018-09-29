/*
 * init_vpe1.S
 *
 *  Initialize the second vpe and additional TCs
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

#include "boot.h"
#include "mt.h"
#include "regdef.h"
#include "m32c0.h"

#define a3_target_TC a3			// will hold the current TC being configured
#define VPE_1 1

	.set	noreorder           // Don't allow the assembler to reorder instructions.
	.set	noat                // Don't allow the assembler to use r1(at) for synthetic instr.
/**************************************************************************************
**************************************************************************************/
LEAF(init_vpe1)

#if !defined(PANTHER)
   // Each vpe will need to set up additional TC bound to it. (No rebinding.)

    beqz    r21_more_tcs, done_init_vpe1   // If there is no more TCs then return
    nop
    beqz    r20_more_vpes, done_init_vpe1   // If there is no vpe1 then return
    nop

#endif
	// This is executing on TC0 bound to VPE0.  Therefore VPEConf0.MVP is already set.
	// Enable Virtual Processor Configuration to enter configuration mode
	mfc0	v0, C0_MVPCONTROL	// read C0_MVPCtl
	or		v0, (1 << 1)		// set VPC to enable Virtual Processor Configuration
	mtc0	v0, C0_MVPCONTROL	// write C0_MVPCtl
	ehb

	// Initialize a3_target_TC, a3_target_TC will be incremented at the
	// bottom of the loop if there are more TCs
	li	a3_target_TC, 1

nexttc:

	// Set TargTC in the CP0 VPECONTROL register
	// TargTC  Selects the TC number of the "other thread context" for any
	// Move to Thread Context or Move from Thread Context instructions
	// (any instructions that begin with mtt or mft)

	mfc0	v0, C0_VPECONTROL		// read C0_VPECTL
	ins		v0, a3_target_TC, 0, 8	// insert TargTC
	mtc0	v0, C0_VPECONTROL		// write C0_VPECTL
	ehb

	// Halt a3_target_TC being configured
	li		v0, 1			// set Halt bit
	mttc0	v0, C0_TCHALT	// write C0_TCHALT
	ehb

	// Set up TCStatus register:
	// Disable Coprocessor Usable bits
	// Disable MDMX/DSP ASE
	// Clear Dirty a3_target_TC
	// not dynamically allocatable
	// not allocated
	// Kernel mode
	// interrupt exempt
	// ASID 0
	// NOTE: Only bit that needs to be set is IXMT
	li		v0, (1 << 10)	// set IXMT
	mttc0	v0, C0_TCSTATUS	// write C0_TCSTATUS of target TC

	// Initialize the a3_target_TC's register file (NOTE: v0 is in the gpr of the tc executing  this code)
	// NOTE: Good practice but not programmatically necessary
#if defined(PANTHER)
	mttgpr	$1, $1
	mttgpr	$2, $2
	mttgpr	$3, $3
	mttgpr	$4, $4
	mttgpr	$5, $5
	mttgpr	$6, $6
	mttgpr	$7, $7
	mttgpr	$8, $8
	mttgpr	$9, $9
	mttgpr	$10, $10
	mttgpr	$11, $11
	mttgpr	$12, $12
	mttgpr	$13, $13
	mttgpr	$14, $14
	mttgpr	$15, $15
	mttgpr	$16, $16
	mttgpr	$17, $17
	mttgpr	$18, $18
	mttgpr	$19, $19
	mttgpr	$20, $20
	mttgpr	$21, $21
	mttgpr	$22, $22
	mttgpr	$23, $23
	mttgpr	$24, $24
	mttgpr	$25, $25
	mttgpr	$26, $26
	mttgpr	$27, $27
	mttgpr	$28, $28
	mttgpr	$29, $29
	mttgpr	$30, $30
	mttgpr	$31, $31
#else
	li v0, 0xdeadbeef
	mttgpr	v0, $1
	mttgpr	v0, $2
	mttgpr	v0, $3
	mttgpr	v0, $4
	mttgpr	v0, $5
	mttgpr	v0, $6
	mttgpr	v0, $7
	mttgpr	v0, $8
	mttgpr	v0, $9
	mttgpr	v0, $10
	mttgpr	v0, $11
	mttgpr	v0, $12
	mttgpr	v0, $13
	mttgpr	v0, $14
	mttgpr	v0, $15
	mttgpr	v0, $16
	mttgpr	v0, $17
	mttgpr	v0, $18
	mttgpr	v0, $19
	mttgpr	v0, $20
	mttgpr	v0, $21
	mttgpr	v0, $22
	mttgpr	v0, $23
	mttgpr	v0, $24
	mttgpr	v0, $25
	mttgpr	v0, $26
	mttgpr	v0, $27
	mttgpr	v0, $28
	mttgpr	v0, $29
	mttgpr	v0, $30
	mttgpr	v0, $31
#endif

	// Bind TC1 and all other remaining TCs to VPE1 context
	// All mftc0 and mttc0 instructions
	// will then operate on the VPE1 CP0 registers
	li		v0, VPE_1
	mftc0   v1, C0_TCBIND	// Read C0_TCBind
	ins     v1, v0, 0, 4	// insert vpe 1 into CurVPE field
	mttc0   v1, C0_TCBIND	// write C0_TCBind

#if !defined(PANTHER)
// Must only do next part up to check_for_more_TC label once
	bne		v0, a3_target_TC, check_for_more_TC // branch if not TC1 NOTE: v0 set to 1 above
#endif
	nop

	// Set XTC for a3_target_TC (sets TC1 to be the only TC runnable on the VPE1)
	mftc0	v0, C0_VPECONF0	// read C0_VPECONF0
	ins		v0, a3_target_TC, 21, 8 // insert XTC
	mttc0	v0, C0_VPECONF0	// write C0_VPECONF0

	// Disable multi-threading for VPE1
	mftc0	v0, C0_VPECONTROL	// read C0_VPECTL
	ins		v0, zero, 15, 1		// clear TE (only tc1 can execute code)
	mttc0	v0, C0_VPECONTROL	// write C0_VPECTL

	// for VPE1  (Just Clear VPA to prevent any TC bound to it  from executing
	// and set master VPE so CP0 registers are writable
	mftc0	v0, C0_VPECONF0	// read C0_VPECONF0
	ins		v0, zero, 0, 1	// clear VPA
	or		v0, (1 << 1)	// set MVP
	mttc0	v0, C0_VPECONF0	// write C0_VPECONF0

	mfc0	v0, C0_STATUS	// read C0_STATUS
	mttc0	v0, C0_STATUS	// write C0_Status

#if defined(PANTHER)
	mfc0	v0, C0_EPC
	mttc0	v0, C0_EPC		// write C0_EPC
#else
	li		v0, 0x12345678
	mttc0	v0, C0_EPC		// write C0_EPC
#endif

	mttc0	zero, C0_CAUSE	// write C0_CAUSE

	mfc0	v0, C0_CONFIG	// read VPE 0 C0_CONFIG
	mttc0	v0, C0_CONFIG	// write it to VPE 1 C0_CONFIG

	mftc0	v0, C0_EBASE	// read C0_EBASE
	ext		v0, v0, 0, 10	// extract CPUNum
	mttgpr	v0, r23_cpu_num // write CPUNum to GPR 23 of target TC

	// vpe1 of each core can execute cached as it's L1 I$ has already been initialized.
    // and the L2$ has been initialized or "disabled" via CCA override.
#if defined(PANTHER)
	la		a1, main2
#else
	la		a1, __reset_vector // load boot code starting address
#endif

	#if !defined(EVA) && !defined(MPU)
		ins     a1, zero, 29, 1  // Convert to cached kseg0 address in case we linked to kseg1.
	#endif
	mttc0   a1, C0_TCRESTART // write C0_TCRestart so TC 1 on VPE 1
							 // will execute this boot code once EVP is set in C0_MVPCtl below

	// now set TC1 to take interrupts and set TC1 to active
	mftc0	v0, C0_TCSTATUS	// read C0_TCSTATUS
	ins	    v0, zero, 10, 1	// insert IXMT to enable this TC to take interrupts
	ori	    v0, (1 << 13)	// set A to Activate this TC
	mttc0	v0, C0_TCSTATUS	// write C0_TCSTATUS

	// For this boot code only TC1 bound to VPE1 will be needed to execute (unhalted)
	// The other TCs if any will remain in a Halted state assuming they will later be
	// started by the OS.
	// Clear H in TCHalt to unhalt this TC
	// NOTE will not execute until EVP is set in C0_MVPCtl below
	mttc0	zero, C0_TCHALT

#if !defined(PANTHER)
check_for_more_TC:	// Done initializing VPE 1 if there was one

	addu	a3_target_TC, 1					// advance TC number
	sltu	v1, r21_more_tcs, a3_target_TC  // set v1 if TC number is less than total # of TC in system
	beqz	v1, nexttc						// go back and initialize another TC
#endif
	nop

	mftc0	v0, C0_VPECONF0	// read C0_VPECONF0
	ori	    v0, 1		    // set VPA (Virtual Processor Activated)
	mttc0	v0, C0_VPECONF0	// write C0_VPECONF0 so this TC is able to execute once EVP is set in C0_MVPCtl below

	// Exit config mode
	mfc0	v0, C0_MVPCONTROL	// read C0_MVPCtl
    ori     v0, 1           	// set EVP (Enable Virtual Processing) to enable execution by vpe1
	ins	    v0, zero, 1, 1		// Clear VPC (VPE Configuration State) bit
	mtc0	v0, C0_MVPCONTROL	// write C0_MVPCtl
	ehb

done_init_vpe1:

    jr   ra
    nop
END(init_vpe1)

#undef a3_target_TC
#undef VPE_1
