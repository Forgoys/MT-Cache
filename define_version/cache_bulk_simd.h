#include "hthread_device.h"

// TODO: 用跨步读取的方式改进读API
#define INTERFACE_RD_K(__Ea, __size, __type)                                                                                                                                \
    unsigned int _channel_number_##__Ea = 0;                                                                                                                                \
    char *_interface_buffer_##__Ea = (char *)scalar_malloc(__size + 64);                                                                                                    \
    unsigned long _interface_Ea_align_##__Ea = (unsigned long)__Ea & 0xFFFFFFFFFFFFFFE0;                                                                                    \
    unsigned long _interface_dma_offset_##__Ea = (unsigned long)__Ea - (unsigned long)_interface_Ea_align_##__Ea;                                                           \
    unsigned long _interface_original_##__Ea;                                                                                                                               \
    unsigned long _interface_sizes_##__Ea = ((__size + _interface_dma_offset_##__Ea) & 0xFFFFFFFFFFFFFFE0) + 64;                                                            \
    _channel_number_##__Ea = dma_p2p((void *)_interface_Ea_align_##__Ea, 1, _interface_sizes_##__Ea, 0, _interface_buffer_##__Ea, 1, _interface_sizes_##__Ea, 0, false, 0); \
    dma_wait(_channel_number_##__Ea);                                                                                                                                       \
    _interface_original_##__Ea = __Ea;                                                                                                                                      \
    __Ea = (__type *)((unsigned long)_interface_buffer_##__Ea + _interface_dma_offset_##__Ea);

#define INTERFACE_WR_K(__Ea, __size)                                                                                                                                        \
    _interface_sizes_##__Ea = ((__size + _interface_dma_offset_##__Ea) & 0xffffff80) + 128;                                                                                 \
    _channel_number_##__Ea = dma_p2p(_interface_buffer_##__Ea, 1, _interface_sizes_##__Ea, 0, (void *)_interface_Ea_align_##__Ea, 1, _interface_sizes_##__Ea, 0, false, 0); \
    dma_wait(_channel_number_##__Ea);

#define INTERFACE_INIT(__name, __type, __csets, __cways, __cline, __unit_num, __index, __Ea, __size) \
    INTERFACE_RD_K(__Ea, __size, __type)

#define INTERFACE_FLUSH(__name, __Ea, __size) \
    INTERFACE_WR_K(__Ea, __size);             \
    __Ea = _interface_original_##__Ea;        \
    scalar_free(_interface_buffer_##__Ea);

#define INTERFACE_INVALID(__name, __Ea, __size) \
    __Ea = _interface_original_##__Ea;          \
    scalar_free(_interface_buffer_##__Ea);

#define INTERFACE_SEC_W_RD(__name, __addr, __value, __type) \
    __value = *(__type *)(__addr)

#define INTERFACE_SEC_W_WR(__name, __addr, __value, __type) \
    *(__type *)(__addr) = __value

#define INTERFACE_SEC_R_RD(__name, __addr, __value, __type) \
    __value = *(__type *)(__addr)
