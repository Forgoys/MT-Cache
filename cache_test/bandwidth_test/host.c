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

#include "hthread_host.h"
#define BUFFER_SIZE 61440

uint64_t doubleToRawBits(double d)
{
    union
    {
        uint64_t i;
        double f;
    } word;
    word.f = d;
    return word.i;
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

uint64_t getCurrentTimeMicros()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (uint64_t)((time.tv_sec * INT64_C(1000000)) + time.tv_usec);
}

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

int main(int argc, char **argv)
{
    int retc;
    int clusterId = 1;
    int n = 1024 * 1024 * 100;
    double alph = 3.14;
    char *devProgram = "./kernel.dat";
    int nthreads = 1;
    char *kernel = "bandwidth_test_kernel";
    
    if (argc > 1)
    {
        clusterId = atoi(argv[1]);
    }
    if (argc > 2)
    {
        n = atoi(argv[2]);
    }
    if (argc > 3)
    {
        nthreads = atoi(argv[3]);
    }
    fprintf(stdout, "clusterId : %d, n : %d, "
                    "devProgram : %s, nthreads : %d\n",
            clusterId, n, devProgram, nthreads);

    // check args
    if (clusterId < 0 || clusterId > 3)
    {
        M_logError("invalid clusterId : %d", clusterId);
        return 2;
    }
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
    n = BUFFER_SIZE * nthreads / sizeof(double);
    size_t bufSize = (size_t)(n * sizeof(double));
    double *x = (double *)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(x);

    // init data
    size_t i = 0;
    for (i = 0; i < (size_t)n; i++)
    {
        x[i] = 0.0 + 1.0 * rand() / RAND_MAX * (6.28 - 0.0);
    }

    // launch dev kernel
    fprintf(stdout, "launch kernel : %s(%d, %lf, x, y)\n",
            kernel, n, alph);
    fflush(stdout);
    int threadId = hthread_group_create(clusterId, nthreads);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }

    uint64_t args[2];
    args[0] = (uint64_t)bufSize;
    args[1] = (uint64_t)x;

    uint64_t startTime, endTime;
    double elapsedTime, bandwidth;
    startTime = getCurrentTimeMicros();
    kernel = "dma_p2p_mem2sm";
    hthread_group_exec(threadId, kernel, 1, 1, args);
    hthread_group_wait(threadId);
    endTime = getCurrentTimeMicros();
    elapsedTime = (double)(endTime - startTime) / 1e6;
    bandwidth = (double)(n * sizeof(double)) / 1024 / 1024 / 1024 / elapsedTime;
    fprintf(stdout, "bandwidth of dma_p2p_mem2sm: %lf GB/s\n", bandwidth);

    startTime = getCurrentTimeMicros();
    kernel = "scalar_load_mem2sm";
    hthread_group_exec(threadId, kernel, 1, 1, args);
    hthread_group_wait(threadId);
    endTime = getCurrentTimeMicros();
    elapsedTime = (double)(endTime - startTime) / 1e6;
    bandwidth = (double)(n * sizeof(double)) / 1024 / 1024 / 1024 / elapsedTime;
    fprintf(stdout, "bandwidth of scalar_load_mem2sm: %lf GB/s\n", bandwidth);

    // fini
    hthread_group_destroy(threadId);
    if (x != NULL)
        hthread_free(x);

    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);
}
