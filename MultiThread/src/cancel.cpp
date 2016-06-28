#include "common_inc.h"

static int counter;

void* thread_routine(void* arg)
{
    printf("thread_routine starting...\n");
    for (counter = 0; ; counter++)
    {
        // counter must be multiple of 1000 when current thread is canceled by main thread using cancel function
        if (0 == (counter % 1000))
        {
            printf("calling testcancel\n");   
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
    sleep(1);

    printf("calling cancel\n");
    status = pthread_cancel(thread_id);
    assert(status == 0);

    printf("calling join\n");
    status = pthread_join(thread_id, &result);
    assert(status == 0);
    if (result == PTHREAD_CANCELED)
        printf("thread canceled at iteration %d\n", counter);
    else
        printf("thread was not canceled\n");

    return 0;
}
