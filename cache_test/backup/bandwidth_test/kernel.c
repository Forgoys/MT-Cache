#include <stdint.h>

#include <compiler/m3000.h>
#include "hthread_device.h"

#define BUFFER_SIZE 61440
#define HZ 4150e6
unsigned long test_data_size = 0;

__global__ double get_time(unsigned long t1, unsigned long t2)
{
    return (double)(t2 - t1) / HZ;
}

__global__ double get_bandwidth(unsigned long n, unsigned long t1, unsigned long t2)
{
    return (double)(n) / get_time(t1, t2) / 1024 / 1024 / 1024;
}

__global__ void dma_p2p_mem2sm(unsigned long n, double* x)
{
    int idx = get_thread_id();
    int off = idx * BUFFER_SIZE;
    if (off >= n * sizeof(double))
    {
        return;
    }
    double *buffer = (double*)scalar_malloc(BUFFER_SIZE);
    int dma_no = dma_p2p(x + off, 1, BUFFER_SIZE, 0, buffer, 1, BUFFER_SIZE, 0, false, 0);
    dma_wait(dma_no);
    scalar_free(buffer);
}

__global__ void scalar_load_mem2sm(unsigned long n, double* x)
{
    int idx = get_thread_id();
    int off = idx * BUFFER_SIZE;
    if (off >= n * sizeof(double))
    {
        return;
    }
    double *buffer = (double*)scalar_malloc(BUFFER_SIZE);
    scalar_load(x + off, buffer, BUFFER_SIZE);
    scalar_free(buffer);
}

__global__ void sm2reg(unsigned long n, double* x_buffer)
{
    double x_tmp = 0;
    for (int i = 0; i < n / sizeof(double); i++)
    {
        x_tmp = x_buffer[i];
        x_tmp++;
    }
}

__global__ void mem2reg(unsigned long n, double* x)
{
    double x_tmp = 0;
    for (int i = 0; i < n / sizeof(double); i++)
    {
        x_tmp = x[i];
        x_tmp++;
    }
}

__global__ void bandwidth_test_kernel(unsigned long n, double* x)
{
    double *buffer = (double*)scalar_malloc(BUFFER_SIZE);
    unsigned long t1, t2;
    n = BUFFER_SIZE;
    
    t1 = get_clk();
    // dma_p2p_mem2sm(n, x, buffer);
    t2 = get_clk();
    hthread_printf("[mem -> SM] (dma_p2p): %lfGB/s\n", get_bandwidth(n, t1, t2));

    t1 = get_clk();
    // scalar_load_mem2sm(n, x, buffer);
    t2 = get_clk();
    hthread_printf("[mem -> SM] (scalar_load): %lfGB/s\n", get_bandwidth(n, t1, t2));

    t1 = get_clk();
    sm2reg(n, x);
    t2 = get_clk();
    hthread_printf("[SM -> register] (direct): %lfGB/s\n", get_bandwidth(n, t1, t2));

    t1 = get_clk();
    mem2reg(n, x);
    t2 = get_clk();
    hthread_printf("[mem -> register] (direct): %lfGB/s\n", get_bandwidth(n, t1, t2));


    scalar_free(buffer);
}