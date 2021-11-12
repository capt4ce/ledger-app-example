import ed25519

# data      = helloworld
# dataBytes = 68656c6c6f776f726c64
# hash512   = 1594244d52f2d8c12b142bb61f47bc2eaf503d6d9ca8480cae9fcf112f66e4967dc5e8fa98285e36db8af1b8ffa8b84cb15e0fbcf836c3deb803c13f37659a60

testData = [
  {
    'name': "manual signing with data 68656c6c6f776f726c64",
    'data': "68656c6c6f776f726c64",
    'publicKeyHex': "212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e",
    'signature': "38a95b951f1dc2c9c83f64d2ccbfa9f3c565520899559fd3b1fff53577f8b411cf475a07178cb5010b865fafeb2a684a66429ba1edee3de803696f7ee2160c06",
  },
  {
    'name': "ledger signing with data 68656c6c6f776f726c64",
    'publicKeyHex': "212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e",
    'data': "68656c6c6f776f726c64",
    'signature': "38a95b951f1dc2c9c83f64d2ccbfa9f3c565520899559fd3b1fff53577f8b411cf475a07178cb5010b865fafeb2a684a66429ba1edee3de803696f7ee2160c06",
  },
  {
    'name': "ledger signing with data 68656c6c6f776f726c64, internally hash it with 256",
    'publicKeyHex': "212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e",
    'data': "68656c6c6f776f726c64",
    'signature': "8c4b39759493f9d3f3e1967f83e5591583cfa226d6a5989fe0ac9431818b1e28e0277aefe51ccbc73ca87f742604aba8d8c888e5d70b8ddad292437a4844010d",
  },
  {
    'name': "ledger signing with data 68656c6c6f776f726c64, internally hash it with 512",
    'publicKeyHex': "212fa22112a1608eca34e50d864ae088e4a1b4057e81f675e215ec266737935e",
    'data': "68656c6c6f776f726c64",
    'signature': "14f441bac2ce5caa182eed8f498e87bbc90d9ad9c02328607980181394c58f2762891b499ac336cccb6d8c156719d4602a574fd72f4d2598af65e1521a2fa00b",
  },
]

for testD in testData:
  verifying_key = ed25519.VerifyingKey(bytes(testD['publicKeyHex'], encoding="utf8"), encoding="hex")
  try:
    verifying_key.verify(bytes(testD['signature'], encoding="utf8"), bytes.fromhex(testD['data']), encoding="hex")
    print("VALID    : " + testD['name'])
  except ed25519.BadSignatureError:
    print("INVALID  : " + testD['name'])