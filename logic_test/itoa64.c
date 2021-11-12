#include <stdio.h>
#include <stdlib.h>

// inline function to swap two numbers
inline void swap(char *x, char *y)
{
    char t = *x;
    *x = *y;
    *y = t;
}

// function to reverse buffer[i..j]
char *reverse(char *buffer, int i, int j)
{
    while (i < j)
        swap(&buffer[i++], &buffer[j--]);

    return buffer;
}

// Iterative function to implement itoa() function in C
char *itoa(int64_t value, char *buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32)
        return buffer;

    // consider absolute value of number
    int64_t n = abs(value);

    int i = 0;
    while (n)
    {
        int64_t r = n % base;

        if (r >= 10)
            buffer[i++] = 65 + (r - 10);
        else
            buffer[i++] = 48 + r;

        n = n / base;
    }

    // if number is 0
    if (i == 0)
        buffer[i++] = '0';

    // If base is 10 and value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10)
        buffer[i++] = '-';

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}

// implement itoa function in C
int main(void)
{
    char buffer[60];

    int64_t value[] = {9223372036854775807, -25, 64, 127};
    int64_t base[] = {10, 10, 8, 2};

    for (int i = 0; i < 4; i++)
    {
        printf("itoa(%lld, buffer, %d) = %s\n", value[i], base[i],
               itoa(value[i], buffer, base[i]));
    }

    return 0;
}