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
#include "cache_set_simd.h"

#define SEGMENT_SIZE 7680 // (2048 * 8) / 1024 = 16KB



// __global__ void mem_sm_test(char *x, char *sm_buffer, int n)
// {
//     scalar_load(x, sm_buffer, n * sizeof(char));
//     char tmp;
//     for (int i = 0; i < n; i++)
//     {
//         tmp = sm_buffer[i];
//         tmp++;
//     }
// }

// 模拟组相连DMA通信与读数据的性能
__global__ void mem_sm_set_test(char *x, char *sm_buffer, int n, int line_size)
{
    for (int i = 0; i < n; i += line_size)
    {
        scalar_load(x + i, sm_buffer, line_size * sizeof(char));
        char tmp;
        for (int j = 0; j < line_size; j++)
        {
            tmp = sm_buffer[j];
            tmp++;
        }
    }
}

__global__ unsigned long set_test(char *x, int n)
{
    unsigned long t1, t2;
    // t1 = get_clk();
    CACHEe_ENV();
    // t2 = get_clk();
    // hthread_printf("CACHEe_ENV: %lu\n", t2 - t1);
    // t1 = get_clk();
    CACHEe_INIT(x, char, 4, 2, 6, 0, 0, x, n * sizeof(char));
    // t2 = get_clk();
    // hthread_printf("CACHEe_INIT: %lu\n", t2 - t1);
    t1 = get_clk();
    char tmp;
    for (int i = 0; i < n; i += 2)
    {
        tmp = CACHEe_SEC_R_RD(x, x + i, tmp, char);
        tmp++;
    }
    t2 = get_clk();
    // hthread_printf("CACHEe_SEC_R_RD: %lu\n", t2 - t1);
    // t1 = get_clk();
    CACHEe_INVALID(x, x, n * sizeof(char));
    // t2 = get_clk();
    // hthread_printf("CACHEe_INVALID: %lu\n", t2 - t1);
    return t2 - t1;
}

__global__ unsigned long set_simd_test(char *x, int n)
{
    unsigned long t1, t2;
    // t1 = get_clk();
    CACHEe_ENV_smid();
    // t2 = get_clk();
    // hthread_printf("CACHEe_ENV: %lu\n", t2 - t1);
    // t1 = get_clk();
    CACHEe_INIT_simd(x, char, 4, 2, 6, 0, 0, x, n * sizeof(char));
    // t2 = get_clk();
    // hthread_printf("CACHEe_INIT: %lu\n", t2 - t1);
    t1 = get_clk();
    char tmp;
    for (int i = 0; i < n; i += 2)
    {
        tmp = CACHEe_SEC_R_RD_simd(x, x + i, tmp, char);
        tmp++;
    }
    t2 = get_clk();
    // hthread_printf("CACHEe_SEC_R_RD: %lu\n", t2 - t1);
    // t1 = get_clk();
    CACHEe_INVALID_simd(x, x, n * sizeof(char));
    // t2 = get_clk();
    // hthread_printf("CACHEe_INVALID: %lu\n", t2 - t1);
    return t2 - t1;
}

__global__ void sm_test(char *sm_buffer, int n)
{
    char tmp;
    for (int i = 0; i < n; i++)
    {
        tmp = sm_buffer[i];
        tmp++;
    }
}
__global__ void mem_test(char *mem_buffer, int n)
{
    char tmp;
    for (int i = 0; i < n; i++)
    {
        tmp = mem_buffer[i];
        tmp++;
    }
}
__global__ void mem_sm_test(uint64_t nn, double a, char *x, char *y)
{
    int size = 60*1024 / sizeof(double);
    double size_GB = (double)size / 1024 / 1024 /1024;
    double* sm_buffer = scalar_malloc(size * sizeof(double));
    scalar_load(x, sm_buffer, size);

    unsigned long t1, t2;
    // t1 = get_clk();
    // sm_test(sm_buffer, size);
    // t2 = get_clk();
    // unsigned long sm_clocks = t2 - t1;
    // double sm_s = (double)sm_clocks / (4150 * 1e6);
    // hthread_printf("sm_test: %lu, %lfGB/s\n", sm_clocks, (size_GB / sm_s));

    // t1 = get_clk();
    // mem_test(x, size);
    // t2 = get_clk();
    // unsigned long mem_clocks = t2 - t1;
    // double mem_s = (double)mem_clocks / (4150 * 1e6);
    // hthread_printf("mem_test: %lu, %lfGB/s\n", mem_clocks, (size_GB / mem_s));
    
    double tmp;
    unsigned long total = 0;
    for (int i = 0; i < size; i++)
    {
        t1 = get_clk();
        tmp = sm_buffer[i];
        t2 = get_clk();
        total += t2 - t1;
    }
    hthread_printf("sm: %lu\n", total / size);

    for (int i = 0; i < size; i++)
    {
        t1 = get_clk();
        tmp = x[i];
        t2 = get_clk();
        total += t2 - t1;
    }
    hthread_printf("mem: %lu\n", total / size);

    // t1 = get_clk();
    // tmp = x[0];
    // t2 = get_clk();
    // hthread_printf("x[0]: %lu\n", t2 - t1);

    scalar_free(sm_buffer);
}

// 组相连API与直接读内存的性能对比测试
__global__ void set_mem_test_kernel(uint64_t nn, double a, char *x, char *y)
{
    unsigned long t1, t2;
    unsigned long mem_sm_test_clocks, scalar_load_clocks, set_test_clocks, mem_test_clocks;
    int max_n = 4096;
    int step = 128;
    // char *sm_buffer = scalar_malloc(max_n * sizeof(char));
    int dma_no = 0;
    // 打印表头
    // hthread_printf("n, set_test_clocks, mem_sm_test_clocks, percent, mem_test_clocks, speed_up\n");
    hthread_printf("n, set_test_clocks, mem_test_clocks, speed_up\n");
    // set_test_clocks = set_test(x, 8);
    // set_test_clocks = set_test(x, 8);
    for (int n = 8; n <= max_n; n = (n < step ? n * 2 : n + step))
    {
        // t1 = get_clk();
        // mem_sm_test(x, sm_buffer, n);
        // t2 = get_clk();
        // mem_sm_test_clocks = t2 - t1;

        // t1 = get_clk();
        set_test_clocks = set_test(x, n);
        // t2 = get_clk();
        // set_test_clocks = t2 - t1;
        double percent = (double)mem_sm_test_clocks / set_test_clocks;

        t1 = get_clk();
        mem_test(x, n);
        t2 = get_clk();
        mem_test_clocks = t2 - t1;
        double speed_up = (double)mem_test_clocks / set_test_clocks;
        // 打印每次测试的结果
        // hthread_printf("%d, %lu, %lu, %lf%%, %lu, %lf%%\n", n, set_test_clocks, mem_sm_test_clocks, percent, mem_test_clocks, speed_up);
        hthread_printf("%d, %lu, %lu, %lf%%\n", n, set_test_clocks, mem_test_clocks, speed_up);
    }
    // scalar_free(sm_buffer);
}

// 组相连API simd与直接读内存的性能对比测试
__global__ void set_simd_mem_test_kernel(uint64_t nn, double a, char *x, char *y)
{
    unsigned long t1, t2;
    unsigned long mem_sm_test_clocks, scalar_load_clocks, set_test_clocks, set_simd_test_clocks, mem_test_clocks;
    int max_n = 4096;
    int step = 128;
    // char *sm_buffer = scalar_malloc(max_n * sizeof(char));
    int dma_no = 0;
    // 打印表头
    // hthread_printf("n, set_test_clocks, mem_sm_test_clocks, percent, mem_test_clocks, speed_up\n");
    hthread_printf("n, set_simd_test_clocks, set_test_clocks, mem_test_clocks, speed_up\n");
    // set_test_clocks = set_test(x, 8);
    // set_test_clocks = set_test(x, 8);
    for (int n = 8; n <= max_n; n = (n < step ? n * 2 : n + step))
    {
        // t1 = get_clk();
        // mem_sm_test(x, sm_buffer, n);
        // t2 = get_clk();
        // mem_sm_test_clocks = t2 - t1;

        // t1 = get_clk();
        set_simd_test_clocks = set_simd_test(x, n);
        // t2 = get_clk();
        // set_test_clocks = t2 - t1;
        // double percent = (double)mem_sm_test_clocks / set_test_clocks;

        // t1 = get_clk();
        set_test_clocks = set_test(x, n);
        // t2 = get_clk();
        // set_test_clocks = t2 - t1;
        // double percent = (double)mem_sm_test_clocks / set_test_clocks;

        t1 = get_clk();
        mem_test(x, n);
        t2 = get_clk();
        mem_test_clocks = t2 - t1;
        double speed_up = (double)mem_test_clocks / set_test_clocks;
        // 打印每次测试的结果
        // hthread_printf("%d, %lu, %lu, %lf%%, %lu, %lf%%\n", n, set_test_clocks, mem_sm_test_clocks, percent, mem_test_clocks, speed_up);
        hthread_printf("%d, %lu,  %lu, %lu, %lf%%\n", n, set_simd_test_clocks, set_test_clocks, mem_test_clocks, speed_up);
    }
    // scalar_free(sm_buffer);
}


// 组相连API的每一步性能测试
__global__ void set_step_test_kernel(uint64_t nn, double a, char *x, char *y)
{
    unsigned long t1, t2;
    unsigned long mem_sm_test_clocks, scalar_load_clocks, set_test_clocks, mem_test_clocks;
    int max_n = 4096;
    int step = 128;
    CACHEe_ENV();
    CACHEe_INIT_DEBUG(x, char, 4, 2, 6, 0, 0, x, n * sizeof(char));
    PRINT_HEADER();
    for (int n = 8; n <= max_n; n = (n < step ? n * 2 : n + step))
    {
        // t1 = get_clk();
        char tmp;
        for (int i = 0; i < n; i += 2)
        {
            tmp = CACHEe_SEC_R_RD_DEBUG_SUM(x, x + i, tmp, char);
            tmp++;
        }
        // t2 = get_clk();
        PRINT_DATA(x, n);

        // t1 = get_clk();
        // for (int i = 0; i < n; i += 2)
        // {
        //     tmp = CACHEe_SEC_R_RD_DEBUG_SUM(x, x + i, tmp, char);
        //     tmp++;
        // }
        // t2 = get_clk();
        // hthread_printf("%lu\n", t2 - t1);
    }
}

// 模拟组相连DMA通信与读数据的性能，与直接读内存对比
__global__ void sm_set_mem_test_kernel(uint64_t nn, double a, char *x, char *y)
{
    unsigned long t1, t2;
    unsigned long mem_sm_set_test_clocks, scalar_load_clocks, set_test_clocks, mem_test_clocks;
    int max_n = 4096;
    int step = 128;
    int line_size = 64;
    char *sm_buffer = scalar_malloc(line_size * sizeof(char));
    int dma_no = 0;
    // 打印表头
    hthread_printf("n, mem_sm_set_test_clocks, mem_test_clocks, speed_up\n");
    // set_test_clocks = set_test(x, 8);
    // set_test_clocks = set_test(x, 8);
    for (int n = 8; n <= max_n; n = (n < step ? n * 2 : n + step))
    {
        t1 = get_clk();
        mem_sm_set_test(x, sm_buffer, n, line_size);
        t2 = get_clk();
        mem_sm_set_test_clocks = t2 - t1;

        t1 = get_clk();
        mem_test(x, n);
        t2 = get_clk();
        mem_test_clocks = t2 - t1;
        double speed_up = (double)mem_test_clocks / mem_sm_set_test_clocks;
        // 打印每次测试的结果
        // hthread_printf("%d, %lu, %lu, %lf%%, %lu, %lf%%\n", n, set_test_clocks, mem_sm_test_clocks, percent, mem_test_clocks, speed_up);
        hthread_printf("%d, %lu, %lu, %lf%%\n", n, mem_sm_set_test_clocks, mem_test_clocks, speed_up);
    }
    scalar_free(sm_buffer);
}

