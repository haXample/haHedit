@ECHO OFF
IF EXIST _FOX_RAW.3?e DEL _FOX_RAW.3?e
IF EXIST _FOX_RAW.3?d DEL _FOX_RAW.3?d
IF EXIST _FOX_RAW_3?e.3_d DEL _FOX_RAW_3?e.3_d
IF EXIST _FOX_RAW_3?e.3~d DEL _FOX_RAW_3?e.3~d
IF EXIST _FOX_PAD.3?e DEL _FOX_PAD.3?e
IF EXIST _FOX_PAD.3?d DEL _FOX_PAD.3?d
IF EXIST _FOX_PAD_3?e.3_d DEL _FOX_PAD_3?e.3_d
IF EXIST _FOX_PAD_3?e.3~d DEL _FOX_PAD_3?e.3~d

PAUSE
REM Build a crypto utility for TDES only.
COPY /Y ..\HEDIT.EXE TDES.EXE

REM Multilingual (Latin I) DOS Console standard codepage=850
REM MODE CON CP SELECT=850

REM Windows Western European codepage = 1252
MODE CON CP SELECT=1252 >nul

CLS
ECHO Testing TDES.exe - Mode: ECB
ECHO ----------------------------------
ECHO ---3DES FOX_RAW.TXT /ECBENCRYPT---
ECHO ----------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
TDES _FOX_RAW.TXT _FOX_RAW.3_e _TDTFOX.#K /ecbencrypt
ECHO [*.3_e] [*.E--]
COMP _FOX_RAW.3_e _TDTFOX.E-- /M /A

:DECIPHER1
ECHO ---3DES FOX_RAW.TXT /ECBDECIPHER---
TDES _FOX_RAW.3_e _FOX_RAW.3_d _TDTFOX.#K /ecbdecipher
ECHO [*.3_d] [*.RAW]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.3_d
echo ---

:STEALING2
PAUSE
CLS
ECHO ----------------------------------
ECHO ---3DES FOX_PAD.TXT /ECBENCRYPT---
ECHO ----------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
TDES _FOX_PAD.TXT _FOX_PAD.3_e _TDTFOX.#K /ecbencrypt
ECHO [*.3_e] [*.E-]
COMP _FOX_PAD.3_e _TDTFOX.E- /M /A

ECHO ---3DES FOX_PAD.TXT /ECBDECIPHER---
TDES _FOX_PAD.3_e _FOX_PAD.3_d _TDTFOX.#K /ecbdecipher
ECHO [*.3_d] [*.TXT]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.3_d
echo ---

:PADDING1
PAUSE
CLS
ECHO ----------------------------
ECHO ---3DES FOX_RAW.TXT /ECBE---
ECHO ----------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
TDES _FOX_RAW.TXT _FOX_RAW.3°e _TDTFOX.#K /ecbe
ECHO [*.3°e] [*.E-+]
COMP _FOX_RAW.3°e _TDTFOX.E-+ /M /A

ECHO ---3DES FOX_RAW.TXT /ECBD---
TDES _FOX_RAW.3°e _FOX_RAW.3°d _TDTFOX.#K /ecbd
TDES _FOX_RAW.3°e _FOX_RAW_3°e.3_d _TDTFOX.#K /ecbdecipher
ECHO [*.3°d] [*.RAW]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.3°d
echo ---
type _FOX_RAW_3°e.3_d
echo ---

:PADDING2
PAUSE
CLS
ECHO ----------------------------
ECHO ---3DES FOX_PAD.TXT /ECBE---
ECHO ----------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
TDES _FOX_PAD.TXT _FOX_PAD.3°e _TDTFOX.#K /ecbe
ECHO [*.3°e] [*.E-=]
COMP _FOX_PAD.3°e _TDTFOX.E-= /M /A

ECHO ---3DES FOX_PAD.TXT /ECBD---
TDES _FOX_PAD.3°e _FOX_PAD.3°d _TDTFOX.#K /ecbd
TDES _FOX_PAD.3°e _FOX_PAD_3°e.3_d _TDTFOX.#K /ecbdecipher
ECHO [*.3°d] [*.TXT]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.3°d
echo ---
type _FOX_PAD_3°e.3_d
echo ---

PAUSE
CLS
ECHO --------------------------
ECHO ---3DES ECB ALL RESULTS---
ECHO --------------------------
PAUSE
echo "3DES /ECBENCRYPT"
COMP _FOX_RAW.3_e _TDTFOX.E-- /M /A
COMP _FOX_PAD.3_e _TDTFOX.E- /M /A

echo "3DES /ECBE"
COMP _FOX_RAW.3°e _TDTFOX.E-+ /M /A
COMP _FOX_PAD.3°e _TDTFOX.E-= /M /A

echo "3DES /ECBDECIPHER"
type _FOX_RAW.3_d
echo.
type _FOX_PAD.3_d
echo.
echo "3DES /ECBD"
type _FOX_RAW.3°d
echo.
type _FOX_PAD.3°d
echo.

:END
REM Multilingual (Latin I) DOS Console standard codepage=850
MODE CON CP SELECT=850 >nul
ECHO > nul 2>&1
ECHO Batch terminated.
GOTO :eof



