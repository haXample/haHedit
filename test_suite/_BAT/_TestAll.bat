@ECHO OFF
CLS
ECHO %0: Crypto Test-Suite V1.0 by ha.

REM Multilingual (Latin I) DOS Console standard codepage=850
REM MODE CON CP SELECT=850

REM Windows Western European codepage = 1252
MODE CON CP SELECT=1252 >nul

REM Redirecting stderr to stdout using 2>&1

REM >nul or 1>nul
REM Redirects anything written to the standard output stream (stream number 1)
REM  to the nul device. Anything sent to this device is discarded.
REM  The effect is that all the normal output of the ping command is hidden.

REM 2>&1
REM This redirects anything written to the standard error stream (stream number 2).
REM  As in the previous case, this is done to hide output (errors in this case),
REM  but instead of directly requesting to write to the nul device (we could have done 2>nul),
REM  this syntax requests to send data in the standard error stream to a copy of the handle
REM  used in the standard output stream.

IF "%2" == "" GOTO TEST_START

REM IF NOT "%2" == "_W10" GOTO UNKNOWN
REM IF "%2" == "fast" GOTO TEST_START
REM IF "%2" == "quick" GOTO TEST_START
REM IF "%2" == "bio" GOTO TEST_START
REM GOTO UNKNOWN

:TEST_START 
IF "%1" == "" GOTO HELP
IF "%1" == "clean" GOTO CLEAN

IF "%1" == "aes" GOTO  _AES
IF "%1" == "des" GOTO  _DES
IF "%1" == "tdes" GOTO _TDES
IF "%1" == "AES" GOTO  _AES
IF "%1" == "DES" GOTO  _DES
IF "%1" == "TDES" GOTO _TDES
GOTO UNKNOWN

:CLEAN
PUSHD ".\aes"
IF EXIST _FOX_RAW.A?e DEL _FOX_RAW.A?e
IF EXIST _FOX_RAW.A?d DEL _FOX_RAW.A?d
IF EXIST _FOX_RAW_A?e.A_d DEL _FOX_RAW_A?e.A_d
IF EXIST _FOX_RAW_A?e.A~d DEL _FOX_RAW_A?e.A~d
IF EXIST _FOX_PAD.A?e DEL _FOX_PAD.A?e
IF EXIST _FOX_PAD.A?d DEL _FOX_PAD.A?d
IF EXIST _FOX_PAD_A?e.A_d DEL _FOX_PAD_A?e.A_d
IF EXIST _FOX_PAD_A?e.A~d DEL _FOX_PAD_A?e.A~d
IF EXIST aes.exe DEL aes.exe
POPD

PUSHD ".\aesmac"
IF EXIST _Aes???_128.A~m DEL _Aes???_128.A~m
IF EXIST _Aes???_192.A~m DEL _Aes???_192.A~m
IF EXIST _Aes???_256.A~m DEL _Aes???_256.A~m
IF EXIST aes.exe DEL aes.exe
POPD

PUSHD ".\des"
IF EXIST _FOX_RAW.D?e DEL _FOX_RAW.D?e
IF EXIST _FOX_RAW.D?d DEL _FOX_RAW.D?d
IF EXIST _FOX_RAW_D?e.D_d DEL _FOX_RAW_D?e.D_d
IF EXIST _FOX_RAW_D?e.D~d DEL _FOX_RAW_D?e.D~d
IF EXIST _FOX_PAD.D?e DEL _FOX_PAD.D?e
IF EXIST _FOX_PAD.D?d DEL _FOX_PAD.D?d
IF EXIST _FOX_PAD_D?e.D_d DEL _FOX_PAD_D?e.D_d
IF EXIST des.exe DEL des.exe
POPD

PUSHD ".\desmac"
IF EXIST _Des???_64.D~m DEL _Des???_64.D~m
IF EXIST des.exe DEL des.exe
POPD

PUSHD ".\tdes"
IF EXIST _FOX_RAW.3?e DEL _FOX_RAW.3?e
IF EXIST _FOX_RAW.3?d DEL _FOX_RAW.3?d
IF EXIST _FOX_RAW_3?e.3_d DEL _FOX_RAW_3?e.3_d
IF EXIST _FOX_RAW_3?e.3~d DEL _FOX_RAW_3?e.3~d
IF EXIST _FOX_PAD.3?e DEL _FOX_PAD.3?e
IF EXIST _FOX_PAD.3?d DEL _FOX_PAD.3?d
IF EXIST _FOX_PAD_3?e.3_d DEL _FOX_PAD_3?e.3_d
IF EXIST _FOX_PAD_3?e.3~d DEL _FOX_PAD_3?e.3~d
IF EXIST tdes.exe DEL tdes.exe
IF EXIST des.exe DEL des.exe
POPD

PUSHD ".\tdesmac"
IF EXIST _Tdes???_128.3~m DEL _Tdes???_128.3~m
IF EXIST _Tdes???_192.3~m DEL _Tdes???_192.3~m
IF EXIST tdes.exe DEL tdes.exe
POPD

ECHO All generated testfiles have been deleted. 
GOTO END

:_AES
call :_AES_TEST %1 %2 %3
ECHO .. press any key to terminate batch processing
Pause > nul 2>&1
GOTO END

:_DES
call :_DES_TEST %1 %2 %3
ECHO .. press any key to terminate batch processing
Pause > nul 2>&1
GOTO END

:_TDES
call :_TDES_TEST %1 %2 %3
ECHO .. press any key to terminate batch processing
Pause > nul 2>&1
GOTO END

REM ---------------------------
REM Batchprogram: '_AES_TEST'
REM ---------------------------
:_AES_TEST
PUSHD ".\aes"
call _atfoxcbc.bat %2 %3
PAUSE
call _atfoxecb.bat %2 %3
POPD
exit /B 0
 
REM ---------------------------
REM Batchprogram: '_DES_TEST'
REM ---------------------------
:_DES_TEST
PUSHD ".\des"
call _dtfoxcbc.bat %2 %3
PAUSE
call _dtfoxecb.bat %2 %3
POPD
exit /B 0

REM ----------------------------
REM Batchprogram: '_TDES_TEST'
REM ----------------------------
:_TDES_TEST
REM IF "%2" == "quick" GOTO _TDES_EMULATION
REM IF "%2" == "bio" GOTO _TDES_EMULATION
PUSHD ".\tdes"
call _tdtfoxcbc.bat %2 %3
PAUSE
call _tdtfoxecb.bat %2 %3
REM POPD
REM exit /B 0

ECHO -----------------------------------------------------
ECHO ---Triple DES Emulation using DES.EXE: (Reference)---
ECHO -----------------------------------------------------
PAUSE
REM :_TDES_EMULATION
REM PUSHD ".\tdes"
call _tdtfox.bat %2 %3
PAUSE
POPD
exit /B 0

REM ------------------------
REM Batchprogram: End & Exit
REM ------------------------
:HELP
ECHO Usage: "_testall [AES | DES | TDES] [CLEAN]"
ECHO.
ECHO Note: Runs on Windows XP or greater
ECHO. 
ECHO "Examples:
ECHO "  _testall clean 
ECHO "  _testall aes"
ECHO "  _testall des"
ECHO "  _testall tdes"
GOTO :END

REM ------------------------
REM Batchprogram: End & Exit
REM ------------------------
:UNKNOWN
ECHO %0: [%1 %2 %3] Unknown Test-suite, batch aborted
GOTO END

:END
REM Multilingual (Latin I) DOS Console standard codepage=850
MODE CON CP SELECT=850 >nul
ECHO > nul 2>&1
ECHO Batch terminated.
GOTO :eof