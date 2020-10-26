#include "common.h"
#include "burst.h"
#include "memory/cache.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);


/* Memory accessing interfaces */
//size_t size type 类型大小
//~0u   0（unsigned int 类型 32位）按位取反后 右移
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int cache1_index = cache_read(addr);
	uint32_t block_bias = addr & (BLOCK_SIZE - 1);
	uint8_t temp[BURST_LEN << 1];
	//如果跨域了边界
	if(block_bias + len > BLOCK_SIZE){
		int cache2_index = cache_read(addr + BLOCK_SIZE - block_bias);
		memcpy(temp, cache[cache1_index].data + block_bias, BLOCK_SIZE - block_bias);
		memcpy(temp  + BLOCK_SIZE - block_bias, cache[cache2_index].data, len - (BLOCK_SIZE - block_bias));
	}
	else {
		memcpy(temp, cache[cache1_index].data + block_bias, len);
	}
	int tmp = 0;
	uint32_t ans = unalign_rw(temp + tmp, 4) & (~0u >> ((4 - len) << 3));
	
	return ans;
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	
	// uint32_t offset = addr & (BLOCK_SIZE-1);
	// uint32_t block = cache_read(addr);
	// uint8_t temp[4];
	// memset (temp,0,sizeof (temp));
	// if (offset + len >= BLOCK_SIZE) {
	// 	uint32_t _block = cache_read(addr + len);
	// 	memcpy(temp , cache[block].data + offset, BLOCK_SIZE - offset);
	// 	memcpy(temp + BLOCK_SIZE - offset,cache[_block].data, len - (BLOCK_SIZE - offset));
	// }
	// else{
	// 	memcpy(temp,cache[block].data + offset,len);
	// }
	// int zero = 0;
	// uint32_t tmp = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3)); 
	// return tmp; 
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	// dram_write(addr, len, data);
	cache_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg_id){
	if(cpu.cr0.protect_enable == 0) return addr;
	else{
		return cpu.sreg[sreg_id].base + addr;
	}
}


uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
	lnaddr_write(lnaddr, len, data);
}

