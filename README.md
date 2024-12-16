# MT-Cache

MT-Cache 是专为 MT-3000 异构片上架构设计的片上缓存软件管理策略库。该库提供了三种缓存管理策略，并支持性能数据采集功能。

## 功能特点

- 提供三种缓存管理策略：直接映射、单缓冲区和批量缓存
- 支持可选的性能数据采集
- 线程安全设计，支持多线程应用
- 统一的API接口，便于使用和维护
- 通过编译选项灵活控制性能采集功能

## 快速开始

### 1. 配置项目

在您的 `Makefile` 中添加：
```makefile
# DEV_CFLAGS 为设备端代码编译选项，可替换为自定义变量名
ifdef PROF
    DEV_CFLAGS += -DENABLE_CACHE_PROFILING
endif
```

### 2. 编写代码

```c
#include "cache_wrapper.h"

__global__ void device_kernel() {
    int *x_array;
    int x_array_tmp;
    
    // 1. 初始化缓存
    CACHEx_INIT(x_array, int, addr, 4, 6);
    
    // 2. 执行缓存操作
    // ... 您的缓存操作代码 ...
    
    // 3. 打印性能统计（可选，仅在性能采集模式下有效）
    CACHEx_STATUS(x_array);
    
    // 4. 清理缓存
    CACHEx_FLUSH(x_array);
    
    return;
}
```

### 3. 编译运行

```bash
# 启用性能采集
make PROF=1

# 不启用性能采集
make
```

## 缓存策略 API

所有策略共享统一的 API 接口风格：

### 直接映射缓存 (CACHEd)
```c
CACHEd_INIT(...)    // 初始化缓存
CACHEd_RD(...)      // 读取数据
CACHEd_WT(...)      // 写入数据
CACHEd_STATUS(...)  // 性能统计（仅性能采集模式）
CACHEd_FLUSH(...)   // 清理缓存
```

### 单缓冲区缓存 (CACHEs)
```c
CACHEs_INIT(...)    // 初始化缓存
CACHEs_RD(...)      // 读取数据
CACHEs_WT(...)      // 写入数据
CACHEs_STATUS(...)  // 性能统计（仅性能采集模式）
CACHEs_FLUSH(...)   // 清理缓存
```

### 批量缓存 (CACHEb)
```c
CACHEb_INIT(...)    // 初始化缓存
CACHEb_RD(...)      // 读取数据
CACHEb_WT(...)      // 写入数据
CACHEb_STATUS(...)  // 性能统计（仅性能采集模式）
CACHEb_FLUSH(...)   // 清理缓存
```

## 性能采集功能

### 采集内容
- 读写操作次数统计
- 读写命中率分析
- 整体缓存命中率

### 调试输出
使用 `CACHE_PRINT` 宏进行调试输出：
```c
CACHE_PRINT("调试信息：缓存操作已完成\n");
CACHE_PRINT("性能指标：命中率 = %.2f%%\n", hit_rate);
```

## 最佳实践

### 推荐做法
1. 统一使用 `cache_wrapper.h` 作为头文件
2. 采用 `CACHE_PRINT` 进行调试输出
3. 遵循初始化 -> 操作 -> 统计 -> 清理的使用流程
4. 使用编译选项控制性能采集功能

### 注意事项
1. 开启性能采集可能略微影响性能
2. 性能统计功能支持多线程环境
3. 两种版本的内存占用基本相同（性能采集版本额外包含计数器变量）
4. 切换性能采集模式只需修改编译选项，无需改动代码

## 维护说明

1. 性能采集版本（*_pro.h）与基础版本的核心逻辑保持一致
2. 添加新功能时需同时更新两个版本
3. 统计代码不应影响核心功能的执行