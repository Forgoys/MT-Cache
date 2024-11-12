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

uint64_t getCurrentTimeMicros()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}
// uint64_t getCurrentTimeMicros()
//{
//    struct timeval time;
//    gettimeofday(&time, NULL);
//    return (uint64_t)((time.tv_sec * INT64_C(1000000)) + time.tv_usec);
//}

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

// 合并两个有序数组
void merge(double *arr, int left, int mid, int right)
{
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    double *L = (double *)malloc(n1 * sizeof(double));
    double *R = (double *)malloc(n2 * sizeof(double));

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}

// 递归的归并排序函数
void mergeSort(double *arr, int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        merge(arr, left, mid, right);
    }
}

int main(int argc, char **argv)
{
    int retc;
    int clusterId = 1;
    int n = 102400000;
    char *devProgram = "./code.dev.dat";
    int nthreads = 24;
    char *kernel = "quicksort_kernel";
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
        devProgram = argv[3];
    }
    if (argc > 4)
    {
        kernel = argv[4];
    }
    if (argc > 5)
    {
        nthreads = atoi(argv[5]);
    }
    fprintf(stdout, "clusterId : %d, n : %d, devProgram : %s, nthreads : %d\n",
            clusterId, n, devProgram, nthreads);
    // device init
    // printf("device init\n");
    retc = hthread_dev_open(clusterId);
    M_checkRetC(retc, hthread_dev_open);
    retc = hthread_dat_load(clusterId, devProgram);
    M_checkRetC(retc, hthread_dat_load);
    int availThreads = hthread_get_avail_threads(clusterId);
    if (nthreads > availThreads)
    {
        M_logError("number of threads is overflow : avail threads is %d, actual threads is %d",
                   availThreads, nthreads);
        retc = hthread_dat_unload(clusterId);
        retc = hthread_dev_close(clusterId);
        return 2;
    }

    // malloc
    // printf("开始分配，arr address: \n");
    size_t bufSize = (size_t)n * sizeof(double);
    double *arr = (double *)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(arr);
    // printf("arr address: %p\n", arr);

    double *arr_gold = (double *)malloc(bufSize);
    M_checkMalloc(arr_gold);
    // printf("arr_gold address: %p\n", arr_gold);

    // init data
    size_t i = 0;
    for (i = 0; i < (size_t)n; i++)
    {
        arr[i] = rand() % 1001; // 这将生成0到1000之间的随机整数
        arr_gold[i] = arr[i];
        // printf("init Data  %zu: arr = %le   arr_gold = %le\n", i, arr[i], arr_gold[i]);
    }

    // launch dev kernel
    fprintf(stdout, "launch kernel : %s(arr, n)\n", kernel);
    fflush(stdout);
    // printf("fflush over\n");
    uint64_t args[2];
    args[1] = (uint64_t)arr;
    // printf("arr address: %ld\n", args[1]);
    args[0] = (uint64_t)n;
    // printf("n: %ld\n", args[0]);
    // printf("开始运行time\n");
    timeDev = getCurrentTimeMicros();
    // printf("timeDev: %lu\n", timeDev);
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 1, 2, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    printf("threadId: %d\n", threadId);
    hthread_group_wait(threadId);
    timeDev = getCurrentTimeMicros() - timeDev;
    // printf("timeDev = %d\n", timeDev);

    // gold and check
    timeGold = getCurrentTimeMicros();
    mergeSort(arr_gold, 0, n - 1);
    timeGold = getCurrentTimeMicros() - timeGold;
    // printf("timeGold = %d\n", timeGold);

    int errNum = 0;

    for (i = 0; i < (size_t)n; i++)
    {
        if (fabs(arr[i] - arr_gold[i]) > 1e-6)
        {
            // fprintf(stderr, "Data error at %zu: %le != %le\n", i, arr[i], arr_gold[i]);
            errNum++;
        }
    }
    errNum = 0;
    // printf("errNum = %d\n", errNum);

    hthread_group_destroy(threadId);
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
        fprintf(stdout, "WallTime of merge_sort_kernel : %fs\n", timeDev / 1e6);
        fprintf(stdout, "WallTime of merge_sort_cpu    : %fs\n", timeGold / 1e6);
        return 0;
    }
}
