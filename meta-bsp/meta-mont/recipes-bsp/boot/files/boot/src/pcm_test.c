/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file pcm_test.c
*   \brief Some Test Cases for PCM
*   \author Montage
*/

#ifdef CONFIG_AI
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <common.h>
#include <arch/chip.h>
#include <ai.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/
#define TST_OK   0
#define TST_ERR -1
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#define tst_no (sizeof(pcm_test)/sizeof(int))
static char test_str[][40] = {
    "Gate PCM module clock",
    "PCM module reset",
    "PCM TX Underrun Counter",
    "PCM RX Overrun Counter",
    "PCM TimeSlot Scan",
};

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
int pt0(int);
int pt1(int);
int pt2(int);
int pt3(int);
int pt4(int);
static int (*pcm_test[]) (int) =
{
pt0, pt1, pt2, pt3, pt4,};

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/
extern struct ai_dev *aidev;
extern struct ai_ch *aich;
/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
int pt0(int nouse)
{
#ifdef CONFIG_FPGA
    return TST_OK;
#else
    int tmp = 3;
    GPREG(SWRST) |= (PAUSE_AI0);        // stop pcm clk
    AIREG(CH0_TXBASE) = tmp;    //write pcm register
    if (AIREG(CH0_TXBASE) != tmp)
        return TST_OK;
    else
        return TST_ERR;
#endif
}

int pt1(int nouse)
{
    int tmp;
    int rc = TST_OK;
    GPREG(SWRST) &= ~(PAUSE_AI0);       // release pcm clk

    /* reset pcm module */
    ai_reset_module();

    tmp = AIREG(CFG);           //read defualt vaule
    AIREG(CFG) = 0x12345678;    //change default vaule

    /* reset pcm module */
    ai_reset_module();

    if (AIREG(CFG) != tmp)
        rc = TST_ERR;

    /* re-int audio interface because reset module */
    ai_init();
    return rc;
}

int pt2(int nouse)
{
    int i, j;
    int no;
    unsigned int addr = 0x80140000;
    unsigned int bs = (1 << aidev->buf_size) * 80;
    volatile struct ai_des *des;
    struct ai_ch *ch = aich;
    int s_delay, e_delay;

    /* init channal */
    ai_ch_init(ch);

    /* how many bytes tx at 1ms
       8k-8bit   -> 8 Bytes
       8k-16bit  -> 16 Bytes
       16k-8bit  -> 16 Bytes
       16k-16bit -> 32 Bytes
     */
    no = ai_tx_rate(ch);
    printf("\n");
    printf("within 1ms, tx: %dByte\n", no);
    /* show delay how long tx a integrated buffer */
    printf("tx an integrated buffer need: %dms\n", bs / no);

    if (nouse != 0)
    {
        s_delay = e_delay = nouse;
    }
    else
    {
        s_delay = bs / no;
        e_delay = s_delay << 1;
    }
    printf("between 1's and 2's tx,delay: %d ~ %dms\n", s_delay, e_delay);
    printf("expected count:");
    for (i = s_delay; i <= e_delay; i++)
    {
        printf("\t%d", (i * no - bs));
    }

    printf("\n");
    printf("underrun count:");

    for (i = s_delay; i <= e_delay; i++)
    {
        /* get second Tx descriptor ptr */
        des = ch->txd + ((ch->txnext + 1) == NUM_DES ? 0 : ch->txnext + 1);
        //printf("des = 0x%x\n",des);

#if 1                           //?
        mdelay(s_delay);
#endif
        /* first Tx */
        if (ai_tx(ch, (void *) addr, bs))
            printf("tx des not ready");

        /* test delay time */
        mdelay(i);

        /* second Tx */
        if (ai_tx(ch, (void *) addr, bs))
            printf("tx des not ready");

#if 0                           //?
        /* test delay time */
        mdelay(s_delay);
#endif

        /* show descriptor's underrun_counter */
        for (j = 0; j < CHECK_LOOP; j++)
        {
            if (des->info & DES_OWN)
            {
                //printf("underrun count: %dByte\n", des->info&CNT_MASK);
                printf("\t%d", des->info & CNT_MASK);
                break;
            }
        }
        if (j == CHECK_LOOP)
            printf("\tN/A");
    }

    return TST_OK;
}

int pt3(int nouse)
{
    int i, j;
    int no;
    unsigned int bs = (1 << aidev->buf_size) * 80;
    volatile struct ai_des *des;
    struct ai_ch *ch = aich;
    int s_delay, e_delay;

    /* init channal */
    ai_ch_init(ch);

    /* how many bytes rx at 1ms
       8k-8bit   -> 8 Bytes
       8k-16bit  -> 16 Bytes
       16k-8bit  -> 16 Bytes
       16k-16bit -> 32 Bytes
     */
    no = ai_tx_rate(ch);
    printf("\n");
    printf("within 1ms, rx: %dByte\n", no);
    /* show delay how long rx a integrated buffer */
    printf("rx an integrated buffer need: %dms\n", bs / no);

    if (nouse != 0)
    {
        s_delay = e_delay = nouse;
    }
    else
    {
        s_delay = bs / no;
        e_delay = s_delay << 1;
    }
    printf("between 1's and 2's rx,delay: %d ~ %dms\n", s_delay, e_delay);
    printf("expected count:");
    for (i = s_delay; i <= e_delay; i++)
    {
        printf("\t%d", (i * no - bs));
    }

    printf("\n");
    printf("overrun count :");
    for (i = s_delay; i <= e_delay; i++)
    {
        /* get second Rx descriptor ptr */
        des = ch->rxd + ((ch->rxnext + 1) == NUM_DES ? 0 : ch->rxnext + 1);
        //printf("des = 0x%x\n",des);

        /* init rx dst ptr */
        ch->rx_ptr = ch->rx_daddr;

        mdelay(40);

        /* first Rx */
        ai_rx_poll(ch, bs, ai_rx_cb_memcpy);

        /* constant delay */
        mdelay(i);

        /* second Rx */
        ai_rx_poll(ch, bs, ai_rx_cb_memcpy);

        /* show descriptor's underrun_counter */
        for (j = 0; j < CHECK_LOOP; j++)
        {
            if (des->info & DES_OWN)
            {
                //printf("underrun count: %dByte\n", tx_des->info&CNT_MASK);
                printf("\t%d", des->info & CNT_MASK);
                break;
            }
        }
        if (j == CHECK_LOOP)
            printf("\tN/A");
        //mdelay(40);
    }

    return TST_OK;
}

static unsigned long powers[9] = {
    0x0L, 0xfL, 0xffL, 0xfffL, 0xffffL,
    0xfffffL, 0xffffffL, 0xfffffffL, 0xffffffffL
};

static char digits[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/* external function definitions */
int ultohex(long unsigned int value, char *result)
{
    int place;

    /* determine number of places to be used */
    if (value == 0)
        place = 1;
    else
        for (place = 1; value > powers[place]; place++) ;

    int rc = place + 1;         /* return code */

    result[place] = '\0';
    while (place-- > 0)
    {
        result[place] = digits[value % 16];
        value /= 16;
    }

    return rc;

    /* NOTREACHED */
}

int pt4(int nouse)
{
    int i, j, k;
    int maxslt;
    struct ai_ch *ch;
    char *tmp;
    char *tmps;
    char str[11];
    unsigned int daddr = 0xa0500000;
    unsigned int len = 0x10000;
    char saddr_str[11] = "0xbe000000";
    char daddr_str[11] = "0xa0500000";
    char len_str[11] = "0x10000";
    char chno[2][2] = { "0", "1" };

    for (k = 0; k < NUM_CH; k++)
    {
        ch = aich + k;

        /* calculate valid timeslot */
        maxslt = 4 << aidev->freq;
        if (aidev->fs_rate)
            maxslt >>= 1;
        if (ch->mode)
            maxslt--;
        maxslt--;
        printf("max slot = %d\n", maxslt);

        for (i = 0; i <= maxslt; i++)
        {
            /* set slot */
            ch->slot = i;
            printf("test slot = %d\n", i);

            /* turn on internal loopback */
            ch->loopback_int = 1;
            /* init channal */
            ai_ch_init(ch);

            /* do loopback */
            aitr_cmd(4, (char *[])
                     {
                     chno[k], saddr_str, len_str, daddr_str});
            /* turn off internal loopback */
            ch->loopback_int = 0;
            /* init channal */
            ai_ch_init(ch);

            tmp = (void *) daddr;

            /* find rx data offset */
            hextoul(saddr_str, &j);
            tmps = (char *) j;
            for (j = 0; j < len; j++)
            {
                if (!memcpy(tmp, tmps, 4))
                    break;
                else
                    tmp++;
            }
            printf("data in 0x%x\n", tmp);
            ultohex((unsigned int) tmp, str);
            //printf("data in %s\n",str);
            if (cmd_cmp(2, (char *[])
                        {
                        saddr_str, str, len_str}))
            {
                //printf("
                goto err;
            }

#if 0
            /* show rx data */
            mem_dump_cmd(2, (char *[])
                         {
                         "dw", str, "0x230"} +1);
#endif
        }

        /* turn off channel */
        ai_disable_channel(ch);
    }
    /* turn on channel */
    for (k = 0; k < NUM_CH; k++)
        ai_enable_channel(aich + k);

    return TST_OK;
  err:
    return TST_ERR;
}

int pcm_tst(int argc, char *argv[])
{
    int i, end;
    int no = -1;
    int arg2 = 0;

    if (argc > 0)               //argument
    {
        if (!hextoul(argv[0], &no))
            goto err;
        if (no >= tst_no)
            goto err;
        if (argc > 1 && sscanf(argv[1], "%x", &arg2) != 1)
            goto err;
    }

    if (no >= 0)
    {
        i = no;
        end = i + 1;
    }
    else
    {
        i = 0;
        end = tst_no;
        printf("scan from test 0 ~ %d\n", tst_no - 1);
    }
    for (; i < end; i++)
    {
        printf("%d. Test %s .. ", i, test_str[i]);
        if (pcm_test[i] (arg2) != TST_OK)
        {
            goto err;
        }
        else
            printf("ok\n");
    }
    return ERR_OK;
  err:
    return ERR_PARM;
}

cmdt cmdt_pcm_test[] __attribute__ ((section("cmdt"))) =
{
    {
    "pt", pcm_tst, "pt <case>;pcm test\n" "pt 2 <dec>\n"}
,};

#endif                          /* CONFIG_AI */
