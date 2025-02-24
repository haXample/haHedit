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
ECHO Testing DES.exe - Mode: CBC
ECHO ------------------------------
ECHO ---DES FOX_RAW.TXT /ENCRYPT---
ECHO ------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
DES _FOX_RAW.TXT _FOX_RAW.D~e _DT64.#K /encrypt _DTFOX.#I
ECHO [*.E#-] [*.D~e]
COMP _FOX_RAW.D~e _DTFOX.E#- /M /A

:DECIPHER1
ECHO ---DES FOX_RAW.TXT /DECIPHER---
DES _FOX_RAW.D~e _FOX_RAW.D~d _DT64.#K /decipher _DTFOX.#I
ECHO [*.RAW] [*.D~d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.D~d
echo ---

:STEALING2
PAUSE
CLS
ECHO ------------------------------
ECHO ---DES FOX_PAD.TXT /ENCRYPT---
ECHO ------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
DES _FOX_PAD.TXT _FOX_PAD.D~e _DT64.#K /encrypt _DTFOX.#I
ECHO [*.E#] [*.D~e]
COMP _FOX_PAD.D~e _DTFOX.E# /M /A

ECHO ---DES FOX_PAD.TXT /DECIPHER---
DES _FOX_PAD.D~e _FOX_PAD.D~d _DT64.#K /decipher _DTFOX.#I
ECHO [*.TXT] [*.D~d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.D~d
echo ---

:PADDING1
PAUSE
CLS
ECHO ---------------------------
ECHO ---DES FOX_RAW.TXT /CBCE---
ECHO ---------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
DES _FOX_RAW.TXT _FOX_RAW.D°e _DT64.#K /cbce _DTFOX.#I
ECHO [*.E#+] [*.D°e]
COMP _FOX_RAW.D°e _DTFOX.E#+ /M /A

ECHO ---DES FOX_RAW.TXT /CBCD---
DES _FOX_RAW.D°e _FOX_RAW.D°d _DT64.#K /cbcd _DTFOX.#I
DES _FOX_RAW.D°e _FOX_RAW_D°e.D~d _DT64.#K /decipher _DTFOX.#I
ECHO [*.RAW] [*.D°d] [*_D°e.D~d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.D°d
echo ---
type _FOX_RAW_D°e.D~d
echo ---

:PADDING2
PAUSE
CLS
ECHO ----------------------------
ECHO ---DES FOX_PAD.TXT /CBCE---
ECHO ----------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
DES _FOX_PAD.TXT _FOX_PAD.D°e _DT64.#K /cbce _DTFOX.#I
ECHO [*.D°e] [*.E#=]
COMP _FOX_PAD.D°e _DTFOX.E#= /M /A

ECHO ---DES FOX_PAD.TXT /CBCD---
DES _FOX_PAD.D°e _FOX_PAD.D°d _DT64.#K /cbcd _DTFOX.#I
DES _FOX_PAD.D°e _FOX_PAD_D°e.D~d _DT64.#K /decipher _DTFOX.#I
ECHO [*.TXT] [*.D°d] [*_D°e.D~d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.D°d
echo ---
type _FOX_PAD_D°e.D~d
echo ---

PAUSE
CLS
ECHO -------------------------
ECHO ---DES CBC ALL RESULTS---
ECHO -------------------------
PAUSE
echo "DES /ENCRYPT"
COMP _FOX_RAW.D~e _DTFOX.E#- /M /A
COMP _FOX_PAD.D~e _DTFOX.E# /M /A

echo "DES /CBCE"
COMP _FOX_RAW.D°e _DTFOX.E#+ /M /A
COMP _FOX_PAD.D°e _DTFOX.E#= /M /A

echo "DES /DECIPHER"
type _FOX_RAW.D~d
echo.
type _FOX_PAD.D~d
echo.
echo "DES /CBCD"
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

