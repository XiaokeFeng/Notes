#include "common_inc.h"

const size_t THREADS = 5;

typedef struct thread_tag
{
    int index;
    pthread_t id;
} thread_t;

thread_t threads[THREADS];
int rr_min_priority;

void* thread_routine(void* arg)
{
    thread_t* self = (thread_t*)arg;
    int my_policy;
    struct sched_param my_param;
    int status;

    my_param.sched_priority = rr_min_priority + 50 - 10 * (self->index + 1);
    printf("%u: will set SCHED_RR, priority %d\n", pthread_self(), my_param.sched_priority);
    
    status = pthread_setschedparam(self->id, SCHED_RR, &my_param);
    if (status != 0)
    {
        printf("Permssion denied, %u exiting. use 'sudo' again\n", pthread_self());
        return NULL;
    }

    status = pthread_getschedparam(self->id, &my_policy, &my_param);
    assert(status == 0);
    printf("%u: running at %s/%d\n",
        pthread_self(),
        (my_policy == SCHED_RR ? 
            "RR" :
            (my_policy == SCHED_FIFO ?
                "FIFO" :
                (my_policy == SCHED_OTHER ? 
                    "OTHER" :
                    "unknown"))),
        my_param.sched_priority);

    // the higher priority, the earlier wake up, the faster compute.
    sleep(1);
    int j = 0;
    while (j < 100)
    {
        for (int i = 0; i < 1000 * 1000; i++)
        {
            ;
        }
        j++;
    }
    return NULL;
}

int main(int argc, char** argv)
{
    int count;
    int status;

    rr_min_priority = sched_get_priority_min(SCHED_RR);
    assert(rr_min_priority != -1);
    printf("RR min priority:%d\n", rr_min_priority);

    for (count = 0; count < THREADS; count++)
    {
        threads[count].index = count;
        status = pthread_create(&threads[count].id, NULL, thread_routine, (void*)&threads[count]);
        assert(status == 0);
    }

    for (count == 0; count < THREADS; count++)
    {
        status = pthread_join(threads[count].id, NULL);
        assert(status == 0);
    }
    
    // you must let main thread sleep some minutes
    // if you want the child threads will run compeletly before the main thread exit.
    sleep(5);
    printf("Main exiting\n");

    return 0;
}
