@ECHO OFF
IF EXIST _FOX_RAW.D?e DEL _FOX_RAW.D?e
IF EXIST _FOX_RAW.D?d DEL _FOX_RAW.D?d
IF EXIST _FOX_RAW_D?e.D_d DEL _FOX_RAW_D?e.D_d
IF EXIST _FOX_RAW_D?e.D~d DEL _FOX_RAW_D?e.D~d
IF EXIST _FOX_PAD.D?e DEL _FOX_PAD.D?e
IF EXIST _FOX_PAD.D?d DEL _FOX_PAD.D?d
IF EXIST _FOX_PAD_D?e.D_d DEL _FOX_PAD_D?e.D_d
IF EXIST _FOX_PAD_D?e.D~d DEL _FOX_PAD_D?e.D~d

PAUSE
REM Build a crypto utility for DES only.
COPY /Y ..\HEDIT.EXE DES.EXE

REM Multilingual (Latin I) DOS Console standard codepage=850
REM MODE CON CP SELECT=850

REM Windows Western European codepage = 1252
MODE CON CP SELECT=1252 >nul

CLS
ECHO Testing des%1%2.exe - Mode: ECB
ECHO ---------------------------------
ECHO ---DES FOX_RAW.TXT /ECBENCRYPT---
ECHO ---------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
des%1%2 _FOX_RAW.TXT _FOX_RAW.D_e _DT64.#K /ecbencrypt
ECHO [*.D_e] [*.E--]
COMP _FOX_RAW.D_e _DTFOX.E-- /M /A

:DECIPHER1
ECHO ---DES FOX_RAW.TXT /ECBDECIPHER---
des%1%2 _FOX_RAW.D_e _FOX_RAW.D_d _DT64.#K /ecbdecipher
ECHO [*.D_d] [*.RAW]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.D_d
echo ---

:STEALING2
PAUSE
CLS
ECHO ---------------------------------
ECHO ---DES FOX_PAD.TXT /ECBENCRYPT---
ECHO ---------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
des%1%2 _FOX_PAD.TXT _FOX_PAD.D_e _DT64.#K /ecbencrypt
ECHO [*.D_e] [*.E-]
COMP _FOX_PAD.D_e _DTFOX.E- /M /A

ECHO ---DES FOX_PAD.TXT /ECBDECIPHER---
des%1%2 _FOX_PAD.D_e _FOX_PAD.D_d _DT64.#K /ecbdecipher
ECHO [*.D_d] [*.TXT]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.D_d
echo ---

:PADDING1
PAUSE
CLS
ECHO ---------------------------
ECHO ---DES FOX_RAW.TXT /ECBE---
ECHO ---------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
des%1%2 _FOX_RAW.TXT _FOX_RAW.D°e _DT64.#K /ecbe
ECHO [*.D°e] [*.E-+]
COMP _FOX_RAW.D°e _DTFOX.E-+ /M /A

ECHO ---DES FOX_RAW.TXT /ECBD---
des%1%2 _FOX_RAW.D°e _FOX_RAW.D°d _DT64.#K /ecbd
des%1%2 _FOX_RAW.D°e _FOX_RAW_D°e.D_d _DT64.#K /ecbdecipher
ECHO [*.TXT] [*.D°d] [*_D°e.D_d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.D°d
echo ---
type _FOX_RAW_D°e.D_d
echo ---

:PADDING2
PAUSE
CLS
ECHO ---------------------------
ECHO ---DES FOX_PAD.TXT /ECBE---
ECHO ---------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
des%1%2 _FOX_PAD.TXT _FOX_PAD.D°e _DT64.#K /ecbe
ECHO [*.D°e] [*.E-=]
COMP _FOX_PAD.D°e _DTFOX.E-= /M /A

ECHO ---DES FOX_PAD.TXT /ECBD---
des%1%2 _FOX_PAD.D°e _FOX_PAD.D°d _DT64.#K /ecbd
des%1%2 _FOX_PAD.D°e _FOX_PAD_D°e.D_d _DT64.#K /ecbdecipher
ECHO [*.TXT] [*.D°d] [*_D°e.D_d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.D°d
echo ---
type _FOX_PAD_D°e.D_d
echo ---

PAUSE
CLS
ECHO -------------------------
ECHO ---DES ECB ALL RESULTS---
ECHO -------------------------
PAUSE
echo "DES /ECBENCRYPT"
COMP _FOX_RAW.D_e _DTFOX.E-- /M /A
COMP _FOX_PAD.D_e _DTFOX.E- /M /A

echo "DES /ECBE"
COMP _FOX_RAW.D°e _DTFOX.E-+ /M /A
COMP _FOX_PAD.D°e _DTFOX.E-= /M /A

echo "DES /ECBDECIPHER"
type _FOX_RAW.D_d
echo.
type _FOX_PAD.D_d
echo.
echo "DES /ECBD"
type _FOX_RAW.D°d
echo.
type _FOX_PAD.D°d
echo.

:END
REM Multilingual (Latin I) DOS Console standard codepage=850
MODE CON CP SELECT=850 >nul
ECHO > nul 2>&1
ECHO Batch terminated.
GOTO :eof

