// hedit_cpp.h - C++ Developer source file.
// (c)2021 by helmut altmann

//  -------------------------------------------------
// |     Copyright (c)1986-2021  Helmut Altmann      |
//  -------------------------------------------------
//
//  $_GENERAL   : Initial coding.
//  $_TECHNICAL : 
//  $_SCOPE     : V1.5
//  $_CHANGEDID : 0000
//  $_DATE      : 12.05.2021
//  $_AUTHOR    : HelmutAltmann
//  
//  $Revision:   1.0  $
//  $Date:   12 May 2021 18:49:50  $
//  $Archive:   C:\@WrkDrv\TOOLBOX\CONSOLE_APP\Hedit\32&64bit\Hedit V1.5/hedit_cpp.h  $
//

#pragma pack(1)                        // pack structure on one-byte boundaries
#pragma pack( )                        // pack structure on previous setting

//----------------------------------------------------------------------------
//                       constant declarations                                
//----------------------------------------------------------------------------
//@001#define UCHAR unsigned char
//@001#define UINT unsigned int
//@001#define ULONG unsigned long int

#define TRUE   1
#define FALSE  0
#define ERR   -1

#define ODD   1
#define EVN   0
#define INIT  2

#define RADIX_10  10
#define RADIX_16  16

#define _VCOLOR       0             // Video modes (Color= Default)
#define _VMONOCHROME  7

#define NORMAL 0                    // Video text attributes in use:
#define INVERT 1
#define LITE   2
#define HILITE 3
#define EDIT   4
#define ALERT  5
                                    // HEDIT modes
#define MOD_FILE    0x00            
#define MOD_MEMORY  0x05
#define MOD_HDD     0x80            // @003 Hard disk sector edit mode

#define HEDIT_EXE   1               // @005 
#define AES_EXE     2               // @005 
#define DES_EXE     3               // @005 

#define _1ST_INIT     1             // @001
#define _QUIT_INIT    0             // @001

#define _PUT_ASC_HEX2BIN 1          // @006
#define _PUT_RAW         0          // @006

#define _NORM_CURSOR  0             // @001
#define _BLOCK_CURSOR 1             // @001

#define _NORM_ALIGNED  0             // @001
#define _BLOCK_ALIGNED 1             // @001

#define _NO_CHANGE    0             // @001
#define _CHANGE       1             // @001

#define _BLOCK_UPDATE 0             // @001
#define _FULL_UPDATE  1             // @001

#define _FULL_READ    0             // @001
#define _BLOCK_READ   1             // @001

#define _INPUT_FILE   0             // @001
#define _OUTPUT_FILE  1             // @001

#define _KEEP_TMP_FILE   0          // @001
#define _DELETE_TMP_FILE 1          // @001

//#define _BEEP      '\a'  //new in C++ ??
#define _BEEP      0x07
#define _CNTL_G    0x07
#define _RUBOUT    0x08
#define _LF        0x0A
#define _CR        0x0D
#define _CNTL_R    0x12
#define _ESC       0x1B

#define _EXTENDED_KEYCODE 0xE0      //@001
#define _CHOME            0x47
#define _CUP              0x48
#define _PAGEUP           0x49
#define _CLEFT            0x4B
#define _CRIGHT           0x4D
#define _CEND             0x4F
#define _CDOWN            0x50
#define _PAGEDOWN         0x51
#define _DELETE           0x53

#define _CTRL_CLEFT    0x73
#define _CTRL_CRIGHT   0x74
#define _CTRL_PAGEDOWN 0x76
#define _CTRL_PAGEUP   0x84
#define _CTRL_CUP      0x8D
#define _CTRL_CDOWN    0x91

#define _LENGTH    128
#define _SIZE      0x7000
#define _SIZE_MRG  0x1800
#define _MAX       0xFFFFFFFF //@001
#define _LONG_MAX  0xFFFFFFFF

//#define COPYRIGHTCS 0x152E  //+ '1'+'4'+' '  V1.4
//#define COPYRIGHTCS 0x153F  //+ '1'+'4'+'1'  V1.41
//#define COPYRIGHTCS 0x1540  //+ '1'+'4'+'2'  V1.42
//#define COPYRIGHTCS 0x1541  //+ '1'+'4'+'3'  V1.43
#define COPYRIGHTCS 0x000C    //+ '1'+'4'+'4'  V1.44
                              //+ '1'+'5'      v1.5   //@001

//----------------------------------------------------------------------------
//                   Text string definitions  (deprecated)
//----------------------------------------------------------------------------
//@001#define char no_room[] = "no room for: %s";
//@001#define open_failed "open failed on: %s"
//@001#define illegal_command "illegal command"
//@001#define illegal_expr "illegal expression"
//@001#define invalid_param "invalid parameter"
//@001#define has " has%s"
//@001#define have " have%s"
//@001#define been_written " been written"
//@001#define hex "Hex"
//@001#define dec "Dec"
//@001#define enter_addr_hex "Hex address: "
//@001#define enter_expr_dec "Dec expression: "
//@001#define enter_expr_hex "Hex expression: "
//@001#define calc_operators "Calc operators: () + - * / %% & | ^ ~ >> <<"
//@001#define block_update "block update %s"
//@001#define changes_lost "all changes lost (y/[n])?"
//@001#define evn "/EVN"
//@001#define hedit_tmp "HEDIT.TMP"

//----------------------------------------------------------------------------
//                        Structure definitions
//----------------------------------------------------------------------------
struct WORDREGS {          //@001
    unsigned int ax;       //@001
    unsigned int bx;       //@001
    unsigned int cx;       //@001
    unsigned int dx;       //@001
    unsigned int si;       //@001
    unsigned int di;       //@001
    unsigned int cflag;    //@001
  };                       //@001
                           //@001
  struct BYTEREGS {        //@001
    unsigned char al,ah;   //@001
    unsigned char bl,bh;   //@001
    unsigned char cl,ch;   //@001
    unsigned char dl,dh;   //@001
  };                       //@001
                           //@001
  union REGS {             //@001
    struct WORDREGS x;     //@001
    struct BYTEREGS h;     //@001
  };                       //@001

//----------------------------  END OF MODULE  -----------------------------
