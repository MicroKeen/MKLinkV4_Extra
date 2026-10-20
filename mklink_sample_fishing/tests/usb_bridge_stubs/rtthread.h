#ifndef TEST_BRIDGE_RTTHREAD_H
#define TEST_BRIDGE_RTTHREAD_H
struct rt_mutex {int unused;};
#define RT_EOK 0
#define RT_WAITING_FOREVER -1
#define RT_IPC_FLAG_PRIO 1
int rt_mutex_init(struct rt_mutex *,const char *,int);
int rt_mutex_take(struct rt_mutex *,int);
int rt_mutex_release(struct rt_mutex *);
#endif
