#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <ctype.h>
#define MAX_PARAMS 100
#define LINE_LENGTH 1024
#define PARAM_LENGTH 2048

typedef struct
{
    char function_name[PARAM_LENGTH];
    char param_type[PARAM_LENGTH];
    char param_name[PARAM_LENGTH];
    int dimension;
    int celue;
    int cets;
    int lines;
} FunctionParamInfo;

// Function to find function declarations and extract pointer/array parameters
void find_pointer_array_params(const char *filename, FunctionParamInfo *param_info_array, int *param_count)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }

    // Updated regex to capture complex types
    const char *pattern = "([a-zA-Z_][a-zA-Z0-9_]*\\s+)?(static\\s+)?(inline\\s+)?(__global__\\s+)?([a-zA-Z_][a-zA-Z0-9_\\s]*\\s+)([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)";

    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED))
    {
        fprintf(stderr, "Failed to compile regex\n");
        fclose(file);
        return;
    }

    char line[LINE_LENGTH];
    regmatch_t matches[8];

    while (fgets(line, sizeof(line), file))
    {
        if (regexec(&regex, line, 8, matches, 0) == 0)
        {
            char function_name[PARAM_LENGTH];
            if (matches[6].rm_so != -1)
            {
                int len = matches[6].rm_eo - matches[6].rm_so;
                if (len >= PARAM_LENGTH)
                    len = PARAM_LENGTH - 1;
                strncpy(function_name, line + matches[6].rm_so, len);
                function_name[len] = '\0';
            }
            else
            {
                fprintf(stderr, "Failed to extract function name\n");
                continue;
            }

            if (matches[7].rm_so != -1)
            {
                char params[LINE_LENGTH];
                int len = matches[7].rm_eo - matches[7].rm_so;
                if (len >= LINE_LENGTH)
                    len = LINE_LENGTH - 1;
                strncpy(params, line + matches[7].rm_so, len);
                params[len] = '\0';

                char *param = strtok(params, ",");
                while (param != NULL)
                {
                    int dimension = 0;

                    if (strchr(param, '*') || strchr(param, '['))
                    {
                        char param_type[PARAM_LENGTH] = {0};
                        char param_name[PARAM_LENGTH] = {0};

                        // Split param into type and name by finding the last space
                        char *last_space = strrchr(param, ' ');
                        if (last_space != NULL)
                        {
                            strncpy(param_type, param, last_space - param);        // Copy type part
                            param_type[last_space - param] = '\0';                 // Null-terminate the type
                            strncpy(param_name, last_space + 1, PARAM_LENGTH - 1); // Copy name part
                        }
                        else
                        {
                            strncpy(param_name, param, PARAM_LENGTH - 1);
                        }

                        // Clean up param_name (remove '*', '[]') and count dimensions
                        char *ptr = param_name;
                        char *dst = param_name;
                        int celue;
                        int cets;
                        int lines;
                        while (*ptr)
                        {
                            if (*ptr == '[')
                            {
                                dimension++;
                                while (*ptr && *ptr != ']')
                                {
                                    ptr++;
                                }
                                if (*ptr == ']')
                                {
                                    ptr++;
                                }
                            }
                            else if (*ptr != '*')
                            {
                                *dst++ = *ptr;
                                ptr++;
                            }
                            else
                            {
                                ptr++;
                            }
                        }
                        *dst = '\0'; // Null-terminate the string

                        if (strchr(param, '*'))
                        {
                            // Prompt user for dimension if a pointer is detected
                            printf("发现需要优化的函数: %s, 变量名: %s\n", function_name, param_name);  
                            printf("\n请输入优化策略代表的数字:\n");  
                            printf("1. 单缓冲区策略\n");  
                            printf("2. 批量访存策略\n");  
                            printf("3. 直接映射策略\n");  
                            printf("请输入选择(1/2/3): ");  
                            scanf("%d", &celue);
                            dimension = 1;
    switch (celue) {  
        case 1:  
            printf("\n选择单缓冲区策略，请输入(lines): ");  
            scanf("%d", &lines);  
            break;  
        case 2:  
            printf("\n选择批量访存策略，无需输入额外参数。\n");  
            break;  
        case 3:  
            printf("\n选择直接映射策略，请输入(sets)和(lines): ");  
            scanf("%d %d", &cets, &lines);  
            break;  
        default:  
            printf("\n无效选择，请输入1, 2, 或3。\n");  
            // 可以添加错误处理逻辑，比如重新请求输入  
            break;  
    }  
                            if (dimension == 0)
                            {
                                param = strtok(NULL, ","); // Skip this parameter
                                continue;
                            }
                        }

                        if (*param_count < MAX_PARAMS)
                        {
                            // Store information in param_info_array
                            strncpy(param_info_array[*param_count].function_name, function_name, PARAM_LENGTH);
                            strncpy(param_info_array[*param_count].param_type, param_type, PARAM_LENGTH);
                            strncpy(param_info_array[*param_count].param_name, param_name, PARAM_LENGTH);
                            param_info_array[*param_count].dimension = dimension;
                            param_info_array[*param_count].celue = celue;
                            param_info_array[*param_count].cets = cets;
                            param_info_array[*param_count].lines = lines;
                            printf("Function: %s, Type: %s, Name: %s, Dimension: %d\n",
                                   function_name, param_type, param_name, dimension);

                            (*param_count)++;
                        }
                        else
                        {
                            fprintf(stderr, "Maximum parameter count exceeded\n");
                            break;
                        }
                    }
                    param = strtok(NULL, ",");
                }
            }
        }
    }

    regfree(&regex);
    fclose(file);
}
void insert_3_celue_code(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function)
{
    for (int i = 0; i < param_count; i++)
    {
        printf("%s, %s, %d, %d,%d\n", param_info_array[i].param_name, param_info_array[i].param_type, param_info_array[i].cets,param_info_array[i].lines,param_count);
        if (strcmp(param_info_array[i].function_name, current_function) == 0 && param_info_array[i].celue == 3)
        {
            const char *param_name = param_info_array[i].param_name;

            fprintf(output_file, "    CACHEe_INIT(%s, %s, %d, %d);\n", param_info_array[i].param_name, param_info_array[i].param_type, param_info_array[i].cets,param_info_array[i].lines);
            fprintf(output_file, "    %s temp_%s;\n",param_info_array[i].param_type,param_info_array[i].param_name);
        }
    }
}
void insert_1_celue_code(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function)
{
    for (int i = 0; i < param_count; i++)
    {
        printf("%s, %s, %d, %d,%d\n", param_info_array[i].param_name, param_info_array[i].param_type, param_info_array[i].cets,param_info_array[i].lines,param_count);
        if (strcmp(param_info_array[i].function_name, current_function) == 0 && param_info_array[i].celue == 1)
        {
            const char *param_name = param_info_array[i].param_name;

            fprintf(output_file, "    CACHEe_INIT(%s, %s, 0,0, %d);\n", param_info_array[i].param_name, param_info_array[i].param_type,param_info_array[i].lines);
            fprintf(output_file, "    %s temp_%s;\n",param_info_array[i].param_type,param_info_array[i].param_name);
        }
    }
}
void insert_end_instrumentation_code(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function)
{
    printf("current_function is : %s;\n", current_function);
    for (int i = 0; i < param_count; i++)
    {
        if (strcmp(param_info_array[i].function_name, current_function) == 0)
        {
            const char *param_name = param_info_array[i].param_name;

            // Insert the end_instrumentation function call at the end of the function
            fprintf(output_file, "    CACHEe_INVALID(%s);\n",
                    param_name);
        }
    }
    fprintf(output_file, "\n");
}
void insert_loop_instrumentation(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function, const char *array_name, const char *loop_variable, const char *line)
{

    // 插入record_memory_access函数调用，使用提取的循环变量
    for(int i = 0;i<param_count;i++){
        if(strcmp(param_info_array[i].param_name, array_name) == 0)
        fprintf(output_file, "    CACHEe_SEC_W_RD(%s, %s + %s, tmp_%s, %s);\n",
                array_name, array_name, loop_variable,array_name,param_info_array[i].param_type);
    }
}

// 判断字符是否是合法的数组名字符
int is_valid_array_char(char c)
{
    return isalnum(c) || c == '_';
}

void insert_loop_instrumentation_code(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function, const char *line)
{
    const char *current_pos = line;// 复制行以便操作
    char array_name[PARAM_LENGTH];
    char loop_var[PARAM_LENGTH];

    while (*current_pos != '\0')
    {
        // 查找 '['，表示数组索引的开始
        if (*current_pos == '[')
        {
            // 向前查找数组名
            const char *name_end = current_pos - 1;
            while (name_end >= line && isspace(*name_end))
            {
                name_end--;
            }

            const char *name_start = name_end;
            while (name_start >= line && is_valid_array_char(*name_start))
            {
                name_start--;
            }
            name_start++;

            // 提取数组名
            int len = name_end - name_start + 1;
            if (len >= PARAM_LENGTH)
                len = PARAM_LENGTH - 1;
            strncpy(array_name, name_start, len);
            array_name[len] = '\0';

            // 查找与 '[' 对应的 ']'
            const char *index_start = current_pos + 1;
            const char *index_end = index_start;
            int bracket_count = 1; // 追踪嵌套的括号数

            while (*index_end != '\0' && bracket_count > 0)
            {
                if (*index_end == '[')
                {
                    bracket_count++;
                }
                else if (*index_end == ']')
                {
                    bracket_count--;
                }
                index_end++;
            }

            if (bracket_count == 0)
            {
                // 提取索引表达式
                len = index_end - index_start - 1;
                if (len >= PARAM_LENGTH)
                    len = PARAM_LENGTH - 1;
                strncpy(loop_var, index_start, len);
                loop_var[len] = '\0';

                // 打印调试信息
                printf("array_name: %s, loop_var: %s\n", array_name, loop_var);

                // 插入record_memory_access
                insert_loop_instrumentation(output_file, param_info_array, param_count, current_function, array_name, loop_var,line);
            }

            // 更新 current_pos 跳过当前数组访问
            current_pos = index_end;
        }
        else
        {
            current_pos++;
        }
    }
}

void insert_loop_instrumentation_2D(FILE *output_file, const FunctionParamInfo *param_info_array, const char *array_name, const char *current_access, const char *current_function)
{
    // 打印传入的参数
    printf("Inside insert_loop_instrumentation_2D with parameters:\n");
    printf("output_file: %p\n", (void *)output_file);
    printf("param_info_array: %p\n", (void *)param_info_array);
    printf("array_name: %s\n", array_name);
    printf("current_access: %s\n", current_access);
    printf("current_function: %s\n", current_function);

    // Insert record_memory_access function call for 2D array access, including constant index
    fprintf(output_file, "    record_memory_access(&%s_%s_mem_info, (unsigned long)&%s);\n",
            current_function, array_name, current_access);
}

void insert_loop_instrumentation_code_2D(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function, const char *line)
{
    const char *pattern = "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]";
    regex_t regex;
    regmatch_t matches[4];

    if (regcomp(&regex, pattern, REG_EXTENDED))
    {
        fprintf(stderr, "Failed to compile regex for loop instrumentation\n");
        return;
    }

    const char *cursor = line;
    char accessed_elements[10][PARAM_LENGTH * 3]; // 保存所有已访问过的元素
    int accessed_count = 0;

    // 打印要匹配的行内容，便于调试
    printf("Processing line: %s\n", line);

    // 继续匹配行中的所有数组访问
    while (regexec(&regex, cursor, 4, matches, 0) == 0 && param_info_array->dimension == 2)
    {
        char array_name[PARAM_LENGTH] = {0};         // 初始化为全零，即空字符串
        char loop_var1[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char loop_var2[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char current_access[PARAM_LENGTH * 3] = {0}; // 初始化为全零，即空字符串

        // 提取数组名称
        int len = matches[1].rm_eo - matches[1].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(array_name, cursor + matches[1].rm_so, len);
        array_name[len] = '\0';

        // 提取第一个维度的下标（可能是变量或常量）
        len = matches[2].rm_eo - matches[2].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var1, cursor + matches[2].rm_so, len);
        loop_var1[len] = '\0';

        // 提取第二个维度的下标（可能是变量或常量）
        len = matches[3].rm_eo - matches[3].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var2, cursor + matches[3].rm_so, len);
        loop_var2[len] = '\0';

        // 组合当前访问的数组元素
        snprintf(current_access, sizeof(current_access), "%s[%s][%s]", array_name, loop_var1, loop_var2);

        // 打印当前访问的数组元素
        printf("Array element access found: %s\n", current_access);

        // 检查该数组元素是否已经记录过
        int already_accessed = 0;
        for (int i = 0; i < accessed_count; ++i)
        {
            if (strcmp(accessed_elements[i], current_access) == 0)
            {
                already_accessed = 1;
                break;
            }
        }

        // 如果当前访问的数组元素没有记录过，插入访存记录
        if (!already_accessed)
        {
            printf("Inserting memory access record for: %s\n", current_access);

            // 打印传入的参数
            // printf("Calling insert_loop_instrumentation_2D with parameters:\n");
            // printf("output_file: %p\n", (void *)output_file);
            // printf("param_info_array: %p\n", (void *)param_info_array);
            // printf("current_function: %s\n", current_function);
            // printf("array_name: %s\n", array_name);
            printf("current_access: %s\n", current_access);

            insert_loop_instrumentation_2D(output_file, param_info_array, array_name, current_access, current_function);
            strncpy(accessed_elements[accessed_count], current_access, sizeof(current_access));
            accessed_count++;

            // 检查是否超过了保存访问元素的数组限制
            if (accessed_count >= 10)
            {
                printf("Warning: exceeded limit of accessed elements array.\n");
            }
        }
        else
        {
            printf("Skipping already recorded access for: %s\n", current_access);
        }

        // 移动 cursor，继续匹配下一个数组访问
        cursor += matches[0].rm_eo;
    }

    regfree(&regex);
    printf("Finished processing line: %s\n", line);
}
// Function to insert loop instrumentation

void insert_loop_instrumentation_3D(FILE *output_file, const FunctionParamInfo *param_info_array, const char *array_name, const char *current_access, const char *current_function)
{
    // Insert record_memory_access function call for 3D array access, including constant index
    fprintf(output_file, "    record_memory_access(&%s_%s_mem_info, (unsigned long)&%s);\n",
            current_function, array_name, current_access);
}

void insert_loop_instrumentation_code_3D(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function, const char *line)
{
    const char *pattern = "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]";
    regex_t regex;
    regmatch_t matches[5];

    if (regcomp(&regex, pattern, REG_EXTENDED))
    {
        fprintf(stderr, "Failed to compile regex for loop instrumentation\n");
        return;
    }

    const char *cursor = line;
    char accessed_elements[10][PARAM_LENGTH * 4]; // 保存所有已访问过的元素
    int accessed_count = 0;

    // 继续匹配行中的所有数组访问
    while (regexec(&regex, cursor, 5, matches, 0) == 0 && param_info_array->dimension == 3)
    {
        char array_name[PARAM_LENGTH] = {0};         // 初始化为全零，即空字符串
        char loop_var1[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char loop_var2[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char loop_var3[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char current_access[PARAM_LENGTH * 4] = {0}; // 初始化为全零，即空字符串

        // 提取数组名称
        int len = matches[1].rm_eo - matches[1].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(array_name, cursor + matches[1].rm_so, len);
        array_name[len] = '\0';

        // 提取第一个维度的下标（可能是变量或常量）
        len = matches[2].rm_eo - matches[2].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var1, cursor + matches[2].rm_so, len);
        loop_var1[len] = '\0';

        // 提取第二个维度的下标（可能是变量或常量）
        len = matches[3].rm_eo - matches[3].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var2, cursor + matches[3].rm_so, len);
        loop_var2[len] = '\0';

        // 提取第三个维度的下标（可能是变量或常量）
        len = matches[4].rm_eo - matches[4].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var3, cursor + matches[4].rm_so, len);
        loop_var3[len] = '\0';

        // 组合当前访问的数组元素
        snprintf(current_access, sizeof(current_access), "%s[%s][%s][%s]", array_name, loop_var1, loop_var2, loop_var3);

        // 检查该数组元素是否已经记录过
        int already_accessed = 0;
        for (int i = 0; i < accessed_count; ++i)
        {
            if (strcmp(accessed_elements[i], current_access) == 0)
            {
                already_accessed = 1;
                break;
            }
        }

        // 如果当前访问的数组元素没有记录过，插入访存记录
        if (!already_accessed)
        {
            printf("Inserting memory access record for: %s\n", current_access);
            insert_loop_instrumentation_3D(output_file, param_info_array, array_name, current_access, current_function);
            strncpy(accessed_elements[accessed_count], current_access, sizeof(current_access));
            accessed_count++;

            // 检查是否超过了保存访问元素的数组限制
            if (accessed_count >= 10)
            {
                printf("Warning: exceeded limit of accessed elements array.\n");
            }
        }
        else
        {
            printf("Skipping already recorded access for: %s\n", current_access);
        }

        // 移动 cursor，继续匹配下一个数组访问
        cursor += matches[0].rm_eo;
    }

    regfree(&regex);
    printf("Finished processing line: %s\n", line);
}
void insert_loop_instrumentation_4D(FILE *output_file, const FunctionParamInfo *param_info_array, const char *array_name, const char *current_access, const char *current_function)
{
    // Insert record_memory_access function call for 4D array access, including constant index
    fprintf(output_file, "    record_memory_access(&%s_%s_mem_info, (unsigned long)&%s);\n",
            current_function, array_name, current_access);
}

void insert_loop_instrumentation_code_4D(FILE *output_file, const FunctionParamInfo *param_info_array, int param_count, const char *current_function, const char *line)
{
    const char *pattern = "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]\\s*\\[\\s*([a-zA-Z_][a-zA-Z0-9_]*|[0-9]+)\\s*\\]";
    regex_t regex;
    regmatch_t matches[6];

    if (regcomp(&regex, pattern, REG_EXTENDED))
    {
        fprintf(stderr, "Failed to compile regex for loop instrumentation\n");
        return;
    }

    const char *cursor = line;
    char accessed_elements[10][PARAM_LENGTH * 5]; // 保存所有已访问过的元素
    int accessed_count = 0;

    // 继续匹配行中的所有数组访问
    while (regexec(&regex, cursor, 6, matches, 0) == 0 && param_info_array->dimension == 4)
    {
        char array_name[PARAM_LENGTH] = {0};         // 初始化为全零，即空字符串
        char loop_var1[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char loop_var2[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char loop_var3[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char loop_var4[PARAM_LENGTH] = {0};          // 初始化为全零，即空字符串
        char current_access[PARAM_LENGTH * 5] = {0}; // 初始化为全零，即空字符串

        // 提取数组名称
        int len = matches[1].rm_eo - matches[1].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(array_name, cursor + matches[1].rm_so, len);
        array_name[len] = '\0';

        // 提取第一个维度的下标（可能是变量或常量）
        len = matches[2].rm_eo - matches[2].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var1, cursor + matches[2].rm_so, len);
        loop_var1[len] = '\0';

        // 提取第二个维度的下标（可能是变量或常量）
        len = matches[3].rm_eo - matches[3].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var2, cursor + matches[3].rm_so, len);
        loop_var2[len] = '\0';

        // 提取第三个维度的下标（可能是变量或常量）
        len = matches[4].rm_eo - matches[4].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var3, cursor + matches[4].rm_so, len);
        loop_var3[len] = '\0';

        // 提取第四个维度的下标（可能是变量或常量）
        len = matches[5].rm_eo - matches[5].rm_so;
        if (len >= PARAM_LENGTH)
            len = PARAM_LENGTH - 1;
        strncpy(loop_var4, cursor + matches[5].rm_so, len);
        loop_var4[len] = '\0';

        // 组合当前访问的数组元素
        snprintf(current_access, sizeof(current_access), "%s[%s][%s][%s][%s]", array_name, loop_var1, loop_var2, loop_var3, loop_var4);

        // 检查该数组元素是否已经记录过
        int already_accessed = 0;
        for (int i = 0; i < accessed_count; ++i)
        {
            if (strcmp(accessed_elements[i], current_access) == 0)
            {
                already_accessed = 1;
                break;
            }
        }

        // 如果当前访问的数组元素没有记录过，插入访存记录
        if (!already_accessed)
        {
            printf("Inserting memory access record for: %s\n", current_access);
            insert_loop_instrumentation_4D(output_file, param_info_array, array_name, current_access, current_function);
            strncpy(accessed_elements[accessed_count], current_access, sizeof(current_access));
            accessed_count++;

            // 检查是否超过了保存访问元素的数组限制
            if (accessed_count >= 10)
            {
                printf("Warning: exceeded limit of accessed elements array.\n");
            }
        }
        else
        {
            printf("Skipping already recorded access for: %s\n", current_access);
        }

        // 移动 cursor，继续匹配下一个数组访问
        cursor += matches[0].rm_eo;
    }

    regfree(&regex);
    printf("Finished processing line: %s\n", line);
}
// Example usage
void instrument_file(const char *input_filename, const char *output_filename, const FunctionParamInfo *param_info_array, int param_count)
{
    FILE *input_file = fopen(input_filename, "r");
    if (!input_file)
    {
        perror("Failed to open input file");
        return;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        perror("Failed to open output file");
        fclose(input_file);
        return;
    }

    char line[LINE_LENGTH];
    int inside_function1 = 0;
    int inside_function2 = 0;
    int block_level = 0; // 用于跟踪代码块的嵌套级别
    int loop_level = 0;  // 用于跟踪循环的嵌套级别
    int if_level = 0;
    char current_function[PARAM_LENGTH] = {0};
    int end_flag = 0;
    fputs("#include \"memdst.h\"\n", output_file); // Correct include directive

    while (fgets(line, sizeof(line), input_file))
    {
        if (strncmp(line, "//", 2) == 0)
        {
            fputs(line, output_file);
            continue;
        }
        // Check for function declaration
        for (int i = 0; i < param_count; i++)
        {
            if (strstr(line, param_info_array[i].function_name) && strchr(line, '(') && strchr(line, ')') && (strstr(line, "int ") || strstr(line, "void ") || strstr(line, "float ") || strstr(line, "double ") || strstr(line, "static ") || strstr(line, "inline ") || strstr(line, "char ") || strstr(line, "short ") || strstr(line, "long ") || strstr(line, "__global__ ")))
            {

                inside_function1 = 1;
                inside_function2 = 1;
                strncpy(current_function, param_info_array[i].function_name, PARAM_LENGTH);
                continue;
            }
        }

        // Write the original line to the output file
        if (inside_function2 && strchr(line, '}') && loop_level == 0 && if_level == 0)
        {
            block_level--;
            if (block_level == 0)
            {
                // 如果块级别返回到 0，说明函数结束
                end_flag = 1;
                insert_end_instrumentation_code(output_file, param_info_array, param_count, current_function);
                inside_function2 = 0; // 重置标志
                end_flag = 0;
            }
        }
                if (end_flag == 0)
        {
            fputs(line, output_file);
        }
        // If at the beginning of a function, insert the instrumentation code
        if (inside_function1 && strchr(line, '{') && loop_level == 0 && if_level == 0)
        {
            block_level++;
            fprintf(output_file, "    CACHEe_ENV();\n");
            insert_1_celue_code(output_file, param_info_array, param_count, current_function);
            //insert_2_celue_code(output_file, param_info_array, param_count, current_function);
            insert_3_celue_code(output_file, param_info_array, param_count, current_function);
            inside_function1 = 0; // Reset the flag after inserting
        }
        // 处理函数块结束
        // Detect loop start and end
        if (strstr(line, "if"))
        {
            if_level++; // Entering a if
            printf("进入if: %s\n", line);
        }
        if (strchr(line, '}') && if_level > 0)
        {
            printf("退出if\n");
            if_level--; // Exiting a if
        }
        if (strstr(line, "for") || strstr(line, "while"))
        {
            loop_level++; // Entering a loop
            printf("进入循环: %s\n", line);
        }
        if (strchr(line, '}') && loop_level > 0)
        {
            printf("退出循环\n");
            loop_level--; // Exiting a loop
        }
        if (loop_level == 1)
        {

            printf("循环级别是1\n");
            insert_loop_instrumentation_code(output_file, param_info_array, param_count, current_function, line);

        }
        else if (loop_level == 2)
        {

            printf("循环级别是2\n");
            insert_loop_instrumentation_code(output_file, param_info_array, param_count, current_function, line);
        }
        else if (loop_level == 3)
        {

            printf("循环级别是3\n");
            insert_loop_instrumentation_code(output_file, param_info_array, param_count, current_function, line);
        }
        else if (loop_level == 4)
        {

            printf("循环级别是3\n");
            insert_loop_instrumentation_code(output_file, param_info_array, param_count, current_function, line);
        }
    }
    fclose(input_file);
    fclose(output_file);
}
int main()
{
    FunctionParamInfo param_info_array[MAX_PARAMS];
    int param_count = 0;

    // Extract pointer/array parameters from the input C file
    find_pointer_array_params("/thfs3/home/xjtu_cx/dst/hthreads_template_unify/1.c", param_info_array, &param_count);

    // Instrument the file based on the extracted parameters
    instrument_file("/thfs3/home/xjtu_cx/dst/hthreads_template_unify/1.c", "/thfs3/home/xjtu_cx/dst/hthreads_template_unify/2.c", param_info_array, param_count);

    // Output the results
    printf("Instrumented %d functions with pointer/array parameters.\n", param_count);
    printf("Instrumented code written to test_output.c\n");

    return 0;
}

