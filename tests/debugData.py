from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException

dongle = getDongle(True)
derivationPathIndex = "00000000"
publicKey = dongle.exchange(bytes.fromhex("8001000004000000"+derivationPathIndex))
print("debug data " + publicKey.hex())


# Zoobc derivation path = m/44'/148'/0'
#** 
# Get debug data data format
# ========================
# 80                : CLA
# 01                : INS (sign)
# 00                : P1
# 00                : P1
# 04000000          : Data Length
# ========================
# 00000000          : derivation path index of the account used
# *#