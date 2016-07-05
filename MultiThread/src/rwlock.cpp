#include "rwlock.h"

int rwl_init(rwlock_t* rwl)
{
    int status;

    rwl->r_active = rwl->w_active = rwl->r_wait = rwl->w_wait = 0;
    status = pthread_mutex_init(&rwl->mutex, NULL);
    assert(status == 0);

    status = pthread_cond_init(&rwl->read, NULL);
    assert(status == 0);

    status = pthread_cond_init(&rwl->write, NULL);
    assert(status == 0);

    rwl->valid = RWLOCK_VALID;

    return 0;
}

// return -1000: not valid
// return 1: busy
int rwl_destroy(rwlock_t* rwl)
{
    int status;
    int status1;
    int status2;

    if (rwl->valid != RWLOCK_VALID)
        return -1000;

    status = pthread_mutex_lock(&rwl->mutex);
    assert(status == 0);

    if (rwl->r_active > 0 || rwl->w_active)
    {
        pthread_mutex_unlock(&rwl->mutex);
        return 1;
    }

    if (rwl->r_wait > 0 || rwl->w_wait > 0)
    {
        pthread_mutex_unlock(&rwl->mutex);
        return 1;
    }

    rwl->valid = 0;
    status = pthread_mutex_unlock(&rwl->mutex);
    assert(status == 0);

    status = pthread_mutex_destroy(&rwl->mutex);
    status1 = pthread_cond_destroy(&rwl->read);
    status2 = pthread_cond_destroy(&rwl->write);
    return (status != 0 ? status :
            (status1 != 0 ? status1 : status2));
}

// installer function
static void rwl_readcleanup(void* arg)
{
    rwlock_t* rwl = (rwlock_t*)arg;
    rwl->r_wait--;
    pthread_mutex_unlock(&rwl->mutex);
}

static void rwl_writecleanup(void* arg)
{
    rwlock_t* rwl = (rwlock_t*)arg;
    rwl->w_wait--;
    pthread_mutex_unlock(&rwl->mutex);
}

int rwl_readlock(rwlock_t* rwl)
{
    int status = 0;

    if (rwl->valid != RWLOCK_VALID)
        return -1;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active)
    {
        rwl->r_wait++;
        // install the cleanup handler
        pthread_cleanup_push(rwl_readcleanup, (void*)rwl);
        while (rwl->w_active)
        {
            // wait the condition from the write unlock
            status = pthread_cond_wait(&rwl->read, &rwl->mutex);
            if (status != 0)
                // wait failed
                break;
        }
        // clear the cleanup handler
        pthread_cleanup_pop(0);
    }

    if (status == 0)
        rwl->r_active++;
    pthread_mutex_unlock(&rwl->mutex);

    return status;
}

// reutrn 0:ok
// return 1:busy
int rwl_readtrylock(rwlock_t* rwl)
{
    int status = 0;

    if (rwl->valid != RWLOCK_VALID)
        return -1;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active)
    {
        return 1;
    }
    else
    {
        rwl->r_active++;
    }
    pthread_mutex_unlock(&rwl->mutex);

    return status;
}

int rwl_readunlock(rwlock_t* rwl)
{
    int status;
    int status2;

    if (rwl->valid != RWLOCK_VALID)
        return -1;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    rwl->r_active--;
    if (rwl->r_active == 0 && rwl->w_wait > 0)
        // randomly, wake up one of the wait threads
        status = pthread_cond_signal(&rwl->write);
    status2 = pthread_mutex_unlock(&rwl->mutex);

    return (status2 == 0 ? status : status2);
}

int rwl_writelock(rwlock_t* rwl)
{
    int status;

    if (rwl->valid != RWLOCK_VALID)
        return -1;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active || rwl->r_active > 0)
    {
        rwl->w_wait++;
        pthread_cleanup_push(rwl_writecleanup, (void*)rwl);
        while (rwl->w_active || rwl->r_active > 0)
        {
            status = pthread_cond_wait(&rwl->write, &rwl->mutex);
            if (status != 0)
                break;
        }
        pthread_cleanup_pop(0);
        rwl->w_wait--;
    }

    if (status == 0)
        rwl->w_active = 1;
    pthread_mutex_unlock(&rwl->mutex);

    return status;
}

// return 0: ok
// reutrn 1: busy
int rwl_writetrylock(rwlock_t* rwl)
{
    int status;

    if (rwl->valid != RWLOCK_VALID)
        return -1;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active || rwl->r_active > 0)
    {
        return 1;
    }
    else
    {
        rwl->w_active = 1;
    }
    pthread_mutex_unlock(&rwl->mutex);

    return status;
}

int rwl_writeunlock(rwlock_t* rwl)
{
    int status;

    if (rwl->valid != RWLOCK_VALID)
        return -1;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;
    
    rwl->w_active = 0;
    if (rwl->r_wait > 0)
    {
        status = pthread_cond_broadcast(&rwl->read);
        if (status != 0)
            return pthread_mutex_unlock(&rwl->mutex);
    } 
    else if (rwl->w_wait > 0)
    {
        status = pthread_cond_signal(&rwl->write);
        if (status != 0)
            return pthread_mutex_unlock(&rwl->mutex);
    }

    return pthread_mutex_unlock(&rwl->mutex);
}
