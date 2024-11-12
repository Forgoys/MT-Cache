#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <omp.h>
#include "hthread_host.h"
#include "util.h"
#include "csv_handler.h"

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

#define TEST_ROUND 2

double cpu_test_serial(int n, long *A, long *B, long *C)
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

double cpu_test_parallel(int n, long *A, long *B, long *C)
{
    uint64_t time_sum = 0;
    uint64_t time;
    gemm_parallel(A, B, C, n, 16);
    printf("Warm up finished\n");
    init_rst(n, C);
    for (int i = 0; i < TEST_ROUND; i++)
    {
        time = getCurrentTimeMicros();
        gemm_parallel(A, B, C, n, 16);
        time = getCurrentTimeMicros() - time;
        init_rst(n, C);
        time_sum += time;
        printf("Round %d finished\n", i);
    }
    return (double)time_sum / TEST_ROUND / 1e6;
}

double dsp_test(int clusterId, char *kernel, int nthreads, int n, int tileSize, long *A, long *B, long *C, int cets, int lines)
{
    unsigned long args[7];
    args[0] = n;
    args[1] = tileSize;
    args[2] = cets;
    args[3] = lines;
    args[4] = (unsigned long)A;
    args[5] = (unsigned long)B;
    args[6] = (unsigned long)C;

    // 预热
    uint64_t time_sum = 0;
    uint64_t time = 0;
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 4, 3, args);
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
        int threadId = hthread_group_create(clusterId, nthreads, kernel, 4, 3, args);
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
    int clusterId = 2;
    int n = 1024;
    char *devProgram = "./gemm_test_tuning.dev.dat";
    int nthreads = 24;
    char *kernel = "gemm_cache_test_kernel";
    int tileSize = 16;
    char* csvFileName = "results.csv";
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
    if (argc > 4)
    {
        csvFileName = argv[4];
    }

    printf("=========================\n");
    printf("Matrix: %d x %d\n", n, n);
    printf("Tile size: %d x %d\n", tileSize, tileSize);
    printf("Number of cores: %d\n", nthreads);
    printf("=========================\n");

    // 检查参数
    if (nthreads <= 0)
    {
        M_logError("Invalid number of threads: %d", nthreads);
        return 2;
    }
    if (fileIsExist(devProgram) != 0)
    {
        M_logError("No such file or directory: %s", devProgram);
        return 2;
    }

    printf("Device initialization started.\n");

    // ==============================设备初始化====================================
    bool clusterFound = false;
    for (clusterId = 0; clusterId <= 3; clusterId++)
    {
        printf("Checking clusterId %d...\n", clusterId);
        retc = hthread_dev_open(clusterId);
        if (retc != 0)
        {
            printf("Failed to open device on clusterId %d\n", clusterId);
            continue; // 尝试下一个 clusterId
        }

        retc = hthread_dat_load(clusterId, devProgram);
        if (retc != 0)
        {
            printf("Failed to load data on clusterId %d\n", clusterId);
            hthread_dev_close(clusterId); // 关闭设备后继续
            continue;
        }

        int availThreads = hthread_get_avail_threads(clusterId);
        if (nthreads <= availThreads)
        {
            printf("Sufficient threads available on clusterId %d: available = %d, required = %d\n", clusterId, availThreads, nthreads);
            clusterFound = true;
            break; // 找到合适的 clusterId，跳出循环
        }
        else
        {
            printf("Not enough threads on clusterId %d: available = %d, required = %d\n", clusterId, availThreads, nthreads);
            hthread_dat_unload(clusterId);
            hthread_dev_close(clusterId); // 关闭设备后继续
        }
    }

    if (!clusterFound)
    {
        printf("No suitable clusterId found with enough threads. Exiting program.\n");
        return 2;
    }
    printf("Device initialized successfully.\n");

    // ==============================分配空间====================================
    printf("Allocating memory for matrices.\n");
    long *A = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RO);
    M_checkMalloc(A);
    long *B = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RO);
    M_checkMalloc(B);
    long *C = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RW);
    M_checkMalloc(C);
    long *C_ans = hthread_malloc(clusterId, n * n * sizeof(long), HT_MEM_RW);
    M_checkMalloc(C_ans);

    // ==============================初始化数据====================================
    printf("Initializing matrix data.\n");
    for (int i = 0; i < n * n; i++)
    {
        A[i] = rand() % 100;
        B[i] = rand() % 100;
        C[i] = 0;
        C_ans[i] = 0;
    }

    // ==============================正确性验证====================================
    printf("Starting correctness verification.\n");
    gemm_parallel(A, B, C_ans, n, 16); // 金标准执行

    printf("Launching device kernel for GEMM.\n");
    unsigned long args[7] = {n, tileSize, 7, 7, (unsigned long)A, (unsigned long)B, (unsigned long)C};
    kernel = "gemm_mem_test_kernel";
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 4, 3, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    hthread_group_wait(threadId);
    hthread_group_destroy(threadId);

    bool correct = check(C, C_ans, n);
    // 输出结果
    if (!correct)
    {
        fprintf(stderr, "GEMM test failed!\n");
        return 1;
    }
    else
    {
        printf("Results correct, starting performance test.\n");
    }

    init_rst(n, C); // Resetting C for performance test

    // ==============================正式测试====================================
    printf("=== Starting CPU test ===\n");
    double cpu_time = cpu_test_parallel(n, A, B, C_ans);
    printf("CPU Time: %lf seconds\n", cpu_time);
    printf("=== CPU test finished ===\n");

    printf("Opening CSV file for data recording.\n");
    CSVFile *csv = open_csv(csvFileName, "w");
    if (csv == NULL)
    {
        perror("Failed to open file");
        return 1;
    }

    // 写入 CSV 表头
    write_header(csv, 3, "Set", "Sum", "SpeedUp");
    int sum, cets, lines;
    double baseline_time, dev_time, speed_up;

    printf("Start baseline test.\n");
    baseline_time = dsp_test(clusterId, "gemm_mem_test_kernel", nthreads, n, tileSize, A, B, C, 0, 0);
    printf("Baseline test done.\n");
    int sum_begin = 11, sum_end = 14;
    // 数据收集和记录
    for (sum = sum_begin; sum <= sum_end; sum++)
    {
        for (cets = 1; cets <= sum - 5; cets++)
        {
            lines = sum - cets;
            dev_time = dsp_test(clusterId, "gemm_cache_test_kernel", nthreads, n, tileSize, A, B, C, cets, lines);
            speed_up = baseline_time / dev_time;

            // 转换 double 为字符串
            char str_speed_up[20];
            sprintf(str_speed_up, "%f", speed_up);

            // 写入一行数据
            write_row(csv, 3, itoa(cets, 10), itoa(sum, 10), str_speed_up);

            // 输出进度
            printf("Processed Set %d, Sum %d, Speed Up: %.2f\n", cets, sum, speed_up);
        }
    }

    printf("All tests completed, closing CSV file.\n");
    // 关闭 CSV 文件
    close_csv(csv);

    // ==============================释放资源====================================
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
    retc = hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);

    printf("Device closed and resources freed.\n");
}