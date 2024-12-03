#include "hthread_device.h"
#include <limits.h>
#include <compiler/m3000.h>
#include "util.h"

typedef unsigned long addr;

typedef lvector unsigned long addr_v;

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

#define CACHEe_ENV_smid()                         \
    unsigned long de_point_p;                     \
    addr_v de_Vp;                                 \
    addr_v de_Vequl;                              \
    char *de_EaAggre, *de_EaWriteback;            \
    addr_v de_Valign;                             \
    addr de_align;                                \
    int _cachee_miss = 0;                         \
    char *_cachee_EaAlign;                        \
    int de_i, de_j;                               \
    unsigned long de_pways;                       \
    unsigned long de_psets;                       \
    unsigned long de_sets_index = 0;              \
    unsigned long de_ways_index = 0;              \
    lvector unsigned long de_Vzero = vec_movi(0); \
    addr_v de_VEas;                               \
    unsigned long de_tmp;

// 组数、路数、块大小,__csets组，每组__cways路，每路__cline个__type
#define CACHEe_INIT_simd(__name, __type, __csets, __cways, __cline, __unit_num, __index, __Ea, __size)               \
    unsigned long de_datasize_##__name = __unit_num * sizeof(__type);                                                \
    int DMA_no_##__name = 0;                                                                                         \
    int de_csets_##__name = __csets;                                                                                 \
    int de_cways_##__name = 4;                                                                                       \
    int de_cline_##__name = __cline;                                                                                 \
    int de_csets_num_##__name = 1 << de_csets_##__name;                                                              \
    int de_cways_num_##__name = 1 << de_cways_##__name;                                                              \
    int de_cline_num_##__name = 1 << de_cline_##__name;                                                              \
    char *de_buffer_##__name = scalar_malloc(de_csets_num_##__name * de_cways_num_##__name * de_cline_num_##__name); \
    lvector unsigned long de_VlineMask_##__name = vec_svbcast(ULONG_MAX << de_cline_##__name);                       \
    unsigned long de_lineMask_##__name = ULONG_MAX << de_cline_##__name;                                             \
    lvector unsigned long de_VindexMask_##__name = (1 << de_cways_num_##__name) - 1;                                 \
    unsigned long de_lineSize_##__name = (1 << de_cline_##__name);                                                   \
    \ 
    addr_v de_VwayAddrInMem_##__name[de_csets_num_##__name];                                                         \
    int de_freeWayIndex_##__name[de_csets_num_##__name];                                                             \
    int de_way0_##__name = 0;                                                                                        \
    __type *de_temp_##__name;                                                                                        \
    addr_v de_VwayAddrInBuffer_##__name[de_csets_num_##__name];                                                      \
    char *src_##__name = 0;                                                                                          \
    for (de_i = 0; de_i < de_csets_num_##__name; de_i++)                                                             \
    {                                                                                                                \
        src_##__name = (char *)&de_buffer_##__name[de_i * (1 << (de_cways_##__name + de_cline_##__name))];           \
        mov_to_svr0((unsigned long)(src_##__name + 0 * (de_cline_num_##__name)));                                    \
        mov_to_svr1((unsigned long)(src_##__name + 1 * (de_cline_num_##__name)));                                    \
        mov_to_svr2((unsigned long)(src_##__name + 2 * (de_cline_num_##__name)));                                    \
        mov_to_svr3((unsigned long)(src_##__name + 3 * (de_cline_num_##__name)));                                    \
        mov_to_svr4((unsigned long)(src_##__name + 4 * (de_cline_num_##__name)));                                    \
        mov_to_svr5((unsigned long)(src_##__name + 5 * (de_cline_num_##__name)));                                    \
        mov_to_svr6((unsigned long)(src_##__name + 6 * (de_cline_num_##__name)));                                    \
        mov_to_svr7((unsigned long)(src_##__name + 7 * (de_cline_num_##__name)));                                    \
        mov_to_svr8((unsigned long)(src_##__name + 8 * (de_cline_num_##__name)));                                    \
        mov_to_svr9((unsigned long)(src_##__name + 9 * (de_cline_num_##__name)));                                    \
        mov_to_svr10((unsigned long)(src_##__name + 10 * (de_cline_num_##__name)));                                  \
        mov_to_svr11((unsigned long)(src_##__name + 11 * (de_cline_num_##__name)));                                  \
        mov_to_svr12((unsigned long)(src_##__name + 12 * (de_cline_num_##__name)));                                  \
        mov_to_svr13((unsigned long)(src_##__name + 13 * (de_cline_num_##__name)));                                  \
        mov_to_svr14((unsigned long)(src_##__name + 14 * (de_cline_num_##__name)));                                  \
        mov_to_svr15((unsigned long)(src_##__name + 15 * (de_cline_num_##__name)));                                  \
        de_VwayAddrInBuffer_##__name[de_i] = mov_from_svr_v16di();                                                   \
        de_VwayAddrInMem_##__name[de_i] = vec_svbcast(ULONG_MAX);                                                    \
        de_freeWayIndex_##__name[de_i] = 0;                                                                          \
    }

#define CACHEe_SEC_R_RD_simd(__name, __addr, __value, __type)                                              \
    de_point_p = (unsigned long)((__type *)__addr);                                                        \
    de_sets_index = (de_point_p >> (de_cline_##__name + de_cways_##__name)) & (de_csets_num_##__name - 1); \
    de_align = de_point_p & de_lineMask_##__name;                                                          \
    de_Valign = vec_svbcast(de_align);                                                                     \
    de_Vequl = vec_eq(de_Valign, de_VwayAddrInMem_##__name[de_sets_index]);                                \
    de_VEas = vec_vmovc(de_Vequl, de_VwayAddrInBuffer_##__name[de_sets_index], de_Vzero);                  \
    de_pways = 0;                                                                                          \
    de_tmp = 0;                                                                                            \
    mov_to_svr_v16di(de_VEas);                                                                             \
    de_tmp = mov_from_svr0();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr1();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr2();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr3();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr4();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr5();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr6();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr7();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr8();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr9();                                                                              \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr10();                                                                             \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr11();                                                                             \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr12();                                                                             \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr13();                                                                             \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr14();                                                                             \
    de_pways += de_tmp;                                                                                    \
    de_tmp = mov_from_svr15();                                                                             \
    de_pways += de_tmp;                                                                                    \
                                                                                                           \
    if (de_pways == 0)                                                                                     \
    {                                                                                                      \
        de_psets = de_sets_index << (de_cways_##__name + de_cline_##__name);                               \
        _cachee_miss++;                                                                                    \
        de_way0_##__name = de_freeWayIndex_##__name[de_sets_index];                                        \
        de_pways = de_buffer_##__name + de_psets + (de_way0_##__name << de_cline_##__name);                \
        scalar_load((void *)de_align, de_pways, de_lineSize_##__name);                                     \
        WRITE_V_Idx(de_VwayAddrInMem_##__name[de_sets_index], de_way0_##__name, de_align);                 \
        de_freeWayIndex_##__name[de_sets_index] = (de_freeWayIndex_##__name[de_sets_index] + 1) & 0xE;     \
    }                                                                                                      \
    de_temp_##__name = de_pways + (de_point_p - de_align);                                                 \
    __value = *(__type *)(de_temp_##__name);

#define CACHEe_INVALID_simd(__name, __Ea, __size) \
    scalar_free(de_buffer_##__name);