;; haDebug.asm - MASM Developer source file.
;; (c)2021 by helmut altmann

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program; see the file COPYING.  If not, write to
;; the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
;; Boston, MA 02111-1307, USA.
 
IFDEF haDEBUG		; Controlled by 'AFLAGS=/DhaDEBUG' in NMAKE 
_TEXT SEGMENT
;;------------------------------------------------------------------------------
;;
;;			   	DebugbufProc
;;
;;   Usage: in C++ Module:
;;   ---------------------
;;	extern "C" void DebugbufProc(UCHAR *);
;;
;;      UCHAR _asmBuf[64];
;;      UCHAR * pszAsmBuf = _asmBuf;
;;
;;      // Call debug proc in asm module
;;      DebugbufProc(pszAsmBuf);
;;	
;;      // Format data into whataever is of interest
;;      sprintf(___DebugBuf, "%02X %02X %02X %02X %02X %02X %02X %02X",
;;                           _asmBuf[0],_asmBuf[1],_asmBuf[2],_asmBuf[3],
;;                           _asmBuf[4],_asmBuf[5],_asmBuf[6],_asmBuf[7]);
;; 
;;      // Display data in a message box window
;;      MessageBoxA(NULL, ___DebugBuf, "DEBUG AES MAC _asmBuf", MB_OK);
;;
;;   in ASM Module: 
;;   --------------
;;	_Debugbuf DB 64 DUP (?)	; _DATA or _BSS SEGMENT (must be writable)
;;
;;	; Example:
;;	mov	rsi, DWORD PTR _key	     ; ASM-module keybuffer testing
;;	mov	rdi, rcx ;=OFFSET _Debugbuf  ; C-module interception buffer
;;	mov	rcx, 32			     ; Show 32 bytes
;;	rep movsb			     ; Transfer ecx bytes
;;
;;   ----------------------------------------------------------------------

_BSS SEGMENT
_Debugbuf DB 64 DUP (?)		; Debug buffer (e.g. 64 bytes)
_BSS ENDS

_DebugbufProc PROC PUBLIC USES rax rcx rbx rsi rdi, _buf:QWORD
	mov	rdi, rcx     ; -->Compiler param from c++ module '_buf:DWORD' = rcx
	lea	rsi, OFFSET _Debugbuf
 	mov	rcx, SIZEOF _Debugbuf
	rep movs BYTE PTR [rdi], [rsi]
	ret
_DebugbufProc ENDP

;; For testing the '_DebugbufProc' mechanism (remove comments to activate)
;;
;;ha;;_Debugbuf DB 64 DUP (?)	; _SMC SEGMENT (writeable): Debug buffer (e.g. 64 bytes)
;;ha;;_DebugbufProc PROC PUBLIC USES rax rcx rbx rsi rdi, _buf:QWORD
;;ha;;	mov	rdi, rcx     ; -->Compiler param from c++ module '_buf:DWORD' = rcx
;;ha;;	lea	rsi, OFFSET _Debugbuf
;;ha;;	;mov	BYTE PTR [rsi],   'A'	; Direct test only
;;ha;;	;mov	BYTE PTR [rsi+1], 'B'	; Direct test only
;;ha;;	;mov	BYTE PTR [rsi+2], 'C'	; Direct test only
;;ha;; 	mov	rcx, 32
;;ha;;	rep movs BYTE PTR [rdi], [rsi]
;;ha;;	ret
;;ha;;_DebugbufProc ENDP

;;------------------------------------------------------------------------------

_TEXT 	ENDS
ENDIF ;haDEBUG

	END

;;------------------------------------------------------------------------------

;; Usage Example:
;; --------------
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;	push	rsi
;;ha;;	push	rdi
;;ha;;	push	rcx
;;ha;;	mov	rsi, QWORD PTR _key
;;ha;;	mov	rdi, OFFSET _aesDebugbuf
;;ha;; 	mov	rcx, SIZEOF _aesDebugbuf/2  ; 1st half to intercept _key
;;ha;;	rep movs BYTE PTR [rdi], [rsi]	    
;;ha;;;	-----------------------------------
;;ha;;	mov	ecx, DWORD PTR _keysize     ; 2nd half multi purpose
;;ha;;	mov	DWORD PTR [rdi], ecx	    
;;ha;;;;ha;;	mov	rcx, QWORD PTR _ctx ; 2nd half multi purpose
;;ha;;;;ha;;	mov	QWORD PTR [rdi+1], rcx
;;ha;;	pop	rcx
;;ha;;	pop	rdi
;;ha;;	pop	rsi
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
