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
ECHO Testing TDES.exe - Mode: CBC
ECHO -------------------------------
ECHO ---3DES FOX_RAW.TXT /ENCRYPT---
ECHO -------------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
TDES _FOX_RAW.TXT _FOX_RAW.3~e _TDTFOX.#K /encrypt _TDTFOX.#I
ECHO [*.E#-] [*.3~e]
COMP _FOX_RAW.3~e _TDTFOX.E#- /M /A

:DECIPHER1
ECHO ---3DES FOX_RAW.TXT /DECIPHER---
TDES _FOX_RAW.3~e _FOX_RAW.3~d _TDTFOX.#K /decipher _TDTFOX.#I
ECHO [*.RAW] [*.3~d]
type _FOX_RAW.TXT
echo ---
type _FOX_RAW.3~d
echo ---

:STEALING2
PAUSE
CLS
ECHO -------------------------------
ECHO ---3DES FOX_PAD.TXT /ENCRYPT---
ECHO -------------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
TDES _FOX_PAD.TXT _FOX_PAD.3~e _TDTFOX.#K /encrypt _TDTFOX.#I
ECHO [*.E#] [*.3~e]
COMP _FOX_PAD.3~e _TDTFOX.E# /M /A

ECHO ---3DES FOX_PAD.TXT /DECIPHER---
TDES _FOX_PAD.3~e _FOX_PAD.3~d _TDTFOX.#K /decipher _TDTFOX.#I
ECHO [*.TXT] [*.3~d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.3~d
echo ---

:PADDING1
PAUSE
CLS
ECHO ----------------------------
ECHO ---3DES FOX_RAW.TXT /CBCE---
ECHO ----------------------------
type _FOX_RAW.TXT
echo ---
PAUSE
TDES _FOX_RAW.TXT _FOX_RAW.3°e _TDTFOX.#K /cbce _TDTFOX.#I
ECHO [*.E#+] [*.3°e]
COMP _FOX_RAW.3°e _TDTFOX.E#+ /M /A

ECHO ---3DES FOX_RAW.TXT /CBCD---
TDES _FOX_RAW.3°e _FOX_RAW.3°d _TDTFOX.#K /cbcd _TDTFOX.#I
TDES _FOX_RAW.3°e _FOX_RAW_3°e.3~d _TDTFOX.#K /decipher _TDTFOX.#I
ECHO [*.RAW] [*.3°d] [*_3°e.3~d]
type _FOX_RAW.TXT					     
echo ---
type _FOX_RAW.3°d
echo ---
type _FOX_RAW_3°e.3~d
echo ---

:PADDING2
PAUSE
CLS
ECHO ----------------------------
ECHO ---3DES FOX_PAD.TXT /CBCE---
ECHO ----------------------------
type _FOX_PAD.TXT
echo ---
PAUSE
TDES _FOX_PAD.TXT _FOX_PAD.3°e _TDTFOX.#K /cbce _TDTFOX.#I
ECHO [*.E#] [*.3°e]
COMP _FOX_PAD.3°e _TDTFOX.E#= /M /A

ECHO ---3DES FOX_PAD.TXT /CBCD---
TDES _FOX_PAD.3°e _FOX_PAD.3°d _TDTFOX.#K /cbcd _TDTFOX.#I
TDES _FOX_PAD.3°e _FOX_PAD_3°e.3~d _TDTFOX.#K /decipher _TDTFOX.#I
ECHO [*.TXT] [*.3°d] [*_3°e.3~d]
type _FOX_PAD.TXT
echo ---
type _FOX_PAD.3°d
echo ---
type _FOX_PAD_3°e.3~d
echo ---

PAUSE
CLS
ECHO --------------------------
ECHO ---3DES CBC ALL RESULTS---
ECHO --------------------------
PAUSE
echo "3DES /ENCRYPT"
COMP _FOX_RAW.3~e _TDTFOX.E#- /M /A
COMP _FOX_PAD.3~e _TDTFOX.E# /M /A

echo "3DES /CBCE"
COMP _FOX_RAW.3°e _TDTFOX.E#+ /M /A
COMP _FOX_PAD.3°e _TDTFOX.E#= /M /A

echo "3DES /DECIPHER"
type _FOX_RAW.3~d
echo.
type _FOX_PAD.3~d
echo.
echo "3DES /CBCD"
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


