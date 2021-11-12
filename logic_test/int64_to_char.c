// Hello world! Cplayground is an online sandbox that makes it easy to try out
// code.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static const char HEXDIGITS[] = "0123456789abcdef";

uint32_t bits64(int64_t *number)
{
    uint32_t result = 0;
    int64_t low = *number;
    while (low)
    {
        low >>= 1;
        result++;
    }
    return result;
}

void shiftr64(int64_t *number, uint32_t value, int64_t *target)
{
    if (value >= 64)
    {
        *target = 0;
    }
    else if (value == 0)
    {
        *target = *number;
    }
    else if (value < 64)
    {
        *target = *number >> value;
    }
    else
    {
        *target = 0;
    }
}

void shiftl64(int64_t *number, uint32_t value, int64_t *target)
{
    if (value >= 64)
    {
        *target = 0;
    }
    else if (value == 0)
    {
        *target = *number;
    }
    else if (value < 64)
    {
        *target = *number << value;
    }
    else
    {
        *target = 0;
    }
}

void divmod64(int64_t *l, int64_t *r, int64_t *retDiv,
              int64_t *retMod)
{
    int64_t copyd, adder, resDiv = 0, resMod = *l;
    int64_t one = 1;
    uint32_t diffBits = bits64(l) - bits64(r);

    if (*r > *l)
    {
        // printf("divmod64 if %d %d\n", *r, *l);
        *retMod = *l;
        *retDiv = 0;
    }
    else
    {
        shiftl64(r, diffBits, &copyd);
        // printf("shiftl64 %d %d %d\n", *r, diffBits, copyd);
        shiftl64(&one, diffBits, &adder);
        // printf("shiftl64 2 %d %d %d\n", one, diffBits, adder);
        if (copyd > resMod)
        {
            shiftr64(&copyd, 1, &copyd);
            shiftr64(&adder, 1, &adder);
        }
        while (resMod >= *r)
        {
            if (resMod >= copyd)
            {
                resMod = resMod - copyd;
                resDiv = resDiv | adder;
            }
            shiftr64(&copyd, 1, &copyd);
            shiftr64(&adder, 1, &adder);
        }
        *retDiv = resDiv;
        *retMod = resMod;
    }
}

static void reverseString(char *str, uint32_t length)
{
    uint32_t i, j;
    for (i = 0, j = length - 1; i < j; i++, j--)
    {
        uint8_t c;
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

bool tostring64(int64_t *number, uint32_t baseParam, char *out,
                uint32_t outLength)
{
    // printf("test %d %hhx %d %d \n", *number, *number, baseParam, outLength);
    int64_t rDiv = *number;
    int64_t rMod = 0;
    int64_t base = baseParam;
    uint32_t offset = 0;

    // if the number is negative number
    // or baseparam is outside of the range
    // => return false
    if (((*number >> 63) & 1) || (baseParam < 2) || (baseParam > 16))
    {
        // printf("test2 %d %hhx %d %d \n", *number, *number, baseParam, outLength);
        return false;
    }

    do
    {
        if (offset > (outLength - 1))
        {
            return false;
        }
        divmod64(&rDiv, &base, &rDiv, &rMod);
        // printf("divmod64 %d %d %d %d\n", rDiv, base, rDiv, rMod);
        out[offset++] = HEXDIGITS[(uint8_t)rMod];
    } while (rDiv != 0);
    out[offset] = '\0';
    reverseString(out, offset);
    return true;
}

int main()
{
    // long numberLong = 9223372036854775807;
    // int64_t testNumber = (int64_t)numberLong;

    // int outputLength = 100;
    // char out[outputLength], testOut[outputLength];

    // tostring64(&testNumber, 10, (char *)out, 100);
    // snprintf(testOut, outputLength, "%lld", numberLong);
    // printf("%s && %s: %d", out, testOut, strcmp(out, testOut));
    // return 0;

    int outputLength = 100;
    char out[outputLength], testOut[outputLength];

    bool isFailed = false;
    for (long i = -1000000000; i <= 1000000000; i++)
    {
        if (i > 999999899)
            printf("%ld ", i);
        int64_t testNumber = (int64_t)i;
        bool result = tostring64(&testNumber, 10, (char *)out, 100);
        snprintf(testOut, outputLength, "%ld", i);

        if ((i < 0 && result) || (i >= 0 && strcmp(out, testOut) != 0))
        {
            isFailed = true;
        }

        if (isFailed)
        {
            printf("\n%ld: %s && %s: %d", i, out, testOut, strcmp(out, "9223372036854775807"));
            break;
        }
    }
    if (!isFailed)
    {
        printf("\nno failure in converting number range");
    }
    return 0;
}