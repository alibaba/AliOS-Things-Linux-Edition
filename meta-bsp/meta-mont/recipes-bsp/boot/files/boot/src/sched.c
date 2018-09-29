#include <lib.h>
#include <arch/irq.h>
#include <arch/chip.h>
#include <common.h>
#include "sched.h"

void __thread_sleep_down_count_update(void);

static void panic(char *message)
{
    printf("PANIC: %s\n", message);
    while (1);
}

struct tcb threads[MAX_THREADS];

static volatile int sched_lock_count;
void sched_lock(void)
{
    if (sched_lock_count==0)
    {
        __asm__ __volatile__("di\n"
                             "nop\n"
                             : :);
    }
    sched_lock_count++;

    if (sched_lock_count > 32)
    {
        panic("sched_lock() bug");
    }
}

void sched_unlock(void)
{
    sched_lock_count--;
    if (sched_lock_count < 0)
    {
        panic("sched_unlock() bug");
    }

    if (sched_lock_count==0)
    {
        __asm__ __volatile__("ei\n"
                             "nop\n"
                             : :);
    }
}

int sched_yield(void)
{
    int i;

    if (sched_lock_count)
    {
        panic("sched_yield while holding lock");
    }

    sched_lock();
    enable_irq(IRQ_TMR1);
    sched_unlock();

    for (i=0;i<100;i++)
    {
        __asm__ __volatile__("nop\n"
                             "nop\n"
                             "nop\n"
                             : :);
    }

    return 0;
}

int thread_id(void)
{
    int i;
    unsigned long addr = (unsigned long) &i;

    for (i=0;i<MAX_THREADS;i++)
    {
        if ((addr < threads[i].stack_base) && (addr > (threads[i].stack_base - threads[i].stack_size)))
        {
            return i;
        }
    }

    return 0;
}

int ready_queue[MAX_THREADS];
int ready_queue_head;
int ready_queue_tail;

int ready_queue_dequeue(void)
{
    int thread_id = -1;

    if (ready_queue_head != ready_queue_tail)
    {
        thread_id = ready_queue[ready_queue_head];

        ready_queue_head = (ready_queue_head + 1) % MAX_THREADS;
    }

    return thread_id;
}

int ready_queue_enqueue(int thread_id)
{
    ready_queue[ready_queue_tail] = thread_id;

    ready_queue_tail = (ready_queue_tail + 1) % MAX_THREADS;

    if (ready_queue_tail==ready_queue_head)
    {
        panic("Ready queue overflow");
    }

    return 0;
}

void __thread_sleep_down_count_update(void)
{
    int i;

    for (i=0;i<MAX_THREADS;i++)
    {
        if ((threads[i].sleep_ticks_down_count) && (threads[i].state == THREAD_STATE_WAITING))
        {
            threads[i].sleep_ticks_down_count--;

            if (threads[i].sleep_ticks_down_count==0)
            {
                threads[i].state = THREAD_STATE_READY;
                ready_queue_enqueue(i);
            }
        }
    }
}

int thread_sleep(void)
{
    int curr_tid;

    curr_tid = thread_id();

    sched_lock();

    threads[curr_tid].flags |= TCB_FLAG_SLEEP;

    sched_unlock();

    sched_yield();

    return 0;
}

int thread_sleep_timeout(unsigned long ticks)
{
    int curr_tid;

    if (ticks > 0)
    {
        curr_tid = thread_id();

        sched_lock();

        threads[curr_tid].sleep_ticks_down_count = ticks;
        threads[curr_tid].flags |= TCB_FLAG_SLEEP;

        sched_unlock();
    }

    sched_yield();

    return 0;
}

void thread_wakeup(int thread_id)
{
    sched_lock();

    if (threads[thread_id].state==THREAD_STATE_WAITING)
    {
        threads[thread_id].state = THREAD_STATE_READY;
        ready_queue_enqueue(thread_id);
    }

    sched_unlock();
}

void thread_exit(void)
{
    int curr_tid;

    curr_tid = thread_id();

    sched_lock();

    threads[curr_tid].flags |= TCB_FLAG_EXIT;

    sched_unlock();

    sched_yield();
}

unsigned long scheduler(unsigned long sp)
{
    unsigned long new_sp;
    int curr_tid;
    int next_tid;
    unsigned long *stack_ptr;
    unsigned long *p;

    //printf("scheduler %lx\n", sp);
    if (sched_lock_count)
    {
        panic("run scheduler while scheduler is locked");
    }

    new_sp = sp;
    next_tid = ready_queue_dequeue();
    if (next_tid >= 0)
    {
        curr_tid = thread_id();

        if ((threads[next_tid].state == THREAD_STATE_READY) || (threads[next_tid].state == THREAD_STATE_NEW))
        {
            if (threads[curr_tid].state != THREAD_STATE_RUNNING)
                panic("Scheduler bug (1)");

            threads[curr_tid].curr_stack_pointer = sp;

            if (threads[curr_tid].flags & TCB_FLAG_EXIT)
            {
                threads[curr_tid].flags &= ~TCB_FLAG_EXIT;
                threads[curr_tid].state = THREAD_STATE_TERMINATED;
            } else if (threads[curr_tid].flags & TCB_FLAG_SLEEP)
            {
                threads[curr_tid].flags &= ~TCB_FLAG_SLEEP;
                threads[curr_tid].state = THREAD_STATE_WAITING;
            } else
            {
                threads[curr_tid].state = THREAD_STATE_READY;
                ready_queue_enqueue(curr_tid);
            }

#define STACK_DEPTH  (38 * 4)
            if (threads[next_tid].state == THREAD_STATE_NEW)
            {
                p = (unsigned long *) sp;
                stack_ptr = (unsigned long *) (threads[next_tid].stack_base - STACK_DEPTH);

                //printf("=========> stack %lx, func %lx\n", threads[next_tid].stack_base, threads[next_tid].entry_func);

                memset((void *)stack_ptr, 0, (38 *4));
                stack_ptr[0] = p[0];
                stack_ptr[1] = p[1];
                stack_ptr[4] = (unsigned long) threads[next_tid].data;
                stack_ptr[28] = p[28];
                stack_ptr[29] = (unsigned long) threads[next_tid].stack_base;
                stack_ptr[31] = (unsigned long) threads[next_tid].entry_func;
                stack_ptr[37] = (unsigned long) threads[next_tid].entry_func;

                threads[next_tid].curr_stack_pointer = (unsigned long) stack_ptr;
            }

            stack_ptr = (unsigned long *) threads[next_tid].curr_stack_pointer;

            threads[next_tid].state = THREAD_STATE_RUNNING;
            new_sp = threads[next_tid].curr_stack_pointer;

            //printf("Switch to thread %d, %lx\n", next_tid, stack_ptr[GPR9]);
        } else if (threads[next_tid].state == THREAD_STATE_RUNNING)
        {
            panic("Scheduler bug (2)");
        }
    }

    //printf("scheduler new_sp %lx\n", new_sp);
    return new_sp;
}

unsigned long scheduler_yield(unsigned long sp)
{
    //printf(".");
    //TMREG(T1IS) = 0;

    disable_irq(IRQ_TMR1);

    return scheduler(sp);
}

unsigned long scheduler_tick(unsigned long sp)
{
    //printf(".");
    TMREG(T0IS) = 1;

    __thread_sleep_down_count_update();
    return scheduler(sp);
}

int thread_create(void *entry_func, void *data, void *stack_base, int stack_size)
{
    int i;
    unsigned long align;

    align = ((unsigned long) stack_base) % 4;
    align += 4;

    //printf("====> %lx, %lx\n", entry_func, stack_base);
    for (i=0;i<MAX_THREADS;i++)
    {
        if (threads[i].state==THREAD_STATE_TERMINATED)
        {
            threads[i].entry_func = entry_func;
            threads[i].data = data;
            threads[i].stack_base = ((unsigned long) stack_base) - align;
            threads[i].stack_size = (stack_size - align);
            threads[i].sleep_ticks_down_count = 0;
            threads[i].flags = 0;
            threads[i].state = THREAD_STATE_NEW;

            sched_lock();
            ready_queue_enqueue(i);
            sched_unlock();

            return i;
        }
    }

    return -1;
}

//#define CREATE_IDLE_THREAD
#if defined(CREATE_IDLE_THREAD)
#define IDLE_THREAD_STACK_SIZE   (64 * 1024)
unsigned char idle_thread_stack[IDLE_THREAD_STACK_SIZE];
void idle_thread(void *data)
{
    do
    {
        printf("*");
        thread_sleep_timeout(100);
    }
    while (1);
}
#endif

void tick_init()
{
    void *pt = (void *) TM_BASE;

    request_irq(IRQ_TMR1, (void (*)(void *)) scheduler_yield, scheduler_yield);
    disable_irq(IRQ_TMR1);

    REG(pt, T1CR) = 0;
    REG(pt, T1LR) = 10;
    REG(pt, T1PR) = 2;
    REG(pt, T1CN) = (1<<2)|(1<<1);

    while ((TMREG(T1IS) & 0x1)==0)
        ;

    request_irq(IRQ_TMR0, (void (*)(void *)) scheduler_tick, scheduler_tick);

    REG(pt, T0CR) = 0;
    REG(pt, T0LR) = 1000000 / 100;  /* 1Mhz -> 100hz */
    REG(pt, T0PR) = 40 - 1;  /* 40Mhz -> 1Mhz */
    REG(pt, T0CN) = (1<<2)|(1<<1)|(1<<0);   /* b2: go, b1: ie, b0: auto-reload */
}

void sched_init()
{
    int i;

    for (i=0;i<MAX_THREADS;i++)
    {
        threads[i].flags = 0;
    }

    ready_queue_head = 0;
    ready_queue_tail = 0;

    threads[0].state = THREAD_STATE_RUNNING;

#if defined(CREATE_IDLE_THREAD)
    /* create idle thread */
    thread_create(idle_thread, (void *) 0, &idle_thread_stack[IDLE_THREAD_STACK_SIZE], IDLE_THREAD_STACK_SIZE);
#endif

    tick_init();

#if defined(CREATE_IDLE_THREAD)
    /* call sched_yield() will force scheduler() to run and start newly created thread immediately */
    sched_yield();
#endif
}

