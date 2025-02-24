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
.MODEL FLAT
.686P			; All latest Intel CPUs (won't run on older PCs)
.XMM			; All latest Intel CPUs (won't run on older PCs)
 
_TEXT SEGMENT
;;+------------------------------------------------------------------------
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
;;	_Debugbuf DB 16 DUP (?)	; _DATA or _BSS SEGMENT (must be writable)
;;
;;	; Example (principle):
;;	mov	esi, DWORD PTR _key	  ; ASM-module keybuffer testing
;;	mov	edi, OFFSET _desDebugbuf  ; C-module interception buffer
;;	mov	ecx, SIZEOF _desDebugbuf  ; Show # bytes
;;	rep movs BYTE PTR [edi], [esi]	  ; Transfer ecx bytes
;;
;;      ; Usage Example (3des inline):
;;	push	esi
;;	push	edi
;;	push	ecx
;;	mov	esi, DWORD PTR _key
;;	mov	edi, OFFSET _3desDebugbuf
;; 	mov	ecx, SIZEOF _3desDebugbuf/2 ; 1st half to intercept _key
;;	rep movs BYTE PTR [edi], [esi]	   
;;;	-----------------------------------
;;	mov	ecx, DWORD PTR _edf	    ; 2nd half multi purpose _edf
;;	mov	DWORD PTR [edi], ecx
;;	pop	ecx
;;	pop	edi
;;	pop	esi
;;
;;   ----------------------------------------------------------------------
;;
_BSS SEGMENT
_Debugbuf DB 16 DUP (?)	 ; _BSS or _SMC SEGMENT: Debug buffer (e.g. 16 bytes)
_BSS ENDS

DebugbufProc PROC C PUBLIC USES ebx esi edi ecx eax, _buf:DWORD
	mov	esi, DWORD PTR _buf
	lea	edi, OFFSET _Debugbuf
 	mov	ecx, SIZEOF _Debugbuf
	rep movsb
	ret
DebugbufProc ENDP

;;ha;;+------------------------------------------------------------------------------------------------
;;ha;;
;;ha;;			   asmPrintf (principle)
;;ha;;
;;ha;;printf PROTO C, :PTR
;;ha;;asmPrintf PROTO C, fmtString:PTR DWORD, :BYTE, :BYTE, :BYTE, :BYTE,
;;ha;;				              :BYTE, :BYTE, :BYTE, :BYTE
;;ha;;
;;ha;;fmtList DB "inbuf: %02X %02X %02X %02X %02X %02X %02X %02X", 0Dh, 0Ah, 0
;;ha;;
;;ha;;asmPrintf PROC C USES ebx edi esi, fmtString:PTR DWORD, p1:BYTE, p2:BYTE, p3:BYTE, p4:BYTE,
;;ha;;				                              p5:BYTE, p6:BYTE, p7:BYTE, p8:BYTE
;;ha;;  movzx  	eax, BYTE PTR p8    ; push mumber 8 All addresses via [ebp]
;;ha;;	push   	eax
;;ha;;  movzx	eax, BYTE PTR p7    ; push number 7
;;ha;;	push	eax
;;ha;;  movzx  	eax, BYTE PTR p6    ; push mumber 6
;;ha;;	push   	eax
;;ha;;  movzx	eax, BYTE PTR p5    ; push number 5
;;ha;;	push	eax
;;ha;;  movzx  	eax, BYTE PTR p4    ; push mumber 4
;;ha;;	push   	eax
;;ha;;  movzx	eax, BYTE PTR p3    ; push number 3
;;ha;;	push	eax
;;ha;;  movzx  	eax, BYTE PTR p2    ; push mumber 2
;;ha;;	push   	eax
;;ha;;  movzx	eax, BYTE PTR p1    ; push number 1
;;ha;;	push	eax
;;ha;;  INVOKE printf, fmtString    ; invoke printf with formatted string
;;ha;;	add	esp, 8*SIZEOF DWORD ; adjust stack  ("add esp, 8)
;;ha;;	ret
;;ha;;asmPrintf ENDP
;;ha;;
;;ha;; ; Example:
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;push 	ebx
;;ha;;mov	ebx, DWORD PTR _key			; Buffer contents to be displayed
;;ha;;INVOKE asmPrintf, ADDR fmtList, BYTE PTR [ebx]+0, BYTE PTR [ebx]+1, BYTE PTR [ebx]+2, BYTE PTR [ebx]+3,
;;ha;;		                      BYTE PTR [ebx]+4, BYTE PTR [ebx]+5, BYTE PTR [ebx]+6, BYTE PTR [ebx]+7
;;ha;;pop 	ebx		        
;;ha;;push 	ebx
;;ha;;lea	ebx, DWORD PTR PermutationPC1[ebp]	; Buffer contents to be displayed
;;ha;;INVOKE asmPrintf, ADDR fmtList, BYTE PTR [ebx]+0, BYTE PTR [ebx]+1, BYTE PTR [ebx]+2, BYTE PTR [ebx]+3,
;;ha;;		      		      BYTE PTR [ebx]+4, BYTE PTR [ebx]+5, BYTE PTR [ebx]+6, BYTE PTR [ebx]+7
;;ha;;pop 	ebx		        
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---

;;+------------------------------------------------------------------------
;;
;;	asmPrintf (using VARARG) - 32bit Console application only
;;
;;   MACRO Usage (for debugging purposes only):
;;   ------------------------------------------
;;	_ASM_PRINTF_MOV _buf
;; 	_ASM_PRINTF_LEA _buf[ebp]
;;
;;   ----------------------------------------------------------------------
;;
;;ha;;includelib msvcrtd		       ; >>This LIB is not needed for VS 2010)
;;ha;;includelib legacy_stdio_definitions.lib  ; >>This LIB is not needed for VS 2010)
;;					       ;   For _scanf, _printf, ...
printf PROTO C, :PTR
asmPrintf PROTO C, :DWORD, :VARARG

_ASM_PRINTF_MOV MACRO _buf, i
	pushad
	mov	ebx, DWORD PTR _buf	;; Buffer contents to be displayed
	INVOKE asmPrintf, 8, BYTE PTR [ebx]+i+0, BYTE PTR [ebx]+i+1, BYTE PTR [ebx]+i+2, BYTE PTR [ebx]+i+3, 
		             BYTE PTR [ebx]+i+4, BYTE PTR [ebx]+i+5, BYTE PTR [ebx]+i+6, BYTE PTR [ebx]+i+7
	popad
	ENDM ;; _ASM_PRINTF_MOV

_ASM_PRINTF_LEA MACRO _buf, i
	pushad
	lea	ebx, DWORD PTR _buf	;; Buffer contents to be displayed
	INVOKE asmPrintf, 8, BYTE PTR [ebx]+i+0, BYTE PTR [ebx]+i+1, BYTE PTR [ebx]+i+2, BYTE PTR [ebx]+i+3, 
		             BYTE PTR [ebx]+i+4, BYTE PTR [ebx]+i+5, BYTE PTR [ebx]+i+6, BYTE PTR [ebx]+i+7
	popad
	ENDM ;; _ASM_PRINTF_LEA

fmtList DB 	"inbuf: %02X %02X %02X %02X %02X %02X %02X %02X", 0Dh, 0Ah, 0

asmPrintf PROC C USES ebx edi esi, argc:DWORD, arg:VARARG
	mov	ecx, argc		    ; Load loop argcount
	mov	esi, ecx		    ; Load arg index  [1 .. argc]
	mov	ebx, ecx		    ; Save for later
	shl	ebx, (SIZEOF DWORD)/2	    ; Set to the rightmost argument
@@:	  			    	    ; .WHILE argcount > 0
  	movzx  	eax, BYTE PTR arg[esi*SIZEOF DWORD-SIZEOF DWORD] ;Index [0 .. argc-1] (right-to-left)
	push   	eax			    ; Place for printf invocation on stack
	dec	esi			    ; Next arg 
	loop	@B			    ; Retrieve next arg
					    ; .ENDW
  	INVOKE printf, ADDR fmtList         ; Invoke printf with formatted string
	add	esp, ebx 		    ; Adjust stack ("add esp, argc*SIZEOF DWORD")
	ret				    ; Done.
asmPrintf ENDP


;;+------------------------------------------------------------------------
;;
;;	asmDDPrintf - 32bit Console application only
;;
;;   MACRO Usage (for debugging purposes only):
;;   ------------------------------------------
;;	_ASM_DD_PRINTF _int32
;;
;;   ----------------------------------------------------------------------
;;
printf PROTO C, :PTR
asmDDPrintf PROTO C, :DWORD

_ASM_DD_PRINTF MACRO _int32
	pushad
	mov	ebx, DWORD PTR _int32	;; DWORD to be displayed
	INVOKE asmDDPrintf, ebx 
	popad		        
	ENDM ;; _ASM_DD_PRINTF

fmtDDList DB "%08X-", 0  ;, 0Dh, 0Ah, 0  ; w/ CRLF
fmtDWList DB "%04X-", 0  ;, 0Dh, 0Ah, 0  ; w/ CRLF
fmtDBList DB "%02X-", 0  ;, 0Dh, 0Ah, 0  ; w/ CRLF

asmDDPrintf PROC C USES eax ebx ecx edx edi esi ebp, _inreg:DWORD
	push	_inreg		          ; Place for printf invocation on stack
	cmp	_inreg, 10000h		  ; For better surveillance on screen
	jae	asmDD
	cmp	_inreg, 100h		  ; For better surveillance on screen
	jae	asmDW
  	INVOKE printf, ADDR fmtDBList     ; Invoke printf with formatted string
	jmp	@F
asmDW:
  	INVOKE printf, ADDR fmtDWList     ; Invoke printf with formatted string
	jmp	@F
asmDD:
	INVOKE printf, ADDR fmtDDList ; Invoke printf with formatted string
@@:	add	esp, SIZEOF DWORD     ; Adjust stack
	ret			      ; Done.
asmDDPrintf ENDP

;;-------------------------------------------------------------------------

_TEXT 	ENDS
ENDIF ;haDEBUG

	END

;;-------------------------------------------------------------------------

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

;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;	push	esi
;;ha;;	push	edi
;;ha;;	push	ecx
;;ha;;	mov	esi, DWORD PTR _key
;;ha;;	mov	edi, OFFSET _3desDebugbuf
;;ha;; 	mov	ecx, SIZEOF _3desDebugbuf/2	; 1st half to intercept _key
;;ha;;	rep movs BYTE PTR [edi], [esi]	   
;;ha;;;	-----------------------------------
;;ha;;	mov	ecx, DWORD PTR _edf	   	; 2nd half multi purpose
;;ha;;	mov	DWORD PTR [edi], ecx
;;ha;;	pop	ecx
;;ha;;	pop	edi
;;ha;;	pop	esi
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---

;;ha;;---DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;; mov	esi, DWORD PTR _outblock
;;ha;; mov	edi, OFFSET _desDebugbuf
;;ha;; mov	ecx, 8
;;ha;; rep movsb
;;ha;;---DEBUG------DEBUG------DEBUG------DEBUG---

;;ha;;---DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;; mov	esi, DWORD PTR _inblock
;;ha;; mov	edi, OFFSET _desDebugbuf
;;ha;; mov	ecx, 8
;;ha;; rep movsb
;;ha;;---DEBUG------DEBUG------DEBUG------DEBUG---


;; Usage Example:
;; --------------
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;	push	rsi
;;ha;;	push	rdi
;;ha;;	push	rcx
;;ha;;	mov	rsi, QWORD PTR _key
;;ha;;	mov	rdi, OFFSET _Debugbuf
;;ha;; 	mov	rcx, SIZEOF _Debugbuf/2 ; 1st half to intercept _key
;;ha;;	rep movs BYTE PTR [rdi], [rsi]	    
;;ha;;;	------------------------------
;;ha;;	mov	ecx, DWORD PTR _keylen	; 2nd half multi purpose
;;ha;;	mov	DWORD PTR [rdi], ecx
;;ha;;	pop	rcx
;;ha;;	pop	rdi
;;ha;;	pop	rsi
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
														;
