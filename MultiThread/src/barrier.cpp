#include "barrier.h"

int barrier_init(barrier_t* barrier, int count)
{
    int status;

    barrier->threshold = barrier->counter = count;
    barrier->cycle = 0;

    status = pthread_mutex_init(&barrier->mutex, NULL);
    assert(status == 0);

    status = pthread_cond_init(&barrier->cv, NULL);
    assert(status == 0);

    barrier->valid = BARRIER_VALID;

    return 0;
}

// return 0: is ok
// return -1: is error
// return -2: is busy
// return other: mutex or cond destroy failed
int barrier_destroy(barrier_t* barrier)
{
    int status;
    int status2;

    if (barrier->valid != BARRIER_VALID)
        return -1;

    status = pthread_mutex_lock(&barrier->mutex);
    assert(status == 0);

    // check counter
    if (barrier->counter != barrier->threshold)
    {
        pthread_mutex_unlock(&barrier->mutex);
        return -2; /* busy */
    }

    barrier->valid = 0;
    status = pthread_mutex_unlock(&barrier->mutex);
    assert(status == 0);

    status = pthread_mutex_destroy(&barrier->mutex);
    status2 = pthread_cond_destroy(&barrier->cv);
    return (status != 0 ? status : status2);
}

/* 
 * wait for all members of a barrier to reach the barrier.
 */
int barrier_wait(barrier_t* barrier)
{
    int status;
    int cancel;
    int tmp;
    int cycle;

    if (barrier->valid != BARRIER_VALID)
        return -100;

    status = pthread_mutex_lock(&barrier->mutex);
    assert(status == 0);

    printf("barrier cycle:%d\n", barrier->cycle);
    cycle = barrier->cycle;

    if (--barrier->counter == 0)
    {
        /*
         * all of threads has reach the barrier.
         * wake up the others.
         */
        barrier->cycle++; /* update the predicate, so that condition will be wake up*/
        barrier->counter = barrier->threshold; /* reset the counter using the threads number */
        status = pthread_cond_broadcast(&barrier->cv);
        
        // the last thread reaching the barrier will return -1
        if (status == 0)
            status = -1;
    }
    else
    {
        /*
         * wait with cancelling disabled, because barrier_wait should not be
         * a cancelling point.
         */
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel);
        
        printf("%u wait the other threads...\n", pthread_self());
        /*
         * 先到的线程会等待当前cycle完成，即最后进入barrier的线程会改变这个cycle谓词
         */
        while (cycle == barrier->cycle)
        {
            status = pthread_cond_wait(&barrier->cv, &barrier->mutex);
            if (status != 0)
                break;
        }
        printf("%u wait the other threads...done\n", pthread_self());

        pthread_setcancelstate(cancel, &tmp);
    }

    int status2 = pthread_mutex_unlock(&barrier->mutex);
    assert(status2 == 0);
    
    return status;
}
