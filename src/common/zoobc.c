#include <stdint.h>
#include "crypto.h"
#include "base32.h"

#define ADDRESS_BYTES_LENGTH 35
#define HASH256_LENGTH 32
#define PREFIX_LENGTH 3
#define PREFIX_ZOOBC "ZBC"
#define BASE32_RESULT_LENGTH 56
#define ZBC_EXPONENT_UNIT 8
#define CHAR_NULL 0
#define CHAR_0 48     // "0"
#define CHAR_POINT 46 // "."

int getAddressLength(int type)
{
    switch (type)
    {
    case 0:
        return 32;

    default:
        return 0;
    }
}

void bytesToZoobcAddress(uint8_t *dst, uint8_t *publicKeyBytes, char *prefix, int prefixSize)
{
    int8_t tempBytes[ADDRESS_BYTES_LENGTH];
    char temp32Result[BASE32_RESULT_LENGTH];
    char hashResult[HASH256_LENGTH];
    int dstOffset = 0;
    cx_sha3_t tempHashContext;

    for (int i = 0; i < 32; i++)
    {
        tempBytes[i] = publicKeyBytes[i];
    }

    // bytes size 35 [public key 32 + 3 char prefix]
    for (int i = 0; i < 3; i++)
    {
        tempBytes[i + 32] = prefix[i];
    }

    // change the last 3 bytes to the first 3 bytes of checksum
    cx_sha3_init(&tempHashContext, 256);
    hash3_256(&tempHashContext, hashResult, tempBytes, ADDRESS_BYTES_LENGTH);

    for (int i = 0; i < 3; i++)
    {
        tempBytes[i + 32] = hashResult[i];
    }

    // encode the 35 bytes to 32bytes RFC4648
    base32_encode(tempBytes, ADDRESS_BYTES_LENGTH, temp32Result);

    // append the prefix with the 56 charactes separated by _ every 8 bytes
    for (int i = 0; i < prefixSize; i++)
    {
        dst[dstOffset] = prefix[i];
        dstOffset++;
    }

    for (int i = 0; i < BASE32_RESULT_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            dst[dstOffset] = '_';
            dstOffset++;
        }
        dst[dstOffset] = temp32Result[i];
        dstOffset++;
    }
}

void bytesToZoobcAccountAddress(uint8_t *dst, uint8_t *publicKeyBytes)
{
    bytesToZoobcAddress(dst, publicKeyBytes, "ZBC", 3);
}

void bytesToZoobcNodeAddress(uint8_t *dst, uint8_t *publicKeyBytes)
{
    bytesToZoobcAddress(dst, publicKeyBytes, "ZNK", 3);
}

void convertDigits(char *digit, int totalArraylength)
{
    char temp[totalArraylength];
    int lengthDigit = 0;
    int lengthNormalizedDigit = 0;
    int tempIdx = 0;
    int floatPlacement = 0;

    // finding the length of digit
    for (int i = 0; i < totalArraylength; i++)
    {
        if (digit[i] == 0)
        {
            lengthDigit = i;
            break;
        }
    }
    floatPlacement = lengthDigit - ZBC_EXPONENT_UNIT - 1;

    int counter = 0;
    // creating normalized digits in reverse order
    for (int i = lengthDigit - 1; i >= 0; i--)
    {
        counter++;
        // skipping zero digit behind point
        if (digit[i] == CHAR_0 && i > floatPlacement)
        {
            continue;
        }

        if (counter - 1 == ZBC_EXPONENT_UNIT && tempIdx != 0)
        {
            temp[tempIdx] = CHAR_POINT;
            tempIdx++;
        }
        temp[tempIdx] = digit[i];
        tempIdx++;
    }
    tempIdx--;

    if (floatPlacement < 0)
    {
        tempIdx++;
        while (floatPlacement < 0 - 1)
        {
            temp[tempIdx] = CHAR_0;
            tempIdx++;
            floatPlacement++;
        }
        temp[tempIdx] = CHAR_POINT;
        tempIdx++;
        temp[tempIdx] = CHAR_0;
    }

    // reversing the result
    int digitIdx = 0;
    for (int i = tempIdx; i >= 0; i--)
    {
        digit[digitIdx] = temp[i];
        digitIdx++;
    }

    // adding space and ZBC units
    digit[digitIdx] = *" ";
    digitIdx++;
    digit[digitIdx] = *"Z";
    digitIdx++;
    digit[digitIdx] = *"B";
    digitIdx++;
    digit[digitIdx] = *"C";
    digitIdx++;

    // appending end character
    digit[digitIdx] = CHAR_NULL;
}