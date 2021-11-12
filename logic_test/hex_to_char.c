// Hello world! Cplayground is an online sandbox that makes it easy to try out
// code.

#include <stdio.h>
#include <stdlib.h>

int main()
{
    char a[4];
    snprintf(a, sizeof(a), "%02X", 0x2c);
    printf("%d\n", *(int *)a);
    snprintf(a, sizeof(a), "%02X", 0x0000002c);
    printf("%d\n", *(int *)a);
    printf("0x%x\n", 44);
    printf("%d", 0x2c);
    return 0;
}