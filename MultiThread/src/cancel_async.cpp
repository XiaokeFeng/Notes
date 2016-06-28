#include "common_inc.h"
#include "common_tools.h"

const size_t SIZE = 10;

static int matrixa[SIZE][SIZE];
static int matrixb[SIZE][SIZE];
static int matrixc[SIZE][SIZE];

void* thread_routine(void* arg)
{
    int cancel_type;
    int status;
    
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            matrixa[i][j] = i;
            matrixb[i][j] = j;
        }

        while (1)
        {
            // save current cancel type into cancel_type
            // & set to async
            status = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &cancel_type);
            assert(status == 0);
            struct timeval tv = multithread::Timer::get_and_set_start_time();
            printf("thread_routine set cancel type to async, [%d]s, [%d]us\n", tv.tv_sec, tv.tv_usec);

            for (i = 0; i < SIZE; i++)
            {
                for (int j = 0; j < SIZE; j++)
                {
                    matrixc[i][j] = 0;
                    for (int k = 0; k < SIZE; k++)
                    {
                        matrixc[i][j] += matrixa[i][k] * matrixb[k][j];
                    }
                }
            }

            sleep(1);

            // bring back to saved type
            status = pthread_setcanceltype(cancel_type, &cancel_type);
            assert(status == 0 && cancel_type == PTHREAD_CANCEL_ASYNCHRONOUS);
            int cost_time = multithread::Timer::get_cost_time();
            printf("thread_routine set cancel type to origin, async cost time [%d]ms\n", cost_time);

            for (i = 0; i < SIZE; i++)
            {
                for (int j = 0; j < SIZE; j++)
                {
                    matrixa[i][j] = matrixc[i][j];
                }
            }
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

    sleep(5);

    // this call will return right now
    status = pthread_cancel(thread_id);
    struct timeval tv = multithread::Timer::get_cur_time();
    printf("main thread call cancel, [%d]s, [%d]us\n", tv.tv_sec, tv.tv_usec);
    assert(status == 0);

    status = pthread_join(thread_id, &result);
    assert(status == 0);

    tv = multithread::Timer::get_cur_time();
    if (result == PTHREAD_CANCELED)
        printf("Thread canceled, [%d]s [%d]us\n", tv.tv_sec, tv.tv_usec);
    else
        printf("Thread was not canceled\n");
    return 0;
}
