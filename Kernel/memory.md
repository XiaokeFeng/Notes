# Overview

* [内存组织](#ch1)
* [页表](#ch2)
* [初始化内存管理](#ch3)
* [物理内存管理](#ch4)
* [slab分配器](#ch5)
* [处理器高速缓存和TLB控制]($ch6)

内存管理涉及如下领域：
* 内存中的物理内存页的管理
* 分配大块内存的伙伴系统
* 分配较小快内存的slab、slub和slob分配器
* 分配非连续内存块的vmalloc机制
* 进程的地址空间

首先区分两种计算机类型：
* UMA（一致内存访问，uniform memory access）将可用内存以连续的方式组织起来，每个CPU共享同一个内存
* NUMA（非一致内存访问，non-uniform memory access）总是多处理器计算机，每个CPU独占一个内存，各个内存用总线相连


<h2 id="ch1">内存组织</h2>

<h3 id="ch1.1">NUMA中的基本概念</h3>

* 结点，内存划分的基本单位，一个结点连接一个CPU，`pg_data_t`
* 内存域，结点划分的基本单位，`zone_type`
    * ZONE_DMA
    * ZONE_NORMAL
    * ZONE_HIGHMEM
    * ZONE_MOVABLE

<h3 id="ch1.2">数据结构</h3>

<h4 id="ch1.2.1">结点管理</h4>

<mmzone.h>
`pg_data_t`为结点管理的数据结构，提供结点状态、页帧寻址功能，主要包括各zone的指针，所有page实例的指针，本结点的交换守护进程等

<h4 id="ch1.2.2">内存域</h4>

<mmzone.h>
`zone`为内存域的数据结构，主要为域内页管理，何时写入disk、何时收回页、冷热页管理等
