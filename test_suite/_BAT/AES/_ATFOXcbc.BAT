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
COPY /Y ..\HEDIT.EXE AES.EXE >nul
ECHO Crypto utility AES.EXE has been built.

REM Multilingual (Latin I) DOS Console standard codepage=850
REM MODE CON CP SELECT=850

REM Windows Western European codepage = 1252
MODE CON CP SELECT=1252 >nul

CLS
ECHO Testing AES.exe - Mode: CBC
ECHO ---------------------------------
ECHO ---AES128 FOX_RAW.TXT /ENCRYPT---
ECHO ---------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
AES _FOX_RAW.TXT _FOX_RAW.A~e _AT128.#K /encrypt _ATFOX.#I
ECHO [*.E#-] [*.A~e]
COMP _FOX_RAW.A~e _ATFOX.E#- /M /A

:DECIPHER1
ECHO ---AES128 FOX_RAW.TXT /DECIPHER---
AES _FOX_RAW.A~e _FOX_RAW.A~d _AT128.#K /decipher _ATFOX.#I
ECHO [*.RAW] [*.A~d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.A~d
echo ---

:STEALING2
PAUSE
CLS
ECHO ---------------------------------
ECHO ---AES128 FOX_PAD.TXT /ENCRYPT---
ECHO ---------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
AES _FOX_PAD.TXT _FOX_PAD.A~e _AT128.#K /encrypt _ATFOX.#I
ECHO [*.E#] [*.A~e]
COMP _FOX_PAD.A~e _ATFOX.E# /M /A

ECHO ---AES128 FOX_PAD.TXT /DECIPHER---
AES _FOX_PAD.A~e _FOX_PAD.A~d _AT128.#K /decipher _ATFOX.#I
ECHO [*.TXT] [*.A~d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.A~d
echo ---

:PADDING1
PAUSE
CLS
ECHO ------------------------------
ECHO ---AES128 FOX_RAW.TXT /CBCE---
ECHO ------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
AES _FOX_RAW.TXT _FOX_RAW.A°e _AT128.#K /cbce _ATFOX.#I
ECHO [*.E#+] [*.A°e]
COMP _FOX_RAW.A°e _ATFOX.E#+ /M /A

ECHO ---AES128 FOX_RAW.TXT /CBCD---
AES _FOX_RAW.A°e _FOX_RAW.A°d _AT128.#K /cbcd _ATFOX.#I
AES _FOX_RAW.A°e _FOX_RAW_A°e.A~d _AT128.#K /decipher _ATFOX.#I
ECHO [*.RAW] [*.A°d] [*_A°e.A~d]
type _FOX_RAW.TXT					     
echo ---
type _FOX_RAW.A°d
echo ---
type _FOX_RAW_A°e.A~d
echo ---

:PADDING2
PAUSE
CLS
ECHO ------------------------------
ECHO ---AES128 FOX_PAD.TXT /CBCE---
ECHO ------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
AES _FOX_PAD.TXT _FOX_PAD.A°e _AT128.#K /cbce _ATFOX.#I
ECHO [*.E#=] [*.A°e]
COMP _FOX_PAD.A°e _ATFOX.E#= /M /A

ECHO ---AES128 FOX_PAD.TXT /CBCD---
AES _FOX_PAD.A°e _FOX_PAD.A°d _AT128.#K /cbcd _ATFOX.#I
AES _FOX_PAD.A°e _FOX_PAD_A°e.A~d _AT128.#K /decipher _ATFOX.#I
ECHO [*.TXT] [*.A°d] [*_A°e.A~d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.A°d
echo ---
type _FOX_PAD_A°e.A~d
echo ---

PAUSE
CLS
ECHO ----------------------------
ECHO ---AES128 CBC ALL RESULTS---
ECHO ----------------------------
PAUSE
echo "AES128 /ENCRYPT"
COMP _FOX_RAW.A~e _ATFOX.E#- /M /A
COMP _FOX_PAD.A~e _ATFOX.E# /M /A

echo "AES128 /CBCE"
COMP _FOX_RAW.A°e _ATFOX.E#+ /M /A
COMP _FOX_PAD.A°e _ATFOX.E#= /M /A

echo "AES128 /DECIPHER"
type _FOX_RAW.A~d
echo.
type _FOX_PAD.A~d
echo.
echo "AES128 /CBCD"
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

