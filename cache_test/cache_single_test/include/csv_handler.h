#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h> // 引入可变参数功能的库

// 定义 CSV 文件结构
typedef struct {
    FILE *fp;
    char *filename;
} CSVFile;

// 打开 CSV 文件
CSVFile* open_csv(const char *filename, const char *mode) {
    CSVFile *csv = (CSVFile *)malloc(sizeof(CSVFile));
    if (!csv) return NULL;  // 内存分配失败

    csv->filename = strdup(filename);  // 保存文件名
    csv->fp = fopen(filename, mode);   // 打开文件
    if (csv->fp == NULL) {
        free(csv);
        return NULL;
    }
    return csv;
}

// 写入表头，接收多个字符串参数
void write_header(CSVFile *csv, int count, ...) {
    va_list args;
    va_start(args, count); // 初始化可变参数列表

    if (csv && csv->fp) {
        for (int i = 0; i < count; i++) {
            const char* field = va_arg(args, const char*);
            fprintf(csv->fp, "%s", field);
            if (i < count - 1) {
                fprintf(csv->fp, ","); // 添加逗号分隔符，除了最后一个字段
            }
        }
        fprintf(csv->fp, "\n"); // 每行结束后添加换行符
    }

    va_end(args); // 清理可变参数列表
}

// 写入数据行，接收多个字符串参数
void write_row(CSVFile *csv, int count, ...) {
    va_list args;
    va_start(args, count); // 初始化可变参数列表

    if (csv && csv->fp) {
        for (int i = 0; i < count; i++) {
            const char* field = va_arg(args, const char*);
            fprintf(csv->fp, "%s", field);
            if (i < count - 1) {
                fprintf(csv->fp, ","); // 添加逗号分隔符，除了最后一个字段
            }
        }
        fprintf(csv->fp, "\n"); // 每行结束后添加换行符
    }

    va_end(args); // 清理可变参数列表
}

// 关闭 CSV 文件
void close_csv(CSVFile *csv) {
    if (csv) {
        if (csv->fp) fclose(csv->fp);  // 关闭文件指针
        free(csv->filename);           // 释放存储的文件名
        free(csv);                     // 释放结构体内存
    }
}

#endif // CSV_HANDLER_H
