#include "common_inc.h"

const size_t THREADS = 5;
int counter[THREADS];

typedef struct team_tag 
{
    int join_i;
    pthread_t worker[THREADS];
}team_t;

void* worker_routine(void* arg)
{
    int index = *(int*)arg;
    for (counter[index] = 0;  ; counter[index]++)
    {
        // current thread will be canceled when counter is multiple of 1000
        if ((counter[index] % 1000) == 0)
        {
            // printf("call testcancel, worker[%u], counter[%d]\n", pthread_self(), counter[index]);
            pthread_testcancel();
        }
    }
}

void cleanup(void* arg)
{
    team_t* team = (team_t*)arg;
    int count;
    int status;

    printf("thread_routine receive cancel from main thread\n");
    for (count = team->join_i; count < THREADS; count++)
    {
        status = pthread_cancel(team->worker[count]);
        assert(status == 0);

        // all of threads will free themselves resource automatic, instead of blocking the father thread
        status = pthread_detach(team->worker[count]);
        assert(status == 0);

        // if you want the follow statement has the right counter value(multiple of 1000)
        // please sleep a while, utile the counter value refresh.
        sleep(1);
        printf("cleanup: canceled %d, thread[%u], counter[%d]\n", count, team->worker[count], counter[count]);
    }
}

void* thread_routine(void* arg)
{
    team_t team;
    int count;
    void* result;
    int status;
    int index[THREADS];

    printf("create worker\n");
    for (count = 0; count < THREADS; count++)
    {
        index[count] = count;
        status = pthread_create(&team.worker[count], NULL, worker_routine, (void*)&index[count]);
        assert(status == 0);
    }

    printf("install cleanup handler\n");
    pthread_cleanup_push(cleanup, (void*)&team);

    for (team.join_i = 0; team.join_i < THREADS; team.join_i++)
    {
        // block current thread(thread_routine) to join his children.
        status = pthread_join(team.worker[team.join_i], &result);
        assert(status == 0);
        printf("[FATAL]never come to here!\n");
    }

    pthread_cleanup_pop(0);

    return NULL;
}

int main(int argc, char** argv)
{
    pthread_t thread_id;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    assert(status == 0);

    sleep(2);

    printf("main thread cancel...\n");
    status = pthread_cancel(thread_id);
    assert(status == 0);

    status = pthread_join(thread_id, NULL);
    assert(status == 0);

    return 0;
}
