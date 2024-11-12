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

#include <compiler/m3000.h>
#include "hthread_device.h"
#include "cache_bulk.h"

#define SEGMENT_SIZE 3840 // (2048 * 8) / 1024 = 16KB

static inline void daxpy_single(uint64_t n, double a, double *x, double *y)
{
    for (uint64_t i = 0; i < n; ++i)
    {
        y[i] = a * x[i] + y[i];
    }
}

static inline void daxpy_single_bulk_cache(uint64_t n, double a, double *x, double *y)
{
    INTERFACE_INIT(x, double, 1, 1, 64, 0, 0, x, n * sizeof(double));
    INTERFACE_INIT(y, double, 1, 1, 64, 0, 0, y, n * sizeof(double));
    double xx, yy;
    for (uint64_t i = 0; i < n; ++i)
    {
        INTERFACE_SEC_W_RD(x, x + i, xx, double);
        INTERFACE_SEC_W_RD(y, y + i, yy, double);
        yy = a * xx + yy;
        INTERFACE_SEC_W_WR(y, y + i, yy, double);
    }
    INTERFACE_INVALID(x, x, n * sizeof(double));
    INTERFACE_FLUSH(y, y, n * sizeof(double));
}

__global__ void daxpy_kernel(uint64_t n, double a, double *x, double *y)
{
    int threadId = get_thread_id();
    int threadsNum = get_group_size();
    if (threadId >= threadsNum)
        return;
    uint64_t n_p = n / threadsNum;
    uint64_t extras = n % threadsNum;
    uint64_t offset;
    if (threadId < extras)
    {
        n_p++;
        offset = threadId * n_p;
    }
    else
    {
        offset = threadId * (n_p + 1) - (threadId - extras);
    }

    int len = SEGMENT_SIZE;
    for (int i = 0; i < n_p; i += len)
    {
        len = (n_p - i) >= SEGMENT_SIZE ? SEGMENT_SIZE : (n_p - i);
        // daxpy_single(len, a, x + offset + i, y + offset + i);
        daxpy_single_bulk_cache(len, a, x + offset + i, y + offset + i);
    }
}
