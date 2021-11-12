#include <stdint.h>
#include <os.h>

#define MAX_TX_BYTES_LENGTH 1024
#define MAX_TX_HASH256_LENGTH 32 // 256 bits

struct txData
{
    int derivationPath;
    int txLength;
    uint8_t txBytes[MAX_TX_BYTES_LENGTH];
    uint8_t txHash[MAX_TX_HASH256_LENGTH]; // used in the next iteration
};

void reset_tx_data(void);
void append_tx_data(uint8_t *data, int length);
void set_derivation_path(int derivationPath);
void finalize_transaction();
int get_tx_signature(uint8_t *dest);

extern struct txData currentTx;