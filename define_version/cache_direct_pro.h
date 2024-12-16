#include "hthread_device.h"
#include <limits.h>

#define CACHEd_ENV()                                                                                                   \
    // unsigned long de_point_p;          \
    // unsigned long de_offset;           \
    // unsigned long de_index;            \
    // unsigned long de_tag;              \
    // unsigned long de_line_addr_buffer; \
    // unsigned long de_align;

#define CACHEd_INIT(__name, __type, __Ea, __sets, __lines)                                                             \
    int de_csets_##__name = __sets;                                                                                    \
    int de_cline_##__name = __lines;                                                                                   \
    int de_csets_num_##__name = 1 << de_csets_##__name;                                                                \
    int de_cline_num_##__name = 1 << de_cline_##__name;                                                                \
    char *de_buffer_##__name = scalar_malloc(de_csets_num_##__name * de_cline_num_##__name);                           \
    unsigned long de_tag_##__name[de_csets_num_##__name];                                                              \
    char de_dirty_##__name[de_csets_num_##__name];                                                                     \
    unsigned long de_offsetMask_##__name = de_cline_num_##__name - 1;                                                  \
    unsigned long de_indexMask_##__name = de_csets_num_##__name - 1;                                                   \
    unsigned long de_alignMask_##__name = ~de_offsetMask_##__name;                                                     \
    /* 初始化统计计数器 */                                                                                     \
    unsigned long de_read_count_##__name = 0;                                                                          \
    unsigned long de_write_count_##__name = 0;                                                                         \
    unsigned long de_read_hits_##__name = 0;                                                                           \
    unsigned long de_write_hits_##__name = 0;                                                                          \
    for (int i = 0; i < de_csets_num_##__name; i++) {                                                                  \
        de_tag_##__name[i] = ULONG_MAX;                                                                                \
        de_dirty_##__name[i] = 0;                                                                                      \
    }

#define CACHEd_RD(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        de_read_count_##__name++;                                                                                      \
        unsigned long point_p = (unsigned long)((__type *)(__addr));                                                   \
        unsigned long offset = point_p & de_offsetMask_##__name;                                                       \
        unsigned long index = (point_p >> de_cline_##__name) & de_indexMask_##__name;                                  \
        unsigned long tag = point_p >> (de_cline_##__name + de_csets_##__name);                                        \
        unsigned long align = point_p & de_alignMask_##__name;                                                         \
        char *line_addr_buffer = de_buffer_##__name + index * de_cline_num_##__name;                                   \
        if (de_tag_##__name[index] != tag) {                                                                           \
            if (de_dirty_##__name[index] == 1) {                                                                       \
                unsigned long Ea = (de_tag_##__name[index] << (de_cline_##__name + de_csets_##__name)) +               \
                                   (index << de_cline_##__name);                                                       \
                scalar_store(line_addr_buffer, Ea, de_cline_num_##__name);                                             \
            }                                                                                                          \
            scalar_load(align, line_addr_buffer, de_cline_num_##__name);                                               \
            de_tag_##__name[index] = tag;                                                                              \
            de_dirty_##__name[index] = 0;                                                                              \
        } else {                                                                                                       \
            de_read_hits_##__name++;                                                                                   \
        }                                                                                                              \
        __value = *((__type *)(line_addr_buffer + offset));                                                            \
    } while (0)

#define CACHEd_WT(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        de_write_count_##__name++;                                                                                     \
        unsigned long point_p = (unsigned long)((__type *)(__addr));                                                   \
        unsigned long offset = point_p & de_offsetMask_##__name;                                                       \
        unsigned long index = (point_p >> de_cline_##__name) & de_indexMask_##__name;                                  \
        unsigned long tag = point_p >> (de_cline_##__name + de_csets_##__name);                                        \
        unsigned long align = point_p & de_alignMask_##__name;                                                         \
        char *line_addr_buffer = de_buffer_##__name + index * de_cline_num_##__name;                                   \
        if (de_tag_##__name[index] != tag) {                                                                           \
            if (de_dirty_##__name[index] == 1) {                                                                       \
                unsigned long Ea = (de_tag_##__name[index] << (de_cline_##__name + de_csets_##__name)) +               \
                                   (index << de_cline_##__name);                                                       \
                scalar_store(line_addr_buffer, Ea, de_cline_num_##__name);                                             \
            }                                                                                                          \
            scalar_load(align, line_addr_buffer, de_cline_num_##__name);                                               \
            de_tag_##__name[index] = tag;                                                                              \
        } else {                                                                                                       \
            de_write_hits_##__name++;                                                                                  \
        }                                                                                                              \
        *((__type *)(line_addr_buffer + offset)) = __value;                                                            \
        de_dirty_##__name[index] = 1;                                                                                  \
    } while (0)

#define CACHEd_STATUS(__name)                                                                                          \
    do {                                                                                                               \
        if (get_thread_id() == 0) {                                                                                    \
            float read_hit_rate =                                                                                      \
                (de_read_count_##__name > 0) ? (float)de_read_hits_##__name / de_read_count_##__name * 100 : 0;        \
            float write_hit_rate =                                                                                     \
                (de_write_count_##__name > 0) ? (float)de_write_hits_##__name / de_write_count_##__name * 100 : 0;     \
            float total_hit_rate = ((de_read_count_##__name + de_write_count_##__name) > 0)                            \
                                       ? (float)(de_read_hits_##__name + de_write_hits_##__name) /                     \
                                             (de_read_count_##__name + de_write_count_##__name) * 100                  \
                                       : 0;                                                                            \
            hthread_printf("\n====================================================\n");                                \
            hthread_printf("              Cache Statistics Report                \n");                                 \
            hthread_printf("====================================================\n");                                  \
            hthread_printf("Thread ID:    %d\n", get_thread_id());                                                     \
            hthread_printf("Cache Name:   %s\n", #__name);                                                             \
            hthread_printf("Cache Type:   Direct Mapped Cache\n");                                                     \
            hthread_printf("----------------------------------------------------\n");                                  \
            hthread_printf("Access Statistics:\n");                                                                    \
            hthread_printf("  Read Operations:     %lu\n", de_read_count_##__name);                                    \
            hthread_printf("  Write Operations:    %lu\n", de_write_count_##__name);                                   \
            hthread_printf("  Total Operations:    %lu\n", de_read_count_##__name + de_write_count_##__name);          \
            hthread_printf("\nCache Performance:\n");                                                                  \
            hthread_printf("  Read Hit Rate:       %.2f%%\n", read_hit_rate);                                          \
            hthread_printf("  Write Hit Rate:      %.2f%%\n", write_hit_rate);                                         \
            hthread_printf("  Overall Hit Rate:    %.2f%%\n", total_hit_rate);                                         \
            hthread_printf("====================================================\n\n");                                \
        }                                                                                                              \
    } while (0)

#define CACHEd_FLUSH(__name)                                                                                           \
    do {                                                                                                               \
        for (int i = 0; i < de_csets_num_##__name; i++) {                                                              \
            if (de_tag_##__name[i] != ULONG_MAX && de_dirty_##__name[i] == 1) {                                        \
                unsigned long Ea =                                                                                     \
                    (de_tag_##__name[i] << (de_cline_##__name + de_csets_##__name)) + (i << de_cline_##__name);        \
                unsigned long addr_buffer = (unsigned long)(de_buffer_##__name + i * de_cline_num_##__name);           \
                scalar_store(addr_buffer, Ea, de_cline_num_##__name);                                                  \
            }                                                                                                          \
        }                                                                                                              \
        scalar_free(de_buffer_##__name);                                                                               \
    } while (0)

#define CACHEd_INVALID(__name)                                                                                         \
    do {                                                                                                               \
        scalar_free(de_buffer_##__name);                                                                               \
    } while (0)