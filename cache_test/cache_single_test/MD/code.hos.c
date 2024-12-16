#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>
#include "hthread_host.h"
#include <stdbool.h>
#define N 500      // 粒子数量
#define DIM 2        // 维度 (2D)
#define BOX_SIZE 10.0 // 模拟箱大小
#define EPSILON 1.0  // Lennard-Jones 势参数
#define SIGMA 1.0    // Lennard-Jones 势参数
#define RCUT 2.5     // 截断距离
#define DT 0.0001     // 时间步长
#define STEPS 100// 模拟步数
#define TOLERANCE 1e-2  // 比较时的容差
#define VELOCITY_SCALE 0.1 // 速度缩放因子
#define GRID_SIZE 200 // 位置网格大小
#define VELOCITY_GRID_SIZE 1000 // 速度网格大小
//不用管
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
uint64_t getCurrentTimeMicros()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000;
}
typedef struct {
    double x[DIM];   // 位置
    double v[DIM];   // 速度
    double f[DIM];   // 力
} Particle;

// 初始化粒子位置和速度
void initialize_particles(Particle *particles) {
    // 计算每边的粒子数量
    int n_side = (int)ceil(sqrt(N));  // 假设是二维情况
    double spacing = BOX_SIZE / n_side;  // 粒子间距

    // 使用固定的种子以确保结果可重复
    srand(42);  // 固定种子 42，可以根据需要更改

    for (int i = 0; i < N; i++) {
        // 计算粒子在网格中的行和列
        int row = i / n_side;
        int col = i % n_side;

        // 位置：将粒子放置在规则的网格上
        for (int d = 0; d < DIM; d++) {
            if (d == 0) {
                particles[i].x[d] = (col + 0.5) * spacing;  // x 坐标
            } else if (d == 1) {
                particles[i].x[d] = (row + 0.5) * spacing;  // y 坐标
            }
        }

        // 速度：将速度限制在离散的网格上，并减去 0.5 以确保速度在 [-0.05, 0.05] 范围内
        for (int d = 0; d < DIM; d++) {
            particles[i].v[d] = ((rand() % 1000) / 1000.0 - 0.5) * VELOCITY_SCALE;
        }

        // 力初始化为零
        for (int d = 0; d < DIM; d++) {
            particles[i].f[d] = 0.0;
        }
    }
}

// 模拟一步：计算力、更新位置和速度、计算势能
void simulate_step(Particle *particles) {
    int n_side = (int)ceil(sqrt(N));  // 每边的粒子数量
    double spacing = BOX_SIZE / n_side;  // 粒子间距
    for (int step = 0; step < STEPS; step++) {
        //output_results(particles,step,0);
    // 重置所有粒子的力
    for (int i = 0; i < N; i++) {
        for (int d = 0; d < DIM; d++) {
            particles[i].f[d] = 0.0;
        }
    }

    // 计算 Lennard-Jones 力
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            double r[DIM];
            double r2 = 0.0;
            for (int d = 0; d < DIM; d++) {
                r[d] = particles[j].x[d] - particles[i].x[d];
                if (r[d] > BOX_SIZE / 2.0) r[d] -= BOX_SIZE;
                if (r[d] < -BOX_SIZE / 2.0) r[d] += BOX_SIZE;
                r2 += r[d] * r[d];
            }

            if (r2 < RCUT * RCUT) {
                double sr6 = pow(SIGMA * SIGMA / r2, 3);
                double force_factor = 48.0 * EPSILON * sr6 * (sr6 - 0.5) / r2;
                for (int d = 0; d < DIM; d++) {
                    particles[i].f[d] += force_factor * r[d];
                    particles[j].f[d] -= force_factor * r[d];
                }
            }
        }
    }

    // 更新粒子的位置和速度 (Velocity Verlet 算法)
    for (int i = 0; i < N; i++) {
        for (int d = 0; d < DIM; d++) {
            particles[i].v[d] += particles[i].f[d] * DT / 2.0;
            particles[i].x[d] += particles[i].v[d] * DT;

            // 应用周期性边界条件 (PBC)
            if (particles[i].x[d] >= BOX_SIZE) particles[i].x[d] -= BOX_SIZE;
            if (particles[i].x[d] < 0.0) particles[i].x[d] += BOX_SIZE;
        }
    }

    // 计算总势能
    double potential_energy = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            double r2 = 0.0;
            for (int d = 0; d < DIM; d++) {
                double r = particles[j].x[d] - particles[i].x[d];
                if (r > BOX_SIZE / 2.0) r -= BOX_SIZE;
                if (r < -BOX_SIZE / 2.0) r += BOX_SIZE;
                r2 += r * r;
            }

            if (r2 < RCUT * RCUT) {
                double sr6 = pow(SIGMA * SIGMA / r2, 3);
                potential_energy += 4.0 * EPSILON * (sr6 * sr6 - sr6);
            }
        }
    }
    //        output_results(particles, step, 0);
    //printf("cpu:potential_energy = %.6f\n", potential_energy);
    }
}

// 输出粒子的位置和能量
void output_results(Particle *particles, int step, double potential_energy) {
    printf("Step %d: Potential Energy = %.6f\n", step, potential_energy);
    for (int i = 0; i < N; i++) {
        printf("Particle %d: ", i);
        for (int d = 0; d < DIM; d++) {
            printf("x[%d] = %.6f, v[%d] = %.6f, f[%d] = %.6f ", d, particles[i].x[d], d, particles[i].v[d], d, particles[i].f[d]);
        }
        printf("\n");
    }
    printf("\n");
}
void compare_results(Particle *particles1, Particle *particles2) {
    int differences = 0;
    printf("Comparing results at Step %d:\n", STEPS);

    for (int i = 0; i < N; i++) {
        for (int d = 0; d < DIM; d++) {
            // 比较位置、速度和力
            if (fabs(particles1[i].x[d] - particles2[i].x[d]) > TOLERANCE ||
                fabs(particles1[i].v[d] - particles2[i].v[d]) > TOLERANCE ||
                fabs(particles1[i].f[d] - particles2[i].f[d]) > TOLERANCE) {
                differences++;
                printf("Difference found in Particle %d:\n", i);
                printf("  x[%d]: %.6f vs %.6f\n", d, particles1[i].x[d], particles2[i].x[d]);
                printf("  v[%d]: %.6f vs %.6f\n", d, particles1[i].v[d], particles2[i].v[d]);
                printf("  f[%d]: %.6f vs %.6f\n", d, particles1[i].f[d], particles2[i].f[d]);
                printf("\n");
            }
        }
    }

    if (differences == 0) {
        printf("No differences found. The two particle systems are identical.\n");
    } else {
        printf("Total differences found: %d\n", differences);
    }

    printf("\n");
}
int main(int argc, char **argv)
{
    unsigned long retc;
    unsigned long clusterId = 1;
    char *devProgram = "./code.dev.dat";
    unsigned long nthreads = 1;
    //改成hn即可
    char *kernel1 = "simulate_step";
    char *kernel2 = "simulate_step_pro";
    char *kernel3 = "simulate_step_cache";
    //改成hn即可
    uint64_t timeGold, timeDev, timeGoldsum = 0;

    if (fileIsExist(devProgram) != 0)
    {
        M_logError("No such file or directory: %s", devProgram);
        return 2;
    }
    printf("Device initialized successfully.\n");

    // ==============================设备初始化====================================
    bool clusterFound = false;
    for (clusterId = 0; clusterId <= 3; clusterId++)
    {
        printf("Checking clusterId %lu...\n", clusterId);
        retc = hthread_dev_open(clusterId);
        if (retc != 0)
        {
            printf("Failed to open device on clusterId %lu\n", clusterId);
            continue; // 尝试下一个 clusterId
        }

        retc = hthread_dat_load(clusterId, devProgram);
        if (retc != 0)
        {
            printf("Failed to load data on clusterId %lu\n", clusterId);
            hthread_dev_close(clusterId); // 关闭设备后继续
            continue;
        }

        int availThreads = hthread_get_avail_threads(clusterId);
        if (nthreads <= availThreads)
        {
            printf("Sufficient threads available on clusterId %lu: available = %d, required = %lu\n", clusterId, availThreads, nthreads);
            clusterFound = true;
            break; // 找到合适的 clusterId，跳出循环
        }
        else
        {
            printf("Not enough threads on clusterId %lu: available = %d, required = %lu\n", clusterId, availThreads, nthreads);
            hthread_dat_unload(clusterId);
            hthread_dev_close(clusterId); // 关闭设备后继续
        }
    }

    if (!clusterFound)
    {
        printf("No suitable clusterId found with enough threads. Exiting program.\n");
        return 2;
    }
    printf("Device initialized successfully.\n");
    // 开始写cpu端
    //cpu端
    Particle *particles = (Particle *)malloc(N * sizeof(Particle));
    if (particles == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    // 初始化粒子
    initialize_particles(particles);
    Particle *base1 = (Particle *)malloc(N * sizeof(Particle));
    if (base1 == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
        memcpy(base1,particles,N * sizeof(Particle));
            Particle *base2 = (Particle *)malloc(N * sizeof(Particle));
    if (base2 == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
        memcpy(base2,particles,N * sizeof(Particle));
    // 模拟循环
    //for (int step = 0; step < STEPS; step++) {
        // 模拟一步并计算势能
        //printf("cpu step = %d\n",step);
        simulate_step(particles);

        // 输出结果
    //}
    //加速端
    Particle *x = (Particle *)hthread_malloc(clusterId, (size_t)N * sizeof(Particle), HT_MEM_RW);
    M_checkMalloc(x);
    memcpy(x,base1,N * sizeof(Particle));
    uint64_t args[1];//有几个参数
    args[0] = (uint64_t)x;//指针类型传地址
        uint64_t time = getCurrentTimeMicros();
        int threadId = hthread_group_create(clusterId, nthreads, kernel1, 0, 1, args);
        // 输出结果
        hthread_group_wait(threadId);
        //compare_results(x,particles);
        time = getCurrentTimeMicros() - time;
        printf("\nTime for kernel1 execution: %llu microseconds\n", time);

    Particle *y = (Particle *)hthread_malloc(clusterId, (size_t)N * sizeof(Particle), HT_MEM_RW);
    M_checkMalloc(y);
    memcpy(y,base2,N * sizeof(Particle));
    uint64_t args1[1];//有几个参数
    args1[0] = (uint64_t)y;//指针类型传地址
        uint64_t time1 = getCurrentTimeMicros();
        int threadId1 = hthread_group_create(clusterId, nthreads, kernel3, 0, 1, args1);
        hthread_group_wait(threadId1);
        time1 = getCurrentTimeMicros() - time1;
        printf("\nTime for kernel3 execution: %llu microseconds\n", time1);

        if (time1 > 0) {
            float speedup = (float)time / time1;
            printf("\nSpeedup: %f\n", speedup);
        } else {
            printf("\nSpeedup: Error in kernel3 execution (time1 is zero)\n");
        }


    retc = hthread_dat_unload(clusterId);
    M_checkRetC(retc, hthread_dat_unload);
    hthread_dev_close(clusterId);
    M_checkRetC(retc, hthread_dev_close);
	return 0;
}