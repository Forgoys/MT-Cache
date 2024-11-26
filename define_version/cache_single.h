#include "hthread_device.h"

#define CACHEs_ENV()

#define CACHEs_INIT(__name, __type, __Ea, __sets, __lines)                   \
    do {                                                                     \
        unsigned long de_cline_##__name = __lines;                           \
        unsigned long de_cline_num_##__name = (1UL << de_cline_##__name);    \
        unsigned long de_make_mask_##__name = ~(de_cline_num_##__name - 1);  \
        char *de_sm_buffer_##__name = scalar_malloc(de_cline_num_##__name);  \
        if (!de_sm_buffer_##__name)                                          \
        {                                                                    \
            hthread_printf("Memory allocation failed\n");                    \
        }                                                                    \
        unsigned long de_memAddr_begine_##__name = 0;                        \
        unsigned long de_memAddr_end_##__name = 0;                           \
        char de_dirty_##__name = 0;                                          \
        unsigned long de_tmp_addr_##__name = 0;                              \
    } while (0)


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

#define CACHEs_RD(__name, __addr, __value, __type)     \
    do                                                 \
    {                                                  \
        CACHEs_SEC_W_RD_K(__name, __addr, __value);    \
        (__value) = *(__type *)(de_tmp_addr_##__name); \
        de_dirty_##__name = 0;                         \
    } while (0)

#define CACHEs_WR(__name, __addr, __value, __type)     \
    do                                                 \
    {                                                  \
        CACHEs_SEC_W_RD_K(__name, __addr, __value);    \
        de_dirty_##__name = 1;                         \
        *(__type *)(de_tmp_addr_##__name) = (__value); \
    } while (0)

#define CACHEs_FLUSH(__name)                                                                    \
    do                                                                                          \
    {                                                                                           \
        scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_cline_num_##__name); \
        scalar_free(de_sm_buffer_##__name);                                                     \
    } while (0)

#define CACHEs_INVALID(__name)              \
    do                                      \
    {                                       \
        scalar_free(de_sm_buffer_##__name); \
    } while (0)
