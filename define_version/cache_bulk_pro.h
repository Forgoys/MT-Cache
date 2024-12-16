#include "hthread_device.h"

#define CACHEb_RD_K(__name, __Ea, __size, __type)                                                                      \
    char *_interface_buffer_##__name = (char *)scalar_malloc(__size + 64);                                             \
    unsigned long _interface_Ea_align_##__name = (unsigned long)__Ea & 0xFFFFFFFFFFFFFFE0;                             \
    unsigned long _interface_dma_offset_##__name = (unsigned long)__Ea - (unsigned long)_interface_Ea_align_##__name;  \
    unsigned long _interface_original_##__name;                                                                        \
    unsigned long _interface_sizes_##__name = (((__size + _interface_dma_offset_##__name) + 63) & ~63);                \
    scalar_load((void *)_interface_Ea_align_##__name, _interface_buffer_##__name, _interface_sizes_##__name);          \
    _interface_original_##__name = __Ea;                                                                               \
    __Ea = (__type *)((unsigned long)_interface_buffer_##__name + _interface_dma_offset_##__name);                     \
    unsigned long de_Ea_##__name = (unsigned long)__Ea;

#define CACHEb_WR_K(__name)                                                                                            \
    do {                                                                                                               \
        _interface_sizes_##__name = ((de_size_##__name + _interface_dma_offset_##__name) & 0xffffff80) + 128;          \
        scalar_store(_interface_buffer_##__name, (void *)_interface_Ea_align_##__name, _interface_sizes_##__name);     \
    } while (0)

#define CACHEb_INIT(__name, __type, __Ea, __sets, __lines)                                                             \
    unsigned long de_size_##__name = __lines;                                                                          \
    /* Initialize performance counters */                                                                              \
    unsigned long de_read_count_##__name = 0;                                                                          \
    unsigned long de_write_count_##__name = 0;                                                                         \
    CACHEb_RD_K(__name, __Ea, de_size_##__name, __type);

#define CACHEb_RD(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        de_read_count_##__name++;                                                                                      \
        __value = *(__type *)(__addr);                                                                                 \
    } while (0)

#define CACHEb_WT(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        de_write_count_##__name++;                                                                                     \
        *(__type *)(__addr) = __value;                                                                                 \
    } while (0)

#define CACHEb_STATUS(__name)                                                                                          \
    do {                                                                                                               \
        float read_hit_rate = 100.0;                                                                                   \
        float write_hit_rate = 100.0;                                                                                  \
        float total_hit_rate = 100.0;                                                                                  \
        hthread_print("\n====================================================\n");                                     \
        hthread_print("              Cache Statistics Report                \n");                                      \
        hthread_print("====================================================\n");                                       \
        hthread_print("Thread ID:    %d\n", get_thread_id());                                                          \
        hthread_print("Cache Name:   %s\n", #__name);                                                                  \
        hthread_print("Cache Type:   Batch Cache\n");                                                                  \
        hthread_print("----------------------------------------------------\n");                                       \
        hthread_print("Access Statistics:\n");                                                                         \
        hthread_print("  Read Operations:     %lu\n", de_read_count_##__name);                                         \
        hthread_print("  Write Operations:    %lu\n", de_write_count_##__name);                                        \
        hthread_print("  Total Operations:    %lu\n", de_read_count_##__name + de_write_count_##__name);               \
        hthread_print("\nCache Performance:\n");                                                                       \
        hthread_print("  Read Hit Rate:       %.2f%%\n", read_hit_rate);                                               \
        hthread_print("  Write Hit Rate:      %.2f%%\n", write_hit_rate);                                              \
        hthread_print("  Overall Hit Rate:    %.2f%%\n", total_hit_rate);                                              \
        hthread_print("====================================================\n\n");                                     \
    } while (0)

#define CACHEb_FLUSH(__name)                                                                                           \
    do {                                                                                                               \
        CACHEb_WR_K(__name);                                                                                           \
        __name = _interface_original_##__name;                                                                         \
        scalar_free(_interface_buffer_##__name);                                                                       \
    } while (0)

#define CACHEb_INVALID(__name)                                                                                         \
    do {                                                                                                               \
        __name = _interface_original_##__name;                                                                         \
        scalar_free(_interface_buffer_##__name);                                                                       \
    } while (0)