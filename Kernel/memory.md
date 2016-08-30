# Overview
book: [Understand Linux Memory Management](https://www.kernel.org/doc/gorman/html/understand/)
source code: [Linux Cross Refrence](http://lxr.free-electrons.com/)

> Only for some notes.

基础部分：
* [Describing Physical Memory](#ch1)
    * [Nodes](#ch1.1)
    * [Zones](#ch1.2)
    * [Zone Initialisation](#ch1.3)
    * [Pages](#ch1.4)
    * [High Memory](#ch1.5)
* [Page Table Manager](#ch2)
* [Process Address Space](#ch3)
适用于各类case的内存管理：
* [Boot Memory Allocator](#ch4)
* [Physical Page Allocation](#ch5)
* [Non-Contiguous Memory Allocation](#ch6)
* [Slab Allocator](#ch7)
* [High Memory Management](#ch8)
* [Page Frame Reclamation](#ch9)
* [Swap Management](#ch10)
* [Shared Memory Virtual Filesystem](#ch11)
* [Out Of Memory Management](#ch12)


<h2 id="ch1">Describing Physical Memory</h2>

两种内存模型：
* NUMA(Non-Uniform Memory Access), CPU0->MEM0, CPU1->MEM1
* UMA(Uniform Memory Access), CPUs->MEM

内存组织：
* Nodes, MEM0 + MEM1, `struct pglist_data`
* Zones, ZONE_DMA + ZONE_NORMAL + ZONE_HIGHMEM, `struct zone`
* Pages, Physical page frame, `struct page`, all pages are kept in a global mem_map array

关系图：
![Describing Physical Memory](https://raw.githubusercontent.com/XiaokeFeng/notes/master/picture/DescribingPhysicalMemory.png)

<h3 id="ch1.1">Nodes</h3>

header: <linux/mmzone.h>

```
    typedef struct pglist_data {
        /*
        * 分别指向ZONE_DMA, ZONE_NORMAL, ZONE_HIGHMEM
        */
        struct zone node_zones[MAX_NR_ZONES];
        /*
        * 关联其他CPU的ZONE_{TYPE}，node_zones的备用结点
        */
        struct zonelist node_zonelists[MAX_ZONELISTS];
        /*
        * [1, 3]，不总是MAX_NR_ZONES，有些CPU内存块可能不会有ZONE_DMA或其他的
        */
        int nr_zones;
        /*
        * all Physical frame in current node.
        */
        struct page *node_mem_map;
        /*
        * Boot Memory Allocator
        */
        struct bootmem_data *bdata;
        unsigned long node_start_pfn; // 全局唯一的当前node的第一个页逻辑编号
        unsigned long node_present_pages; // 当前node的全部页数目
        unsigned long node_spanned_pages; // total numbers, including holes
        int node_id; // node ID(NID)
        // coming soon ...
        wait_queue_head_t kswapd_wait;
        struct task_struct *kswapd;
        int kswapd_max_order;
    } pg_data_t;
```

每个node由`pglist_data`维护，由`init_bootmem_core()`初始化，用宏`for_each_online_pgdat()`进行遍历

<h3 id="ch1.2">Zones</h3>

header: <linux/mmzone.h>

```
    typedef struct zone {
        
    } ____cacheline_internodealigned_in_smp;
```
