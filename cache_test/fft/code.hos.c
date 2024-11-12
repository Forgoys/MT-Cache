#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include "hthread_host.h"
#include <stdbool.h>
// #include "mem.h"
#define EPSILON 1e-6 // 定义一个误差范围，用于比较浮点数
int fileIsExist(const char *filePath)
{
    return access(filePath, F_OK);
}

#define M_logError(_FMT, ...)                            \
    do                                                   \
    {                                                    \
        fprintf(stderr, "Error : " _FMT "in %d of %s\n", \
                __VA_ARGS__, __LINE__, __FILE__);        \
    } while (0);

#define M_checkRetC(_RETC, _MSG)                                    \
    do                                                              \
    {                                                               \
        if (_RETC != HT_SUCCESS)                                    \
        {                                                           \
            fprintf(stderr, "Failed to exec %s in %d of %s : %d\n", \
                    #_MSG, __LINE__, __FILE__, _RETC);              \
            return 2;                                               \
        }                                                           \
    } while (0);

#define M_checkMalloc(_PTR)                                      \
    do                                                           \
    {                                                            \
        if (_PTR == NULL)                                        \
        {                                                        \
            fprintf(stderr, "Failed to malloc %s in %d of %s\n", \
                    #_PTR, __LINE__, __FILE__);                  \
            return 2;                                            \
        }                                                        \
    } while (0);
typedef struct
{
    double real;
    double imag;
} complex;

double PI = 3.1415826; // 定义π

// 辅助函数
void add(complex a, complex b, complex *c)
{
    c->real = a.real + b.real;
    c->imag = a.imag + b.imag;
}

void sub(complex a, complex b, complex *c)
{
    c->real = a.real - b.real;
    c->imag = a.imag - b.imag;
}

void mul(complex a, complex b, complex *c)
{
    c->real = a.real * b.real - a.imag * b.imag;
    c->imag = a.real * b.imag + a.imag * b.real;
}

// 计算复数指数
void complex_exp(double theta, complex *W)
{
    W->real = cos(theta);
    W->imag = sin(theta);
}
void cpu_serial(uint64_t i, uint64_t n, complex *arr, complex *W, complex *result)
{
    uint64_t j = i;
    for (uint64_t k = i; k < n; k++)
    {
        complex temp;
        mul(W[k], arr[k], &temp);      // Multiply by twiddle factor
        add(arr[i], temp, &result[j]); // Store result
        j++;
    }
}

void fft_cpu_serial(uint64_t n, complex *arr, complex *W, complex *result)
{
    int threadId = 0;
    int threadsNum = 16;
    for (threadId = 0; threadId < threadsNum; threadId++)
    {
        uint64_t n_p = n / threadsNum; // Number of elements per thread
        uint64_t start = threadId * n_p;
        uint64_t end = (threadId == threadsNum - 1) ? n : start + n_p; // Handle remainder
        for (uint64_t i = start; i < end; i++)
        {
            cpu_serial(i, end, arr, W, result); // Call serial FFT for each element
        }
    }
}
/*
// 封装后的FFT函数
void fft_cpu_serial(uint64_t n, complex *arr, complex *W, complex *result)
{
    for (uint64_t i = 0; i < n; i++)
    {
        uint64_t j = 0;
        // 对每个元素执行FFT
        for (uint64_t k = i; k < n; k++)
        {
            complex temp;
            mul(W[k], arr[k], &temp);  // 乘以旋转因子
            add(arr[i], temp, &result[j]);  // 存储结果
            j++;
        }
    }
}*/
// 比较两个 complex 结构体是否相同（考虑浮点数精度问题）
bool compare_complex(complex a, complex b)
{
    return (fabs(a.real - b.real) < EPSILON) && (fabs(a.imag - b.imag) < EPSILON);
}

// 比较两个 complex 数组，返回 true 如果它们相同，false 如果不同
int compare_complex_arrays(complex *result, complex *result_gold, size_t n)
{
    int error = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (!compare_complex(result[i], result_gold[i]))
        {
            // printf("Difference found at index %lu: result = (%.6f + %.6fj), result_gold = (%.6f + %.6fj)\n",
            //  i, result[i].real, result[i].imag, result_gold[i].real, result_gold[i].imag);
            error++;
        }
    }
    return error;
}
uint64_t getCurrentTimeMicros()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}
int main(int argc, char **argv)
{
    unsigned long retc;
    unsigned long clusterId = 0;
    size_t base = 1; // 数据规模
    char *devProgram = "./code.dev.dat";
    unsigned long nthreads = 16;
    char *kernel1 = "fft_kernel";
    char *kernel2 = "fft_kernel_cache";
    uint64_t timeGold, timeDev, timeGoldsum = 0;
    if (argc > 1)
    {
        clusterId = (unsigned long)atoi(argv[1]);
    }
    if (argc > 2)
    {
        base = (unsigned long)atoi(argv[2]);
    }
    if (argc > 3)
    {
        nthreads = (unsigned long)atoi(argv[3]);
    }
    // size_t n = base * mul;
    // fprintf(stdout, "clusterId : %lu, n : %lu, devProgram : %s, nthreads : %lu\n",
    //         clusterId, n, devProgram, nthreads);
    if (nthreads <= 0)
    {
        M_logError("Invalid number of threads: %lu", nthreads);
        return 2;
    }
    if (fileIsExist(devProgram) != 0)
    {
        M_logError("No such file or directory: %s", devProgram);
        return 2;
    }
    printf("Device initialized successfully.\n");

    // ==============================设备初始化====================================
    bool clusterFound = false;
    for (clusterId = 0; clusterId <= 3; clusterId++)
    {
        printf("Checking clusterId %lu...\n", clusterId);
        retc = hthread_dev_open(clusterId);
        if (retc != 0)
        {
            printf("Failed to open device on clusterId %lu\n", clusterId);
            continue; // 尝试下一个 clusterId
        }

        retc = hthread_dat_load(clusterId, devProgram);
        if (retc != 0)
        {
            printf("Failed to load data on clusterId %lu\n", clusterId);
            hthread_dev_close(clusterId); // 关闭设备后继续
            continue;
        }

        int availThreads = hthread_get_avail_threads(clusterId);
        if (nthreads <= availThreads)
        {
            printf("Sufficient threads available on clusterId %lu: available = %d, required = %lu\n", clusterId, availThreads, nthreads);
            clusterFound = true;
            break; // 找到合适的 clusterId，跳出循环
        }
        else
        {
            printf("Not enough threads on clusterId %lu: available = %d, required = %lu\n", clusterId, availThreads, nthreads);
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
    // malloc

    for (int mul = 1; mul < 4; mul = mul * 2)
    {
        int base = 1024;
        char filename[50]; // To store the filename
        sprintf(filename, "%d*%d fft single.txt", mul, base);
        FILE *file = fopen(filename, "w");
        if (file == NULL)
        {
            printf("Error creating file!\n");
        }
        printf("Allocating memory for matrices.\n");
        size_t n = base * mul;
        size_t bufSize = (size_t)n * sizeof(complex);

        complex *x = (complex *)hthread_malloc(clusterId, bufSize, HT_MEM_RW);
        M_checkMalloc(x);
        // printf("从cpu来看，运行情况: x address = %d \n",x);
        complex *w = (complex *)hthread_malloc(clusterId, bufSize, HT_MEM_RW);
        M_checkMalloc(x);

        complex *result = (complex *)hthread_malloc(clusterId, bufSize, HT_MEM_RW);
        M_checkMalloc(result);
        complex *result1 = (complex *)hthread_malloc(clusterId, bufSize, HT_MEM_RW);
        M_checkMalloc(result1);
        for (size_t i = 0; i < n; i++)
        {
            w[i].real = cos(2 * M_PI / n * i);
            w[i].imag = -sin(2 * M_PI / n * i);
            // printf("生成的第 %lu个w数据: w = (%.6f + %.6fj)\n",
            //        i, w[i].real, w[i].imag);
        }
        for (size_t i = 0; i < n; i++)
        {
            result[i].real = 1;
            result[i].imag = -1;
        }
        for (size_t i = 0; i < n; i++)
        {
            result1[i].real = 1;
            result1[i].imag = -1;
        }
        srand(time(NULL));
        complex *x_gold = (complex *)malloc(bufSize);
        M_checkMalloc(x_gold);
        complex *w_gold = (complex *)malloc(bufSize);
        M_checkMalloc(w_gold);
        complex *result_gold = (complex *)malloc(bufSize);
        M_checkMalloc(result_gold);
        for (size_t i = 0; i < n; i++)
        {
            result_gold[i].real = 0;
            result_gold[i].imag = 0;
        }
        size_t i = 0;
        srand(time(NULL));
        for (i = 0; i < (size_t)n; i++)
        {
            x[i].real = rand() % 11; // 生成0到1000之间的随机数
            x[i].imag = rand() % 11;
            x_gold[i].real = x[i].real;
            x_gold[i].imag = x[i].imag;
            // printf("生成的第 %lu个数据: x = (%.6f + %.6fj)\n",
            //        i, x[i].real, x[i].imag);
        }
        // gold and check

        printf("*==========================*\n");
        printf("=== Starting CPU test : %d * %d fft===\n", mul, n);
        timeGold = getCurrentTimeMicros();
        fft_cpu_serial(n, x_gold, w_gold, result_gold);
        timeGold = getCurrentTimeMicros() - timeGold;
        timeGoldsum = timeGold + timeGoldsum;
        // for (size_t i = 0; i < n; i++)
        // {
        // printf("Result[%zu]: real = %f, imag = %f\n", i, result_gold[i].real, result_gold[i].imag);
        // }
        printf("CPU Time (average of 3 rounds): %lf seconds\n", timeGoldsum / 1e6);
        fprintf(file, "CPU Time (average of 3 rounds): %lf seconds\n", timeGoldsum / 1e6);
        printf("=== CPU test finished ===\n");
        printf("*==========================*\n\n");
        int sum, sets = 0, lines = 1;
        for (lines = 7; lines <= 14; lines++)
        {
            printf("Baseline start.\n");
            double baseline_time, dev_time, speed_up, baseline_time_sum = 0, test_time_sum = 0;
            M_checkMalloc(x);
            uint64_t args[4];
            args[0] = (uint64_t)n;
            args[1] = (uint64_t)x;
            args[2] = (uint64_t)w;
            args[3] = (uint64_t)result;
            uint64_t time = getCurrentTimeMicros();
            int threadId = hthread_group_create(clusterId, nthreads, kernel1, 1, 3, args);
            // printf("下一行是hthread_group_wait\n");
            hthread_group_wait(threadId);
            time = getCurrentTimeMicros() - time;
            baseline_time = (double)time / 1e6;
            hthread_group_destroy(threadId);
            baseline_time_sum += baseline_time;
            printf("Baseline done.\n");
            // int errNum = compare_complex_arrays(result, result_gold, n);
            // printf("errNum: %d\n", errNum);
            // if (errNum < 100)
            // {
            //     printf("no err pass!\n");
            // }

            uint64_t args1[6];
            args1[0] = (uint64_t)n;
            args1[1] = sets;
            args1[2] = lines;
            args1[3] = (uint64_t)x;
            args1[4] = (uint64_t)w;
            args1[5] = (uint64_t)result1;
            dev_time = getCurrentTimeMicros();
            // printf("********n: %d.\n", n);
            int threadId1 = hthread_group_create(clusterId, nthreads, kernel2, 3, 3, args1);
            hthread_group_wait(threadId1);
            dev_time = getCurrentTimeMicros() - dev_time;
            dev_time = (double)dev_time / 1e6;
            hthread_group_destroy(threadId1);
            test_time_sum += dev_time;

            int errNum = compare_complex_arrays(result, result1, n);
            printf("errNum: %d\n", errNum);
            if (errNum < 100)
            {
                printf("no err pass!\n");
            }

            speed_up = baseline_time_sum / test_time_sum; // Convert to percentage
            printf("test time:(sets:%d,lines:%d): %lf seconds,speed up : %.2f\n", sets, lines, test_time_sum, speed_up);
            fprintf(file, "test time:(sets:%d,lines:%d): %lf seconds,speed up : %.2f\n", sets, lines, test_time_sum, speed_up);
        }
        printf("All tests completed\n");
        printf("*==========================*\n");
        fprintf(file, "All tests completed\n");
        fclose(file);
        if (x != NULL)
            hthread_free(x);
        if (w != NULL)
            hthread_free(w);
        if (result != NULL)
            hthread_free(result);
        if (x_gold != NULL)
            free(x_gold);
        if (w_gold != NULL)
            free(w_gold);
        if (result_gold != NULL)
            free(result_gold);
    }
    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);
    return 0;
}
/*
double dsp_test(int clusterId,uint64_t n, double *arr, int sets, int lines){
    uint64_t args[4];
    args[0] = n;
    args[1] = (uint64_t)arr;
    args[2] = sets;
    args[3] = lines;
    uint64_t time = getCurrentTimeMicros();
    //printf("clusterId : %d, n : %d, arr : %ld, "
    //                "kernel : %s, nthreads : %d\n",
    //        clusterId, n, arr, "quicksort_kernel_mem", 1);
    int threadId = hthread_group_create(clusterId, 24, "bubblesort_kernel_mem", 3, 1, args);
    //printf("threadId : %d\n",threadId);
    if (threadId == -1)
        {
            return 2;
        }
    hthread_group_wait(threadId);
    hthread_group_destroy(threadId);
    time = getCurrentTimeMicros() - time;
    double timedev = (double)time / 1e6;
    return timedev;
}
int main(int argc, char** argv)
{
    int retc;
    int clusterId = 1;
    int n = a * b * c;
    char* devProgram = "./code.dev.dat";
    int nthreads = 24;
    char* kernel = "bubblesort_kernel_cache";
    uint64_t timeGold, timeDev;

    if (argc > 1)
    {
        clusterId = atoi(argv[1]);
    }
    if (argc > 2)
    {
        n = atoi(argv[2]) * 2;
    }
    if (argc > 3)
    {
        nthreads = atoi(argv[5]);
    }
    fprintf(stdout, "clusterId : %d, n : %d, devProgram : %s, nthreads : %d\n",
        clusterId, n, devProgram, nthreads);
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

    // malloc
    printf("Allocating memory for matrices.\n");
    size_t bufSize = (size_t)n * sizeof(double);
    double* arr = (double*)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(arr);

    double* arr_gold = (double*)malloc(bufSize);
    M_checkMalloc(arr_gold);
    // init data
    size_t i = 0;
    for (i = 0; i < (size_t)n; i++)
    {
    arr[i] = rand() % 1001; // 这将生成0到1000之间的随机数
    arr_gold[i] = arr[i];
    }
    //printf("arr address: %ld\n", arr);
    // gold and check
    printf("*==========================*\n");
    printf("=== Starting CPU test ===\n");
    timeGold = getCurrentTimeMicros();
    bubblesort(arr_gold , n);
    timeGold = getCurrentTimeMicros() - timeGold;
    char filename[50];   // To store the filename
    sprintf(filename, "%d*%d*%d sort.txt", a,b,c);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error creating file!\n");
        return 1;
    }else{
        printf("successfully creating txt file!\n");
    }
    printf("CPU Time: %lf seconds\n", timeGold/ 1e6);
    fprintf(file,"CPU Time: %lf seconds\n", timeGold/ 1e6);
    printf("=== CPU test finished ===\n");
    printf("*==========================*\n");
    //printf("dev data recording.\n");
    int sum, sets, lines;
    double baseline_time, dev_time, speed_up;
    printf("*==========================*\n");
    printf("Start baseline test.\n");
    baseline_time = dsp_test(clusterId, n, arr,0, 0);
    fprintf(file,"baseline_time: %lf seconds\n", baseline_time);
    printf("baseline_time: %lf seconds\n", baseline_time);
    printf("Baseline test done.\n");
    printf("*==========================*\n");
    printf("Start offical test.\n");
    int sum_begin = 10, sum_end = 18;
        for (sum = sum_begin; sum <= sum_end; sum++)
    {
        for (sets = 1; sets <= sum - 5; sets++)
        {
            lines = sum - sets;
            dev_time = dsp_test(clusterId,n, arr,sets, lines);
            speed_up = (baseline_time - dev_time) / baseline_time * 100; // Convert to percentage
            printf("Processed Set %d, line %d, Speed Up: %.2f%%\n", sets, lines, speed_up);
            fprintf(file, "Processed Set %d, line %d, Speed Up: %.2f%%\n", sets, lines, speed_up);
        }
    }
    printf("All tests completed\n");
    printf("*==========================*\n");
    fprintf(file,"All tests completed\n");
    fclose(file);
    int errNum = 0;
    if (arr != NULL)
        hthread_free(arr);
    if (arr_gold != NULL)
        free(arr_gold);
    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);

    if (errNum > 100)
    {
        fprintf(stderr, "Failed to test merge_sort!\n");
        return 1;
    }
    else
    {
        fprintf(stdout, "merge_sort passed!\n");
        return 0;
    }
}
*/
