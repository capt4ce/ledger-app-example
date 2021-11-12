#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <os.h>
#include <os_io_seproxyhal.h>

void extractPubkeyBytes(unsigned char *dst, cx_ecfp_public_key_t *publicKey);
void deriveKeypair(uint32_t index, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey);
uint32_t deriveAndSign(uint8_t *dst, uint32_t index, const uint8_t *data, unsigned int dataLength);
void hash3_256(cx_sha3_t *hashContext, uint8_t *dst, const uint8_t *data, unsigned int dataLength);

extern cx_sha3_t hashContext;