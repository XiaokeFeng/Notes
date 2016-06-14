#include <pthread.h>
#include <assert.h>
#include <stdio.h>

pthread_once_t once_block = PTHREAD_ONCE_INIT;
pthread_mutex_t mutex;

// 保证mutex只被初始化一次
void once_init_routine(void)
{
    int status;
    status = pthread_mutex_init(&mutex, NULL);
    assert(status == 0);
}

void* thread_routine(void* arg)
{
    int status;
    status = pthread_once(&once_block, once_init_routine);
    assert(status == 0);
    
    status = pthread_mutex_lock(&mutex);
    assert(status == 0);
    printf("%s, has locked the mutex\n", __FUNCTION__);
    
    status = pthread_mutex_unlock(&mutex);
    assert(status == 0);
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t thread_id;
    char* input;
    char buffer[64];
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    assert(status == 0);

    status = pthread_once(&once_lock, once_init_routing);
    assert(status == 0);

    status = pthread_mutex_lock(&mutex);
    assert(status == 0);
    printf("%s, has locked the mutex\n", __FUNCTION__);

    status = pthread_mutex_unlock(&mutex);
    assert(status == 0);

    return 0;
}
