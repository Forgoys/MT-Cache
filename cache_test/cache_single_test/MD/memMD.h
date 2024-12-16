#define MEMORY_PRO_MAX_NUM 16
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include "hthread_device.h"
#define SM_CAPACITY 8192 // 8KB in bytes
#define CACHE_LINE_SIZE_4KB 1024 // 4KB line size in bytes
#define CACHE_LINE_SIZE_128B 128 // 128B line size in bytes
void determine_cache_settings(int max_step, double probability, char *celue) {
    int lines = 0;
    int cets = 0;

    if (probability >= 0.995) {
        lines = 12; // 4KB
        cets = SM_CAPACITY / (lines * CACHE_LINE_SIZE_4KB / 1024); // 确保cets * 缓存行大小小于SM容量
    } else if (probability >= 0.85) {
        lines = 7; // 128B
        cets = SM_CAPACITY / (lines * CACHE_LINE_SIZE_128B / 1024); // 确保cets * 缓存行大小小于SM容量
    } else {
        // 中间情况，线性分配
        // 这里我们假设线性分配意味着在7到12之间进行插值
        // 从85%到99.5%，概率每增加1%，lines减少1
        lines = 12 - (int)((probability - 0.85) / (0.995 - 0.85) * (12 - 7));
        // 确保lines至少为7
        lines = (lines < 7) ? 7 : lines;
        // 计算cets
        cets = SM_CAPACITY / (lines * (lines <= 7 ? CACHE_LINE_SIZE_128B : CACHE_LINE_SIZE_4KB) / 1024);
    }

    // 根据计算出的lines和cets生成建议字符串
    sprintf(celue, "s %d %d", cets, lines);
}
typedef struct {
    char function_name[64];
    char variable_name[256];
    char variable_type[64];
    unsigned long address_begin;
    unsigned long address_end;
    unsigned long accessed;
    unsigned long memory_step[MEMORY_PRO_MAX_NUM];
    unsigned long memory_step_pro[MEMORY_PRO_MAX_NUM];
    unsigned long variable_size;
    unsigned long sorted_steps[MEMORY_PRO_MAX_NUM];
    float sorted_pro[MEMORY_PRO_MAX_NUM];
    size_t type_size;  // 添加变量类型大小
} memory_info_t;

static inline void start_instrumentation(const char* var_name, void* var_ptr, size_t type_size, memory_info_t* mem_info) { 
    strncpy(mem_info->variable_name, var_name, sizeof(mem_info->variable_name)); 
    mem_info->address_begin = (unsigned long)var_ptr; 
    mem_info->address_end = mem_info->address_begin; // Initialize to the beginning
    mem_info->accessed = 0; 
    mem_info->type_size = type_size; // 初始化变量类型大小
    memset(mem_info->memory_step, 0, sizeof(mem_info->memory_step)); 
    memset(mem_info->memory_step_pro, 0, sizeof(mem_info->memory_step_pro)); 
} 

static inline void record_memory_access(memory_info_t* mem_info, unsigned long current_addr) { 
    unsigned long last_addr = mem_info->address_end; 
    unsigned long raw_step = current_addr - last_addr; 
    unsigned long normalized_step = raw_step / mem_info->type_size; // 归一化步长
    if(current_addr > mem_info->address_end){
        mem_info->address_end = current_addr; 
    }

    if (normalized_step >= 0 && normalized_step < 66536) { 
        mem_info->accessed++; 
        int found = 0; 
        for (int j = 0; j < MEMORY_PRO_MAX_NUM; j++) { 
            if (mem_info->memory_step[j] == normalized_step) { 
                mem_info->memory_step_pro[j]++; 
                found = 1; 
                break; 
            } else if (mem_info->memory_step[j] == 0) { 
                mem_info->memory_step[j] = normalized_step; 
                mem_info->memory_step_pro[j] = 1; 
                found = 1; 
                break; 
            } 
        } 
        if (!found) { 
            // Handle overflow if MEMORY_PRO_MAX_NUM is exceeded 
        } 
    } 
} 

static inline void end_instrumentation(memory_info_t* mem_info) { 
    mem_info->variable_size = mem_info->address_end - mem_info->address_begin + mem_info->type_size; 

    // 计算步长比例并排序
    float step_pro[MEMORY_PRO_MAX_NUM] = {0};
    for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
        //if (mem_info->memory_step[i] != 0) {
            step_pro[i] = (float)mem_info->memory_step_pro[i] / mem_info->accessed;
        //}
    }

    // 对步长和步长比例进行排序（按比例从大到小）
    for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
        mem_info->sorted_steps[i] = mem_info->memory_step[i];
        mem_info->sorted_pro[i] = step_pro[i];
    }

    for (int i = 0; i < MEMORY_PRO_MAX_NUM - 1; i++) {
        for (int j = i + 1; j < MEMORY_PRO_MAX_NUM; j++) {
            if (mem_info->sorted_pro[i] < mem_info->sorted_pro[j]) {
                // 交换步长
                unsigned long temp_step = mem_info->sorted_steps[i];
                mem_info->sorted_steps[i] = mem_info->sorted_steps[j];
                mem_info->sorted_steps[j] = temp_step;

                // 交换比例
                float temp_pro = mem_info->sorted_pro[i];
                mem_info->sorted_pro[i] = mem_info->sorted_pro[j];
                mem_info->sorted_pro[j] = temp_pro;
            }
        }
    }
}

static inline void hprint_memory_info(const memory_info_t *info) {
    char buffer[1024]; // 假设这个缓冲区足够大以容纳所有输出  
    char celue[64];
    if (info->variable_size <= 4096) {
        strcpy(celue, "b 4096");
    }else if(info->sorted_steps[0] <= 1 && info->sorted_pro[0] > 0.95){
        strcpy(celue, "s 12");
    }else if(info->sorted_steps[0] > 1 && info->sorted_pro[0] > 0.85){
         determine_cache_settings(info->sorted_steps[0], info->sorted_pro[0], celue);
    }
    snprintf(buffer, sizeof(buffer), 
        "\nfunction_name: %s\n" 
        "\nVariable Name: %s\n"  
        "Address Begin: 0x%lx\n"  
        "Address End: 0x%lx\n"  
        "Variable Size: %lu bytes\n"  
        "Accessed: %lu times\n"
        "Policy configuration: %s\n" ,  
        info->function_name,
        info->variable_name,  
        info->address_begin,  
        info->address_end,  
        info->variable_size,  
        info->accessed,
        celue);  
      
    hthread_printf("%s", buffer);  
    hthread_printf("Sorted Memory Steps and Proportions:\n");
    for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
        //if (info->sorted_steps[i] != 0) {
            hthread_printf("Step: %lu, Proportion: %.8f\n", info->sorted_steps[i], info->sorted_pro[i]);
        //}
    }
}
