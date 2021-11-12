# Message Format

This documentation explains about the APDU message format of each operations that can be performed by Zoobc ledger application.
In the Ledger app, each command/response packet is called an APDU.
[Reference to the APDU standard format](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit)

**Command Format**
| **CLA** (1B) | **INS** (1B) | **P1** (1B) | **P2** (1B) | **LC** (4B) | **Input Data** (as defined in **LC**) (max 256 bytes) |
|--------------|--------------|-------------|-------------|-------------|---------------------------------|-------------|
| Instruction class | Instruction code | Instruction parameter 1 | Instruction parameter 2 | Length of data | Data to be used for the instruction |

<br/>

## Get Public Key

This command is to get Zoobc public key and address associated with the hardware wallet connected for the given BIP 32 path.
The public key is ed25519 compatible and is produced using Slip10 algorithm.

~The address can be optionally checked on the device before being returned.~

**Command**

| Data (hex)          | Description                                     |
| ------------------- | ----------------------------------------------- |
| 80                  | **CLA** Application instruction class           |
| 02                  | **INS** Instruction for getting public key      |
| 00                  | **P1** No Param1                                |
| 00                  | **P2** No Param2                                |
| 00000000 - FFFFFFFF | **LC** to indicate the length of the input data |

**Input Data** :

| Length (Bytes) | Description              |
| -------------- | ------------------------ |
| 4              | Derivation Account Index |

**Response Data**

| Length (Bytes) | Description |
| -------------- | ----------- |
| 32             | Public Key  |

<br/>

## Sign Transaction

This app can be used to sign (using ed25519 algorithm) several transaction types supported by Zoobc. The data sent to device should be compatible transaction bytes with ZooBC algorithm. The transaction bytes will contain the type of transaction that the bytes containts (based on Zoobc standard). Depending the type of transaction contained in the data, the piiece of data shown on the screen also will differ.

**Command**

| Data (hex)          | Description                                     |
| ------------------- | ----------------------------------------------- |
| 80                  | **CLA** Application instruction class           |
| 03                  | **INS** Instruction for signing transaction     |
| 00                  | **P1** No Param1                                |
| 00                  | **P2** No Param2                                |
| 00000000 - FFFFFFFF | **LC** to indicate the length of the input data |

**Input Data** :

| Length (Bytes) | Description              |
| -------------- | ------------------------ |
| 4              | Derivation Account Index |
| var            | Transaction Bytes        |

**Response Data**

| Length (Bytes) | Description |
| -------------- | ----------- |
| 32             | Public Key  |

<br/>

### Send Money transaction bytes structure :

| Length (Bytes) | Description                 |
| -------------- | --------------------------- |
| 2              | transaction type            |
| 2              | transaction subtype         |
| 1              | version                     |
| 8              | timestamp                   |
| 4              | length of sender address    |
| var            | sender address              |
| 4              | length of recipient address |
| var            | recipient address           |
| 8              | fee                         |
| 4              | length of parse byte        |
| 8              | amount to send              |
| 24             | --                          |
