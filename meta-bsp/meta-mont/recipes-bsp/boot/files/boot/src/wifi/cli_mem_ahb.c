/*=============================================================================+
|                                                                              |
| Copyright 2012                                                               |
| Montage Inc. All right reserved.                                             |
|                                                                              |
+=============================================================================*/
/*! 
*   \file 
*   \brief 
*   \author Montage
*/

/*=============================================================================+
| Included Files
+=============================================================================*/
#include <lib.h>
#include <bb.h>
#include <rf.h>
#include <cli_api.h>
#include <panther_debug.h>
#include <os_compat.h>
#include <send_pkt.h>

int printf(char *fmt, ...);
/*=============================================================================+
| Define
+=============================================================================*/
#define BUF_ADDR_BOUNDARY       0x80000000
#define STEP_BYTE 		sizeof(char)
#define STEP_HWORD 		sizeof(short)
#define STEP_WORD 		sizeof(int)
#define BYTES_PER_LINE 	16
#define MAX_BYTES_PER_DUMP	1024
#define DEF_BYTES_PER_DUMP	128
#define VBYTE volatile unsigned char *
#define VHWORD volatile unsigned short *
#define VWORD volatile unsigned long *
/*=============================================================================+
| Function Prototypes
+=============================================================================*/

/*=============================================================================+
| Extern Function/Variables
+=============================================================================*/

/*=============================================================================+
| Variables
+=============================================================================*/

//static unsigned int buf_address = BUF_ADDR_BOUNDARY;

/*=============================================================================+
| Functions
+=============================================================================*/

#if 0
/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
static int step_sz(char c)
{
	int step;
	switch (c)
	{
	case 'b':
		step = STEP_BYTE;
		break;

	case 'h':
		step = STEP_HWORD;
		break;

	default:
		step = STEP_WORD;
	}
	return step;
}
#endif

/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
//int hextoul(char *str, void *v)
//{
//    return (sscanf(str, "%x", v) == 1);
//}

/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
#ifdef CONFIG_TODO
int mem_dump_cmd(int argc, char *argv[])
{
	unsigned long addr, caddr;
	unsigned long size;
	int step;

	step = step_sz(argv[-1][1]);
	addr = buf_address;
	size = DEF_BYTES_PER_DUMP;
	if (argc > 0 && !hextoul(argv[0], &addr))
		goto help;
	if (argc > 1 && !hextoul(argv[1], &size))
		goto help;

	if (size > MAX_BYTES_PER_DUMP)
		size = MAX_BYTES_PER_DUMP;

	addr &= ~(step - 1);
	if (vaddr_check(addr))
		goto err2;
	caddr = addr;
	while (caddr < (addr + size))
	{
		if (caddr % BYTES_PER_LINE == 0 || caddr == addr)
		{
			printf("\n%08x: ", caddr);
		}
		switch (step)
		{
		case STEP_BYTE:
			printf("%02x ", *((VBYTE) caddr));
			break;
		case STEP_HWORD:
			printf("%04x ", *((VHWORD) caddr));
			break;
		case STEP_WORD:
			printf("%08x ", *((VWORD) caddr));
			break;
		}
		caddr += step;
	}
	buf_address = addr + size;
	printf("\n\r");
	return 0;

  err2:
	return CLI_ERR_ADDR;

  help:
	return CLI_SHOW_USAGE;
}

/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
int mem_enter_cmd(int argc, char *argv[])
{
	unsigned long addr;
	unsigned long size, val, i;
	int step;

	step = step_sz(argv[-1][1]);

	size = STEP_BYTE;
	if (2 > argc)
		goto help;

	if (!hextoul(argv[0], &addr))
		goto err1;
	if (!hextoul(argv[1], &val))
		goto err1;
	if (argc > 2 && !hextoul(argv[2], &size))
		goto err1;

	addr &= ~(step - 1);
	if (vaddr_check(addr))
		goto err2;
	for (i = 0; i < size; i += step)
	{
		switch (step)
		{
		case STEP_BYTE:
			*(VBYTE) (addr + i) = val;
			break;
		case STEP_HWORD:
			*(VHWORD) (addr + i) = val;
			break;
		case STEP_WORD:
			*(VWORD) (addr + i) = val;
			break;
		}
	}
	return 0;

  err1:
	return CLI_ERR_PARM;
  err2:
	return CLI_ERR_UNALIGN;
  help:
	return CLI_SHOW_USAGE;

}

/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
int mem_copy_cmd(int argc, char *argv[])
{
	unsigned long src, dst, size;
	int step, i;

	step = step_sz(argv[-1][1]);
	size = STEP_BYTE;
	if (2 > argc)
		goto help;
	if (!hextoul(argv[0], &src))
		goto err1;
	if (!hextoul(argv[1], &dst))
		goto err1;
	if (argc > 2 && !hextoul(argv[2], &size))
		goto err1;

	src &= ~(step - 1);
	dst &= ~(step - 1);
	if (vaddr_check(src) || vaddr_check(dst))
		goto err2;
	for (i = 0; i < size; i += step)
	{
		switch (step)
		{
		case STEP_BYTE:
			*((VBYTE) dst) = *((VBYTE) src);
			break;
		case STEP_HWORD:
			*((VHWORD) dst) = *((VHWORD) src);
			break;
		case STEP_WORD:
			*((VWORD) dst) = *((VWORD) src);
			break;
		}
		src += step;
		dst += step;
	}
	return 0;

  err1:
	return CLI_ERR_PARM;
  err2:
	return CLI_ERR_UNALIGN;
  help:
	return CLI_SHOW_USAGE;
}

/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
int mem_cmp_cmd(int argc, char *argv[])
{
	unsigned long size, i;
	volatile char *src, *dst;

	size = STEP_BYTE;
	if (argc < 2)
		goto help;
	if (!hextoul(argv[0], &src))
		goto err1;
	if (!hextoul(argv[1], &dst))
		goto err1;
	if (argc > 2 && !hextoul(argv[2], &size))
		goto err1;

	if (vaddr_check((unsigned long) src) || vaddr_check((unsigned long) dst))
		goto err2;

	for (i = 0; i < size; i++, src++, dst++)
	{
		if (*src != *dst)
		{
			printf("diff @ %x, src=%02x != dst=%02x \n", i, 0xff & *src,
				   0xff & *dst);
			return ((0xff & *src) - (0xff & *dst));
		}
	}
	if (i == size)
		printf("identical!!\n");
	return 0;

  err1:
	return CLI_ERR_PARM;
  err2:
	return CLI_ERR_ADDR;
  help:
	return CLI_SHOW_USAGE;
}

/*!-----------------------------------------------------------------------------
 * function: 
 *
 *      \brief 
 *		\param 
 *      \return 
 +----------------------------------------------------------------------------*/
CMD_DECL(mem_cmd_proc)
//static int mem_cmd_proc(int argc, char *argv[])
{
	int rc = -1;

	if (2 > argc)
		return CLI_SHOW_USAGE;

	switch (argv[0][0])
	{
	case 'e':					//enter
		rc = mem_enter_cmd(argc - 1, argv + 1);
		break;
	case 'd':					//dump
		rc = mem_dump_cmd(argc - 1, argv + 1);
		break;
	case 'm':					//move
		rc = mem_copy_cmd(argc - 1, argv + 1);
		break;
	case 'f':					//flash
		break;
	case 'c':					//cmp
		rc = mem_cmp_cmd(argc - 1, argv + 1);
		break;
	default:
		return CLI_SHOW_USAGE;
	}
	return CLI_OK;
}

//CLI_CMD(mem, mem_cmd_proc, "mem dw/dh/db/mw/mh/mb/ew/eh/eb/cmp/fw/fe ...", 0);
shell_cmd("mem", "mem dw/dh/db/mw/mh/mb/ew/eh/eb/cmp/fw/fe ...", "", mem_cmd_proc);
#endif	// boot code already have similar functions

static int bb_reg10_data = 0;
static int bb_reg12_data = 0;
static int bb_reg14_data = 0;
CMD_DECL(bb_cmd)
{
	int set = 0;
	int reg;
	int val;

	if (argc > 0) {
		if (!strcmp("agcdump", argv[0]))
		{
			int i;
			for (i=0; i<20; i++)
			{
				val = bb_register_read(1, i);
				serial_printf("1-10-%02x\n", i);
				serial_printf("1-11-%02x\n", val);
			}
			return CLI_OK;
		}
		else if (!strcmp("initiq", argv[0]))
		{
			bb_register_write(0, 0x20, 0x11);
		}
		else if (!strcmp("txpwr", argv[0]))
		{
			if (argc > 1) {
				bb_set_tx_gain(atoi(argv[1]));
			} else {
				serial_printf("%d\n", bb_get_tx_gain());
			}
			return CLI_OK;
		}
		else
		{
			if (1 != sscanf(argv[0], "%x", &reg))
				goto help;
		}
	} else {
		serial_printf("BB reg ");
		for (reg = 0; reg < 255;reg++) {
			switch(reg){
				case 0x10:
					val = bb_reg10_data & 0xffUL;
					break;
				case 0x11:
					val = bb_register_read(1, bb_reg10_data & 0xffUL) & 0xffUL;
					break;
				case 0x12:
					val = bb_reg12_data & 0xffUL;
					break;
				case 0x13:
					val = bb_register_read(2, bb_reg12_data & 0xffUL) & 0xffUL;
					break;
				case 0x14:
					val = bb_reg14_data & 0xffUL;
					break;
				case 0x15:
					val = bb_register_read(3, bb_reg14_data & 0xffUL) & 0xffUL;
					break;
				default:
					val = bb_register_read(0, reg) & 0xffUL;
					break;
			}
			if ((reg % 16) == 0)
				serial_printf("\n%02X: ", reg);
			serial_printf("%02x ", val);
		}
		return CLI_OK;
	}

	if (argc > 1) {
		if (1 != sscanf(argv[1], "%x", &val))
			goto help;
		switch(reg){
			case 0x10:
				bb_reg10_data = val & 0xffUL;
				break;
			case 0x11:
				bb_register_write(1, bb_reg10_data & 0xffUL, val & 0xffUL);
				break;
			case 0x12:
				bb_reg12_data = val & 0xffUL;
				break;
			case 0x13:
				bb_register_write(2, bb_reg12_data & 0xffUL, val & 0xffUL);
				break;
			case 0x14:
				bb_reg14_data = val & 0xffUL;
				break;
			case 0x15:
				bb_register_write(3, bb_reg14_data & 0xffUL, val & 0xffUL);
				break;
			default:
				bb_register_write(0, reg, val & 0xffUL);
				break;
		}
		set++;
	} else {
		switch(reg){
			case 0x10:
				val = bb_reg10_data & 0xffUL;
				break;
			case 0x11:
				val = bb_register_read(1, bb_reg10_data & 0xffUL) & 0xffUL;
				break;
			case 0x12:
				val = bb_reg12_data & 0xffUL;
				break;
			case 0x13:
				val = bb_register_read(2, bb_reg12_data & 0xffUL) & 0xffUL;
				break;
			case 0x14:
				val = bb_reg14_data & 0xffUL;
				break;
			case 0x15:
				val = bb_register_read(3, bb_reg14_data & 0xffUL) & 0xffUL;
				break;
			default:
				val = bb_register_read(0, reg) & 0xffUL;
				break;
		}
	}
	serial_printf("%sBB%x = %02x\n", set ? "SET " : "", reg, val);
	return CLI_OK;
help:
	serial_printf("bb [reg] [val]\n\r");
	serial_printf("bb agcdump  (dump AGC table)\n\r");
	serial_printf("bb initiq  (IQ imbalance)\n\r");
	serial_printf("bb txpwr [val] (tx power)\n\r");
	return CLI_OK;
}
/*
cmdt cmdt_bb __attribute__ ((section("cmdt"))) =
{
"bb", bb_cmd, "bb <reg> <val> ;Access BB register"};
*/
//shell_cmd("bb", "bb agcdump/initiq/txpwr ...", "", bb_cmd);
CLI_CMD(bb, bb_cmd, "bb agcdump/initiq/txpwr ...");

CMD_DECL(rf_cmd)
{
	int set = 0;
	int reg;
	int val;

	if (argc > 0) {
		if (1 != sscanf(argv[0], "%x", &reg))
			goto help;
	} else {
		serial_printf("RF reg\n");
	#if 1
		for(reg=0; reg<=0x1f; reg++)
			serial_printf("read reg %x=0x%x\n", reg, rf_read(reg));
	#else
		for (reg = 0; reg < 0x20;reg++) {
			val = rf_read(reg) & 0xff;
			if ((reg % 16) == 0)
				serial_printf("\n%02X: ", reg);
			serial_printf("%02x ", val);
		}
		serial_printf("\n");
	#endif
		return CLI_OK;
	}

	if (argc > 1) {
		if (1 != sscanf(argv[1], "%x", &val))
			goto help;
		rf_write(reg, val);
		set++;
	} else {
		val = rf_read(reg) ;
	}
	serial_printf("%sRF%x = %02x\n", set ? "SET " : "", reg, val);
	return CLI_OK;
help:
	serial_printf("RF [reg] [val]\n\r");
	return CLI_OK;
}
//shell_cmd("rf", "rf reg ...", "", rf_cmd);
CLI_CMD(rf, rf_cmd, "rf reg ...");

CMD_DECL(send_cmd)
{
	int i, num=0, rt;
	if(argc >0)
	{
		sscanf(argv[0], "%d", &num);
	}
	for(i=0;i<num;i++)
	{
		rt = send_pkt();
		if(rt!=0)
		{
			printf("error to send\n");
			return 0;
		}
	}
	return 0;
}
//shell_cmd("sd", "send number pkts", "", send_cmd);
CLI_CMD(sd, send_cmd, "send number pkts");

CMD_DECL(sleep_cmd)
{
	int i, num=0;
	if(argc >0)
	{
		sscanf(argv[0], "%d", &num);
	}
	for(i=0;i<num;i++)
	{
		printf("sleep 1s\n");
		udelay(1000000);
	}
	return 0;
}
//shell_cmd("sleep", "sleep a while (seconds)", "", sleep_cmd);
CLI_CMD(sleep, sleep_cmd, "sleep a while (seconds)");
