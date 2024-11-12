#include <compiler/m3000.h>
#include "hthread_device.h"

__global__ void test()
{
    char* buffer = scalar_malloc(10);
    char tmp;
    tmp = buffer[0];
    tmp++;
    scalar_free(buffer);
}