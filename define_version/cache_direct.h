#include "hthread_device.h"
#include <limits.h>

#define CACHEe_ENV() \
    // unsigned long de_point_p;          \
    // unsigned long de_offset;           \
    // unsigned long de_index;            \
    // unsigned long de_tag;              \
    // unsigned long de_line_addr_buffer; \
    // unsigned long de_align;

#define CACHEe_INIT(__name, __type, __csets, __cline)                                        \
    int de_csets_##__name = __csets;                                                         \
    int de_cline_##__name = __cline;                                                         \
    int de_csets_num_##__name = 1 << de_csets_##__name;                                      \
    int de_cline_num_##__name = 1 << de_cline_##__name;                                      \
    char *de_buffer_##__name = scalar_malloc(de_csets_num_##__name * de_cline_num_##__name); \
    unsigned long de_tag_##__name[de_csets_num_##__name];                                    \
    char de_dirty_##__name[de_csets_num_##__name];                                           \
    unsigned long de_offsetMask_##__name = de_cline_num_##__name - 1;                        \
    unsigned long de_indexMask_##__name = de_csets_num_##__name - 1;                         \
    unsigned long de_alignMask_##__name = ~de_offsetMask_##__name;                           \
    for (int i = 0; i < de_csets_num_##__name; i++)                                          \
    {                                                                                        \
        de_tag_##__name[i] = ULONG_MAX;                                                      \
        de_dirty_##__name[i] = 0;                                                            \
    }

#define CACHEe_SEC_R_RD(__name, __addr, __value, __type)                                                                               \
    do                                                                                                                                 \
    {                                                                                                                                  \
        unsigned long point_p = (unsigned long)((__type *)(__addr));                                                                   \
        unsigned long offset = point_p & de_offsetMask_##__name;                                                                       \
        unsigned long index = (point_p >> de_cline_##__name) & de_indexMask_##__name;                                                  \
        unsigned long tag = point_p >> (de_cline_##__name + de_csets_##__name);                                                        \
        unsigned long align = point_p & de_alignMask_##__name;                                                                         \
        char *line_addr_buffer = de_buffer_##__name + index * de_cline_num_##__name;                                                   \
        if (de_tag_##__name[index] != tag)                                                                                             \
        {                                                                                                                              \
            if (de_dirty_##__name[index] == 1)                                                                                         \
            {                                                                                                                          \
                unsigned long Ea = (de_tag_##__name[index] << (de_cline_##__name + de_csets_##__name)) + (index << de_cline_##__name); \
                scalar_store(line_addr_buffer, Ea, de_cline_num_##__name);                                                             \
            }                                                                                                                          \
            scalar_load(align, line_addr_buffer, de_cline_num_##__name);                                                               \
            de_tag_##__name[index] = tag;                                                                                              \
            de_dirty_##__name[index] = 0;                                                                                              \
        }                                                                                                                              \
        __value = *((__type *)(line_addr_buffer + offset));                                                                            \
    } while (0)

#define CACHEe_SEC_W_RD(__name, __addr, __value, __type)                                                                               \
    do                                                                                                                                 \
    {                                                                                                                                  \
        unsigned long point_p = (unsigned long)((__type *)(__addr));                                                                   \
        unsigned long offset = point_p & de_offsetMask_##__name;                                                                       \
        unsigned long index = (point_p >> de_cline_##__name) & de_indexMask_##__name;                                                  \
        unsigned long tag = point_p >> (de_cline_##__name + de_csets_##__name);                                                        \
        unsigned long align = point_p & de_alignMask_##__name;                                                                         \
        char *line_addr_buffer = de_buffer_##__name + index * de_cline_num_##__name;                                                   \
        if (de_tag_##__name[index] != tag)                                                                                             \
        {                                                                                                                              \
            if (de_dirty_##__name[index] == 1)                                                                                         \
            {                                                                                                                          \
                unsigned long Ea = (de_tag_##__name[index] << (de_cline_##__name + de_csets_##__name)) + (index << de_cline_##__name); \
                scalar_store(line_addr_buffer, Ea, de_cline_num_##__name);                                                             \
            }                                                                                                                          \
            scalar_load(align, line_addr_buffer, de_cline_num_##__name);                                                               \
            de_tag_##__name[index] = tag;                                                                                              \
        }                                                                                                                              \
        *((__type *)(line_addr_buffer + offset)) = __value;                                                                            \
        de_dirty_##__name[index] = 1;                                                                                                  \
    } while (0);

#define CACHEe_FLUSH(__name)                                                                                           \
    do                                                                                                                         \
    {                                                                                                                          \
        for (int i = 0; i < de_csets_num_##__name; i++)                                                                        \
        {                                                                                                                      \
            if (de_tag_##__name[i] != ULONG_MAX && de_dirty_##__name[i] == 1)                                                  \
            {                                                                                                                  \
                unsigned long Ea = (de_tag_##__name[i] << (de_cline_##__name + de_csets_##__name)) + (i << de_cline_##__name); \
                unsigned long addr_buffer = (unsigned long)(de_buffer_##__name + i * de_cline_num_##__name);                   \
                scalar_store(addr_buffer, Ea, de_cline_num_##__name);                                                          \
            }                                                                                                                  \
        }                                                                                                                      \
        scalar_free(de_buffer_##__name);                                                                                       \
    } while (0)

#define CACHEe_INVALID(__name)           \
    do                                   \
    {                                    \
        scalar_free(de_buffer_##__name); \
    } while (0)
