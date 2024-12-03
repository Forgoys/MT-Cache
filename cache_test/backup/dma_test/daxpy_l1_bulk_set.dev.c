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
#include "cache_set.h"

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
    double *x_tmp = x, *y_tmp = y;
    CACHEe_ENV();
    CACHEe_INIT(x, double, 4, 4, 64, 0, 0, x, n * sizeof(double));
    CACHEe_INIT(y, double, 4, 4, 64, 0, 0, y, n * sizeof(double));
    double xx, yy;
    for (uint64_t i = 0; i < n; ++i)
    {
        CACHEe_SEC_R_RD(x, x + i, xx, double);
        CACHEe_SEC_R_RD(y, y + i, yy, double);
        yy = a * xx + yy;
        // INTERFACE_SEC_W_WR(y, y + i, yy, double);
    }
    // INTERFACE_INVALID(x, x, n * sizeof(double));
    // INTERFACE_FLUSH(y, y, n * sizeof(double));
}

#define PRINT_TIME(message, time)                                                    \
    hthread_printf("%s time: %lfs\n", message, (double)(get_clk() - time) / 4150e6); \
    time = get_clk();

__global__ void daxpy_kernel(uint64_t n, double a, double *x, double *y)
{
    // CACHEe_ENV();
    // int threadId = get_thread_id();
    // int threadsNum = get_group_size();
    // if (threadId >= threadsNum)
    //     return;
    // uint64_t n_p = n / threadsNum;
    // uint64_t extras = n % threadsNum;
    // uint64_t offset;
    // if (threadId < extras)
    // {
    //     n_p++;
    //     offset = threadId * n_p;
    // }
    // else
    // {
    //     offset = threadId * (n_p + 1) - (threadId - extras);
    // }

    // int len = SEGMENT_SIZE;
    // for (int i = 0; i < n_p; i += len)
    // {
    //     len = (n_p - i) >= SEGMENT_SIZE ? SEGMENT_SIZE : (n_p - i);
    //     daxpy_single_cache_set(len, a, x + offset + i, y + offset + i);

    // }
    CACHEe_ENV();
    // CACHEe_INIT(__name, __type, __csets, __cways, __cline, __unit_num, __index, __Ea, __size)
    CACHEe_INIT(x, double, 3, 2, 7, 0, 0, x, n * sizeof(double));
    CACHEe_INIT(y, double, 3, 2, 7, 0, 0, y, n * sizeof(double));
    double x_tmp, y_tmp;
    unsigned long t1, t2;
    int step = 2;

    t1 = get_clk();
    for (int i = 0; i < n; i += step)
    {
        CACHEe_SEC_R_RD(x, x + i, x_tmp, double);
        CACHEe_SEC_R_RD(y, y + i, y_tmp, double);
    }
    t2 = get_clk();
    hthread_printf("%s time: %lfs\n", "cache set", (double)(t2 - t1) / 4150e6);
    hthread_printf("cache hit rate: %g%%\n", 1 - (double)_cachee_miss / (2 * n / step));

    t1 = get_clk();
    for (int i = 0; i < n; i += step)
    {
        x_tmp = *(x + i);
        y_tmp = *(y + i);
    }
    t2 = get_clk();
    hthread_printf("%s time: %lfs\n", "without cache set", (double)(t2 - t1) / 4150e6);

    // CACHEe_SEC_R_RD_DEBUG(x, x, x_tmp, double);
    // x_tmp++;

    // t1 = get_clk();
    // CACHEe_SEC_R_RD(x, x, x_tmp, double);
    // t2 = get_clk();
    // x_tmp++;
    // hthread_printf("cache miss clocks total: %lu\n", t2 - t1);

    // t1 = get_clk();
    // CACHEe_SEC_R_RD(x, x + 1, x_tmp, double);
    // t2 = get_clk();
    // x_tmp++;
    // hthread_printf("cache hit clocks total: %lu\n", t2 - t1);

    // t1 = get_clk();
    // x_tmp = *(x + 2);
    // t2 = get_clk();
    // x_tmp++;
    // hthread_printf("without cache set clocks total: %lu\n", t2 - t1);
}
