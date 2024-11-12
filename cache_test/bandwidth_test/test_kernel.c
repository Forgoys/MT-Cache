#include <stdio.h>

void test_DMA_p2p_mem2sm()
{
    unsigned long n = 1000; // Set the desired value for n
    double *x = malloc(n * sizeof(double));
    double *x_buffer = malloc(BUFFER_SIZE * sizeof(double));

    // Initialize x and x_buffer with test data

    DMA_p2p_mem2sm<<<1, 1>>>(n, x, x_buffer);
    cudaDeviceSynchronize();

    // Perform assertions or print test results

    free(x);
    free(x_buffer);
}

void test_sm2reg()
{
    unsigned long n = 1000; // Set the desired value for n
    double *x = malloc(n * sizeof(double));

    // Initialize x with test data

    sm2reg<<<1, 1>>>(n, x);
    cudaDeviceSynchronize();

    // Perform assertions or print test results

    free(x);
}

void test_mem2reg()
{
    unsigned long n = 1000; // Set the desired value for n
    double *x = malloc(n * sizeof(double));

    // Initialize x with test data

    mem2reg<<<1, 1>>>(n, x);
    cudaDeviceSynchronize();

    // Perform assertions or print test results

    free(x);
}

int main()
{
    test_DMA_p2p_mem2sm();
    test_sm2reg();
    test_mem2reg();

    return 0;
}