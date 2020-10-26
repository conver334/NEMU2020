#include "nemu.h"
#include <time.h>
#include "burst.h"
#include <stdlib.h>

void init_cache(){
    int i;
    for(i = 0; i < Cache_L1_Set_Num * Cache_L1_Way_Num; i++){
        cache_L1[i].valid_value = false;
    }
    for(i = 0; i < Cache_L2_Set_Num * Cache_L2_Way_Num; i++){
        cache_L2[i].dirty_value = false;
        cache_L2[i].valid_value = false;
    }
    return ;
}

void ddr3_read_ref(hwaddr_t addr, void *data);

// return whole index of way in cacheL1
int read_Cache_L1(hwaddr_t addr){
    uint32_t set_index = ((addr >> Cache_Block_Bit) & (Cache_L1_Set_Num - 1));
    uint32_t tag = (addr >> (Cache_Block_Bit + Cache_L1_Set_Bit));
    // uint32_t block_start = ((addr >> Cache_Block_Bit) << Cache_Block_Bit);

    int way_index;
    int whole_begin_way_index = set_index * Cache_L1_Way_Num;
    int whole_end_way_index = (set_index + 1) * Cache_L1_Way_Num;
    for(way_index = whole_begin_way_index; way_index < whole_end_way_index; way_index++){
        if(cache_L1[way_index].valid_value && cache_L1[way_index].tag == tag){
            // Hit!
            return way_index;
        }
    }
    // Hit loss!
    // go to CacheL2
    srand(time(0));
    int way_index_L2 = read_Cache_L2(addr);
    way_index = whole_begin_way_index + rand() % Cache_L1_Way_Num;
    memcpy(cache_L1[way_index].data, cache_L2[way_index_L2].data, Cache_Block_Size);

    cache_L1[way_index].valid_value = true;
    cache_L1[way_index].tag = tag;
    return way_index;
}

void ddr3_write_ref(hwaddr_t addr, void *data, uint8_t *mask);

// return whole index of way in cacheL2
int read_Cache_L2(hwaddr_t addr){
    uint32_t set_index = ((addr >> Cache_Block_Bit) & (Cache_L2_Set_Num - 1));
    uint32_t tag = (addr >> (Cache_Block_Bit + Cache_L2_Set_Bit));
    uint32_t block_start = ((addr >> Cache_Block_Bit) << Cache_Block_Bit);

    int way_index;
    int whole_begin_way_index = set_index * Cache_L2_Way_Num;
    int whole_end_way_index = (set_index + 1) * Cache_L2_Way_Num;
    for(way_index = whole_begin_way_index; way_index < whole_end_way_index; way_index++){
        if(cache_L2[way_index].valid_value && cache_L2[way_index].tag == tag){
            // Hit!
            return way_index;
        }
    }
    // Hit loss!
    srand(time(0));
    way_index = whole_begin_way_index + rand() % Cache_L2_Way_Num;
    int i;
    if(cache_L2[way_index].valid_value && cache_L2[way_index].dirty_value){
        // write down
        uint8_t tmp[BURST_LEN << 1];
        memset(tmp, 1, sizeof(tmp));
        uint32_t block_start_x = (cache_L2[way_index].tag << (Cache_L2_Set_Bit + Cache_Block_Bit)) | (set_index << Cache_Block_Bit);
        for(i = 0; i < Cache_Block_Size / BURST_LEN; i++){
            ddr3_write_ref(block_start_x + BURST_LEN * i, cache_L2[way_index].data + BURST_LEN * i, tmp);
        }
    }
    for(i = 0; i < Cache_Block_Size / BURST_LEN; i++){
        ddr3_read_ref(block_start + BURST_LEN * i, cache_L2[way_index].data + BURST_LEN * i);
    }
    cache_L2[way_index].valid_value = true;
    cache_L2[way_index].dirty_value = false;
    cache_L2[way_index].tag = tag;
    return way_index;
}

void dram_write(hwaddr_t addr, size_t len, uint32_t data);

void write_Cache_L1(hwaddr_t addr, size_t len, uint32_t data){
    uint32_t set_index = ((addr >> Cache_Block_Bit) & (Cache_L1_Set_Num - 1));
    uint32_t tag = (addr >> (Cache_Block_Bit + Cache_L1_Set_Bit));
    uint32_t block_bias = addr & (Cache_Block_Size - 1);
    int way_index;
    int whole_begin_way_index = set_index * Cache_L1_Way_Num;
    int whole_end_way_index = (set_index + 1) * Cache_L1_Way_Num;
    for(way_index = whole_begin_way_index; way_index < whole_end_way_index; way_index++){
        if(cache_L1[way_index].valid_value && cache_L1[way_index].tag == tag){
            // Hit!
            // write through
            if(block_bias + len > Cache_Block_Size){
                dram_write(addr, Cache_Block_Size - block_bias, data);
                memcpy(cache_L1[way_index].data + block_bias, &data, Cache_Block_Size - block_bias);
                write_Cache_L2(addr, Cache_Block_Size - block_bias, data);
                write_Cache_L1(addr + Cache_Block_Size - block_bias,len - (Cache_Block_Size - block_bias),data >> (Cache_Block_Size - block_bias));
            }
            else{
                dram_write(addr, len, data);
                memcpy(cache_L1[way_index].data + block_bias, &data, len);
                write_Cache_L2(addr, len, data);
            }
            return ;
        }
    }
    //  Hit loss!
    // not write allocate
    write_Cache_L2(addr, len, data);
    return ;
}

void write_Cache_L2(hwaddr_t addr, size_t len, uint32_t data){
    uint32_t set_index = ((addr >> Cache_Block_Bit) & (Cache_L2_Set_Num - 1));
    uint32_t tag = (addr >> (Cache_Block_Bit + Cache_L2_Set_Bit));
    uint32_t block_bias = addr & (Cache_Block_Size - 1);
    int way_index;
    int whole_begin_way_index = set_index * Cache_L2_Way_Num;
    int whole_end_way_index = (set_index + 1) * Cache_L2_Way_Num;
    for(way_index = whole_begin_way_index; way_index < whole_end_way_index; way_index++){
        if(cache_L2[way_index].valid_value && cache_L2[way_index].tag == tag){
            // Hit!
            // write back
            cache_L2[way_index].dirty_value = true;
            if(block_bias + len > Cache_Block_Size){
                memcpy(cache_L2[way_index].data + block_bias, &data, Cache_Block_Size - block_bias);
                write_Cache_L2(addr + Cache_Block_Size - block_bias,len - (Cache_Block_Size - block_bias),data >> (Cache_Block_Size - block_bias));
            }
            else{
                memcpy(cache_L2[way_index].data + block_bias, &data, len);
            }
            return ;
        }
    }
    //  Hit loss!
    // write allocate
    way_index = read_Cache_L2(addr);
    cache_L2[way_index].dirty_value = true;
    memcpy(cache_L2[way_index].data + block_bias, &data, len);
    return ;
}
// // #include "common.h"
// // #include "burst.h"
// // #include "memory/cache.h"
// // #include <stdlib.h>
// // #include <time.h>
// #include "nemu.h"
// #include <time.h>
// #include "burst.h"
// #include <stdlib.h>
// #define random(x) (rand()%x)
// void ddr3_read_me(hwaddr_t addr, void *data);
// void ddr3_write_me(hwaddr_t addr, void *data, uint8_t *mask);
// uint32_t dram_read(hwaddr_t addr, size_t len) ;
// void dram_write(hwaddr_t addr, size_t len, uint32_t data);
// void init_cache() {
// 	int i, block_num = CACHE_SIZE/BLOCK_SIZE;
// 	for(i = 0; i < block_num; i ++) {
// 		cache[i].valid=0;
// 		// cache[i].tag=0;
// 		// memset(cache[i].data,0,BLOCK_SIZE);
// 	}
// 	block_num = CACHE2_SIZE/BLOCK_SIZE;
// 	for(i = 0; i < block_num; i ++) {
// 		cache2[i].valid=0;
// 		cache2[i].dirty=0;
// 	}
// }


// /*
// cache block 存储空间的大小为 64B 
// cache 存储空间的大小为 4MB  =2+10+10
// 16-way set associative 每组有16行
// 2^16 行 = 2^12组 * 2^4行/组
// */
// uint32_t cache2_read(hwaddr_t addr){
// 	uint32_t group_num = (addr >> BLOCK_SIZE_BIT) & ((1 << GROUP_BIT_L2)  -  1);//2^12组
//     uint32_t tag = addr>>(BLOCK_SIZE_BIT+GROUP_BIT_L2);
//     uint32_t block =  (addr >> BLOCK_SIZE_BIT)<< BLOCK_SIZE_BIT;
// 	int i;
// 	for(i = group_num * ASSOCIATIVE_WAY_L2 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L2; i++){
// 		if (cache2[i].valid && cache2[i].tag == tag){
// 			return i;
// 		}
// 	}
	
// 	srand((int)time(0));
// 	int qwq = group_num * ASSOCIATIVE_WAY_L2 + random(ASSOCIATIVE_WAY_L2);
// 	if(cache2[qwq].valid && cache2[qwq].dirty){
// 		uint8_t tmp[BURST_LEN << 1];
//         memset(tmp, 1, sizeof(tmp));
//         for(i = 0; i < BLOCK_SIZE / BURST_LEN; i++){
//             ddr3_write_me(block + BURST_LEN * i, cache2[qwq].data + BURST_LEN * i, tmp);
//         }
// 	}
// 	for(i = 0; i < BLOCK_SIZE / BURST_LEN; i++){
//         ddr3_read_me(block + BURST_LEN * i, cache2[qwq].data + BURST_LEN * i);
//     }
//     // for(i = group_num * ASSOCIATIVE_WAY_L2 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L2; i++){
//     //     if(!cache2[i].valid)break;
//     // }
//     // if(i == (group_num + 1) * ASSOCIATIVE_WAY_L2){
//     //         srand((int)time(0));
//     //         i = group_num * ASSOCIATIVE_WAY_L2 + random(ASSOCIATIVE_WAY_L2);
//     //         if(cache2[i].dirty){
//     //         uint8_t mask[BURST_LEN * 2];
//     //         memset(mask, 1, BURST_LEN * 2);
//     //         for (j = 0;j < BLOCK_SIZE/BURST_LEN;j ++)
//     //         ddr3_write_me(block + j * BURST_LEN, cache2[i].data + j * BURST_LEN, mask);
//     //     }
//     // }
//     cache2[i].valid = true;
//     cache2[i].dirty = false;
//     cache2[i].tag = tag;
//     // ddr3_read_me(block + j * BURST_LEN , cache2[i].data + j * BURST_LEN);
//     return qwq;
// }
// /*
// 8-way  set associative  每组有8行
// 2^10 行 = 2^7组 * 8行/组
// */
// uint32_t cache_read(hwaddr_t addr){
// 	uint32_t group_num=(addr>>BLOCK_SIZE_BIT) & 0x7f;//2^7组
//     uint32_t tag = addr>>(BLOCK_SIZE_BIT + GROUP_BIT_L1);
// 	int i;
// 	for(i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1; i++){
// 		if (cache[i].valid && cache[i].tag == tag){
// 			return i;
// 		}
// 	}
	
//     int j = cache2_read(addr);
// 	srand((int)time(0));
// 	i = group_num * ASSOCIATIVE_WAY_L1 + random(ASSOCIATIVE_WAY_L1);
	
//     // for(i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1; i++){
//     //     if(!cache[i].valid)break;
//     // }
//     // if(i == (group_num + 1) * ASSOCIATIVE_WAY_L1){
//     //         
//     //         i = 
//     // }
//     cache[i].valid = true;
//     cache[i].tag = tag;
//     memcpy (cache[i].data,cache2[j].data,BLOCK_SIZE);
//     return i;
// }
// void cache2_write(hwaddr_t addr, size_t len,uint32_t data) {
//     uint32_t group_num = (addr >> BLOCK_SIZE_BIT) & ((1 << GROUP_BIT_L2) - 1);//2^12组
// 	uint32_t offset = addr & (BLOCK_SIZE - 1);
//     uint32_t tag = addr>>(BLOCK_SIZE_BIT+GROUP_BIT_L2);
// 	int i;
// 	for (i = group_num * ASSOCIATIVE_WAY_L2 ; i < (group_num + 1) *  ASSOCIATIVE_WAY_L2 ;i ++){
// 		if (cache2[i].tag == tag && cache2[i].valid){
// 			cache2[i].dirty = true;
// 			if(offset + len > BLOCK_SIZE) {//边界
// 				memcpy(cache2[i].data + offset, &data, BLOCK_SIZE - offset);
// 				cache2_write(addr + BLOCK_SIZE - offset, len - BLOCK_SIZE + offset, data >> (BLOCK_SIZE - offset));
// 			} else {
// 				memcpy(cache2[i].data + offset, &data, len);
// 			}
// 			return;
// 		}
// 	}
// 	i = cache2_read (addr);
// 	cache2[i].dirty = true;
// 	memcpy (cache2[i].data + offset , &data , len);
// }
// void cache_write(hwaddr_t addr, size_t len,uint32_t data) {
// 	uint32_t group_num = (addr>>BLOCK_SIZE_BIT) & 0x7f; 
// 	uint32_t offset = addr & (BLOCK_SIZE - 1); 
//     uint32_t tag = addr>>(BLOCK_SIZE_BIT + GROUP_BIT_L1);
// 	int i;
// 	for (i = group_num * ASSOCIATIVE_WAY_L1 ; i < (group_num + 1) * ASSOCIATIVE_WAY_L1  ;i ++){
//         if ( cache[i].valid && cache[i].tag == tag){
// 			if(offset + len > BLOCK_SIZE) {
// 				dram_write(addr, BLOCK_SIZE - offset, data);
// 				memcpy(cache[i].data + offset, &data, BLOCK_SIZE - offset);
// 				cache2_write(addr, BLOCK_SIZE - offset, data);
// 				cache_write(addr + BLOCK_SIZE - offset,len - (BLOCK_SIZE - offset),data >> (BLOCK_SIZE - offset));
// 				// cache2_write(addr + BLOCK_SIZE - offset, len - BLOCK_SIZE + offset, data >> (BLOCK_SIZE - offset));
// 			}
// 			else {
// 				dram_write(addr, len, data);
// 				memcpy(cache[i].data + offset, &data, len);
// 				cache2_write(addr, len, data);
// 			}
// 			return;
// 		}
// 	}
// 	cache2_write(addr,len,data);
// }