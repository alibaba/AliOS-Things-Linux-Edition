/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file gpio_set.c
*   \brief set assign pin to gpio function
*   \author Montage
*/
/*=============================================================================+
| Included Files                                                               |
+=============================================================================*/
#include <arch/chip.h>
#include <common.h>
#include <mt_types.h>
#include <pmu.h>
#include <lib.h>

int pinmux_pin_func_gpio(int pin_number)
{
    int gpio_ids[] = { -1, -1 };
    unsigned long gpio_funcs[] = { -1, -1 };

    if((pin_number >= 0)&&(pin_number <= 16))
    {
        gpio_funcs[0] = 0;
    }
    else if((pin_number >= 17)&&(pin_number <= 20))
    {
        gpio_funcs[0] = 3;
    }
    else if((pin_number >= 21)&&(pin_number <= 22))
    {
        gpio_funcs[0] = 0;
    }
    else if((pin_number >= 23)&&(pin_number <= 24))
    {
        gpio_funcs[0] = 3;
    }
    if(gpio_funcs[0] >= 0)
    {
        gpio_ids[0] = pin_number;
        pmu_set_gpio_function(gpio_ids, gpio_funcs);
        return ERR_OK;
    }

    return ERR_HELP;
}

int gpio_set_cmd(int argc, char *argv[])
{
    int pin;

    if(argc >= 1) {
        pin = atoi(argv[0]);
        pinmux_pin_func_gpio(pin);
        return ERR_OK;
    }
    else
        return ERR_HELP;
}

cmdt cmdt_stb[] __attribute__ ((section("cmdt"))) =
{
    {
    "gpset", gpio_set_cmd, "gpset pinnum; set pinnum to gpio function\n"}
,};
