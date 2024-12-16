#include <sys/time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Convert an integer to a string.
 * 
 * @param value Integer to convert.
 * @param base Numerical base (e.g., 10 for decimal).
 * @return Pointer to the dynamically allocated null-terminated string.
 */
char* itoa(int value, int base) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* str = malloc(35); // 35 bytes should be enough for binary representations of an int
    if (!str) return NULL; // In case malloc fails

    char* wstr = str;
    int sign;

    // Validate base
    if (base < 2 || base > 35) { *wstr = '\0'; return str; }

    // Handle negative values
    if ((sign = value) < 0) value = -value;

    // Conversion. Number is reversed.
    do {
        *wstr++ = num[value % base];
    } while (value /= base);

    if (sign < 0) *wstr++ = '-';
    *wstr = '\0';

    // Reverse string
    char *begin = str;
    char *end = wstr - 1;
    char aux;
    while (end > begin) {
        aux = *end;
        *end-- = *begin;
        *begin++ = aux;
    }

    return str;
}

uint64_t doubleToRawBits(double d)
{
    union
    {
        uint64_t i;
        double f;
    } word;
    word.f = d;
    return word.i;
}

uint64_t getCurrentTimeMicros()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return (uint64_t)((time.tv_sec * INT64_C(1000000)) + time.tv_usec);
}

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
    

