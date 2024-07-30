#include <stdio.h>
#include <compiler/m3000.h>
#include "hthread_device.h"

unsigned long interface_sizes;      // 接口处理的数据块的总大小（包含对齐后的长度）
unsigned long interface_dma_count;  // 用于记录分批次数据传输的次数
unsigned long interface_Ea_align;   // 对齐后的有效地址
unsigned long interface_dma_offset; // DMA偏移量
unsigned long interface_original;   // 用于记录原始有效地址
char *interface_buffer;             // 接口处理的数据块的缓冲区

// 批量数据传输 - 读
__global__ void interface_read_kernel(void **Ea, unsigned long size) // 有效地址，数据块大小
{
#ifdef DEBUG
    hthread_printf("======================\n");
    hthread_printf("thread %u on core %u.\n", get_thread_id(), get_core_id());
    hthread_printf("SM space before alloc: %d.\n", get_sm_free_space());
#endif

    unsigned int channel_number = 0;
    // 最大申请60KB标量缓存
    interface_buffer = (char *)scalar_malloc(size + 64); // 批量数据传输读入的缓冲区，加64是为了给对齐留出空间
    // char interface_buffer[size + 64];

    interface_Ea_align = (unsigned long)*Ea & 0xFFFFFFFFFFFFFFE0;                  // 将地址按64位对齐
    interface_dma_offset = (unsigned long)*Ea - (unsigned long)interface_Ea_align; // 计算未对齐部分的偏移量
    interface_sizes = ((size + interface_dma_offset) & 0xFFFFFFFFFFFFFFE0) + 64;   // 计算需要传输的总大小，包含未对齐的部分
    // interface_dma_count = 0;                                                       // 初始化传输次数

#ifdef DEBUG
    hthread_printf("Ea align: %p\nDMA offset: %ld\nsize: %ld\nbuffer address: %p\n", (void *)interface_Ea_align, interface_dma_offset, interface_sizes, interface_buffer);
    hthread_printf("Ea before cache read: %p\n", *Ea);
#endif

    channel_number = dma_p2p((void *)interface_Ea_align, 1, interface_sizes, 0, interface_buffer, 1, interface_sizes, 0, false, 0);
    dma_wait(channel_number);
    interface_original = (unsigned long)*Ea;                                // 存储原有效地址
    *Ea = (void *)((unsigned long)interface_buffer + interface_dma_offset); // 将有效地址转为指向SM缓冲区

#ifdef DEBUG
    hthread_printf("Ea after cache read: %p\n", *Ea);
    hthread_printf("SM space after alloc: %d.\n", get_sm_free_space());
    hthread_printf("======================\n");
#endif
}

// 批量数据传输 - 写
__global__ void interface_write_kernel(void **Ea, unsigned long size) // 有效地址，数据块大小
{
#ifdef DEBUG
    hthread_printf("======================\n");
    hthread_printf("thread %u on core %u.\n", get_thread_id(), get_core_id());
    hthread_printf("SM space before alloc: %d.\n", get_sm_free_space());
#endif

    unsigned int channel_number = 0;
    interface_sizes = ((size + interface_dma_offset) & 0xFFFFFFFFFFFFFFE0) + 64; // 计算需要传输的总大小，包含未对齐的部分

#ifdef DEBUG
    hthread_printf("Ea align: %p\nDMA offset: %ld\nsize: %ld\nbuffer address: %p\n", (void *)interface_Ea_align, interface_dma_offset, interface_sizes, interface_buffer);
    hthread_printf("Ea before cache read: %p\n", *Ea);
#endif

    channel_number = dma_p2p(interface_buffer, 1, interface_sizes, 0, (void *)interface_Ea_align, 1, interface_sizes, 0, false, 0);
    dma_wait(channel_number);         // 将buffer内的数据写回主存
    *Ea = (void *)interface_original; // 把有效地址更改为主存地址

#ifdef DEBUG
    hthread_printf("Ea after cache read: %p\n", *Ea);
    hthread_printf("SM space after alloc: %d.\n", get_sm_free_space());
    hthread_printf("======================\n");
#endif
}

// 批量数据传输 - 初始化
__global__ void interface_init_kernel(char* name, int csets, int cways, int cline, int unit_num, int index, int tag, void** Ea, unsigned long size)
{
    interface_read_kernel(Ea, size);
}

// 批量数据传输 - 刷缓存
__global__ void interface_flush_kernel(char* name, void **Ea, unsigned long size) // 有效地址，数据块大小
{
    interface_write_kernel(Ea, size);
}