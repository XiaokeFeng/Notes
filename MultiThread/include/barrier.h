#include "common_inc.h"

/*
 * barrier：所有注册barrier的线程都必须全部到达barrier，才能进行下一步计算
 * 用cycle来表示当前是第几轮，最后一个到达barrier的线程才能改动这个值
 */
typedef struct barrier_tag
{
    pthread_mutex_t     mutex;      /* control access to barrier */
    pthread_cond_t      cv;         /* wait for barrier */
    int                 valid;      /* set when valid */
    int                 threshold;  /* number of threads required */
    int                 counter;    /* current number of threads */
    /*
     * cycle：表示当前进行到第几轮计算了
     * 是唤醒其他先到达barrier的线程的谓词
     */
    unsigned long       cycle;      /* count cycles */
} barrier_t;

#define BARRIER_VALID 0xdbcafe

#define BARRIER_INITIALIZER(cnt) \
    {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, \
    BARRIER_BALID, cnt, cnt, 0}

/*
 * define barrier functions
 */
int barrier_init(barrier_t* barrier, int count);
int barrier_destroy(barrier_t* barrier);
int barrier_wait(barrier_t* barrier);
