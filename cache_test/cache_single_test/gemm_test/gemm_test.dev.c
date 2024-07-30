#include <compiler/m3000.h>
#include "hthread_device.h"
#include "cache_single.h"

static inline void gemm_single_mem(long *A, long *B, long *C, int length, int stride)
{
    for (int k = 0; k < length; k++)
    {
        for (int i = 0; i < length; i++)
        {
            for (int j = 0; j < length; j++)
            {
                C[i * stride + j] += A[i * stride + k] * B[k * stride + j];
            }
        }
    }
}

static inline void gemm_single_cache(long *A, long *B, long *C, int length, int stride)
{
    CACHEs_INIT(A, long, 0, 0, 14);
    CACHEs_INIT(B, long, 0, 0, 14);
    CACHEs_INIT(C, long, 0, 0, 14);
    long tmp_A, tmp_B, tmp_C;
    for (int k = 0; k < length; k++)
    {
        for (int i = 0; i < length; i++)
        {
            for (int j = 0; j < length; j++)
            {
                CACHEs_SEC_W_RD(A, A + i * stride + k, tmp_A, long);
                CACHEs_SEC_W_RD(B, B + k * stride + j, tmp_B, long);
                CACHEs_SEC_W_RD(C, C + i * stride + j, tmp_C, long);
                tmp_C += tmp_A * tmp_B;
                CACHEs_SEC_W_WR(C, C + i * stride + j, tmp_C, long);
            }
        }
    }
    CACHEs_FLUSH(A);
    CACHEs_FLUSH(B);
    CACHEs_FLUSH(C);
}

static inline void gemm_single_dma(long *A, long *B, long *C, int length, int stride)
{
    long *aa = (long *)scalar_malloc(length * length * sizeof(long));
    long *bb = (long *)scalar_malloc(length * length * sizeof(long));
    long *cc = (long *)scalar_malloc(length * length * sizeof(long));

    if (aa == NULL || bb == NULL || cc == NULL)
    {
        // 错误处理：内存分配失败
        hthread_printf("malloc failed\n");
        return;
    }

    int dma_no = dma_p2p(A, length, length * sizeof(long), (stride - length) * sizeof(long), aa, 1, length * length * sizeof(long), 0, false, 0);
    dma_wait(dma_no);
    dma_no = dma_p2p(B, length, length * sizeof(long), (stride - length) * sizeof(long), bb, 1, length * length * sizeof(long), 0, false, 0);
    dma_wait(dma_no);
    dma_no = dma_p2p(C, length, length * sizeof(long), (stride - length) * sizeof(long), cc, 1, length * length * sizeof(long), 0, false, 0xFFFFFF);
    dma_wait(dma_no);

    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length; j++)
        {
            for (int k = 0; k < length; k++)
            {
                cc[i * length + j] += aa[i * length + k] * bb[k * length + j];
            }
        }
    }

    dma_no = dma_p2p(cc, 1, length * length * sizeof(long), 0, C, length, length * sizeof(long), (stride - length) * sizeof(long), false, 0xFFFFFF);
    dma_wait(dma_no);

    scalar_free(aa);
    scalar_free(bb);
    scalar_free(cc);
}

static inline int min(int a, int b)
{
    return a < b ? a : b;
}

__global__ void gemm_mem_test_kernel(int length, int tile_size, long *A, long *B, long *C)
{
    int tid = get_thread_id();
    int thread_num = get_group_size();

    int block_row_num = (length + tile_size - 1) / tile_size;
    int block_num = block_row_num * block_row_num;

    int block_num_per_thread = (block_num + thread_num - 1) / thread_num; // 每个线程处理的块数

    int block_id_begin = tid * block_num_per_thread; // 当前线程处理的块的起始编号

    for (int block_id = block_id_begin; block_id < min(block_id_begin + block_num_per_thread, block_num); block_id++)
    {
        int block_row = block_id / block_row_num;
        int block_col = block_id % block_row_num;

        for (int block_k = 0; block_k < block_row_num; block_k++)
        {
            gemm_single_mem(A + (block_row * tile_size * length + block_k * tile_size),
                            B + (block_k * tile_size * length + block_col * tile_size),
                            C + (block_row * tile_size * length + block_col * tile_size),
                            tile_size, length);
        }
    }
}

__global__ void gemm_cache_test_kernel(int length, int tile_size, long *A, long *B, long *C)
{
    int tid = get_thread_id();
    int thread_num = get_group_size();

    int block_row_num = (length + tile_size - 1) / tile_size;
    int block_num = block_row_num * block_row_num;

    int block_num_per_thread = (block_num + thread_num - 1) / thread_num; // 每个线程处理的块数

    int block_id_begin = tid * block_num_per_thread; // 当前线程处理的块的起始编号

    for (int block_id = block_id_begin; block_id < min(block_id_begin + block_num_per_thread, block_num); block_id++)
    {
        int block_row = block_id / block_row_num;
        int block_col = block_id % block_row_num;

        for (int block_k = 0; block_k < block_row_num; block_k++)
        {
            gemm_single_cache(A + (block_row * tile_size * length + block_k * tile_size),
                              B + (block_k * tile_size * length + block_col * tile_size),
                              C + (block_row * tile_size * length + block_col * tile_size),
                              tile_size, length);
        }
    }
}