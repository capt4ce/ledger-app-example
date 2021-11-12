// Hello world! Cplayground is an online sandbox that makes it easy to try out
// code.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define UPPER_P(x) x->elements[0]
#define LOWER_P(x) x->elements[1]
#define UPPER(x) x.elements[0]
#define LOWER(x) x.elements[1]

static const char HEXDIGITS[] = "0123456789abcdef";

typedef struct uint128_t
{
    uint64_t elements[2];
} uint128_t;

void copy128(uint128_t *target, uint128_t *number)
{
    UPPER_P(target) = UPPER_P(number);
    LOWER_P(target) = LOWER_P(number);
}

void clear128(uint128_t *target)
{
    UPPER_P(target) = 0;
    LOWER_P(target) = 0;
}

uint32_t bits128(uint128_t *number)
{
    uint32_t result = 0;
    if (UPPER_P(number))
    {
        result = 64;
        uint64_t up = UPPER_P(number);
        while (up)
        {
            up >>= 1;
            result++;
        }
    }
    else
    {
        uint64_t low = LOWER_P(number);
        while (low)
        {
            low >>= 1;
            result++;
        }
    }
    return result;
}

bool gt128(uint128_t *number1, uint128_t *number2)
{
    if (UPPER_P(number1) == UPPER_P(number2))
    {
        return (LOWER_P(number1) > LOWER_P(number2));
    }
    return (UPPER_P(number1) > UPPER_P(number2));
}

bool equal128(uint128_t *number1, uint128_t *number2)
{
    return (UPPER_P(number1) == UPPER_P(number2)) &&
           (LOWER_P(number1) == LOWER_P(number2));
}

bool gte128(uint128_t *number1, uint128_t *number2)
{
    return gt128(number1, number2) || equal128(number1, number2);
}

void minus128(uint128_t *number1, uint128_t *number2, uint128_t *target)
{
    UPPER_P(target) =
        UPPER_P(number1) - UPPER_P(number2) -
        ((LOWER_P(number1) - LOWER_P(number2)) > LOWER_P(number1));
    LOWER_P(target) = LOWER_P(number1) - LOWER_P(number2);
}

void or128(uint128_t *number1, uint128_t *number2, uint128_t *target)
{
    UPPER_P(target) = UPPER_P(number1) | UPPER_P(number2);
    LOWER_P(target) = LOWER_P(number1) | LOWER_P(number2);
}

void shiftr128(uint128_t *number, uint32_t value, uint128_t *target)
{
    if (value >= 128)
    {
        clear128(target);
    }
    else if (value == 64)
    {
        UPPER_P(target) = 0;
        LOWER_P(target) = UPPER_P(number);
    }
    else if (value == 0)
    {
        copy128(target, number);
    }
    else if (value < 64)
    {
        uint128_t result;
        UPPER(result) = UPPER_P(number) >> value;
        LOWER(result) =
            (UPPER_P(number) << (64 - value)) + (LOWER_P(number) >> value);
        copy128(target, &result);
    }
    else if ((128 > value) && (value > 64))
    {
        LOWER_P(target) = UPPER_P(number) >> (value - 64);
        UPPER_P(target) = 0;
    }
    else
    {
        clear128(target);
    }
}

void shiftl128(uint128_t *number, uint32_t value, uint128_t *target)
{
    if (value >= 128)
    {
        clear128(target);
    }
    else if (value == 64)
    {
        UPPER_P(target) = LOWER_P(number);
        LOWER_P(target) = 0;
    }
    else if (value == 0)
    {
        copy128(target, number);
    }
    else if (value < 64)
    {
        UPPER_P(target) =
            (UPPER_P(number) << value) + (LOWER_P(number) >> (64 - value));
        LOWER_P(target) = (LOWER_P(number) << value);
    }
    else if ((128 > value) && (value > 64))
    {
        UPPER_P(target) = LOWER_P(number) << (value - 64);
        LOWER_P(target) = 0;
    }
    else
    {
        clear128(target);
    }
}

void divmod128(uint128_t *l, uint128_t *r, uint128_t *retDiv,
               uint128_t *retMod)
{
    uint128_t copyd, adder, resDiv, resMod;
    uint128_t one;
    UPPER(one) = 0;
    LOWER(one) = 1;
    uint32_t diffBits = bits128(l) - bits128(r);
    clear128(&resDiv);
    copy128(&resMod, l);
    if (gt128(r, l))
    {
        copy128(retMod, l);
        clear128(retDiv);
    }
    else
    {
        shiftl128(r, diffBits, &copyd);
        shiftl128(&one, diffBits, &adder);
        if (gt128(&copyd, &resMod))
        {
            shiftr128(&copyd, 1, &copyd);
            shiftr128(&adder, 1, &adder);
        }
        while (gte128(&resMod, r))
        {
            if (gte128(&resMod, &copyd))
            {
                minus128(&resMod, &copyd, &resMod);
                or128(&resDiv, &adder, &resDiv);
            }
            shiftr128(&copyd, 1, &copyd);
            shiftr128(&adder, 1, &adder);
        }
        copy128(retDiv, &resDiv);
        copy128(retMod, &resMod);
    }
}

bool zero128(uint128_t *number)
{
    return ((LOWER_P(number) == 0) && (UPPER_P(number) == 0));
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

bool tostring128(uint128_t *number, uint32_t baseParam, char *out,
                 uint32_t outLength)
{
    uint128_t rDiv;
    uint128_t rMod;
    uint128_t base;
    copy128(&rDiv, number);
    clear128(&rMod);
    clear128(&base);
    LOWER(base) = baseParam;
    uint32_t offset = 0;
    if ((baseParam < 2) || (baseParam > 16))
    {
        return false;
    }
    do
    {
        if (offset > (outLength - 1))
        {
            return false;
        }
        divmod128(&rDiv, &base, &rDiv, &rMod);
        out[offset++] = HEXDIGITS[(uint8_t)LOWER(rMod)];
    } while (!zero128(&rDiv));
    out[offset] = '\0';
    reverseString(out, offset);
    return true;
}

int main()
{
    uint128_t testNumber;
    char out[100];

    UPPER(testNumber) = 0;
    LOWER(testNumber) = 10;

    tostring128(&testNumber, 10, (char *)out, 100);
    printf("%s", out);
    return 0;
}