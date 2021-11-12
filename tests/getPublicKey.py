from helpers.ledgerInterface import sendData
import helpers.constants as constants

derivationPathIndex = "00000000"
sendData(constants.INSTRUCTION_GET_PUBLIC_KEY, derivationPathIndex)

# Zoobc derivation path = m/44'/148'/0'
#** 
# Get Public key data format
# ========================
# 80                : CLA
# 02                : INS (sign)
# 00                : P1
# 00                : P1
# 04000000          : Data Length
# ========================
# 00000000          : derivation path index of the account used
# *#