# High Level Of Thread

## Overview
* [一次性初始化](#ch1)
* [属性](#ch2)
* [取消](#ch3)
    * [推迟取消](#ch3.1)
    * [异步取消](#ch3.2)
    * [清除](#ch3.3)
* [线程私有数据](#ch4)

<h2 id="ch1">一次性初始化</h2>

示例代码[once.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/once.cpp)

类似于`static`的作用，`pthread_once_t`是由互斥量保护的共享不变量

```
    pthread_once_t once_control = PTHREAD_ONCE_INIT;
    int phtread_once (pthread_once_t* once_control, void (*init_routine)(void));
```

<h2 id="ch2">属性</h2>

示例代码TODO

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

示例代码[cancle.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/cancel.cpp)

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

示例代码[cancel\_disable.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/cancel_disable.cpp)

* 取消类型：`PTHREAD_CANCEL_DEFERRED`
* 线程仅仅在达到取消点时才响应取消请求。
* 如果没有取消是当前未解决的，那么函数将继续执行。如果线程正在等待一些东西（I/O）时，另外的线程请求取消该线程，那么等待将被打断，并且，线程将开始它的取消清除。
* 一般是将cancel state设置为PTHREAD\_CANCEL\_DISABLE，此时将不会响应其他thread传来的cancel请求（maybe block or wait the calling thread），等到sleep返回后，可以重新设置为之前的state，来处理cancel请求

<h3 id="ch3.2">异步取消</h3>

示例代码[cancel\_async.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/cancel_async.cpp)

* 不需要通过取消检查点来查询取消请求，在计算密集型的并发计算中，可以缓解`pthread_testcancel`带来的开销
* 安全性：使用异步取消的时候，不允许调用任何获得资源（例如malloc、加解锁）的函数
    * 异步取消安全函数：
        ```
            pthread_cancel
            pthread_setcancelstate
            pthread_setcanceltype
        ```
    * 除上述，没有其他POSIX或ANSIC函数是异步取消安全的
* 设置cancel type为`PTHREAD_CANCEL_ASYNCHRONOUS`即可允许当前thread可以被异步取消

<h3 id="ch3.3">清除</h3>

示例代码[cancel\_cleanup.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/cancel_cleanup.cpp)
示例代码[cancel\_subcontract.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/cancel_subcontract.cpp)

* 何时需要清除函数
    * 当thread在等待一个条件变量时被取消，它将被唤醒，并需要保持mutex的加锁状态
    * 当thread终止前，通常需要恢复不变量，并且释放mutex
* 如何使用清除函数
    * `pthread_cleanup_push`将清除函数压入thread自己的栈
    * `pthread_cleanup_pop`将最近增加的清除函数删除，调用清除函数后，是不是执行pop操作的，会直接exit
* 何时调用清除函数
    * 当thread被取消时
    * 当thread调用`pthread_exit`时
* 清除函数就是在thread结束之前作一些自身状态的还原、保存操作的

<h2 id="ch4">线程私有数据</h2>

示例代码[tsd\_once.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/tsd_once.cpp)
示例代码[tsd\_destructor.cpp](https://github.com/XiaokeFeng/notes/blob/master/MultiThread/src/tsd_destructor.cpp)

线程拥有自己的栈，共享其他地址（代码段、数据段、堆等等）

```
    pthread_key_t key;
    int pthread_key_create(pthread_key_t* key, void(*destructor)(void*));
    int pthread_key_delete(pthread_key_t key);
    int pthread_setspecific(pthread_key_t key, const void* value);
    void pthread_getspecific(pthread_key_t key);
```

key必须保证一个进程内唯一，线程内对同一个key进行value的set操作，不同的线程通过get来获取自己set的value
destructor函数的参数来自于线程私有数据value
