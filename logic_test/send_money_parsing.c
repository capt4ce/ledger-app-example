#include <stdio.h>

#define SIZE 153
#define ADDRESS_SIZE 50
#define INT32_LENGTH 4
#define INT64_LENGTH 8

// unsigned TX example: 010000000170f0f94a000000002c00000042435a45474f6233574e783366444f5666395a5334456a764f49765f55655734545642514a5f3674484b6c452c000000565a76596438307035532d7278534e516d4d5a77595843374c79417a426d6366636a344d555541647564574d01000000000000000800000000f2052a01000000000000000000000000000000000000000000000000000000

int main()
{
    char unsignedTx[SIZE];
    char senderAddress[ADDRESS_SIZE];
    char recipientAddress[ADDRESS_SIZE];
    int dataOffset = 13;
    int addressLength = 44;
    char breakChar[] = "\0";
    char tempInt32Byte[4];
    unsigned char tempInt64Byte[8];
    int tempInt32 = 0;
    long tempInt64 = 0;
    int c;
    int x = 0;

    printf("Type hex (l/c) digits, 0 to end: ");
    while (x < SIZE)
    {
        scanf("%2x", &c);
        unsignedTx[x] = (char)c;
        // if(c == 0) break;
        x++;
    }
    printf("\nUnsignedTx:%s\n", unsignedTx);

    // parsing length of sender senderAddress
    for (int i = 0; i < INT32_LENGTH; i++)
    {
        tempInt32Byte[i] = unsignedTx[i + dataOffset];
    }
    dataOffset += INT32_LENGTH;
    tempInt32 = *(int *)tempInt32Byte;
    printf("senderAddress Length: %d", tempInt32);

    for (int i = 0; i < ADDRESS_SIZE; i++)
    {
        if (i >= addressLength)
        {
            // printf("\ndebug:%c\n", breakChar[0]);
            senderAddress[i] = breakChar[0];
            continue;
        }
        senderAddress[i] = unsignedTx[i + dataOffset];
    }
    dataOffset += addressLength;
    printf("\nSender Address:%s\n", senderAddress);

    // parsing length of sender senderAddress
    for (int i = 0; i < INT32_LENGTH; i++)
    {
        tempInt32Byte[i] = unsignedTx[i + dataOffset];
    }
    dataOffset += INT32_LENGTH;
    tempInt32 = *(int *)tempInt32Byte;
    printf("recipient Address Length: %d", tempInt32);

    for (int i = 0; i < ADDRESS_SIZE; i++)
    {
        if (i >= addressLength)
        {
            // printf("\ndebug:%c\n", breakChar[0]);
            recipientAddress[i] = breakChar[0];
            continue;
        }
        recipientAddress[i] = unsignedTx[i + dataOffset];
    }
    dataOffset += addressLength;
    printf("\nRecipient Address:%s\n", recipientAddress);

    // parsing fee
    for (int i = 0; i < INT64_LENGTH; i++)
    {
        tempInt64Byte[i] = unsignedTx[i + dataOffset];
    }
    dataOffset += INT64_LENGTH;
    tempInt64 = *(long *)tempInt64Byte;
    printf("\nFee:%ld\n", tempInt64);

    // skipping parse byte length
    dataOffset += INT32_LENGTH;

    // parsing amount
    for (int i = 0; i < INT64_LENGTH; i++)
    {
        tempInt64Byte[i] = unsignedTx[i + dataOffset];
    }
    dataOffset += INT64_LENGTH;
    tempInt64 = *(long *)tempInt64Byte;
    printf("\nAmount:%ld\n", tempInt64);

    return (0);
}