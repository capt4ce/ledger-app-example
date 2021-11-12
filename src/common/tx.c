#include "tx.h"

#include "crypto.h"

struct txData currentTx;

// reset struct used to store temporarily the currently processed transaction
void reset_tx_data(void)
{
    // reset tx bytes
    os_memset(currentTx.txBytes, 0, sizeof(currentTx.txBytes));
    currentTx.txLength = 0;

    // reset hash context
    cx_sha3_init(&hashContext, 256);

    // reset tx hash
    os_memset(currentTx.txHash, 0, sizeof(currentTx.txHash));
}

void append_tx_data(uint8_t *data, int length)
{
    os_memmove(currentTx.txBytes + currentTx.txLength, data, length);
    currentTx.txLength += length;

    hash3_256(&hashContext, NULL, data, length);
}

void set_derivation_path(int derivationPath)
{
    currentTx.derivationPath = derivationPath;
}

void finalize_transaction()
{
    hash3_256(&hashContext, currentTx.txHash, NULL, 0);
}

int get_tx_signature(uint8_t *dest)
{
    return deriveAndSign(dest, currentTx.derivationPath, currentTx.txHash, MAX_TX_HASH256_LENGTH);
}