#include "common_inc.h"

typedef struct tsd_tag
{
    pthread_t thread_id;
    char* str;
}tsd_t;

pthread_key_t tsd_key;
pthread_once_t key_once = PTHREAD_ONCE_INIT;

void once_routine(void)
{
    int status;

    printf("initializing key\n");
    status = pthread_key_create(&tsd_key, NULL);
    assert(status == 0);
}

void* thread_routine(void* arg)
{
    tsd_t* value;
    int status;

    // every thread has their own stack space
    // every thread has shared text segment
    printf("%s stack: status 0x%x, text: function 0x%x\n", arg, &status, thread_routine);
    status = pthread_once(&key_once, once_routine);
    assert(status == 0);

    // current source has the memory leak
    // see tsd_destructor.cpp to solve it.
    value = (tsd_t*)malloc(sizeof(tsd_t));
    assert(value != NULL);

    // every thread will has theirself memory space
    status = pthread_setspecific(tsd_key, value);
    assert(status == 0);

    printf("%s set tsd value 0x%x\n", arg, value);
    value->thread_id = pthread_self();
    value->str = (char*)arg;
    
    // every thread could get theirself value use the key initialized once
    value = (tsd_t*)pthread_getspecific(tsd_key);
    printf("%s value 0x%x starting...\n", value->str, value);
    sleep(2);
    value = (tsd_t*)pthread_getspecific(tsd_key);
    printf("%s value 0x%x done...\n", value->str, value);

    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread1;
    pthread_t thread2;
    int status;

    status = pthread_create(&thread1, NULL, thread_routine, (void*)"thread 1");
    assert(status == 0);
    status = pthread_create(&thread2, NULL, thread_routine, (void*)"thread 2");
    assert(status == 0);

    sleep(5);

    pthread_exit(NULL);
    return 0;
}
