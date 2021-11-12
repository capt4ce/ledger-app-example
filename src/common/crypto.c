#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <os.h>
#include <os_io_seproxyhal.h>

cx_sha3_t hashContext;

void extractPubkeyBytes(unsigned char *dst, cx_ecfp_public_key_t *publicKey)
{
    for (int i = 0; i < 32; i++)
    {
        dst[i] = publicKey->W[64 - i];
    }
    if (publicKey->W[32] & 1)
    {
        dst[31] |= 0x80;
    }
}

void deriveKeypair(uint32_t index, cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey)
{
    uint8_t keySeed[32];
    cx_ecfp_private_key_t pk;

    // bip32 path for zoobc: 44'/883'/n'/0'/0'
    uint32_t bip32Path[] = {44 | 0x80000000, 883 | 0x80000000, index | 0x80000000};
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32Path, 3, keySeed, NULL, (unsigned char *)"ed25519 seed", 12);

    cx_ecfp_init_private_key(CX_CURVE_Ed25519, keySeed, sizeof(keySeed), &pk);
    if (publicKey)
    {
        cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, publicKey);
        cx_ecfp_generate_pair(CX_CURVE_Ed25519, publicKey, &pk, 1);
    }
    if (privateKey)
    {
        *privateKey = pk;
    }
    os_memset(keySeed, 0, sizeof(keySeed));
    os_memset(&pk, 0, sizeof(pk));
}

uint32_t deriveAndSign(uint8_t *dst, uint32_t index, const uint8_t *data, unsigned int dataLength)
{
    cx_ecfp_public_key_t publicKey;
    cx_ecfp_private_key_t privateKey;
    uint32_t signatureLength = 0;

    deriveKeypair(index, &privateKey, &publicKey);
    signatureLength = cx_eddsa_sign(&privateKey, CX_LAST, CX_SHA512, data, dataLength, NULL, 0, dst, NULL);
    os_memset(&privateKey, 0, sizeof(privateKey));
    return signatureLength;
}

void hash3_256(cx_sha3_t *hashContext, uint8_t *dst, const uint8_t *data, unsigned int dataLength)
{
    if (dst == NULL)
    {
        cx_hash((cx_hash_t *)hashContext, 0, data, dataLength, NULL);
    }
    else
    {
        cx_hash((cx_hash_t *)hashContext, CX_LAST, data, dataLength, dst);
    }
}