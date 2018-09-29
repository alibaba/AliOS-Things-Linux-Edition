/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file led_api.h
*   \brief LED API
*   \author Montage
*/

#ifndef  LED_API_H
#define  LED_API_H

#define LED_STATUS  0
#define LED_POWER   1
#define LED_MAX_NUN 4
enum
{
    LED_ON = 1,
    LED_OFF = 0,
    LED_TOGGLE = 2,
    LED_BLINK = 3,

    LED_PERIOD_TOGGLE = 100,
    LED_PERIOD_SHORT = 10,
    LED_PERIOD_LONG = (unsigned short) (-1),
};

struct led_ctrl
{
    unsigned int time_stamp;
    unsigned short Onperiod;
    unsigned short Offperiod;
    char bit;
    char mode;
    char on;
};

void led_init(char *gpio);
void led_on(int idx);
void led_off(int idx);
void led_blink(int idx, int Onperiod, int Offperiod);
void led_toggle(int idx);

#endif                          /* LED_API_H */
