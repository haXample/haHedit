@ECHO OFF
REM Build a crypto utility for AES only.
COPY /Y ..\HEDIT.EXE AES.EXE >nul
ECHO Crypto utility AES.EXE has been built.

IF EXIST _Aes???_128.A~m DEL _Aes???_128.A~m
IF EXIST _Aes???_192.A~m DEL _Aes???_192.A~m
IF EXIST _Aes???_256.A~m DEL _Aes???_256.A~m

PAUSE
CLS
ECHO Testing aes.exe - Mode: MAC
ECHO -------------------------
ECHO ---AES 128bit Key /MAC---
ECHO -------------------------
PAUSE
AES __AES_CMACMSG_128.BIN _Aes128_128.A~m __AES_CMAC128.#K /MAC
ECHO [*.A~m] [*.M-]
COMP _Aes128_128.A~m _ATMAC128.M- /M /A
AES __AES_CMACMSG_320.BIN _Aes320_128.A~m __AES_CMAC128.#K /MAC
ECHO [*.A~m] [*.M]
COMP _Aes320_128.A~m _ATMAC128.M /M /A
AES __AES_CMACMSG_512.BIN _Aes512_128.A~m __AES_CMAC128.#K /MAC
ECHO [*.A~m] [*.M+]
COMP _Aes512_128.A~m _ATMAC128.M+ /M /A

PAUSE
CLS
ECHO -------------------------
ECHO ---AES 192bit Key /MAC---
ECHO -------------------------
PAUSE
AES __AES_CMACMSG_128.BIN _Aes128_192.A~m __AES_CMAC192.#K /MAC
ECHO [*.A~m] [*.M-]
COMP _Aes128_192.A~m _ATMAC192.M- /M /A
AES __AES_CMACMSG_320.BIN _Aes320_192.A~m __AES_CMAC192.#K /MAC
ECHO [*.A~m] [*.M]
COMP _Aes320_192.A~m _ATMAC192.M /M /A
AES __AES_CMACMSG_512.BIN _Aes512_192.A~m __AES_CMAC192.#K /MAC
ECHO [*.A~m] [*.M+]
COMP _Aes512_192.A~m _ATMAC192.M+ /M /A

PAUSE
CLS
ECHO -------------------------
ECHO ---AES 256bit Key /MAC---
ECHO -------------------------
PAUSE
AES __AES_CMACMSG_128.BIN _Aes128_256.A~m __AES_CMAC256.#K /MAC
ECHO [*.A~m] [*.M-]
COMP _Aes128_256.A~m _ATMAC256.M- /M /A
AES __AES_CMACMSG_320.BIN _Aes320_256.A~m __AES_CMAC256.#K /MAC
ECHO [*.A~m] [*.M]
COMP _Aes320_256.A~m _ATMAC256.M /M /A
AES __AES_CMACMSG_512.BIN _Aes512_256.A~m __AES_CMAC256.#K /MAC
ECHO [*.A~m] [*.M+]
COMP _Aes512_256.A~m _ATMAC256.M+ /M /A

PAUSE
