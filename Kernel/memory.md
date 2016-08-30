# Overview
book: [Understand Linux Memory Management](https://www.kernel.org/doc/gorman/html/understand/)
source code: [Linux Cross Refrence](http://lxr.free-electrons.com/)

> Only for some notes.

基础部分：
* [Describing Physical Memory](#ch1)
    * [Nodes](#ch1.1)
    * [Zones](#ch1.2)
        * [Zone Watermarks](#ch1.2.1)
        * [Calculating The Size of Zones](#ch1.2.2)
        * [Zone Wait Queue Table](#ch1.2.3)
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
![Describing Physical Memory](https://raw.githubusercontent.com/XiaokeFeng/notes/master/pictures/DescribingPhysicalMemory.png)

<h3 id="ch1.1">Nodes</h3>

header: \<linux/mmzone.h\>

```c++
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

header: \<linux/mmzone.h\>

```c++
    typedef struct zone {
    	unsigned long watermark[NR_WMARK];			// 水印，用于决定是否进行page balance
    	unsigned long nr_reserved_highatomic;
    	long lowmem_reserve[MAX_NR_ZONES];			// 预留内存，防止关键内存分配操作失败
    	unsigned int inactive_ratio;
    	struct pglist_data      *zone_pgdat;		// 指向自身node，用于遍历各个zone，具体看下面的例子1
        struct per_cpu_pageset __percpu *pageset;   // pre_cpu的pages
        unsigned long           totalreserve_pages; // 为用户态的内存申请预留的pages数目
        unsigned long           zone_start_pfn;     // zone_start_paddr >> PAGE_SHIFT
        unsigned long           spanned_pages;      // spanned_pages = zone_end_pfn - zone_start_pfn;
        unsigned long           present_pages;      // present_pages = spanned_pages - absent_pages(pages in holes);
        unsigned long           managed_pages;      // managed_pages = present_pages - reserved_pages;
        seqlock_t               span_seqlock;
        wait_queue_head_t       *wait_table;        // a hash table of wait queues of processes waiting on a page to be freed
        unsigned long           wait_table_hash_nr_entries; // the size of the hash table array
        unsigned long           wait_table_bits;    // the shift of wait_table_size == (1 << wait_table_bits)
        struct free_area        free_area[MAX_ORDER];   // free areas of different sizes
        spinlock_t              lock;
        spinlock_t              lru_lock;
        struct lruvec           lruvec;
        unsigned long percpu_drift_mark;
        ...
    } ____cacheline_internodealigned_in_smp;
```

每个zone由`zone`维护，`wait_on_page_locked()`和`unlock_page()`来等待一个page释放和释放一个page

例子1：

```c++
	struct zone* next_zone(struct zone* zone)
	{
		pg_data_t* pgdat = zone->zone_pgdat;
		if (zone < pgdat->node_zones + MAX_NR_ZONES - 1)
			zone++;
		else {
			pgdat = next_online_pgdat(pgdat);
			if (pgdat)
				zone = pgdat->node_zones;
			else
				zone = NULL;
		}
		return zone;
	}
```

<h4 id="ch1.2.1">Zone Watermarks</h4>

*kswapd*: 页置换守护进程（the pageout daemon）将在可用内存降低时被唤醒

* pages_low，在内存初始化的时候，由`free_area_init_core()`计算得出，*kswapd*由buddy allocator唤醒来释放Pages
* pages_min，*kswapd*将同步的将页写回到disk
* pages_high，*kswapd*休眠

<h4 id="ch1.2.2">Calculating The Size of Zones</h4>

TODO

<h4 id="ch1.2.3">Zone Wait Queue Table</h4>

Why Hash Table?当IO发生时，一个page应该有一个存放被`wait_on_page_locked()`的进程queue，当page被`unlock_page()`，唤醒queue；
显然应该让所有的page共享一个queue，但是为了避免惊群(thundering herd)问题，需要用hash table实现`zone->wait_table`；

`wait_table`在`free_area_init_core()`的时候申请。
`wait_table_size()`用于计算其大小。
`page_waitqueue()`负责返回一个可用的wait queue。

等待队列流程图：
![Sleeping On A Locked Page](https://raw.githubusercontent.com/XiaokeFeng/notes/master/pictures/SleepingOnALockedPage.png)

<h3 id="ch1.3">Zone Initialisation</h3>

`paging_init()` Each architecture performs this task differently.
UMA: `free_area_init()`
NUMA: `free_area_init_node()`
流程图：
![ZoneInitialisation](https://raw.githubusercontent.com/XiaokeFeng/notes/master/pictures/ZoneInitialisation.png)
