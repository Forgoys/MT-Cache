#include "hthread_device.h"

#define CACHEs_ENV()

#define CACHEs_INIT(__name, __type, __Ea, __sets, __lines)                                                             \
    unsigned long de_cline_##__name = __lines;                                                                         \
    unsigned long de_cline_num_##__name = (1UL << de_cline_##__name);                                                  \
    unsigned long de_make_mask_##__name = ~(de_cline_num_##__name - 1);                                                \
    char *de_sm_buffer_##__name = scalar_malloc(de_cline_num_##__name);                                                \
    if (!de_sm_buffer_##__name) {                                                                                      \
        hthread_printff("Memory allocation failed\n");                                                                 \
    }                                                                                                                  \
    unsigned long de_memAddr_begine_##__name = 0;                                                                      \
    unsigned long de_memAddr_end_##__name = 0;                                                                         \
    char de_dirty_##__name = 0;                                                                                        \
    unsigned long de_tmp_addr_##__name = 0;                                                                            \
    /* Initialize performance counters */                                                                              \
    unsigned long de_read_count_##__name = 0;                                                                          \
    unsigned long de_write_count_##__name = 0;                                                                         \
    unsigned long de_read_hits_##__name = 0;                                                                           \
    unsigned long de_write_hits_##__name = 0;

#define CACHEs_SEC_W_RD_K(__name, __addr, __value)                                                                     \
    do {                                                                                                               \
        unsigned long de_mem_addr = (__addr);                                                                          \
        unsigned long de_mem_addr_align = de_mem_addr & de_make_mask_##__name;                                         \
        unsigned long de_sm_off = de_mem_addr - de_mem_addr_align;                                                     \
        if (!(de_mem_addr_align >= de_memAddr_begine_##__name && de_mem_addr_align < de_memAddr_end_##__name)) {       \
            if (de_dirty_##__name == 1) {                                                                              \
                scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_cline_num_##__name);                \
            }                                                                                                          \
            de_memAddr_begine_##__name = de_mem_addr_align;                                                            \
            de_memAddr_end_##__name = de_mem_addr_align + de_cline_num_##__name;                                       \
            scalar_load(de_mem_addr_align, de_sm_buffer_##__name, de_cline_num_##__name);                              \
            de_dirty_##__name = 0;                                                                                     \
        } else {                                                                                                       \
            /* Cache hit - increment hit counter based on operation state */                                           \
            if (de_op_state_##__name == 0) {                                                                           \
                de_read_hits_##__name++;                                                                               \
            } else {                                                                                                   \
                de_write_hits_##__name++;                                                                              \
            }                                                                                                          \
        }                                                                                                              \
        de_tmp_addr_##__name = (unsigned long)(de_sm_buffer_##__name + de_sm_off);                                     \
    } while (0)

#define CACHEs_RD(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        de_read_count_##__name++;                                                                                      \
        de_op_state_##__name = 0; /* Set state to read */                                                              \
        CACHEs_SEC_W_RD_K(__name, __addr, __value);                                                                    \
        (__value) = *(__type *)(de_tmp_addr_##__name);                                                                 \
        de_dirty_##__name = 0;                                                                                         \
    } while (0)

#define CACHEs_WT(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        de_write_count_##__name++;                                                                                     \
        de_op_state_##__name = 1; /* Set state to write */                                                             \
        CACHEs_SEC_W_RD_K(__name, __addr, __value);                                                                    \
        de_dirty_##__name = 1;                                                                                         \
        *(__type *)(de_tmp_addr_##__name) = (__value);                                                                 \
    } while (0)

#define CACHEs_STATUS(__name)                                                                                          \
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
            hthread_printf("Cache Type:   Single Buffer Cache\n");                                                     \
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

#define CACHEs_FLUSH(__name)                                                                                           \
    do {                                                                                                               \
        scalar_store(de_sm_buffer_##__name, de_memAddr_begine_##__name, de_cline_num_##__name);                        \
        scalar_free(de_sm_buffer_##__name);                                                                            \
    } while (0)

#define CACHEs_INVALID(__name)                                                                                         \
    do {                                                                                                               \
        scalar_free(de_sm_buffer_##__name);                                                                            \
    } while (0)