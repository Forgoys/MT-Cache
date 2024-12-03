#include "hthread_device.h"

#define CACHEs_ENV()

#define CACHEs_INIT(__name, __type, __cets, __cways, __cline)           \
    unsigned long de_cline_##__name = __cline;                          \
    unsigned long de_cline_num_##__name = (1UL << de_cline_##__name);   \
    unsigned long de_make_mask_##__name = ~(de_cline_num_##__name - 1); \
    char *de_sm_buffer_##__name = scalar_malloc(de_cline_num_##__name); \
    if (!de_sm_buffer_##__name)                                         \
    {                                                                   \
        hthread_printf("Memory allocation failed\n");                  \
    }                                                                   \
    unsigned long de_memAddr_begine_##__name = 0;                       \
    unsigned long de_memAddr_end_##__name = 0;                          \
    char de_dirty_##__name = 0;                                         \
    unsigned long de_tmp_addr_##__name = 0;

// #define CACHEs_INIT(__name, __type, __cets, __cways, __cline, __base, __size)          \
//     unsigned long de_cline_##__name = __cline;                                         \
//     unsigned long de_cline_num_##__name = (1UL << de_cline_##__name);                  \
//     unsigned long de_make_mask_##__name = ~(de_cline_num_##__name - 1);                \
//     char *de_sm_buffer_##__name = scalar_malloc(de_cline_num_##__name);                \
//     if (!de_sm_buffer_##__name)                                                        \
//     {                                                                                  \
//         hthread_printf("内存分配失败\n");                                             \
//     }                                                                                  \
//     unsigned long de_memAddr_begine_##__name = 0;                                      \
//     unsigned long de_memAddr_end_##__name = 0;                                         \
//     char de_dirty_##__name = 0;                                                        \
//     unsigned long de_tmp_addr_##__name = 0;                                            \
//     unsigned long de_base_addr_##__name = (unsigned long)(__base);                     \
//     unsigned long de_array_size_##__name = (__size);

#define CACHEs_SEC_R_RD()

#define CACHEs_SEC_W_RD_K(__name, __addr, __value)                                                             \
    do                                                                                                         \
    {                                                                                                          \
        unsigned long de_mem_addr = (__addr);                                                                  \
        unsigned long de_mem_addr_align = de_mem_addr & de_make_mask_##__name;                                 \
        unsigned long de_sm_off = de_mem_addr - de_mem_addr_align;                                             \
        if (!(de_mem_addr_align >= de_memAddr_begine_##__name && de_mem_addr_align < de_memAddr_end_##__name)) \
        {                                                                                                      \
            if (de_dirty_##__name == 1)                                                                        \
            {                                                                                                  \
                scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_cline_num_##__name);        \
            }                                                                                                  \
            de_memAddr_begine_##__name = de_mem_addr_align;                                                    \
            de_memAddr_end_##__name = de_mem_addr_align + de_cline_num_##__name;                               \
            scalar_load(de_mem_addr_align, de_sm_buffer_##__name, de_cline_num_##__name);                      \
            de_dirty_##__name = 0;                                                                             \
        }                                                                                                      \
        de_tmp_addr_##__name = (unsigned long)(de_sm_buffer_##__name + de_sm_off);                             \
    } while (0)

// 带边界检查的 CACHEs_SEC_W_RD_K
// #define CACHEs_SEC_W_RD_K(__name, __addr, __value)                                                             \
//     do                                                                                                          \
//     {                                                                                                           \
//         unsigned long de_mem_addr = (unsigned long)(__addr);                                                    \
//         unsigned long de_mem_addr_align = de_mem_addr & de_make_mask_##__name;                                  \
//         unsigned long de_sm_off = de_mem_addr - de_mem_addr_align;                                              \
//         if (!(de_mem_addr_align >= de_memAddr_begine_##__name && de_mem_addr_align < de_memAddr_end_##__name))  \
//         {                                                                                                       \
//             if (de_dirty_##__name == 1)                                                                         \
//             {                                                                                                   \
//                 unsigned long de_store_size = de_cline_num_##__name;                                            \
//                 if (de_memAddr_end_##__name > de_base_addr_##__name + de_array_size_##__name)                   \
//                 {                                                                                               \
//                     de_store_size = de_base_addr_##__name + de_array_size_##__name - de_memAddr_begine_##__name;\
//                 }                                                                                               \
//                 scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_store_size);                 \
//             }                                                                                                   \
//             de_memAddr_begine_##__name = de_mem_addr_align;                                                     \
//             de_memAddr_end_##__name = de_mem_addr_align + de_cline_num_##__name;                                \
//             unsigned long de_load_size = de_cline_num_##__name;                                                 \
//             if (de_memAddr_end_##__name > de_base_addr_##__name + de_array_size_##__name)                       \
//             {                                                                                                   \
//                 de_load_size = de_base_addr_##__name + de_array_size_##__name - de_mem_addr_align;              \
//             }                                                                                                   \
//             scalar_load(de_mem_addr_align, de_sm_buffer_##__name, de_load_size);                                \
//             de_dirty_##__name = 0;                                                                              \
//         }                                                                                                       \
//         de_tmp_addr_##__name = (unsigned long)(de_sm_buffer_##__name + de_sm_off);                              \
//     } while (0)

#define CACHEs_SEC_W_RD(__name, __addr, __value, __type) \
    CACHEs_SEC_W_RD_K(__name, __addr, __value);          \
    (__value) = *(__type *)(de_tmp_addr_##__name);       \
    de_dirty_##__name = 0;

#define CACHEs_SEC_W_WR(__name, __addr, __value, __type) \
    CACHEs_SEC_W_RD_K(__name, __addr, __value);          \
    de_dirty_##__name = 1;                               \
    *(__type *)(de_tmp_addr_##__name) = (__value)

// #define CACHEs_FLUSH(__name)                                                                    \
//     if (de_dirty_##__name == 1)                                                                 \
//     {                                                                                           \
//         scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_cline_num_##__name); \
//     }                                                                                           \
//     scalar_free(de_sm_buffer_##__name);

#define CACHEs_FLUSH(__name) \
    scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_cline_num_##__name); \
    scalar_free(de_sm_buffer_##__name);

#define CACHEs_INVALID(__name)\
    scalar_free(de_sm_buffer_##__name);