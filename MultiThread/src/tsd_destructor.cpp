#include "common_inc.h"

typedef struct private_tag
{
    pthread_t thread_id;
    char* str;
} private_t;

pthread_key_t identity_key;
pthread_mutex_t identity_key_mutex = PTHREAD_MUTEX_INITIALIZER;
long identity_key_counter = 0;

// destructor handler
void identity_key_destructor(void* value)
{
    private_t* priv = (private_t*)value;
    int status;

    printf("%s exiting...\n", priv->str);
    free(value);

    status = pthread_mutex_lock(&identity_key_mutex);
    assert(status == 0);

    identity_key_counter--;
    printf("identity_key_counter %d\n", identity_key_counter);
    if (identity_key_counter <= 0)
    {
        status = pthread_key_delete(identity_key);
        assert(status == 0);
        printf("key deleted...\n");
    }

    status = pthread_mutex_unlock(&identity_key_mutex);
    assert(status == 0);
}

void* identity_key_get(void)
{
    void* value;
    int status;

    value = pthread_getspecific(identity_key);
    if (value == NULL)
    {
        value = malloc(sizeof(private_t));
        assert(value != NULL);

        status = pthread_setspecific(identity_key, (void*)value);
        assert(status == 0);

        status = pthread_mutex_lock(&identity_key_mutex);
        assert(status == 0);

        identity_key_counter++;
        
        status = pthread_mutex_unlock(&identity_key_mutex);
        assert(status == 0);
    }

    return value;
}

void* thread_routine(void* arg)
{
    private_t* value;

    value = (private_t*)identity_key_get();
    value->thread_id = pthread_self();
    value->str = (char*)arg;
    printf("%s : value 0x%x starting...\n", value->str, value);
    sleep(2);

    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread1;
    pthread_t thread2;
    private_t* value;
    int status;

    status = pthread_key_create(&identity_key, identity_key_destructor);
    assert(status == 0);
    
    value = (private_t*)identity_key_get();
    value->thread_id = pthread_self();
    value->str = (char*)"Main thread";

    status = pthread_create(&thread1, NULL, thread_routine, (void*)"Thread 1");
    assert(status == 0);
    status = pthread_create(&thread2, NULL, thread_routine, (void*)"Thread 2");
    assert(status == 0);

    sleep(5);
    
    pthread_exit(NULL);
    return 0;
}
