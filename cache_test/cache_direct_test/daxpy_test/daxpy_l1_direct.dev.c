#include <stdint.h>

#include <compiler/m3000.h>
#include "hthread_device.h"
#include "cache_direct.h"

#define SEGMENT_SIZE 2048 // (2048 * 8) / 1024 = 16KB

static inline void daxpy_single(uint64_t n, double a, double *x, double *y)
{
    for (uint64_t i = 0; i < n; ++i)
    {
        y[i] = a * x[i] + y[i];
    }
}

void daxpy_single_cache_set(uint64_t n, double a, double *x, double *y)
{
    CACHEe_ENV();
    CACHEe_INIT(x, double, 7, 7);
    CACHEe_INIT(y, double, 7, 7);
    double xx, yy;
    for (uint64_t i = 0; i < n; ++i)
    {
        CACHEe_SEC_R_RD(x, x + i, xx, double);
        CACHEe_SEC_R_RD(y, y + i, yy, double);
        yy = a * xx + yy;
        CACHEe_SEC_W_RD(y, y + i, yy, double);
    }
    CACHEe_INVALID(x);
    CACHEe_FLUSH(y, double);
}

#define PRINT_TIME(message, time)                                                    \
    hthread_printf("%s time: %lfs\n", message, (double)(get_clk() - time) / 4150e6); \
    time = get_clk();

__global__ void daxpy_mem_test_kernel(uint64_t n, double a, double *x, double *y)
{
    CACHEe_ENV();
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

    // int len = SEGMENT_SIZE;
    // for (int i = 0; i < n_p; i += len)
    // {
    //     len = (n_p - i) >= SEGMENT_SIZE ? SEGMENT_SIZE : (n_p - i);
    //     daxpy_single_cache_set(len, a, x + offset + i, y + offset + i);
    // }
    daxpy_single(n_p, a, x + offset, y + offset);
}

__global__ void daxpy_cache_test_kernel(uint64_t n, double a, double *x, double *y)
{
    CACHEe_ENV();
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

    // int len = SEGMENT_SIZE;
    // for (int i = 0; i < n_p; i += len)
    // {
    //     len = (n_p - i) >= SEGMENT_SIZE ? SEGMENT_SIZE : (n_p - i);
    //     daxpy_single_cache_set(len, a, x + offset + i, y + offset + i);
    // }
    daxpy_single_cache_set(n_p, a, x + offset, y + offset);
}