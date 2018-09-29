/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file madc.c
*   \brief control mADC
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <lib.h>
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
#ifdef CONFIG_SCHED
#include <sched.h>
#endif
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

#ifdef	CONFIG_MADC

#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

#define ANAREG_UPDATE(x, val, mask) do {                            \
	ANAREG((x)) = (( ANAREG((x)) & ~(mask) ) | ( (val) & (mask) )); \
} while(0)

#define STDBY_PD_CTRL 0x0C
    #define STDBY_PD_MADC   0x04000000UL

int powerchan;
int madchan;
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
#if 0
int checkNotDuplicated(int *arr, int len, int val)
{
    int i = 0;
    for (; i < len; i++)
    {
        if (val == arr[i])
            return 0;
    }
    return 1;
}

void bsort(int *arr, int len)
{
    int swap = 1;
    int cmptimes = len - 1;
    int i = 0;
    int tmp = 0;
    while (swap)
    {
        swap = 0;
        for (i = 0; i < cmptimes; i++)
        {
            if (arr[i] > arr[i + 1])
            {
                tmp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = tmp;
                swap = 1;
            }
        }
        cmptimes--;
    }
}

void printVal(int *arr, int key)
{
    int i = 0;

    printf("\nValues are: \n");
    for (; i < key; i++)
    {
        printf("%d ", arr[i]);
    }
}

int madc_cmd(int argc, char *argv[])
{
    static unsigned int tick_thd = 0x50;
    unsigned int no = 0;
    unsigned int val = 0;
    char c = 0;
    int i;

    if (argc == 0)
    {
        return ERR_HELP;
    }
    else if (!strcmp("sample", argv[0]) && argc == 1)
    {
        unsigned short int time = 5000;
        unsigned short int count = 0;
        int key = 0;
        int t = 0;
        int notduplicated = 1;
        int sampleValue[4] = { 0 };
        int tmpVal[4] = { 0 };
        int tmpValNum = 0;
        int errorTolerance = 1;
        int oldVal = -1;
        int sampleTimes = 4;

        printf
            ("Please input the values from the keypad 1-6 sequentially in about %d(ms).\n",
             time);

        do
        {
            tmpValNum = 0;
            for (t = 0; t < sampleTimes; t++)
            {
                ANAREG_UPDATE(MADCCTL, BIT0, BIT0);
                mdelay(1);
                ANAREG_UPDATE(MADCCTL, (no << 4), (BIT4 | BIT0));
                mdelay(1);
                val = (tick_thd << 16) | BIT0;
                GPREG(MADCSAMP) = val;
                for (i = 0; i < 1000; i++)
                {
                    mdelay(1);
                    if (!(GPREG(MADCSAMP) & BIT0))      // Check bit0.
                        break;
                }
                val = (GPREG(MADCSAMP) >> 8) & 0xFF;

                if (val != 0)
                {
                    tmpVal[tmpValNum] = val;
                    printf("tmpVal[%d] is %d\n", tmpValNum, tmpVal[tmpValNum]);
                    ++tmpValNum;
                    if (tmpValNum >= 4)
                        break;
                }
            }                   // for 
            bsort(tmpVal, 4);
            val = (tmpVal[1] + tmpVal[2]) / 2;
            count += 100;
            notduplicated = checkNotDuplicated(sampleValue, key, val);
            if (val != 0 && notduplicated)
            {
                if (val > oldVal + errorTolerance
                    || val < oldVal - errorTolerance)
                {
                    sampleValue[key] = val;
                    oldVal = val;
                    key++;
                }
            }
            if (key >= 4)
                break;

            mdelay(100);
        }
        while (count < time);

        if (key >= 4)
        {
            bsort(sampleValue, 4);
            printVal(sampleValue, key);
            printf("\nOk\n");
            return ERR_OK;
        }
        else
        {
            printVal(sampleValue, key);
            printf
                ("\nThe number of sample values is less than target one. Please do it again.\n");
            goto err_arg;
        }
    }                           // madc sample
    else if (argc == 1)
    {
        if (!hextoul(argv[0], &no) || (no > 1))
            goto err_arg;

        ANAREG_UPDATE(MADCCTL, BIT0, BIT0);
        mdelay(1);
        ANAREG_UPDATE(MADCCTL, (no << 4), (BIT4 | BIT0));
        mdelay(1);
        val = (tick_thd << 16) | BIT0;
        GPREG(MADCSAMP) = val;
        for (i = 0; i < 1000; i++)
        {
            mdelay(1);
            printf(".");
            if (!(GPREG(MADCSAMP) & BIT0))
                break;
        }
        printf("\n");
        val = (GPREG(MADCSAMP) >> 8) & 0xFF;
        printf("MADC sample value: 0x%x(%d)\n", val, val);
        return ERR_OK;
    }
  err_arg:
    return ERR_PARM;
}

cmdt cmdt_madc[] __attribute__ ((section("cmdt"))) =
{
    {
    "madc", madc_cmd, "madc sample    (Sample keypad value)\n"
            "madc <ch>      (0:channle_1 1:channel_2 start to sample)\n"}
,};
#endif

int analog_read(int ch, int volt)
{
    int data = 0;
    PMUREG(STDBY_PD_CTRL) &= ~STDBY_PD_MADC;//MADC power ON
    udelay(50);
    ANAREG(0x34) |= 1 << 3;
    while(ANAREG(0x34) & (1 << 3));

    if(ch == 0)//CH I
        data = (ANAREG(0x3C) & 0x0fff);//J13
    else if(ch == 1)//CH Q
        data = (ANAREG(0x3C) & 0x0fff0000) >> 16;//J14
    else if (ch == 2) //got CH I and CH Q
    {
        data = (ANAREG(0x3C) & 0x0fff);//J13
        if(volt >= 0)
            printf("CHAN 0 VAL:%d VOLT:%01d.%01d\n", data, volt/10, volt%10);
        else
            printf("CHAN 0 VAL:%d\n", data);
        data = (ANAREG(0x3C) & 0x0fff0000) >> 16;//J14
        if(volt >= 0)
            printf("CHAN 1 VAL:%d VOLT:%01d.%01d\n", data, volt/10, volt%10);
        else
            printf("CHAN 1 VAL:%d\n", data);
        goto close;
    }

    if(volt >= 0)
        printf("CHAN:%d VAL:%d VOLT:%01d.%01d\n", ch, data, volt/10, volt%10);
    else
        printf("CHAN:%d VAL:%d\n",ch, data);
close:
    PMUREG(STDBY_PD_CTRL) |= STDBY_PD_MADC;//MADC power OFF
    return data;
}

extern void cheetah_putc(char c);
void set_chan_voltage(int chan, int volt)//set ch1 & 2 voltage resolution 0.01 V
{
    char madc_buf[20]={0};
    int pch, i;

    if(chan < 0 || chan > 3 || volt < 0 || volt > 36)
        return;

    for(pch=1; pch <= chan; pch++)
    {
        sprintf(madc_buf, ":CHAN%d:VOLT %01d.%01d\n", pch, volt/10, volt%10);

        //printf("%s", madc_buf);
        for(i = 0;i < strlen(madc_buf); i++)
            cheetah_putc(madc_buf[i]);
    }

    return;
}

void set_chan_current(int chan, int curr)
{
    char madc_buf[20]={0};
    int pch, i;

    if(chan < 0 || chan > 3 || curr < 0 || curr > 5)
         return;

    for(pch=1; pch <= chan; pch++)
    {
        sprintf(madc_buf, ":CHAN%d:CURR 0.%01d\n", pch, curr);

        //printf("%s", madc_buf);
        for(i = 0;i < strlen(madc_buf); i++)
            cheetah_putc(madc_buf[i]);
    }
    return;
}

static void set_output_onoff(int output)
{
    int i;
    char cmd[20]={0};

    if(output)
        sprintf(cmd, ":OUTP:STAT 1");
    else
        sprintf(cmd, ":OUTP:STAT 0");

    for(i = 0;i < strlen(cmd); i++)
        cheetah_putc(cmd[i]);
}

void auto_measure(void)
{
    int value;
    unsigned int regval;

    regval = URREG(URCS);
    URREG(URCS) = 0xb0cc0005; //9600 baudrate
    udelay(2000000);//2s

    set_output_onoff(0);
    {
        set_chan_voltage(powerchan, 0); // need to determine 1 or 2 chan
        set_chan_current(powerchan, 1);
        set_output_onoff(1);
        for(value = 0; value <= 36; value++)
        {
            set_chan_voltage(powerchan, value);
            udelay(2000000);//2s
            analog_read(madchan, value);
        }
        set_output_onoff(0);
        set_chan_voltage(powerchan, 0);
        set_chan_current(powerchan, 0);
    }
    URREG(URCS) = regval;
#if defined(CONFIG_SCHED)
    thread_exit();
#endif
}

#if defined(CONFIG_SCHED)
#define MADC_TEST_THREAD_STACK_SIZE  (128*1024)
unsigned char madc_test_thread_stack[MADC_TEST_THREAD_STACK_SIZE];
#endif

int madc_cmd(int argc, char *argv[])
{
    if(argc >= 1)
    {
        madchan = atoi(argv[0]);
        if((madchan > 2) || (madchan < 0)) // 0:CHI; 1:CHQ; 2:both CHI and CHQ
            goto done;
        if((argc > 1) && !strncmp(argv[1], "auto", 4))
        {
            if(argc > 2)
            {
                powerchan = atoi(argv[2]);
                if((powerchan > 2) || (powerchan <= 0))
                    goto done;
#if defined(CONFIG_SCHED)
                thread_create(auto_measure, 0,
                    &madc_test_thread_stack[MADC_TEST_THREAD_STACK_SIZE], MADC_TEST_THREAD_STACK_SIZE);
#else
                auto_measure();
#endif
            }
            else
            {
#if defined(CONFIG_SCHED)
                thread_create(auto_measure, 0,
                    &madc_test_thread_stack[MADC_TEST_THREAD_STACK_SIZE], MADC_TEST_THREAD_STACK_SIZE);
#else
                auto_measure();
#endif
            }
        }
        else
            analog_read(madchan, -1);
        return ERR_OK;
    }
done:
    return ERR_HELP;
}

cmdt cmdt_madc[] __attribute__ ((section("cmdt"))) =
{
    {
    "madc", madc_cmd, "madc sample (get adc value)\n"
            "madc <ch> --measue CHI or CHQ or both [ch:0/1/2]\n"
            "madc <ch> auto <pch> --auto measure adc value\n"}
,};
#endif
