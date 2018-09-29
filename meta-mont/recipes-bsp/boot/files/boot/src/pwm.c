/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file pwm.c
*   \brief control PWM
*   \author Montage
*/
#ifdef CONFIG_PWM
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

struct pwm_dev
{
    int init;
    int enable;
    int mode;
    int duty;
    int sweep;
    int polarity;
};

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
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/

struct pwm_dev devA = {
  init:0,
  enable:1,
  mode:0,
  duty:31,
  sweep:0,
  polarity:1,
};

struct pwm_dev devB = {
  init:0,
  enable:1,
  mode:0,
  duty:31,
  sweep:0,
  polarity:1,
};

#ifdef CONFIG_FPGA
#else
#endif

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/
void pwm_show_info(struct pwm_dev *dev)
{
    printf("\tEnable     :%d\n", dev->enable);
    printf("\tAuto Mode  :%d\n", dev->mode);
    printf("\tDuty Cycle :%d\n", dev->duty);
    printf("\tSweep Freq :%d\n", dev->sweep);
    printf("\tPolarity   :%d\n", dev->polarity);
}

int pwm_cmd(int argc, char *argv[])
{
    static int issync = 0;
    struct pwm_dev *dev;
    unsigned int no = 0;
    unsigned int val = 0;
    unsigned int val0 = 0;
    unsigned int val1 = 0;
    unsigned int val2 = 0;
    unsigned int val3 = 0;
    unsigned int val4 = 0;

    if (argc == 0)
    {
        return ERR_HELP;
    }
    else if (argc == 1)
    {
        if (!strcmp("info", argv[0]))
        {
            if (issync)
                printf("PWM Dev A/B are sync\n");
            if (!(devA.init) || !(devB.init))
                printf("PWM Not SET YET\n");
            else
            {
                printf("PWM A:\n");
                pwm_show_info(&devA);
                printf("PWM B:\n");
                pwm_show_info(&devB);
            }
            return ERR_OK;
        }
    }
    else if (argc == 2)
    {
        if (!strcmp("sync", argv[0]))
        {
            if (!hextoul(argv[1], &no) || (no > 1))
                goto err_arg;
            issync = no;
            printf("%s sync devs\n", no ? "enable" : "disable");
            return ERR_OK;
        }
        if (!strcmp("sweep", argv[1]))
        {
            if ((argv[0][0] != 'a') && (argv[0][0] != 'b'))
                goto err_arg;
            dev = (argv[0][0] == 'a') ? (&devA) : (&devB);
            if (dev->mode)
            {
                printf("auto mode");
                printf
                    ("0:0.1s 1:0.2s 2:0.4s 3:0.8s 4:1.6s 5:3.2s 6:6.4s 7:13s\n");
            }
            else
            {
                printf("non auto mode");
                printf
                    ("0:3.3ms 1:0.1s 2:0.2s 3:0.4s 4:0.8s 5:1.6s 6:3.2s 7:6.4s\n");
            }
            return ERR_OK;
        }
    }
    else if (argc == 3)
    {
        if ((argv[0][0] != 'a') && (argv[0][0] != 'b'))
            goto err_arg;
        dev = (argv[0][0] == 'a') ? (&devA) : (&devB);
        if (!strcmp("modu", argv[1]))
        {
            if (!hextoul(argv[2], &no) || (no > 1))
                goto err_arg;
            dev->enable = no;
        }
        else if (!strcmp("auto", argv[1]))
        {
            if (!hextoul(argv[2], &no) || (no > 1))
                goto err_arg;
            dev->mode = no;
        }
        else if (!strcmp("duty", argv[1]))
        {
            if (1 != sscanf(argv[2], "%d", &no) || (no > 31))
                goto err_arg;
            dev->duty = no;
        }
        else if (!strcmp("sweep", argv[1]))
        {
            if (!hextoul(argv[2], &no) || (no > 7))
                goto err_arg;
            dev->sweep = no;
        }
        else if (!strcmp("pol", argv[1]))
        {
            if (!hextoul(argv[2], &no) || (no > 1))
                goto err_arg;
            dev->polarity = no;
        }
        // switch PINMUX
        val = GPREG(PINMUX);
        if (GPREG(PINMUX) & EN_SIP_FNC) // DDR-QFN88-PWM
        {
            val &= ~(EN_PWM_AUX_FNC | EN_ADC_OUT_FNC);
            val |= EN_PWM_FNC;
        }
        else                    // SDR-QFN128-PWM_AUX
        {
            val &= ~(EN_PWM_FNC | EN_AGC_FNC);
            val |= EN_PWM_AUX_FNC;
        }
        GPREG(PINMUX) = val;

        if (issync)
        {
            memcpy(&devB, &devA, sizeof (struct pwm_dev));
            printf("devB is sync from devA\n");
            GPREG(PWM) = 0;
        }

        // do the settings update
        val0 = (devB.polarity << 21) | (devA.polarity << 20);
        val1 = (devB.sweep << 17) | (devA.sweep << 14);
        val2 = (devB.duty << 9) | (devA.duty << 4);
        val3 = (devB.mode << 3) | (devA.mode << 2);
        val4 = (devB.enable << 1) | (devA.enable << 0);
        val = val0 | val1 | val2 | val3 | val4;
        GPREG(PWM) = val;
        devA.init = 1;
        devB.init = 1;
        return ERR_OK;
    }

  err_arg:
    return ERR_PARM;
}

cmdt cmdt_pwm[] __attribute__ ((section("cmdt"))) =
{
    {
    "pwm", pwm_cmd, "pwm info           (display settings)\n"
            "pwm sync      <no> (0:disbale  1:enable sync dev)\n"
            "pwm dev modu  <no> (0:disbale  1:enable module)\n"
            "pwm dev auto  <no> (0:non-auto 1:auto mode)\n"
            "pwm dev duty  <no> (0~31 duty cycle)\n"
            "pwm dev sweep <no> (0~7 period)\n"
            "pwm dev pol   <no> (0:low 1:high output active)\n"}
,};

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
    else if (argc == 2)
    {
        if (!strcmp("tick", argv[0]))
        {
            if (1 != sscanf(argv[1], "%d", &no) || (no > 255))
                goto err_arg;
            tick_thd = no;
            return ERR_OK;
        }
        if (!strcmp("auto", argv[0]))
        {
            if (!hextoul(argv[1], &no) || (no > 1))
                goto err_arg;

            printf("Press 'B' or 'b' to break!!\n");
            do
            {
                if (tstc())
                    c = getchar();
                if (c == 'B' || c == 'b')
                    break;

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
            }
            while (1);

            return ERR_OK;
        }
    }

  err_arg:
    return ERR_PARM;
}

cmdt cmdt_madc[] __attribute__ ((section("cmdt"))) =
{
    {
    "madc", madc_cmd,
            "madc <ch>      (0:channle_1 1:channel_2 start to sample)\n"
            "madc auto <no> (0:channle_1 1:channel_2 start to sample)\n"
            "madc tick <no> (0~255 tick threshold; default 80)\n"}
,};
#endif
