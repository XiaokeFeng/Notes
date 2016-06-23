#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
static int counter;

void* thread_routine(void* arg)
{
    int state;
    int status;

    for (counter = 0; ; counter++)
    {
        printf("counter %d\n", counter);
        if (counter && 0 == (counter % 95))
        {
            // disable the cancel
            status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
            assert(status == 0);

            // sleep 5 seconds.
            // delay 5 seconds for cancel.
            struct timeval start;
            struct timeval end;
            gettimeofday(&start, NULL);
            printf("disable the cancel at time[%ds %lus]\n", start.tv_sec, start.tv_usec);
            sleep(5);
            gettimeofday(&end, NULL);
            printf("restore the cancel at time[%ds %lus]\n", end.tv_sec, end.tv_usec);
            int cost = (int)((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000);
            printf("counter %d, sleep time %dms\n", counter, cost);
            
            // restore the cancel
            status = pthread_setcancelstate(state, &state);
            assert(status == 0);
        }
        else if (0 == (counter % 1000))
        {
            pthread_testcancel();
        }
    }
}

int main(int argc, char** argv)
{
    pthread_t thread_id;
    void* result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    assert(status == 0);
    sleep(2);

    struct timeval tv1;
    gettimeofday(&tv1, NULL);
    printf("calling cancel at time[%ds %lus]\n", tv1.tv_sec, tv1.tv_usec);
    status = pthread_cancel(thread_id);
    assert(status == 0);

    struct timeval tv2;
    gettimeofday(&tv2, NULL);
    printf("calling join at time[%ds %lus]\n", tv2.tv_sec, tv2.tv_usec);
    status = pthread_join(thread_id, &result);
    assert(status == 0);
    if (result == PTHREAD_CANCELED)
        printf("thread canceled at iteration %d\n", counter);
    else
        printf("thread was not canceled\n");

    return 0;
}
