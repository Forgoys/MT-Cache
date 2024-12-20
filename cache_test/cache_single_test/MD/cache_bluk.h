#include "hthread_device.h"
#define CACHEd_ENV()    
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

// 批量写回内存
#define CACHEb_WR_K(__name)                                                                                            \
    do {                                                                                                               \
        _interface_sizes_##__name = ((de_size_##__name + _interface_dma_offset_##__name) & 0xffffff80) + 128;          \
        scalar_store(_interface_buffer_##__name, (void *)_interface_Ea_align_##__name, _interface_sizes_##__name);     \
    } while (0)

#define CACHEb_RD(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        __value = *(__type *)(__addr);                                                                                 \
    } while (0)

#define CACHEb_WT(__name, __addr, __value, __type)                                                                     \
    do {                                                                                                               \
        *(__type *)(__addr) = __value;                                                                                 \
    } while (0)

#define CACHEb_INIT(__name, __type, __Ea, __sets, __lines)                                                             \
    unsigned long de_size_##__name = __lines;                                                                          \
    CACHEb_RD_K(__name, __Ea, de_size_##__name, __type);

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