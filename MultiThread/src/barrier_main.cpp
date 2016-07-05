#include "barrier.h"

const size_t THREADS = 5;
const size_t ARRAY = 6;
const size_t INLOOPS = 100000;
const size_t OUTLOOPS = 10;

typedef struct thread_tag
{
    pthread_t thread_id;
    int number;
    int increment;
    int array[ARRAY];
} thread_t;

barrier_t barrier;
thread_t thread[THREADS];

void* thread_routine(void* arg)
{
    thread_t* self = (thread_t*)arg;
    int in_loop;
    int out_loop;
    int count;
    int status;

    for (out_loop = 0; out_loop < OUTLOOPS; out_loop++)
    {
        status = barrier_wait(&barrier);
        if (status == -1)
            printf("the last thread %u wait on barrier, go to compute\n", pthread_self());

        for (in_loop = 0; in_loop < INLOOPS; in_loop++)
        {
            for (count = 0; count < ARRAY; count++)
                self->array[count] += self->increment;
        }

        status = barrier_wait(&barrier);
        if (status == -1)
            printf("the last thread %u wait on barrier done\n", pthread_self());

        if (status == -1)
        {
            int thread_num;
            for (thread_num = 0; thread_num < THREADS; thread_num++)
                thread[thread_num].increment += 1;
        }
    }

    return NULL;
}

int main(int argc, char** argv)
{
    int thread_count;
    int array_count;
    int status;

    barrier_init(&barrier, THREADS);

    for (thread_count = 0; thread_count < THREADS; thread_count++)
    {
        thread[thread_count].increment = thread_count;
        thread[thread_count].number = thread_count;

        for (array_count = 0; array_count < ARRAY; array_count++)
            thread[thread_count].array[array_count] = array_count + 1;
        
        status = pthread_create(&thread[thread_count].thread_id,
            NULL, thread_routine, (void*)&thread[thread_count]);
        assert(status == 0);
    }
    
    for (thread_count = 0; thread_count < THREADS; thread_count++)
    {
        status = pthread_join(thread[thread_count].thread_id, NULL);
        assert(status == 0);

        printf("%02d: (%d)", thread_count, thread[thread_count].increment);
        for (array_count = 0; array_count < ARRAY; array_count++)
            printf("%010u ", thread[thread_count].array[array_count]);
        printf("\n");
    }

    barrier_destroy(&barrier);

    return 0;
}
