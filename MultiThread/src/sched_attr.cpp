#include "common_inc.h"

void* thread_routine(void* arg)
{
    int my_policy;
    struct sched_param my_param;
    int status;

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
    // get myself policy and sched param
    status = pthread_getschedparam(pthread_self(), &my_policy, &my_param);
    assert(status == 0);
    printf("thread_routine running at %s/%d\n",
        (my_policy == SCHED_FIFO ?
            "FIFO" :
            (my_policy == SCHED_RR ?
                "RR" :
                (my_policy == SCHED_OTHER ?
                    "OTHER":
                    "unknown"))),
        my_param.sched_priority);
#else
    printf("thread_routine running\n");
#endif
    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread_id;
    pthread_attr_t thread_attr;
    int thread_policy;
    struct sched_param thread_param;
    int status;
    int rr_min_priority;
    int rr_max_priority;

    status = pthread_attr_init(&thread_attr);
    assert(status == 0);

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
    status = pthread_attr_getschedpolicy(&thread_attr, &thread_policy);
    assert(status == 0);

    status = pthread_attr_getschedparam(&thread_attr, &thread_param);
    assert(status == 0);

    printf("defualt policy is %s, priority is %d\n",
        (thread_policy == SCHED_FIFO ?
            "FIFO" :
            (thread_policy == SCHED_RR ?
                "RR" :
                (thread_policy == SCHED_OTHER ?
                    "OTHER" :
                    "unknown"))),
        thread_param.sched_priority);

    status = pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    assert(status == 0);
        
    rr_min_priority = sched_get_priority_min(SCHED_RR);
    assert(rr_min_priority != -1);

    rr_max_priority = sched_get_priority_max(SCHED_RR);
    assert(rr_max_priority != -1);

    thread_param.sched_priority = (rr_min_priority + rr_max_priority) / 2;
    printf("using priority %d\n", thread_param.sched_priority);

    status = pthread_attr_setschedparam(&thread_attr, &thread_param);
    assert(status == 0);
        
    // you should set PTHREAD_EXPLICIT_SCHED
    // when you change the thread's sched_param or policy 
    // in pthread_attr_t
    status = pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    assert(status == 0);
#else
    printf("priority scheduling not supported\n");
#endif
    status = pthread_create(&thread_id, &thread_attr, thread_routine, NULL);
    if (status != 0)
    {
        printf("Permission denied, use: $sudo %s ...\n", argv[0]);
        return -1;
    }

    status = pthread_join(thread_id, NULL);
    assert(status == 0);

    sleep(5);
    printf("Main thread exiting\n");

    return 0;
}
