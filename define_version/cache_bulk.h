#include "hthread_device.h"

#define CACHEb_RD_K(__name, __Ea, __size, __type)                                     \
    unsigned long _interface_Ea_##__name = __Ea;\
    unsigned long _interface_original_##__name = __name;                              \
    unsigned long _interface_sizes_##__name = __size;                                 \
    __type *_interface_buffer_##__name = (__type *)scalar_malloc(__size);             \
    scalar_load((void *)__Ea, _interface_buffer_##__name, _interface_sizes_##__name); \
    __name = (__type *)(unsigned long)_interface_buffer_##__name;

// 批量写回内存
#define CACHEb_WR_K(__name)                                                                                      \
    do {                                                                                                         \
        scalar_store(_interface_buffer_##__name, (void *)_interface_Ea_##__name, _interface_sizes_##__name); \
    } while (0);

#define CACHEb_RD(__name, __addr, __value, __type) \
    do {                                           \
        __value = *(__type *)(__addr);             \
    } while (0);

#define CACHEb_WT(__name, __addr, __value, __type) \
    do {                                           \
        *(__type *)(__addr) = __value;             \
    } while (0);

#define CACHEb_INIT(__name, __type, __Ea, __sets, __lines) \
    unsigned long de_size_##__name = __lines;              \
    CACHEb_RD_K(__name, __Ea, de_size_##__name, __type);

#define CACHEb_FLUSH(__name)                     \
    do {                                         \
        CACHEb_WR_K(__name);                     \
        __name = _interface_original_##__name;   \
        scalar_free(_interface_buffer_##__name); \
    } while (0);

#define CACHEb_INVALID(__name)                   \
    do {                                         \
        __name = _interface_original_##__name;   \
        scalar_free(_interface_buffer_##__name); \
    } while (0);
