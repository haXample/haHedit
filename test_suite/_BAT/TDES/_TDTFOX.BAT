@ECHO OFF

IF EXIST _FOX_PAD.3_e DEL _FOX_PAD.3_e
IF EXIST _FOX_PAD.3_d DEL _FOX_PAD.3_d
IF EXIST _FOX_PAD.3~e DEL _FOX_PAD.3~e
IF EXIST _FOX_PAD.3~d DEL _FOX_PAD.3~d

PAUSE
REM Build a crypto utility for DES only.
COPY /Y ..\HEDIT.EXE DES.EXE

CLS
ECHO Emulating tDES.exe with des.exe - Modes: ECB/CBC
ECHO -------------------------------------------------
ECHO ---DES: 3DES-EMULATION FOX_PAD.TXT /ECBENCRYPT---
ECHO -------------------------------------------------
IF EXIST *.en1 DEL *.en1
IF EXIST *.en2 DEL *.en2
type _FOX_PAD.TXT
echo.
pause
echo ---
DES _FOX_PAD.TXT _TDTFOX.en1 _TDTFOX.#k1 /ecbencrypt
DES _TDTFOX.en1 _TDTFOX.en2 _TDTFOX.#k2  /ecbdecipher
DES _TDTFOX.en2 _FOX_PAD.3_e _TDTFOX.#k3  /ecbencrypt
DEL *.en1
DEL *.en2
type _FOX_PAD.3_e
echo ---
type _TDTFOX.E- 
echo ---

ECHO --------------------------------------------------
ECHO ---DES: 3DES-EMULATION FOX_PAD.TXT /ECBDECIPHER---
ECHO --------------------------------------------------
IF EXIST *.de2 DEL *.de2
IF EXIST *.de3 DEL *.de3
DES _FOX_PAD.3_e _TDTFOX.de3 _TDTFOX.#k3 /ecbdecipher
DES _TDTFOX.de3 _TDTFOX.de2 _TDTFOX.#k2 /ecbencrypt
DES _TDTFOX.de2 _FOX_PAD.3_d _TDTFOX.#k1 /ecbdecipher
IF EXIST *.de2 DEL *.de2
IF EXIST *.de3 DEL *.de3
type _FOX_PAD.3_d
echo ---
type _FOX_PAD.TXT
echo ---

PAUSE
CLS
ECHO ----------------------------------------------
ECHO ---DES: 3DES-EMULATION FOX_PAD.TXT /ENCRYPT---
ECHO ----------------------------------------------
IF EXIST *.e?a DEL *.e?a
IF EXIST *.e?b DEL *.e?b
IF EXIST *.e?c DEL *.e?c
type _FOX_PAD.TXT
echo.
pause
echo ---
DES _TDTFOX1.txt _TDTFOX.e1a _TDTFOX.#K1 /encrypt _TDTFOX.#I
DES _TDTFOX.e1a _TDTFOX.e1b _TDTFOX.#K2  /decipher
DES _TDTFOX.e1b _TDTFOX.e1c _TDTFOX.#K3  /encrypt 

DES _TDTFOX2.txt _TDTFOX.e2a _TDTFOX.#K1 /encrypt _TDTFOX.e1c
DES _TDTFOX.e2a _TDTFOX.e2b _TDTFOX.#K2  /decipher
DES _TDTFOX.e2b _TDTFOX.e2c _TDTFOX.#K3  /encrypt 

DES _TDTFOX3.txt _TDTFOX.e3a _TDTFOX.#K1 /encrypt _TDTFOX.e2c
DES _TDTFOX.e3a _TDTFOX.e3b _TDTFOX.#K2  /decipher
DES _TDTFOX.e3b _TDTFOX.e3c _TDTFOX.#K3  /encrypt 

DES _TDTFOX4.txt _TDTFOX.e4a _TDTFOX.#K1 /encrypt _TDTFOX.e3c
DES _TDTFOX.e4a _TDTFOX.e4b _TDTFOX.#K2  /decipher
DES _TDTFOX.e4b _TDTFOX.e4c _TDTFOX.#K3  /encrypt 

DES _TDTFOX5.txt _TDTFOX.e5a _TDTFOX.#K1 /encrypt _TDTFOX.e4c
DES _TDTFOX.e5a _TDTFOX.e5b _TDTFOX.#K2  /decipher
DES _TDTFOX.e5b _TDTFOX.e5c _TDTFOX.#K3  /encrypt 

DES _TDTFOX6.txt _TDTFOX.e6a _TDTFOX.#K1 /encrypt _TDTFOX.e5c
DES _TDTFOX.e6a _TDTFOX.e6b _TDTFOX.#K2  /decipher
DES _TDTFOX.e6b _TDTFOX.e6c _TDTFOX.#K3  /encrypt 
echo ---
COPY /Y /B _TDTFOX.e1c + _TDTFOX.e2c + _TDTFOX.e3c + _TDTFOX.e4c + _TDTFOX.e5c + _TDTFOX.e6c _FOX_PAD.3~e
DEL *.e?a
DEL *.e?b
DEL *.e?c
echo ---
type _FOX_PAD.3~e
echo ---
type _TDTFOX.E#
echo ---

ECHO -----------------------------------------------
ECHO ---DES: 3DES-EMULATION FOX_PAD.TXT /DECIPHER---
ECHO -----------------------------------------------
pause
IF EXIST *.d?a DEL *.d?a
IF EXIST *.d?b DEL *.d?b
IF EXIST *.d?c DEL *.d?c

copy /Y _TDTFOX.#I _TDTFOX.ivx
DES _TDTFOX1.E _TDTFOX.d1a _TDTFOX.#K3  /ecbdecipher
DES _TDTFOX.d1a _TDTFOX.d1b _TDTFOX.#K2 /ecbencrypt
DES _TDTFOX.d1b _TDTFOX.d1c _TDTFOX.#K1 /ecbdecipher
DES _TDTFOX.d1c _TDTFOX.ivx /XOR
copy /y _TDTFOX1.E _TDTFOX.ivx

DES _TDTFOX2.E _TDTFOX.d2a _TDTFOX.#K3  /ecbdecipher
DES _TDTFOX.d2a _TDTFOX.d2b _TDTFOX.#K2 /ecbencrypt
DES _TDTFOX.d2b _TDTFOX.d2c _TDTFOX.#K1 /ecbdecipher 
DES _TDTFOX.d2c _TDTFOX.ivx /XOR
copy /Y _TDTFOX2.E _TDTFOX.ivx

DES _TDTFOX3.E _TDTFOX.d3a _TDTFOX.#K3  /ecbdecipher
DES _TDTFOX.d3a _TDTFOX.d3b _TDTFOX.#K2 /ecbencrypt
DES _TDTFOX.d3b _TDTFOX.d3c _TDTFOX.#K1 /ecbdecipher 
DES _TDTFOX.d3c _TDTFOX.ivx /XOR
copy /Y _TDTFOX3.E _TDTFOX.ivx

DES _TDTFOX4.E _TDTFOX.d4a _TDTFOX.#K3  /ecbdecipher
DES _TDTFOX.d4a _TDTFOX.d4b _TDTFOX.#K2 /ecbencrypt
DES _TDTFOX.d4b _TDTFOX.d4c _TDTFOX.#K1 /ecbdecipher 
DES _TDTFOX.d4c _TDTFOX.ivx /XOR
copy /Y _TDTFOX4.E _TDTFOX.ivx

DES _TDTFOX5.E _TDTFOX.d5a _TDTFOX.#K3  /ecbdecipher
DES _TDTFOX.d5a _TDTFOX.d5b _TDTFOX.#K2 /ecbencrypt
DES _TDTFOX.d5b _TDTFOX.d5c _TDTFOX.#K1 /ecbdecipher 
DES _TDTFOX.d5c _TDTFOX.ivx /XOR
copy /Y _TDTFOX5.E _TDTFOX.ivx

DES _TDTFOX6.E _TDTFOX.d6a _TDTFOX.#K3  /ecbdecipher
DES _TDTFOX.d6a _TDTFOX.d6b _TDTFOX.#K2 /ecbencrypt
DES _TDTFOX.d6b _TDTFOX.d6c _TDTFOX.#K1 /ecbdecipher 
DES _TDTFOX.d6c _TDTFOX.ivx /XOR

echo ---
COPY /Y /B _TDTFOX.d1c + _TDTFOX.d2c + _TDTFOX.d3c + _TDTFOX.d4c + _TDTFOX.d5c + _TDTFOX.d6c _FOX_PAD.3~d
DEL *.d?a
DEL *.d?b
DEL *.d?c
DEL *.ivx
echo ---
type _FOX_PAD.3~d
echo ---
type _FOX_PAD.TXT
echo ---

ECHO ---------------------------------------------------------
ECHO ---Triple DES EMULATION using DES.EXE: ALL RESULTS---
ECHO ---------------------------------------------------------
PAUSE
echo "DES: 3DES-EMULATION /ECBENCRYPT"
COMP _FOX_PAD.3_e _TDTFOX.E- /M /A

echo "DES: 3DES-EMULATION /ENCRYPT"
COMP _FOX_PAD.3~e _TDTFOX.E# /M /A

echo "DES: 3DES-EMULATION /ECBDECIPHER"
type _FOX_PAD.3_d
echo.
echo "DES: 3DES-EMULATION /DECIPHER"
type _FOX_PAD.3~d
echo.
:END