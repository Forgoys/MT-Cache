#include <stdint.h>
#include <compiler/m3000.h>
#include "hthread_device.h"

static inline void swap(double *a, double *b) {  
    double temp = *a;  
    *a = *b;  
    *b = temp;  
}  
  
// Serial QuickSort function (ascending order)  
static inline void quicksort_serial(double *arr, int left, int right) {    
    if (left < right) {    
        double pivot = arr[right]; // 选择最右边的元素作为枢轴    
        int i = left - 1;    
        for (int j = left; j < right; j++) {    
            // 如果当前元素小于枢轴，则将其移动到枢轴的左侧    
            if (arr[j] < pivot) {    
                i++;    
                swap(&arr[i], &arr[j]);    
            }    
        }    
        swap(&arr[i + 1], &arr[right]); // 将枢轴放到正确的位置    
        int pivotIndex = i + 1;    
    
        // 递归地对枢轴左侧和右侧的子数组进行排序    
        quicksort_serial(arr, left, pivotIndex - 1);    
        quicksort_serial(arr, pivotIndex + 1, right);    
    }    
}

// Parallel QuickSort kernel
__global__ void quicksort_kernel(uint64_t n, double *arr) {
    int threadId = get_thread_id();
    int threadsNum = get_group_size();
    if (threadId >= threadsNum)
        return;

    // Calculate the portion of the array for this thread
    uint64_t n_p = n / threadsNum;
    uint64_t extras = n % threadsNum;
    uint64_t offset;
    if (threadId < extras) {
        n_p++;
        offset = threadId * n_p;
    } else {
        offset = threadId * (n_p + 1) - (threadId - extras);
    }

    // Sort the assigned segment of the array
    quicksort_serial(arr + offset, 0, n_p - 1);
}

