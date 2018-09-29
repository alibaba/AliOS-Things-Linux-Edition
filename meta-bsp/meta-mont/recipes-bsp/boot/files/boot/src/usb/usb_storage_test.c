/*!
*   \file usb_storage_test.c
*   \brief USB Storage test program
*   \author Montage
*/
#ifdef CONFIG_USB_LOOPBACK_TEST
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#ifdef __ECOS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_api.h>
#include <cli_api.h>
#include <ehci.h>
#include <usb.h>

#include <sys/time.h>
#include <lib/lib_timeutil.h>
#else
#include <common.h>
#include <lib.h>
#include <part.h>
#include <ehci.h>
#include <usb.h>
#include <sched.h>
#endif
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define SECTOR_SIZE 0x200
#define MIN(A,B) ( ((A) < (B)) ? (A) : (B) )
#define usb_printd(format, args...) printd(format, ##args)
#define NULL 0
#define addr_align(addr, residue) { residue = (int)addr&(SECTOR_SIZE-1); if(residue) {addr = (void *)((int)addr - residue + SECTOR_SIZE);} }
#define addr_recover(addr, residue) { if(residue) {addr = (void *)((int)addr - SECTOR_SIZE + residue);} }
#define HAL_PAGE_BOUNDARY_SIZE 4096     // 4 KB buffer boundary alignment by usb
#define MAX_BLK_NUM 0x400
#define LOW_LEVEL_TEST_MAX_SIZE (512*MAX_BLK_NUM)       // how many Bytes
#define LOW_LEVEL_TEST_MAX_ITEM 11
#define FIXED_PATTERN_LEN (SECTOR_SIZE-1)
#define BACKUP_CONTENT
#ifdef __ECOS
#else
#undef CLK_OK
#define CLI_OK ERR_OK
#undef CLI_SHOW_USAGE
#define CLI_SHOW_USAGE ERR_HELP
#undef printd
#define printd printf
#undef diag_dump_buf
#define diag_dump_buf(a, b) dump_hex("", (char *)a, b)
#undef diag_printf
#define diag_printf printf
#endif
#define USBTEST_EXIT    (1<<0)
#define USBTEST_VERBOSE (1<<1)
#define USBTEST_STAT    (1<<2)
#define USBTEST_CLEAR   (1<<3)
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
struct usbtest_dev
{
    int loop_count;             /* usbtest loop count */
    int error_count;            /* usbtest loop error count */
    int idx;                    /* usbtest storage idx */
#ifdef __ECOS
    OS_FLAG_ID flag;
    OS_THREAD_ID handle;
#endif
    volatile int testing;
} ut_dev_one;
#ifdef BACKUP_CONTENT
unsigned char backup[LOW_LEVEL_TEST_MAX_SIZE]
    __attribute__ ((aligned(HAL_PAGE_BOUNDARY_SIZE)));
#endif
#ifdef __ECOS
#else
unsigned char addr1[LOW_LEVEL_TEST_MAX_SIZE]
    __attribute__ ((aligned(HAL_PAGE_BOUNDARY_SIZE)));
unsigned char addr2[LOW_LEVEL_TEST_MAX_SIZE]
    __attribute__ ((aligned(HAL_PAGE_BOUNDARY_SIZE)));
#endif
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern int cli_gets(char *buf, str_chain * p_cmd, char peek);
extern volatile struct ehci_hcor *hcor;
extern int usb_max_devs;
extern int dev_index;
extern char usb_started;
extern block_dev_desc_t *usb_stor_get_dev(int index);
extern struct QH qh_array[1];
extern struct qTD td_array[3];
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
#pragma GCC optimize ("O0")
static int idle_cycle = 0, mode = 0;
#endif
char *usb_subcmd[] =
{
    "stop",
};
void usb_tst_thread(void *param)
{
    int rc;
    int blk = 0x0;
    u32 cnt, bcnt, tlen;
    block_dev_desc_t *stor_dev;
    char pdata[FIXED_PATTERN_LEN];
    int i, tmp, src;
    //int s1, s2;
    struct usbtest_dev *dev = &ut_dev_one;
#ifdef __ECOS
    void *addr1;
    void *addr2;
#endif
#if !defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
    unsigned int flag = USBTEST_VERBOSE;
#endif
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
    int cycle;
#endif

    /* init pdata */
    for (i = 0; i < FIXED_PATTERN_LEN; i++)
        pdata[i] = i;

    printd("[%s] startup!\n", __func__);
    printd("CLI>");
    /* show stroage information */
    stor_dev = usb_stor_get_dev(dev->idx);
    printd("storge block size:0x%x bytes\n", stor_dev->blksz);
    if (stor_dev->blksz != SECTOR_SIZE)
        printd("!!!Not yet support block size isn't 512 byte");
    printd("max count:0x%x\n", MAX_BLK_NUM);

#ifdef __ECOS
#else
    /* stop prompt */
    printd("(press `q` to stop)\n");
    unsigned int time;
    time = clock_get();
#endif

    dev->testing = 1;

#if !defined(CONFIG_SCHED)
    char buffer[256 + 1];
    int ret;
    str_chain *phistory = 0x00;
    char *argv_t[10];
    int argc_t;
#endif

#ifdef __ECOS
    for (;;)
#else
    //while (!(serial_poll() && (serial_getc() == 'q')))
    while (dev->testing)
#endif
    {
#if !defined(CONFIG_SCHED)
        ret=cli_gets(buffer, phistory, 1);
        if(ret == ERR_OK)
        {
            if (1 <= (argc_t = get_args(&buffer[0], argv_t)))
            {
                if(!strcmp("ut", argv_t[0]))
                {
                    int i;
                    for(i=0; i < sizeof(usb_subcmd)/sizeof(usb_subcmd[0]); i++)
                    {
                        if(!strcmp(usb_subcmd[i], argv_t[1]))
                        {
                            cmd_proc(argc_t, argv_t);
                            break;
                        }
                    }
                }
            }
        }
#endif
        /* calculate 1)start_block 2)count */
#ifdef __ECOS
        blk = os_current_time() % MAX_BLK_NUM;
        cnt = os_current_time() % (MAX_BLK_NUM - blk + 1);
        flag = os_flag_poll(dev->flag);
#else
        blk = how_long(time) % MAX_BLK_NUM;
        cnt = how_long(time) % (MAX_BLK_NUM - blk + 1);
#endif
        if (!cnt)
            cnt = 1;
        bcnt = cnt << 9;        //cnt x512 = bcnt

#if !defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
        if (flag & USBTEST_VERBOSE)
        {
            usb_printd("===#%u===\n", dev->loop_count);
            usb_printd("start block:0x%03x, count:0x%03x\n", blk, cnt);
        }
#endif
        dev->loop_count++;

        /* memory allocation */
#ifdef __ECOS
        addr1 = (void *) malloc(bcnt + SECTOR_SIZE);    //request extra SECTOR_SIZE byte to enbale addr 1 align SECTOR_SIZE
        addr2 = (void *) malloc(bcnt + SECTOR_SIZE);

        /* buffer address align SECTOR_SIZE to avoid flush the neighbors */
        addr_align(addr1, s1);
        addr_align(addr2, s2);
#endif

        /* copy data from prepared data */
        tmp = bcnt;
        src = (int) addr1;
        while (tmp > 0)
        {
            tlen = MIN(tmp, FIXED_PATTERN_LEN);
            memcpy((void *) src, (void *) pdata, tlen);
            src += tlen;
            tmp -= tlen;
        }

        /* USB storage read/write */
        stor_dev = usb_stor_get_dev(dev->idx);

#ifdef BACKUP_CONTENT
        rc = (stor_dev->block_read) ? stor_dev->block_read(dev->idx, blk, cnt,
                                                           backup) : 0;
        if (rc != cnt)
        {
            dev->loop_count--;
            printd("block_read error block_read, rc:0x%x\n", rc);
            printd("start block:0x%x, count:0x%x, backup:0x%x\n", blk, cnt,
                   backup);
            goto err;
        }
#endif

#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
	if(mode == 1)
	{
		cnt = 1;
		blk = 0;
		rc = (stor_dev->block_read) ? stor_dev->block_read(dev->idx, blk, cnt,
				addr2) : 0;
		if (rc != cnt)
		{
			printd("block_read error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr2:0x%x\n", blk, cnt,
					addr2);
			goto err;
		}
		blk = 0xff;
		rc = (stor_dev->block_read) ? stor_dev->block_read(dev->idx, blk, cnt,
				addr2) : 0;
		if (rc != cnt)
		{
			printd("block_read error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr2:0x%x\n", blk, cnt,
					addr2);
			goto err;
		}
		blk = 0xaa;
		rc = (stor_dev->block_read) ? stor_dev->block_read(dev->idx, blk, cnt,
				addr2) : 0;
		if (rc != cnt)
		{
			printd("block_read error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr2:0x%x\n", blk, cnt,
					addr2);
			goto err;
		}
		for(cycle=0;cycle<idle_cycle;cycle++)
			rc = cycle;
	}
	else if(mode == 2)
	{
		cnt = 1;
		blk = 0xff;
		rc = (stor_dev->block_write) ? stor_dev->block_write(dev->idx, blk, cnt,
				addr1) : 0;
		if (rc != cnt)
		{
			printd("block_write error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr1:0x%x\n", blk, cnt,
					addr1);
			goto err;
		}
		blk = 0xaa;
		rc = (stor_dev->block_write) ? stor_dev->block_write(dev->idx, blk, cnt,
				addr1) : 0;
		if (rc != cnt)
		{
			printd("block_write error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr1:0x%x\n", blk, cnt,
					addr1);
			goto err;
		}
		blk = 0x55;
		rc = (stor_dev->block_write) ? stor_dev->block_write(dev->idx, blk, cnt,
				addr1) : 0;
		if (rc != cnt)
		{
			printd("block_write error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr1:0x%x\n", blk, cnt,
					addr1);
			goto err;
		}
		for(cycle=0;cycle<idle_cycle;cycle++)
			rc = cycle;
	}
	else
	{
		if(mode == 3)
			cnt = 1;
		rc = (stor_dev->block_write) ? stor_dev->block_write(dev->idx, blk, cnt,
				addr1) : 0;
		if (rc != cnt)
		{
			printd("block_write error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr1:0x%x\n", blk, cnt,
					addr1);
			goto err;
		}
		rc = (stor_dev->block_read) ? stor_dev->block_read(dev->idx, blk, cnt,
				addr2) : 0;
		if (rc != cnt)
		{
			printd("block_read error, rc:0x%x\n", rc);
			printd("start block:0x%x, count:0x%x, addr2:0x%x\n", blk, cnt,
					addr2);
			goto err;
		}
		for(cycle=0;cycle<idle_cycle;cycle++)
			rc = cycle;
	}
#else
        rc = (stor_dev->block_write) ? stor_dev->block_write(dev->idx, blk, cnt,
                                                             addr1) : 0;
        if (rc != cnt)
        {
            printd("block_write error, rc:0x%x\n", rc);
            printd("start block:0x%x, count:0x%x, addr1:0x%x\n", blk, cnt,
                   addr1);
            goto err;
        }
        rc = (stor_dev->block_read) ? stor_dev->block_read(dev->idx, blk, cnt,
                                                           addr2) : 0;
        if (rc != cnt)
        {
            printd("block_read error, rc:0x%x\n", rc);
            printd("start block:0x%x, count:0x%x, addr2:0x%x\n", blk, cnt,
                   addr2);
            goto err;
        }
        if (memcmp(addr1, addr2, bcnt))
        {
            dev->error_count++;
            printd("===usb loopback tst compare fail===\n");
            printd("start block:0x%x, count:0x%x, addr1:0x%x, addr2:0x%x\n",
                   blk, cnt, addr1, addr2);
            if (flag & USBTEST_VERBOSE)
            {
                diag_dump_buf(addr1, bcnt);
                printd("======\n");
                diag_dump_buf(addr2, bcnt);
            }
        }
#endif

#ifdef BACKUP_CONTENT
        rc = (stor_dev->block_write) ? stor_dev->block_write(dev->idx, blk, cnt,
                                                             backup) : 0;
        if (rc != cnt)
        {
            printd("block_write error, rc:0x%x\n", rc);
            printd("start block:0x%x, count:0x%x, backup:0x%x\n", blk, cnt,
                   backup);
            goto err;
        }
#endif

#ifdef __ECOS
        /* recover unaligned address */
        addr_recover(addr1, s1);
        addr_recover(addr2, s2);

        /* memory free */
        free(addr1);
        free(addr2);
#endif
#ifdef __ECOS
        if (flag & USBTEST_EXIT)
            break;
        if (flag & USBTEST_STAT)
        {
            printd("Total: %u\n", dev->loop_count);
            printd("Pass: %u\n", dev->loop_count - dev->error_count);
            printd("Fail: %u\n", dev->error_count);
        }
        if (flag & USBTEST_CLEAR)
            dev->loop_count = dev->error_count = 0;
#endif
    }
    printd("[%s] exit!\n", __func__);
    printd("Total: %u\n", dev->loop_count);
    printd("Pass: %u\n", dev->loop_count - dev->error_count);
    printd("Fail: %u\n", dev->error_count);
#ifdef __ECOS
    goto exit;
#endif
#if defined(CONFIG_SCHED)
    thread_exit();
#endif
    return;
  err:
    /* show extra information to debug */
    printd("===usb loopback tst fail!===\n");
#ifdef __ECOS
    printd("[Host Controller Registers]\n");
    diag_dump_buf_32bit((void *) hcor, sizeof (struct ehci_hcor));
    printd("[QH]\n");
    diag_dump_buf_32bit(qh_array, EHCI_QH_SIZE);
    printd("[QTDs]\n");
    diag_dump_buf_32bit(&td_array[0], EHCI_QTD_SIZE);
    diag_dump_buf_32bit(&td_array[1], EHCI_QTD_SIZE);
    diag_dump_buf_32bit(&td_array[2], EHCI_QTD_SIZE);
#else
    ehci_dump();
#endif
#ifdef __ECOS
  exit:
    os_flag_destory(dev->flag);
    dev->flag = 0;
    os_thread_exit();
#endif
    return;
}

#ifdef __ECOS
static int timevaldiff(struct timeval const *a, struct timeval const *b)
{
    int msec;
    msec = (a->tv_sec - b->tv_sec) * 1000;
    msec += (a->tv_usec - b->tv_usec) / 1000;
    return msec;
}
#endif

int test_burst(int base, int count, int read)
{
#ifdef __ECOS
    struct timeval pretv;
    struct timeval tv;
#endif
    //int msec = 0;
    int i, n = 0;

    block_dev_desc_t *stor_dev;
    int test_dev = 0;
    int rc;
    u32 cnt = base;
    /* sequential access */
    static u32 blk = 102400;
#ifdef __ECOS
    static unsigned char buf[LOW_LEVEL_TEST_MAX_SIZE]
        __attribute__ ((aligned(HAL_PAGE_BOUNDARY_SIZE)));
#else
    static unsigned char *buf = addr1;
#endif

    stor_dev = usb_stor_get_dev(test_dev);
    if (!stor_dev)
        return 0;
    if (!stor_dev->block_read)
        return 0;
#ifdef __ECOS
    gettimeofday(&pretv, NULL);
#else
    unsigned int time;
    time = clock_get();
#endif
    for (i = 0, n = 0; i < count; i++)
    {
        rc = 0;
        if (read)
            rc = (stor_dev->block_read) ? stor_dev->block_read(test_dev, blk,
                                                               cnt, buf) : 0;
        else
            rc = (stor_dev->block_write) ? stor_dev->block_write(test_dev, blk,
                                                                 cnt, buf) : 0;
        n += (cnt * 512);
        /* sequential access */
        blk += cnt;
        if (rc != cnt)
        {
            diag_printf("<USB>: err, rc=%d\n", rc);
            break;
        }
    }
#ifdef __ECOS
    gettimeofday(&tv, NULL);
    msec = timevaldiff(&tv, &pretv);
    return (n / msec);
#else
    return (n / how_long(time));
#endif
}

int usbtest_burst_one(int base, int kind)
{
    int read_speed = 0;
    int write_speed = 0;

    base = (int) (1 << base);

    if (base > (LOW_LEVEL_TEST_MAX_SIZE / 512))
    {
        diag_printf("<USB>: burst too many data\n");
        return CLI_OK;
    }
    if (base == 1)
        diag_printf("<USB>: burst 0.5 KB \n");
    else
        diag_printf("<USB>: burst %d KB \n", base * 512 / 1024);

    if (kind & 0x01)
        read_speed = test_burst(base, 200, 1);
    if (kind & 0x02)
        write_speed = test_burst(base, 200, 0);
    diag_printf("<USB>: speed read = %d KB/s, write = %d KB/s \n", read_speed,
                write_speed);
    return CLI_OK;
}

int usbtest_burst(void)
{
    int base = 1;               // 512 Bytes
    int read_speed[LOW_LEVEL_TEST_MAX_ITEM] = { 0 };
    int write_speed[LOW_LEVEL_TEST_MAX_ITEM] = { 0 };
    int i;

    /* sequential access to read */
    diag_printf("<USB>:");
    for (i = 0, base = 1; i < LOW_LEVEL_TEST_MAX_ITEM; i++)
    {
        if (base > (LOW_LEVEL_TEST_MAX_SIZE / 512))
            break;
        read_speed[i] = test_burst(base, 200, 1);
        diag_printf(".");

        base <<= 1;
    }
    /* sequential access to write */
    for (i = 0, base = 1; i < LOW_LEVEL_TEST_MAX_ITEM; i++)
    {
        if (base > (LOW_LEVEL_TEST_MAX_SIZE / 512))
            break;
        write_speed[i] = test_burst(base, 200, 0);
        diag_printf(".");

        base <<= 1;
    }
    diag_printf("\n");
    for (i = 0, base = 1; i < LOW_LEVEL_TEST_MAX_ITEM; i++)
    {
        if (base > (LOW_LEVEL_TEST_MAX_SIZE / 512))
            break;
        if (base == 1)
            diag_printf("<USB>: burst 0.5 KB \n");
        else
            diag_printf("<USB>: burst %d KB \n", base * 512 / 1024);
        diag_printf("<USB>: speed read = %d KB/s, write = %d KB/s \n",
                    read_speed[i], write_speed[i]);

        base <<= 1;
    }
    return CLI_OK;
}

#if defined(CONFIG_SCHED)
#define USB_TEST_THREAD_STACK_SIZE  (128*1024)
unsigned char usb_test_thread_stack[USB_TEST_THREAD_STACK_SIZE];
#endif
int usbtest_start(void)
{
    struct usbtest_dev *dev = &ut_dev_one;
#ifdef __ECOS
    if (!dev->flag)
    {
        dev->loop_count = 0;
        dev->error_count = 0;
        dev->flag = os_flag_create();
        dev->handle =
            os_thread_create(usb_tst_thread, NULL, "usbtst", 30,
                             CYGNUM_HAL_STACK_SIZE_TYPICAL);
    }
    else
        printd("usbtst thread not yet exit!!!\n");
#else
    dev->loop_count = 0;
    dev->error_count = 0;
#if defined(CONFIG_SCHED)
    thread_create(usb_tst_thread, (void *) 0,
                   &usb_test_thread_stack[USB_TEST_THREAD_STACK_SIZE], USB_TEST_THREAD_STACK_SIZE);
#else
    usb_tst_thread(NULL);
#endif
#endif

    return CLI_OK;
}

#ifdef __ECOS
int usbtest_stat(int auto_clear)
{
    struct usbtest_dev *dev = &ut_dev_one;
    int stat = (auto_clear ? (USBTEST_CLEAR | USBTEST_STAT) : USBTEST_STAT);
    if (dev->flag)
        os_flag_set(dev->flag, stat);
    else
        printd("usbtst thread not yet run\n");

    return CLI_OK;
}

int usbtest_stat_url(void)
{
    struct usbtest_dev *dev = &ut_dev_one;
    if (dev->flag == 0)
    {
        printf("Total: %u\n", dev->loop_count);
        printf("Pass: %u\n", dev->loop_count - dev->error_count);
        printf("Fail: %u\n", dev->error_count);
    }
    return CLI_OK;
}

int usbtest_stop(void)
{
    struct usbtest_dev *dev = &ut_dev_one;
    if (dev->flag)
        os_flag_set(dev->flag, USBTEST_EXIT);
    else
        printd("usbtst thread not yet run\n");

    return CLI_OK;
}
#else
int usbtest_stat_url(void)
{
    struct usbtest_dev *dev = &ut_dev_one;
    //if (dev->flag == 0)
    {
        printf("Total: %u\n", dev->loop_count);
        printf("Pass: %u\n", dev->loop_count - dev->error_count);
        printf("Fail: %u\n", dev->error_count);
    }
    return CLI_OK;
}
int usbtest_stop(void)
{
    struct usbtest_dev *dev = &ut_dev_one;

    dev->testing = 0;

    return CLI_OK;
}
#endif

int usbtest_cmd(int argc, char *argv[])
{
#ifdef __ECOS
    if (usb_max_devs <= 0)
    {
        printd("Please plug in USB storage first.\n");
        return CLI_OK;
    }
#else
    if (!usb_started || dev_index < 2)
    {
        printd("usb_started=%d, dev_index=%d\n", usb_started, dev_index);
        printd("USB is stopped. Please issue 'usb start' first.\n");
        return CLI_OK;
    }
#endif

#ifdef __ECOS
#define USBTEST_ARG_ONE 1
#define USBTEST_ARG_TWO 2
#define USBTEST_ARG_MINNUM 2
#define USBTEST_GET_ARG(a) a = strtoul(argv[USBTEST_ARG_MINNUM], NULL, 16);
#else
#define USBTEST_ARG_ONE 0
#define USBTEST_ARG_MINNUM 1
#define USBTEST_GET_ARG(a) hextoul(argv[USBTEST_ARG_MINNUM], &a)
#endif
    if (argc >= USBTEST_ARG_MINNUM)
    {
        if (strcmp(argv[USBTEST_ARG_ONE], "start") == 0)
            return usbtest_start();
#if defined(GENERATE_DDR_ACCESS_FOR_RF_INTERFERENCE)
        else if (strcmp(argv[USBTEST_ARG_ONE], "ddr_test") == 0)
        {
            if(argc != 3)
                printf("ut ddr_test [r/w/rw] [idle cycle]\n");
            if (strcmp(argv[1], "r") == 0)
            {
                mode = 1;
                idle_cycle = atoi(argv[2]);
                printf("start ddr_test idle_cycle: %d, (mode %d) read pattern\n", idle_cycle, mode);
            }
            else if (strcmp(argv[1], "w") == 0)
            {
                mode = 2;
                idle_cycle = atoi(argv[2]);
                printf("start ddr_test idle_cycle: %d, (mode %d) write pattern\n", idle_cycle, mode);
            }
            else if (strcmp(argv[1], "rw") == 0)
            {
                mode = 3;
                idle_cycle = atoi(argv[2]);
                printf("start ddr_test idle_cycle: %d, (mode %d) read/write pattern\n", idle_cycle, mode);
            }
            else if (strcmp(argv[1], "n") == 0)
            {
                mode = 0;
                idle_cycle = 0;
            }
            return 0;
        }
#endif
#ifdef __ECOS
        else if (strcmp(argv[USBTEST_ARG_ONE], "stat") == 0)
        {
            if (strcmp(argv[USBTEST_ARG_TWO], "url") == 0)
                return usbtest_stat_url();
            else if (strcmp(argv[USBTEST_ARG_TWO], "keep") == 0)
                return usbtest_stat(0);
            else
                return usbtest_stat(1);
        }
        else if (strcmp(argv[USBTEST_ARG_ONE], "stop") == 0)
            return usbtest_stop();
#endif
        else if (strcmp(argv[USBTEST_ARG_ONE], "stat") == 0)
        {
            if (strcmp(argv[1], "url") == 0)
                return usbtest_stat_url();
        }
        else if (strcmp(argv[USBTEST_ARG_ONE], "stop") == 0)
            return usbtest_stop();
        else if (strcmp(argv[USBTEST_ARG_ONE], "burst") == 0)
        {
            if (argc > USBTEST_ARG_MINNUM)
            {
                int base;
                USBTEST_GET_ARG(base);
                return usbtest_burst_one(base, 3);
            }
            else
                return usbtest_burst();
        }
        else if (strcmp(argv[USBTEST_ARG_ONE], "burstr") == 0)
        {
            if (argc > USBTEST_ARG_MINNUM)
            {
                int base;
                USBTEST_GET_ARG(base);
                return usbtest_burst_one(base, 1);
            }
        }
        else if (strcmp(argv[USBTEST_ARG_ONE], "burstw") == 0)
        {
            if (argc > USBTEST_ARG_MINNUM)
            {
                int base;
                USBTEST_GET_ARG(base);
                return usbtest_burst_one(base, 2);
            }
        }
    }
    return CLI_SHOW_USAGE;
}

#ifdef __ECOS
CLI_CMD(ut, usbtest_cmd, "ut(Usb storage Test)\n"
        "\tstart (start long time access test)\n"
        "\tstop (stop long time access test)\n"
        "\tstat [keep] (show statistics)\n"
        "\tburst <burst_unit> (evaluate storage access throughput)\n"
        "\tburstr <burst_unit> (read only)\n"
        "\tburstw <burst_unit> (write only)\n", 0);
#else
cmdt cmdt_usbtest __attribute__ ((section("cmdt"))) =
{
"ut", usbtest_cmd, "ut [start/burst/burstr/burstw]"};
#endif

#endif                          //CONFIG_USB_LOOPBACK_TEST
