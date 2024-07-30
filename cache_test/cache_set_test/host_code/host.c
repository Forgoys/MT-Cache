#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "hthread_host.h"

int main(int argc, char **argv)
{

    if (argc < 3)
    {
        printf("Useage: ~ length coreNum\n");
        return -1;
    }

    // Make sure length can divide 16(vector length)
    int length = atoi(argv[1]);
    int coreNum = atoi(argv[2]);

    int cluster_id = 0;
    size_t bufSize = (size_t)length * sizeof(long);
    long *x = (long *)hthread_malloc(cluster_id, bufSize, HT_MEM_RO);
    long *y = (long *)hthread_malloc(cluster_id, bufSize, HT_MEM_RO);

    hthread_dev_open(cluster_id);
    hthread_dat_load(cluster_id, "kernel.dat");

    unsigned long args[5];
    args[0] = length;
    args[1] = coreNum;
    // args[2] = (unsigned long)A;
    // args[3] = (unsigned long)B;
    // args[4] = (unsigned long)C;

    int thread_id = hthread_group_create(cluster_id, coreNum, "vec_add_kernel", 2, 3, args);

    hthread_group_wait(thread_id);

    hthread_group_destroy(thread_id);
    hthread_dev_close(cluster_id);

    return 0;
}
