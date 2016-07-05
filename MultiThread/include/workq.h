#include "common_inc.h"

//keep track of work queue requests
typedef struct workq_ele_tag
{
    struct workq_ele_tag* next;
    void* data;
} workq_ele_t;

typedef struct workq_tag
{
    pthread_mutex_t     mutex;
    pthread_cond_t      cv;
    pthread_attr_t      attr;
    workq_ele_t*        first;
    workq_ele_t*        last;
    int                 valid;
    int                 quit;
    int                 parallelism;            /* maximum threads */
    int                 counter;                /* current threads */
    int                 idle;                   /* number of idle threads */
    void                (*engine)(void* arg);    /* user engine */
} workq_t;

#define WORKQ_VALID 0xdec1993

int workq_init(
    workq_t*    wq,
    int         threads,
    void        (*engine)(void*));
int workq_destroy(workq_t* wq);
int workq_add(workq_t* wq, void* data);
static void* workq_server(void* arg);
