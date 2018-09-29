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

#include <arch/irq.h>
#include <arch/chip.h>
#include <common.h>

#define UINT_MAX 0xffffffffU

#define CONFIG_TIMER_CLK CONFIG_REF_CLK
#define CLOCK_MAX ((1<<20)-1)
#define TLR_INI CLOCK_MAX
#define PR       CONFIG_SYS_CLK_PR

#define ISTS_VAL (1<<IRQ_TMR1)  /* timer1 */
#define TCN_INI  (1<<2)|(1<<1)|(1<<0)   /* b2: go, b1: ie, b0: auto-reload */
#define T1PR_INI (PR-1)

#define TICKS_PER_SEC   (CONFIG_TIMER_CLK/PR)
#define TICKS_PER_MS    ((TICKS_PER_SEC/CONFIG_SYS_HZ)-1)

unsigned int jiffies;

#ifdef CONFIG_CPU_COUNT
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

#if defined(BURST_READ_TEST)
unsigned int clock_get_clk(void)
{
    unsigned int now, diff;

    asm volatile ("mfc0 %0, $9":"=r" (now):);

    diff = now - lastcvr;

    lastcvr = now;
    jiffies += diff;
    return jiffies;
}
#endif

#else
#ifndef CONFIG_CLOCK_NOISR

/*!
 * function:
 *
 *  \brief
 *  \param id irq number
 *  \return
 */

void tmr_handler(void *id)
{
    TMREG(T1IS) = 1;            /* clear */
    jiffies++;
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

void clock_init()
{
    void *pt = (void *) TM_BASE;

    request_irq(IRQ_TMR1, &tmr_handler, 0);
    REG(pt, T1LR) = TICKS_PER_MS;
    REG(pt, T1PR) = T1PR_INI;
    REG(pt, T1CN) = TCN_INI;
    jiffies = 0;
    enable_irq(IRQ_TMR1);
}

/*!
 * function:
 *
 *  \brief
 *  \return
 */

unsigned int clock_get(void)
{
    return jiffies;
}

#else

#if ((CONFIG_TIMER_CLK/CONFIG_SYS_HZ) > CLOCK_MAX)
#error  "Timer precaler too large"
#else
#define PR_INI  ((CONFIG_TIMER_CLK/CONFIG_SYS_HZ)-1)
#endif

static unsigned int lastcvr;
/*!
 * function:
 *
 *  \brief
 *  \param
 *  \return
 */

void clock_init()
{
    void *pt = (void *) TM_BASE;

    REG(pt, T1LR) = TLR_INI;
    REG(pt, T1CN) = TCN_INI;
    REG(pt, T1PR) = PR_INI;
    lastcvr = TLR_INI;
    jiffies = 0;
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

    now = TMREG(T1CR);
    if (now >= lastcvr)
        diff = now - lastcvr;
    else
        diff = now + (TLR_INI - lastcvr);

    if (diff <= 0)
        return jiffies;

    lastcvr = now;
    jiffies += diff;
    return jiffies;
}
#endif
#endif

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

/*!
 * function:
 *
 *  \brief
 *  \param last
 *  \return
 */

unsigned int how_long(unsigned int last)
{
    unsigned int d;

    d = clock_get();
    return ((d >= last) ? (d - last) : (UINT_MAX - last + d));
}

/*!
 * function: mdelay
 *
 *  \brief delay specific millisecond
 *  \param time delay time
 *  \return none
 */

inline void mdelay(unsigned int time)
{
    delay(time);
}

/*!
 * function: idelay
 *
 *  \brief delay specific instructions
 *  \param instruction count
 *  \return none
 */
void idelay(unsigned int count)
{
    __asm__ __volatile__(".set noreorder\n"
                         "1:\n"
                         "bnez    %0, 1b\n"
                         "subu    %0, 1\n"
                         ".set reorder\n":"=r"(count):"0"(count));
}
