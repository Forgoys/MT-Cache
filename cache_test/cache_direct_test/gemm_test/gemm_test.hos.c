#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <omp.h>
#include "hthread_host.h"
#include "util.h"

void init_rst(unsigned long n, long *rst)
{
    for (unsigned long i = 0; i < n; i++)
    {
        rst[i] = 0.0;
    }
}

// gemm use openmp
void gemm_serial(long *A, long *B, long *C, int length)
{
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length; j++)
        {
            for (int k = 0; k < length; k++)
            {
                C[i * length + j] += A[i * length + k] * B[k * length + j];
            }
        }
    }
}

void gemm_parallel(long *A, long *B, long *C, int N, int num_threads)
{
    omp_set_num_threads(num_threads);

#pragma omp parallel for collapse(2)
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            float sum = 0.0;
            for (int k = 0; k < N; ++k)
            {
                sum += A[i * N + k] * B[k * N + j];
            }
            C[i * N + j] = sum;
        }
    }
}

// check C and C_ans
bool check(long *C, long *C_ans, int length)
{
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length; j++)
        {
            if (C[i * length + j] != C_ans[i * length + j])
            {
                printf("C[%d][%d] = %ld, C_ans[%d][%d] = %ld\n", i, j, C[i * length + j], i, j, C_ans[i * length + j]);
                return false;
            }
        }
    }
    return true;
}

#define TEST_ROUND 5

double cpu_test(int n, long *A, long *B, long *C)
{
    uint64_t time_sum = 0;
    uint64_t time;
    gemm_serial(A, B, C, n);
    printf("Warm up finished\n");
    init_rst(n, C);
    for (int i = 0; i < TEST_ROUND; i++)
    {
        time = getCurrentTimeMicros();
        gemm_serial(A, B, C, n);
        time = getCurrentTimeMicros() - time;
        init_rst(n, C);
        time_sum += time;
        printf("Round %d finished\n", i);
    }
    return (double)time_sum / TEST_ROUND / 1e6;
}

double dsp_test(int clusterId, char *kernel, int nthreads, int n, int tileSize, long *A, long *B, long *C)
{
    unsigned long args[5];
    args[0] = n;
    args[1] = tileSize;
    args[2] = (unsigned long)A;
    args[3] = (unsigned long)B;
    args[4] = (unsigned long)C;

    // 预热
    uint64_t time_sum = 0;
    uint64_t time = 0;
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 2, 3, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    hthread_group_wait(threadId);
    init_rst(n, C);
    hthread_group_destroy(threadId);
    printf("Warm up finished\n");

    // 开始测试
    for (int i = 0; i < TEST_ROUND; i++)
    {
        time = getCurrentTimeMicros();
        int threadId = hthread_group_create(clusterId, nthreads, kernel, 2, 3, args);
        if (threadId == -1)
        {
            M_logError("Failed to create threads with %s", kernel);
            return 2;
        }
        hthread_group_wait(threadId);
        time = getCurrentTimeMicros() - time;
        init_rst(n, C);
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
    int n = 1024;
    char *devProgram = "./gemm_test.dev.dat";
    int nthreads = 24;
    char *kernel = "gemm_cache_test_kernel";
    int tileSize = 16;
    if (argc > 1)
    {
        n = atoi(argv[1]);
    }
    if (argc > 2)
    {
        tileSize = atoi(argv[2]);
    }
    if (argc > 3)
    {
        nthreads = atoi(argv[3]);
    }

    printf("=========================\n");
    printf("Matrix: %d x %d\n", n, n);
    printf("Tile size: %d x %d\n", tileSize, tileSize);
    printf("Number of cores: %d\n", nthreads);
    printf("=========================\n");

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

    // 分配空间
    long *A = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RO);
    M_checkMalloc(A);
    long *B = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RO);
    M_checkMalloc(B);
    long *C = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RW);
    M_checkMalloc(C);
    long *C_ans = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RW);
    M_checkMalloc(C_ans);

    // 初始化数据
    for (int i = 0; i < n * n; i++)
    {
        A[i] = rand() % 100;
        B[i] = rand() % 100;
        C[i] = 0;
        C_ans[i] = 0;
    }

    // gold and check
    gemm_parallel(A, B, C_ans, n, 16);
    // launch dev kernel
    unsigned long args[5];
    args[0] = n;
    args[1] = tileSize;
    args[2] = (unsigned long)A;
    args[3] = (unsigned long)B;
    args[4] = (unsigned long)C;
    kernel = "gemm_cache_test_kernel";
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 2, 3, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    hthread_group_wait(threadId);
    hthread_group_destroy(threadId);
    
    bool correct = check(C, C_ans, n);

    // output result
    if (!correct)
    {
        fprintf(stderr, "Failed to test gemm!\n");
        return 1;
    }
    else
    {
        printf("Results correct, start test!\n");
    }

    init_rst(n, C);

    printf("===CPU test start!===\n");
    double cpu_time = cpu_test(n, A, B, C_ans);
    printf("===CPU test finished!===\n");
    printf("===DSP mem test start!===\n");
    double dev_mem_time = dsp_test(clusterId, "gemm_mem_test_kernel", nthreads, n, tileSize, A, B, C);
    printf("===DSP mem test finished!===\n");
    printf("===DSP cache test start!===\n");
    double dev_cache_time = dsp_test(clusterId, "gemm_cache_test_kernel", nthreads, n, tileSize, A, B, C);
    printf("===DSP cache test finished!===\n\n");
    printf("CPU Time: %lf seconds\n", cpu_time);
    printf("Direct memory access, Time: %lf seconds\n", dev_mem_time);
    printf("Access through cache, Time: %lf seconds\n", dev_cache_time);

    // fini
    if (A != NULL)
        hthread_free(A);
    if (B != NULL)
        hthread_free(B);
    if (C != NULL)
        hthread_free(C);
    if (C_ans != NULL)
        hthread_free(C_ans);

    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);
}
