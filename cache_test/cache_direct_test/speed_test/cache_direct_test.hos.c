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
    char *devProgram = "./daxpy.dev.dat";
    int nthreads = 1;
    char *kernel = "set_mem_test_kernel";
    //
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
    size_t bufSize = (size_t)n * sizeof(char);
    char *x = (char *)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(x);
    char *y = (char *)hthread_malloc(clusterId, bufSize, HT_MEM_RO);
    M_checkMalloc(y);


    // init data
    size_t i = 0;
    for (i = 0; i < (size_t)n; i++)
    {
        x[i] = 1;
    }

    uint64_t args[4];
    args[0] = (uint64_t)n;
    args[1] = (uint64_t)doubleToRawBits(alph);
    args[2] = (uint64_t)x;
    args[3] = (uint64_t)y;
    int threadId = hthread_group_create(clusterId, nthreads,
                                        kernel, 2, 2, args);
    if (threadId == -1)
    {
        M_logError("Failed to create threads with %s", kernel);
        return 2;
    }
    hthread_group_wait(threadId);

    // fini
    hthread_group_destroy(threadId);
    if (x != NULL)
        hthread_free(x);
    if (y != NULL)
        hthread_free(y);
    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);
}
