#include "cache_bluk.h"
#include "cache_direct.h"
#include "cache_single.h"
#include "hthread_device.h"
#include "memMD.h"
#include <compiler/m3000.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define N 500         // 粒子数量
#define DIM 2         // 维度 (2D)
#define BOX_SIZE 10.0 // 模拟箱大小
#define EPSILON 1.0   // Lennard-Jones 势参数
#define SIGMA 1.0     // Lennard-Jones 势参数
#define RCUT 2.5      // 截断距离
#define DT 0.0001     // 时间步长
#define STEPS 100     // 模拟步数
typedef struct
{
    double x[DIM]; // 位置
    double v[DIM]; // 速度
    double f[DIM]; // 力
} Particle;
void output_results(Particle *particles, int step)
{
    hthread_printf("dst Step %d\n", step);
    for (int i = 0; i < N; i++) {
        hthread_printf("Particle %d: ", i);
        for (int d = 0; d < DIM; d++) {
            hthread_printf("x[%d] = %.6f, v[%d] = %.6f, f[%d] = %.6f ", d, particles[i].x[d], d, particles[i].v[d], d,
                           particles[i].f[d]);
        }
        hthread_printf("\n");
    }
    hthread_printf("\n");
}
__global__ void simulate_step(Particle *particles)
{

    for (int step = 0; step < STEPS; step++) {
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
                    if (r[d] > BOX_SIZE / 2.0)
                        r[d] -= BOX_SIZE;
                    if (r[d] < -BOX_SIZE / 2.0)
                        r[d] += BOX_SIZE;
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
                if (particles[i].x[d] >= BOX_SIZE)
                    particles[i].x[d] -= BOX_SIZE;
                if (particles[i].x[d] < 0.0)
                    particles[i].x[d] += BOX_SIZE;
            }
        }

        // 计算总势能
        double potential_energy = 0.0;
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                double r2 = 0.0;
                for (int d = 0; d < DIM; d++) {
                    double r = particles[j].x[d] - particles[i].x[d];
                    if (r > BOX_SIZE / 2.0)
                        r -= BOX_SIZE;
                    if (r < -BOX_SIZE / 2.0)
                        r += BOX_SIZE;
                    r2 += r * r;
                }

                if (r2 < RCUT * RCUT) {
                    double sr6 = pow(SIGMA * SIGMA / r2, 3);
                    potential_energy += 4.0 * EPSILON * (sr6 * sr6 - sr6);
                }
            }
        }
        // hthread_printf("step : %d Potential Energy = %.6f\n", step, potential_energy);
        //                     output_results(particles,step);
    }
}
__global__ void simulate_step_pro(Particle *particles)
{
    // 定义并初始化内存信息
    memory_info_t particles_mem_info;
    strncpy(particles_mem_info.function_name, "simulate_step", sizeof(particles_mem_info.function_name) - 1);

    // 在核函数外部进行开始插桩，保证每个变量只插入一次
    start_instrumentation("particles", (void *)particles, sizeof(Particle), &particles_mem_info);

    int n_side = (int)ceil(sqrt(N));    // 每边的粒子数量
    double spacing = BOX_SIZE / n_side; // 粒子间距

    for (int step = 0; step < STEPS; step++) {
        // output_results(particles, step, 0);
        // hthread_printf("dsp step = %d\n", step);

        // 重置所有粒子的力
        for (int i = 0; i < N; i++) {
            for (int d = 0; d < DIM; d++) {
                // 记录对 particles 的访问
                record_memory_access(&particles_mem_info, (unsigned long)&particles[i]);
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
                    if (r[d] > BOX_SIZE / 2.0)
                        r[d] -= BOX_SIZE;
                    if (r[d] < -BOX_SIZE / 2.0)
                        r[d] += BOX_SIZE;
                    r2 += r[d] * r[d];

                    // 记录对 particles 的访问
                    record_memory_access(&particles_mem_info, (unsigned long)&particles[i]);
                    record_memory_access(&particles_mem_info, (unsigned long)&particles[j]);
                }

                if (r2 < RCUT * RCUT) {
                    double sr6 = pow(SIGMA * SIGMA / r2, 3);
                    double force_factor = 48.0 * EPSILON * sr6 * (sr6 - 0.5) / r2;
                    for (int d = 0; d < DIM; d++) {
                        // 记录对 particles 的访问
                        record_memory_access(&particles_mem_info, (unsigned long)&particles[i]);
                        record_memory_access(&particles_mem_info, (unsigned long)&particles[j]);

                        particles[i].f[d] += force_factor * r[d];
                        particles[j].f[d] -= force_factor * r[d];
                    }
                }
            }
        }

        // 更新粒子的位置和速度 (Velocity Verlet 算法)
        for (int i = 0; i < N; i++) {
            for (int d = 0; d < DIM; d++) {
                // 记录对 particles 的访问
                record_memory_access(&particles_mem_info, (unsigned long)&particles[i]);
                record_memory_access(&particles_mem_info, (unsigned long)&particles[i]);

                particles[i].v[d] += particles[i].f[d] * DT / 2.0;
                particles[i].x[d] += particles[i].v[d] * DT;

                // 应用周期性边界条件 (PBC)
                if (particles[i].x[d] >= BOX_SIZE)
                    particles[i].x[d] -= BOX_SIZE;
                if (particles[i].x[d] < 0.0)
                    particles[i].x[d] += BOX_SIZE;
            }
        }

        // 计算总势能
        double potential_energy = 0.0;
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                double r2 = 0.0;
                for (int d = 0; d < DIM; d++) {
                    double r = particles[j].x[d] - particles[i].x[d];
                    if (r > BOX_SIZE / 2.0)
                        r -= BOX_SIZE;
                    if (r < -BOX_SIZE / 2.0)
                        r += BOX_SIZE;
                    r2 += r * r;

                    // 记录对 particles 的访问
                    record_memory_access(&particles_mem_info, (unsigned long)&particles[i]);
                    record_memory_access(&particles_mem_info, (unsigned long)&particles[j]);
                }

                if (r2 < RCUT * RCUT) {
                    double sr6 = pow(SIGMA * SIGMA / r2, 3);
                    potential_energy += 4.0 * EPSILON * (sr6 * sr6 - sr6);
                }
            }
        }
        // hthread_printf("step : %d Potential Energy = %.6f\n", step, potential_energy);
    }

    // 在核函数外部进行结束插桩，保证每个变量只插入一次
    end_instrumentation(&particles_mem_info);
    hprint_memory_info(&particles_mem_info);
}
__global__ void simulate_step_cache(Particle *particles)
{
    // 初始化缓存环境

    // 初始化缓存，假设Particle结构体大小为sizeof(Particle)，并且N是全局定义的粒子数量
    CACHEb_INIT(particles, Particle, particles, 0, 4096); // 单缓冲区策略，缓冲区大小为2^12 bytes

    for (int step = 0; step < STEPS; step++) {
        // 重置所有粒子的力
        for (int i = 0; i < N; i++) {
            for (int d = 0; d < DIM; d++) {
                Particle temp_particle;
                CACHEb_RD(particles, particles + i, temp_particle, Particle);
                temp_particle.f[d] = 0.0;
                CACHEb_WT(particles, particles + i, temp_particle, Particle);
            }
        }

        // 计算 Lennard-Jones 力
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                Particle particle_i, particle_j;
                CACHEb_RD(particles, particles + i, particle_i, Particle);
                CACHEb_RD(particles, particles + j, particle_j, Particle);

                double r[DIM];
                double r2 = 0.0;
                for (int d = 0; d < DIM; d++) {
                    r[d] = particle_j.x[d] - particle_i.x[d];
                    if (r[d] > BOX_SIZE / 2.0)
                        r[d] -= BOX_SIZE;
                    if (r[d] < -BOX_SIZE / 2.0)
                        r[d] += BOX_SIZE;
                    r2 += r[d] * r[d];
                }

                if (r2 < RCUT * RCUT) {
                    double sr6 = pow(SIGMA * SIGMA / r2, 3);
                    double force_factor = 48.0 * EPSILON * sr6 * (sr6 - 0.5) / r2;
                    for (int d = 0; d < DIM; d++) {
                        particle_i.f[d] += force_factor * r[d];
                        particle_j.f[d] -= force_factor * r[d];
                    }
                    CACHEb_WT(particles, particles + i, particle_i, Particle);
                    CACHEb_WT(particles, particles + j, particle_j, Particle);
                }
            }
        }

        // 更新粒子的位置和速度 (Velocity Verlet 算法)
        for (int i = 0; i < N; i++) {
            Particle temp_particle;
            CACHEb_RD(particles, particles + i, temp_particle, Particle);
            for (int d = 0; d < DIM; d++) {
                temp_particle.v[d] += temp_particle.f[d] * DT / 2.0;
                temp_particle.x[d] += temp_particle.v[d] * DT;

                // 应用周期性边界条件 (PBC)
                if (temp_particle.x[d] >= BOX_SIZE)
                    temp_particle.x[d] -= BOX_SIZE;
                if (temp_particle.x[d] < 0.0)
                    temp_particle.x[d] += BOX_SIZE;
            }
            CACHEb_WT(particles, particles + i, temp_particle, Particle);
        }

        // 计算总势能
        double potential_energy = 0.0;
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                Particle particle_i, particle_j;
                CACHEb_RD(particles, particles + i, particle_i, Particle);
                CACHEb_RD(particles, particles + j, particle_j, Particle);

                double r2 = 0.0;
                for (int d = 0; d < DIM; d++) {
                    double r = particle_j.x[d] - particle_i.x[d];
                    if (r > BOX_SIZE / 2.0)
                        r -= BOX_SIZE;
                    if (r < -BOX_SIZE / 2.0)
                        r += BOX_SIZE;
                    r2 += r * r;
                }

                if (r2 < RCUT * RCUT) {
                    double sr6 = pow(SIGMA * SIGMA / r2, 3);
                    potential_energy += 4.0 * EPSILON * (sr6 * sr6 - sr6);
                }
            }
        }
        // hthread_printf("step : %d Potential Energy = %.6f\n", step, potential_energy);
        //                              output_results(particles,step);
    }

    // 清除缓存
    CACHEb_FLUSH(particles);
}