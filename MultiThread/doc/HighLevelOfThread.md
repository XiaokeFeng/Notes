# High Level Of Thread

## Overview
* [一次性初始化](#ch1)

<h2 id="ch1">一次性初始化</h2>

类似于`static`的作用，`pthread_once_t`是由互斥量保护的共享不变量

```
    pthread_once_t once_control = PTHREAD_ONCE_INIT;
    int phtread_once (pthread_once_t* once_control, void (*init_routine)(void));
```
