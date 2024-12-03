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

#define SEGMENT_SIZE 7680 // (2048 * 8) / 1024 = 16KB

__global__ void sm_test(char* sm_buffer, int n)
{
    char tmp;
    for (int i = 0; i < n; i++)
    {
        tmp = sm_buffer[i];
        tmp++;
    }
}

__global__ void mem_test(char* mem_buffer, int n)
{
    char tmp;
    for (int i = 0; i < n; i++)
    {
        tmp = mem_buffer[i];
        tmp++;
    }
}

__global__ void daxpy_kernel(uint64_t nn, double a, char *x, char *y)
{
    unsigned long t1, t2;
    unsigned long dma_p2p_clocks, scalar_load_clocks, sm_test_clocks, mem_test_clocks;

    char *sm_buffer = scalar_malloc(SEGMENT_SIZE * sizeof(char));
    int dma_no = 0;

    // 打印表头
    hthread_printf("n, dma_p2p_clocks, scalar_load_clocks, sm_test_clocks, mem_test_clocks\n");

    for (int n = 8; n < 128; n *= 2)
    {
        t1 = get_clk();
        dma_no = dma_p2p(x, 1, n * sizeof(char), 0, sm_buffer, 1, n * sizeof(char), 0, false, 0);
        dma_wait(dma_no);
        t2 = get_clk();
        dma_p2p_clocks = t2 - t1;

        t1 = get_clk();
        scalar_load(x, sm_buffer, n * sizeof(char));
        t2 = get_clk();
        scalar_load_clocks = t2 - t1;

        t1 = get_clk();
        sm_test(sm_buffer, n);
        t2 = get_clk();
        sm_test_clocks = t2 - t1;

        t1 = get_clk();
        mem_test(x, n);
        t2 = get_clk();
        mem_test_clocks = t2 - t1;

        // 打印每次测试的结果
        hthread_printf("%d, %lu, %lu, %lu, %lu\n", n, dma_p2p_clocks, scalar_load_clocks, sm_test_clocks, mem_test_clocks);
    }

    for (int n = 128; n <= SEGMENT_SIZE; n += 64)
    {
        t1 = get_clk();
        dma_no = dma_p2p(x, 1, n * sizeof(char), 0, sm_buffer, 1, n * sizeof(char), 0, false, 0);
        dma_wait(dma_no);
        t2 = get_clk();
        dma_p2p_clocks = t2 - t1;

        t1 = get_clk();
        scalar_load(x, sm_buffer, n * sizeof(char));
        t2 = get_clk();
        scalar_load_clocks = t2 - t1;

        t1 = get_clk();
        sm_test(sm_buffer, n);
        t2 = get_clk();
        sm_test_clocks = t2 - t1;

        t1 = get_clk();
        mem_test(x, n);
        t2 = get_clk();
        mem_test_clocks = t2 - t1;

        // 打印每次测试的结果
        hthread_printf("%d, %lu, %lu, %lu, %lu\n", n, dma_p2p_clocks, scalar_load_clocks, sm_test_clocks, mem_test_clocks);
    }

    scalar_free(sm_buffer);
}
