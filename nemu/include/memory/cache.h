#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

/*
Cache L1
cache block 存储空间的大小为 64B
cache 存储空间的大小为 64KB
8-way set associative
标志位只需要 valid bit 即可
替换算法采用随机方式
write through(write
through(写通)策略要求每次 cache 的写操作都同时更新内存)
not write allocate(非写分配法：若写不命中,仅更新主存单元而不装入主存块到 Cache 中)
*/

#define Cache_Block_Bit 6
#define Cache_Block_Size 64
#define Cache_L1_Cap 64 * 1024
#define Cache_L1_Way_Bit 3
#define Cache_L1_Way_Num 8
#define Cache_L1_Set_Bit 7
#define Cache_L1_Set_Num 128

typedef struct{
    uint8_t data[Cache_Block_Size];
    uint32_t tag;
    bool valid_value;
}Cache_L1_Way;

Cache_L1_Way cache_L1[Cache_L1_Set_Num * Cache_L1_Way_Num];

void init_cache();

int read_Cache_L1(hwaddr_t);

void write_Cache_L1(hwaddr_t, size_t, uint32_t);

/*
Cache L2
cache block 存储空间的大小为 64B
cache 存储空间的大小为 4MB
16-way set associative
标志位包括 valid bit 和 dirty bit
替换算法采用随机方式
write back
write allocate
*/

#define Cache_L2_Cap 4 * 1024 * 1024
#define Cache_L2_Way_Bit 4
#define Cache_L2_Way_Num 16
#define Cache_L2_Set_Bit 12
#define Cache_L2_Set_Num 4096

typedef struct{
    uint8_t data[Cache_Block_Size];
    uint32_t tag;
    bool valid_value;
    bool dirty_value;
}Cache_L2_Way;

Cache_L2_Way cache_L2[Cache_L2_Set_Num * Cache_L2_Way_Num];

void init_cache();

int read_Cache_L1(hwaddr_t);
int read_Cache_L2(hwaddr_t);

void write_Cache_L1(hwaddr_t, size_t, uint32_t);
void write_Cache_L2(hwaddr_t, size_t, uint32_t);


#endif
// #ifndef __CACHE_H__
// #define __CACHE_H_

// #include "common.h"

// #define BLOCK_SIZE_BIT 6
// #define BLOCK_SIZE  (1<<BLOCK_SIZE_BIT)
// #define CACHE_SIZE_BIT 16
// #define CACHE_SIZE  (1<<CACHE_SIZE_BIT )
// #define ASSOCIATIVE_WAY_L1 8
// #define WAY_BIT_L1 3
// // #define GROUP_BIT_L1  (CACHE_SIZE_BIT-BLOCK_SIZE_BIT-WAY_BIT_L1)
// #define GROUP_BIT_L1 7
// /*
// cache block 存储空间的大小为 64B 
// cache 存储空间的大小为 64KB 
// 8-way set associative 
// 标志位只需要 valid bit 即可 
// 替换算法采用随机方式 
// write through 
// not write allocate 
// */
// #define CACHE2_SIZE_BIT 22
// #define CACHE2_SIZE  (1<<CACHE2_SIZE_BIT )
// #define ASSOCIATIVE_WAY_L2 16
// #define WAY_BIT_L2 4
// // #define GROUP_BIT_L2  (CACHE2_SIZE_BIT-BLOCK_SIZE_BIT-WAY_BIT_L2)
// #define GROUP_BIT_L2 12
// /*
// cache block 存储空间的大小为 64B 
// cache 存储空间的大小为 4MB  =2+10+10
// 16-way set associative 
// 标志位包括 valid bit 和 dirty bit 
// 替换算法采用随机方式 
// write back 
// write allocate 
// */
// typedef struct  {
// 	bool valid;
// 	int tag;
// 	uint8_t data[BLOCK_SIZE];
// }Cache_zsm;
// Cache_zsm cache_zsm[CACHE_SIZE/BLOCK_SIZE];
// typedef struct{
// 	bool valid,dirty;
// 	int tag;
// 	uint8_t data[BLOCK_SIZE];
// } Cache2_zsm ;
// Cache2_zsm cache2_zsm[CACHE2_SIZE/BLOCK_SIZE];

// void init_cache();

// uint32_t cache_read(hwaddr_t addr);
// void cache_write(hwaddr_t addr, size_t len,uint32_t data);
// uint32_t cache2_read(hwaddr_t addr);
// void cache2_write(hwaddr_t addr, size_t len,uint32_t data);

// #endif