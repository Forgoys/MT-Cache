# 1 "kernel.c"
# 1 "/thfs3/home/xjtu_cx/lhj/mt3000-cache/gemm/device_code//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "kernel.c"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/compiler/m3000.h" 1
# 2 "kernel.c" 2
# 1 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 1
# 9 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stdbool.h" 1 3 4
# 10 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 2
# 1 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stdarg.h" 1 3 4
# 40 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stdarg.h" 3 4

# 40 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 99 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 11 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 2
# 1 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 1 3 4
# 149 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 3 4
typedef long int ptrdiff_t;
# 216 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 328 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 3 4
typedef int wchar_t;
# 426 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 3 4
typedef struct
{
    long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
    long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
# 437 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 3 4
} max_align_t;
# 12 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 2
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h" 1
# 23 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/features.h" 1
# 187 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/features.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/uClibc_config.h" 1
# 188 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/features.h" 2
# 416 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/features.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/cdefs.h" 1
# 417 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/features.h" 2
# 24 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h" 1
# 28 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/wordsize.h" 1
# 29 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env_202312/dsp_compiler/lib/gcc/tic6x-elf/8.3.0/include/stddef.h" 1 3 4
# 32 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h" 2

# 34 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h"
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;

typedef long int __quad_t;
typedef unsigned long int __u_quad_t;
# 134 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/typesizes.h" 1
# 135 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/types.h" 2

typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct
{
    int __val[2];
} __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;

typedef int __daddr_t;
typedef long int __swblk_t;
typedef int __key_t;

typedef int __clockid_t;

typedef void *__timer_t;

typedef long int __blksize_t;

typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;

typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;

typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;

typedef long int __ssize_t;

typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;

typedef long int __intptr_t;

typedef unsigned int __socklen_t;
# 26 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/time.h" 1
# 75 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/time.h"

typedef __time_t time_t;

# 28 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/time.h" 1
# 73 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/time.h"
struct timeval
{
    __time_t tv_sec;
    __suseconds_t tv_usec;
};
# 30 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h" 1
# 31 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h"
# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/select.h" 1
# 32 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/sigset.h" 1
# 23 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/sigset.h"
typedef int __sig_atomic_t;
# 40 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/sigset.h"
typedef struct
{
    unsigned long __val[(64 / (8 * sizeof(unsigned long)))];
} __sigset_t;
# 35 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h" 2

typedef __sigset_t sigset_t;

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/time.h" 1
# 121 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/time.h"
struct timespec
{
    __time_t tv_sec;
    long int tv_nsec;
};
# 45 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h" 2

# 1 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/bits/time.h" 1
# 47 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h" 2

typedef __suseconds_t suseconds_t;

typedef long int __fd_mask;
# 67 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h"
typedef struct
{

    __fd_mask __fds_bits[1024 / (8 * sizeof(__fd_mask))];

} fd_set;

typedef __fd_mask fd_mask;
# 99 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h"

# 109 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h"
extern int select(int __nfds, fd_set *__restrict __readfds,
                  fd_set *__restrict __writefds,
                  fd_set *__restrict __exceptfds,
                  struct timeval *__restrict __timeout);
# 121 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/select.h"
extern int pselect(int __nfds, fd_set *__restrict __readfds,
                   fd_set *__restrict __writefds,
                   fd_set *__restrict __exceptfds,
                   const struct timespec *__restrict __timeout,
                   const __sigset_t *__restrict __sigmask);

# 32 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h" 2

# 57 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h"
struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

typedef struct timezone *__restrict __timezone_ptr_t;
# 73 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h"
extern int gettimeofday(struct timeval *__restrict __tv,
                        __timezone_ptr_t __tz) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

extern int settimeofday(__const struct timeval *__tv,
                        __const struct timezone *__tz)
    __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

extern int adjtime(__const struct timeval *__delta,
                   struct timeval *__olddelta) __attribute__((__nothrow__));

enum __itimer_which
{

    ITIMER_REAL = 0,

    ITIMER_VIRTUAL = 1,

    ITIMER_PROF = 2

};

struct itimerval
{

    struct timeval it_interval;

    struct timeval it_value;
};

typedef int __itimer_which_t;

extern int getitimer(__itimer_which_t __which,
                     struct itimerval *__value) __attribute__((__nothrow__));

extern int setitimer(__itimer_which_t __which,
                     __const struct itimerval *__restrict __new,
                     struct itimerval *__restrict __old) __attribute__((__nothrow__));

extern int utimes(__const char *__file, __const struct timeval __tvp[2])
    __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

extern int lutimes(__const char *__file, __const struct timeval __tvp[2])
    __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));
# 193 "/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler/include/sys/time.h"

# 13 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 2
# 27 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
__asm__(".section ._gsm,\"aw\",%nobits");
# 44 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
int get_group_size();
unsigned int get_group_cores();
int get_thread_id();
int get_core_id();

void group_barrier(unsigned id);

void core_barrier(unsigned int id, unsigned int num);
void core_barrier_wait(unsigned int id, unsigned int num, unsigned long wait_time);

int rwlock_try_rdlock(unsigned int lock_id);
int rwlock_try_wrlock(unsigned int lock_id);
void rwlock_rdlock(unsigned int lock_id);
void rwlock_wrlock(unsigned int lock_id);
void rwlock_unlock(unsigned int lock_id);

void *vector_malloc(unsigned int bytes);
int vector_free(void *ptr);
int vector_load(void *mem, void *buf, unsigned int bytes);
unsigned int vector_load_async(void *mem, void *buf, unsigned int bytes);
int vector_store(void *buf, void *mem, unsigned int bytes);
unsigned int vector_store_async(void *buf, void *mem, unsigned int bytes);
int get_am_free_space();

void *scalar_malloc(unsigned int bytes);
int scalar_free(void *ptr);
int scalar_load(void *mem, void *buf, unsigned int bytes);
unsigned int scalar_load_async(void *mem, void *buf, unsigned int bytes);
int scalar_store(void *buf, void *mem, unsigned int bytes);
unsigned int scalar_store_async(void *buf, void *mem, unsigned int bytes);
int get_sm_free_space();

unsigned int dma_p2p(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                     void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,

# 90 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 3 4
                     _Bool
# 90 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
                         row_syn,
                     unsigned int synmask);

unsigned int dma_broadcast(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                           void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,
                           unsigned core_id, unsigned int barrire_id);

unsigned int dma_segment(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                         void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,
                         unsigned int c_start, unsigned int c_num, unsigned int c_step, unsigned int barrire_id);

unsigned int dma_sg(void *src_base, void *src_index, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                    void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step);

void dma_wait(unsigned int ch);

int dma_query(unsigned int ch);
void dma_clear();

unsigned int raw_dma_p2p(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                         void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,

# 111 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 3 4
                         _Bool
# 111 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
                             row_syn,
                         unsigned int synmask, int prir, int chno);
unsigned int raw_dma_broadcast(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                               void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,
                               unsigned core_id, unsigned int barrire_id, int total_cores, int prir, int chno);
unsigned int raw_dma_segment(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                             void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,
                             unsigned int c_start, unsigned int c_num, unsigned int c_step, unsigned int barrire_id, int prir, int chno);
unsigned int raw_dma_sg(void *src_base, void *src_index, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                        void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step, int prir, int chno);
void raw_dma_wait(int tcc);

unsigned int dma_sg_opt(void *src_base, void *src_index, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                        void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step, int ch);

unsigned int dma_p2p_opt(void *src, unsigned long src_row_num, unsigned int src_row_size, int src_row_step,
                         void *dst, unsigned long dst_row_num, unsigned int dst_row_size, int dst_row_step,

# 128 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h" 3 4
                         _Bool
# 128 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
                             row_syn,
                         unsigned int p2pmask, int ch);

void dma_wait_p2p(unsigned int ch_no);
void dma_wait_sg(unsigned int ch_no);
void set_prir(unsigned long val);

void dsp_abort(unsigned int err_no);
void dsp_halt();
void dsp_sleep(unsigned long usec);

void hthread_printf(const char *fmt, ...);
int hthread_sprintf(char *buffer, const char *format, ...);
int hthread_snprintf(char *buffer, size_t count, const char *format, ...);
int hthread_vsnprintf(char *buffer, size_t count, const char *format, va_list va);

void hthread_gettimeofday(struct timeval *tv);

void *hbm_malloc(unsigned long bytes);
void hbm_free(void *ptr);

void trigger_cpu(unsigned long val);
# 182 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
void prof_start(int event_id);
unsigned long prof_end(int event_id);
unsigned long prof_read(int event_id);

unsigned long get_clk();

void set_sata_mode(int val);
# 206 "/thfs3/software/programming_env/mt3000_programming_env/hthreads/include/hthread_device.h"
void set_ecr(int EID);
void clear_ecr(int EID);

unsigned long intr_handler_register(void (*func)(int no));

void cpu_interrupt(unsigned long val);
# 3 "kernel.c" 2
# 1 "/thfs3/home/xjtu_cx/lhj/mt3000-cache/define_version/cache_bulk.h" 1
unsigned long interface_sizes;
# 4 "kernel.c" 2

static inline void gemm_single(long *A, long *B, long *C, int length, int stride)
{
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < length; j++)
        {
            for (int k = 0; k < length; k++)
            {
                C[i * stride + j] += A[i * stride + k] * B[k * stride + j];
            }
        }
    }
}

static inline void gemm_single_dma(long *A, long *B, long *C, int length, int stride)
{
    long *aa = (long *)scalar_malloc(length * length * sizeof(long));
    long *bb = (long *)scalar_malloc(length * length * sizeof(long));
    long *cc = (long *)scalar_malloc(length * length * sizeof(long));

    if (aa ==
# 25 "kernel.c" 3 4
            ((void *)0)
# 25 "kernel.c"
        || bb ==
# 25 "kernel.c" 3 4
               ((void *)0)
# 25 "kernel.c"
        || cc ==
# 25 "kernel.c" 3 4
               ((void *)0)
# 25 "kernel.c"
    )
    {

        hthread_printf("malloc failed\n");
        return;
    }

    int dma_no = dma_p2p(A, length, length * sizeof(long), (stride - length) * sizeof(long), aa, 1, length * length * sizeof(long), 0,
# 32 "kernel.c" 3 4
                         0
# 32 "kernel.c"
                         ,
                         0);
    dma_wait(dma_no);
    dma_no = dma_p2p(B, length, length * sizeof(long), (stride - length) * sizeof(long), bb, 1, length * length * sizeof(long), 0,
# 34 "kernel.c" 3 4
                     0
# 34 "kernel.c"
                     ,
                     0);
    dma_wait(dma_no);
    dma_no = dma_p2p(C, length, length * sizeof(long), (stride - length) * sizeof(long), cc, 1, length * length * sizeof(long), 0,
# 36 "kernel.c" 3 4
                     0
# 36 "kernel.c"
                     ,
                     0xFFFFFF);
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

    dma_no = dma_p2p(cc, 1, length * length * sizeof(long), 0, C, length, length * sizeof(long), (stride - length) * sizeof(long),
# 50 "kernel.c" 3 4
                     0
# 50 "kernel.c"
                     ,
                     0xFFFFFF);
    dma_wait(dma_no);

    scalar_free(aa);
    scalar_free(bb);
    scalar_free(cc);
}

static inline int min(int a, int b)
{
    return a < b ? a : b;
}

__attribute__((section(".global"))) void gemm(int length, int tile_size, int barrier_no, long *A, long *B, long *C)
{
    int tid = get_thread_id();
    int thread_num = get_group_size();

    int block_row_num = (length + tile_size - 1) / tile_size;
    int block_num = block_row_num * block_row_num;

    int block_num_per_thread = (block_num + thread_num - 1) / thread_num;

    int block_id_begin = tid * block_num_per_thread;
# 88 "kernel.c"
        long *test_p = A + 1;
        hthread_printf("%p\n", test_p);
        int n = 10;
        unsigned int channel_number_test_p = 0;
        char *interface_buffer_test_p = (char *)scalar_malloc(n * sizeof(long) + 64);
        unsigned long interface_Ea_align_test_p = (unsigned long)test_p & 0xFFFFFFFFFFFFFFE0;
        unsigned long interface_dma_offset_test_p = (unsigned long)test_p - (unsigned long)interface_Ea_align_test_p;
        unsigned long interface_original_test_p;
        interface_sizes = ((n * sizeof(long) + interface_dma_offset_test_p) & 0xFFFFFFFFFFFFFFE0) + 64;
        hthread_printf("__Ea: %p, __size: %d\ninterface_Ea_align_##__Ea: %p, interface_sizes: %d\n", test_p, n * sizeof(long), interface_Ea_align_test_p, interface_sizes);
        channel_number_test_p = dma_p2p((void *)interface_Ea_align_test_p, 1, interface_sizes, 0, interface_buffer_test_p, 1, interface_sizes, 0,
    # 91 "kernel.c" 3 4
                                        0
    # 91 "kernel.c"
                                        ,
                                        0);
        dma_wait(channel_number_test_p);
        interface_original_test_p = test_p;
        test_p = (long *)((unsigned long)interface_buffer_test_p + interface_dma_offset_test_p);
        hthread_printf("Buffer addresses: %p, %p\n", ((long *)interface_Ea_align_test_p), ((long *)interface_buffer_test_p));
        for (int i = 0; i < (n * sizeof(long) / sizeof(long)); i++)
        {
            hthread_printf("DMA original: %ld, DMA buffer: %ld\n", *((long *)interface_original_test_p + i), (long *)test_p[i]);
        };
        hthread_printf("%p\n", test_p);
        for (int i = 1; i < n + 1; i++)
        {
            hthread_printf("A(%ld)  --  test_p(%ld)\n", A[i], test_p[i-1]);
        }
}
