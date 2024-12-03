#include "hthread_device.h"
#include <limits.h>

/**
 * char *de_point_p;：指向当前处理的数据地址。
 * char *de_EaAggre, *de_EaWriteback;：用于聚合和写回数据的地址。
 * vector unsigned long de_Zero = {0, 0, 0, 0};：初始化为0的向量，用于向量操作。
 * vector unsigned long de_p, de_align;：用于存储地址对齐计算的向量。
 * vector unsigned long de_VEas = {0, 0, 0, 0};：用于存储每组的有效地址向量。
 * vector unsigned long de_Vbig;：用于向量比较操作。
 * int _cachee_miss = 0;：用于记录cache未命中的次数。
 * char *_cachee_EaAlign;：对齐后的有效地址。
 * int de_i, de_j;：循环变量。
 * int de_DMA_tag = 31;：用于DMA操作的标签。
 * int de_pways;：用于存储当前组中匹配的路数。
 * int de_int;：通用整数变量。
 * unsigned long de_psets;：当前组的起始地址。
 * unsigned long de_sets_index = 0;：当前组的编号。
 */

#define CACHEe_ENV()                   \
    unsigned long de_point_p;          \
    char *de_EaAggre, *de_EaWriteback; \
    unsigned long de_align;            \
    int _cachee_miss = 0;              \
    char *_cachee_EaAlign;             \
    int de_i, de_j;                    \
    unsigned long de_pways;            \
    unsigned long de_psets;            \
    unsigned long de_sets_index = 0;   \
    unsigned long de_ways_index = 0

// 组数、路数、块大小,__csets组，每组__cways路，每路__cline个__type
#define CACHEe_INIT(__name, __type, __csets, __cways, __cline, __unit_num, __index, __Ea, __size)                                                                                 \
    unsigned long de_datasize_##__name = __unit_num * sizeof(__type);                                                                                                             \
    int DMA_no_##__name = 0;                                                                                                                                                      \
    int de_csets_##__name = __csets;                                                                                                                                              \
    int de_cways_##__name = __cways;                                                                                                                                              \
    int de_cline_##__name = __cline;                                                                                                                                              \
    int de_csets_num_##__name = 1 << de_csets_##__name;                                                                                                                           \
    int de_cways_num_##__name = 1 << de_cways_##__name;                                                                                                                           \
    int de_cline_num_##__name = 1 << de_cline_##__name;                                                                                                                           \
    char *de_buffer_##__name = scalar_malloc(de_csets_num_##__name * de_cways_num_##__name * de_cline_num_##__name);                                                              \
    unsigned long de_lineMask_##__name = ULONG_MAX << de_cline_##__name;                                                                                                          \
    unsigned long de_indexMask_##__name = (1 << de_cways_num_##__name) - 1;                                                                                                       \
    unsigned long de_lineSize_##__name = (1 << de_cline_##__name);                                                                                                                \
    \ 
    unsigned long de_wayAddrInMem_##__name[de_csets_num_##__name][de_cways_num_##__name];                                                                                         \
    int de_freeWayIndex_##__name[de_csets_num_##__name];                                                                                                                          \
    int de_way0_##__name = 0;                                                                                                                                                     \
    __type *de_temp_##__name;                                                                                                                                                     \
    unsigned long de_wayAddrInBuffer_##__name[de_csets_num_##__name][de_cways_num_##__name];                                                                                      \
    for (de_i = 0; de_i < de_csets_num_##__name; de_i++)                                                                                                                          \
    {                                                                                                                                                                             \
        for (de_j = 0; de_j < de_cways_num_##__name; de_j++)                                                                                                                      \
        {                                                                                                                                                                         \
            de_wayAddrInBuffer_##__name[de_i][de_j] = (unsigned long)&de_buffer_##__name[de_i * (1 << (de_cways_##__name + de_cline_##__name)) + de_j * (de_cline_num_##__name)]; \
            de_wayAddrInMem_##__name[de_i][de_j] = ULONG_MAX;                                                                                                                     \
            de_freeWayIndex_##__name[de_i] = 0;                                                                                                                                   \
        }                                                                                                                                                                         \
    }                                                                                                                                                                             \
    // for (de_i = 0; de_i < de_csets_num_##__name; de_i++)                                                                                                                          \
    // {                                                                                                                                                                             \
    //     hthread_printf("%d ", de_freeWayIndex_##__name[de_i]);                                                                                                                    \
    // }                                                                                                                                                                             \
    // hthread_printf("\n");                                                                                                                                                         \
    // hthread_printf("de_datasize_: %lu\n", de_datasize_##__name);                                                                                                                  \
    // hthread_printf("DMA_no: %d\n", DMA_no_##__name);                                                                                                                              \
    // hthread_printf("de_csets: %d\n", de_csets_##__name);                                                                                                                          \
    // hthread_printf("de_cways: %d\n", de_cways_##__name);                                                                                                                          \
    // hthread_printf("de_cline: %d\n", de_cline_##__name);                                                                                                                          \
    // hthread_printf("de_csets_num: %d\n", de_csets_num_##__name);                                                                                                                  \
    // hthread_printf("de_cways_num: %d\n", de_cways_num_##__name);                                                                                                                  \
    // hthread_printf("de_cline_num: %d\n", de_cline_num_##__name);                                                                                                                  \
    // hthread_printf("de_mask: %lx\n", de_lineMask_##__name);                                                                                                                           \
    // hthread_printf("de_lineSize: %lu\n", de_lineSize_##__name);                                                                                                                   \
    // hthread_printf("de_buffer_##__name: %p\n", de_buffer_##__name);                                                                                                               \
    // hthread_printf("de_wayAddrInMem_##__name: %p\n", de_wayAddrInMem_##__name);                                                                                                   \
    // hthread_printf("de_freeWayIndex_##__name: %p\n", de_freeWayIndex_##__name);                                                                                                   \
    // hthread_printf("de_wayAddrInBuffer_##__name: %p\n", de_wayAddrInBuffer_##__name);

#define CACHEe_INIT_DEBUG(__name, __type, __csets, __cways, __cline, __unit_num, __index, __Ea, __size)                                                                           \
    unsigned long de_datasize_##__name = __unit_num * sizeof(__type);                                                                                                             \
    int DMA_no_##__name = 0;                                                                                                                                                      \
    int de_csets_##__name = __csets;                                                                                                                                              \
    int de_cways_##__name = __cways;                                                                                                                                              \
    int de_cline_##__name = __cline;                                                                                                                                              \
    int de_csets_num_##__name = 1 << de_csets_##__name;                                                                                                                           \
    int de_cways_num_##__name = 1 << de_cways_##__name;                                                                                                                           \
    int de_cline_num_##__name = 1 << de_cline_##__name;                                                                                                                           \
    char *de_buffer_##__name = scalar_malloc(de_csets_num_##__name * de_cways_num_##__name * de_cline_num_##__name);                                                              \
    unsigned long de_lineMask_##__name = ULONG_MAX << de_cline_##__name;                                                                                                          \
    unsigned long de_indexMask_##__name = (1 << de_cways_num_##__name) - 1;                                                                                                       \
    unsigned long de_lineSize_##__name = (1 << de_cline_##__name);                                                                                                                \
    unsigned long de_step_clocks_##__name[16] = {0};                                                                                                                              \
    unsigned long total_miss_time_##__name = 0;                                                                                                                                   \
    unsigned long total_required_time_##__name = 0;                                                                                                                               \
    unsigned long de_wayAddrInMem_##__name[de_csets_num_##__name][de_cways_num_##__name];                                                                                         \
    int de_freeWayIndex_##__name[de_csets_num_##__name];                                                                                                                          \
    int de_way0_##__name = 0;                                                                                                                                                     \
    __type *de_temp_##__name;                                                                                                                                                     \
    unsigned long de_wayAddrInBuffer_##__name[de_csets_num_##__name][de_cways_num_##__name];                                                                                      \
    for (int de_i = 0; de_i < de_csets_num_##__name; de_i++)                                                                                                                      \
    {                                                                                                                                                                             \
        for (int de_j = 0; de_j < de_cways_num_##__name; de_j++)                                                                                                                  \
        {                                                                                                                                                                         \
            de_wayAddrInBuffer_##__name[de_i][de_j] = (unsigned long)&de_buffer_##__name[de_i * (1 << (de_cways_##__name + de_cline_##__name)) + de_j * (de_cline_num_##__name)]; \
            de_wayAddrInMem_##__name[de_i][de_j] = ULONG_MAX;                                                                                                                     \
            de_freeWayIndex_##__name[de_i] = 0;                                                                                                                                   \
        }                                                                                                                                                                         \
    }

// #define CACHEe_SEC_R_RD(__name, __addr, __value, __type)                                                                 \
//     de_point_p = (unsigned long)((__type *)__addr);                                                                      \
//     de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1);               \
//     de_ways_index = (de_point_p >> (de_cline_##__name)) & (de_cways_num_##__name - 1);                                   \
//     de_align = (unsigned long)de_point_p & de_lineMask_##__name;                                                         \
//     de_pways = 0;                                                                                                        \
//     for (de_i = 0; de_i < de_cways_num_##__name; de_i++)                                                                 \
//     {                                                                                                                    \
//         if (de_wayAddrInMem_##__name[de_sets_index][de_i] == de_align)                                                   \
//         {                                                                                                                \
//             de_pways = de_wayAddrInBuffer_##__name[de_sets_index][de_i];                                                 \
//             break;                                                                                                       \
//         }                                                                                                                \
//     }                                                                                                                    \
//     if (de_pways == 0)                                                                                                   \
//     {                                                                                                                    \
//         de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                                             \
//         _cachee_miss++;                                                                                                  \
//         de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                                      \
//         de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                              \
//         DMA_no_##__name = dma_p2p(de_align, 1, de_lineSize_##__name, 0, de_pways, 1, de_lineSize_##__name, 0, false, 0); \
//         dma_wait(DMA_no_##__name);                                                                                       \
//         de_wayAddrInMem_##__name[de_sets_index][de_ways_index] = de_align;                                               \
//         de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) & de_indexMask_##__name; \
//     }                                                                                                                    \
//     de_temp_##__name = de_pways + (de_point_p - de_align);                                                               \
//     __value = *(__type *)(de_temp_##__name);

#define CACHEe_SEC_R_RD(__name, __addr, __value, __type)                                                                 \
    de_point_p = (unsigned long)((__type *)__addr);                                                                      \
    de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1);               \
    de_align = (unsigned long)de_point_p & de_lineMask_##__name;                                                         \
    de_pways = 0;                                                                                                        \
    for (de_i = 0; de_i < de_cways_num_##__name; de_i++)                                                                 \
    {                                                                                                                    \
        if (de_wayAddrInMem_##__name[de_sets_index][de_i] == de_align)                                                   \
        {                                                                                                                \
            de_pways = de_wayAddrInBuffer_##__name[de_sets_index][de_i];                                                 \
            break;                                                                                                       \
        }                                                                                                                \
    }                                                                                                                    \
    if (de_pways == 0)                                                                                                   \
    {                                                                                                                    \
        de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                                             \
        _cachee_miss++;                                                                                                  \
        de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                                      \
        de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                              \
        scalar_load(de_align, de_pways, de_lineSize_##__name);                                                           \
        de_wayAddrInMem_##__name[de_sets_index][de_way0_##__name] = de_align;                                            \
        de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) & de_indexMask_##__name; \
    }                                                                                                                    \
    de_temp_##__name = de_pways + (de_point_p - de_align);                                                               \
    __value = *(__type *)(de_temp_##__name);

// 改进主存地址划分逻辑
#define CACHEe_SEC_R_RD_2(__name, __addr, __value, __type)                                                                 \
    de_point_p = (unsigned long)((__type *)__addr);                                                                      \
    de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1);               \
    de_align = (unsigned long)de_point_p & de_lineMask_##__name;                                                         \
    de_pways = 0;                                                                                                        \
    for (de_i = 0; de_i < de_cways_num_##__name; de_i++)                                                                 \
    {                                                                                                                    \
        if (de_wayAddrInMem_##__name[de_sets_index][de_i] == de_align)                                                   \
        {                                                                                                                \
            de_pways = de_wayAddrInBuffer_##__name[de_sets_index][de_i];                                                 \
            break;                                                                                                       \
        }                                                                                                                \
    }                                                                                                                    \
    if (de_pways == 0)                                                                                                   \
    {                                                                                                                    \
        de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                                             \
        _cachee_miss++;                                                                                                  \
        de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                                      \
        de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                              \
        scalar_load(de_align, de_pways, de_lineSize_##__name);                                                           \
        de_wayAddrInMem_##__name[de_sets_index][de_way0_##__name] = de_align;                                            \
        de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) & de_indexMask_##__name; \
    }                                                                                                                    \
    de_temp_##__name = de_pways + (de_point_p - de_align);                                                               \
    __value = *(__type *)(de_temp_##__name);

#define CACHEe_SEC_R_RD_DEBUG_SUM(__name, __addr, __value, __type)                                                                                                                                                                                                          \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    de_point_p = (unsigned long)((__type *)__addr);                                                                                                                                                                                                                         \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[0] += clk_end - clk_start;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1);                                                                                                                                                                  \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[1] += clk_end - clk_start;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    de_ways_index = (de_point_p >> (de_cline_##__name)) & (de_cways_num_##__name - 1);                                                                                                                                                                                      \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[2] += clk_end - clk_start;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    de_align = (unsigned long)de_point_p & de_lineMask_##__name;                                                                                                                                                                                                            \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[3] += clk_end - clk_start;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    de_pways = 0;                                                                                                                                                                                                                                                           \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[4] += clk_end - clk_start;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    for (int de_i = 0; de_i < de_cways_num_##__name; de_i++)                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                                       \
        if (de_wayAddrInMem_##__name[de_sets_index][de_i] == de_align)                                                                                                                                                                                                      \
        {                                                                                                                                                                                                                                                                   \
            de_pways = de_wayAddrInBuffer_##__name[de_sets_index][de_i];                                                                                                                                                                                                    \
            break;                                                                                                                                                                                                                                                          \
        }                                                                                                                                                                                                                                                                   \
    }                                                                                                                                                                                                                                                                       \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[5] += clk_end - clk_start;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                                            \
    if (de_pways == 0)                                                                                                                                                                                                                                                      \
    {                                                                                                                                                                                                                                                                       \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                                                                                                                                                                                                \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[6] += clk_end - clk_start;                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                            \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        _cachee_miss++;                                                                                                                                                                                                                                                     \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[7] += clk_end - clk_start;                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                            \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                                                                                                                                                                                         \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[8] += clk_end - clk_start;                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                            \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                                                                                                                                                                                 \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[9] += clk_end - clk_start;                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                                            \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        DMA_no_##__name = dma_p2p(de_align, 1, de_lineSize_##__name, 0, de_pways, 1, de_lineSize_##__name, 0, false, 0);                                                                                                                                                    \
        dma_wait(DMA_no_##__name);                                                                                                                                                                                                                                          \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[10] += clk_end - clk_start;                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                                                            \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        de_wayAddrInMem_##__name[de_sets_index][de_ways_index] = de_align;                                                                                                                                                                                                  \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[11] += clk_end - clk_start;                                                                                                                                                                                                                 \
                                                                                                                                                                                                                                                                            \
        clk_start = get_clk();                                                                                                                                                                                                                                              \
        de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) & de_indexMask_##__name;                                                                                                                                                    \
        clk_end = get_clk();                                                                                                                                                                                                                                                \
        de_step_clocks_##__name[12] += clk_end - clk_start;                                                                                                                                                                                                                 \
    }                                                                                                                                                                                                                                                                       \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    de_temp_##__name = de_pways + (de_point_p - de_align);                                                                                                                                                                                                                  \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[13] += clk_end - clk_start;                                                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                            \
    clk_start = get_clk();                                                                                                                                                                                                                                                  \
    __value = *(__type *)(de_temp_##__name);                                                                                                                                                                                                                                \
    clk_end = get_clk();                                                                                                                                                                                                                                                    \
    de_step_clocks_##__name[14] += clk_end - clk_start;                                                                                                                                                                                                                     \
                                                                                                                                                                                                                                                                            \
    total_required_time_##__name = de_step_clocks_##__name[0] + de_step_clocks_##__name[1] + de_step_clocks_##__name[2] + de_step_clocks_##__name[3] + de_step_clocks_##__name[4] + de_step_clocks_##__name[5] + de_step_clocks_##__name[13] + de_step_clocks_##__name[14]; \
    total_miss_time_##__name = de_step_clocks_##__name[6] + de_step_clocks_##__name[7] + de_step_clocks_##__name[8] + de_step_clocks_##__name[9] + de_step_clocks_##__name[10] + de_step_clocks_##__name[11] + de_step_clocks_##__name[12];

#define PRINT_HEADER() \
    hthread_printf("n,step1,step2,step3,step4,step5,step6,step7,step8,step9,step10,step11,step12,step13,step14,step15,miss_time,required_time\n");

#define PRINT_DATA(__name, n)                               \
    hthread_printf("%d", n);                                \
    for (int i = 0; i < 15; ++i)                            \
    {                                                       \
        hthread_printf(",%lu", de_step_clocks_##__name[i]); \
    }                                                       \
    hthread_printf(",%lu,%lu\n", total_miss_time_##__name, total_required_time_##__name);

unsigned long clk_start, clk_end, clk_diff;
unsigned long total_miss_time, total_time;
#define CACHEe_SEC_R_RD_DEBUG(__name, __addr, __value, __type)                                                                                           \
    hthread_printf("========== CACHEe_SEC_R_RD Start ==========\n");                                                                                     \
    total_miss_time = 0;                                                                                                                                 \
    total_time = 0;                                                                                                                                      \
    clk_start = get_clk();                                                                                                                               \
    de_point_p = (unsigned long)((__type *)__addr);                                                                                                      \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 1: de_point_p = (unsigned long)((__type *)__addr); Clocks: %lu\n", clk_diff);                                                   \
    hthread_printf("    __addr = %p, de_point_p = 0x%lx\n", __addr, de_point_p);                                                                         \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1);                                               \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 2: de_sets_index calculation; Clocks: %lu\n", clk_diff);                                                                        \
    hthread_printf("    de_sets_index = %u\n", de_sets_index);                                                                                           \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    de_ways_index = (de_point_p >> (de_cline_##__name)) & (de_cways_num_##__name - 1);                                                                   \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 3: de_ways_index calculation; Clocks: %lu\n", clk_diff);                                                                        \
    hthread_printf("    de_ways_index = %u\n", de_ways_index);                                                                                           \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    de_align = (unsigned long)de_point_p & de_lineMask_##__name;                                                                                         \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 4: de_align calculation; Clocks: %lu\n", clk_diff);                                                                             \
    hthread_printf("    de_align = 0x%lx\n", de_align);                                                                                                  \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    de_pways = 0;                                                                                                                                        \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 5: de_pways initialization; Clocks: %lu\n", clk_diff);                                                                          \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    for (de_i = 0; de_i < de_cways_num_##__name; de_i++)                                                                                                 \
    {                                                                                                                                                    \
        if (de_wayAddrInMem_##__name[de_sets_index][de_i] == de_align)                                                                                   \
        {                                                                                                                                                \
            de_pways = de_wayAddrInBuffer_##__name[de_sets_index][de_i];                                                                                 \
            break;                                                                                                                                       \
        }                                                                                                                                                \
    }                                                                                                                                                    \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 6: Loop and check; Clocks: %lu\n", clk_diff);                                                                                   \
    hthread_printf("    de_pways = 0x%lx\n", de_pways);                                                                                                  \
                                                                                                                                                         \
    if (de_pways == 0)                                                                                                                                   \
    {                                                                                                                                                    \
        hthread_printf("*******!!!!!cache miss!!!!!*******\n");                                                                                          \
        clk_start = get_clk();                                                                                                                           \
        de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                                                                             \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 7: de_psets calculation; Clocks: %lu\n", clk_diff);                                                                         \
        hthread_printf("    de_psets = 0x%lx\n", de_psets);                                                                                              \
                                                                                                                                                         \
        clk_start = get_clk();                                                                                                                           \
        _cachee_miss++;                                                                                                                                  \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 8: _cachee_miss increment; Clocks: %lu\n", clk_diff);                                                                       \
                                                                                                                                                         \
        clk_start = get_clk();                                                                                                                           \
        de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                                                                      \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 9: de_way0_##__name assignment; Clocks: %lu\n", clk_diff);                                                                  \
        hthread_printf("    de_way0_" #__name " = %u\n", de_way0_##__name);                                                                              \
                                                                                                                                                         \
        clk_start = get_clk();                                                                                                                           \
        de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                                                              \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 10: de_pways calculation; Clocks: %lu\n", clk_diff);                                                                        \
        hthread_printf("    de_pways = 0x%lx\n", de_pways);                                                                                              \
                                                                                                                                                         \
        clk_start = get_clk();                                                                                                                           \
        DMA_no_##__name = dma_p2p(de_align, 1, de_lineSize_##__name, 0, de_pways, 1, de_lineSize_##__name, 0, false, 0);                                 \
        dma_wait(DMA_no_##__name);                                                                                                                       \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 11: DMA_no_##__name calculation and dma_wait; Clocks: %lu\n", clk_diff);                                                    \
                                                                                                                                                         \
        clk_start = get_clk();                                                                                                                           \
        de_wayAddrInMem_##__name[de_sets_index][de_ways_index] = de_align;                                                                               \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 12: de_wayAddrInMem_##__name assignment; Clocks: %lu\n", clk_diff);                                                         \
                                                                                                                                                         \
        clk_start = get_clk();                                                                                                                           \
        de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) & de_indexMask_##__name;                                 \
        clk_end = get_clk();                                                                                                                             \
        clk_diff = clk_end - clk_start;                                                                                                                  \
        total_miss_time += clk_diff;                                                                                                                     \
        hthread_printf("Step 13: de_freeWayIndex_##__name update; Clocks: %lu\n", clk_diff);                                                             \
        hthread_printf("*******!!!!!cache miss!!!!!*******\n");                                                                                          \
    }                                                                                                                                                    \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    de_temp_##__name = de_pways + (de_point_p - de_align);                                                                                               \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 14: de_temp_##__name calculation; Clocks: %lu\n", clk_diff);                                                                    \
    hthread_printf("    de_temp_" #__name " = 0x%lx\n", de_temp_##__name);                                                                               \
                                                                                                                                                         \
    clk_start = get_clk();                                                                                                                               \
    __value = *(__type *)(de_temp_##__name);                                                                                                             \
    clk_end = get_clk();                                                                                                                                 \
    clk_diff = clk_end - clk_start;                                                                                                                      \
    total_time += clk_diff;                                                                                                                              \
    hthread_printf("Step 15: __value assignment; Clocks: %lu\n", clk_diff);                                                                              \
    hthread_printf("    __addr value = %p, __type value = %d\n", __addr, *(__type *)__addr);                                                             \
    hthread_printf("    de_temp_" #__name " value = %p, __type value = %d\n", (void *)de_temp_##__name, *(__type *)de_temp_##__name);                    \
    hthread_printf("Required processing time: %lu, %lf%%\n", total_time, (double)total_time / (total_time + total_miss_time));                           \
    hthread_printf("Processing time introduced by cache miss: %lu, %lf%%\n", total_miss_time, (double)total_miss_time / (total_time + total_miss_time)); \
    hthread_printf("========== CACHEe_SEC_R_RD End ==========\n");

// #define CACHEe_SEC_R_RD(__name, __addr, __value, __type)                                                                        \
//     de_point_p = (unsigned long)((__type *)__addr);                                                                             \
//     de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1);                      \
//     de_ways_index = (de_point_p >> (de_cline_##__name)) & (de_cways_num_##__name - 1);                                          \
//     de_align = (unsigned long)de_point_p & de_lineMask_##__name;                                                                    \
//     hthread_printf("+++++++++++++++++++++\n");                                                                                  \
//     hthread_printf("de_point_p: %lx\n", de_point_p);                                                                            \
//     hthread_printf("value of __Ea: %lf\n", *(__type *)(de_point_p));                                                            \
//     hthread_printf("de_sets_index: %lu\n", de_sets_index);                                                                      \
//     hthread_printf("de_ways_index: %lu\n", de_ways_index);                                                                      \
//     hthread_printf("de_align: %lx\n", de_align);                                                                                \
//     de_pways = 0;                                                                                                               \
//     for (de_i = 0; de_i < de_cways_num_##__name; de_i++)                                                                        \
//     {                                                                                                                           \
//         if (de_wayAddrInMem_##__name[de_sets_index][de_i] == de_align)                                                          \
//         {                                                                                                                       \
//             de_pways = de_wayAddrInBuffer_##__name[de_sets_index][de_i];                                                        \
//             break;                                                                                                              \
//         }                                                                                                                       \
//     }                                                                                                                           \
//     if (de_pways == 0)                                                                                                          \
//     {                                                                                                                           \
//         de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                                                    \
//         _cachee_miss++;                                                                                                         \
//         de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                                             \
//         hthread_printf("_cachee_miss: %d\n", _cachee_miss);                                                                     \
//         hthread_printf("de_way0: %d\n", de_way0_##__name);                                                                      \
//         hthread_printf("de_psets: %lu\n", de_psets);                                                                            \
//         de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                                     \
//         hthread_printf("de_pways: %lx\n", de_pways);                                                                            \
//         hthread_printf("de_buffer: %p\n", de_buffer_##__name);                                                                  \
//         DMA_no_##__name = dma_p2p(de_align, 1, de_lineSize_##__name, 0, de_pways, 1, de_lineSize_##__name, 0, false, 0xffffff); \
//         dma_wait(DMA_no_##__name);                                                                                              \
//         de_wayAddrInMem_##__name[de_sets_index][de_ways_index] = de_align;                                                      \
//         de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) % de_cways_num_##__name;        \
//     }                                                                                                                           \
//     de_temp_##__name = de_pways + (de_point_p - de_align);                                                                      \
//     hthread_printf("de_temp: %p\n", de_temp_##__name);                                                                          \
//     __value = *(__type *)(de_temp_##__name);                                                                                    \
//     hthread_printf("__value: %lf\n", __value);                                                                                  \
//     hthread_printf("+++++++++++++++++++++\n");

#define CACHEe_INVALID(__name, __Ea, __size) \
    scalar_free(de_buffer_##__name);