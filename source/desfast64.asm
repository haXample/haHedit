;; desfast64.asm - MASM Developer source file for DES.
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
 
;;
;; The PERMUTATION_48BIT_BLOCK macro supports the definition of the permuted
;; choice tables. It transforms the notation described in the DES specs into
;; the convention expected by this algorithm. Its purpose is mainatin legibility.
;;
PERMUTATION_48BIT_BLOCK MACRO b1, b2, b3, b4, b5, b6
  DB b1;-1
  DB b2;-1
  DB b3;-1
  DB b4;-1
  DB b5;-1
  DB b6;-1
  ENDM

PERMUTATION_56BIT_BLOCK MACRO b1, b2, b3, b4, b5, b6, b7
  DB b1;-1
  DB b2;-1
  DB b3;-1
  DB b4;-1
  DB b5;-1
  DB b6;-1
  DB b7;-1
  ENDM
;
; Supported DES Algorithm Modes
;
ENCRYPT         EQU     0       ; Encrypts a block of plain text (see desmain.cpp)
DECIPHER        EQU     1       ; Deciphers a block of encrypted text  (see desmain.cpp)

_DATA   SEGMENT
;------------------------------------------------------------------------------
;       DES Substitution Boxes
;
; The table is organized just as described in the common DES documentations.
; It is very easy to survey and understand.
;
; The SBoxTable is accessed by a 6-bit index:
;
;       [ *  *  r1 c3 c2 c1 c0 r0 ]
;
;        Bits r1r0 = [5,0] select one of 4 rows in a box
;        Bits c3..c0 = [4:1] select one of 16 colums in a box
;
;       Within a loop all eight S-Boxes are consulted for DES substitution.
;
;       Depending on the selection either the lsb or the msb from the
;       table value (column) is used for substitution (see DES specification).
;
SBoxTable:
;
; S[1]     c0     c1     c2      c3     c4    c5     c6     c7     c8     c9     cA     cB     cC     cD     cE     cF
;
SBox1 LABEL BYTE ; SHL 4
     DB   14*16,  4*16, 13*16,  1*16,  2*16, 15*16, 11*16,  8*16,  3*16, 10*16,  6*16, 12*16,  5*16,  9*16,  0*16,  7*16   ; r0
     DB    0*16, 15*16,  7*16,  4*16, 14*16,  2*16, 13*16,  1*16, 10*16,  6*16, 12*16, 11*16,  9*16,  5*16,  3*16,  8*16   ; r1
     DB    4*16,  1*16, 14*16,  8*16, 13*16,  6*16,  2*16, 11*16, 15*16, 12*16,  9*16,  7*16,  3*16, 10*16,  5*16,  0*16   ; r2
     DB   15*16, 12*16,  8*16,  2*16,  4*16,  9*16,  1*16,  7*16,  5*16, 11*16,  3*16, 14*16, 10*16,  0*16,  6*16, 13*16   ; r3
SBOX_SIZE EQU $-SBox1 ; All S-boxes have the same size

;
; S[2]    c0  c1  c2  c3  c4  c5  c6  c7  c8  c9  cA  cB  cC  cD  cE  cF
;
SBox2 LABEL BYTE ; !! SHL 0 no shift left !!
     DB   15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10  ; r0
     DB    3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5  ; r1
     DB    0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15  ; r2
     DB   13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9  ; r3

;
; S[3]
;
SBox3 LABEL BYTE ; SHL 4
     DB   10*16,  0*16,  9*16, 14*16,  6*16,  3*16, 15*16,  5*16,  1*16, 13*16, 12*16,  7*16, 11*16,  4*16,  2*16,  8*16   ; r0
     DB   13*16,  7*16,  0*16,  9*16,  3*16,  4*16,  6*16, 10*16,  2*16,  8*16,  5*16, 14*16, 12*16, 11*16, 15*16,  1*16   ; r1
     DB   13*16,  6*16,  4*16,  9*16,  8*16, 15*16,  3*16,  0*16, 11*16,  1*16,  2*16, 12*16,  5*16, 10*16, 14*16,  7*16   ; r2
     DB    1*16, 10*16, 13*16,  0*16,  6*16,  9*16,  8*16,  7*16,  4*16, 15*16, 14*16,  3*16, 11*16,  5*16,  2*16, 12*16   ; r3

;
; S[4]
;
SBox4 LABEL BYTE ; !! SHL 0 no shift left !!
     DB    7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15  ; r0
     DB   13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9  ; r1
     DB   10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4  ; r2
     DB    3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14  ; r3

;
; S[5]
;
SBox5 LABEL BYTE ; SHL 4
     DB    2*16, 12*16,  4*16,  1*16,  7*16, 10*16, 11*16,  6*16,  8*16,  5*16,  3*16, 15*16, 13*16,  0*16, 14*16,  9*16  ; r0
     DB   14*16, 11*16,  2*16, 12*16,  4*16,  7*16, 13*16,  1*16,  5*16,  0*16, 15*16, 10*16,  3*16,  9*16,  8*16,  6*16  ; r1
     DB    4*16,  2*16,  1*16, 11*16, 10*16, 13*16,  7*16,  8*16, 15*16,  9*16, 12*16,  5*16,  6*16,  3*16,  0*16, 14*16  ; r2
     DB   11*16,  8*16, 12*16,  7*16,  1*16, 14*16,  2*16, 13*16,  6*16, 15*16,  0*16,  9*16, 10*16,  4*16,  5*16,  3*16  ; r3

;
; S[6]
;
SBox6 LABEL BYTE ; !! SHL 0 no shift left !!
     DB   12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11  ; r0
     DB   10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8  ; r1
     DB    9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6  ; r2
     DB    4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13  ; r3

;
; S[7]
;
SBox7 LABEL BYTE ; SHL 4
     DB    4*16, 11*16,  2*16, 14*16, 15*16,  0*16,  8*16, 13*16,  3*16, 12*16,  9*16,  7*16,  5*16, 10*16,  6*16,  1*16  ; r0
     DB   13*16,  0*16, 11*16,  7*16,  4*16,  9*16,  1*16, 10*16, 14*16,  3*16,  5*16, 12*16,  2*16, 15*16,  8*16,  6*16  ; r1
     DB    1*16,  4*16, 11*16, 13*16, 12*16,  3*16,  7*16, 14*16, 10*16, 15*16,  6*16,  8*16,  0*16,  5*16,  9*16,  2*16  ; r2
     DB    6*16, 11*16, 13*16,  8*16,  1*16,  4*16, 10*16,  7*16,  9*16,  5*16,  0*16, 15*16, 14*16,  2*16,  3*16, 12*16  ; r3

;
; S[8]
;
SBox8 LABEL BYTE ; !! SHL 0 no shift left !!
     DB   13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7  ; r0
     DB   01, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2  ; r1
     DB   07, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8  ; r2
     DB   02,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11  ; r3


;------------------------------------------------------------------------------
;
PermutationPC2 LABEL BYTE
        PERMUTATION_48BIT_BLOCK         13, 16, 10, 23,  0,  4
        PERMUTATION_48BIT_BLOCK          2, 27, 14,  5, 20,  9
        PERMUTATION_48BIT_BLOCK         22, 18, 11,  3, 25,  7
        PERMUTATION_48BIT_BLOCK         15,  6, 26, 19, 12,  1
        PERMUTATION_48BIT_BLOCK         40, 51, 30, 36, 46, 54
        PERMUTATION_48BIT_BLOCK         29, 39, 50, 44, 32, 47
        PERMUTATION_48BIT_BLOCK         43, 48, 38, 55, 33, 52
        PERMUTATION_48BIT_BLOCK         45, 41, 49, 35, 28, 31

PermutationPC1 LABEL BYTE               ; Permuted choice table (key)
        PERMUTATION_56BIT_BLOCK         56, 48, 40, 32, 24, 16,  8
        PERMUTATION_56BIT_BLOCK          0, 57, 49, 41, 33, 25, 17
        PERMUTATION_56BIT_BLOCK          9,  1, 58, 50, 42, 34, 26
        PERMUTATION_56BIT_BLOCK         18, 10,  2, 59, 51, 43, 35
        PERMUTATION_56BIT_BLOCK         62, 54, 46, 38, 30, 22, 14
        PERMUTATION_56BIT_BLOCK          6, 61, 53, 45, 37, 29, 21
        PERMUTATION_56BIT_BLOCK         13,  5, 60, 52, 44, 36, 28
        PERMUTATION_56BIT_BLOCK         20, 12,  4, 27, 19, 11,  3


;------------------------------------------------------------------------------
;
PermutationIP   DD      40h, 10h, 04h, 01h, 80h, 20h, 08h, 02h
ReferenceIP     DD      01h, 02h, 04h, 08h, 10h, 20h, 40h, 80h

Permutation48i \
        DB      24, 15,  6, 19, 20, 28, 20, 28, 11, 27, 16,  0,
                16,  0, 14, 22, 25,  4, 25,  4, 17, 30,  9,  1,
                 9,  1,  7, 23, 13, 31, 13, 31, 26,  2,  8, 18,
                 8, 18, 12, 29,  5, 21,  5, 21, 10,  3, 24, 15


;
; Note: Bit 0 is left-most in byte (big-endian)
;
NotationTable8Bit \
        DD      80h, 40h, 20h, 10h, 08h, 04h, 02h, 01h

NotationTable48Bit \
        DD      00800000h, 00400000h, 00200000h, 00100000h,
                00080000h, 00040000h, 00020000h, 00010000h,
                00008000h, 00004000h, 00002000h, 00001000h,
                00000800h, 00000400h, 00000200h, 00000100h,
                00000080h, 00000040h, 00000020h, 00000010h,
                00000008h, 00000004h, 00000002h, 00000001h
NOTATION_48BIT_SIZE EQU $-NotationTable48Bit

PC1LeftRotationTable LABEL BYTE                 ; Number left rotations of pc1
        DB      1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28
ROTATION_TABLE_SIZE EQU $-PC1LeftRotationTable
;
; Temporary auxiliary pointer (placed at SIZEOF PermutationArray48a-d + SIZEOF QWORD)
;
pfill   DQ      0; OFFSET PermutationArray48a + (4*256*2)+8   ; =2056           
_cmp48x DD      0

_DATA   ENDS

_BSS    SEGMENT
BLOCK_SIZE      EQU     8                       ; Size of a DES plaintext block
SBOX_ARRAY_SIZE EQU     8 * (8*SBOX_SIZE)       ; =4096

IgnitedDES      DD      0                       ; Init flag

KeyArray        DD      BLOCK_SIZE * SIZEOF DWORD DUP (?)

SBoxArray12     DB      SBOX_ARRAY_SIZE DUP (?)   
SBoxArray34     DB      SBOX_ARRAY_SIZE DUP (?)   
SBoxArray56     DB      SBOX_ARRAY_SIZE DUP (?)   
SBoxArray78     DB      SBOX_ARRAY_SIZE DUP (?)   

PermutationArray48a     DD  2*256 DUP (?)
PermutationArray48b     DD  2*256 DUP (?)
PermutationArray48c     DD  2*256 DUP (?)
PermutationArray48d     DD  2*256 DUP (?)

_BSS    ENDS

_TEXT   SEGMENT
;+-----------------------------------------------------------------------------
;
;                               desKeyInit
;
;               desKeyInit PROTO C, key:DWORD, edf:DWORD
;
;       Language type: PROC C (Public interface to desmain.cpp)
;       desmain.cpp: extern "C" void kinit(char* p, int);
;
; Initialize key schedule array. Discard the key parity and take only 56 bits
;
; Function compile flags: /Ogtpy
; COMDAT ?kinit@@YAXPADH@Z PROC PUBLIC (C++ without language type!)
;
desKeyInit PROC PUBLIC USES rbx rsi rdi, _key:QWORD, _edf:DWORD
        LOCAL   _k:DWORD
        LOCAL   _PermutationPC1m[56]:BYTE
        LOCAL   _pcr[56]:BYTE
;
; Caveat programmer:
; Extern parameters _key:QWORD, _edf:DWORD from C++ Compiler x64 not via stack
;  but instead via registers rcx, edx.
;
        mov     DWORD PTR _edf, edx     ; Extern param _edf:DWORD from C-Compiler via edx
        mov     QWORD PTR _key, rcx     ; Extern param _key:QWORD from C-Compiler via rcx

        cmp     DWORD PTR IgnitedDES, 1
        je      SHORT @F

        call    p48init                 ; Prepared for Algorithm here (optimal speed)
;
; Convert PermutationPC1 to bits of key
;
@@:
        xor     rdx, rdx

kinit_1:
        movsx   rax, BYTE PTR PermutationPC1[rdx]
        mov     ecx, eax
        and     eax, 7
        movsxd  rbx, eax                   ; Build index in rbx for later
        mov     eax, ecx
        sar     eax, 3
        cdqe
        mov     rcx, QWORD PTR _key
        movsx   eax, BYTE PTR [rcx+rax]
        and     eax, DWORD PTR NotationTable8Bit[rbx*SIZEOF DWORD] ; rbx index addressing
        test    eax, eax

        mov     ecx, ENCRYPT
        mov     eax, DECIPHER
        ;~~~~~~~~~~~~~~~
        cmovne  ecx, eax                   ; .686 CPUs only, Encrypt/Decipher
        ;~~~~~~~~~~~~~~~

        mov     BYTE PTR _PermutationPC1m[rdx], cl
        inc     rdx
        cmp     rdx, SIZEOF _PermutationPC1m ; 56
        jl      SHORT kinit_1
;       ---------------------

kinit_2:
        xor     ecx, ecx                       ; Loop counter

kinit_3:
        cmp     DWORD PTR _edf, DECIPHER
        jne     SHORT $kinit_5
        mov     eax, 15
        sub     eax, ecx
        shl     eax, 1
        movsxd  rdi, eax                        ; _m$
        jmp     SHORT kinit_6                   

$kinit_5:
        mov     eax, ecx
        shl     eax, 1
        movsxd  rdi, eax                        ; _m$

kinit_6:
        mov     QWORD PTR KeyArray[rdi*SIZEOF DWORD], 0 ; Clear key array buffer
        mov     DWORD PTR _k, 0                         ; Init

kinit_7:
        cmp     DWORD PTR _k, SIZEOF _PermutationPC1m/2 ;28
        jge     SHORT kinit_9

        mov     esi, SIZEOF _PermutationPC1m/2 ;28
        jmp     SHORT kinit_10

kinit_9:
        mov     esi, SIZEOF _PermutationPC1m ; 56

kinit_10:
        movsxd  rax, ecx
        movsx   eax, BYTE PTR PC1LeftRotationTable[rax]
        add     eax, DWORD PTR _k
        cmp     eax, esi
        jge     SHORT kinit_11

        mov     esi, eax
        jmp     SHORT kinit_12

kinit_11:
        sub     eax, SIZEOF _PermutationPC1m/2 ;28
        mov     esi, eax

kinit_12:
        movsxd  rax, esi
        movsxd  rbx, DWORD PTR _k
        mov     al, BYTE PTR _PermutationPC1m[rax]
        mov     BYTE PTR _pcr[rbx], al

        inc     DWORD PTR _k
        cmp     DWORD PTR _k, SIZEOF _PermutationPC1m ;56
        jl      kinit_7

kinit_13:
        mov     DWORD PTR _k, 0

kinit_14:
        movsxd  rax, DWORD PTR _k
        movsx   rax, BYTE PTR PermutationPC2[rax]
        movsx   eax, BYTE PTR _pcr[rax]
        test    eax, eax
        je      SHORT kinit_16

        movsxd  rdx, DWORD PTR _k
        mov     edx, DWORD PTR NotationTable48Bit[rdx*SIZEOF DWORD]
        or      DWORD PTR KeyArray[rdi*SIZEOF DWORD], edx

kinit_16:
        mov     eax, DWORD PTR _k
        add     eax, NOTATION_48BIT_SIZE / SIZEOF DWORD ; 24
        cdqe
        movsx   rax, BYTE PTR PermutationPC2[rax]
        movsx   eax, BYTE PTR _pcr[rax]
        test    eax, eax
        je      SHORT kinit_17

        inc     rdi                                     ; rdi = _n$
        movsxd  rdx, DWORD PTR _k
        mov     edx, DWORD PTR NotationTable48Bit[rdx*SIZEOF DWORD]
        or      DWORD PTR KeyArray[rdi*SIZEOF DWORD], edx
        dec     rdi                                     ; rdi = _m$

kinit_17:
        inc     DWORD PTR _k
        cmp     DWORD PTR _k, NOTATION_48BIT_SIZE / SIZEOF DWORD ; 24
        jl      kinit_14

kinit_18:
        inc     ecx
        cmp     ecx, ROTATION_TABLE_SIZE
        jl      kinit_3

kinit_ret:
        ret     0
desKeyInit ENDP


;+-----------------------------------------------------------------------------
;
;                          desAlgorithm
;
;       desAlgorithm PROTO C, inblock:DWORD, outblock:DWORD
;
;       Language type: PROC C (Public interface to desmain.cpp)
;       desmain.cpp: extern "C" void desAlgorithm (char* p1, char* p2);
;
;       Note: 1) The Language type may be defined for each function seperately.
;             2) C++ Compiler uses "ebp and [ebp]" when "/O2" option is used.
;                                                                                                   
;       see: Intel(R) 64 and IA-32 Optimization Reference Manual
;            "unrolling loops"
;
; Function compile flags: /Ogtpy
; COMDAT ?desAlgorithm@@YAXPAD0@Z PROC PUBLIC (C++ without language type!)
;
desAlgorithm PROC PUBLIC USES r9 r9 rbx rcx rdx rsi rdi, _inblock:QWORD, _outblock:QWORD
        LOCAL   __ebp:DWORD ; We can't use "ebp" (because of "INVOKE" convention)
        LOCAL   _keys:QWORD
        LOCAL   _swap[16]:BYTE
        LOCAL   _scratch[8]:BYTE

;
; Caveat programmer:
; Extern parameters _inblock:QWORD, _outblock:QWORD from C++ Compiler x64 not via stack
;  but instead via registers rcx, rdx.
;
        mov     QWORD PTR _outblock, rdx; Extern param _edf:DWORD from C-Compiler via edx
        mov     QWORD PTR _inblock, rcx ; Extern param _key:QWORD from C-Compiler via rcx

;;      INVOKE permute, _inblock, OFFSET PermutationIP, OFFSET ReferenceIP, _scratch    
;       --------------------------------
        lea     rsi, QWORD PTR _scratch
        lea     rcx, OFFSET ReferenceIP
        lea     rdx, OFFSET PermutationIP
        mov     rdi, QWORD PTR _inblock
        push    rsi
        push    rcx
        push    rdx
        push    rdi
        call    desPermute              ; desPermute
        add     rsp, 4*SIZEOF QWORD
;       --------------------------------

        lea     rdi, QWORD PTR _swap            ; Init pointer to _swap
        lea     rbx, QWORD PTR _swap[16]        ; Init end-of _swap

        ALIGN 16

desAlgorit_1:
        movsx   edx, BYTE PTR [rsi][0]          ; Data of _scratch[0]
        lea     rsi, DWORD PTR [rsi+4]          ; Pointer to _scratch from permute
        movzx   eax, BYTE PTR [rsi-3]           ; Data of _scratch[1]
        shl     eax, 16
        shl     edx, 24
        or      edx, eax

        movzx   eax, BYTE PTR [rsi-2]           ; Data of _scratch[2]
        shl     eax, 8
        or      edx, eax

        movzx   eax, BYTE PTR [rsi-1]           ; Data of _scratch[3]
        or      edx, eax

        mov     ecx, edx
        mov     eax, edx
        sar     ecx, 2
        and     eax, 01F80000h
        and     ecx, 0007E000h
        or      ecx, eax

        mov     eax, edx
        shr     ecx, 2
        and     eax, 1F800000h
        or      ecx, eax

        mov     eax, edx
        sar     eax, 9
        shr     ecx, 11
        and     eax, 007C0000h
        or      ecx, eax

        mov     eax, edx
        and     eax, 1
        shl     eax, 23
        or      ecx, eax
        mov     DWORD PTR [rdi], ecx            ; Data of _swap

        mov     eax, edx
        and     eax, 00001F80h                          
        mov     ecx, edx
        and     ecx, 0001F800h                          
        shl     ecx, 2
        or      ecx, eax

        mov     eax, edx
        shl     ecx, 2
        and     eax, 000001F8h                          
        or      ecx, eax

        mov     eax, edx
        shl     ecx, 2
        and     eax, 0000001Fh                          
        or      ecx, eax
        sar     edx, 31
        add     ecx, ecx
        and     edx, 1
        or      ecx, edx
        mov     DWORD PTR [rdi+4], ecx          ; _swap[+4]

        add     rdi, BLOCK_SIZE
        cmp     rdi, rbx                        ; Reached end of _swap?
        jb      desAlgorit_1                    ; Continue looping
;       --------------------------              ------------------

        lea     r8, QWORD PTR _swap             ; r8 = Pointer to _swap[0] (1st half)
        lea     r9, QWORD PTR _swap[8]          ; r9 = Pointer to _swap[8] (2nd half)
        lea     rax, OFFSET KeyArray
        mov     QWORD PTR _keys, rax

        mov     DWORD PTR __ebp, 16             ; Init loop counter

        ALIGN 16

desAlgorit_2:
        mov     ecx, DWORD PTR [r9]              ; Data of _swap
        xor     ecx, DWORD PTR [rax]             ; Data of keyArray
        mov     eax, ecx

        sar     eax, 12                          ; rax, OFFSET KeyArray
        cdqe
        movzx   rsi, BYTE PTR SBoxArray12[rax]
        lea     rsi, QWORD PTR PermutationArray48a[rsi*8]
        mov     eax, DWORD PTR [rsi]             ; Save _suba
        mov     ebx, DWORD PTR [rsi+4]           ; Save _subb

        mov     edx, ecx                         ; Data of keyArray
        and     rdx, 00000FFFh
        movzx   rsi, BYTE PTR SBoxArray34[rdx]
        lea     rsi, QWORD PTR PermutationArray48b[rsi*8]
        or      eax, DWORD PTR [rsi]             ; Save _suba
        or      ebx, DWORD PTR [rsi+4]           ; Save _subb

        add     QWORD PTR _keys, SIZEOF DWORD    ; Advance pointers
        add     r9, SIZEOF DWORD

        mov     rdx, QWORD PTR _keys
        mov     edx, DWORD PTR [rdx]             ; Key data
        xor     edx, DWORD PTR [r9]              ; swap

        mov     ecx, edx
        sar     ecx, 12
        and     rcx, 0FFFFFFFFh
        movzx   rsi, BYTE PTR SBoxArray56[rcx]
        lea     rsi, QWORD PTR PermutationArray48c[rsi*8]
        or      eax, DWORD PTR [rsi]             ; Save _suba
        or      ebx, DWORD PTR [rsi+4]           ; Save _subb

        and     rdx, 00000FFFh                   ; keys 
        movzx   rsi, BYTE PTR SBoxArray78[rdx]
        lea     rsi, QWORD PTR PermutationArray48d[rsi*8]
        or      eax, DWORD PTR [rsi]             ; _suba
        or      ebx, DWORD PTR [rsi+4]           ; _subb

        xor     DWORD PTR [r8], eax              ; _suba
        xor     DWORD PTR [r8+4], ebx            ; _subb

        add     r8, SIZEOF QWORD         ; Advance pointers
        add     r9, SIZEOF DWORD
        add     QWORD PTR _keys, SIZEOF DWORD

        lea     rax, QWORD PTR _swap
        test    BYTE PTR __ebp, 1        ; Test _swap-index evn/odd
        ;~~~~~~~~~~~~~~~
        cmove   r9, rax                  ; .686 EVN: ebx = _swap (performance)
        cmovnbe r8, rax                  ; .686 ODD: edi = _swap (performance)
        ;~~~~~~~~~~~~~~~ 

        mov     rax, QWORD PTR _keys
        dec     DWORD PTR __ebp
        jnz     desAlgorit_2             ; Continue looping
;       ----------------------------

desAlgorit_7:
        mov     rax, QWORD PTR [r8]      ; Swap left and right halfs of _swap
        mov     rcx, QWORD PTR [r9]
        mov     QWORD PTR [r8], rcx      ; r8 = Pointer to _swap[0] (1st half)
        mov     QWORD PTR [r9], rax      ; r9 = Pointer to _swap[8] (2nd half)

        lea     rsi, QWORD PTR _scratch
        lea     rdi, QWORD PTR _swap
        lea     rbx, QWORD PTR _swap[16] ; Init end-of _swap

        ALIGN 16

desAlgorit_8:
        mov     eax, DWORD PTR [rdi]
        and     eax, 00780000h
        sar     eax, 15
        mov     ecx, DWORD PTR [rdi]
        and     ecx, 0001E000h
        sar     ecx, 13
        or      al, cl
        mov     BYTE PTR [rsi], al
        inc     rsi

        mov     eax, DWORD PTR [rdi]
        and     eax, 00000780h
        sar     eax, 3
        mov     ecx, DWORD PTR [rdi]
        and     ecx, 30
        sar     ecx, 1
        or      al, cl
        mov     BYTE PTR [rsi], al
        inc     rsi

        add     rdi, SIZEOF DWORD
        cmp     rdi, rbx              ; Reached end of _swap?
        jb      desAlgorit_8          ; no - continue

desAlgorit_9:
;;      INVOKE permute, _scratch, OFFSET ReferenceIP, OFFSET PermutationIP, _outblock   
;       --------------------------------
        mov     rdi, QWORD PTR _outblock
        lea     rcx, OFFSET PermutationIP
        lea     rdx, OFFSET ReferenceIP
        lea     rsi, QWORD PTR _scratch
        push    rdi
        push    rcx
        push    rdx
        push    rsi
        call    desPermute
        add     rsp, 4*SIZEOF QWORD
;       --------------------------------

        ret     0
desAlgorithm ENDP


;------------------------------------------------------------------------------
;
;                       desPermute  (very fast, .686 only)
;
;       see: Intel(R) 64 and IA-32 Optimization Reference Manual
;            "unrolling LOOPs, using CMOVcc instructions instead of Jcc"
;
desPermute PROC USES r9 r9 rbx rcx rdx rsi rdi, __inblock:QWORD, _test:QWORD, _vals:QWORD, __outblock:QWORD
        mov     rsi, QWORD PTR __inblock         ; Init ptr to __inblock
        mov     rdi, QWORD PTR __outblock        ; Init ptr to __outblock
        mov     QWORD PTR [rdi], 0               ; Clear _outblock 

        mov     rax, QWORD PTR _vals             ; ReferenceIP

        lea     rbx, QWORD PTR [rdi+BLOCK_SIZE]  ; End-of __outblock
        lea     rcx, QWORD PTR [rsi+BLOCK_SIZE]  ; End-of __inblock

permute_1:
        mov     rdx, QWORD PTR _test             ; PermutationIP
        push    rcx                              ; Save end-of __inblock
        push    rdi                              ; Save start-of __outblock  

        ALIGN 16

permute_2:                                        ; - Performance optimized -
        movsx   ecx, BYTE PTR [rsi]               ; Data from __inblock
        lea     rdx, QWORD PTR [rdx+SIZEOF DWORD] ; PermutationIP
        and     ecx, DWORD PTR [rdx-SIZEOF DWORD] ; ZR: ecx=0, NZ ecx=data (Perm / RefIP)

        ;~~~~~~~~~~~~~~~~~~~~~~~~~~~
        cmovnbe ecx, DWORD PTR [rax]            ; .686 NZ: Prepare data for __outblock (performance)
        ;~~~~~~~~~~~~~~~~~~~~~~~~~~~
        or      BYTE PTR [rdi], cl              ; ZR/NZ: Apply to __outblock data

        inc     rdi                             ; Advance __outblock
        cmp     rdi, rbx                        ; End-of __outblock?
        jb      SHORT permute_2

        pop     rdi                             ; Restore start-of __outblock
        pop     rcx                             ; Restore end-of __inblock
        add     rax, SIZEOF DWORD               ; ReferenceIP (_vals)
        inc     rsi                             ; Advance __inblock
        cmp     rsi, rcx                        ; End-of __inblock?
        jb      SHORT permute_1                 ; "loop permute_1"

        ret
desPermute ENDP ; permute                        


;+----------------------------------------------------------------------------
;
;                               p48init
;
p48init PROC USES r8 r9 rbx rcx rdx rsi rdi
        xor     rdi, rdi                ; Init index counter

        ALIGN 16

p48init_1:
        mov     rcx, rdi               

        mov     rax, rdi
        and     rax, 00000800h
        shr     rcx, 1                 
        mov     rsi, rcx
        mov     rdx, rdi
        and     rsi, 000003C0h 
        and     rdx, 00000001h
        or      rsi, rax
        shl     rdx, 4

        mov     rax, rdi
        shr     rsi, 4
        and     rax, 00000040h
        and     rcx, 0000000Fh                                  
        or      rsi, rax
        or      rdx, rcx               

        mov     rax, rdi
        shr     rsi, 2
        and     rax, 00000020h                                  
        or      rdx, rax

        movzx   eax, BYTE PTR SBox2[rdx]        ; Merge SBox1 & SBox2
        or      al, BYTE PTR SBox1[rsi]
        mov     BYTE PTR SBoxArray12[rdi], al

        movzx   eax, BYTE PTR SBox4[rdx]        ; Merge SBox3 & SBox4
        or      al, BYTE PTR SBox3[rsi]
        mov     BYTE PTR SBoxArray34[rdi], al

        movzx   eax, BYTE PTR SBox6[rdx]        ; Merge SBox5 & SBox6
        or      al, BYTE PTR SBox5[rsi]
        mov     BYTE PTR SBoxArray56[rdi], al

        movzx   eax, BYTE PTR SBox8[rdx]        ; Merge SBox7 & SBox8
        or      al, BYTE PTR SBox7[rsi]
        mov     BYTE PTR SBoxArray78[rdi], al

        inc     rdi                             ; Advance SBoxArray merge index 
        cmp     rdi, SBOX_ARRAY_SIZE            ; 4096
        jl      p48init_1

;------------------------------------------------------------------------------
;
;       PermutationArray48a-d dispatcher
;
        xor     rbx, rbx                ; Jump table start index

p48initDispatch:                        ; Fill PermutationArrays
        cmp     rbx, 3
        ja      p48xinit_exit                   ; Done.
        jmp     QWORD PTR p48xinit_jmp[rbx*SIZEOF QWORD]        

p48xinit_jmp:                     
        DQ      InitArray48a            ; Jump table
        DQ      InitArray48b
        DQ      InitArray48c
        DQ      InitArray48d

InitArray48a:
        lea     rax, OFFSET PermutationArray48a ; p48a
        mov     DWORD PTR _cmp48x, 0
        jmp     SHORT p48xinit

InitArray48b:
        lea     rax, OFFSET PermutationArray48b ; p48b
        mov     DWORD PTR _cmp48x, 8
        jmp     SHORT p48xinit

InitArray48c:
        lea     rax, OFFSET PermutationArray48c ; p48c
        mov     DWORD PTR _cmp48x, 16
        jmp     SHORT p48xinit

InitArray48d:
        lea     rax, OFFSET PermutationArray48d ; p48d
        mov     DWORD PTR _cmp48x, 24
        jmp     SHORT p48xinit


;------------------------------------------------------------------------------
;
;               Init PermutationArray48a-d
;
p48xinit:
        push    rbx             ; Save jmp table index from dispatcher
        mov     rbx, rax        ; Get pointer to PermutationArray48x 

        xor     rcx, rcx        ; Init array index count

        ALIGN   16

p48xinit_2:
        mov     QWORD PTR [rbx][rcx*8], 0     ; Clear next chunk

        xor     rdx, rdx

p48xinit_3:
        movzx   rsi, BYTE PTR Permutation48i[rdx]
        mov     eax, esi
        and     eax, 0FFFFFFF8h                                 
        cmp     eax, DWORD PTR _cmp48x         ; 0,8,16,24
        jne     SHORT @F

        and     rsi, 7
        test    DWORD PTR NotationTable8Bit[rsi*4], ecx
        je      SHORT @F

        mov     eax, DWORD PTR NotationTable48Bit[rdx*4]
        or      DWORD PTR [rbx][rcx*8], eax
@@:
        inc     rdx
        cmp     rdx, NOTATION_48BIT_SIZE / SIZEOF DWORD
        jl      SHORT p48xinit_3

        mov     rdi, NOTATION_48BIT_SIZE / SIZEOF DWORD
        mov     rsi, OFFSET NotationTable48Bit

p48xinit_4:
        movzx   edx, BYTE PTR Permutation48i[rdi]
        mov     eax, edx
        and     eax, 0FFFFFFF8h                                 
        cmp     eax, DWORD PTR _cmp48x         ; 0,8,16,24
        jne     SHORT @F

        and     rdx, 7
        test    DWORD PTR NotationTable8Bit[rdx*4], ecx
        je      SHORT @F

        mov     eax, DWORD PTR [rsi]
        or      DWORD PTR [rbx][rcx*8+4], eax
@@:
        add     rsi, SIZEOF DWORD
        inc     rdi
                                                
        lea     r9,  OFFSET NotationTable48Bit  ;+NOTATION_48BIT_SIZE
        add     r9,  NOTATION_48BIT_SIZE
        cmp     rsi, r9
        jl      SHORT p48xinit_4

        inc     ecx
        cmp     ecx, 256
        jl      p48xinit_2                      

        pop     rbx                             ; Restore jmp table index
        inc     rbx                             ; Advance jmp table entry
        jmp     p48initDispatch                 ; Init next p48Array

p48xinit_exit:
        mov     DWORD PTR IgnitedDES, 1 ; Tell the outside world we're done

p48xinit_ret:
        ret     0
p48init ENDP ; p48init

;------------------------------------------------------------------------------

_TEXT   ENDS
        END

;------------------------------------------------------------------------------
