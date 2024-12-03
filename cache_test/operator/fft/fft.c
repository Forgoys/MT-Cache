#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
typedef struct
{
    double real;
    double imag;
} complex;

double PI = 4.0 * atan(1); // 定义π

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

// 封装后的FFT函数
void perform_fft(complex *x, size_t size, complex *W, complex *result)
{
    if (size == 1)
    {
        result[0] = x[0]; // 基础情况：只剩一个元素
        return;
    }

    size_t half_size = size / 2;

    // 分配内存存储偶数和奇数项的FFT
    complex even[half_size];
    complex odd[half_size];
    complex Wn, t, u;

    // 拆分成偶数和奇数项
    for (size_t i = 0; i < half_size; i++)
    {
        even[i] = x[i * 2];
        odd[i] = x[i * 2 + 1];
    }

    // 递归调用FFT
    perform_fft(even, half_size, W, result);            // 对偶数项递归FFT
    perform_fft(odd, half_size, W, result + half_size); // 对奇数项递归FFT

    // 合并
    for (size_t k = 0; k < half_size; k++)
    {
        double theta = -2 * PI * k / size; // Twiddle factor
        complex_exp(theta, &Wn);

        mul(Wn, result[k + half_size], &t);  // t = Wn * odd[k]
        u = result[k];                       // 保存 result[k] 作为 u

        add(u, t, &result[k]);               // result[k] = u + t
        sub(u, t, &result[k + half_size]);   // result[k + half_size] = u - t
    }
}
uint64_t getCurrentTimeMicros()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}
int main()
{
    size_t size = 64;
    size_t bufSize = (size_t)size * sizeof(complex);
    complex *x_gold = (complex *)malloc(bufSize);

        srand(time(NULL));
        for (int i = 0; i < (size_t)size; i++)
        {
            x_gold[i].real = rand() % 10001;
            x_gold[i].imag = rand() % 10001;
             printf("生成的第 %lu个数据: x = (%.6f + %.6fj)\n",
                     i, x_gold[i].real, x_gold[i].imag);
        }

    complex W[size];
    complex result[size];

    perform_fft(x_gold, size, W, result);

    for (size_t i = 0; i < size; i++)
    {
        printf("Result[%zu]: real = %f, imag = %f\n", i, result[i].real, result[i].imag);
    }

    return 0;
}

