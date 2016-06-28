#include "common_inc.h"
typedef struct alarm_tag
{
    struct alarm_tag* link;
    int seconds;
    time_t time;
    char message[64];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t alarm_cond = PTHREAD_COND_INITIALIZER;
alarm_t* alarm_list = NULL;
time_t current_alarm = 0;

void alarm_insert(alarm_t* alarm)
{
    int status;
    alarm_t** last;
    alarm_t* next;

    last = &alarm_list;
    next = *last;
    while (next != NULL)
    {
        if (next->time >= alarm->time)
        {
            alarm->link = next;
            *last = alarm;
            break;
        }
        last = &next->link;
        next = next->link;
    }

    // insert into the end of the linklist
    if (next == NULL)
    {
        *last = alarm;
        alarm->link = NULL;
    }
    printf("[list: ");
    for (next = alarm_list; next != NULL; next = next->link)
    {
        printf("%d(%d)[\"%s\"] ", next->time, next->time - time(NULL), next->message);
    }
    printf("]\n");

    // wake up the thread
    if (current_alarm == 0 || alarm->time < current_alarm)
    {
        current_alarm = alarm->time;
        status = pthread_cond_signal(&alarm_cond);
        assert(status == 0);
    }
}

void* alarm_thread(void* arg)
{
    alarm_t* alarm;
    struct timespec cond_time;
    time_t now;
    int status;
    int expired;

    status = pthread_mutex_lock(&alarm_mutex);
    assert(status == 0);
    while (1)
    {
        current_alarm = 0;
        while (alarm_list == NULL)
        {
            // pthread_cond_wait 相当于 unlock && lock
            status = pthread_cond_wait(&alarm_cond, &alarm_mutex);
            assert(status == 0);
        }
        
        alarm = alarm_list;
        alarm_list = alarm->link;
        now = time(NULL);
        expired = 0;
        if (alarm->time > now)
        {
            printf("[waiting: %d(%d)\"%s\"]\n", alarm->time, alarm->time - time(NULL), alarm->message);
            cond_time.tv_sec = alarm->time;
            cond_time.tv_nsec = 0;
            current_alarm = alarm->time;
            while (current_alarm == alarm->time)
            {
                status = pthread_cond_timedwait(&alarm_cond, &alarm_mutex, &cond_time);
                if (status == ETIMEDOUT)
                {
                    expired = 1;
                    break;
                }
                assert(status == 0);
            }
            if (!expired)
            {
                alarm_insert(alarm);
            }
        }
        else
        {
            expired = 1;
        }

        if (expired)
        {
            printf("(%d) %s\n", alarm->seconds, alarm->message);
            free(alarm);
        }
    }
}

int main(int argc, char** argv)
{
    int status;
    char line[128];
    alarm_t* alarm;
    pthread_t thread;

    status = pthread_create(&thread, NULL, alarm_thread, NULL);
    assert(status == 0);

    while (1)
    {
        printf("Alarm> ");
        if (fgets(line, sizeof(line), stdin) == NULL) exit(0);
        if (strlen(line) <= 1) continue;
        alarm = (alarm_t*)malloc(sizeof(alarm_t));
        assert(alarm != NULL);

        if (sscanf(line, "%d %64[^\n]", &alarm->seconds, alarm->message) < 2)
        {
            fprintf(stderr, "bad command\n");
            free(alarm);
        }
        else
        {
            status = pthread_mutex_lock(&alarm_mutex);
            assert(status == 0);
            alarm->time = time(NULL) + alarm->seconds;
            alarm_insert(alarm);
            status = pthread_mutex_unlock(&alarm_mutex);
            assert(status == 0);
        }
    }
}
