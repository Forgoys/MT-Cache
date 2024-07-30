#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <omp.h>
#include "hthread_host.h"
#include "util.h"

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


int main(int argc, char **argv)
{

    if (argc < 3)
    {
        printf("Useage: ~ length tileSize coreNum\n");
        return -1;
    }

    int length = atoi(argv[1]);
    int tileSize = atoi(argv[2]);
    int coreNum = atoi(argv[3]);

    printf("=========================\n");
    printf("Matrix: %d x %d\n", length, length);
    printf("Tile size: %d x %d\n", tileSize, tileSize);
    printf("Number of cores: %d\n", coreNum);
    printf("=========================\n");

    // 开DSP，载入dat文件
    int cluster_id = 0;
    int retc = hthread_dev_open(cluster_id);
    M_checkRetC(retc, "hthread_dev_open");
    retc = hthread_dat_load(cluster_id, "kernel.dat");
    M_checkRetC(retc, "hthread_dat_load");
    int availThreads = hthread_get_avail_threads(cluster_id);
    if (coreNum > availThreads)
    {
        M_logError("number of threads is overflow : avail threads is %d, "
                   "actual threads is %d",
                   availThreads, coreNum);
        retc = hthread_dat_unload(cluster_id);
        retc = hthread_dev_close(cluster_id);
        return 2;
    }

    // 分配空间
    long *A = hthread_malloc(cluster_id, length * length * sizeof(long), HT_MEM_RO);
    M_checkMalloc(A);
    long *B = hthread_malloc(cluster_id, length * length * sizeof(long), HT_MEM_RO);
    M_checkMalloc(B);
    long *C = hthread_malloc(cluster_id, length * length * sizeof(long), HT_MEM_RW);
    M_checkMalloc(C);
    long *C_ans = hthread_malloc(cluster_id, length * length * sizeof(long), HT_MEM_RW);
    M_checkMalloc(C_ans);

    // 初始化数据
    for (int i = 0; i < length * length; i++)
    {
        A[i] = rand() % 100;
        B[i] = rand() % 100;
        C[i] = 0;
        C_ans[i] = 0;
    }

    // 载入内核
    int barrierNo = hthread_barrier_create(cluster_id);
    unsigned long args[6];
    args[0] = length;
    args[1] = tileSize;
    args[2] = barrierNo;
    args[3] = (unsigned long)A;
    args[4] = (unsigned long)B;
    args[5] = (unsigned long)C;

    // 计时和执行串行版本
    uint64_t time = getCurrentTimeMicros();
    gemm_serial(A, B, C_ans, length);
    time = getCurrentTimeMicros() - time;
    printf("Serial time: %fs\n", time / 1e6);

    // 计时和执行并行版本
    time = getCurrentTimeMicros();
    gemm_parallel(A, B, C_ans, length, 16);
    time = getCurrentTimeMicros() - time;
    printf("Parallel using OpenMP time: %fs\n", time / 1e6);

    // 计时和执行DSP版本
    time = getCurrentTimeMicros();
    // 创建线程组
    int thread_id = hthread_group_create(cluster_id, coreNum);
    // 在线程组上执行函数
    retc = hthread_group_exec(thread_id, "gemm", 3, 3, args);
    M_checkRetC(retc, "hthread_group_exec");
    // 等待执行完成
    retc = hthread_group_wait(thread_id);
    M_checkRetC(retc, "hthread_group_wait");
    time = getCurrentTimeMicros() - time;
    printf("Parallel using DSP time: %fs\n", time / 1e6);

    bool isRight = check(C, C_ans, length);
    if (isRight)
    {
        printf("Result is correct\n");
    }
    else
    {
        printf("Result is wrong\n");
    }

    hthread_barrier_destroy(cluster_id);

    hthread_group_destroy(thread_id);
    hthread_dev_close(cluster_id);

    return 0;
}
