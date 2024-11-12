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

uint64_t getCurrentTimeMicros() {  
    struct timespec ts;  
    clock_gettime(CLOCK_REALTIME, &ts);  
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;  
}  
//uint64_t getCurrentTimeMicros()
//{
 //   struct timeval time;
 //   gettimeofday(&time, NULL);
 //   return (uint64_t)((time.tv_sec * INT64_C(1000000)) + time.tv_usec);
//}
#define M_logError(_FMT, ...)                            \
    do                                                   \
    {                                                    \
        fprintf(stderr, "Error : " _FMT "in %d of %s\n", \
                __VA_ARGS__, __LINE__, __FILE__);        \
    } while (0);
double dsp_test(int clusterId, char *kernel, int nthreads, uint64_t n, double *arr, int cets, int lines){
    unsigned long args[7];
    args[0] = n;
    args[1] = (unsigned long)arr;
    args[2] = cets;
    args[3] = lines;
    uint64_t time_sum = 0;
    uint64_t time = 0;
    int threadId = hthread_group_create(clusterId, nthreads, kernel, 4, 3, args);
    if (threadId == -1)
        {
            M_logError("Failed to create threads with %s", kernel);
            return 2;
        }
    hthread_group_wait(threadId);
    time = getCurrentTimeMicros() - time;
    return (double)time / 1e6;
}
int fileIsExist(const char* filePath)
{
    return access(filePath, F_OK);
}


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
void merge(double* arr, int left, int mid, int right)
{
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    double* L = (double*)malloc(n1 * sizeof(double));
    double* R = (double*)malloc(n2 * sizeof(double));

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
void mergeSort(double* arr, int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;

        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);

        merge(arr, left, mid, right);
    }
}
int main(int argc, char** argv)
{
    int retc;
    int clusterId = 1;
    int n = 102400000;
    char* devProgram = "./code.dev.dat";
    int nthreads = 24;
    char* kernel = "quicksort_kernel_cache";
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
    //printf("device init\n");
    retc = hthread_dev_open(clusterId);
    M_checkRetC(retc, hthread_dev_open);
    retc = hthread_dat_load(clusterId, devProgram);
    M_checkRetC(retc, hthread_dat_load);
    int availThreads = hthread_get_avail_threads(clusterId);

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
    arr[i] = rand() % 10001; // 这将生成0到1000之间的随机数
    arr_gold[i] = arr[i];
    }
    // gold and check
    printf("=== Starting CPU test ===\n");
    timeGold = getCurrentTimeMicros();
    mergeSort(arr_gold, 0, n - 1);
    timeGold = getCurrentTimeMicros() - timeGold;
    printf("CPU Time: %lf seconds\n", timeGold);
    printf("=== CPU test finished ===\n");
    printf("dev data recording.\n");
    int sum, cets, lines;
    double baseline_time, dev_time, speed_up;
    printf("Start baseline test.\n");
    baseline_time = dsp_test(clusterId, "quicksort_kernel_mem", nthreads, n, arr,0, 0);
    printf("Baseline test done.\n");
    int sum_begin = 10, sum_end = 18;
        for (sum = sum_begin; sum <= sum_end; sum++)
    {
        for (cets = 1; cets <= sum - 5; cets++)
        {
            lines = sum - cets;
            dev_time = dsp_test(clusterId, "quicksort_kernel_mem", nthreads, n, arr,cets, lines);
            speed_up = baseline_time / dev_time;
            printf("Processed Set %d, line %d, Speed Up: %.4f\n", cets, lines, speed_up);
        }
    }
    printf("All tests completed\n");
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

