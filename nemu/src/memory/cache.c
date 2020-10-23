#include "common.h"
#include "burst.h"
#include "memory/cache.h"
#include <stdlib.h>
#include <time.h>
#define random(x) (rand()%x)
void ddr3_read_me(hwaddr_t addr, void *data);
void ddr3_write_me(hwaddr_t addr, void *data, uint8_t *mask);
uint32_t dram_read(hwaddr_t addr, size_t len) ;
void dram_write(hwaddr_t addr, size_t len, uint32_t data);
void init_cache() {
	int i, block_num = CACHE_SIZE/BLOCK_SIZE;
	for(i = 0; i < block_num; i ++) {
		cache[i].valid=0;
		// cache[i].tag=0;
		// memset(cache[i].data,0,BLOCK_SIZE);
	}
	block_num = CACHE2_SIZE/BLOCK_SIZE;
	for(i = 0; i < block_num; i ++) {
		cache2[i].valid=0;
		cache2[i].dirty=0;
	}
}


/*
cache block 存储空间的大小为 64B 
cache 存储空间的大小为 4MB  =2+10+10
16-way set associative 每组有16行
2^16 行 = 2^12组 * 2^4行/组
*/
uint32_t cache2_read(hwaddr_t addr){
	uint32_t group_num = (addr >> BLOCK_SIZE_BIT) & ((1 << GROUP_BIT_L2)  -  1);//2^12组
    uint32_t tag = addr>>(BLOCK_SIZE_BIT+GROUP_BIT_L2);
    uint32_t block =  (addr >> BLOCK_SIZE_BIT)<< BLOCK_SIZE_BIT;
	int i;
	for(i = group_num * ASSOCIATIVE_WAY_L2 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L2; i++){
		if (cache2[i].valid && cache2[i].tag == tag){
			return i;
		}
	}
	
    int j;
    for(i = group_num * ASSOCIATIVE_WAY_L2 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L2; i++){
        if(!cache2[i].valid)break;
    }
    if(i == (group_num + 1) * ASSOCIATIVE_WAY_L2){
            srand((int)time(0));
            i = group_num * ASSOCIATIVE_WAY_L2 + random(ASSOCIATIVE_WAY_L2);
            if(cache2[i].dirty){
            uint8_t mask[BURST_LEN * 2];
            memset(mask, 1, BURST_LEN * 2);
            for (j = 0;j < BLOCK_SIZE/BURST_LEN;j ++)
            ddr3_write_me(block + j * BURST_LEN, cache2[i].data + j * BURST_LEN, mask);
        }
    }
    cache2[i].valid = true;
    cache2[i].dirty = false;
    cache2[i].tag = tag;
    ddr3_read_me(block + j * BURST_LEN , cache2[i].data + j * BURST_LEN);
    return i;
}
/*
8-way  set associative  每组有8行
2^10 行 = 2^7组 * 8行/组
*/
uint32_t cache_read(hwaddr_t addr){
	uint32_t group_num=(addr>>BLOCK_SIZE_BIT) & 0x7f;//2^7组
    uint32_t tag = addr>>(BLOCK_SIZE_BIT + GROUP_BIT_L1);
	int i;
	for(i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1; i++){
		if (cache[i].valid && cache[i].tag == tag){
			return i;
		}
	}
	
    int j = cache2_read(addr);
	srand((int)time(0));
	i = group_num * ASSOCIATIVE_WAY_L1 + random(ASSOCIATIVE_WAY_L1);
	
    // for(i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1; i++){
    //     if(!cache[i].valid)break;
    // }
    // if(i == (group_num + 1) * ASSOCIATIVE_WAY_L1){
    //         
    //         i = 
    // }
    cache[i].valid = true;
    cache[i].tag = tag;
    memcpy (cache[i].data,cache2[j].data,BLOCK_SIZE);
    return i;
}
void cache2_write(hwaddr_t addr, size_t len,uint32_t data) {
    uint32_t group_num = (addr >> BLOCK_SIZE_BIT) & ((1 << GROUP_BIT_L2) - 1);//2^12组
	uint32_t offset = addr & (BLOCK_SIZE - 1);
    uint32_t tag = addr>>(BLOCK_SIZE_BIT+GROUP_BIT_L2);
	int i;
	for (i = group_num * ASSOCIATIVE_WAY_L2 ; i < (group_num + 1) *  ASSOCIATIVE_WAY_L2 ;i ++){
		if (cache2[i].tag == tag && cache2[i].valid){
			cache2[i].dirty = true;
			if(offset + len > BLOCK_SIZE) {//across
				memcpy(cache2[i].data + offset, &data, BLOCK_SIZE - offset);
				cache2_write(addr + BLOCK_SIZE - offset, len - BLOCK_SIZE + offset, data >> (BLOCK_SIZE - offset));
			} else {
				memcpy(cache2[i].data + offset, &data, len);
			}
			return;
		}
	}
	i = cache2_read (addr);
	cache2[i].dirty = true;
	memcpy (cache2[i].data + offset , &data , len);
}
void cache_write(hwaddr_t addr, size_t len,uint32_t data) {
	uint32_t group_num = (addr>>BLOCK_SIZE_BIT) & 0x7f; 
	uint32_t offset = addr & (BLOCK_SIZE - 1); 
    uint32_t tag = addr>>(BLOCK_SIZE_BIT + GROUP_BIT_L1);
	int i;
	for (i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1  ;i ++){
        if ( cache[i].valid && cache[i].tag ==tag){
			if(offset + len > BLOCK_SIZE) {
				dram_write(addr, BLOCK_SIZE - offset, data);
				memcpy(cache[i].data + offset, &data, BLOCK_SIZE - offset);
				cache2_write(addr, BLOCK_SIZE - offset, data);
				cache2_write(addr + BLOCK_SIZE - offset, len - BLOCK_SIZE + offset, data >> (BLOCK_SIZE - offset));
			}
			else {
				dram_write(addr, len, data);
				memcpy(cache[i].data + offset, &data, len);
				cache2_write(addr, len, data);
			}
			return;
		}
	}
	cache2_write(addr,len,data);
}