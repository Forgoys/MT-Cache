/**
 * daxpy.hos.c
 * @Author      : jshen, longbiao
 * @Date        : 2022-06-27
 * @Project     : MT3000 daxpy
 * @description : example implementation of daxpy on MT3000
 *                y = x * alph + y
 *
 **/
#include <stdint.h>

#include "cache_bulk.h"
#include "hthread_device.h"

#define SEGMENT_SIZE 1024 // 1024 * 8 / 1024 8KB

static inline void daxpy_single(uint64_t n, double a, double *x, double *y)
{
    for (uint64_t i = 0; i < n; ++i) {
        y[i] = a * x[i] + y[i];
    }
}

static inline void daxpy_single_cache(uint64_t n, double a, double *x, double *y)
{
    double *x_rcd = x, *y_rcd = y;
    CACHEb_INIT(x, double, x, 0, n * sizeof(double));
    CACHEb_INIT(y, double, y, 0, n * sizeof(double));
    double xx, yy;
    for (uint64_t i = 0; i < n; ++i) {
        CACHEb_RD(x, x + i, xx, double);
        CACHEb_RD(y, y + i, yy, double);
        yy = a * xx + yy;
        // if (yy != a * x_rcd[i] + y_rcd[i]) {
        //     hthread_printf("id: %d, index: %d\n", get_thread_id(), i);
        // }
        CACHEb_WT(y, y + i, yy, double);
    }
    CACHEb_INVALID(x);
    CACHEb_FLUSH(y);
}

static inline void daxpy_single_cache_test(uint64_t n, double a, double *x, double *y)
{
    double *x_tmp = x, *y_tmp = y;
    CACHEb_INIT(x, double, x, 0, n * sizeof(double));
    CACHEb_INIT(y, double, y, 0, n * sizeof(double));
    double xx, yy;
    for (uint64_t i = 0; i < n; ++i) {
        CACHEb_RD(x, x + i, xx, double);
        CACHEb_RD(y, y + i, yy, double);
        if (x_tmp[i] != xx || y_tmp[i] != yy) {
            hthread_printf("id: %d, index: %d\n", get_thread_id(), i);
        }
        yy = a * xx + yy;
        CACHEb_WT(y, y + i, yy, double);
    }
    CACHEb_INVALID(x);
    CACHEb_FLUSH(y);
}

__global__ void daxpy_kernel(uint64_t n, double a, double *x, double *y)
{
    int threadId = get_thread_id();
    int threadsNum = get_group_size();
    if (threadId >= threadsNum)
        return;

    // 计算每个线程的起始位置和处理长度
    uint64_t base = n / threadsNum;
    uint64_t extra = n % threadsNum;
    uint64_t start = threadId * base + (threadId < extra ? threadId : extra);
    uint64_t length = base + (threadId < extra ? 1 : 0);

    // hthread_printf("id: %d, start: %llu, length: %llu\n", get_thread_id(), start, length);

    if (length == 0)
        return; // 避免空任务

    // 分段处理
    uint64_t processed = 0;
    while (processed < length) {
        uint64_t current_chunk = SEGMENT_SIZE < length - processed ? SEGMENT_SIZE : length - processed;
        daxpy_single_cache(current_chunk, a, x + start + processed, y + start + processed);
        processed += current_chunk;
    }
}