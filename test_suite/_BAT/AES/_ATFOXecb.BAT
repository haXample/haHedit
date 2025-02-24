@ECHO OFF
IF EXIST _FOX_RAW.A?e DEL _FOX_RAW.A?e
IF EXIST _FOX_RAW.A?d DEL _FOX_RAW.A?d
IF EXIST _FOX_RAW_A?e.A_d DEL _FOX_RAW_A?e.A_d
IF EXIST _FOX_RAW_A?e.A~d DEL _FOX_RAW_A?e.A~d
IF EXIST _FOX_PAD.A?e DEL _FOX_PAD.A?e
IF EXIST _FOX_PAD.A?d DEL _FOX_PAD.A?d
IF EXIST _FOX_PAD_A?e.A_d DEL _FOX_PAD_A?e.A_d
IF EXIST _FOX_PAD_A?e.A~d DEL _FOX_PAD_A?e.A~d

PAUSE
REM Build a crypto utility for AES only.
COPY /Y ..\HEDIT.EXE AES.EXE

REM Multilingual (Latin I) DOS Console standard codepage=850
REM MODE CON CP SELECT=850

REM Windows Western European codepage = 1252
MODE CON CP SELECT=1252 >nul

CLS
ECHO Testing AES.exe - Mode: ECB
ECHO ------------------------------------
ECHO ---AES128 FOX_RAW.TXT /ECBENCRYPT---
ECHO ------------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
AES _FOX_RAW.TXT _FOX_RAW.A_e _AT128.#K /ecbencrypt
ECHO [*.E--] [*.A_e]
COMP _FOX_RAW.A_e _ATFOX.E-- /M /A

:DECIPHER1
ECHO ---AES128 FOX_RAW.TXT /ECBDECIPHER---
AES _FOX_RAW.A_e _FOX_RAW.A_d _AT128.#K /ecbdecipher
ECHO [*.RAW] [*.A_d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.A_d
echo ---

:STEALING2
PAUSE
CLS
ECHO -------------------------------------
ECHO ---AES128 FOX_PAD.TXT /ECBENCRYPT---
ECHO -------------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
AES _FOX_PAD.TXT _FOX_PAD.A_e _AT128.#K /ecbencrypt
ECHO [*.E-] [*.A_e]
COMP _FOX_PAD.A_e _ATFOX.E- /M /A

ECHO ---AES128 FOX_PAD.TXT /ECBDECIPHER---
AES _FOX_PAD.A_e _FOX_PAD.A_d _AT128.#K /ecbdecipher
ECHO [*.TXT] [*.d-x]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.A_d
echo ---

:PADDING1
PAUSE
CLS
ECHO ------------------------------
ECHO ---AES128 FOX_RAW.TXT /ECBE---
ECHO ------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
AES _FOX_RAW.TXT _FOX_RAW.A°e _AT128.#K /ecbe
ECHO [*.E-+] [*.A°e]
COMP _FOX_RAW.A°e _ATFOX.E-+ /M /A

ECHO ---AES128 FOX_RAW.TXT /ECBD---
AES _FOX_RAW.A°e _FOX_RAW.A°d _AT128.#K /ecbd
AES _FOX_RAW.A°e _FOX_RAW_A°e.A_d _AT128.#K /ecbdecipher
ECHO [*.RAW] [*.A°d] [*_A°e.A_d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.A°d
echo ---
type _FOX_RAW_A°e.A_d
echo ---

:PADDING2
PAUSE
CLS
ECHO ------------------------------
ECHO ---AES128 FOX_PAD.TXT /ECBE---
ECHO ------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
AES _FOX_PAD.TXT _FOX_PAD.A°e _AT128.#K /ecbe
ECHO [*.E-=] [*.A°e]
COMP _FOX_PAD.A°e _ATFOX.E-= /M /A

ECHO ---AES128 FOX_PAD.TXT /ECBD---
AES _FOX_PAD.A°e _FOX_PAD.A°d _AT128.#K /ecbd
AES _FOX_PAD.A°e _FOX_PAD_A°e.A_d _AT128.#K /ecbdecipher
ECHO [*.TXT] [*.A°d] [*_A°e.A_d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.A°d
echo ---
type _FOX_PAD_A°e.A_d
echo ---

PAUSE
CLS
ECHO ----------------------------
ECHO ---AES128 ECB ALL RESULTS---
ECHO ----------------------------
PAUSE
echo "AES128 /ECBENCRYPT"
COMP _FOX_RAW.A_e _ATFOX.E-- /M /A
COMP _FOX_PAD.A_e _ATFOX.E- /M /A

echo "AES128 /ECBE"
COMP _FOX_RAW.A°e _ATFOX.E-+ /M /A
COMP _FOX_PAD.A°e _ATFOX.E-= /M /A

echo "AES128 /ECBDECIPHER"
type _FOX_RAW.A_d
echo.
type _FOX_PAD.A_d
echo.
echo "AES128 /ECBD"
type _FOX_RAW.A°d
echo.
type _FOX_PAD.A°d
echo.

:END
REM Multilingual (Latin I) DOS Console standard codepage=850
MODE CON CP SELECT=850 >nul
ECHO > nul 2>&1
ECHO Batch terminated.
GOTO :eof


