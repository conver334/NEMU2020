#ifndef __CACHE_H__
#define __CACHE_H_

#define BLOCK_SIZE_BIT 6
#define BLOCK_SIZE 1<<BLOCK_SIZE_BIT
#define CACHE_SIZE_BIT 16
#define CACHE_SIZE 1<<CACHE_SIZE_BIT 
#define ASSOCIATIVE_WAY_L1 8
#define WAY_BIT_L1 3
#define GROUP_BIT_L1 CACHE_SIZE_BIT-BLOCK_SIZE_BIT-WAY_BIT_L1

/*
cache block 存储空间的大小为 64B 
cache 存储空间的大小为 64KB 
8-way set associative 
标志位只需要 valid bit 即可 
替换算法采用随机方式 
write through 
not write allocate 
*/
#define CACHE2_SIZE_BIT 22
#define CACHE2_SIZE 1<<CACHE2_SIZE_BIT 
#define ASSOCIATIVE_WAY_L2 16
#define WAY_BIT_L2 4
#define GROUP_BIT_L2 CACHE2_SIZE_BIT-BLOCK_SIZE_BIT-WAY_BIT_L2

/*
cache block 存储空间的大小为 64B 
cache 存储空间的大小为 4MB  =2+10+10
16-way set associative 
标志位包括 valid bit 和 dirty bit 
替换算法采用随机方式 
write back 
write allocate 
*/
struct Cache {
	bool valid;
	int tag;
	uint8_t data[BLOCK_SIZE];
}cache[CACHE_SIZE/BLOCK_SIZE];
struct Cache2 {
	bool valid,dirty;
	int tag;
	uint8_t data[BLOCK_SIZE];
}cache2[CACHE2_SIZE/BLOCK_SIZE];

void init_cache();
uint32_t cache2_read(hwaddr_t addr);
uint32_t cache_read(hwaddr_t addr);
void cache_write(hwaddr_t addr, size_t len,uint32_t data);
#endif