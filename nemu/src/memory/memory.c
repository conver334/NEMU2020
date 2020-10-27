#include "common.h"
#include "memory/cache.h"
#include "burst.h"
#include "nemu.h"
#include <string.h>

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);


/* Memory accessing interfaces */
//size_t size type 类型大小
//~0u   0（unsigned int 类型 32位）按位取反后 右移
// uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
// 	int cache1_index = cache_read(addr);
// 	uint32_t block_bias = addr & (BLOCK_SIZE - 1);
// 	uint8_t temp[BURST_LEN << 1];
// 	//如果跨域了边界
// 	if(block_bias + len > BLOCK_SIZE){
// 		int cache2_index = cache_read(addr + BLOCK_SIZE - block_bias);
// 		memcpy(temp, cache[cache1_index].data + block_bias, BLOCK_SIZE - block_bias);
// 		memcpy(temp  + BLOCK_SIZE - block_bias, cache[cache2_index].data, len - (BLOCK_SIZE - block_bias));
// 	}
// 	else {
// 		memcpy(temp, cache[cache1_index].data + block_bias, len);
// 	}
// 	int tmp = 0;
// 	uint32_t ans = unalign_rw(temp + tmp, 4) & (~0u >> ((4 - len) << 3));
	
// 	return ans;
// 	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	
// 	// uint32_t offset = addr & (BLOCK_SIZE-1);
// 	// uint32_t block = cache_read(addr);
// 	// uint8_t temp[4];
// 	// memset (temp,0,sizeof (temp));
// 	// if (offset + len >= BLOCK_SIZE) {
// 	// 	uint32_t _block = cache_read(addr + len);
// 	// 	memcpy(temp , cache[block].data + offset, BLOCK_SIZE - offset);
// 	// 	memcpy(temp + BLOCK_SIZE - offset,cache[_block].data, len - (BLOCK_SIZE - offset));
// 	// }
// 	// else{
// 	// 	memcpy(temp,cache[block].data + offset,len);
// 	// }
// 	// int zero = 0;
// 	// uint32_t tmp = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3)); 
// 	// return tmp; 
// 	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
// }

// void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
// 	// dram_write(addr, len, data);
// 	cache_write(addr, len, data);
// }
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg_id){
	if(cpu.cr0.protect_enable == 0) return addr;
	else{
		return cpu.sreg[sreg_id].base + addr;
	}
}
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int cache_L1_way_1_index = read_Cache_L1(addr);
	uint32_t block_bias = addr & (Cache_Block_Size - 1);
	uint8_t ret[BURST_LEN << 1];
	// printf("%d\n", block_bias);
	if(block_bias + len > Cache_Block_Size){
		int cache_L1_way_2_index = read_Cache_L1(addr + Cache_Block_Size - block_bias);
		memcpy(ret, cache_L1[cache_L1_way_1_index].data + block_bias, Cache_Block_Size - block_bias);
		memcpy(ret  + Cache_Block_Size - block_bias, cache_L1[cache_L1_way_2_index].data, len - (Cache_Block_Size - block_bias));
	}
	else {
		memcpy(ret, cache_L1[cache_L1_way_1_index].data + block_bias, len);
	}
	int tmp = 0;
	uint32_t ans = unalign_rw(ret + tmp, 4) & (~0u >> ((4 - len) << 3));
	// print(ans);
	return ans;
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	write_Cache_L1(addr, len, data);
	// dram_write(addr, len, data);
}

hwaddr_t page_translate_without_assert(lnaddr_t addr, int* success){
	// TO DO
	if(cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1){
		uint32_t dir = addr >> 22;
		uint32_t page = (addr >> 12) & 0x3ff;
		uint32_t bias = addr & 0xfff;
		
		int i = read_tlb(addr);
		if(i != -1) return (tlb[i].page_num << 12) + bias;

		uint32_t dir_start = cpu.cr3.page_directory_base;
		uint32_t dir_position = (dir_start << 12) +(dir << 2);
		Page_Descriptor first_content;
		first_content.val = hwaddr_read(dir_position, 4);
		// Assert(first_content.p == 1,"Page unavailable");
		if(first_content.p != 1){
			*success = 1;
			return 0;
		}
		uint32_t page_start = first_content.addr;
		uint32_t page_pos = (page_start << 12) + (page << 2);
		Page_Descriptor second_content;
		second_content.val =  hwaddr_read(page_pos,4);
		// Assert(second_content.p == 1,"Page unavailable");
		if(second_content.p != 1){
			*success = 2;
			return 0;
		}
		uint32_t addr_start = second_content.addr;
		hwaddr_t hwaddr = (addr_start << 12) + bias;
		return hwaddr;
	}
	else{
		return addr;
	}
}
hwaddr_t page_translate(lnaddr_t addr){
	// TO DO
	// printf("%x\n", addr);
	if(cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1){
		uint32_t dir = addr >> 22;
		uint32_t page = (addr >> 12) & 0x3ff;
		uint32_t bias = addr & 0xfff;

		int i = read_tlb(addr);
		if(i != -1) return (tlb[i].page_num << 12) + bias;

		uint32_t dir_start = cpu.cr3.page_directory_base;
		uint32_t dir_position = (dir_start << 12) +(dir << 2);
		Page_Descriptor first_content;
		first_content.val = hwaddr_read(dir_position, 4);
		Assert(first_content.p == 1,"Page unavailable");
		uint32_t page_start = first_content.addr;
		uint32_t page_pos = (page_start << 12) + (page << 2);
		Page_Descriptor second_content;
		second_content.val =  hwaddr_read(page_pos,4);
		Assert(second_content.p == 1,"Page unavailable");
		uint32_t addr_start = second_content.addr;
		hwaddr_t hwaddr = (addr_start << 12) + bias;
		write_tlb(addr, hwaddr);
		return hwaddr;
	}
	else{
		return addr;
	}
}
uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	// assert(len == 1 || len == 2 || len == 4);
	uint32_t cur_bias = addr & 0xfff;
	if(cur_bias + len - 1 > 0xfff){
		size_t len1 = 0xfff - cur_bias + 1;
		size_t len2 = len - len1;
		uint32_t addr_len1 = lnaddr_read(addr, len1);
		uint32_t addr_len2 = lnaddr_read(addr + len1, len2);
		uint32_t value = (addr_len2 << (len1 << 3)) | addr_len1;
		return value;
	}
	else{
		hwaddr_t hwaddr = page_translate(addr);
		return hwaddr_read(hwaddr, len);
	}
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	// assert(len == 1 || len == 2 || len == 4);
	uint32_t cur_bias = addr & 0xfff;
	if(cur_bias + len - 1 > 0xfff){
		size_t len1 = 0xfff - cur_bias + 1;
		size_t len2 = len - len1;
		lnaddr_write(addr, len1, data & ((1 << (len1 << 3)) - 1));
		lnaddr_write(addr + len1, len2, data >> (len1 << 3));
	}
	else{
		hwaddr_t hwaddr = page_translate(addr);
		hwaddr_write(hwaddr, len, data);	
	}
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

