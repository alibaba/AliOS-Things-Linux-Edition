#ifndef __SCHED_H__
#define __SCHED_H__

void sched_init(void);
void sched_lock(void);
void sched_unlock(void);
int sched_yield(void);

int thread_id(void);
int thread_sleep(void);
int thread_sleep_timeout(unsigned long ticks);
void thread_wakeup(int thread_id);
void thread_exit(void);

#define MAX_THREADS 8

#define THREAD_STATE_TERMINATED  0
#define THREAD_STATE_READY       1
#define THREAD_STATE_RUNNING     2
#define THREAD_STATE_WAITING     3
#define THREAD_STATE_NEW         4

struct tcb
{
   unsigned long stack_base;
   int stack_size;
   unsigned long curr_stack_pointer;
   void *entry_func;
   void *data;
   unsigned long state;

#define TCB_FLAG_SLEEP  0x01
#define TCB_FLAG_EXIT   0x02

   unsigned long sleep_ticks_down_count;
   unsigned long flags;
};

int thread_create(void *entry_func, void *data, void *stack_base, int stack_size);


#endif // __SCHED_H__


