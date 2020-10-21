#include "common.h"
/*
cache block 存储空间的大小为 64B 
cache 存储空间的大小为 64KB 
8-way set associative 
标志位只需要 valid bit 即可 
替换算法采用随机方式 
write through 
not write allocate 

*/
#define BLOCK_SIZE 64
#define CACHE_SIZE 64<<10
#define ASSOCIATIVE_WAY_L1 8

struct Cache {
	bool valid;
	int tag;
	uint8_t data[BLOCK_SIZE];
}cache[CACHE_SIZE/BLOCK_SIZE];

void init_cache() {
	int i, j, block_num=CACHE_SIZE/BLOCK_SIZE;
	for(i = 0; i < block_num; i ++) {
		cache[i].valid=0;
		cache[i].tag=0;
		memset(cache[i].data,0,BLOCK_SIZE);
	}
}


uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/*
8-way  set associative  每组有8行
2^10 行 = 2^7组 * 8行/组
*/
uint32_t cache_read(hwaddr_t addr){
	uint32_t group_num=(addr>>6) & 0x7f;//2^7组
	int i;
	bool ok=false;
	for(i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1; i++){
		if (cache[i].valid && cache[i].tag == (addr>>13)){
			ok=true;
			break;
		}
	}
	
}

/* Memory accessing interfaces */
//size_t size type 类型大小
//~0u   0（unsigned int 类型 32位）按位取反后 右移
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	uint32_t offset = addr & (BLOCK_SIZE-1);
	uint32_t block = 
	return dram_read(addr, len) & (~0u >> ((4 - len) << 3));

}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	dram_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

