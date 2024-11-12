#define MEMORY_PRO_MAX_NUM 1024
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include "hthread_device.h"
typedef struct {
    char function_name[64];
    char variable_name[64];
    unsigned long address_begin;
    unsigned long address_end;
    unsigned long variable_size;
    unsigned long accessed;
    unsigned long memory_step[MEMORY_PRO_MAX_NUM];
    unsigned long memory_step_pro[MEMORY_PRO_MAX_NUM];
    unsigned long max_memory_step;
    float max_memory_step_pro;
} memory_info_t;
void print_memory_info(const memory_info_t *info) {

    char buffer[1024]; // 假设这个缓冲区足够大以容纳所有输出  
    snprintf(buffer, sizeof(buffer),  
        "\nVariable Name: %s\n"  
        "Address Begin: 0x%lx\n"  
        "Address End: 0x%lx\n"  
        "Variable Size: %lu bytes\n"  
        "Accessed: %lu times\n"  
        "Max Memory Step: %lu\n"  
        "Max Memory Step Proportion: %.8f\n"  
        "\n",  
        info->variable_name,  
        info->address_begin,  
        info->address_end,  
        info->variable_size,  
        info->accessed,  
        info->max_memory_step,  
        info->max_memory_step_pro);  
      
    printf("%s", buffer);  
    //printf("Memory Steps: ");
    //for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
    //    printf("%lu ", info->memory_step[i]);
    //}
    //printf("\n");

    //printf("Memory Step Proportions: ");
    //for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
    //    printf("%lu ", info->memory_step_pro[i]);
    //}
    //printf("\n");
}
void hprint_memory_info(const memory_info_t *info) {

    char buffer[1024]; // 假设这个缓冲区足够大以容纳所有输出  
    snprintf(buffer, sizeof(buffer), 
        "\nfunction_name: %s\n" 
        "\nVariable Name: %s\n"  
        "Address Begin: 0x%lx\n"  
        "Address End: 0x%lx\n"  
        "Variable Size: %lu bytes\n"  
        "Accessed: %lu times\n"  
        "Max Memory Step: %lu\n"  
        "Max Memory Step Proportion: %.8f\n"  
        "\n",  
        info->function_name,
        info->variable_name,  
        info->address_begin,  
        info->address_end,  
        info->variable_size,  
        info->accessed,  
        info->max_memory_step,  
        info->max_memory_step_pro);  
      
    hthread_printf("%s", buffer);  
    //printf("Memory Steps: ");
    //for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
    //    printf("%lu ", info->memory_step[i]);
    //}
    //printf("\n");

    //printf("Memory Step Proportions: ");
    //for (int i = 0; i < MEMORY_PRO_MAX_NUM; i++) {
    //    printf("%lu ", info->memory_step_pro[i]);
    //}
    //printf("\n");
}

static inline void start_instrumentation(const char* var_name, void* var_ptr, memory_info_t* mem_info) {
    strncpy(mem_info->variable_name, var_name, sizeof(mem_info->variable_name));
    mem_info->address_begin = (unsigned long)var_ptr;
    mem_info->accessed = 0;
    memset(mem_info->memory_step, 0, sizeof(mem_info->memory_step));
    memset(mem_info->memory_step_pro, 0, sizeof(mem_info->memory_step_pro));
}

static inline void record_memory_access(memory_info_t* mem_info, unsigned long current_addr) {
    unsigned long last_addr = mem_info->address_end == 0 ? mem_info->address_begin : mem_info->address_end;
    unsigned long step = current_addr - last_addr;
    mem_info->accessed++;
    mem_info->address_end = current_addr;

    if (step > 0) {
        int found = 0;
        for (int j = 0; j < MEMORY_PRO_MAX_NUM; j++) {
            if (mem_info->memory_step[j] == step) {
                mem_info->memory_step_pro[j]++;
                found = 1;
                break;
            }
            else if (mem_info->memory_step[j] == 0) {
                mem_info->memory_step[j] = step;
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
    mem_info->variable_size = mem_info->address_end - mem_info->address_begin;

    unsigned long max_count = 0;
    for (int j = 0; j < MEMORY_PRO_MAX_NUM; j++) {
        if (mem_info->memory_step_pro[j] > max_count) {
            max_count = mem_info->memory_step_pro[j];
            mem_info->max_memory_step = mem_info->memory_step[j];
        }
    }

    mem_info->max_memory_step_pro = (float)max_count / mem_info->accessed;
}

