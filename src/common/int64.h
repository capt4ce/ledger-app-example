#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

uint32_t bits64(int64_t *number);
void shiftr64(int64_t *number, uint32_t value, int64_t *target);
void shiftl64(int64_t *number, uint32_t value, int64_t *target);
void divmod64(int64_t *l, int64_t *r, int64_t *retDiv,
              int64_t *retMod);
static void reverseString(char *str, uint32_t length);
bool tostring64(int64_t *number, uint32_t baseParam, char *out,
                uint32_t outLength);