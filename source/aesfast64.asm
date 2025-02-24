;; aesfast64.asm - MASM Developer source file.
;; (c)2021 by helmut altmann

;/*
; *  FIPS-197 compliant AES implementation
; *
; *  Copyright (C) 2003-2006  Christophe Devine
; *
; *  This library is free software; you can redistribute it and/or
; *  modify it under the terms of the GNU Lesser General Public
; *  License, version 2.1 as published by the Free Software Foundation.
; *
; *  This library is distributed in the hope that it will be useful,
; *  but WITHOUT ANY WARRANTY; without even the implied warranty of
; *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; *  Lesser General Public License for more details.
; *
; *  You should have received a copy of the GNU Lesser General Public
; *  License along with this library; if not, write to the Free Software
; *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
; *  MA  02110-1301  USA
; *
; *  The AES block cipher was designed by Vincent Rijmen and Joan Daemen.
; *
; *  http://csrc.nist.gov/encryption/aes/rijndael/Rijndael.pdf
; *  http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
; */
;
; Further development (2021 ha):
;  Refurbished 2021, using Visual Studio VS C++ 2019 (Windows 10)
;  and transscribed into assembling language by helmut altmann for use with the
;  Microsoft (R) Macro Assembler (x64) Version 14.28.29336.0
;
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
 
;;ha;;.MODEL FLAT
;;ha;;  .686P
;;ha;;  .XMM

include aesfast64.inc
;------------------------------------------------------------------------------
; File c:\temp600\__\aes.cpp
; Function compile flags: /Odtp                                

;
; Supported AES Algorithm Modes
;
ENCRYPT  EQU    0 ; Encrypts a block of plain text (see desmain.cpp)
DECIPHER EQU    1 ; Deciphers a block of encrypted text (see aesfastmain.cpp)

;------------------------------------------------------------------------------
_BSS SEGMENT
kt_init DD 1 DUP (0)    ; aesSetKey, aesGenerateTables (Performance)
aesMode DD 1 DUP (?)    ; ENCRYPT or DECIPHER

ctxAesContext  DB (SIZEOF _ctxStruct) DUP (?)  ; (2*256)+4 DUP (?)

_SIZE_SBOX        EQU 256
_SIZE_TABLE       EQU 256
_SIZE_ROUND_CONST EQU  10

_KT3    DWORD   _SIZE_TABLE DUP (?)
_KT2    DWORD   _SIZE_TABLE DUP (?)
_KT1    DWORD   _SIZE_TABLE DUP (?)
_KT0    DWORD   _SIZE_TABLE DUP (?)

_FT3    DWORD   _SIZE_TABLE DUP (?)
_FT2    DWORD   _SIZE_TABLE DUP (?)
_FT1    DWORD   _SIZE_TABLE DUP (?)
_FT0    DWORD   _SIZE_TABLE DUP (?)

_RT0    DWORD   _SIZE_TABLE DUP (?)
_RT1    DWORD   _SIZE_TABLE DUP (?)
_RT2    DWORD   _SIZE_TABLE DUP (?)
_RT3    DWORD   _SIZE_TABLE DUP (?)

_FSb    DWORD   _SIZE_SBOX DUP (?)
_RSb    DWORD   _SIZE_SBOX DUP (?)

_RCON   DWORD   _SIZE_ROUND_CONST DUP (?)
_BSS    ENDS

_TEXT SEGMENT

;------------------------------------------------------------------------------
;
;                       aesGenerateTables
;
;               COMDAT ?aesGenerateTables@@YAXXZ
;
;   Change: Removed usage of [ebp] to allow "INVOKE" addressing technique.
;           (the optimizing C++ Compiler foces direct [esp] addresiing
;            to gain [ebp] as an additional multipurpose register) 
;
aesGenerateTables PROC C
        LOCAL   _log[_SIZE_TABLE]:BYTE       ; uint8 log[256];
        LOCAL   _pow[_SIZE_TABLE]:BYTE       ; uint8 pow[256];
        LOCAL   _tv1:DWORD           
        LOCAL   _tv2:DWORD           
        LOCAL   _tv3:DWORD
        LOCAL   __i:DWORD            ; int i;
        LOCAL   __x:BYTE             ; uint8 x, y;
        LOCAL   __y:BYTE

; // compute pow and log tables over GF(2^8), Modulo 2, so that 1+1=0
;
; https://www.samiam.org/galois.html
; Rijndael's Galois Field GF(2) only allows an 8 bit number (0 to 255)
; Addition and subtraction are performed by the exclusive or operation.
;  The two operations are the same.
;  There is no difference between addition and subtraction.
;
; Multiplication in Rijndael's Galois Field is a little more complicated.
; The procedure is as follows:
;
; Take two eight-bit numbers, x and y, and an 8bit product p
; - Set the product to zero.
; - Make a copy of x and y, which we will simply call x and y
;    in the rest of this algorithm.
; - Run the following loop eight times:
; 1. If the low bit of y is set, XOR the product p by the value of x
; 2. Keep track of whether the high (eighth from left) bit of x is set to 1
; 3. Rotate x one bit to the left, discarding the high bit,
;     and making the low bit have a value of zero
; 4. If x's hi bit had a value of 1 prior to this rotation,
;     XOR x with the hexadecimal number 0x1B (27)
; 5. Rotate y one bit to the right, discarding the low bit,
;     and making the high (eighth from left) bit have a value of zero. 
; - The product p now has the product of x and y
;
; Example: x=7, y=3, p=0
; 1. Low bit of y is one. Product is made 7 as a result
;    x, which is 7 (0x07), is rotated one bit to the left.
;     This makes x have a value of 0x0E (14)
;    The high bit of x is not set (x is below 128),
;     so x is not XORed with 0x1B (27)
;    y is rotated one bit to the right. y now has a value of 1 (0x01)
; 2. Low bit of y is 1. Product, which was 7, is made 7 XOR 0x0E (14),
;     which has a value of 9.
;    x, which is 14, is rotate one bit to the left.
;     This makes x have a value of 0x1C (28)
;    The high bit of x is not set (x is below 0x80 (128),
;     so x is not XORed with 0x1B (27)
;    y is rotated one bit to the right. y now has a value of zero 
;
; While there are six more steps, none of them will affect the product p,
; so we will discard them in this example. Note that, if this method
; of multiplying two numbers together is used in the real world,
; the six steps should be performed in order to protect key information
; from being leaked via a timing attack.
; The final product is p=0x09. 
;
; Exponents and logarithms
; Exponentiation is done by repeated multiplication of the same number.
; With some, but not all, numbers in Rijndael's Galois Field,
; it is possible to traverse all possible values in the galois field
; except zero via exponentiation. Numbers for which this is possible
; are called generators. Rijndael's galois field has the following generators:
;  03 05 06 09 0B 0E 11 12 13 14 17 18 19 1A 1C 1E 
;  1F 21 22 23 27 28 2A 2C 30 31 3C 3E 3F 41 45 46 
;  47 48 49 4B 4C 4E 4F 52 54 56 57 58 59 5A 5B 5F 
;  64 65 68 69 6D 6E 70 71 76 77 79 7A 7B 7E 81 84 
;  86 87 88 8A 8E 8F 90 93 95 96 98 99 9B 9D A0 A4 
;  A5 A6 A7 A9 AA AC AD B2 B4 B7 B8 B9 BA BE BF C0 
;  C1 C4 C8 C9 CE CF D0 D6 D7 DA DC DD DE E2 E3 E5 
;  E6 E7 E9 EA EB EE F0 F1 F4 F5 F6 F8 FB FD FE FF
; When any of these numbers is exponentiated multiple times,
;  the original number is reached again after 255 exponentiations.
;
; Using the log[] table to more quickly multiply numbers
; https://www.samiam.org/galois.html
;  Look up 0x03 on the log table. We get 0x08
;  Look up 0x07 on the log table. We get 0x36
;  Add up these two numbers together (using normal, not Galois Field, addition)
;   mod 255. 0x08 + 0x36 = 0x3e
;  Look up the sum, 0x3e, on the exponentiation table. We get 0x09. 
;
; Division
; Dividing x by y in Rijndael's Galois Field is performed by taking 
; the logarithm of x and subtracting the logarithm of y from it, modulo 255.
; In particular, when x (the numerator) is 1, division is done by taking
; the logarithm of x, which can be represented as the number 255,
; and subtracting the logarithm of y from 255. 
;
; Multiplicative inverse
; 1 divided by a given number is the multiplicative inverse of that number.
; To find the multiplicative inverse of the number x:
;    Find the logarithm for x
;    Subtract 255 by x's logarithm
;    Take the anti-log of the resulting number
;    This is the multiplicative inverse (In other words, 1/x) 
; Example:
; Code using log and anti-log tables to calculate the multiplicative inverse: 
; unsigned char gmul_inverse(unsigned char x)
;   {
;   if(x == 0) return 0;                 // 0 is self inverting
;   else return pow[(255 - log[x])];
;   }

; ---------------
; Generator: 0x03      GF(256)
; ---------------
; log[] table:
;    | 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
; ---|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
; 00 |-- 00 19 01 32 02 1a c6 4b c7 1b 68 33 ee df 03 
; 10 |64 04 e0 0e 34 8d 81 ef 4c 71 08 c8 f8 69 1c c1 
; 20 |7d c2 1d b5 f9 b9 27 6a 4d e4 a6 72 9a c9 09 78 
; 30 |65 2f 8a 05 21 0f e1 24 12 f0 82 45 35 93 da 8e 
; 40 |96 8f db bd 36 d0 ce 94 13 5c d2 f1 40 46 83 38 
; 50 |66 dd fd 30 bf 06 8b 62 b3 25 e2 98 22 88 91 10 
; 60 |7e 6e 48 c3 a3 b6 1e 42 3a 6b 28 54 fa 85 3d ba 
; 70 |2b 79 0a 15 9b 9f 5e ca 4e d4 ac e5 f3 73 a7 57 
; 80 |af 58 a8 50 f4 ea d6 74 4f ae e9 d5 e7 e6 ad e8 
; 90 |2c d7 75 7a eb 16 0b f5 59 cb 5f b0 9c a9 51 a0 
; a0 |7f 0c f6 6f 17 c4 49 ec d8 43 1f 2d a4 76 7b b7 
; b0 |cc bb 3e 5a fb 60 b1 86 3b 52 a1 6c aa 55 29 9d 
; c0 |97 b2 87 90 61 be dc fc bc 95 cf cd 37 3f 5b d1 
; d0 |53 39 84 3c 41 a2 6d 47 14 2a 9e 5d 56 f2 d3 ab 
; e0 |44 11 92 d9 23 20 2e 89 b4 7c b8 26 77 99 e3 a5 
; f0 |67 4a ed de c5 31 fe 18 0d 63 8c 80 c0 f7 70 07 
; 
; pow[] table:
;    | 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
; ---|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
; 00 |01 03 05 0f 11 33 55 ff 1a 2e 72 96 a1 f8 13 35 
; 10 |5f e1 38 48 d8 73 95 a4 f7 02 06 0a 1e 22 66 aa 
; 20 |e5 34 5c e4 37 59 eb 26 6a be d9 70 90 ab e6 31 
; 30 |53 f5 04 0c 14 3c 44 cc 4f d1 68 b8 d3 6e b2 cd 
; 40 |4c d4 67 a9 e0 3b 4d d7 62 a6 f1 08 18 28 78 88 
; 50 |83 9e b9 d0 6b bd dc 7f 81 98 b3 ce 49 db 76 9a 
; 60 |b5 c4 57 f9 10 30 50 f0 0b 1d 27 69 bb d6 61 a3 
; 70 |fe 19 2b 7d 87 92 ad ec 2f 71 93 ae e9 20 60 a0 
; 80 |fb 16 3a 4e d2 6d b7 c2 5d e7 32 56 fa 15 3f 41 
; 90 |c3 5e e2 3d 47 c9 40 c0 5b ed 2c 74 9c bf da 75 
; a0 |9f ba d5 64 ac ef 2a 7e 82 9d bc df 7a 8e 89 80 
; b0 |9b b6 c1 58 e8 23 65 af ea 25 6f b1 c8 43 c5 54 
; c0 |fc 1f 21 63 a5 f4 07 09 1b 2d 77 99 b0 cb 46 ca 
; d0 |45 cf 4a de 79 8b 86 91 a8 e3 3e 42 c6 51 f3 0e 
; e0 |12 36 5a ee 29 7b 8d 8c 8f 8a 85 94 a7 f2 0d 17 
; f0 |39 4b dd 7c 84 97 a2 fd 1c 24 6c b4 c7 52 f6 01
 
        ; XTIME(x) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) )
        ; for( i = 0, x = 1; i < 256; i++, x ^= XTIME( x ) )
        mov      BYTE PTR __x, 1        ; 255 entries in log/pow table
        xor      rcx, rcx               ; i = 0;
        
aesGenTab_1:                                          ; Loop #1      ; Loop #2      ; Loop #3
        movzx    rax, BYTE PTR __x                    ; -----------  ; -----------  ; -----------
        mov      BYTE PTR _pow[rcx], al ; pow[i] = x;   Pow[0]=0x01    Pow[1]=0x03    Pow[2]=0x05
        mov      BYTE PTR _log[rax], cl ; log[x] = i;   log[1]=0x00    log[3]=0x01    log[5]=0x02
        mov      edx, eax                             ;  edx = 0x01  ;  edx = 0x03  ;  edx = 0x05
        shl      edx, 1                               ;  edx = 0x02  ;  edx = 0x06  ;  edx = 0x0A
        and      eax, 80h                             ;  eax = 0x00  ;  eax = 0x00  ;  eax = 0x00
        neg      eax       ; 0-eax, CF=1 if eax != 0  ;  eax = 0x00  ;  eax = 0x00  ;  eax = 0x00
        sbb      eax, eax  ; DEST := (DEST-(SRC+CF))  ;  eax = 0x00  ;  eax = 0x00  ;  eax = 0x00
        and      eax, 1Bh
        xor      edx, eax                             ;  edx = 0x02  ;  edx = 0x06  ;  edx = 0x0A
        movzx    eax, BYTE PTR __x                    ;  eax = 0x01  ;  eax = 0x03  ;  eax = 0x05
        xor      eax, edx                             ;  eax = 0x03  ;  eax = 0x05  ;  eax = 0x0F
        mov      BYTE PTR __x, al                     ;  __x = 0x03  ;  __x = 0x05  ;  __x = 0x0F

        inc      ecx                    ; i++;        ;  ecx = 0x01  ;  ecx = 0x02  ;  ecx = 0x03
        cmp      ecx, _SIZE_TABLE       ; =256
        jb       SHORT aesGenTab_1
  
;// Round constants (fixed by ;;ha;;)
;static const uint32 RCON[10] =
;{
;    0x01000000, 0x02000000, 0x04000000, 0x08000000,    // (01000000 << 1), ..
;    0x10000000, 0x20000000, 0x40000000, 0x80000000,    // (10000000 << 1), .. 
;    0x1B000000, 0x36000000                             //  1B000000, (1B000000 << 1)
;};
        ; // calculate the round constants
        ; RCON[i] = (uint32) x << 24;
        ; XTIME(x) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) ); // ??
        ; for( i = 0, x = 1; i < 10; i++, x = XTIME( x ) )
        mov      BYTE PTR __x, 1
        xor      rcx, rcx               ; i=0;

aesGenTab_2:                                            ; Loop #0     ; Loop #7    ; Loop #8    ; Loop #9                          
        movzx    eax, BYTE PTR __x                      ; ---------   ; ---------  ; ---------  ; ---------
        shl      eax, 24                                ;             ;            ;            ;
        mov      DWORD PTR _RCON[rcx*SIZEOF DWORD], eax ; 01000000h   ; 80000000h  ; 1B000000h  ; 36000000h
        movzx    eax, BYTE PTR __x                      ; 00000001h   ; 00000080h  ; 0000001Bh  ; 00000036h
        mov      edx, eax
        shl      eax, 1                                 ; 00000002h   ; 00000100h  ; 00000036h  ; 0000006Ch
        and      edx, 80h                               ; 00000000h   ; 00000080h  ; 00000000h  ; end-of-loop
        neg      edx      ; 0-edx, CF=1 if edx != 0     ; 00000000h   ; FFFFFF80h  ; 00000000h  
        and      edx, 1Bh SHL 24                        ; 00000000h   ; 1B000000h  ; 00000000h  
        xor      eax, edx                               ; 00000002h   ; 1B000100h  ; 00000036h  
        test     eax, 0FFh SHL 24
        jz       @F
        bswap    eax                                                  ; 0001001Bh  
@@:     mov      BYTE PTR __x, al                       ; 00000002h   ; 0001001Bh  ; 00000036h

        inc      ecx                    ; i++;
        cmp      ecx, _SIZE_ROUND_CONST ; =10
        jb       SHORT aesGenTab_2

;// Forward S-box
;static const uint8 FSb[256] =
;{
;    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5,   ; [00]..[07] -> [00]=0x63
;    0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,   ; [08]..[0F]
;    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0,
;    0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
;    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC,
;    0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
;    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A,
;    0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
;    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0,
;    0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
;    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B,
;    0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
;    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85,
;    0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
;    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5,
;    0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
;    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17,
;    0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
;    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88,
;    0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
;    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C,
;    0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
;    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9,
;    0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
;    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6,
;    0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
;    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
;    0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
;    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94,
;    0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
;    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68,
;    0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
;};

;// Reverse S-box
;static const uint8 RSb[256] =
;{
;    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38,   ; [00]..[07]
;    0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,   ; [08]..[0F]
;    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87,   ; [10]..[17]
;    0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,   ; [18]..[1F]
;    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D,   ; [20]..[37]
;    0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,   ; [28]..[3F]
;    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2,   ; [30]..[47]
;    0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,   ; [38]..[4F]
;    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16,   ; [40]..[37]
;    0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,   ; [48]..[3F]
;    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA,   ; [50]..[47]
;    0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,   ; [58]..[4F]
;    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A,   ; [60]..[67] -> [63]=0x00
;    0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,   ; [68]..[6F]
;    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02,
;    0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
;    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA,   
;    0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,   
;    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85,   
;    0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,   
;    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89,
;    0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
;    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20,
;    0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
;    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31,   
;    0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,   
;    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D,   
;    0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,   
;    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0,
;    0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
;    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26,
;    0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
;};
        ; // generate the forward and reverse S-boxes
        mov      BYTE PTR _FSb[00h], 63h ; FSb[0x00] = 0x63;
        mov      BYTE PTR _RSb[63h], 0   ; RSb[0x63] = 0x00;

        ; for( i = 1; i < 256; i++ )
        mov      DWORD PTR __i, 1         

aesGenTab_3:                                                            
        ; x = pow[(255 - log[i])];
        mov      ecx, DWORD PTR __i
        and      rcx, 0FFFFFFFFh
        movzx    rax, BYTE PTR _log[rcx]
        mov      rcx, 255
        sub      rcx, rax
        movzx    rax, BYTE PTR _pow[rcx]
         

        ; y = x; y = ( y << 1 ) | ( y >> 7 );
        mov      BYTE PTR __x, al       
        mov      BYTE PTR __y, al
        mov      edx, eax         ; y = x
        shl      eax, 1
        sar      edx, 7           ; SAR instruction fills the empty bit position’s              
        or       eax, edx         ;  shifted value with the sign of the unshifted value
        mov      BYTE PTR __y, al

        mov      ecx, 3           ; The same to be performed 3 times
@@:     ; x ^= y; y = ( y << 1 ) | ( y >> 7 );
        movzx    edx, BYTE PTR __x
        movzx    eax, BYTE PTR __y
        xor      edx, eax
        mov      BYTE PTR __x, dl
        mov      edx, eax
        shl      eax, 1
        sar      edx, 7
        or       eax, edx
        mov      BYTE PTR __y, al
        loop     @B

        ; x ^= y ^ 0x63;
        movzx    edx, BYTE PTR __x
        movzx    eax, BYTE PTR __y
        xor      eax, 63h
        xor      edx, eax
        mov      BYTE PTR __x, dl

        ; FSb[i] = x;
        mov      al, BYTE PTR __x
        mov      ecx, DWORD PTR __i
        and      rcx, 0FFFFFFFFh
        mov      BYTE PTR _FSb[rcx], al        ; Forward S-box

        ; RSb[x] = i;
        mov      al, BYTE PTR __i
        movzx    rcx, BYTE PTR __x
        mov      BYTE PTR _RSb[rcx], al        ; Reverse S-box

        inc      DWORD PTR __i
        cmp      DWORD PTR __i, _SIZE_TABLE    ; =256
        jb       aesGenTab_3

;// Forward tables
;#define FT \
;\
;    V(C6,63,63,A5), V(F8,7C,7C,84), V(EE,77,77,99), V(F6,7B,7B,8D), \           // FT0 
;    V(FF,F2,F2,0D), V(D6,6B,6B,BD), V(DE,6F,6F,B1), V(91,C5,C5,54), \                  
;    V(60,30,30,50), V(02,01,01,03), V(CE,67,67,A9), V(56,2B,2B,7D), \                  
;    V(E7,FE,FE,19), V(B5,D7,D7,62), V(4D,AB,AB,E6), V(EC,76,76,9A), \                  
;    V(8F,CA,CA,45), V(1F,82,82,9D), V(89,C9,C9,40), V(FA,7D,7D,87), \                  
;    V(EF,FA,FA,15), V(B2,59,59,EB), V(8E,47,47,C9), V(FB,F0,F0,0B), \                  
;    V(41,AD,AD,EC), V(B3,D4,D4,67), V(5F,A2,A2,FD), V(45,AF,AF,EA), \                  
;    V(23,9C,9C,BF), V(53,A4,A4,F7), V(E4,72,72,96), V(9B,C0,C0,5B), \                  
;    V(75,B7,B7,C2), V(E1,FD,FD,1C), V(3D,93,93,AE), V(4C,26,26,6A), \                  
;    V(6C,36,36,5A), V(7E,3F,3F,41), V(F5,F7,F7,02), V(83,CC,CC,4F), \                  
;    V(68,34,34,5C), V(51,A5,A5,F4), V(D1,E5,E5,34), V(F9,F1,F1,08), \                  
;    V(E2,71,71,93), V(AB,D8,D8,73), V(62,31,31,53), V(2A,15,15,3F), \                  
;    V(08,04,04,0C), V(95,C7,C7,52), V(46,23,23,65), V(9D,C3,C3,5E), \                  
;    V(30,18,18,28), V(37,96,96,A1), V(0A,05,05,0F), V(2F,9A,9A,B5), \                  
;    V(0E,07,07,09), V(24,12,12,36), V(1B,80,80,9B), V(DF,E2,E2,3D), \                  
;    V(CD,EB,EB,26), V(4E,27,27,69), V(7F,B2,B2,CD), V(EA,75,75,9F), \                  
;
;    V(12,09,09,1B), V(1D,83,83,9E), V(58,2C,2C,74), V(34,1A,1A,2E), \           // FT1 
;    V(36,1B,1B,2D), V(DC,6E,6E,B2), V(B4,5A,5A,EE), V(5B,A0,A0,FB), \                  
;    V(A4,52,52,F6), V(76,3B,3B,4D), V(B7,D6,D6,61), V(7D,B3,B3,CE), \                  
;    V(52,29,29,7B), V(DD,E3,E3,3E), V(5E,2F,2F,71), V(13,84,84,97), \                  
;    V(A6,53,53,F5), V(B9,D1,D1,68), V(00,00,00,00), V(C1,ED,ED,2C), \                  
;    V(40,20,20,60), V(E3,FC,FC,1F), V(79,B1,B1,C8), V(B6,5B,5B,ED), \                  
;    V(D4,6A,6A,BE), V(8D,CB,CB,46), V(67,BE,BE,D9), V(72,39,39,4B), \                  
;    V(94,4A,4A,DE), V(98,4C,4C,D4), V(B0,58,58,E8), V(85,CF,CF,4A), \                  
;    V(BB,D0,D0,6B), V(C5,EF,EF,2A), V(4F,AA,AA,E5), V(ED,FB,FB,16), \                  
;    V(86,43,43,C5), V(9A,4D,4D,D7), V(66,33,33,55), V(11,85,85,94), \                  
;    V(8A,45,45,CF), V(E9,F9,F9,10), V(04,02,02,06), V(FE,7F,7F,81), \                  
;    V(A0,50,50,F0), V(78,3C,3C,44), V(25,9F,9F,BA), V(4B,A8,A8,E3), \                  
;    V(A2,51,51,F3), V(5D,A3,A3,FE), V(80,40,40,C0), V(05,8F,8F,8A), \                  
;    V(3F,92,92,AD), V(21,9D,9D,BC), V(70,38,38,48), V(F1,F5,F5,04), \                  
;    V(63,BC,BC,DF), V(77,B6,B6,C1), V(AF,DA,DA,75), V(42,21,21,63), \                  
;    V(20,10,10,30), V(E5,FF,FF,1A), V(FD,F3,F3,0E), V(BF,D2,D2,6D), \                  
;    
;    V(81,CD,CD,4C), V(18,0C,0C,14), V(26,13,13,35), V(C3,EC,EC,2F), \           // FT2 
;    V(BE,5F,5F,E1), V(35,97,97,A2), V(88,44,44,CC), V(2E,17,17,39), \                  
;    V(93,C4,C4,57), V(55,A7,A7,F2), V(FC,7E,7E,82), V(7A,3D,3D,47), \                  
;    V(C8,64,64,AC), V(BA,5D,5D,E7), V(32,19,19,2B), V(E6,73,73,95), \                  
;    V(C0,60,60,A0), V(19,81,81,98), V(9E,4F,4F,D1), V(A3,DC,DC,7F), \                  
;    V(44,22,22,66), V(54,2A,2A,7E), V(3B,90,90,AB), V(0B,88,88,83), \                  
;    V(8C,46,46,CA), V(C7,EE,EE,29), V(6B,B8,B8,D3), V(28,14,14,3C), \                  
;    V(A7,DE,DE,79), V(BC,5E,5E,E2), V(16,0B,0B,1D), V(AD,DB,DB,76), \                  
;    V(DB,E0,E0,3B), V(64,32,32,56), V(74,3A,3A,4E), V(14,0A,0A,1E), \                  
;    V(92,49,49,DB), V(0C,06,06,0A), V(48,24,24,6C), V(B8,5C,5C,E4), \                  
;    V(9F,C2,C2,5D), V(BD,D3,D3,6E), V(43,AC,AC,EF), V(C4,62,62,A6), \                  
;    V(39,91,91,A8), V(31,95,95,A4), V(D3,E4,E4,37), V(F2,79,79,8B), \                  
;    V(D5,E7,E7,32), V(8B,C8,C8,43), V(6E,37,37,59), V(DA,6D,6D,B7), \                  
;    V(01,8D,8D,8C), V(B1,D5,D5,64), V(9C,4E,4E,D2), V(49,A9,A9,E0), \                  
;    V(D8,6C,6C,B4), V(AC,56,56,FA), V(F3,F4,F4,07), V(CF,EA,EA,25), \                  
;    V(CA,65,65,AF), V(F4,7A,7A,8E), V(47,AE,AE,E9), V(10,08,08,18), \                  
;
;    V(6F,BA,BA,D5), V(F0,78,78,88), V(4A,25,25,6F), V(5C,2E,2E,72), \           // FT3 
;    V(38,1C,1C,24), V(57,A6,A6,F1), V(73,B4,B4,C7), V(97,C6,C6,51), \                  
;    V(CB,E8,E8,23), V(A1,DD,DD,7C), V(E8,74,74,9C), V(3E,1F,1F,21), \                  
;    V(96,4B,4B,DD), V(61,BD,BD,DC), V(0D,8B,8B,86), V(0F,8A,8A,85), \                  
;    V(E0,70,70,90), V(7C,3E,3E,42), V(71,B5,B5,C4), V(CC,66,66,AA), \                  
;    V(90,48,48,D8), V(06,03,03,05), V(F7,F6,F6,01), V(1C,0E,0E,12), \                  
;    V(C2,61,61,A3), V(6A,35,35,5F), V(AE,57,57,F9), V(69,B9,B9,D0), \                  
;    V(17,86,86,91), V(99,C1,C1,58), V(3A,1D,1D,27), V(27,9E,9E,B9), \                  
;    V(D9,E1,E1,38), V(EB,F8,F8,13), V(2B,98,98,B3), V(22,11,11,33), \                  
;    V(D2,69,69,BB), V(A9,D9,D9,70), V(07,8E,8E,89), V(33,94,94,A7), \                  
;    V(2D,9B,9B,B6), V(3C,1E,1E,22), V(15,87,87,92), V(C9,E9,E9,20), \                  
;    V(87,CE,CE,49), V(AA,55,55,FF), V(50,28,28,78), V(A5,DF,DF,7A), \                  
;    V(03,8C,8C,8F), V(59,A1,A1,F8), V(09,89,89,80), V(1A,0D,0D,17), \                  
;    V(65,BF,BF,DA), V(D7,E6,E6,31), V(84,42,42,C6), V(D0,68,68,B8), \                  
;    V(82,41,41,C3), V(29,99,99,B0), V(5A,2D,2D,77), V(1E,0F,0F,11), \                  
;    V(7B,B0,B0,CB), V(A8,54,54,FC), V(6D,BB,BB,D6), V(2C,16,16,3A)                             

;// Reverse tables
;#define RT \
;\
;    V(51,F4,A7,50), V(7E,41,65,53), V(1A,17,A4,C3), V(3A,27,5E,96), \           // RT0         
;    V(3B,AB,6B,CB), V(1F,9D,45,F1), V(AC,FA,58,AB), V(4B,E3,03,93), \                          
;    V(20,30,FA,55), V(AD,76,6D,F6), V(88,CC,76,91), V(F5,02,4C,25), \                          
;    V(4F,E5,D7,FC), V(C5,2A,CB,D7), V(26,35,44,80), V(B5,62,A3,8F), \                          
;    V(DE,B1,5A,49), V(25,BA,1B,67), V(45,EA,0E,98), V(5D,FE,C0,E1), \                          
;    V(C3,2F,75,02), V(81,4C,F0,12), V(8D,46,97,A3), V(6B,D3,F9,C6), \                          
;    V(03,8F,5F,E7), V(15,92,9C,95), V(BF,6D,7A,EB), V(95,52,59,DA), \                          
;    V(D4,BE,83,2D), V(58,74,21,D3), V(49,E0,69,29), V(8E,C9,C8,44), \                          
;    V(75,C2,89,6A), V(F4,8E,79,78), V(99,58,3E,6B), V(27,B9,71,DD), \                          
;    V(BE,E1,4F,B6), V(F0,88,AD,17), V(C9,20,AC,66), V(7D,CE,3A,B4), \                          
;    V(63,DF,4A,18), V(E5,1A,31,82), V(97,51,33,60), V(62,53,7F,45), \                          
;    V(B1,64,77,E0), V(BB,6B,AE,84), V(FE,81,A0,1C), V(F9,08,2B,94), \                          
;    V(70,48,68,58), V(8F,45,FD,19), V(94,DE,6C,87), V(52,7B,F8,B7), \                          
;    V(AB,73,D3,23), V(72,4B,02,E2), V(E3,1F,8F,57), V(66,55,AB,2A), \                          
;    V(B2,EB,28,07), V(2F,B5,C2,03), V(86,C5,7B,9A), V(D3,37,08,A5), \                          
;    V(30,28,87,F2), V(23,BF,A5,B2), V(02,03,6A,BA), V(ED,16,82,5C), \                          
;
;    V(8A,CF,1C,2B), V(A7,79,B4,92), V(F3,07,F2,F0), V(4E,69,E2,A1), \           // RT1         
;    V(65,DA,F4,CD), V(06,05,BE,D5), V(D1,34,62,1F), V(C4,A6,FE,8A), \                          
;    V(34,2E,53,9D), V(A2,F3,55,A0), V(05,8A,E1,32), V(A4,F6,EB,75), \                          
;    V(0B,83,EC,39), V(40,60,EF,AA), V(5E,71,9F,06), V(BD,6E,10,51), \                          
;    V(3E,21,8A,F9), V(96,DD,06,3D), V(DD,3E,05,AE), V(4D,E6,BD,46), \                          
;    V(91,54,8D,B5), V(71,C4,5D,05), V(04,06,D4,6F), V(60,50,15,FF), \                          
;    V(19,98,FB,24), V(D6,BD,E9,97), V(89,40,43,CC), V(67,D9,9E,77), \                          
;    V(B0,E8,42,BD), V(07,89,8B,88), V(E7,19,5B,38), V(79,C8,EE,DB), \                          
;    V(A1,7C,0A,47), V(7C,42,0F,E9), V(F8,84,1E,C9), V(00,00,00,00), \                          
;    V(09,80,86,83), V(32,2B,ED,48), V(1E,11,70,AC), V(6C,5A,72,4E), \                          
;    V(FD,0E,FF,FB), V(0F,85,38,56), V(3D,AE,D5,1E), V(36,2D,39,27), \                          
;    V(0A,0F,D9,64), V(68,5C,A6,21), V(9B,5B,54,D1), V(24,36,2E,3A), \                          
;    V(0C,0A,67,B1), V(93,57,E7,0F), V(B4,EE,96,D2), V(1B,9B,91,9E), \                          
;    V(80,C0,C5,4F), V(61,DC,20,A2), V(5A,77,4B,69), V(1C,12,1A,16), \                          
;    V(E2,93,BA,0A), V(C0,A0,2A,E5), V(3C,22,E0,43), V(12,1B,17,1D), \                          
;    V(0E,09,0D,0B), V(F2,8B,C7,AD), V(2D,B6,A8,B9), V(14,1E,A9,C8), \                          
;
;    V(57,F1,19,85), V(AF,75,07,4C), V(EE,99,DD,BB), V(A3,7F,60,FD), \           // RT2         
;    V(F7,01,26,9F), V(5C,72,F5,BC), V(44,66,3B,C5), V(5B,FB,7E,34), \                          
;    V(8B,43,29,76), V(CB,23,C6,DC), V(B6,ED,FC,68), V(B8,E4,F1,63), \                          
;    V(D7,31,DC,CA), V(42,63,85,10), V(13,97,22,40), V(84,C6,11,20), \                          
;    V(85,4A,24,7D), V(D2,BB,3D,F8), V(AE,F9,32,11), V(C7,29,A1,6D), \                          
;    V(1D,9E,2F,4B), V(DC,B2,30,F3), V(0D,86,52,EC), V(77,C1,E3,D0), \                          
;    V(2B,B3,16,6C), V(A9,70,B9,99), V(11,94,48,FA), V(47,E9,64,22), \                          
;    V(A8,FC,8C,C4), V(A0,F0,3F,1A), V(56,7D,2C,D8), V(22,33,90,EF), \                          
;    V(87,49,4E,C7), V(D9,38,D1,C1), V(8C,CA,A2,FE), V(98,D4,0B,36), \                          
;    V(A6,F5,81,CF), V(A5,7A,DE,28), V(DA,B7,8E,26), V(3F,AD,BF,A4), \                          
;    V(2C,3A,9D,E4), V(50,78,92,0D), V(6A,5F,CC,9B), V(54,7E,46,62), \                          
;    V(F6,8D,13,C2), V(90,D8,B8,E8), V(2E,39,F7,5E), V(82,C3,AF,F5), \                          
;    V(9F,5D,80,BE), V(69,D0,93,7C), V(6F,D5,2D,A9), V(CF,25,12,B3), \                          
;    V(C8,AC,99,3B), V(10,18,7D,A7), V(E8,9C,63,6E), V(DB,3B,BB,7B), \                          
;    V(CD,26,78,09), V(6E,59,18,F4), V(EC,9A,B7,01), V(83,4F,9A,A8), \                          
;    V(E6,95,6E,65), V(AA,FF,E6,7E), V(21,BC,CF,08), V(EF,15,E8,E6), \                          
;
;    V(BA,E7,9B,D9), V(4A,6F,36,CE), V(EA,9F,09,D4), V(29,B0,7C,D6), \           // RT3         
;    V(31,A4,B2,AF), V(2A,3F,23,31), V(C6,A5,94,30), V(35,A2,66,C0), \                          
;    V(74,4E,BC,37), V(FC,82,CA,A6), V(E0,90,D0,B0), V(33,A7,D8,15), \                          
;    V(F1,04,98,4A), V(41,EC,DA,F7), V(7F,CD,50,0E), V(17,91,F6,2F), \                          
;    V(76,4D,D6,8D), V(43,EF,B0,4D), V(CC,AA,4D,54), V(E4,96,04,DF), \                          
;    V(9E,D1,B5,E3), V(4C,6A,88,1B), V(C1,2C,1F,B8), V(46,65,51,7F), \                          
;    V(9D,5E,EA,04), V(01,8C,35,5D), V(FA,87,74,73), V(FB,0B,41,2E), \                          
;    V(B3,67,1D,5A), V(92,DB,D2,52), V(E9,10,56,33), V(6D,D6,47,13), \                          
;    V(9A,D7,61,8C), V(37,A1,0C,7A), V(59,F8,14,8E), V(EB,13,3C,89), \                          
;    V(CE,A9,27,EE), V(B7,61,C9,35), V(E1,1C,E5,ED), V(7A,47,B1,3C), \                          
;    V(9C,D2,DF,59), V(55,F2,73,3F), V(18,14,CE,79), V(73,C7,37,BF), \                          
;    V(53,F7,CD,EA), V(5F,FD,AA,5B), V(DF,3D,6F,14), V(78,44,DB,86), \                          
;    V(CA,AF,F3,81), V(B9,68,C4,3E), V(38,24,34,2C), V(C2,A3,40,5F), \                          
;    V(16,1D,C3,72), V(BC,E2,25,0C), V(28,3C,49,8B), V(FF,0D,95,41), \                          
;    V(39,A8,01,71), V(08,0C,B3,DE), V(D8,B4,E4,9C), V(64,56,C1,90), \                          
;    V(7B,CB,84,61), V(D5,32,B6,70), V(48,6C,5C,74), V(D0,B8,57,42)                                     

        ; // generate the forward and reverse tables
        ; for( i = 0; i < 256; i++ )
        mov      DWORD PTR __i, 0

aesGenTab_4:
        ; XTIME(x) ( ( x << 1 ) ^ ( ( x & 0x80 ) ? 0x1B : 0x00 ) );
        ; x = FSb[i]; y = XTIME( x );
        mov      ecx, DWORD PTR __i
        and      rcx, 0FFFFFFFFh
        movzx    rcx, BYTE PTR _FSb[rcx]
        mov      BYTE PTR __x, cl
        mov      eax, ecx
        and      ecx, 80h
        neg      ecx
        sbb      ecx, ecx
        and      ecx, 1Bh
        shl      eax, 1
        xor      eax, ecx
        mov      BYTE PTR __y, al

        ; FT0[i] =   (uint32) ( x ^ y ) ^
        ;          ( (uint32) x <<  8 ) ^
        ;          ( (uint32) x << 16 ) ^
        ;          ( (uint32) y << 24 );
        movzx    edx, BYTE PTR __x
        movzx    eax, BYTE PTR __y
        xor      edx, eax
        movzx    ecx, BYTE PTR __x
        shl      ecx, 8                ; << 8
        xor      edx, ecx
        shl      ecx, 8                ; << 16
        xor      edx, ecx
        shl      eax, 24
        xor      edx, eax
        mov      ecx, DWORD PTR __i                     ; ecx = i
        and      rcx, 0FFFFFFFFh
        mov      DWORD PTR _FT0[rcx*SIZEOF DWORD], edx  ; FT0[i]

        ; FT1[i] = ROTR8( FT0[i] );
        AES_ROTATE_I _FT0, _FT1
        ; FT2[i] = ROTR8( FT1[i] );
        AES_ROTATE_I _FT1, _FT2
        ; FT3[i] = ROTR8( FT2[i] );
        AES_ROTATE_I _FT2, _FT3

        ; y = RSb[i];
        mov      dl, BYTE PTR _RSb[rcx]
        mov      BYTE PTR __y, dl

        ; MUL(i,y) ( ( i && y ) ? pow[(log[i] + log[y]) % 255] : 0 );
        movzx    edx, BYTE PTR __y       ; edx = __y
        test     edx, edx
        je       SHORT aesGenTab_5       ; Skip MOD(255) calculation if __y == 0
        
        AES_DIV_GF 11

aesGenTab_5:
        mov      DWORD PTR _tv3, edx
        movzx    edx, BYTE PTR __y
        test     edx, edx
        je       SHORT aesGenTab_6
        
        AES_DIV_GF 13

aesGenTab_6:
        mov      DWORD PTR _tv2, edx
        movzx    edx, BYTE PTR __y
        test     edx, edx
        je       SHORT aesGenTab_7
        
        AES_DIV_GF 09

aesGenTab_7:
        mov      DWORD PTR _tv1, edx
        movzx    edx, BYTE PTR __y
        test     edx, edx
        je       SHORT aesGenTab_8
  
        AES_DIV_GF 14

aesGenTab_8:
        ; RT0[i] = ( (uint32) MUL( 0x0B, y )       ) ^
        ;          ( (uint32) MUL( 0x0D, y ) <<  8 ) ^
        ;          ( (uint32) MUL( 0x09, y ) << 16 ) ^
        ;          ( (uint32) MUL( 0x0E, y ) << 24 );
        mov      eax, DWORD PTR _tv3                     ; _tv2 = _pow[via log[11]]
        mov      ecx, DWORD PTR _tv2                     ; _tv1 = _pow[via log[13]]
        shl      ecx, 8                                  
        xor      eax, ecx
        mov      ecx, DWORD PTR _tv1                     ; _tv3 = _pow[via log[9]]
        shl      ecx, 16                                 
        xor      eax, ecx
        shl      edx, 24                                 ; edx =  _pow[via log[14]]
        xor      eax, edx
        mov      ecx, DWORD PTR __i                      ; ecx = i
        and     rcx, 0FFFFFFFFh
        mov      DWORD PTR _RT0[rcx*SIZEOF DWORD], eax   ; RT0[i]

        ; RT1[i] = ROTR8( RT0[i] );
        AES_ROTATE_I _RT0, _RT1
        ; RT2[i] = ROTR8( RT1[i] );
        AES_ROTATE_I _RT1, _RT2
        ; RT3[i] = ROTR8( RT2[i] );
        AES_ROTATE_I _RT2, _RT3

        inc      DWORD PTR __i
        cmp      DWORD PTR __i, _SIZE_TABLE
        jb       aesGenTab_4
;       ---------------------
aesGenTab_ret:
        ret
aesGenerateTables ENDP ; aesGenerateTables


;------------------------------------------------------------------------------
;
;                       aesSetKey
;
;   void aes_set_key(aes_context *ctx, uint8 *key, int keysize)
;
;   Change: Removed usage of [rbp] to allow "INVOKE" addressing technique.
;           (the optimizing C++ Compiler froces direct [rsp] addresiing
;            to gain [rbp] as an additional multipurpose register) 
;
aesSetKey PROC USES rbx rsi rdi, _ctx:QWORD, _key:QWORD, _keysize:DWORD
        LOCAL   _SK:DWORD     ; uint32 *SK, *RK;
        LOCAL   _RK:DWORD     
        LOCAL   __i:DWORD     ; int i;

        call     aesGenerateTables

        ; RK = ctx->erk;
        mov      edx, DWORD PTR _ctx._ctxStruct.erk ; Encryption round keys
        and      rcx, 0FFFFFFFFh
        mov      DWORD PTR _RK, edx
        ; switch( keysize )
        mov      eax, DWORD PTR _keysize
        xor      rcx, rcx                       ; Init index counter __i=0

aesSetKey_1:                                
        ; case 128: ctx->nr = 10; break;
        mov      DWORD PTR [rdx]._ctxStruct.rnr, 10
        cmp      eax, 128                       ; Keysize 128 bits
        je       SHORT @F
        
        ; case 192: ctx->nr = 12; break;
        mov      DWORD PTR [rdx]._ctxStruct.rnr, 12
        cmp      eax, 192                       ; Keysize 192 bits
        je       SHORT @F
        
        ; case 256: ctx->nr = 14; break;
        mov      DWORD PTR [rdx]._ctxStruct.rnr, 14
        cmp      eax, 256                       ; Keysize 256 bits
        jne      aesSetKey_ret                  ; default : return; break;
        
@@:     ; for( i = 0; i < (keysize >> 5); i++ )
        ; GET_UINT32_BE( RK[i], key, i << 2 );
        mov      rdx, QWORD PTR _key
        mov      eax, DWORD PTR [rdx+rcx*SIZEOF DWORD]    ; ecx = i
        bswap    eax
        mov      edx, DWORD PTR _RK
        and      rdx, 0FFFFFFFFh
        mov      DWORD PTR [rdx+rcx*SIZEOF DWORD], eax
        mov      eax, DWORD PTR _keysize
        shr      eax, 5                         ; i: 4*4, 6*4, 8*4 key-bytes
        inc      ecx                            ; ecx = i++;
        cmp      ecx, eax
        jb       SHORT @B
;       -----------------

        ; // setup encryption round keys
        ; switch( ctx->nr )                    ; edx = ctx
        mov      DWORD PTR __i, 0              ; Prepare and init index __i=0
        mov      ecx, DWORD PTR _RK            ; ecx = RK

        cmp      DWORD PTR [rdx]._ctxStruct.rnr, 14 ; 256bit-Key: 14 rounds
        je       aesSetKey_4

        cmp      DWORD PTR [rdx]._ctxStruct.rnr, 12 ; 192bit-Key: 12 rounds
        je       aesSetKey_3

        cmp      DWORD PTR [rdx]._ctxStruct.rnr, 10 ; 128bit-Key: 10 rounds
        jne      aesSetKey_5                        ; Illegal keysize.

aesSetKey_2:
        ; case 10:
        ;     for( i = 0; i < 10; i++, RK += 4 )
        ; RK[4]  = RK[0] ^ RCON[i] ^
        ;     ( FSb[ (uint8) ( RK[3] >> 16 ) ] << 24 ) ^
        ;     ( FSb[ (uint8) ( RK[3] >>  8 ) ] << 16 ) ^
        ;     ( FSb[ (uint8) ( RK[3]       ) ] <<  8 ) ^
        ;     ( FSb[ (uint8) ( RK[3] >> 24 ) ]       );
        mov      edx, DWORD PTR __i                       ; 128bit-Key: 10 rounds

        KEYSIZE_SBOX_RCON 3,4                             ; Params: 128/32-1, 128/32
                                                          
        ; RK[5]  = RK[1] ^ RK[4];
        mov      eax, DWORD PTR [rcx+(1*SIZEOF DWORD)]    ; eax = RK[1]
        xor      eax, DWORD PTR [rcx+(4*SIZEOF DWORD)]
        mov      DWORD PTR [rcx+(5*SIZEOF DWORD)], eax    ; RK[5] = eax

        ; RK[6]  = RK[2] ^ RK[5];
        mov      eax, DWORD PTR [rcx+(2*SIZEOF DWORD)]    ; eax = RK[2]
        xor      eax, DWORD PTR [rcx+(5*SIZEOF DWORD)]
        mov      DWORD PTR [rcx+(6*SIZEOF DWORD)], eax    ; RK[6] = eax

        ; RK[7]  = RK[3] ^ RK[6];
        mov      eax, DWORD PTR [rcx+(3*SIZEOF DWORD)]    ; eax = RK[3]
        xor      eax, DWORD PTR [rcx+(6*SIZEOF DWORD)]
        mov      DWORD PTR [rcx+(7*SIZEOF DWORD)], eax    ; RK[7] = eax

        add      rcx, 4*SIZEOF DWORD                      ; RK += 4;
        inc      DWORD PTR __i                            ; i++;
        cmp      DWORD PTR __i, _SIZE_ROUND_CONST
        jb       aesSetKey_2
        ; break;
        jmp      aesSetKey_5

aesSetKey_3:
        ; case 12:
        ;     for( i = 0; i < 8; i++, RK += 6 )
        ; RK[6]  = RK[0] ^ RCON[i] ^
        ;     ( FSb[ (uint8) ( RK[5] >> 16 ) ] << 24 ) ^
        ;     ( FSb[ (uint8) ( RK[5] >>  8 ) ] << 16 ) ^
        ;     ( FSb[ (uint8) ( RK[5]       ) ] <<  8 ) ^
        ;     ( FSb[ (uint8) ( RK[5] >> 24 ) ]       );
        mov      edx, DWORD PTR __i                       ; 192bit-Key: 12 rounds

        KEYSIZE_SBOX_RCON 5,6                             ; Params: 192/32-1, 192/32

        ; RK[7]  = RK[1] ^ RK[6];
        mov      eax, DWORD PTR [rcx+1*SIZEOF DWORD]      ; eax = RK[1]
        xor      eax, DWORD PTR [rcx+6*SIZEOF DWORD]
        mov      DWORD PTR [rcx+7*SIZEOF DWORD], eax      ; RK[7] = eax

        ; RK[8]  = RK[2] ^ RK[7];
        mov      eax, DWORD PTR [rcx+2*SIZEOF DWORD]      ; eax = RK[2]
        xor      eax, DWORD PTR [rcx+7*SIZEOF DWORD]
        mov      DWORD PTR [rcx+8*SIZEOF DWORD], eax      ; RK[8] = eax

        ; RK[9]  = RK[3] ^ RK[8];
        mov      eax, DWORD PTR [rcx+3*SIZEOF DWORD]      ; eax = RK[3]
        xor      eax, DWORD PTR [rcx+8*SIZEOF DWORD]
        mov      DWORD PTR [rcx+9*SIZEOF DWORD], eax      ; RK[9] = eax

        ; RK[10] = RK[4] ^ RK[9];
        mov      eax, DWORD PTR [rcx+4*SIZEOF DWORD]      ; eax = RK[4]
        xor      eax, DWORD PTR [rcx+9*SIZEOF DWORD]
        mov      DWORD PTR [rcx+10*SIZEOF DWORD], eax     ; RK[10] = eax

        ; RK[11] = RK[5] ^ RK[10];
        mov      eax, DWORD PTR [rcx+5*SIZEOF DWORD]      ; eax = RK[5]
        xor      eax, DWORD PTR [rcx+10*SIZEOF DWORD]
        mov      DWORD PTR [rcx+11*SIZEOF DWORD], eax     ; RK[11] = eax

        add      rcx, 6*SIZEOF DWORD                      ; RK += 6;
        inc      DWORD PTR __i                            ; i++;
        cmp      DWORD PTR __i, _SIZE_ROUND_CONST-2       ; =8
        jb       aesSetKey_3                              ; Loop
        ; break;
        jmp      aesSetKey_5                              ; break;
 
aesSetKey_4:                                              ; Keysize 256 bits
        ; case 14:
        ;     for( i = 0; i < 7; i++, RK += 8 )
        ; RK[8]  = RK[0] ^ RCON[i] ^
        ;     ( FSb[ (uint8) ( RK[7] >> 16 ) ] << 24 ) ^
        ;     ( FSb[ (uint8) ( RK[7] >>  8 ) ] << 16 ) ^
        ;     ( FSb[ (uint8) ( RK[7]       ) ] <<  8 ) ^
        ;     ( FSb[ (uint8) ( RK[7] >> 24 ) ]       );
        mov      edx, DWORD PTR __i                       ; edx = i

        KEYSIZE_SBOX_RCON 7,8                             ; Params: 256/32-1, 256/32

        ; RK[9]  = RK[1] ^ RK[8];
        mov      eax, DWORD PTR [rcx+1*SIZEOF DWORD]      ; eax = RK[1]
        xor      eax, DWORD PTR [rcx+8*SIZEOF DWORD]
        mov      DWORD PTR [rcx+9*SIZEOF DWORD], eax      ; RK[9] = eax

        ; RK[10] = RK[2] ^ RK[9];
        mov      eax, DWORD PTR [rcx+2*SIZEOF DWORD]      ; eax = RK[2]
        xor      eax, DWORD PTR [rcx+9*SIZEOF DWORD]
        mov      DWORD PTR [rcx+10*SIZEOF DWORD], eax     ; RK[10] = eax

        ; RK[11] = RK[3] ^ RK[10];
        mov      eax, DWORD PTR [rcx+3*SIZEOF DWORD]      ; eax = RK[3]
        xor      eax, DWORD PTR [rcx+10*SIZEOF DWORD]
        mov      DWORD PTR [rcx+11*SIZEOF DWORD], eax     ; RK[11] = eax

        ; RK[12] = RK[4] ^
        ;     ( FSb[ (uint8) ( RK[11] >> 24 ) ] << 24 ) ^
        ;     ( FSb[ (uint8) ( RK[11] >> 16 ) ] << 16 ) ^
        ;     ( FSb[ (uint8) ( RK[11] >>  8 ) ] <<  8 ) ^
        ;     ( FSb[ (uint8) ( RK[11]       ) ]       );
        mov      edx, DWORD PTR [rcx+11*SIZEOF DWORD]     ; RK[11]
        shr      edx, 24
        movzx    rdx, dl
        movzx    eax, BYTE PTR _FSb[rdx]
        shl      eax, 24
        xor      eax, DWORD PTR [rcx+4*SIZEOF DWORD]      ; RK[4]
        mov      edx, DWORD PTR [rcx+11*SIZEOF DWORD]     ; RK[11]
        shr      edx, 16
        movzx    rdx, dl
        movzx    edx, BYTE PTR _FSb[rdx]
        shl      edx, 16
        xor      eax, edx
        mov      edx, DWORD PTR [rcx+11*SIZEOF DWORD]     ; RK[11]
        shr      edx, 8
        movzx    rdx, dl
        movzx    edx, BYTE PTR _FSb[rdx]
        shl      edx, 8
        xor      eax, edx
        movzx    rdx, BYTE PTR [rcx+11*SIZEOF DWORD]      ; RK[11]
        movzx    edx, BYTE PTR _FSb[rdx]
        xor      eax, edx
        mov      DWORD PTR [rcx+12*SIZEOF DWORD], eax     ; RK[12]

        ; RK[13] = RK[5] ^ RK[12];
        mov      eax, DWORD PTR [rcx+5*SIZEOF DWORD]      ; eax = RK[5]
        xor      eax, DWORD PTR [rcx+12*SIZEOF DWORD]
        mov      DWORD PTR [rcx+13*SIZEOF DWORD], eax     ; RK[13] = eax

        ; RK[14] = RK[6] ^ RK[13];
        mov      eax, DWORD PTR [rcx+6*SIZEOF DWORD]      ; eax = RK[6]
        xor      eax, DWORD PTR [rcx+13*SIZEOF DWORD]
        mov      DWORD PTR [rcx+14*SIZEOF DWORD], eax     ; RK[14] = eax

        ; RK[15] = RK[7] ^ RK[14];
        mov      eax, DWORD PTR [rcx+7*SIZEOF DWORD]      ; eax = RK[7]
        xor      eax, DWORD PTR [rcx+14*SIZEOF DWORD]     
        mov      DWORD PTR [rcx+15*SIZEOF DWORD], eax     ; RK[15] = eax

        add      rcx, 8*SIZEOF DWORD                      ; RK += 8;
        inc      DWORD PTR __i                            ; i++;
        cmp      DWORD PTR __i, _SIZE_ROUND_CONST-3       ; =7
        jb       aesSetKey_4
;       --------------------

aesSetKey_5:
        ; default:
        ; // setup decryption round keys
        ; for( i = 0; i < 256; i++ )
        xor      rdx, rdx                                 ; edx: i = 0;

aesSetKey_6:
        movzx    rbx, BYTE PTR _FSb[rdx]                  ; ebx = FSb[i]

        ; KT0[i] = RT0[ FSb[i] ];
        mov      eax, DWORD PTR _RT0[rbx*SIZEOF DWORD]    
        mov      DWORD PTR _KT0[rdx*SIZEOF DWORD], eax
        ; KT1[i] = RT1[ FSb[i] ];
        mov      eax, DWORD PTR _RT1[rbx*SIZEOF DWORD]
        mov      DWORD PTR _KT1[rdx*SIZEOF DWORD], eax
        ; KT2[i] = RT2[ FSb[i] ];
        mov      eax, DWORD PTR _RT2[rbx*SIZEOF DWORD]
        mov      DWORD PTR _KT2[rdx*SIZEOF DWORD], eax
        ; KT3[i] = RT3[ FSb[i] ];
        mov      eax, DWORD PTR _RT3[rbx*SIZEOF DWORD]
        mov      DWORD PTR _KT3[rdx*SIZEOF DWORD], eax
 
        inc      rdx
        cmp      rdx, _SIZE_TABLE                         ; 256
        jb       aesSetKey_6
;       --------------------

;;ha;;  ; kt_init = 1;
;;ha;;  mov      DWORD PTR kt_init, 1                   ; Set kt_init done

aesSetKey_7:
        ; SK = ctx->drk;
        mov      edx, DWORD PTR _ctx
        and      rcx, 0FFFFFFFFh
        add      rdx, _ctxStruct.drk                      ; Decryption round keys 
        mov      DWORD PTR _SK, edx                       ; edx = SK
                                                          ; ecx = RK
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+0*SIZEOF DWORD]      ; RK[0]
        mov      DWORD PTR [rdx+0*SIZEOF DWORD], eax      ; SK[0]
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+1*SIZEOF DWORD]      ; RK[1]
        mov      DWORD PTR [rdx+1*SIZEOF DWORD], eax      ; SK[1]
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+2*SIZEOF DWORD]      ; RK[2]
        mov      DWORD PTR [rdx+2*SIZEOF DWORD], eax      ; SK[2]
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+3*SIZEOF DWORD]      ; RK[3]
        mov      DWORD PTR [rdx+3*SIZEOF DWORD], eax      ; SK[3]

        ; for( i = 1; i < ctx->nr; i++ )
        mov      DWORD PTR __i, 1

aesSetKey_8:
        sub      rcx, 4*SIZEOF DWORD                      ; RK -= 4;
        add      rdx, 4*SIZEOF DWORD                      ; SK += 4;

        SET_KEY 0,1,2,3

        inc      DWORD PTR __i                            ; i++
        mov      eax, DWORD PTR __i
        mov      ebx, DWORD PTR _ctx
        and      rbx, 0FFFFFFFFh
        cmp      eax, DWORD PTR [rbx]._ctxStruct.rkEnd    ; +512
        jb       aesSetKey_8
;       --------------------
        
aesSetKey_exit:
        sub      rcx, 4*SIZEOF DWORD                      ; RK -= 4;            
        add      rdx, 4*SIZEOF DWORD                      ; SK += 4; (edx = ctx.drk)
                                                          
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+0*SIZEOF DWORD]      ; eax = RK[0]
        mov      DWORD PTR [rdx+0*SIZEOF DWORD], eax      ; SK[0] = eax
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+1*SIZEOF DWORD]      ; eax = RK[1]
        mov      DWORD PTR [rdx+1*SIZEOF DWORD], eax      ; SK[1] = eax
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+2*SIZEOF DWORD]      ; eax = RK[2]
        mov      DWORD PTR [rdx+2*SIZEOF DWORD], eax      ; SK[2] = eax
        ; *SK++ = *RK++;
        mov      eax, DWORD PTR [rcx+3*SIZEOF DWORD]      ; eax = RK[3]
        mov      DWORD PTR [rdx+3*SIZEOF DWORD], eax      ; SK[3] = eax

aesSetKey_ret:
        ret
aesSetKey ENDP ; aesSetKey


;------------------------------------------------------------------------------
; 
;                       _aesAlgorithm
;
; void aes_encrypt(aes_context *ctx, unsigned char input[16],
;                                    unsigned char output[16])
;
;       Function compile flags: /Odtp
;
;       ----------- C++ Compiler optimization turned off  -----------
;            Therefore extensive [rsp] indexing is inevitable.
;              May cause headaches sometimes, when rewriting
;               for INVOKE interface mechanism using [rbp].
;
_aesAlgorithm PROC USES rbx rsi rdi, _ctx, _input, _output
        LOCAL   _X0:DWORD       ; uint32 *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;
        LOCAL   _X1:DWORD
        LOCAL   _X2:DWORD
        LOCAL   _X3:DWORD
        LOCAL   _Y0:DWORD
        LOCAL   _Y1:DWORD
        LOCAL   _Y2:DWORD
        LOCAL   _Y3:DWORD                                        
        LOCAL   _RK:DWORD

        mov      eax, DWORD PTR _ctx    ; eax = ctx->_ctxStruct;
        add      eax, DWORD PTR aesMode ; _ctxStruct.erk / _ctxStruct.drk
        mov      DWORD PTR _RK, eax     ; Init _RK

        ; GET_UINT32_BE( X0, input,  0 ); X0 ^= RK[0];   ; Get plain data
        _GET_UINT32 _X0, _input, 0
        ; GET_UINT32_BE( X1, input,  4 ); X1 ^= RK[1];
        _GET_UINT32 _X1, _input, 1
        ; GET_UINT32_BE( X2, input,  8 ); X2 ^= RK[2];
        _GET_UINT32 _X2, _input, 2
        ; GET_UINT32_BE( X3, input, 12 ); X3 ^= RK[3];
        _GET_UINT32 _X3, _input, 3

        ;  #define AES_FROUND(X0,X1,X2,X3,Y0,Y1,Y2,Y3)          \
        ;  {                                                    \
        ;      RK += 4;                                         \
        ;                                                       \
        ;      X0 = RK[0] ^ FT0[ (uint8) ( Y0 >> 24 ) ] ^       \
        ;                   FT1[ (uint8) ( Y1 >> 16 ) ] ^       \
        ;                   FT2[ (uint8) ( Y2 >>  8 ) ] ^       \
        ;                   FT3[ (uint8) ( Y3       ) ];        \
        ;                                                       \
        ;      X1 = RK[1] ^ FT0[ (uint8) ( Y1 >> 24 ) ] ^       \
        ;                   FT1[ (uint8) ( Y2 >> 16 ) ] ^       \
        ;                   FT2[ (uint8) ( Y3 >>  8 ) ] ^       \
        ;                   FT3[ (uint8) ( Y0       ) ];        \
        ;                                                       \
        ;      X2 = RK[2] ^ FT0[ (uint8) ( Y2 >> 24 ) ] ^       \
        ;                   FT1[ (uint8) ( Y3 >> 16 ) ] ^       \
        ;                   FT2[ (uint8) ( Y0 >>  8 ) ] ^       \
        ;                   FT3[ (uint8) ( Y1       ) ];        \
        ;                                                       \
        ;      X3 = RK[3] ^ FT0[ (uint8) ( Y3 >> 24 ) ] ^       \
        ;                   FT1[ (uint8) ( Y0 >> 16 ) ] ^       \
        ;                   FT2[ (uint8) ( Y1 >>  8 ) ] ^       \
        ;                   FT3[ (uint8) ( Y2       ) ];        \
        ;  }

        ; 01 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3       
        
        ; 02 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);
        AES_ROUND _X, _Y, 0,1,2,3       
        
        ; 03 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3       
                
        ; 04 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);
        AES_ROUND _X, _Y, 0,1,2,3       
                
        ; 05 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3                           
                
        ; 06 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);
        AES_ROUND _X, _Y, 0,1,2,3       
                
        ; 07 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3       
                
        ; 08 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);
        AES_ROUND _X, _Y, 0,1,2,3       
                
        ; 09 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3       
        
        mov      ecx, DWORD PTR _ctx                    ; if( ctx->nr == 10 )
        cmp      DWORD PTR [rcx]._ctxStruct.rnr, 10     ; 128bit-key?   
        je       @F                                     ; Last round = 10th Round.
        
        ; 10 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);
        AES_ROUND _X, _Y, 0,1,2,3       
                
        ; 11 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3       

        mov      edx, DWORD PTR _ctx                    ; if( ctx->nr == 12 )
        cmp      DWORD PTR [rdx]._ctxStruct.rnr, 12     ; 192bit-key?   
        je       @F                                     ; Last round = 12th Round.

        ; 12 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);      ; ctx->nr == 14
        AES_ROUND _X, _Y, 0,1,2,3                       ; 256bit-key?   
                                                        ; Last round = 14th Round.
        ; 13 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        AES_ROUND _Y, _X, 0,1,2,3       
        
@@:     ; #define AES_LAST_ROUND Fsb(X0,X1,X2,X3,Y0,Y1,Y2,Y3)           \
        ; {                                                             \
        ;     RK += 4;                                                  \
        ;                                                               \
        ;     X0 = RK[0] ^ ( FSb[ (uint8) ( Y0 >> 24 ) ] << 24 ) ^      \
        ;                  ( FSb[ (uint8) ( Y1 >> 16 ) ] << 16 ) ^      \
        ;                  ( FSb[ (uint8) ( Y2 >>  8 ) ] <<  8 ) ^      \
        ;                  ( FSb[ (uint8) ( Y3       ) ]       );       \
        ;                                                               \
        ;     X1 = RK[1] ^ ( FSb[ (uint8) ( Y1 >> 24 ) ] << 24 ) ^      \
        ;                  ( FSb[ (uint8) ( Y2 >> 16 ) ] << 16 ) ^      \
        ;                  ( FSb[ (uint8) ( Y3 >>  8 ) ] <<  8 ) ^      \
        ;                  ( FSb[ (uint8) ( Y0       ) ]       );       \
        ;                                                               \
        ;     X2 = RK[2] ^ ( FSb[ (uint8) ( Y2 >> 24 ) ] << 24 ) ^      \
        ;                  ( FSb[ (uint8) ( Y3 >> 16 ) ] << 16 ) ^      \
        ;                  ( FSb[ (uint8) ( Y0 >>  8 ) ] <<  8 ) ^      \
        ;                  ( FSb[ (uint8) ( Y1       ) ]       );       \
        ;                                                               \
        ;     X3 = RK[3] ^ ( FSb[ (uint8) ( Y3 >> 24 ) ] << 24 ) ^      \
        ;                  ( FSb[ (uint8) ( Y0 >> 16 ) ] << 16 ) ^      \
        ;                  ( FSb[ (uint8) ( Y1 >>  8 ) ] <<  8 ) ^      \
        ;                  ( FSb[ (uint8) ( Y2       ) ]       );       \
        ; )

        ; 10th/12th/14th Final Round
        AES_LAST_ROUND _X, _Y, 0,1,2,3
        
        ; PUT_UINT32_BE( X0, output,  0 );               ; Put encrypted data
        _PUT_UINT32 _X0, _output, 0
        ; PUT_UINT32_BE( X1, output,  4 );
        _PUT_UINT32 _X1, _output, 1
        ; PUT_UINT32_BE( X2, output,  8 );
        _PUT_UINT32 _X2, _output, 2
        ; PUT_UINT32_BE( X3, output, 12 );
        _PUT_UINT32 _X3, _output, 3

         ret
_aesAlgorithm ENDP ; _aesAlgorithm

;------------------------------------------------------------------------------
; 
;                       aesAlgorithm
;
;      Function compile flags: /Odtp
;                                                                                          
;      void aesAlgorithm(char *inblock, char *outblock, int _mode)
;
;      aesAlgorithm PROTO C, :DWORD, :DWORD, :DWORD
;
;      Language type: PROC C (Public interface to aesfastmain.cpp)
;      aesfastmain.cpp: extern "C" void aesAlgorithm (char*, char*, int);
;
;      Stub to invoke aesEncrypt & aesDecrypt procedures
;
aesAlgorithm PROC PUBLIC USES rbx rsi rdi, _inblock:DWORD, _outblock:DWORD, _mode:DWORD
;
; NOTE: The '_mode:DWORD parameter is not used in this AESfast.ASM module,
;       but we force compatibility with the AES.CPP module in order
;       to have (nearly) equal haCryptAESfast.CPP / haCryptAES.CPP modules.
;
; The invocation paremeters for _aesAlgorithm must be re-arranged:
;  aesAlgorithm(char *inblock, char *outblock);
;  -->
;  aes_encrypt(&ctx, (unsigned char *)inblock, (unsigned char *)outblock);
;  aes_decrypt(&ctx, (unsigned char *)inblock, (unsigned char *)outblock);
;  <--
;
;  Parameter translation is applied as follows:
;  aesAlgorithm() -->  _aesAlgorithm() = aesEncrypt & aesDcrypt
;  -------------------------------------------------------------
;  _inblock       <--  OFFSET ctxAesContext  (ctx    = [-12+08])
;  _outblock      <--  _inblock              (inbuf  = [-12+12])
;                 <--  _outblock             (outbuf = [-12+16])
;
        mov     DWORD PTR _mode, r8d
        mov     QWORD PTR _outblock, rdx
        mov     QWORD PTR _inblock, rcx

;;      INVOKE _aesAlgorithm, OFFSET ctxAesContext, _inblock, _outblock
;       ---------------------------------
        lea     rdi, OFFSET ctxAesContext
        push    QWORD PTR _outblock
        push    QWORD PTR _inblock
        push    rdi
        call    _aesAlgorithm
        add     rsp, 3*SIZEOF QWORD

        ret
aesAlgorithm ENDP ; aesAlgorithm

;------------------------------------------------------------------------------
;
;                               aesKeyInit
;
;       void aesKeyInit(char *key, int keylen, int mode)
;
;       kinit PROTO C, key:DWORD, edf:DWORD
;
;      Language type: PROC C (Public interface to aesfastmain.cpp)
;      aesfastmain.cpp: extern "C" void aesKeyInit(char*, int, int);
;
;      Stub to initialize key schedule array for aes
;
aesKeyInit PROC PUBLIC USES rbx rdx rsi rdi, _key:QWORD, _keylen:DWORD, _mode:DWORD
        mov     DWORD PTR _mode, r8d
        mov     DWORD PTR _keylen, edx
        mov     QWORD PTR _key, rcx

        mov     DWORD PTR aesMode, _ctxStruct.erk ; .erk Encrypt Struct
        mov     eax, DWORD PTR _mode
        cmp     eax, ENCRYPT
        je      @F

        mov     DWORD PTR aesMode, _ctxStruct.drk ; .drk Decipher Struct
@@:
;;      INVOKE aesSetKey, OFFSET ctxAesContext, _key, _keylen
;       ---------------------------------
        mov     esi, DWORD PTR _keylen
        and     rsi, 0FFFFFFFFh
        mov     rdx, QWORD PTR _key
        lea     rdi, OFFSET ctxAesContext
        push    rsi
        push    rdx
        push    rdi
        call    aesSetKey
        add     rsp, 3*SIZEOF QWORD
        
        ret
aesKeyInit ENDP ; aesKeyInit
 
;------------------------------------------------------------------------------

_TEXT   ENDS
        END

;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
;;ha;;  push    rsi
;;ha;;  push    rdi
;;ha;;  push    rcx
;;ha;;  mov     rsi, QWORD PTR _key
;;ha;;  mov     rdi, OFFSET _aesDebugbuf
;;ha;;  mov     rcx, SIZEOF _aesDebugbuf/2 ; 1st half to intercept _key
;;ha;;  rep movs BYTE PTR [rdi], [rsi]      
;;ha;;; -----------------------------------
;;ha;;  mov     ecx, DWORD PTR _keysize    ; 2nd half multi purpose
;;ha;;  mov     DWORD PTR [rdi], ecx
;;ha;;;;ha;;    mov     rcx, QWORD PTR _ctx        ; 2nd half multi purpose
;;ha;;;;ha;;    mov     QWORD PTR [rdi+1], rcx
;;ha;;  pop     rcx
;;ha;;  pop     rdi
;;ha;;  pop     rsi
;;ha;;;;---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---

