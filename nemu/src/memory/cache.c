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
                write_Cache_L1(addr + Cache_Block_Size - block_bias,len - (Cache_Block_Size - block_bias),data >> (8 * (Cache_Block_Size - block_bias)));
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
                write_Cache_L2(addr + Cache_Block_Size - block_bias,len - (Cache_Block_Size - block_bias),data >> (8 * (Cache_Block_Size - block_bias)));
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