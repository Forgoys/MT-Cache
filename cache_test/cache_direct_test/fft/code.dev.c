#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <compiler/m3000.h>
#include "cache_diret.h"
#include "hthread_device.h"
#include "mem.h"
int global_cets = 7;
int global_lines = 7;
typedef struct {       // Complex number structure
    double real;
    double imag;
} complex;

double PI = 4.0 * atan(1);  // Define π

// Complex number addition
static inline void add(complex a, complex b, complex *c) {
    c->real = a.real + b.real;
    c->imag = a.imag + b.imag;
}

// Complex number subtraction
static inline void sub(complex a, complex b, complex *c) {
    c->real = a.real - b.real;
    c->imag = a.imag - b.imag;
}

// Complex number multiplication
static inline void mul(complex a, complex b, complex *c) {
    c->real = a.real * b.real - a.imag * b.imag;
    c->imag = a.real * b.imag + a.imag * b.real;
}

// Serial FFT function without cache
static inline void fft_serial(uint64_t i, uint64_t n, complex *arr, complex *W, complex *result)
{
    uint64_t j = i;
    for (uint64_t k = i; k < n; k++)
    {
        complex temp;
        mul(W[k], arr[k], &temp);  // Multiply by twiddle factor
        add(arr[i], temp, &result[j]);  // Store result
        j++;
    }
}

// Serial FFT function with cache support
static inline void fft_serial_cache(uint64_t i, uint64_t n, complex *arr, complex *W, complex *result)
{
    CACHEe_ENV();
    CACHEe_INIT(arr, complex, global_cets, global_lines);  // Initialize cache for arr
    CACHEe_INIT(W, complex, global_cets, global_lines);    // Initialize cache for W
    CACHEe_INIT(result, complex, global_cets, global_lines);  // Initialize cache for result

    uint64_t j = 0;
    complex temp,tmp_arr, tmp_arr_i, tmp_w, tmp_result;
    CACHEe_SEC_R_RD(arr, arr + i, tmp_arr_i, complex);
    for (uint64_t k = i; k < n; k++)
    {
        // complex temp;
        // CACHEe_SEC_W_RD(arr, arr + k, temp, complex);  // Read from cache
        // CACHEe_SEC_W_RD(W, W + k, temp, complex);  // Read from cache
        // mul(W[k], arr[k], &temp);  // Multiply by twiddle factor
        // CACHEe_SEC_W_RD(arr, arr + i, temp, complex);  // Add to result
        // CACHEe_SEC_W_RD(result, result + i, temp, complex);  // Add to result
        // add(arr[i], temp, &result[j]);  
        // j++;
        CACHEe_SEC_R_RD(arr, arr + k, tmp_arr, complex);  // Read from cache
        CACHEe_SEC_R_RD(W, W + k, tmp_w, complex);  // Read from cache
        mul(tmp_w, tmp_arr, &temp);  // Multiply by twiddle factor
        add(tmp_arr_i, temp, &tmp_result);  
        CACHEe_SEC_W_RD(result, result + j, tmp_result, complex);  // Add to result
        j++;
    }

    CACHEe_INVALID(arr);  // Flush cache after computation
    CACHEe_INVALID(W);
    CACHEe_FLUSH(result, complex);
}

// Parallel FFT kernel without cache
__global__ void fft_kernel(uint64_t n, complex *arr, complex *W, complex *result) {  
    int threadId = get_thread_id();  
    hthread_printf("threadId : %d\n",threadId);
    int threadsNum = get_group_size();
    
    uint64_t n_p = n / threadsNum;  // Number of elements per thread
    uint64_t start = threadId * n_p;
    uint64_t end = (threadId == threadsNum - 1) ? n : start + n_p;  // Handle remainder

    for (uint64_t i = start; i < end; i++)
    {
        fft_serial(i, end, arr, W, result);  // Call serial FFT for each element
    }
}

// Parallel FFT kernel with cache

__global__ void fft_kernel_cache(uint64_t n, int cets, int lines, complex *arr, complex *W, complex *result)
{
    global_cets = cets;
    global_lines = lines;
    int threadId = get_thread_id();
    int threadsNum = get_group_size();
    
    uint64_t n_p = n / threadsNum;  // Number of elements per thread
    uint64_t start = threadId * n_p;
    uint64_t end = (threadId == threadsNum - 1) ? n : start + n_p;  // Handle remainder

    for (uint64_t i = start; i < end; i++)
    {
        fft_serial_cache(i, end, arr, W, result);  // Call cached FFT for each element
    }
}