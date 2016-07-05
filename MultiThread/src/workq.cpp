#include "workq.h"

int workq_init(workq_t* wq, int threads, void (*engine)(void* arg))
{
    int status;

    status = pthread_attr_init(&wq->attr);
    assert(status == 0);

    status = pthread_attr_setdetachstate(&wq->attr, PTHREAD_CREATE_DETACHED);
    if (status != 0)
    {
        pthread_attr_destroy(&wq->attr);
        return status;
    }

    status = pthread_mutex_init(&wq->mutex, NULL);
    if (status != 0)
    {
        pthread_attr_destroy(&wq->attr);
        return status;
    }

    status = pthread_cond_init(&wq->cv, NULL);
    if (status != 0)
    {
        pthread_attr_destroy(&wq->attr);
        pthread_mutex_destroy(&wq->mutex);
        return status;
    }

    wq->quit = 0;
    wq->first = wq->last = NULL;
    wq->parallelism = threads;
    wq->counter = 0;
    wq->engine = engine;
    wq->valid = WORKQ_VALID;

    return 0;
}

int workq_destroy(workq_t* wq)
{
    int status;
    int status1;
    int status2;

    if (wq->valid != WORKQ_VALID)
        return -1;

    status = pthread_mutex_lock(&wq->mutex);
    if (status != 0)
        return status;

    wq->valid = 0;

    if (wq->counter > 0)
    {
        // set quit flag
        wq->quit = 1;
        // wake up all of the idle threads
        if (wq->idle > 0)
        {
            status = pthread_cond_broadcast(&wq->cv);
            if (status != 0)
            {
                pthread_mutex_unlock(&wq->mutex);
                return status;
            }
        }

        while (wq->counter > 0)
        {
            status = pthread_cond_wait(&wq->cv, &wq->mutex);
            if (status != 0)
            {
                pthread_mutex_unlock(&wq->mutex);
                return status;
            }
        }
    }

    status = pthread_mutex_unlock(&wq->mutex);
    if (status != 0)
        return status;

    status = pthread_mutex_destroy(&wq->mutex);
    status1 = pthread_cond_destroy(&wq->cv);
    status2 = pthread_attr_destroy(&wq->attr);

    return (status ? status : 
                (status1 ? status1 : status2));
}

int workq_add(workq_t* wq, void* element)
{
    workq_ele_t* item;
    pthread_t id;
    int status;

    if (wq->valid != WORKQ_VALID)
        return -1;

    item = (workq_ele_t*)malloc(sizeof(workq_ele_t));
    if (item == NULL)
        return -2;

    item->data = element;
    item->next = NULL;

    status = pthread_mutex_lock(&wq->mutex);
    if (status != 0)
    {
        free(item);
        return status;
    }

    if (wq->first == NULL)
        wq->first = item;
    else
        wq->last->next = item;

    wq->last = item;

    if (wq->idle > 0)
    {
        status = pthread_cond_signal(&wq->cv);
        if (status != 0)
        {
            pthread_mutex_unlock(&wq->mutex);
            return status;
        }
    }
    else if (wq->counter < wq->parallelism)
    {
        printf("creating new worker\n");
        status = pthread_create(&id, &wq->attr, workq_server, (void*)wq);
        if (status != 0)
        {
            pthread_mutex_unlock(&wq->mutex);
            return status;
        }
        wq->counter++;
    }

    pthread_mutex_unlock(&wq->mutex);

    return 0;
}

static void* workq_server(void* arg)
{
    struct timespec timeout;
    workq_t* wq = (workq_t*)arg;
    workq_ele_t* we;
    int status;
    int timedout;

    printf("a worker is starting\n");
    /*
     * workq_add中已经先加锁了，这里再lock的时候，会等到workq_add的create完毕才会返回成功
     */
    status = pthread_mutex_lock(&wq->mutex);
    if (status != 0)
        return NULL;

    while (1)
    {
        timedout = 0;
        printf("worker waiting for work\n");
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 2;

        while (wq->first == NULL && !wq->quit)
        {
            status = pthread_cond_timedwait(&wq->cv, &wq->mutex, &timeout);
            if (status == ETIMEDOUT)
            {
                printf("worker wait timed out\n");
                timedout = 1;
                break;
            }
            else if (status != 0)
            {
                // couldn't reach here
                printf("worker wait failed\n");
                wq->counter--;
                pthread_mutex_unlock(&wq->mutex);
                return NULL;
            }
        }

        printf("work queue: 0x%p, quit: %d\n", wq->first, wq->quit);
        we = wq->first;

        if (we != NULL)
        {
            wq->first = we->next;
            if (wq->last == we)
                wq->last = NULL;
            status = pthread_mutex_unlock(&wq->mutex);
            if (status != 0)
                return NULL;

            printf("worker calling engine\n");
            wq->engine(we->data);
            free(we);
            
            status = pthread_mutex_lock(&wq->mutex);
            if (status != 0)
                return NULL;
        }

        if (we == NULL && wq->quit)
        {
            printf("worker shutting down\n");
            wq->counter--;

            if (wq->counter == 0)
                // wake up the destroy
                pthread_cond_broadcast(&wq->cv);
            pthread_mutex_unlock(&wq->mutex);
            return NULL;
        }

        if (we == NULL && timedout)
        {
            printf("engine terminating due to timedout\n");
            wq->counter--;
            break;
        }
    }

    pthread_mutex_unlock(&wq->mutex);
    printf("working exiting\n");
    return NULL;
}
