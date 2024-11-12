#include <stdio.h>

int main()
{
    int a = 1;
    printf("%ld\n", (unsigned long)((int*)(a)));
    printf("%ld\n", &a);
    return 0;
}