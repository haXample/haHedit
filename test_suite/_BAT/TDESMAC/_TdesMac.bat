@ECHO OFF
REM Build a crypto utility for TDES only.
COPY /Y ..\HEDIT.EXE TDES.EXE

IF EXIST _Tdes???_128.3~m DEL _Tdes???_128.3~m
IF EXIST _Tdes???_192.3~m DEL _Tdes???_192.3~m

PAUSE
CLS
ECHO Testing tdes.exe - Mode: MAC
ECHO --------------------------
ECHO ---Tdes 112bit Key /MAC---
ECHO --------------------------
PAUSE
Tdes __3DES_CMACMSG_064.BIN _Tdes064_128.3~m __3DES_CMAC128.#K /MAC
ECHO [*.3~m] [*.M-]
COMP _Tdes064_128.3~m _TDTMAC128.M- /M /A
Tdes __3DES_CMACMSG_160.BIN _Tdes160_128.3~m __3DES_CMAC128.#K /MAC
ECHO [*.3~m] [*.M]
COMP _Tdes160_128.3~m _TDTMAC128.M /M /A
TDES __3DES_CMACMSG_256.BIN _Tdes256_128.3~m __3DES_CMAC128.#K /MAC
ECHO [*.3~m] [*.M+]
COMP _Tdes256_128.3~m _TDTMAC128.M+ /M /A

PAUSE
CLS
ECHO --------------------------
ECHO ---Tdes 168bit Key /MAC---
ECHO --------------------------
PAUSE
Tdes __3DES_CMACMSG_064.BIN _Tdes064_192.3~m __3DES_CMAC192.#K /MAC
ECHO [*.3~m] [*.M-]
COMP _Tdes064_192.3~m _TDTMAC192.M- /M /A
Tdes __3DES_CMACMSG_160.BIN _Tdes160_192.3~m __3DES_CMAC192.#K /MAC
ECHO [*.3~m] [*.M]
COMP _Tdes160_192.3~m _TDTMAC192.M /M /A
TDES __3DES_CMACMSG_256.BIN _Tdes256_192.3~m __3DES_CMAC192.#K /MAC
ECHO [*.3~m] [*.M+]
COMP _Tdes256_192.3~m _TDTMAC192.M+ /M /A

PAUSE
