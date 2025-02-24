;; aesfast.inc - MASM Developer source file.
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

;
; Structure definitions for aesfast.asm
;
_ctxStruct STRUCT               ; ctx STRUCT (ctxAesContext)
  erk   DB 256 DUP (?)          ; Encryption round keys
  drk   DB 256 DUP (?)          ; Decipher round keys
  UNION
    rkEnd DB 4 DUP (?)          ; End of round keys area
    rnr   DB 4 DUP (?)          ;  rsp. Number of rounds
  ENDS     
_ctxStruct ENDS
 
;
; Macro definitions employed in aesfast.asm
;
;-----------------------------------------------  
;  #define GET_UINT32_BE(n,b,i)                 \
;  {                                            \
;      (n) = ( (uint32) (b)[(i)    ] << 24 )    \
;          | ( (uint32) (b)[(i) + 1] << 16 )    \
;          | ( (uint32) (b)[(i) + 2] <<  8 )    \
;          | ( (uint32) (b)[(i) + 3]       );   \
;  }
;-----------------------------------------------  
        ; _GET_UINT32 _X0, _input, 0                    ; Get enc / dec data
_GET_UINT32 MACRO _x, _buf, _i
        mov      ecx, DWORD PTR _buf
        mov      eax, DWORD PTR [ecx+_i*SIZEOF DWORD]
        bswap    eax                                    ; Reverse the byte order
        mov      ecx, DWORD PTR _RK                     ; X0 ^= RK[i];
        xor      eax, DWORD PTR [ecx+_i*SIZEOF DWORD]    
        mov      DWORD PTR _x, eax                      ; Bits[31:0]
        ENDM
                                         
;-----------------------------------------------  
;  #define PUT_UINT32_BE(n,b,i)                 \
;  {                                            \
;      (b)[(i)    ] = (uint8) ( (n) >> 24 );    \
;      (b)[(i) + 1] = (uint8) ( (n) >> 16 );    \
;      (b)[(i) + 2] = (uint8) ( (n) >>  8 );    \
;      (b)[(i) + 3] = (uint8) ( (n)       );    \
;  }                                            
;-----------------------------------------------  
        ; _PUT_UINT32 _X0, _output, 0                   ; Put enc / dec data
_PUT_UINT32 MACRO _x, _buf, _i
        mov      ecx, DWORD PTR _buf
        mov      eax, DWORD PTR _x                      ; Bits[31:0]
        bswap    eax                                    ; Reverse the byte order
        mov      DWORD PTR [ecx+_i*SIZEOF DWORD], eax   ; Bits[31:0]
        ENDM


; AES-ENCRYPT/DECIPHER
;----------------------------------------------------------------  
;  #define AES_ROUND(FT,X0,X1,X2,X3,Y0,Y1,Y2,Y3,RK0,RK1,RK2,RK3) \
;  {                                                             \
;      RK += 4;                                                  \
;                                                                \
;      X0 = RK[0] ^ FT0[ (uint8) ( Y0 >> 24 ) ] ^                \
;                   FT1[ (uint8) ( Y1 >> 16 ) ] ^                \
;                   FT2[ (uint8) ( Y2 >>  8 ) ] ^                \
;                   FT3[ (uint8) ( Y3       ) ];                 \
;                                                                \
;      X1 = RK[1] ^ FT0[ (uint8) ( Y1 >> 24 ) ] ^                \
;                   FT1[ (uint8) ( Y2 >> 16 ) ] ^                \
;                   FT2[ (uint8) ( Y3 >>  8 ) ] ^                \
;                   FT3[ (uint8) ( Y0       ) ];                 \
;                                                                \
;      X2 = RK[2] ^ FT0[ (uint8) ( Y2 >> 24 ) ] ^                \
;                   FT1[ (uint8) ( Y3 >> 16 ) ] ^                \
;                   FT2[ (uint8) ( Y0 >>  8 ) ] ^                \
;                   FT3[ (uint8) ( Y1       ) ];                 \
;                                                                \
;      X3 = RK[3] ^ FT0[ (uint8) ( Y3 >> 24 ) ] ^                \
;                   FT1[ (uint8) ( Y0 >> 16 ) ] ^                \
;                   FT2[ (uint8) ( Y1 >>  8 ) ] ^                \
;                   FT3[ (uint8) ( Y2       ) ];                 \
;  }                                                           
;----------------------------------------------------------------   
;----------------------------------------------------------------          
;  #define AES_ROUND(RT,X0,X3,X2,X1,Y0,Y3,Y2,Y1,RK0,RK3,RK2,RK1) \
;  {                                                             \
;      RK += 4;                                                  \
;                                                                \
;      X0 = RK[0] ^ RT0[ (uint8) ( Y0 >> 24 ) ] ^                \
;                   RT1[ (uint8) ( Y3 >> 16 ) ] ^                \
;                   RT2[ (uint8) ( Y2 >>  8 ) ] ^                \
;                   RT3[ (uint8) ( Y1       ) ];                 \
;                                                                \
;      X3 = RK[3] ^ RT0[ (uint8) ( Y1 >> 24 ) ] ^                \
;                   RT1[ (uint8) ( Y0 >> 16 ) ] ^                \
;                   RT2[ (uint8) ( Y3 >>  8 ) ] ^                \
;                   RT3[ (uint8) ( Y2       ) ];                 \
;                                                                \
;      X2 = RK[2] ^ RT0[ (uint8) ( Y2 >> 24 ) ] ^                \
;                   RT1[ (uint8) ( Y1 >> 16 ) ] ^                \
;                   RT2[ (uint8) ( Y0 >>  8 ) ] ^                \
;                   RT3[ (uint8) ( Y3       ) ];                 \
;                                                                \
;      X1 = RK[1] ^ RT0[ (uint8) ( Y3 >> 24 ) ] ^                \
;                   RT1[ (uint8) ( Y2 >> 16 ) ] ^                \
;                   RT2[ (uint8) ( Y1 >>  8 ) ] ^                \
;                   RT3[ (uint8) ( Y0       ) ];                 \
;  }
;----------------------------------------------------------------  
AES_ROUND_CODE MACRO _cTab, _a, _b, _rk0, _rk1, _rk2, _rk3 
        mov      eax, DWORD PTR [ecx+_rk0*SIZEOF DWORD]              ; ecx = RK[0]
        mov      edx, DWORD PTR @CatStr(_b,_rk0)                     ; b0
        shr      edx, 24                        
        movzx    edx, dl
        xor      eax, DWORD PTR @CatStr(_cTab,0)[edx*SIZEOF DWORD]   ; FT[0] / RT[0]
        mov      edx, DWORD PTR @CatStr(_b,_rk1)                     ; b1    / b3
        shr      edx, 16                        
        movzx    edx, dl
        xor      eax, DWORD PTR @CatStr(_cTab,1)[edx*SIZEOF DWORD]   ; FT[1] / RT[1]
        mov      edx, DWORD PTR @CatStr(_b,_rk2)                     ; b2
        shr      edx, 8
        movzx    edx, dl
        xor      eax, DWORD PTR @CatStr(_cTab,2)[edx*SIZEOF DWORD]   ; FT[2] / RT[2]
        movzx    edx, BYTE PTR @CatStr(_b,_rk3)                      ; b3    / b1
        xor      eax, DWORD PTR @CatStr(_cTab,3)[edx*SIZEOF DWORD]   ; FT[3] / RT[3]
        mov      DWORD PTR @CatStr(_a,_rk0), eax                     ; a0
        ENDM

        ; 01 AES_FROUND(Y0,Y1,Y2,Y3, X0,X1,X2,X3);
        ; AES_ROUND _Y, _X, 0,1,2,3     
        ; 02 AES_FROUND(X0,X1,X2,X3, Y0,Y1,Y2,Y3);
        ; AES_ROUND _X, _Y, 0,1,2,3     
AES_ROUND MACRO _a, _b, _rk0, _rk1, _rk2, _rk3 
        LOCAL    ???aesr1, ???aesr2

        add      DWORD PTR _RK, 4*SIZEOF DWORD          ; RK += 4; ; ecx = RK           
        mov      ecx, DWORD PTR _RK                                  
        cmp      DWORD PTR aesMode, _ctxStruct.drk      ; Check: ENCRYPT / DECIPHER
        je       ???aesr1                                       
                                                                ; Encrypt   
        AES_ROUND_CODE _FT, _a, _b, _rk0, _rk1, _rk2, _rk3      ; 0,1,2,3   
        AES_ROUND_CODE _FT, _a, _b, _rk1, _rk2, _rk3, _rk0      ; 1,2,3,0   
        AES_ROUND_CODE _FT, _a, _b, _rk2, _rk3, _rk0, _rk1      ; 2,3,0,1   
        AES_ROUND_CODE _FT, _a, _b, _rk3, _rk0, _rk1, _rk2      ; 3,0,1,2   
        jmp      ???aesr2

???aesr1:                                                       ; Decipher  
        AES_ROUND_CODE _RT, _a, _b, _rk0, _rk3, _rk2, _rk1      ; 0,3,2,1  
        AES_ROUND_CODE _RT, _a, _b, _rk3, _rk2, _rk1, _rk0      ; 3,2,1,0  
        AES_ROUND_CODE _RT, _a, _b, _rk2, _rk1, _rk0, _rk3      ; 2,1,0,3  
        AES_ROUND_CODE _RT, _a, _b, _rk1, _rk0, _rk3, _rk2      ; 1,0,3,2  
???aesr2:
        ENDM                                                                        


; AES-ENCRYPT/DECIPHER (Last Round SBOX)
;---------------------------------------------------------------           
; #define AES_LAST_ROUND Fsb(X0,X1,X2,X3,Y0,Y1,Y2,Y3)           \
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
;---------------------------------------------------------------           
;---------------------------------------------------------------           
; #define AES_LAST_ROUND Rsb(X0,X1,X2,X3,Y0,Y1,Y2,Y3)           \
; {                                                             \
;     RK += 4;                                                  \
;                                                               \
;     X0 = RK[0] ^ ( RSb[ (uint8) ( Y0 >> 24 ) ] << 24 ) ^      \
;                  ( RSb[ (uint8) ( Y3 >> 16 ) ] << 16 ) ^      \
;                  ( RSb[ (uint8) ( Y2 >>  8 ) ] <<  8 ) ^      \
;                  ( RSb[ (uint8) ( Y1       ) ]       );       \
;                                                               \
;     X1 = RK[1] ^ ( RSb[ (uint8) ( Y1 >> 24 ) ] << 24 ) ^      \
;                  ( RSb[ (uint8) ( Y0 >> 16 ) ] << 16 ) ^      \
;                  ( RSb[ (uint8) ( Y3 >>  8 ) ] <<  8 ) ^      \
;                  ( RSb[ (uint8) ( Y2       ) ]       );       \
;                                                               \
;     X2 = RK[2] ^ ( RSb[ (uint8) ( Y2 >> 24 ) ] << 24 ) ^      \
;                  ( RSb[ (uint8) ( Y1 >> 16 ) ] << 16 ) ^      \
;                  ( RSb[ (uint8) ( Y0 >>  8 ) ] <<  8 ) ^      \
;                  ( RSb[ (uint8) ( Y3       ) ]       );       \
;                                                               \
;     X3 = RK[3] ^ ( RSb[ (uint8) ( Y3 >> 24 ) ] << 24 ) ^      \
;                  ( RSb[ (uint8) ( Y2 >> 16 ) ] << 16 ) ^      \
;                  ( RSb[ (uint8) ( Y1 >>  8 ) ] <<  8 ) ^      \
;                  ( RSb[ (uint8) ( Y0       ) ]       );       \
; )
;---------------------------------------------------------------           
AES_LAST_ROUND_CODE MACRO _sBox, _a, _b, _i0, _i1, _i2, _i3
        mov      eax, DWORD PTR [ecx+_i0*SIZEOF DWORD]  ; RK[0]
        mov      edx, DWORD PTR  @CatStr(_b,_i0)        ; _b0
        shr      edx, 24                        
        movzx    edx, dl
        movzx    edx, BYTE PTR _sBox[edx]               ; Fsb / RSb[(uint8)(Y0 >> 24)]
        shl      edx, 24                        
        xor      eax, edx
        mov      edx, DWORD PTR @CatStr(_b,_i1)         ; _b1
        shr      edx, 16                        
        movzx    edx, dl
        movzx    edx, BYTE PTR _sBox[edx]
        shl      edx, 16                        
        xor      eax, edx
        mov      edx, DWORD PTR @CatStr(_b,_i2)         ; _b2
        shr      edx, 8
        movzx    edx, dl
        movzx    edx, BYTE PTR _sBox[edx]
        shl      edx, 8
        xor      eax, edx
        movzx    edx, BYTE PTR @CatStr(_b,_i3)          ; _b3
        movzx    edx, BYTE PTR _sBox[edx]
        xor      eax, edx
        mov      DWORD PTR @CatStr(_a,_i0), eax         ; _a0
        ENDM

        ; 10th/12th/14th Final Round
        ; AES_LAST_ROUND _X, _Y, 0,1,2,3
AES_LAST_ROUND MACRO _a, _b, _i0, _i1, _i2, _i3
        LOCAL    ???aeslr1, ???aeslr2

        mov      ecx, DWORD PTR _RK                             ; RK += 4; ; ecx = RK   
        add      ecx, 4*SIZEOF DWORD                                                    
        cmp      DWORD PTR aesMode, _ctxStruct.drk              ; Check: ENCRYPT / DECIPHER
        je       ???aeslr1                                      
                                                                ; Encrypt   ; Decipher
        AES_LAST_ROUND_CODE _Fsb, _a, _b, _i0, _i1, _i2, _i3    ; 0,1,2,3   ; 0,3,2,1
        AES_LAST_ROUND_CODE _Fsb, _a, _b, _i1, _i2, _i3, _i0    ; 1,2,3,0   ; 3,2,1,0
        AES_LAST_ROUND_CODE _Fsb, _a, _b, _i2, _i3, _i0, _i1    ; 2,3,0,1   ; 2,1,0,3
        AES_LAST_ROUND_CODE _Fsb, _a, _b, _i3, _i0, _i1, _i2    ; 3,0,1,2   ; 1,0,3,2
        jmp      ???aeslr2                                                              

???aeslr1:                                                      ; Encrypt   ; Decipher
        AES_LAST_ROUND_CODE _Rsb, _a, _b, _i0, _i3, _i2, _i1    ; 0,1,2,3   ; 0,3,2,1
        AES_LAST_ROUND_CODE _Rsb, _a, _b, _i3, _i2, _i1, _i0    ; 1,2,3,0   ; 3,2,1,0
        AES_LAST_ROUND_CODE _Rsb, _a, _b, _i2, _i1, _i0, _i3    ; 2,3,0,1   ; 2,1,0,3
        AES_LAST_ROUND_CODE _Rsb, _a, _b, _i1, _i0, _i3, _i2    ; 3,0,1,2   ; 1,0,3,2
???aeslr2:
        ENDM


;-----------------------------------------------------------
;
;       Initialization Macros
;
        ; *SK++ = KT0[ (uint8) ( *RK >> 24 ) ] ^
        ;         KT1[ (uint8) ( *RK >> 16 ) ] ^
        ;         KT2[ (uint8) ( *RK >>  8 ) ] ^
        ;         KT3[ (uint8) ( *RK       ) ]; RK++;
SET_KEY_CODE MACRO _i
        mov      eax, DWORD PTR [ecx+_i*SIZEOF DWORD]     ; eax = RK[0]
        shr      eax, 24
        movzx    eax, al
        mov      eax, DWORD PTR _KT0[eax*SIZEOF DWORD]
        mov      ebx, DWORD PTR [ecx+_i*SIZEOF DWORD]     ; ebx = RK[0]
        shr      ebx, 16
        movzx    ebx, bl
        xor      eax, DWORD PTR _KT1[ebx*SIZEOF DWORD]
        mov      ebx, DWORD PTR [ecx+_i*SIZEOF DWORD]     ; ebx = RK[0]
        shr      ebx, 8
        movzx    ebx, bl
        xor      eax, DWORD PTR _KT2[ebx*SIZEOF DWORD]
        movzx    ebx, BYTE PTR [ecx+_i*SIZEOF DWORD]      ; ebx = RK[0]
        xor      eax, DWORD PTR _KT3[ebx*SIZEOF DWORD]
        mov      DWORD PTR [edx+_i*SIZEOF DWORD], eax     ; SK[0] = eax
        ENDM

        ; SET_KEY 0,1,2,3
SET_KEY MACRO _i0, _i1, _i2, _i3
        SET_KEY_CODE _i0
        SET_KEY_CODE _i1
        SET_KEY_CODE _i2
        SET_KEY_CODE _i3
        ENDM


        ; RK[4]  = RK[0] ^ RCON[i] ^
        ;     ( FSb[ (uint8) ( RK[3] >> 16 ) ] << 24 ) ^
        ;     ( FSb[ (uint8) ( RK[3] >>  8 ) ] << 16 ) ^
        ;     ( FSb[ (uint8) ( RK[3]       ) ] <<  8 ) ^
        ;     ( FSb[ (uint8) ( RK[3] >> 24 ) ]       );
        ; KEYSIZE_SBOX_RCON 3,4                           ; Params: 128/32-1, 128/32
        ; KEYSIZE_SBOX_RCON 5,6                           ; Params: 192/32-1, 192/32
        ; KEYSIZE_SBOX_RCON 7,8                           ; Params: 256/32-1, 256/32
KEYSIZE_SBOX_RCON MACRO _i3, _i4
        mov      eax, DWORD PTR [ecx+0*SIZEOF DWORD]      ; eax = RK[0]
        xor      eax, DWORD PTR _RCON[edx*SIZEOF DWORD]   ; RCON[i]
        mov      edx, DWORD PTR [ecx+_i3*SIZEOF DWORD]    ; RK[3]
        shr      edx, 16
        movzx    edx, dl
        movzx    edx, BYTE PTR _FSb[edx]
        shl      edx, 24
        xor      eax, edx                                 
        mov      edx, DWORD PTR [ecx+_i3*SIZEOF DWORD]    ; RK[3]
        shr      edx, 8
        movzx    edx, dl
        movzx    edx, BYTE PTR _FSb[edx]
        shl      edx, 16
        xor      eax, edx
        movzx    edx, BYTE PTR [ecx+_i3*SIZEOF DWORD]     ; RK[3]
        movzx    edx, BYTE PTR _FSb[edx]
        shl      edx, 8
        xor      eax, edx
        mov      edx, DWORD PTR [ecx+_i3*SIZEOF DWORD]    ; RK[3]
        shr      edx, 24
        movzx    edx, dl
        movzx    edx, BYTE PTR _FSb[edx]
        xor      eax, edx
        mov      DWORD PTR [ecx+_i4*SIZEOF DWORD], eax    ; RK[4] = eax
        ENDM

        ; ROTR8(x) ( ( ( x << 24 ) & 0xFFFFFFFF ) | \
        ;            ( ( x & 0xFFFFFFFF ) >> 8 ) )
        ; RT1[i] = ROTR8( RT0[i] );
        ; AES_ROTATE_I _FT0, _FT1
AES_ROTATE_I MACRO _rt1, _rt0
        mov      edx, DWORD PTR _rt1[ecx*SIZEOF DWORD]
        shl      edx, 24
        mov      eax, DWORD PTR _rt1[ecx*SIZEOF DWORD]
        shr      eax, 8
        or       edx, eax
        mov      DWORD PTR _rt0[ecx*SIZEOF DWORD], edx   ; RT1[i]
        ENDM

        ; MUL(i,y) ( ( i && y ) ? pow[(log[i] + log[y]) % 255] : 0 );
        ; AES_DIV_GF 11
        ; AES_DIV_GF 13
        ; AES_DIV_GF 09
        ; AES_DIV_GF 14
AES_DIV_GF MACRO _y
        movzx    eax, BYTE PTR _log[_y]  ; eax = _log[0x0B], ecx = log[y]
        movzx    ecx, BYTE PTR _log[edx] ; The CDQ instruction
        add      eax, ecx                ;  copies the sign (bit 31) of the value in the EAX register
        cdq                              ;   into every bit position in the EDX register [edx::eax].
        mov      ecx, 255                ; IDIV divides the (signed) value in the [edx::eax]
        idiv     ecx                     ;  by operand (divisor) - result in the [edx::eax] registers.
        movzx    edx, BYTE PTR _pow[edx] ; pow[(log[i] + log[y]) % 255];  
        ENDM