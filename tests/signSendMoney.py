#!/usr/bin/env python
#*******************************************************************************
#*   Ledger Blue
#*   (c) 2016 Ledger
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************
from helpers.ledgerInterface import signTransaction

unsignedTx            = "01000000010bb0c05f0000000000000000212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e0000000024d9ba99dc21226613c7dd2261ef55cf474b0b040eca0fe9a3ae37fdb183768a00a3e111000000000800000000e1f505000000000200000000000000" # hash = 3f7268c8f7d3440a1fb28dcdbea9ae3890ede127b885d7bdf5ddb7af765f9240, signature = eb37b9dfb9e6de968d79458598c62b403d58f2c029de4bc4344c547747903dd099cb1b42238a3f684f34ebdc189797877f6a231e753d00fd990351cf30e57a0f
derivationPathIndex = '00000000'
inputData = derivationPathIndex + unsignedTx
signTransaction(inputData)

#** 
# Send Money data format
# ========================
# Example: 8003000075000000000000000100000001c07cbb5f0000000000000000212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e00000000212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e00a3e111000000000800000000e1f505000000000200000000000000
# ========================
# 80                : CLA (1)
# 03                : INS (sign) (1) [sign transaction instruction type]
# 00                : P1 (1)
# 00                : P1 (1)
# 75000000          : Data Length (4)
# ========================
# 00000000          : derivation path index of the account used (4)
# 01000000          : transaction type (4)
# 01                : version (1)
# c07cbb5f00000000  : timestamp (8)
# 00000000          : account type of sender address (4)
# 212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e : sender public key
# 00000000          : length of recipient address (4)
# 212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e : recipient public key
# 00a3e11100000000  : fee
# 08000000          : length of parse byte
# 00e1f50500000000  : amount
# 0200000000000000 : other options
# *#