/*!
*   \file tsi_test.c
*   \brief TSI test program
*   \author Montage
*/
#ifdef CONFIG_TSI_TEST
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <lib.h>
#include <tsi.h>

void delay(unsigned int time);
void tsi_stat(void);
void tsi_start(struct tsi_dev *dev);
void tsi_stop(struct tsi_dev *dev);
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define TSI_TESTCASE1_DST_PKTNUM 16
#define TSI_TESTCASE1_DST0_ADDR 0x20000 //need align 8
#define TSI_TESTCASE1_DST0_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE1_DST_PKTNUM)
#define TSI_TESTCASE1_DST1_ADDR 0x40000 //need align 8
#define TSI_TESTCASE1_DST1_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE1_DST_PKTNUM)
#define TSI_TESTCASE1_WAIT2COMPLETE 100
#define TSI_TESTCASE2_DST_PKTNUM 16
#define TSI_TESTCASE2_DST0_ADDR 0x20000 //need align 8
#define TSI_TESTCASE2_DST0_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE2_DST_PKTNUM)
#define TSI_TESTCASE2_DST1_ADDR 0x40000 //need align 8
#define TSI_TESTCASE2_DST1_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE2_DST_PKTNUM)
#define TSI_TESTCASE2_WAIT2COMPLETE 200
#define TSI_TESTCASE3_DST_PKTNUM 16
#define TSI_TESTCASE3_DST0_ADDR 0x20000 //need align 8
#define TSI_TESTCASE3_DST0_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE3_DST_PKTNUM)
#define TSI_TESTCASE3_DST1_ADDR 0x40000 //need align 8
#define TSI_TESTCASE3_DST1_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE3_DST_PKTNUM)
#define TSI_TESTCASE3_WAIT2COMPLETE 300
#define TSI_TESTCASE4_DST_PKTNUM 16
#define TSI_TESTCASE4_DST0_ADDR 0x20000 //need align 8
#define TSI_TESTCASE4_DST0_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE4_DST_PKTNUM)
#define TSI_TESTCASE4_DST1_ADDR 0x40000 //need align 8
#define TSI_TESTCASE4_DST1_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE4_DST_PKTNUM)
#define TSI_TESTCASE4_WAIT2COMPLETE 400
#define TSI_TESTCASE5_DST_PKTNUM 16
#define TSI_TESTCASE5_DST0_ADDR 0x20000 //need align 8
#define TSI_TESTCASE5_DST0_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE5_DST_PKTNUM)
#define TSI_TESTCASE5_DST1_ADDR 0x40000 //need align 8
#define TSI_TESTCASE5_DST1_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE5_DST_PKTNUM)
#define TSI_TESTCASE5_WAIT2COMPLETE 500
#define TSI_TESTCASE6_DST_PKTNUM 1600
#define TSI_TESTCASE6_DST0_ADDR 0x20000 //need align 8
#define TSI_TESTCASE6_DST0_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE6_DST_PKTNUM)
#define TSI_TESTCASE6_DST1_ADDR 0x80000 //need align 8
#define TSI_TESTCASE6_DST1_SIZE (MPEGTS_FRAME_LEN*TSI_TESTCASE6_DST_PKTNUM)
#define TSI_TESTCASE6_WAIT2COMPLETE 500
#define TSI_SIZE(len) ((len)>>3)        //TSI size unit is 8 byte
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
extern struct tsi_dev g_tsi_dev;
/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void tsit1(void)
{
    int i;
    struct tsi_dev *p = &g_tsi_dev;
    unsigned char *p0 = (void *) phy_to_virt(TSI_TESTCASE1_DST0_ADDR);
    unsigned char *p1 = (void *) phy_to_virt(TSI_TESTCASE1_DST1_ADDR);
    unsigned char tmp;
    printf("case 1: ");
    memset((void *) p0, 0, TSI_TESTCASE1_DST0_SIZE);
    memset((void *) p1, 0, TSI_TESTCASE1_DST1_SIZE);
    memset((void *) p, 0, sizeof (struct tsi_dev));
    p->addr0 = (unsigned int) p0;
    p->size0 = TSI_SIZE(TSI_TESTCASE1_DST0_SIZE);
    p->addr1 = phy_to_virt(TSI_TESTCASE1_DST1_ADDR);
    p->size1 = TSI_SIZE(TSI_TESTCASE1_DST1_SIZE);
    p->flags = TSI_FLAG_BOUNDARY_STOP;
    //start transmit
    tsi_start(p);
    //wait TSI complete
    delay(TSI_TESTCASE1_WAIT2COMPLETE);
    //check data bank0 data
    tmp = p0[1];
    for (i = 0; i < TSI_TESTCASE1_DST1_SIZE; i++)
    {
        if (i % MPEGTS_FRAME_LEN == 0)
        {
            if (p0[i] != MPEGTS_SYMBOL)
            {
                printf("addr:%x ", p0 + i);
                printf("out of sync ");
                goto err;
            }
        }
        else
        {
            if (p0[i] != tmp)
            {
                printf("addr:%x ", p0 + i);
                printf("data error ");
                goto err;
            }
            tmp++;
        }
    }
    //check data bank1 data
    for (i = 0; i < TSI_TESTCASE1_DST1_SIZE; i++)
        if (*(p1 + i) != 0)
        {
            printf("addr:%x ", p1 + i);
            printf("bank1 failed ");
            goto err;
        }

    printf("succeed\n");
    return;
  err:
    printf("failed\n");
}

void tsit2(void)
{
    struct tsi_dev *p = &g_tsi_dev;
    unsigned char *p0 = (void *) phy_to_virt(TSI_TESTCASE2_DST0_ADDR);
    unsigned char *p1 = (void *) phy_to_virt(TSI_TESTCASE2_DST1_ADDR);
    printf("case 2: ");
    memset((char *) p0, 0, TSI_TESTCASE2_DST0_SIZE);
    memset((char *) p1, 0, TSI_TESTCASE2_DST1_SIZE);
    memset((char *) p, 0, sizeof (struct tsi_dev));
    p->addr0 = (unsigned int) p0;
    p->size0 = TSI_SIZE(TSI_TESTCASE2_DST0_SIZE);
    p->addr1 = phy_to_virt(TSI_TESTCASE2_DST1_ADDR);
    p->size1 = TSI_SIZE(TSI_TESTCASE2_DST1_SIZE);
    //start transmit
    tsi_start(p);
    //wait TSI complete
    delay(TSI_TESTCASE2_WAIT2COMPLETE);
    //check data bank1 isn't zero
    if (*p1 == 0)
    {
        printf("bank1 failed ");
        goto err;
    }

    printf("succeed\n");
    return;
  err:
    printf("failed\n");
}

void tsit3(void)
{
    struct tsi_dev *p = &g_tsi_dev;
    unsigned char *p0 = (void *) phy_to_virt(TSI_TESTCASE3_DST0_ADDR);
    unsigned char *p1 = (void *) phy_to_virt(TSI_TESTCASE3_DST1_ADDR);
    printf("case 3: ");
    memset((char *) p0, 0, TSI_TESTCASE3_DST0_SIZE);
    memset((char *) p1, 0, TSI_TESTCASE3_DST1_SIZE);
    memset((char *) p, 0, sizeof (struct tsi_dev));
    p->addr0 = (unsigned int) p0;
    p->size0 = TSI_SIZE(TSI_TESTCASE3_DST0_SIZE);
    p->addr1 = phy_to_virt(TSI_TESTCASE3_DST1_ADDR);
    p->size1 = TSI_SIZE(TSI_TESTCASE3_DST1_SIZE);
    //start transmit
    tsi_start(p);
    //wait TSI complete
    delay(TSI_TESTCASE3_WAIT2COMPLETE);
    /* disable TS interface */
    TSIREG(TSI_CTRL) &= ~(TSI_EN);
    //show 32 byte in buffer, check boundary by your eyes
    dump_hex("", (char *) phy_to_virt(TSIREG(TSI_CUR_ADDR) - 0x10), 0x20);

    printf("succeed\n");
    return;
}

void tsit4(void)
{
    struct tsi_dev *p = &g_tsi_dev;
    unsigned char *p0 = (void *) phy_to_virt(TSI_TESTCASE4_DST0_ADDR);
    unsigned char *p1 = (void *) phy_to_virt(TSI_TESTCASE4_DST1_ADDR);
    printf("case 4: ");
    memset((void *) p0, 0, TSI_TESTCASE4_DST0_SIZE);
    memset((void *) p1, 0, TSI_TESTCASE4_DST1_SIZE);
    memset((void *) p, 0, sizeof (struct tsi_dev));
    p->addr0 = (unsigned int) p0;
    p->size0 = TSI_SIZE(TSI_TESTCASE4_DST0_SIZE);
    p->addr1 = phy_to_virt(TSI_TESTCASE4_DST1_ADDR);
    p->size1 = TSI_SIZE(TSI_TESTCASE4_DST1_SIZE);
    p->filter_number = 3;
    p->filter_id[0] = 0x9798;
    p->filter_id[1] = 0x0607;
    p->filter_id[2] = 0xa2a3;
    //start transmit
    tsi_start(p);
    //wait TSI complete
    delay(TSI_TESTCASE4_WAIT2COMPLETE);

    printf("succeed\n");
    return;
}

void tsit5_printf(void *p, unsigned int status)
{
    //printf("%s\n",__func__);
    if (status & TSI_DST_CHG_I)
        TSIREG(TSI_INTR_EN) = 0;
}

void tsit5(void)
{
    int i;
    struct tsi_dev *p = &g_tsi_dev;
    unsigned char *p0 = (void *) phy_to_virt(TSI_TESTCASE5_DST0_ADDR);
    unsigned char *p1 = (void *) phy_to_virt(TSI_TESTCASE5_DST1_ADDR);
    unsigned char tmp;
    printf("case 5: ");
    memset((void *) p0, 0, TSI_TESTCASE5_DST0_SIZE);
    memset((void *) p1, 0, TSI_TESTCASE5_DST1_SIZE);
    memset((void *) p, 0, sizeof (struct tsi_dev));
    p->addr0 = (unsigned int) p0;
    p->size0 = TSI_SIZE(TSI_TESTCASE5_DST0_SIZE);
    p->addr1 = phy_to_virt(TSI_TESTCASE5_DST1_ADDR);
    p->size1 = TSI_SIZE(TSI_TESTCASE5_DST1_SIZE);
    p->flags = TSI_FLAG_BOUNDARY_STOP;
    p->intr_status = (TSI_DST_CHG_I | TSI_OFLW_I);
    p->func = tsit5_printf;
    //start transmit
    tsi_start(p);
    //wait TSI complete
    delay(TSI_TESTCASE5_WAIT2COMPLETE);
    //check data bank0 data
    tmp = p0[1];
    for (i = 0; i < TSI_TESTCASE5_DST1_SIZE; i++)
    {
        if (i % MPEGTS_FRAME_LEN == 0)
        {
            if (p0[i] != MPEGTS_SYMBOL)
            {
                printf("addr:%x ", p0 + i);
                printf("out of sync ");
                goto err;
            }
        }
        else
        {
            if (p0[i] != tmp)
            {
                printf("addr:%x ", p0 + i);
                printf("data error ");
                goto err;
            }
            tmp++;
        }
    }
    //check data bank1 data
    for (i = 0; i < TSI_TESTCASE5_DST1_SIZE; i++)
        if (*(p1 + i) != 0)
        {
            printf("addr:%x ", p1 + i);
            printf("bank1 failed ");
            goto err;
        }

    printf("succeed\n");
    return;
  err:
    printf("failed\n");
}

void tsit6_irqhanlder(void *p, unsigned int status)
{
    //printf("I");
}

void tsit6(void)
{
    struct tsi_dev *p = &g_tsi_dev;
    unsigned char *p0 = (void *) phy_to_virt(TSI_TESTCASE6_DST0_ADDR);
    unsigned char *p1 = (void *) phy_to_virt(TSI_TESTCASE6_DST1_ADDR);
    printf("case 6: ");
    memset((void *) p0, 0, TSI_TESTCASE6_DST0_SIZE);
    memset((void *) p1, 0, TSI_TESTCASE6_DST1_SIZE);
    memset((void *) p, 0, sizeof (struct tsi_dev));
    p->addr0 = (unsigned int) p0;
    p->size0 = TSI_SIZE(TSI_TESTCASE6_DST0_SIZE);
    p->addr1 = phy_to_virt(TSI_TESTCASE6_DST1_ADDR);
    p->size1 = TSI_SIZE(TSI_TESTCASE6_DST1_SIZE);
    p->intr_pktnum_th = TSI_SIZE(MPEGTS_FRAME_LEN * 800);
    p->intr_status = (TSI_SPKT_DONE_I);
    p->func = tsit6_irqhanlder;
    //start transmit
    tsi_start(p);
    //wait TSI complete
    delay(TSI_TESTCASE6_WAIT2COMPLETE);

    printf("succeed\n");
    return;
}

int tsit_cmd(int argc, char *argv[])
{
    unsigned int test_case;
    if (!hextoul(argv[0], &test_case))
        goto err_arg;
    if (test_case == 1)
        tsit1();
    else if (test_case == 2)
        tsit2();
    else if (test_case == 3)
        tsit3();
    else if (test_case == 4)
        tsit4();
    else if (test_case == 5)
        tsit5();
    else if (test_case == 6)
        tsit6();
    else
        goto err_arg;
    //show statistic
    tsi_stat();
    //stop transmit
    tsi_stop(&g_tsi_dev);
    return ERR_OK;
  err_arg:
    return ERR_PARM;
}

cmdt cmdt_tsitest __attribute__ ((section("cmdt"))) =
{
"tsit", tsit_cmd,
        "tsit [case]\n"
        "1; check boundary stop func.\n"
        "2; check ping-pong mode\n"
        "3; check stop transmit bit\n"
        "4; PID filter test\n"
        "5; enable destination change interrupt\n"
        "6; enable pkt done interrupt\n"};
#endif
