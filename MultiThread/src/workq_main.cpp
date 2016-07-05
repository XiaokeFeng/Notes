#include "workq.h"

const size_t ITERATIONS = 25;

typedef struct power_tag
{
    int value;
    int power;
} power_t;

typedef struct engine_tag
{
    struct engine_tag* link;
    pthread_t thread_id;
    int calls;
} engine_t;

pthread_key_t engine_key;
pthread_mutex_t engine_list_mutex = PTHREAD_MUTEX_INITIALIZER;
engine_t* engine_list_head = NULL;
workq_t workq;

void destructor(void* value_ptr)
{
    engine_t* engine = (engine_t*)value_ptr;
    pthread_mutex_lock(&engine_list_mutex);
    engine->link = engine_list_head;
    engine_list_head = engine;
    pthread_mutex_unlock(&engine_list_mutex);
}

void engine_routine(void* arg)
{
    engine_t* engine;
    power_t* power = (power_t*)arg;
    int result;
    int count;
    int status;

    engine = (engine_t*)pthread_getspecific(engine_key);
    if (engine == NULL)
    {
        engine = (engine_t*)malloc(sizeof(engine_t));
        assert(engine != NULL);
        status = pthread_setspecific(engine_key, (void*)engine);
        assert(status == 0);
        engine->thread_id = pthread_self();
        engine->calls = 1;
    }
    else
        engine->calls++;

    result = 1;
    printf("Engine: computing %d^%d\n", power->value, power->power);
    for (count = 1; count <= power->power; count++)
        result += power->value;
    free(arg);
}

void* thread_routine(void* arg)
{
    power_t* element;
    int count;
    unsigned int seed = (unsigned int)time(NULL);
    int status;

    for (count = 0; count < ITERATIONS; count++)
    {
        element = (power_t*)malloc(sizeof(power_t));
        assert(element != NULL);

        element->value = rand_r(&seed) % 20;
        element->power = rand_r(&seed) % 7;
        printf("Request: %d^%d\n", element->value, element->power);
        status = workq_add(&workq, (void*)element);
        assert(status == 0);
        sleep(rand_r(&seed) % 5);
    }

    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread_id;
    engine_t* engine;
    int count = 0;
    int calls = 0;
    int status;

    status = pthread_key_create(&engine_key, destructor);
    assert(status == 0);

    status = workq_init(&workq, 4, engine_routine);
    assert(status == 0);

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    assert(status == 0);

    (void)thread_routine(NULL);
    
    status = pthread_join(thread_id, NULL);
    assert(status == 0);

    status = workq_destroy(&workq);
    assert(status == 0);

    engine = engine_list_head;
    while (engine != NULL)
    {
        count++;
        calls += engine->calls;
        printf("engine %d: %d calls\n", count, engine->calls);
        engine = engine->link;
    }
    printf("%d engine threads processed %d calls\n", count, calls);

    return 0;
}
