#include <stdint.h>

#include <compiler/m3000.h>
#include "hthread_device.h"

static inline void printVec64l(lvector unsigned long a)
{
    unsigned long rst[16];
    mov_to_svr_v16di(a);
    rst[0] = mov_from_svr0();
    rst[1] = mov_from_svr1();
    rst[2] = mov_from_svr2();
    rst[3] = mov_from_svr3();
    rst[4] = mov_from_svr4();
    rst[5] = mov_from_svr5();
    rst[6] = mov_from_svr6();
    rst[7] = mov_from_svr7();
    rst[8] = mov_from_svr8();
    rst[9] = mov_from_svr9();
    rst[10] = mov_from_svr10();
    rst[11] = mov_from_svr11();
    rst[12] = mov_from_svr12();
    rst[13] = mov_from_svr13();
    rst[14] = mov_from_svr14();
    rst[15] = mov_from_svr15();
    for (int i = 0; i < 16; i++)
    {
        hthread_printf("%lu ", rst[i]);
    }
    hthread_printf("\n");
}

static inline void printVecd(lvector double a)
{
    double rst[16];
    mov_to_svr_v16df(a);
    rst[0] = mov_from_svr0_df();
    rst[1] = mov_from_svr1_df();
    rst[2] = mov_from_svr2_df();
    rst[3] = mov_from_svr3_df();
    rst[4] = mov_from_svr4_df();
    rst[5] = mov_from_svr5_df();
    rst[6] = mov_from_svr6_df();
    rst[7] = mov_from_svr7_df();
    rst[8] = mov_from_svr8_df();
    rst[9] = mov_from_svr9_df();
    rst[10] = mov_from_svr10_df();
    rst[11] = mov_from_svr11_df();
    rst[12] = mov_from_svr12_df();
    rst[13] = mov_from_svr13_df();
    rst[14] = mov_from_svr14_df();
    rst[15] = mov_from_svr15_df();
    for (int i = 0; i < 16; i++)
    {
        hthread_printf("%.1lf ", rst[i]);
    }
    hthread_printf("\n");
}