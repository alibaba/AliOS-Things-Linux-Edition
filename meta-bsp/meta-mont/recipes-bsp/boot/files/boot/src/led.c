/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file led.c
*   \brief LED Driver
*   \author Montage
*/
#ifdef CONFIG_LED
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <sys/types.h>
#include <led_api.h>
#include <arch/chip.h>
#include <common.h>
#include <cm_mac.h>

/*=============================================================================+
| Define                                                                       |
+=============================================================================*/

#define SECOND 1000
#define ETHP0_LED 0x098
#define ETHP1_LED 0x198
#define  LED_DARK_TH    0x7FF80000
#define  LED_LIGHT_TH   0x0007FF80
#define  LED_POLARITY   (1<<6)
#define  LED_COUNTUNIT  (1<<5)
#define  LED_AUTOMODE   (1<<4)
#define  LED_AUTOFREQ   0xC
#define  LED_MODE       0x3
#define  DARK_TH_SHIFT  19
#define  LIGHT_TH_SHIFT 7
#define  AUTOFREQ_SHIFT 2
/*=============================================================================+
| Variables                                                                    |
+=============================================================================*/
short led_num;
struct led_ctrl led[LED_MAX_NUN];

/*=============================================================================+
| Function Prototypes                                                          |
+=============================================================================*/
void led_timer_handle(void *data);

/*=============================================================================+
| Extern Function/Variables                                                    |
+=============================================================================*/

/*=============================================================================+
| Functions                                                                    |
+=============================================================================*/

void led_timer_handle(void *id)
{
    unsigned int time;
    struct led_ctrl *ledp;
    unsigned int msk;
    short i;

    time = clock_get();

    for (i = 0; i < led_num; i++)
    {
        ledp = &led[i];
        msk = (1 << ledp->bit);

        if (LED_BLINK == ledp->mode)
        {
            if (ledp->on & 1)
            {
                if (ledp->Onperiod > (time - ledp->time_stamp))
                    continue;
                else
                    ledp->on ^= 1;

            }
            else
            {
                if (ledp->Offperiod > (time - ledp->time_stamp))
                    continue;
                else
                    ledp->on ^= 1;

            }
            goto do_it;
        }

        if (ledp->Onperiod > (time - ledp->time_stamp))
            continue;

        if (LED_BLINK != ledp->mode)
        {
            ledp->Onperiod = LED_PERIOD_LONG;
            ledp->on = (LED_ON == ledp->mode) ? 1 : 0;
        }

      do_it:
        if (ledp->on & 1)
            GPREG(GPCLR) = msk;
        else
            GPREG(GPSET) = msk;
        ledp->time_stamp = time;
    }
}

/*!
 * function: led_set()
 *
 *      \brief
 *      \param
 *      \return
 */
void led_set(int id, int mode, int onperiod, int offperiod)
{
    struct led_ctrl *ledp;
    unsigned int time;
    unsigned long bit;
    short Onnewp = 0, Offnewp = 0;

    if (id >= led_num)
        return;

    ledp = &led[id];
    bit = (1 << ledp->bit);
    time = time = clock_get();

    if (LED_ON == mode)
    {
        Onnewp = LED_PERIOD_LONG;
        ledp->on = 1;
    }
    else if (LED_OFF == mode)
    {
        Onnewp = LED_PERIOD_LONG;
        ledp->on = 0;
    }
    else if (LED_BLINK == mode)
    {
        if (onperiod < LED_PERIOD_SHORT)
            onperiod = LED_PERIOD_SHORT;
        if (offperiod < LED_PERIOD_SHORT)
            offperiod = LED_PERIOD_SHORT;
        Onnewp = onperiod / 2;
        Offnewp = offperiod / 2;
        ledp->on = 1;
    }
    else if (LED_TOGGLE == mode)
    {
        if ((ledp->mode == LED_BLINK)
            || (time - ledp->time_stamp) < (LED_PERIOD_SHORT / 2))
            return;
        ledp->on ^= 1;
        Onnewp = LED_PERIOD_TOGGLE;
    }

    ledp->time_stamp = time;
    if (LED_TOGGLE != mode)
        ledp->mode = mode;
    if (ledp->on & 1)
        GPREG(GPCLR) = bit;
    else
        GPREG(GPSET) = bit;

    if ((Onnewp != ledp->Onperiod) || (Offnewp != ledp->Offperiod))
    {
        ledp->Onperiod = Onnewp;
        ledp->Offperiod = Offnewp;
    }
}

/*!
 * function: led_init()
 *
 *      \brief
 *      \param  pins : array of led's bit location
 *      \return void
 */
void led_init(char *pins)
{
    int i;
    struct led_ctrl *ledp;
    unsigned int led_gpio_mask = 0;

    memset(&led[0], 0, sizeof (led));   /* clear all */
    for (i = 0; i < LED_MAX_NUN; i++)
    {

        if (-1 == pins[i])
            break;

        ledp = &led[i];
        ledp->Onperiod = LED_PERIOD_LONG;
        ledp->Offperiod = LED_PERIOD_LONG;
        ledp->bit = pins[i];
        led_gpio_mask |= (1 << ledp->bit);
    }
    led_num = i;
    GPREG(GPDIR) |= led_gpio_mask;      //output
    GPREG(GPSET) = led_gpio_mask;       //off
    GPREG(GPSEL) |= led_gpio_mask;      //gpio mode
}

/*!
 * function: led_hw_init()
 *
 *      \brief
 *      \return void
 */
void led_hw_init(void)
{
    int idx = 1;
    //printf("0=%x\n",cm_phy_status(0));
    //printf("1=%x\n",cm_phy_status(1));
    //only one ethernet port in bootloader
#ifdef CONFIG_P0_RMII
    idx = 0;
#endif
    if (cm_phy_status(idx) & PHY_LINK)
        SWREG((idx ? ETHP1_LED : ETHP0_LED)) &= ~LED_POLARITY;
}

/*!
 * function: led_on()
 *
 *      \brief
 *      \param  idx : led index
 *      \return void
 */
void led_on(int idx)
{
    led_set(idx, LED_ON, 0, 0);
}

/*!
 * function: led_off()
 *
 *      \brief
 *      \param  idx : led index
 *      \return void
 */
void led_off(int idx)
{
    led_set(idx, LED_OFF, 0, 0);
}

/*!
 * function: led_toggle()
 *
 *      \brief
 *      \param  idx : led index
 *      \return void
 */
void led_toggle(int idx)
{
    led_set(idx, LED_TOGGLE, 0, 0);
}

/*!
 * function: led_blink()
 *
 *      \brief
 *      \param  idx : led index
 *      \return void
 */
void led_blink(int idx, int Onperiod, int Offperiod)
{
    led_set(idx, LED_BLINK, Onperiod, Offperiod);
}

/*!
 * function: led_hw_dump()
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return void
 */
void led_hw_dump(void)
{
    int idx = 1;
    unsigned int *p;
    unsigned int data;
    //only one ethernet port in bootloader
#ifdef CONFIG_P0_RMII
    idx = 0;
#endif
    p = (void *) &SWREG((idx ? ETHP1_LED : ETHP0_LED));
    data = *p;
    printf("%08x=%08x\n", p, data);
    if (!(data & LED_AUTOMODE))
    {
        printf("[auto mode]\n");
        printf("freq:%d\n", (data & LED_AUTOFREQ) >> AUTOFREQ_SHIFT);
    }
    else
    {
        printf("[non-auto mode]\n");
        printf("dark TH:%d\n", (data & LED_DARK_TH) >> DARK_TH_SHIFT);
        printf("light TH:%d\n", (data & LED_LIGHT_TH) >> LIGHT_TH_SHIFT);
        printf("count by %s\n", (data & LED_COUNTUNIT) ? "packet" : "timer");
    }
}

/*!
 * function: led_cmd()
 *
 *  \brief
 *  \param argc
 *  \param argv
 *  \return void
 */
int led_cmd(int argc, char *argv[])
{
    int ton, toff, idx;
    char *cmd;

    if (2 > argc)
        goto help;

    sscanf(argv[0], "%d", &idx);

    cmd = argv[1];
    if (!strcmp(cmd, "blink"))
    {
        ton = 1 * SECOND;
        toff = 1 * SECOND;
        if (3 < argc && (1 != sscanf(argv[2], "%d", &ton)))
        {
            printf("period not number!\n");
            goto help;
        }
        if (3 < argc && (1 != sscanf(argv[3], "%d", &toff)))
        {
            printf("period not number!\n");
            goto help;
        }
        led_blink(idx, ton, toff);
        printf("Led blink, On period=%d, off period=%d\n", ton, toff);
    }
    else if (!strcmp(cmd, "toggle"))
        led_toggle(idx);
    else if (!strcmp(cmd, "on"))
        led_on(idx);
    else if (!strcmp(cmd, "off"))
        led_off(idx);
    else if (!strcmp(cmd, "hw"))
        led_hw_dump();
    else
        goto help;
    return ERR_OK;
  err1:
    return ERR_PARM;
  help:
    return ERR_HELP;
}

cmdt cmdt_led[] __attribute__ ((section("cmdt"))) =
{
    {
    "led", led_cmd, "led [idx] on/off/blink[on off]/toggle/hw ;LED Cmd"}
,};
#endif
