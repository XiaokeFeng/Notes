# High Level Of Thread

## Overview
* [一次性初始化](#ch1)
* [属性](#ch2)
* [取消](#ch3)
    * [推迟取消](#ch3.1)

<h2 id="ch1">一次性初始化</h2>

类似于`static`的作用，`pthread_once_t`是由互斥量保护的共享不变量

```
    pthread_once_t once_control = PTHREAD_ONCE_INIT;
    int phtread_once (pthread_once_t* once_control, void (*init_routine)(void));
```

<h2 id="ch2">属性</h2>

### 互斥量属性
```
    pthread_mutexattr_t attr;
    int pthread_mutexattr_init (pthread_mutexattr_t* attr);
    int pthread_mutexattr_destroy (pthread_mutexattr_t* attr);
    #ifdef _POSIX_THREAD_PROCESS_SHARED
    int pthread_mutexattr_getpshared(pthread_mutexattr_t* attr, int* pshared);
    int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared);
    #endif
```

### 条件变量属性
```
    pthread_condattr_t attr;
    int pthread_condattr_init (pthread_condattr_t* attr);
    int pthread_condattr_destroy (pthread_condattr_t* attr);
    #ifdef _POSIX_THREAD_PROCESS_SHARED
    int pthread_condattr_getpshared(pthread_condattr_t* attr, int* pshared);
    int pthread_condattr_setpshared(pthread_condattr_t* attr, int pshared);
    #endif
```

### 线程属性
```
    pthread_attr_t attr;
    int pthread_attr_init(pthread_attr_t* attr);
    int pthread_attr_destroy(pthread_attr_t* attr);
    int pthread_attr_getdetachstate(pthread_attr_t* attr, int* detachstate);
    int pthread_attr_setdetachstate(pthread_attr_t* attr, int* detachstate);
    #ifdef _POSIX_THREAD_ATTR_STACKSIZE
    int pthread_attr_getstacksize(pthread_attr_t* attr, size_t* stacksize);
    int pthread_attr_setstacksize(pthread_attr_t* attr, size_t* stacksize);
    #endif
    #ifdef _POSIX_THREAD_ATTR_STACKADDR
    int pthread_attr_getstackaddr(pthread_attr_t* attr, void* stackaddr);
    int pthread_attr_setstackaddr(pthread_attr_t* attr, void* stackaddr);
    #endif
```

<h2 id="ch3">取消</h2>

* 用途：多线程通常被用于一些启发式的并行“探索”一个数据集，例如寻找最大解、全局最优等。

```
    int pthread_cancel(pthread_t thread);
    int pthread_setcancelstate(int state, int* oldstate);
    int pthread_setcanceltype(int type, int* oldtype);
    void pthread_testcancel(void);
    void pthread_cleanup_push(void (*routine)(void*), void* arg);
    void pthread_cleanup_pop(int execute);
```

<h3 id="ch3.1">推迟取消</h3>

* 取消类型：`PTHREAD_CANCEL_DEFERRED`
* 线程仅仅在达到取消点时才响应取消请求。
* 如果没有取消是当前未解决的，那么函数将继续执行。如果线程正在等待一些东西（I/O）时，另外的线程请求取消该线程，那么等待将被打断，并且，线程将开始它的取消清除。
