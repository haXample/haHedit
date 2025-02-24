// hedit_vi.cpp - C++ Developer source file.
// (c)2021 by helmut altmann

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

#include "hedit_cpp.h"      // Include the common declarations

#include <conio.h>
#include <string>           // For printf, etc.
                            
#include <windows.h>        // C++ Windows10 console general definitions

using namespace std;
      
//----------------------------------------------------------------------------
//                          external declarations
//----------------------------------------------------------------------------
extern REGS regs, inregs, outregs;                     // BIOS (INT10h) interface
extern unsigned char v_mode, v_attrib, v_page, vsave_page;  // current video parameters

//----------------------------------------------------------------------------
//                          global declarations
//----------------------------------------------------------------------------

// typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
//   COORD      dwSize;
//   COORD      dwCursorPosition;
//   WORD       wAttributes;
//   SMALL_RECT srWindow;
//   COORD      dwMaximumWindowSize;
// } CONSOLE_SCREEN_BUFFER_INFO;
CONSOLE_SCREEN_BUFFER_INFO csbi = { };
CONSOLE_SCREEN_BUFFER_INFO std_csbi = { };
CONSOLE_SCREEN_BUFFER_INFO debug_csbi = { };

// typedef struct _COORD {
//   SHORT X;
//   SHORT Y;
// } COORD, *PCOORD;
COORD pos, debug_pos, std_pos;

DWORD mode = 0;
DWORD originalMode;

HANDLE hConsoleStd, hConsoleHedit, hConsoleSave;

SMALL_RECT srctRect;        // Structure for a rectangle screen section

CHAR_INFO chiBuffer[25*80]; // [25][80];
COORD coordBufSize;
COORD coordBufCoord;
BOOL fSuccess;


//----------------------------------------------------------------------------
//                          Fuction Prototypes
//----------------------------------------------------------------------------
void __SetCursor(SHORT, SHORT);


//@001************************************************************************
//----------------------------------------------------------------------------
//                          DebugPrintBuffer
//
//  Usage example:  DebugPrintBuffer(buffer, i);
//
void DebugPrintBuffer(char *buf, int count)
  {
  int i;

  printf("Buf: ");
  for (i=0; i<count; i++) printf("%02X ", (unsigned char)buf[i]);
  printf("\n");
  printf("-- press 'q' for exit --\n");
  if (_getch() == 'q') exit(0);
  } // DebugPrintBuffer

//----------------------------------------------------------------------------
//                          DebugStopBuf
//
//  Usage example:  DebugStop("__SaveVideoPage" , i);
//
void DebugStopBuf(unsigned char _char, unsigned long num)
  {
  while (_kbhit() != 0) _getch();   // flush key-buffer 
  printf("\n%02X [%08X]\n", _char, num);
  printf("-- press 'q' for exit --\n");
  if (_getch() == 'q') exit(0);
  } // DebugStopBuf

//----------------------------------------------------------------------------
//                          DebugStop
//
//  Usage example:  DebugStop("__SaveVideoPage" , i);
//
void DebugStop(char * _info, int num)
  {
  while (_kbhit() != 0) _getch();   // flush key-buffer 
  printf("\n%s [%04X]\n", _info, num);
  printf("-- press 'q' for exit --\n");
  if (_getch() == 'q') exit(0);
  } // DebugStop

//----------------------------------------------------------------------------
//
//                          DebugCursor_SET_XY
//
void DebugCursor_SET_XY(SHORT _x, SHORT _y)
  {
  GetConsoleScreenBufferInfo (hConsoleHedit, &debug_csbi);  // Save cursor
//ha//  debug_pos = {0, 20};                                // Debug display position
  debug_pos.X =  0;
  debug_pos.Y = 20;                                     // Debug display position
  SetConsoleCursorPosition(hConsoleHedit, debug_pos);       

  while (_kbhit() != 0) _getch();                       // flush key-buffer 
  printf("__DebugCursor_SET_XY() x=%02d, y=%02d       \n", _x, _y);
  printf("-- press 'q' for exit --");
  if (_getch() == 'q') exit(0);

//ha//  debug_pos = {debug_csbi.dwCursorPosition.X, debug_csbi.dwCursorPosition.Y};
  debug_pos.X = debug_csbi.dwCursorPosition.X;
  debug_pos.Y = debug_csbi.dwCursorPosition.Y;              // Debug display position
  SetConsoleCursorPosition(hConsoleHedit, debug_pos);       // Restore cursor
  }  // DebugCursor_SET_XY

//----------------------------------------------------------------------------
//
//                          DebugCursor_SHOW_POS
//
void DebugCursor_SHOW_POS(int nr, unsigned char x1,unsigned char y1,
                          unsigned char x2,unsigned char y2)
  {
  GetConsoleScreenBufferInfo (hConsoleHedit, &debug_csbi);  // Save cursor
//ha//  debug_pos = {0, 20};                                      // Debug display position
  debug_pos.X =  0;
  debug_pos.Y = 20;                                     // Debug display position
  SetConsoleCursorPosition(hConsoleHedit, debug_pos);       

  while (_kbhit() != 0) _getch();                           // flush key-buffer 
  printf("__DebugCursor_SHOW_POS(%02d) x=%02d, y=%02d - [%02d, %02d, %02d, %02d]  \n",
            nr, debug_csbi.dwCursorPosition.X, debug_csbi.dwCursorPosition.Y, x1,y1,x2,y2);
  printf("-- press 'q' for exit --");
  if (_getch() == 'q') exit(0);

//ha//  debug_pos = {debug_csbi.dwCursorPosition.X, debug_csbi.dwCursorPosition.Y};
  debug_pos.X = debug_csbi.dwCursorPosition.X;
  debug_pos.Y = debug_csbi.dwCursorPosition.Y;              // Debug display position
  SetConsoleCursorPosition(hConsoleHedit, debug_pos);       // Restore cursor
  }  // DebugCursor_SHOW_POS

//****************************************************************************

//----------------------------------------------------------------------------
//
//                          put_fast_ch 
//
//  mov   ah,0Eh  ; Print the ascii character in (al)
//  push  bp      ; save caller's frame pointer (IBM-XT will destroy it!)
//  int   10h     ; Video I/O
//  pop   bp      ; Restore caller's frame pointer
//
void put_fast_ch(unsigned char c)
  {
  SetConsoleTextAttribute(hConsoleHedit, v_attrib);
  _putch(c);         
  } // put_fast_ch

//----------------------------------------------------------------------------
//
//                          put_fast_hex
//
// conv_asc_al  MACRO
//  and al,0Fh    ; Convert lsb of (AL) hex to ASCII
//  add al,90h
//  daa
//  adc al,40h
//  daa           ; (al) = ASCII
// ENDM
//
//  mov   ah,0Eh  ; Print the ascii character in (al)
//  push  bp      ; save caller's frame pointer (IBM-XT will destroy it!)
//  int   10h     ; Video I/O
//  mov   al,' '  ; Append a space
//  mov   ah,0Eh  ; Print the ascii character
//  push  bp      ; save caller's frame pointer (IBM-XT will destroy it!)
//  int   10h     ; Video I/O
//  pop   bp      ; Restore caller's frame pointer
//
// !! MS-Bug: Can't use "printf"  on "hConsoleHedit" !!
//            (see __Save / __Set / __RestoreVideoPage)
//            'printf("%02X ", (int)(char *)(c & 0xFF));' ????
//            'printf("%02X ", (UCHAR)c);' is Okay
// 
void put_fast_hex(unsigned char c)  
  {                                  
  char __c;                          

  SetConsoleTextAttribute(hConsoleHedit, v_attrib);

#ifdef x64      // 64bit Version
  printf("%02X ", (UCHAR)c);
#else           // 32bit Version
  __asm                              
    {                                
    mov al,c    ; Get ascii char
    shr al,4    ; msb
    and al,0Fh  ; Convert msb of (AL) hex to ascii
    add al,90h
    daa
    adc al,40h
    daa
    mov __c,al
    }
 _putch(__c);   // Display msb

  __asm
    {
    mov al,c    ; Get ascii char
    and al,0Fh  ; Convert lsb of (AL) hex to ascii
    add al,90h
    daa
    adc al,40h
    daa
    mov __c,al
    }
  _putch(__c);  // Display lsb
  _putch(' ');  // Print a space
#endif
  } // put_fast_hex


//----------------------------------------------------------------------------
//
//                          __SetCursor
//
//  inregs.h.ah = 0x02;           // set cursor
//  inregs.h.bh = v_page;         // page
//  inregs.h.dl = --x;            // column
//  inregs.h.dh = --y;            // row
//  int86(0x10, &inregs, &regs);  // BIOS - DOS 16bit Interface
//
void __SetCursor(SHORT x, SHORT y)
  {
//ha//  pos = {x, y};
  pos.X = x;
  pos.Y = y;                                      // Debug display position
  SetConsoleCursorPosition(hConsoleHedit, pos);
//@001DebugCursor_SHOW_POS(20);
//@001DebugCursor_SET_XY(x, y);
  } // __SetCursor


//----------------------------------------------------------------------------
//
//                      __SetSpaceAttrib
//
//  inregs.h.ah = 0x09;               // write char with attribute
//  inregs.h.al = ' ';                // space char
//  inregs.h.bl = v_attrib;           // char attribute
//  inregs.h.bh = v_page;             // video page
//  inregs.h.cl = cnt;                // number of spaces to write (lsb)
//  inregs.h.ch = 0;                  // # of spaces (msb)
//  int86(0x10, &inregs, &outregs)    // BIOS - DOS 16bit Interface
//
void __SetSpaceAttrib(unsigned char cnt)
  {
  int i;
  DWORD _dw;

  SetConsoleTextAttribute(hConsoleHedit, v_attrib);
  GetConsoleScreenBufferInfo (hConsoleHedit, &csbi);  // Save cursor

//ha//    FillConsoleOutputCharacter(
//ha//      hConsoleHedit,
//ha//      (TCHAR) ' ',
//ha//      cnt,
//ha//      pos,
//ha//      &_dw
//ha//      );    //ha// Wont work properly problems with attributes later;

  cnt--;                     //ha// XP Last char is special (screen scrolling)
  for (i=0; i<cnt; i++)
    {                          
    _putch(' ');
    }

  pos.X = 79;                 //ha// XP Last char is special  (prevents scrolling)
  pos.Y = csbi.dwCursorPosition.Y;  
  FillConsoleOutputAttribute(
      hConsoleHedit,
      v_attrib,
      1,
      pos,
      &_dw
      );

//  pos = {csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y};  // VS 2010 problems
  pos.X = csbi.dwCursorPosition.X;
  pos.Y = csbi.dwCursorPosition.Y;              // Debug display position
  SetConsoleCursorPosition(hConsoleHedit, pos); // Restore cursor
  } // __SetSpaceAttrib


//----------------------------------------------------------------------------
//
//                      __AttrDisplayChar
//
//  inregs.h.ah = 0x19;               // write char with attribute (ah=0x09)
//  inregs.h.al = c;                  // char to be displayed
//  inregs.h.bl = v_attrib;           // char attribute
//  inregs.h.bh = v_page;             // video page
//  inregs.h.cl = 1;                  // number of characters to write (lsb)
//  inregs.h.ch = 0;                  // # of chars (msb)
//  int86(0x10, &inregs, &outregs)  // BIOS - DOS 16bit Interface
//
void __AttrDisplayChar(char c)
  {
  SetConsoleTextAttribute(hConsoleHedit, v_attrib);
  _putch(c);
  } // __AttrDisplayChar


//----------------------------------------------------------------------------
//
//                          __SkipColumn
//
//  inregs.h.ah = 0x03;               // get cursor
//  inregs.h.bh = v_page;             // video page
//  inregs.h.cl = n;                  //@001 number of spaces to write (lsb)
//  int86(0x10, &inregs, &outregs);
//  _SetCursor((UCHAR)outregs.h.dh +1, (UCHAR)outregs.h.dl +n+1);  //(Y,X)
//  int86(0x10, &inregs, &outregs)    // BIOS - DOS 16bit Interface
//
void __SkipColumn(unsigned char n)
  {
  GetConsoleScreenBufferInfo (hConsoleHedit, &csbi);
  __SetCursor(csbi.dwCursorPosition.X+n, csbi.dwCursorPosition.Y);
  } // __SkipColumn


//----------------------------------------------------------------------------
//
//                        __ClearRestOfRow
//
//  unsigned char cnt;
//  
//  inregs.h.ah = 0x13;                 // get cursor  (ah=0x03)
//  inregs.h.bh = v_page;               // video page
//  int86(0x10, &inregs, &outregs);
//  cnt = 80 - outregs.h.dl;            // space to end of row
//  SetSpaceAttrib(cnt);
//  int86(0x10, &inregs, &outregs)      // BIOS - DOS 16bit Interface
//
void __ClearRestOfRow()
  {
  unsigned char cnt;

  GetConsoleScreenBufferInfo (hConsoleHedit, &csbi);    // get cursor (video page=
  cnt = 80 - csbi.dwCursorPosition.X;                 // space to end of row
  __SetSpaceAttrib(cnt);
  } // __ClearRestOfRow


//----------------------------------------------------------------------------
//
//                          __ClearWindow
//
//  inregs.h.ah = 0x06;               // write char with attribute
//  inregs.h.al = 0;                  // clear
//  inregs.h.bh = v_attrib;           // char attribute
//  inregs.h.ch = urow;               // y1 = upper row          = 4 (Example)         
//  inregs.h.cl = lcolumn;            // x1 = upper left column  = 0 
//  inregs.h.dh = brow;               // y2 = bottom row         =18         
//  inregs.h.dl = rcolumn;            // x2 = lower right column =79
//  int86(0x10, &inregs, &outregs);   // BIOS - DOS 16bit Interface
//
void __ClearWindow(unsigned char y1, unsigned char x1, unsigned char y2, unsigned char x2)
  {
  int ri, ci;
  COORD __pos;

//ha//DebugCursor_SHOW_POS(10,y1,x1,y2,v_attrib);

//ha//  __pos = {csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y};
  __pos.X = csbi.dwCursorPosition.X;
  __pos.Y = csbi.dwCursorPosition.Y;              // Debug display position

  SetConsoleTextAttribute(hConsoleHedit, v_attrib);
  __SetCursor(x1, y1);                                

  ci=x1;
  ri=y1;
  while (ri <= y2)             // Row (vertical -> Y)
    {
    __SetCursor(x1, ri);                                
    for (ci=x1; ci<=x2; ci++)  // Column (horizontal -> X)
      {
      _putch(' ');             // Space-out the column positions in the row
      }
    ri++;                      // Next row
    } 

  SetConsoleCursorPosition(hConsoleHedit, __pos);        // Restore cursor
  } // __ClearWindow


//----------------------------------------------------------------------------
//
//                        __SetVideoPage
//
//  if (vsave_page < 7) v_page = vsave_page+1;
//  else v_page = vsave_page-1;
//  inregs.h.ah = 0x05;              // Set (restore previous) video page
//  inregs.h.al = v_page;
//  int86(0x10, &inregs, &outregs)   // BIOS - DOS 16bit Interface
//
void __SetVideoPage()
  {
  // Activate the original std screen buffer for HEDIT screen.
  hConsoleHedit = hConsoleStd;    // Force Hedit to use the Std Screen.
  SetConsoleActiveScreenBuffer(hConsoleHedit);
                                 // Must use std-Console because
                                // "printf" won't work on
  } // __SetVideoPage          // hConsoleSave = CreateConsoleScreenBuffer

  
//----------------------------------------------------------------------------
//
//                          __RestoreVideoPage
//
//  inregs.h.ah = 0x05 (==0x15!);   // Set (restore saved) video page  (ah=0x05)
//  inregs.h.al = vsave_page;
//  int86(0x10, &inregs, &outregs); // BIOS - DOS 16bit Interface
//
void __RestoreVideoPage()  // Restore the original active screen buffer.
  {
  SetConsoleActiveScreenBuffer(hConsoleStd);
  SetConsoleCursorPosition(hConsoleStd, std_pos); // Restore cursor position

  hConsoleHedit = hConsoleStd;       // Must use std-Console because
                                    // "printf" won't work on
                                   // hConsoleSave = CreateConsoleScreenBuffer
  
  // Here we copy the contents of "hConsoleSave = CreateConsoleScreenBuffer"
  // back to "SetConsoleActiveScreenBuffer(hConsoleStd)" - its original contents.
  
  // Set the source/destination rectangle (both 25x80 chars, starting top left).
  srctRect.Top    =  0;    // top left: row 0, col 0
  srctRect.Left   =  0;
  srctRect.Bottom = 24;    // bottom right: row 24, col 79
  srctRect.Right  = 79;

  // The temporary buffer size is 25 rows x 80 columns.
  coordBufSize.Y = 25;
  coordBufSize.X = 80;

  // The top left destination cell of the temporary buffer is row 0, col 0.
  coordBufCoord.X = 0;
  coordBufCoord.Y = 0;

  // Read the save std data from the hConsoleSave buffer into the chiBuffer.
  fSuccess = ReadConsoleOutput(
     hConsoleSave,      // temp screen buffer to read from
     chiBuffer,         // buffer to intercept the screen contents
     coordBufSize,      // col-row size of chiBuffer
     coordBufCoord,     // top left dest. cell in chiBuffer
     &srctRect);        // screen buffer source rectangle

  // Restore the std data in chiBuffer back to the std screen buffer.
  fSuccess = WriteConsoleOutput(
      hConsoleStd,      // screen buffer to write to
      chiBuffer,        // buffer to copy from
      coordBufSize,     // col-row size of chiBuffer
      coordBufCoord,    // top left src cell in chiBuffer
      &srctRect);       // screen buffer destination rectangle

    // Actvate the original std screen buffer.
    SetConsoleActiveScreenBuffer(hConsoleStd);
  } // __RestoreVideoPage


//----------------------------------------------------------------------------
//
//                          __SaveVideoPage
//
//  inregs.h.ah = 0x0F;               // Get mode
//  int86(0x10, &inregs, &outregs);
//  v_mode = outregs.h.al;            // Save mode
//  vsave_page = outregs.h.bh;        // Save original page (previous user)
//  if (vsave_page < 7) v_page = vsave_page+1;
//  else v_page = vsave_page-1;
//  inregs.h.ah = 0x05;               // Set Hedit video page
//  inregs.h.al = v_page;
//  int86(0x10, &inregs, &outregs);   // BIOS - DOS 16bit Interface
//
void __SaveVideoPage()
  {
  // Get a handle to the STDOUT screen buffer to copy from.
  hConsoleStd = GetStdHandle(STD_OUTPUT_HANDLE);
  // Save std screen cursor position
  GetConsoleScreenBufferInfo (hConsoleStd, &std_csbi);
//ha//  std_pos = {std_csbi.dwCursorPosition.X, std_csbi.dwCursorPosition.Y};
  std_pos.X = std_csbi.dwCursorPosition.X;
  std_pos.Y = std_csbi.dwCursorPosition.Y;              // Debug display position

  v_mode = _VCOLOR;                       // Mode = Color videomode
 
  // Create a new screen buffer to save the std Screen contents.
  hConsoleSave = CreateConsoleScreenBuffer(
     GENERIC_READ | GENERIC_WRITE,        // read/write access
     FILE_SHARE_READ | FILE_SHARE_WRITE,
     NULL,
     CONSOLE_TEXTMODE_BUFFER,             // must be TEXTMODE
     NULL);                               // reserved; must be NULL

  // Make the new screen buffer the active screen buffer.
  SetConsoleActiveScreenBuffer(hConsoleSave);

  // Set the source/destination rectangle (both 25x80 chars, starting top left).
  srctRect.Top    =  0;    // top left: row 0, col 0
  srctRect.Left   =  0;
  srctRect.Bottom = 24;    // bottom right: row 24, col 79
  srctRect.Right  = 79;

  // The temporary buffer size is 2 rows x 80 columns.
  coordBufSize.Y = 25;
  coordBufSize.X = 80;

  // The top left destination cell of the temporary buffer is row 0, col 0.
  coordBufCoord.X = 0;
  coordBufCoord.Y = 0;

  // Read and save the std data from the hConsolestd buffer into the chiBuffer.
  fSuccess = ReadConsoleOutput(
     hConsoleStd,       // screen buffer to read from
     chiBuffer,         // buffer to copy into
     coordBufSize,      // col-row size of chiBuffer
     coordBufCoord,     // top left dest. cell in chiBuffer
     &srctRect);        // screen buffer source rectangle
//@001    if (! fSuccess)
//@001    {
//@001        printf("ReadConsoleOutput failed - (%d)\n", GetLastError());
//@001        return 1;
//@001    }

  // Set the destination rectangle.
  // Save and store the std data in chiBuffer into the hConsoleSave buffer.
  fSuccess = WriteConsoleOutput(
      hConsoleSave,     // screen buffer to write to
      chiBuffer,        // buffer to copy from
      coordBufSize,     // col-row size of chiBuffer
      coordBufCoord,    // top left src cell in chiBuffer
      &srctRect);       // destination screen buffer rectangle
//@001    if (! fSuccess)
//@001    {
//@001        printf("WriteConsoleOutput failed - (%d)\n", GetLastError());
//@001        return 1;
//@001    }
//@001    Sleep(5000);  // Give the system time to settle the copy process

  // Activate the original std screen buffer for HEDIT screen.
  SetConsoleActiveScreenBuffer(hConsoleStd);
  hConsoleHedit = hConsoleStd;    // Force Hedit to use the Std Screen.
                                 // Must use std-Console because
                                // "printf" won't work on
  } //__SaveVideoPage          // hConsoleSave = CreateConsoleScreenBuffer
                                 
//@001  // ----------TEST ONLY, NOT USED HERE------------------------------
//@001  GetConsoleMode(hConsoleHedit, &dwMode);
//@001
//@001  // Set output mode to handle virtual ANSII terminal sequences
//@001  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
//@001  SetConsoleMode(hConsoleHedit, dwMode);
//@001
//@001  // Try some Set Graphics Rendition (SGR) terminal escape sequences
//@001  printf("\x1b[31mThis text has a red foreground using SGR.31.\r\n");
//@001  // ----------TEST ONLY, NOT USED HERE------------------------------


//----------------------------------------------------------------------------
//
//                          __int86
//
//  Interface converting BIOS 16bit viodeo INt10h into Windows 32bit functions
//
//
void __int86(int ConsoleMode, union REGS * inregs, union REGS * outregs)
  {
  switch((unsigned char)inregs->h.ah)
    {
    case 0x00:           // Set mode
      break;
    case 0x02:           // Set cursor(x,y)
      __SetCursor(inregs->h.dl, inregs->h.dh);
      break;
    case 0x03:           // Get cursor 0x03
      __SkipColumn(inregs->h.cl);
      break;
    case 0x13:           // Get cursor 0x03+0x13
      __ClearRestOfRow();
      break;
    case 0x05:           // Set video page 0x05+0x15
      __SetVideoPage();
      system("cls");     // Clear screen
      break;
    case 0x15:           // Restore video page
      __RestoreVideoPage();
      break;
    case 0x06:          // Write rectangular block with attribute   
      __ClearWindow(inregs->h.ch, inregs->h.cl, inregs->h.dh, inregs->h.dl);
      break;
    case 0x09:           // Write space with attribute
      __SetSpaceAttrib(inregs->h.cl);
      break;
    case 0x19:           // Write char with attribute
      __AttrDisplayChar(inregs->h.al);
      break;
    case 0x0F:           // Save and set video console
      __SaveVideoPage();
      __SetVideoPage();   
      break;
    default:
      break;
    } // end Switch
  } // __int86

//--------------------------end-of-module------------------------------------





                              