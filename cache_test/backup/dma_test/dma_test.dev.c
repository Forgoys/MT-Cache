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

#define PRINT_TIME(message, time)                                                    \
    hthread_printf("%s time: %lfs\n", message, (double)(get_clk() - time) / 4150e6); \
    time = get_clk();

__global__ void daxpy_kernel(uint64_t nn, double a, double *x, double *y)
{
    double x_tmp, x_tmp1, x_tmp2, x_tmp3, x_tmp4, x_tmp5, x_tmp6, x_tmp7, x_tmp8;
    unsigned long t1, t2;
    int step = 1;

    double *sm_mem = scalar_malloc(128 * sizeof(double));
    int idx = 0;
    int dma_no = 0;
    int n = 8;

    t1 = get_clk();
    dma_no = dma_p2p(x, 1, n * sizeof(double), 0, sm_mem, 1, n * sizeof(double), 0, false, 0);
    dma_wait(dma_no);
    t2 = get_clk();
    hthread_printf("dma using dma_p2p mem -> sm, clocks total: %lu\n", t2 - t1);

    idx += n;

    t1 = get_clk();
    scalar_load(x + idx, sm_mem + idx, n * sizeof(double));
    t2 = get_clk();
    hthread_printf("dma using scalar_load mem -> sm, clocks total: %lu\n", t2 - t1);

    t1 = get_clk();
    x_tmp1 = sm_mem[idx++];
    x_tmp2 = sm_mem[idx++];
    x_tmp3 = sm_mem[idx++];
    x_tmp4 = sm_mem[idx++];
    x_tmp5 = sm_mem[idx++];
    x_tmp6 = sm_mem[idx++];
    x_tmp7 = sm_mem[idx++];
    x_tmp8 = sm_mem[idx++];
    t2 = get_clk();
    x_tmp1++;
    x_tmp2++;
    x_tmp3++;
    x_tmp4++;
    x_tmp5++;
    x_tmp6++;
    x_tmp7++;
    x_tmp8++;
    hthread_printf("sm -> reg, clocks total: %lu\n", t2 - t1);

    idx += n;
    t1 = get_clk();
    x_tmp1 = *(x + idx++);
    x_tmp2 = *(x + idx++);
    x_tmp3 = *(x + idx++);
    x_tmp4 = *(x + idx++);
    x_tmp5 = *(x + idx++);
    x_tmp6 = *(x + idx++);
    x_tmp7 = *(x + idx++);
    x_tmp8 = *(x + idx++);
    t2 = get_clk();
    x_tmp1++;
    x_tmp2++;
    x_tmp3++;
    x_tmp4++;
    x_tmp5++;
    x_tmp6++;
    x_tmp7++;
    x_tmp8++;
    hthread_printf("mem -> reg, clocks total: %lu\n", t2 - t1);

    // idx += n;
    // t1 = get_clk();
    // x_tmp = *(x + idx--);
    // x_tmp = *(x + idx--);
    // x_tmp = *(x + idx--);
    // x_tmp = *(x + idx--);
    // t2 = get_clk();

    // hthread_printf("mem -> reg, clocks total: %lu\n", t2 - t1);

    scalar_free(sm_mem);
}
