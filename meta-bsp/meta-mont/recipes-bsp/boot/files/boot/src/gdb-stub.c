/*!
*   \file gdb-stub.c
*   \brief GDB support
*   \author
*/
/*
 *  arch/mips/kernel/gdb-stub.c
 *
 *  Originally written by Glenn Engel, Lake Stevens Instrument Division
 *
 *  Contributed by HP Systems
 *
 *  Modified for SPARC by Stu Grossman, Cygnus Support.
 *
 *  Modified for Linux/MIPS (and MIPS in general) by Andreas Busse
 *  Send complaints, suggestions etc. to <andy@waldorf-gmbh.de>
 *
 *  Copyright (C) 1995 Andreas Busse
 *
 * $Id: gdb-stub.c,v 1.7 1999/06/12 18:39:28 ulfc Exp $
 */

/*
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps() is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint().  Breakpoint()
 *  simulates a breakpoint by executing a BREAK instruction.
 *
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 */

#ifdef	CONFIG_GDB
#include <signal.h>
#include <arch/cpu.h>
#include <gdb-stub.h>
#include <inst.h>
#include <common.h>

unsigned strlen(const char * s);
char *strcpy(char * dest,const char *src);

#define cli()			local_irq_disable()
#define sti()			local_irq_enable()
#define save_flags(x)		local_save_flags(x)
#define restore_flags(x)	local_irq_restore(x)
#define save_and_cli(x)	        local_irq_save(x)
#define flush_cache_all()  \
do {				\
	dcache_flush(); \
	icache_inv();	\
} while (0);

#define  CAUSEB_CE              28
#define	 ST0_CU1                (1<<29)
#define  putDebugChar(c)		cheetah_putc(c)
#define  getDebugChar()			cheetah_getc()


void set_except_vector(int n, void *addr);

__asm__ (
	".macro\tlocal_irq_save result\n\t"
	".set\tpush\n\t"
	".set\treorder\n\t"
	".set\tnoat\n\t"
	"mfc0\t\\result, $12\n\t"
	"ori\t$1, \\result, 1\n\t"
	"xori\t$1, 1\n\t"
	".set\tnoreorder\n\t"
	"mtc0\t$1, $12\n\t"
	"sll\t$0, $0, 1\t\t\t# nop\n\t"
	"sll\t$0, $0, 1\t\t\t# nop\n\t"
	"sll\t$0, $0, 1\t\t\t# nop\n\t"
	".set\tpop\n\t"
	".endm");

#define local_irq_save(x)						\
__asm__ __volatile__(							\
	"local_irq_save\t%0"						\
	: "=r" (x)							\
	: /* no inputs */						\
	: "memory")

__asm__(".macro\tlocal_irq_restore flags\n\t"
	".set\tnoreorder\n\t"
	".set\tnoat\n\t"
	"mfc0\t$1, $12\n\t"
	"andi\t\\flags, 1\n\t"
	"ori\t$1, 1\n\t"
	"xori\t$1, 1\n\t"
	"or\t\\flags, $1\n\t"
	"mtc0\t\\flags, $12\n\t"
	"sll\t$0, $0, 1\t\t\t# nop\n\t"
	"sll\t$0, $0, 1\t\t\t# nop\n\t"
	"sll\t$0, $0, 1\t\t\t# nop\n\t"
	".set\tat\n\t"
	".set\treorder\n\t"
	".endm");

#define local_irq_restore(flags)						\
do {									\
	unsigned long __tmp1;						\
									\
	__asm__ __volatile__(						\
		"local_irq_restore\t%0"					\
		: "=r" (__tmp1)						\
		: "0" (flags)						\
		: "memory");						\
} while(0)

/* External low-level support routines. */
#if 0
extern int putDebugChar(char c);    /* Write a single character.      */
extern char getDebugChar(void);     /* Read and return a single char. */
#endif
extern void fltr_set_mem_err(void);
extern void trap_low(void);

/* Breakpoint and test functions. */
extern void breakpoint(void);
extern void breakinst(void);

/* Local prototypes. */
static void getpacket(char *buffer);
static void putpacket(char *buffer);
static int computeSignal(int tt);
static int hex(unsigned char ch);
static int hexToInt(char **ptr, int *intValue);
static unsigned char *mem2hex(char *mem, char *buf, unsigned count, int may_fault);
void handle_exception(struct gdb_regs *regs);

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound
 * buffers at least NUMREGBYTES*2 are needed for register packets.
 */
#define BUFMAX 2048

static char input_buffer[BUFMAX];
static char output_buffer[BUFMAX];
static int initialized;	/* !0 means we've been initialized */
/*
 * To run this GDB stub out of the preloaded instruction cache, it is
 * a requirement that we don't load from the text segment.  That
 * pretty much rules out string constants and initialized data.
 *
 * static const char hexchars[]="0123456789abcdef";
 */
static char hexchars[17];
static char OK[3];
static char E01[4], E02[4], E03[4];

/*
 * Convert ch from a hex digit to an int
 */
static int hex(unsigned char ch)
{
	if (ch >= 'a' && ch <= 'f')
		return ch-'a'+10;
	if (ch >= '0' && ch <= '9')
		return ch-'0';
	if (ch >= 'A' && ch <= 'F')
		return ch-'A'+10;
	return -1;
}

/*
 * scan for the sequence $<data>#<checksum>
 */
static void getpacket(char *buffer)
{
	unsigned char checksum;
	unsigned char xmitcsum;
	int i;
	int count;
	unsigned char ch;

	for (;;) {
		/*
		 * Wait around for the start character, ignore all
		 * other characters.
		 */
		while (getDebugChar() != '$');

		checksum = 0;
		xmitcsum = -1;
		count = 0;

		/* Now, read until a # or end of buffer is found. */
		while (count < BUFMAX) {
			ch = getDebugChar();
			if (ch == '#')
				break;
			checksum += ch;
			buffer[count++] = ch;
		}

		if (count >= BUFMAX) {
                        /* Nack the buffer overflow. */
                        putDebugChar('-');
			continue;
                }

		buffer[count] = 0;

		if (ch == '#') {
			xmitcsum = hex(getDebugChar()) << 4;
			xmitcsum |= hex(getDebugChar());

			if (checksum == xmitcsum) {
				putDebugChar('+'); /* Ack the successful transfer. */
                                return;
			} else {
                                /*
                                 * For the my debugging benefit,
                                 * return what we got and the expected
                                 * checksum.
                                 */
                                putDebugChar('<');
                                putDebugChar(hexchars[(count >> 12) & 15]);
                                putDebugChar(hexchars[(count >>  8) & 15]);
                                putDebugChar(hexchars[(count >>  4) & 15]);
                                putDebugChar(hexchars[ count        & 15]);
                                putDebugChar(':');
                                for (i = 0; i < count; ++i)
                                        putDebugChar(buffer[i]);
                                putDebugChar('#');
                                putDebugChar(hexchars[(checksum >> 4) & 15]);
                                putDebugChar(hexchars[ checksum       & 15]);
                                putDebugChar('>');
			}
		}
                putDebugChar('-');	/* Failed, Nack it. */
	}
}

/* Send the packet in buffer. */
static void putpacket(char *buffer)
{
	unsigned char checksum;
	int count;
	unsigned char ch;

	/* $<packet info>#<checksum>. */
	for (;;) {
		putDebugChar('$');
		checksum = 0;
		count = 0;

		while ((ch = buffer[count])) {
			putDebugChar(ch);
			checksum += ch;
			count++;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);

                /* Wait for ACK or NACK, ignore everything else. */
                do {
                        ch = getDebugChar();
                        if (ch == '+')
                                return;
                } while (ch != '-');
	}
}

#if 0
/*
 * Indicate to caller of mem2hex or hex2mem that there
 * has been an error.
 */
static volatile int mem_err = 0;

static void set_mem_fault_trap(int enable)
{
  mem_err = 0;

#if 0
  if (enable)
    exceptionHandler(9, fltr_set_mem_err);
  else
    exceptionHandler(9, trap_low);
#endif
}
#endif

/*
 * Convert the memory pointed to by mem into hex, placing result in
 * buf.  Return a pointer to the last char put in buf (null), in case
 * of mem fault, return 0.  If MAY_FAULT is non-zero, then we will
 * handle memory faults by returning a 0, else treat a fault like any
 * other fault in the stub.
 */
static unsigned char *mem2hex(char *mem, char *buf, unsigned count, int may_fault)
{
	unsigned char ch;

/*	set_mem_fault_trap(may_fault); */

	while (count--) {
		ch = *mem++;
//		if (mem_err)
//			return 0;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}

	*buf = 0;

/*	set_mem_fault_trap(0); */

	return buf;
}

/*
 * Convert the hex array pointed to by buf into binary to be placed in
 * mem return a pointer to the character AFTER the last byte written.
 */
static char *hex2mem(char *buf, char *mem, int count, int may_fault)
{
	int i;
	unsigned char ch;

/*	set_mem_fault_trap(may_fault); */

	for (i = 0; i < count; i++) {
		ch = hex(*buf++) << 4;
		*mem++ = ch | hex(*buf++);
//		if (mem_err)
//			return 0;
	}

/*	set_mem_fault_trap(0); */

	return mem;
}

/*
 * Safe hex2mem: this will work even if we're given less than
 * expected.  Instead of returning the destination buffer, it returns
 * the next byte in the output.  If a non-hex byte is encountered, it
 * will stop there.
 */
static int ishexdigit(unsigned char ch)
{
	return ('a' <= ch && ch <= 'z') ||
	       ('A' <= ch && ch <= 'Z') ||
	       ('0' <= ch && ch <= '9');
}

static char *safe_hex2mem(char *buf, char *mem, int count)
{
	int i;

	for (i = 0; i < count; ++i) {
		if (!ishexdigit(buf[0]) || !ishexdigit(buf[1]))
			break;
		*mem++ = (hex(buf[0]) << 4) | hex(buf[1]);
		buf += 2;
	}

	return buf;
}

/*
 * This table contains the mapping between SPARC hardware trap types, and
 * signals, which are primarily what GDB understands.  It also indicates
 * which hardware traps we need to commandeer when initializing the stub.
 */
static struct hard_trap_info
{
	unsigned char tt;		/* Trap type code for MIPS R3xxx and R4xxx */
	unsigned char signo;		/* Signal that we map this trap into */
} hard_trap_info[] = {
	{ 4, SIGBUS },			/* address error (load) */
	{ 5, SIGBUS },			/* address error (store) */
	{ 6, SIGBUS },			/* instruction bus error */
	{ 7, SIGBUS },			/* data bus error */
	{ 9, SIGTRAP },			/* break */
	{ 10, SIGILL },			/* reserved instruction */
/*	{ 11, SIGILL },		*/	/* CPU unusable */
	{ 12, SIGFPE },			/* overflow */
	{ 13, SIGTRAP },		/* trap */
	{ 14, SIGSEGV },		/* virtual instruction cache coherency */
	{ 15, SIGFPE },			/* floating point exception */
	{ 23, SIGSEGV },		/* watch */
	{ 31, SIGSEGV },		/* virtual data cache coherency */
	{ 0, 0}				/* Must be last */
};


/* Set up exception handlers for tracing and breakpoints. */
void set_debug_traps(void)
{
	struct hard_trap_info *ht;
	unsigned long flags;

	save_and_cli(flags);
/*
	for (ht = hard_trap_info; ht->tt && ht->signo; ht++)
		set_except_vector(ht->tt, trap_low);
*/
	/*
	 * In case GDB is started before us, nack any packets to cause
	 * a retransmit.
	 */
	putDebugChar('-'); /* Nack it. */
	while (getDebugChar() != '$');
	while (getDebugChar() != '#');
	getDebugChar(); /* Eat first csum byte. */
	getDebugChar(); /* Eat second csum byte. */
	putDebugChar('-'); /* Nack it. */

	initialized = 1;
	restore_flags(flags);
}


/*
 * Trap handler for memory errors.  This just sets mem_err to be non-zero.  It
 * assumes that %l1 is non-zero.  This should be safe, as it is doubtful that
 * 0 would ever contain code that could mem fault.  This routine will skip
 * past the faulting instruction after setting mem_err.
 */
extern void fltr_set_mem_err(void)
{
  /* FIXME: Needs to be written... */
}

/*
 * Convert the MIPS hardware trap type code to a Unix signal number.
 */
static int computeSignal(int tt)
{
	struct hard_trap_info *ht;

	for (ht = hard_trap_info; ht->tt && ht->signo; ht++)
		if (ht->tt == tt)
			return ht->signo;

	return SIGTRAP;		/* Default for things we don't know about. */
}

/*
 * While we find nice hex chars, build an int.  Return number of chars
 * processed.
 */
static int hexToInt(char **ptr, int *intValue)
{
	int numChars = 0;
	int hexValue;

	*intValue = 0;

	while (**ptr) {
		hexValue = hex(**ptr);
		if (hexValue < 0)
			break;

		*intValue = (*intValue << 4) | hexValue;
		numChars ++;

		(*ptr)++;
	}

	return numChars;
}


/*
 * We single-step by setting breakpoints. When an exception
 * is handled, we need to restore the instructions hoisted
 * when the breakpoints were set.
 *
 * This is where we save the original instructions.
 */
static struct gdb_bp_save {
	unsigned int addr;
        unsigned int val;
} step_bp[2];

#define BP 0x0000000d  /* The opcode for the break instruction. */

/* Set breakpoint instructions for single stepping. */
static void single_step(struct gdb_regs *regs)
{
	union mips_instruction insn;
	unsigned int targ;
	int is_branch, is_cond, i;

	targ = regs->cp0_epc;
	insn.word = *(unsigned int *)targ;
	is_branch = is_cond = 0;

	if (insn.i_format.opcode == spec_op) {
                /*
                 * jr and jalr are in r_format format.
                 */
                if (insn.r_format.func == jalr_op ||
                    insn.r_format.func == jr_op) {
			targ = *(&regs->reg0 + insn.r_format.rs);
			is_branch = 1;
		}
        } else if (insn.i_format.opcode == bcond_op) {
                /*
                 * This group contains:
                 * bltz_op, bgez_op, bltzl_op, bgezl_op,
                 * bltzal_op, bgezal_op, bltzall_op, bgezall_op.
                 */
		is_branch = is_cond = 1;
		targ += 4 + (insn.i_format.simmediate << 2);
        } else if (insn.i_format.opcode == jal_op ||
                   insn.i_format.opcode == j_op) {
                /*
                 * These are unconditional and in j_format.
                 */
		is_branch = 1;
		targ += 4;
		targ >>= 28;
		targ <<= 28;
		targ |= (insn.j_format.target << 2);
        } else if (insn.i_format.opcode == beq_op ||
                   insn.i_format.opcode == beq_op ||
                   insn.i_format.opcode == beql_op ||
                   insn.i_format.opcode == bne_op ||
                   insn.i_format.opcode == bnel_op ||
                   insn.i_format.opcode == blez_op ||
                   insn.i_format.opcode == blezl_op ||
                   insn.i_format.opcode == bgtz_op ||
                   insn.i_format.opcode == bgtzl_op ||
                   insn.i_format.opcode == cop0_op ||
                   insn.i_format.opcode == cop1_op ||
                   insn.i_format.opcode == cop2_op ||
                   insn.i_format.opcode == cop1x_op) {
                /* These are conditional. */
		is_branch = is_cond = 1;
		targ += 4 + (insn.i_format.simmediate << 2);
	}

	if (is_branch) {
		i = 0;
		if (is_cond && targ != (regs->cp0_epc + 8)) {
			step_bp[i].addr = regs->cp0_epc + 8;
			step_bp[i++].val = *(unsigned *)(regs->cp0_epc + 8);
			*(unsigned *)(regs->cp0_epc + 8) = BP;
		}
		step_bp[i].addr = targ;
		step_bp[i].val  = *(unsigned *)targ;
		*(unsigned *)targ = BP;
	} else {
		step_bp[0].addr = regs->cp0_epc + 4;
		step_bp[0].val  = *(unsigned *)(regs->cp0_epc + 4);
		*(unsigned *)(regs->cp0_epc + 4) = BP;
	}

}

/*
 *  If asynchronously interrupted by gdb, then we need to set a breakpoint
 *  at the interrupted instruction so that we wind up stopped with a
 *  reasonable stack frame.
 */
static struct gdb_bp_save async_bp;

void set_async_breakpoint(unsigned long *epc)
{
	async_bp.addr = (unsigned) epc;
	async_bp.val  = *epc;
	*(unsigned *)epc = BP;
	flush_cache_all();
}


/*
 * This function does all command processing for interfacing to gdb.  It
 * returns 1 if you should skip the instruction at the trap address, 0
 * otherwise.
 */
void handle_exception (struct gdb_regs *regs)
{
	int trap;			/* Trap type */
	int sigval;
	int addr;
	int length;
	char *ptr;
	unsigned long *stack;

        {
                /*
                 * This GDB stub is a bit different from normal
                 * programs in that it runs out of a preloaded
                 * instruction cache, but no data is loaded.  Thus, we
                 * can't use any constants, like string constrants.
                 */

                int i;
                for (i = 0; i < 16; ++i)
                        hexchars[i] = i < 10 ? '0' + i : 'a' + i - 10;

                OK[0] = 'O';
                OK[1] = 'K';
                OK[2] = 0;
                E01[0] = 'E';
                E01[1] = '0';
                E01[2] = '1';
                E01[3] = 0;
                E02[0] = 'E';
                E02[1] = '0';
                E02[2] = '2';
                E02[3] = 0;
                E03[0] = 'E';
                E03[1] = '0';
                E03[2] = '3';
                E03[3] = 0;
        }

	/*
	 * First check trap type. If this is CPU_UNUSABLE and CPU_ID is 1,
	 * the simply switch the FPU on and return since this is no error
	 * condition. kernel/traps.c does the same.
	 * FIXME: This doesn't work yet, so we don't catch CPU_UNUSABLE
	 * traps for now.
	 */
	trap = (regs->cp0_cause & 0x7c) >> 2;
	if (trap == 11) {
		if (((regs->cp0_cause >> CAUSEB_CE) & 3) == 1) {
			regs->cp0_status |= ST0_CU1;
			return;
		}
	}

	/*
	 * If we're in breakpoint() increment the PC
	 */
	if (trap == 9 && regs->cp0_epc == (unsigned long)breakinst)
		regs->cp0_epc += 4;

	/*
	 * If we were single_stepping, restore the opcodes hoisted
	 * for the breakpoint[s].
	 */
	if (step_bp[0].addr) {
		*(unsigned *)step_bp[0].addr = step_bp[0].val;
		step_bp[0].addr = 0;

		if (step_bp[1].addr) {
			*(unsigned *)step_bp[1].addr = step_bp[1].val;
			step_bp[1].addr = 0;
		}
	}

	/*
	 * If we were interrupted asynchronously by gdb, then a
	 * breakpoint was set at the EPC of the interrupt so
	 * that we'd wind up here with an interesting stack frame.
	 */
	if (async_bp.addr) {
		*(unsigned *)async_bp.addr = async_bp.val;
		async_bp.addr = 0;
	}

	stack = (long *)regs->reg29;
	sigval = computeSignal(trap);

	/* Reply to host that an exception has occurred. */
	ptr = output_buffer;

	/* Send trap type (converted to signal). */
	*ptr++ = 'T';
	*ptr++ = hexchars[sigval >> 4];
	*ptr++ = hexchars[sigval & 0xf];

	/* Send Error PC. */
	*ptr++ = hexchars[REG_EPC >> 4];
	*ptr++ = hexchars[REG_EPC & 0xf];
	*ptr++ = ':';
	ptr = mem2hex((char *)&regs->cp0_epc, ptr, 4, 0);
	*ptr++ = ';';

	/* Send frame pointer. */
	*ptr++ = hexchars[REG_FP >> 4];
	*ptr++ = hexchars[REG_FP & 0xf];
	*ptr++ = ':';
	ptr = mem2hex((char *)&regs->reg30, ptr, 4, 0);
	*ptr++ = ';';

	/* Send stack pointer. */
	*ptr++ = hexchars[REG_SP >> 4];
	*ptr++ = hexchars[REG_SP & 0xf];
	*ptr++ = ':';
	ptr = mem2hex((char *)&regs->reg29, ptr, 4, 0);
	*ptr++ = ';';

	*ptr++ = 0;

	putpacket(output_buffer);	/* Send it off... */

	/* Wait for input from remote GDB. */
	for (;;) {
		output_buffer[0] = 0;
		getpacket(input_buffer);

                /* Required command are g, G, m, M, c, and s. */
                if (input_buffer[0] == '?') {
			output_buffer[0] = 'S';
			output_buffer[1] = hexchars[sigval >> 4];
			output_buffer[2] = hexchars[sigval & 0xf];
			output_buffer[3] = 0;
                } else if (input_buffer[0] == 'd') {
			/* toggle debug flag */
                } else if (input_buffer[0] == 'g') {
                        /* Return the value of the CPU registers */
			ptr = output_buffer;
			ptr = mem2hex((char *)&regs->reg0, ptr, 32*4, 0); /* r0...r31 */
			ptr = mem2hex((char *)&regs->cp0_status, ptr, 6*4, 0); /* cp0 */
			//ptr = mem2hex((char *)&regs->fpr0, ptr, 32*4, 0); /* f0...31 */
			//ptr = mem2hex((char *)&regs->cp1_fsr, ptr, 2*4, 0); /* cp1 */
			//ptr = mem2hex((char *)&regs->frame_ptr, ptr, 2*4, 0); /* frp */
			//ptr = mem2hex((char *)&regs->cp0_index, ptr, 16*4, 0); /* cp0 */
                } else if (input_buffer[0] == 'G') {
                        /*
                         * Set the value of the CPU registers - return OK
                         * FIXME: Needs to be written
                         */
			// unsigned long *newsp, psr;

			ptr = &input_buffer[1];

			ptr = safe_hex2mem(ptr, (char *)&regs->reg0, 32*4); /* r0...r31 */
			ptr = safe_hex2mem(ptr, (char *)&regs->cp0_status, 6*4); /* cp0 */
			//ptr = safe_hex2mem(ptr, (char *)&regs->fpr0, 32*4); /* f0...31 */
			//ptr = safe_hex2mem(ptr, (char *)&regs->cp1_fsr, 2*4); /* cp1 */
			//ptr = safe_hex2mem(ptr, (char *)&regs->frame_ptr, 2*4); /* frp */
			//ptr = safe_hex2mem(ptr, (char *)&regs->cp0_index, 16*4); /* cp0 */

#if 0
			/*
			 * See if the stack pointer has moved. If so, then copy the
			 * saved locals and ins to the new location.
			 */

			newsp = (unsigned long *)registers[SP];
			if (sp != newsp)
				sp = memcpy(newsp, sp, 16 * 4);
#endif

			strcpy(output_buffer, OK);
		} else if (input_buffer[0] == 'm') {
                        /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
			ptr = &input_buffer[1];

			if (hexToInt(&ptr, &addr)
                            && *ptr++ == ','
                            && hexToInt(&ptr, &length)) {
				if (!mem2hex((char *)addr, output_buffer, length, 1))
                                        strcpy(output_buffer, E03);
			} else
				strcpy(output_buffer, E01);
		} else if (input_buffer[0] == 'M') {
                        /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK. */
			ptr = &input_buffer[1];

			if (hexToInt(&ptr, &addr)
                            && *ptr++ == ','
                            && hexToInt(&ptr, &length)
                            && *ptr++ == ':') {
				if (hex2mem(ptr, (char *)addr, length, 1)) {
					strcpy(output_buffer, OK);
                                } else
					strcpy(output_buffer, E03);
			} else {
				strcpy(output_buffer, E02);
                        }
		} else if (input_buffer[0] == 'c') {
                        /* cAA..AA    Continue at address AA..AA(optional). */
			/* try to read optional parameter, pc unchanged if no parm */

			ptr = &input_buffer[1];
			if (hexToInt(&ptr, &addr))
				regs->cp0_epc = addr;

			/*
			 * Need to flush the instruction cache here, as we may
			 * have deposited a breakpoint, and the icache probably
			 * has no way of knowing that a data ref to some location
			 * may have changed something that is in the instruction
			 * cache.
			 * NB: We flush both caches, just to be sure...
			 */

			flush_cache_all();
			return;
                } else if (input_buffer[0] == 'k') {
                        /*
                         * kill the program
                         */
                        /* do nothing */
                } else if (input_buffer[0] == 'r') {
                        /*
                         * Reset the whole machine (FIXME: system dependent)
                         */
                } else if (input_buffer[0] == 's') {
                        /*
                         * Step to next instruction
                         */
			/*
			 * There is no single step insn in the MIPS ISA, so we
			 * use breakpoints and continue, instead.
			 */
			single_step(regs);
			flush_cache_all();
			return;
			/* NOTREACHED */
                } else {
                        /*
                         * According to the GDB manual, for any
                         * command not supported by the stub, an empty
                         * response should be retuned.
                         */
		}

		/* Reply to the request. */
		putpacket(output_buffer);
	}
}

/*
 * This function will generate a breakpoint exception.  It is used at the
 * beginning of a program to sync up with a debugger and can be used
 * otherwise as a quick means to stop program execution and "break" into
 * the debugger.
 */
void breakpoint(void)
{
#if 0
	if (!initialized)
		return;
#endif
	__asm__ __volatile__(
			".globl	breakinst\n"
			".set	noreorder\n"
			"nop\n"
"breakinst:		break\n"
			"nop\n"
			".set	reorder\n");
}

int cmd_gdb(int argc, char *argv[])
{
    unsigned long addr, val=0;

    if ( 1 > argc )
        goto help;
	if ( argv[0][0] == 'i')
    	set_debug_traps();
	else if ( argv[0][0] != 'b')
        goto err1;

    if (( 1 < argc ) && (!hextoul(argv[1], &val)))
        goto err1;

	if (!val)
		breakpoint();
	else
	{
		printf("set bp=%x\n", val);
		set_async_breakpoint((unsigned long *)val);
	}
    return ERR_OK;

  help:
    return ERR_HELP;
  err1:
    return ERR_PARM;

}

cmdt cmdt_gdb[] __attribute__ ((section("cmdt"))) =
{
    { "gdb", cmd_gdb, "gdb i/b <addr>			; gdb break"} ,
};
#endif //CONFIG_GDB
