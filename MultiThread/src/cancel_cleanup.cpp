#include "common_inc.h"

const size_t THREADS = 5;

typedef struct control_tag
{
    int counter;
    int busy; // busy always is not 0.
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} control_t;

control_t control = 
    {0, 1, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

// this function will be called when the thread is canceled by other(eg. main) thread using cancel function.
void cleanup_handler(void* arg)
{
    control_t* st = (control_t*)arg;
    int status;
    st->counter--;
    printf("cleanup_handler: conter == %d\n", st->counter);

    status = pthread_mutex_unlock(&st->mutex);
    assert(status == 0);
}

void* thread_routine(void* arg)
{
    int status;
    // push/install the cleanup handler into the thread's stack
    pthread_cleanup_push(cleanup_handler, (void*)&control);
    printf("[%u]install the cleanup handler\n", pthread_self());

    status = pthread_mutex_lock(&control.mutex);
    assert(status == 0);

    control.counter++;

    while (control.busy)
    {
        printf("[%u]condition wait...\n", pthread_self());
        status = pthread_cond_wait(&control.cv, &control.mutex);
        assert(status == 0);
    }

    // pop/uninstall the cleanup handler from the thread's stack
    printf("ready to pop cleanup\n");
    pthread_cleanup_pop(1);
    printf("finish pop cleanup\n");

    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread_id[THREADS];
    int count;
    void* result;
    int status;

    for (count = 0; count < THREADS; count++)
    {
        status = pthread_create(&thread_id[count], NULL, thread_routine, NULL);
        assert(status == 0);
    }

    sleep(2);

    for (count = 0; count < THREADS; count++)
    {
        printf("main thread call thread[%u] to cancel\n", thread_id[count]);
        // child thread will call hiself cleanup handler
        status = pthread_cancel(thread_id[count]);
        assert(status == 0);

        status = pthread_join(thread_id[count], &result);
        assert(status == 0);

        if (result == PTHREAD_CANCELED)
            printf("thread[%u] canceled\n", thread_id[count]);
        else
            printf("thread[%u] was not canceled\n", thread_id[count]);
    }

    return 0;
}
