#ifndef CACHE_WRAPPER_H
#define CACHE_WRAPPER_H

#ifdef ENABLE_CACHE_PROFILING
    // 包含带性能采集的版本
    #include "cache_direct_pro.h"
    #include "cache_single_pro.h"
    #include "cache_bulk_pro.h"

    // 启用性能打印宏
    #define CACHE_PRINT(...) hthread_print(__VA_ARGS__)
#else
    // 包含基础版本
    #include "cache_direct.h"
    #include "cache_single.h"
    #include "cache_bulk.h"
    
    // 当未启用性能采集时，将STATUS宏定义为空操作
    #define CACHEd_STATUS(__name) do {} while(0)
    #define CACHEs_STATUS(__name) do {} while(0)
    #define CACHEb_STATUS(__name) do {} while(0)

    // 禁用性能打印宏
    #define CACHE_PRINT(...) do {} while(0)
#endif

#endif // CACHE_WRAPPER_H