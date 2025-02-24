@ECHO OFF
REM Build a crypto utility for DES only.
COPY /Y ..\HEDIT.EXE DES.EXE

IF EXIST _Des???_64.D~m DEL _Des???_64.D~m

PAUSE
CLS
ECHO Testing des.exe - Mode: MAC
ECHO ------------------------
ECHO ---Des 56bit Key /MAC---
ECHO ------------------------
PAUSE
DES __DES_CMACMSG_064.BIN _Des064_64.D~m __DES_CMAC64.#K /MAC
ECHO [*.D~m] [*.M-]
COMP _Des064_64.D~m _DTMAC64.M- /M /A
DES __DES_CMACMSG_160.BIN _Des160_64.D~m __DES_CMAC64.#K /MAC
ECHO [*.D~m] [*.M]
COMP _Des160_64.D~m _DTMAC64.M /M /A
DES __DES_CMACMSG_256.BIN _Des256_64.D~m __DES_CMAC64.#K /MAC
ECHO [*.D~m] [*.M+]
COMP _Des256_64.D~m _DTMAC64.M+ /M /A

PAUSE
