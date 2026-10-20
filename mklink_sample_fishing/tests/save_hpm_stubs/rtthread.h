#ifndef TEST_SAVE_RTTHREAD_H
#define TEST_SAVE_RTTHREAD_H
#include <stdint.h>
#include <stddef.h>
typedef intptr_t rt_base_t;
struct rt_mutex {int unused;};
struct rt_thread {int unused;};
struct rt_semaphore {int unused;};
#define RT_EOK 0
#define RT_WAITING_FOREVER -1
#define RT_IPC_FLAG_PRIO 1
#define RT_IPC_FLAG_FIFO 0
#define RT_NULL NULL
int rt_mutex_init(struct rt_mutex *,const char *,int);
int rt_mutex_take(struct rt_mutex *,int);
int rt_mutex_release(struct rt_mutex *);
int rt_sem_init(struct rt_semaphore *,const char *,unsigned,int);
int rt_sem_take(struct rt_semaphore *,int);
int rt_sem_release(struct rt_semaphore *);
int rt_thread_init(struct rt_thread *,const char *,void (*)(void *),void *,void *,unsigned,unsigned,unsigned);
int rt_thread_startup(struct rt_thread *);
rt_base_t rt_hw_interrupt_disable(void);
void rt_hw_interrupt_enable(rt_base_t);
#endif
