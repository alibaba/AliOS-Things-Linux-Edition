/*=============================================================================+
|                                                                              |
| Copyright 2016                                                               |
| Montage Technology, Inc. All rights reserved.                                |
|                                                                              |
+=============================================================================*/
/*!
*   \file clock.c
*   \brief Timer Driver
*   \author Montage
*/

#include <arch/chip.h>
#include <common.h>

#define UINT_MAX 0xffffffffU

unsigned int jiffies;

static unsigned int lastcvr;
#if defined(CONFIG_FPGA)
#define CONFIG_CPU_CLK 75000000
#else
#define CONFIG_CPU_CLK 550000000
#endif
#define CPU_CYCLE_PER_MS ((CONFIG_CPU_CLK/2)/1000)        //P.S. CPU count run at half CPU clock rate
#define CPU_CYCLE_PER_US ((CONFIG_CPU_CLK/2)/1000000)
/*!
 * function:
 *
 *  \brief
 *  \param
 *  \return
 */

void clock_init(void)
{
    unsigned int cycles;
    asm volatile ("mfc0 %0, $9":"=r" (cycles):);
    //printf("boot time: %dms\n", cycles / CPU_CYCLE_PER_MS);
    asm volatile ("mtc0 $0, $9");
    lastcvr = 0;
}

/*!
 * function:
 *
 *  \brief
 *  \param
 *  \return
 */

unsigned int clock_get(void)
{
    unsigned int now, diff;

    asm volatile ("mfc0 %0, $9":"=r" (now):);

    diff = now - lastcvr;

    if (diff < CPU_CYCLE_PER_MS)
        return jiffies;
    else
        diff /= CPU_CYCLE_PER_MS;

    lastcvr = now;
    jiffies += diff;
    return jiffies;
}

unsigned int how_long(unsigned int last)
{
    unsigned int d;

    d = clock_get();
    return ((d >= last) ? (d - last) : (UINT_MAX - last + d));
}

#if defined(CONFIG_BOOT1_MINI_SDHC)
unsigned int cpu_c0count(void)
{
    unsigned int now;

    asm volatile ("mfc0 %0, $9":"=r" (now):);

    return now;
}

unsigned int cpu_c0delta(unsigned int last)
{
    unsigned int d;

    d = cpu_c0count();
    return ((d >= last) ? (d - last) : (UINT_MAX - last + d));
}

void udelay(unsigned int time)
{
    register unsigned int last;
    register unsigned int target_delta_time_us;

    target_delta_time_us = time * CPU_CYCLE_PER_US;

    last = cpu_c0count();
    for (;;)
    {
        if (cpu_c0delta(last) >= target_delta_time_us)
            break;
    }
}

void delay(unsigned int time)
{
    register unsigned int last;

    last = clock_get();
    for (;;)
    {
        if (how_long(last) >= time)
            break;
    }
}

inline void mdelay(unsigned int time)
{
    delay(time);
}
#endif

#if defined(CONFIG_FPGA)
void idelay(unsigned int count)
{
    __asm__ __volatile__(".set noreorder\n"
                         "1:\n"
                         "bnez    %0, 1b\n"
                         "subu    %0, 1\n"
                         ".set reorder\n":"=r"(count):"0"(count));
}
#endif

