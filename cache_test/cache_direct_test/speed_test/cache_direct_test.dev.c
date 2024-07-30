#include <stdint.h>

#include <compiler/m3000.h>
#include "hthread_device.h"
#include "cache_direct.h"

__global__ void cache_direct_correct_test_kernel(unsigned long n, double a, char *x, char *y)
{
    char tmp = 0;
    CACHEe_ENV();
    CACHEe_INIT(x, char, 7, 7);
    for (unsigned long i = 0; i < n; i++)
    {
        CACHEe_SEC_R_RD(x, x + i, tmp, char);
        tmp = 100;
        CACHEe_SEC_W_RD(x, x + i, tmp, char);
        tmp = 0;
        CACHEe_SEC_R_RD(x, x + i, tmp, char);
        if (tmp != 100)
        {
            hthread_printf("cache read error: x[%lu] = %d.\n", i, tmp);
            break;
        }
    }
    CACHEe_FLUSH(x, char);
    unsigned long count = 0;
    for (unsigned long i = 0; i < n; i++)
    {
        if (x[i] != 100)
        {
            if (count == 0) 
            {
                hthread_printf("Error: x[%lu] = %d.\n", i, x[i]);
            }
            count++;
        }
    }
    hthread_printf("Error count: %lu.\n", count);
}

__global__ void cache_direct_speed_test_kernel(unsigned long n, double a, char *x, char *y)
{
    unsigned long t1, t2;
    char tmp = 0;
    unsigned long cache_result = 0;
    unsigned long mem_result = 0;

    CACHEe_ENV();
    CACHEe_INIT(x, char, 7, 7);

    // 测试缓存读取时间
    t1 = get_clk();
    for (unsigned long i = 0; i < n; i++)
    {
        CACHEe_SEC_R_RD(x, x + i, tmp, char);
        cache_result += tmp;
        // tmp++;
    }
    t2 = get_clk();
    unsigned long cache_clocks = t2 - t1;

    // 测试内存读取时间
    t1 = get_clk();
    for (unsigned long i = 0; i < n; i++)
    {
        tmp = x[i];
        mem_result += tmp;
        // tmp++;
    }
    t2 = get_clk();
    unsigned long mem_clocks = t2 - t1;

    // 使用结果变量防止优化
    y[0] = cache_result;
    y[1] = mem_result;

    // 输出时间
    hthread_printf("cache clocks: %lu, mem clocks: %lu, speed: %lf.\n", cache_clocks, mem_clocks, (double)mem_clocks / cache_clocks);
}
