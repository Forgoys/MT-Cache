/**
 * daxpy.hos.c
 * @Author      : jshen, longbiao
 * @Date        : 2022-06-27
 * @Project     : MT3000 daxpy
 * @description : example implementation of daxpy on MT3000
 *                y = x * alph + y
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include "util.h"
#include "hthread_host.h"

void init_rst(unsigned long n, double *rst)
{
    for (unsigned long i = 0; i < n; i++)
    {
        rst[i] = 0.0;
    }
}

void daxpy_cpu(int n, double a, double *x, double *y)
{
    for (int i = 0; i < n; ++i)
    {
        y[i] = a * x[i] + y[i];
    }
}

int check_daxpy(int n, double *y_gold, double *y)
{
    int errNum = 0;
    for (int i = 0; i < n; ++i)
    {
        if (y[i] != y_gold[i])
        {
            fprintf(stderr, "%d-data error : %le != %le\n",
                    i, y[i], y_gold[i]);
            errNum++;
            if (errNum >= 100)
                break;
        }
    }
    return errNum;
}

#define TEST_ROUND 5

double cpu_test(unsigned long n, double alph, double *x, double *y)
{
    uint64_t time_sum = 0;
    uint64_t time;
    daxpy_cpu(n, alph, x, y);
    printf("Warm up finished\n");
    init_rst(n, y);
    for (int i = 0; i < TEST_ROUND; i++)
    {
        time = getCurrentTimeMicros();
        daxpy_cpu(n, alph, x, y);
        time = getCurrentTimeMicros() - time;
        init_rst(n, y);
        time_sum += time;
        printf("Round %d finished\n", i);
    }
    return (double)time_sum / TEST_ROUND / 1e6;
}

double dsp_test(int clusterId, char *kernel, int nthreads, unsigned long n, double alph, double *x, double *y)
{
    uint64_t args[4];
    args[0] = (uint64_t)n;
    args[1] = (uint64_t)doubleToRawBits(alph);
    args[2] = (uint64_t)x;
    args[3] = (uint64_t)y;

    // 预热
    uint64_t time_sum = 0;
    uint64_t time = 0;
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 2, 2, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    hthread_group_wait(threadId);
    init_rst(n, y);
    hthread_group_destroy(threadId);
    printf("Warm up finished\n");

    // 开始测试
    for (int i = 0; i < TEST_ROUND; i++)
    {
        time = getCurrentTimeMicros();
        int threadId = hthread_group_create(clusterId, nthreads, kernel, 2, 2, args);
        if (threadId == -1)
        {
            M_logError("Failed to create threads with %s", kernel);
            return 2;
        }
        hthread_group_wait(threadId);
        time = getCurrentTimeMicros() - time;
        init_rst(n, y);
        hthread_group_destroy(threadId);
        time_sum += time;
        printf("Round %d finished\n", i);
    }

    return (double)time_sum / TEST_ROUND / 1e6;
}

int main(int argc, char **argv)
{
    int retc;
    int clusterId = 0;
    int n = 1024 * 1024 * 100;
    double alph = 3.14;
    char *devProgram = "./daxpy_l1_direct.dev.dat";
    int nthreads = 24;
    char *kernel = "daxpy_cache_test_kernel";
    if (argc > 1)
    {
        n = atoi(argv[1]);
    }
    if (argc > 2)
    {
        nthreads = atoi(argv[2]);
    }

    // check args
    if (nthreads <= 0)
    {
        M_logError("invalid nthreads : %d", nthreads);
        return 2;
    }
    if (fileIsExist(devProgram) != 0)
    {
        M_logError("%s : No such file or directory", devProgram);
        return 2;
    }

    // device init
    retc = hthread_dev_open(clusterId);
    M_checkRetC(retc, hthread_dev_open);
    retc = hthread_dat_load(clusterId, devProgram);
    M_checkRetC(retc, hthread_dat_load);
    int availThreads = hthread_get_avail_threads(clusterId);
    if (nthreads > availThreads)
    {
        M_logError("number of threads is overflow : avail threads is %d, "
                   "actual threads is %d",
                   availThreads, nthreads);
        retc = hthread_dat_unload(clusterId);
        retc = hthread_dev_close(clusterId);
        return 2;
    }

    // malloc
    size_t bufSize = (size_t)n * sizeof(double);
    double *x = (double *)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(x);
    double *y = (double *)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(y);
    double *y_gold = (double *)malloc(bufSize);
    M_checkMalloc(y_gold);

    // init data
    size_t i = 0;
    for (i = 0; i < (size_t)n; i++)
    {
        x[i] = 0.0 + 1.0 * rand() / RAND_MAX * (6.28 - 0.0);
        y[i] = 0.0;
        y_gold[i] = 0.0;
    }

    // gold and check
    daxpy_cpu(n, alph, x, y_gold);

    // launch dev kernel
    uint64_t args[4];
    args[0] = (uint64_t)n;
    args[1] = (uint64_t)doubleToRawBits(alph);
    args[2] = (uint64_t)x;
    args[3] = (uint64_t)y;
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 2, 2, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    hthread_group_wait(threadId);
    hthread_group_destroy(threadId);

    int errNum = check_daxpy(n, y_gold, y);

    // output result
    if (errNum != 0)
    {
        fprintf(stderr, "Failed to test daxpy!\n");
        return 1;
    }
    else
    {
        printf("Results correct, start test!\n");
    }

    init_rst(n, y);
    printf("===CPU test start!===\n");
    double cpu_time = cpu_test(n, alph, x, y);
    printf("===CPU test finished!===\n");
    printf("===DSP mem test start!===\n");
    double dev_mem_time = dsp_test(clusterId, "daxpy_mem_test_kernel", nthreads, n, alph, x, y);
    printf("===DSP mem test finished!===\n");
    printf("===DSP cache test start!===\n");
    double dev_cache_time = dsp_test(clusterId, "daxpy_cache_test_kernel", nthreads, n, alph, x, y);
    printf("===DSP cache test finished!===\n\n");
    printf("CPU Time: %lf seconds\n", cpu_time);
    printf("Direct memory access, Time: %lf seconds\n", dev_mem_time);
    printf("Access through cache, Time: %lf seconds\n", dev_cache_time);

    // fini
    if (x != NULL)
        hthread_free(x);
    if (y != NULL)
        hthread_free(y);
    if (y_gold != NULL)
        free(y_gold);

    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);
}
