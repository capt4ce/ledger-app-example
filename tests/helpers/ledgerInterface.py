from ledgerblue.comm import getDongle
import math

import helpers.constants as constants

# sending data to ledger
# will chunk the data if its length is more than max size allowed
def sendData(istructionHexString, dataHexString):
    dongle = getDongle(True)

    print("all data: ", dataHexString)

    # data length: length of derivation path index + length of unsigned transaction
    stringDataLength = len(dataHexString)
    bytesDataLength = int(stringDataLength/2)
    maxStringDataSize = constants.MAX_INPUT_DATA_SIZE * 2
    print("total input data length              : ", bytesDataLength, " bytes")
    print("max data length                      : ", constants.MAX_INPUT_DATA_SIZE, " bytes")
    print("will be divided into a few chunks    : ", math.ceil(bytesDataLength/constants.MAX_INPUT_DATA_SIZE))
    print("")

    p1 = "00"
    p2 = "00"

    stringOffset = 0
    result = bytes.fromhex("")
    while stringOffset < stringDataLength:
        chunk = ""

        if (stringDataLength - stringOffset) < maxStringDataSize:
            chunk = dataHexString[stringOffset:]
        else:
            endOffset = stringOffset + maxStringDataSize
            chunk = dataHexString[stringOffset:endOffset]

        if stringOffset != 0:
            p1 = constants.P1_MORE
        else:
            p1 = constants.P1_FIRST

        isLastData = (stringOffset + len(chunk)) == stringDataLength
        if isLastData:
            p2 = constants.P2_LAST
        else:
            p2 = constants.P2_MORE

        dataLengthHex = int(len(chunk)/2).to_bytes(4,'little').hex()
        apduBytes = '80' + istructionHexString + p1 + p2 + dataLengthHex + chunk
        result = dongle.exchange(bytes.fromhex(apduBytes))

        # print(chunk)
        # print("stringOffset: ", stringOffset, " maxStringDataSize: ", maxStringDataSize, " stringDataLength: ", stringDataLength)
        # print("")
        stringOffset += len(chunk)
    return result

def signTransaction(dataHexString): 
    result = sendData(constants.INSTRUCTION_SIGN_TRANSACTION, dataHexString)
    print("signature " + result.hex())