// hedit_v.cpp - C++ Developer source file.
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

#include <string>           // For printf, etc.
#include <windows.h>        // C++ Windows10 console general definitions

using namespace std;

//----------------------------------------------------------------------------
//                          external declarations
//----------------------------------------------------------------------------
extern void DebugCursor_SHOW_POS(int, unsigned char,unsigned char,
                                      unsigned char,unsigned char);

extern void __int86(int, REGS *, REGS *);

extern char argv_name[_LENGTH+5];
                                                                                                                       
extern unsigned char hedit_mode, row24_flag, row25_flag;
extern unsigned long offset;

extern int repos_offset, drvSector;

//----------------------------------------------------------------------------
//                          global declarations
//----------------------------------------------------------------------------
union REGS regs, inregs, outregs;       // for BIOS device service call (INT10h)
unsigned char v_mode, v_attrib, v_page, vsave_page;  // current video parameters

void SetSpaceAttrib(unsigned char);
void DisplayFDHDSector(char *, int);

//---------------------------------------------------------------------------
//
//                              GetVidAttrib
//
unsigned char GetVidAttrib(unsigned char attr)
  {
//                                 NORMAL,INVERT,LITE,  HILITE,EDIT, ALERT
//@001unsigned char v_color[6] =  {0x07,  0x70,  0x7F,  0x1F,  0x30, 0x4F}; // VGA color
//@001unsigned char v_color[6] =  {0x07,  0x70,  0x0C,  0x7F,  0x31, 0x4F}; // VGA color
//@001unsigned char v_color[6] =  {0x07,  0x70,  0x7F,  0x1F,  0x17, 0x4F}; // VGA color

  unsigned char v_color[6] =  {0x07,  0x70,  0x27,  0x1F,  0x17, 0x4F}; //@001 VGA color
  unsigned char v_mono[6]  =  {0x07,  0x70,  0x70,  0x70,  0x07, 0xF0}; // monochrome

  return(v_mode==7 ? v_mono[attr] : v_color[attr]);   // xlat appropriately                                                 
  } // GetVidAttrib

//----------------------------------------------------------------------------
//
//                          SetAttribute
//
void SetAttribute(unsigned char attr)
  {
  v_attrib = GetVidAttrib(attr);
  } // SetAttribute

//----------------------------------------------------------------------------
//
//                         SetCursor (__int86)
//
void _SetCursor(unsigned char y, unsigned char x)
  {
  inregs.h.ah = 0x02;               // set cursor
  inregs.h.bh = v_page;             // page
  inregs.h.dl = --x;                // column
  inregs.h.dh = --y;                // row
  __int86(0x10, &inregs, &regs);
  } // _SetCursor

//----------------------------------------------------------------------------
//
//                         SkipColumn (__int86)
//
int SkipColumn(unsigned char n)
  {
  inregs.h.ah = 0x03;               // get cursor
  inregs.h.bh = v_page;             // video page
  inregs.h.cl = n;                  //@001 number of spaces to write (lsb)
  __int86(0x10, &inregs, &outregs);
//@001_SetCursor((unsigned char)outregs.h.dh +1,(unsigned char)outregs.h.dl+n+1);
  return(TRUE);
  } // SkipColumn

//----------------------------------------------------------------------------
//
//                        ClearRestOfRow (__int86)
//
void ClearRestOfRow()
  {
//@001  unsigned char cnt;
  inregs.h.ah = 0x13;               // get cursor
  inregs.h.bh = v_page;             // video page
  __int86(0x10, &inregs, &outregs);
//@001  cnt = 80 - outregs.h.dl;    // space to end of row
//@001  SetSpaceAttrib(cnt);
//@001  int86(0x10, &inregs, &outregs)      // BIOS - DOS 16bit Interface
  } // ClearRestOfRow

//----------------------------------------------------------------------------
//
//                       SetSpaceAttrib (__int86)
//
void SetSpaceAttrib(unsigned char cnt)
  {
  inregs.h.ah = 0x09;               // write char with attribute
  inregs.h.al = ' ';                // space char
  inregs.h.bl = v_attrib;           // char attribute
  inregs.h.bh = v_page;             // video page
  inregs.h.cl = cnt;                // number of spaces to write (lsb)
  inregs.h.ch = 0;                  // # of spaces (msb)
  __int86(0x10, &inregs, &regs);
  } // SetSpaceAttrib

//----------------------------------------------------------------------------
//
//                         AttrDisplayChar (__int86)
//
void AttrDisplayChar(unsigned char c)
  {
  inregs.h.ah = 0x19;               // write char with attribute
  inregs.h.al = c;                  // char to be displayed
  inregs.h.bl = v_attrib;           // char attribute
  inregs.h.bh = v_page;             // video page
  inregs.h.cl = 1;                  // number of spaces to write (lsb)
  inregs.h.ch = 0;                  // # of chars (msb)
  __int86(0x10, &inregs, &regs);
  } // AttrDisplayChar

//----------------------------------------------------------------------------
//
//                          AttrDisplayStr
//
void AttrDisplayStr(char *str)
  {
  while (*str)
    {
    AttrDisplayChar(*str);          // display char
    str++;                          // get next char
    }
  } // AttrDisplayStr

//----------------------------------------------------------------------------
//
//                         ClearWindow (__int86)
//
void ClearWindow(unsigned char urow, unsigned char lcolumn,
                 unsigned char brow, unsigned char rcolumn)
  {
  inregs.h.ah = 0x06;               // write char with attribute
  inregs.h.al = 0;                  // clear
  inregs.h.bh = v_attrib;           // char attribute
  inregs.h.ch = urow;               // y1 = upper row
  inregs.h.cl = lcolumn;            // x1 = upper left column
  inregs.h.dh = brow;               // y2 = bottom row
  inregs.h.dl = rcolumn;            // x2 = lower right column
  __int86(0x10, &inregs, &regs);
  } // ClearWindow

//----------------------------------------------------------------------------
//
//                        SaveVideoPage (__int86)
//
void SaveVideoPage()
  {
  inregs.h.ah = 0x0F;               // get mode
  __int86(0x10, &inregs, &outregs);
  v_mode = outregs.h.al;            // save mode
  vsave_page = outregs.h.bh;        // save page
  if (vsave_page < 7) v_page = vsave_page+1;
  else v_page = vsave_page-1;
  inregs.h.ah = 0x05;               // set video page
  inregs.h.al = v_page;
  __int86(0x10, &inregs, &outregs);
  } // SaveVideoPage

//----------------------------------------------------------------------------
//
//                         SetVideoPage (__int86)
//
void SetVideoPage()
  {
  if (vsave_page < 7) v_page = vsave_page+1;
  else v_page = vsave_page-1;
  inregs.h.ah = 0x05;               // set video page
  inregs.h.al = v_page;
  __int86(0x10, &inregs, &outregs);
  } // SetVideoPage

//----------------------------------------------------------------------------
//
//                         RestoreVideoPage (__int86)
//
void RestoreVideoPage()
  {
  inregs.h.ah = 0x15;               // Restore video page
  inregs.h.al = vsave_page;
  __int86(0x10, &inregs, &outregs);
  } // RestoreVideoPage

//----------------------------------------------------------------------------
//
//                          clr_data_field
//
void clr_data_field()
  {
  SetAttribute(EDIT);
  ClearWindow(3,0,18,79);   //@001
  } // clr_data_field

//----------------------------------------------------------------------------
//
//                           SetColumnRow25
//
void SetColumnRow25(int col)
  {
  _SetCursor(25, (unsigned char)col);
  SetAttribute(INVERT);
  } // SetColumnRow25

void SetRow25()
  {
  _SetCursor(25, 1);
  SetAttribute(NORMAL);
  ClearRestOfRow();
  } // SetRow25

//----------------------------------------------------------------------------
//
//                           ClrRow ...
//
void ClrRowInvert()
  {
  SetAttribute(NORMAL);
  ClearRestOfRow();
  SetAttribute(INVERT);
  } // ClrRowInvert

void ClrNextRow()
  {
  printf("\r\n");
  ClearRestOfRow();
  } // ClrNextRow

void ClrRow24()
  {
  _SetCursor(24, 7);         
  SetAttribute(NORMAL);
  ClearRestOfRow();
  row24_flag = FALSE;
  } // ClrRow24

void ClrRow25()
  {
  _SetCursor(25, 1);
  SetAttribute(NORMAL);
  ClearRestOfRow();
  SetAttribute(INVERT);
  SkipColumn(1);
  } // ClrRow25

void InvClrRow1()
  {
  _SetCursor(1, 1);
  SetAttribute(INVERT);
  ClearRestOfRow();
  SkipColumn(2);
  } // InvClrRow1

void InvClrRow25()
  {
  _SetCursor(25, 1);
  SetAttribute(INVERT);
  ClearRestOfRow();
  SkipColumn(1);
  } // InvClrRow25

//----------------------------------------------------------------------------
//
//                           ... Prompt
//
void BusyPrompt()
  {
  _SetCursor(24, 2);
  SetAttribute(NORMAL);
  AttrDisplayStr("-!!-");      // -!!-
  } // BusyPrompt

void ReadyPrompt()
  {
  _SetCursor(24, 2);
  SetAttribute(NORMAL);
  AttrDisplayStr("-\?\?-");    // -??-
  } // ReadyPrompt

//----------------------------------------------------------------------------
//
//                           RuboutStr
//
void RuboutStr()
  {
  putchar('\b');
  SetAttribute(NORMAL);
  ClearRestOfRow();
  SetAttribute(INVERT);
  } // RuboutStr

//----------------------------------------------------------------------------
//
//                           InvDisplayStart
//
void InvDisplayStart()
  {
  _SetCursor(4, 1);
  SetAttribute(HILITE);
  } // InvDisplayStart

//----------------------------------------------------------------------------
//
//                           NormDisplayStart
//
void NormDisplayStart()
  {
  _SetCursor(4, 1);
  SetAttribute(EDIT);
  ClearRestOfRow();
  } // NormDisplayStart

//----------------------------------------------------------------------------
//
//                           Skip ...
//
void SkipOffsetAddr()
  {
  SkipColumn(11);
  } // SkipOffsetAddr

void SkipHexChar()
  {
  SkipColumn(3);
  } // SkipHexChar

//----------------------------------------------------------------------------
//
//                           TabAsciiField
//
void TabAsciiField()
  {
  SkipColumn(2);
  } // TabAsciiField

//----------------------------------------------------------------------------
//
//                          ScreenPos
//
void ScreenPos(int y, int x)
  {
  _SetCursor((unsigned char)(y+4), (unsigned char)x);
  SetAttribute(EDIT);
  } // ScreenPos

//----------------------------------------------------------------------------
//
//                          InvScreenPos
//
void InvScreenPos(int y, int x)
  {
 _SetCursor((unsigned char)(y+4), (unsigned char)x);
  SetAttribute(HILITE);
  } // InvScreenPos

//----------------------------------------------------------------------------
//
//                          DisplayRow24
//
// Note: This is a function which accepts a variable number of arguments.
//       The implemetation follws the ANSI specification.
//
void DisplayRow24(char *format, ...)        // variable parameter list
  {                                         //
  va_list ap;                               // declare variable of type va_list
  char *nxt_str;                            // declare param retrieval ptr
                                            //
  ClrRow24();                               // clr and set cursor
  row24_flag = TRUE;                        // set pending message
                                            //
  va_start(ap, format);                     // set where to get next arg
  while (nxt_str = va_arg(ap, char *))      // get all parameters
    {                                       //
    printf(format, nxt_str);                // print-format next parameter
    }                                       //
  va_end(ap);                               // reset parameter stack
  } // DisplayRow24                         // return

//----------------------------------------------------------------------------
//
//                          _AttrDisplayHexChar
//
void AttrDisplayHexChar(unsigned char c)
  {
  char hbuf[3] = {0,0,0};

  SetSpaceAttrib(2);            // prepare attribute for 2 chars
  sprintf(hbuf, "%02X", c);     // emit hex chars into buffer for display
  AttrDisplayStr(hbuf);
  SkipColumn(1);                // leave on space for better legibility
  } // AttrDisplayHexChar

//----------------------------------------------------------------------------
//
//                          _AttrDisplayUserGuide
//
void AttrDisplayUserGuide(char *str)
  {
  ClrRow25();                         // to be displayed in row25
  AttrDisplayStr(str);
  repos_offset = strlen(str) + 2;
  row25_flag = TRUE;                  // automatically remove user guide
  } // AttrDisplayUserGuide

//----------------------------------------------------------------------------
//
//                          DisplayEditFilename
//
int DisplayEditFilename(char *filename)
  {
  char sbuf[_LENGTH+5];
  unsigned char stmp;
  int i, j;

  SetAttribute(EDIT);                 // prepare attribute for display
  ClearWindow(21,0,21,54);            // row22, col0..col55
  if (filename == 0) return(FALSE);   // nul pointer, dont try to display
  _SetCursor(22, 2);
  sprintf(sbuf, "Editing %s", filename);
  if ((j = strlen(filename)) > 40)
    {
    sbuf[18] = '.';                   // truncate filename
    sbuf[19] = '.';                   // truncate filename
    sbuf[20] = '.';                   // truncate filename
    sprintf(&sbuf[21], "%s", &filename[j-(51-21)]);
    sbuf[51] = 0;                     // truncate filename
    }
  AttrDisplayStr(sbuf);
  return(TRUE);
  } // DisplayEditFilename

//----------------------------------------------------------------------------
//
//                          DisplayMode
//
void DisplayMode(unsigned char mod)
  {
  _SetCursor(21, 56);
  SetAttribute(EDIT);
  if (mod != MOD_FILE) AttrDisplayStr("Mode: ");

  switch(mod)
    {
    case MOD_MEMORY:
      AttrDisplayStr("MEMORY");
      break;
    case MOD_HDD:                               //@003
      AttrDisplayStr("Hard Disk Sector");
      argv_name[0] = toupper(argv_name[0]);     // Force capital drive letter
      DisplayFDHDSector(argv_name, drvSector);  // HDD drive letter, sector
      break;
    case MOD_FILE:
      DisplayEditFilename(argv_name);           //Editing filename
      break;
    default:
      break;
    } // end switch
  } // DisplayMode


//----------------------------------------------------------------------------
//
//                          InitSscreen
//
void InitScreen()
  {
  SetVideoPage();
  SetAttribute(INVERT);
  ClearWindow(0,0,0,79);            // 1st row
  SetAttribute(NORMAL);
  ClearWindow(23,0,23,79);          // row24
  SetAttribute(EDIT);
  ClearWindow(1,0,22,79);           // row2..row23
  _SetCursor(2, 2);
  AttrDisplayStr("Address");
  SkipColumn(18);
  AttrDisplayStr("Hexadecimal");
  SkipColumn(29);
  AttrDisplayStr("ASCII");
  } // InitScreen

//----------------------------------------------------------------------------
//
//                          DefaultMenu
//
void DefaultMenu()
  {
  InvClrRow1();              // Clear menu help
  InvClrRow25();
  AttrDisplayStr("Block");
  SkipColumn(5);
  AttrDisplayStr("Calc");
  SkipColumn(5);
  AttrDisplayStr("Find");
  SkipColumn(5);
  AttrDisplayStr("Get");
  SkipColumn(4);
  AttrDisplayStr("Jump");
  SkipColumn(5);
  AttrDisplayStr("Quit");
  SkipColumn(4);
  AttrDisplayStr("Tag");
  SkipColumn(4);
  AttrDisplayStr("Xchange");
  SkipColumn(5);
  AttrDisplayStr("!system");
  ReadyPrompt();
  row25_flag = FALSE;        // Menu should be retained
  } // DefaultMenu

//----------------------------------------------------------------------------
//
//                           MenuHelp
//
void MenuHelp()
  {
  InvClrRow1();
  AttrDisplayStr("<ESC=Main Menu>");
  } // MenuHelp

//----------------------------------------------------------------------------
//
//                          BlockMenu
//
void BlockMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Chksum");
  SkipColumn(6);
  AttrDisplayStr("Dump");
  SkipColumn(6);
  AttrDisplayStr("Find");
  SkipColumn(6);
  AttrDisplayStr("Get");
  SkipColumn(6);
  AttrDisplayStr("Jump");
  SkipColumn(6);
  AttrDisplayStr("Put");
  SkipColumn(6);
  AttrDisplayStr("Scramble");
  SkipColumn(5);
  AttrDisplayStr("XFill");
  row25_flag = FALSE;        // Menu should be retained
  } // BlockMenu

//----------------------------------------------------------------------------
//
//                          SystemQuitMenu
//
void SystemQuitMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Abort");
  SkipColumn(7);
  AttrDisplayStr("Exit");
  SkipColumn(7);
  AttrDisplayStr("Init");
  SkipColumn(7);
  AttrDisplayStr("Write");
  row25_flag = FALSE;        // Menu should be retained
  } // QuitMenu

//----------------------------------------------------------------------------
//
//                          FileQuitMenu
//
void FileQuitMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Abort");
  SkipColumn(7);
  AttrDisplayStr("Bin2hex");
  SkipColumn(7);
  AttrDisplayStr("Exit");
  SkipColumn(7);
  AttrDisplayStr("Init");
  SkipColumn(7);
  AttrDisplayStr("Split");
  SkipColumn(7);
  AttrDisplayStr("Update");
  SkipColumn(7);
  AttrDisplayStr("Write");
  row25_flag = FALSE;        // Menu should be retained
  } // QuitMenu

//----------------------------------------------------------------------------
//
//                          QuitMenu
//
void QuitMenu()
  {
  if (hedit_mode == MOD_FILE) FileQuitMenu();
  else SystemQuitMenu();
  } // QuitMenu

//----------------------------------------------------------------------------
//
//                          JumpMenu
//
void JumpMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Address");
  SkipColumn(7);
  AttrDisplayStr("B_tag");
  SkipColumn(7);
  AttrDisplayStr("C_tag");
  SkipColumn(7);
  AttrDisplayStr("D_tag");
  SkipColumn(7);
  AttrDisplayStr("End");
  SkipColumn(7);
  AttrDisplayStr("Start");
  row25_flag = TRUE;         // Remove menu automatically
  } // JumpMenu

//----------------------------------------------------------------------------
//
//                          CalcMenu
//
void CalcMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Hex");
  SkipColumn(9);
  AttrDisplayStr("Dec");
  SkipColumn(9);
  AttrDisplayStr("?Help");
  row25_flag = TRUE;         // Remove menu automatically
  } // CalcMenu

//----------------------------------------------------------------------------
//
//                          TagMenu
//
void TagMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("B_tag");
  SkipColumn(7);
  AttrDisplayStr("C_tag");
  SkipColumn(7);
  AttrDisplayStr("D_tag");
  row25_flag = TRUE;         // Remove menu automatically
  } // TagMenu

//----------------------------------------------------------------------------
//
//                          XchangeMenu
//
void XchangeMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Hex");
  SkipColumn(9);
  AttrDisplayStr("ASCII");
  row25_flag = TRUE;         // Remove menu automatically
  } // XchangeMenu

//-@005-----------------------------------------------------------------------
//
//                          PutMenu
//
void PutMenu()
  {
  MenuHelp();
  InvClrRow25();
  AttrDisplayStr("Raw");
  SkipColumn(9);
  AttrDisplayStr("AscHex2Bin");
  row25_flag = TRUE;         // Remove menu automatically
  } // PutMenu

//----------------------------------------------------------------------------
//
//                           BlockCursor
//
void BlockCursor()
  {
  SetAttribute(LITE);
  AttrDisplayStr("@@");
  printf("\b\b");           // Reposition cursor
  SetAttribute(EDIT);
  } // BlockCursor

//----------------------------------------------------------------------------
//
//                          DisplaySize
//
void DisplaySize(unsigned long size)
  {
  char sbuf[30];

  _SetCursor(22, 56);
  sprintf(sbuf, "Size = %lu byte(s)", size);
  SetAttribute(EDIT);
  AttrDisplayStr(sbuf);
  } // DisplaySize

//----------------------------------------------------------------------------
//
//                          DisplayEOF
//
void DisplayEOF(unsigned long size)
  {
  char sbuf[30];

  _SetCursor(23, 56);
  if (hedit_mode == MOD_FILE) sprintf(sbuf, "End-Of-File at: %08lX", size);
  else sprintf(sbuf, "End-Of-Space at: %08lX", size);
  SetAttribute(EDIT);
  AttrDisplayStr(sbuf);
  } // DisplayEOF

//----------------------------------------------------------------------------
//
//                          DisplayChksum
//
void DisplayChksum(UINT chksum)
  {
  char sbuf[30];

  if (hedit_mode == MOD_FILE)
    {
    _SetCursor(21, 56);
    sprintf(sbuf, "Checksum = %04X", chksum);
    SetAttribute(EDIT);           // prepare attribute for display
    AttrDisplayStr(sbuf);
    }
  } // DisplayChksum

//----------------------------------------------------------------------------
//
//                          DisplayOffset
//
void DisplayOffset(unsigned long addr)
  {
  char hbuf[9];

  sprintf(hbuf, "%08lX", addr);   // emit hex chars into buffer for display
  SetAttribute(EDIT);             // prepare attribute for display
  AttrDisplayStr(hbuf);
  } // DisplayOffset

//----------------------------------------------------------------------------
//
//                          DisplayCurAddr
//
void DisplayCurAddr(int y, unsigned long addr)
  {
  char hbuf[9];

  _SetCursor((unsigned char)(y+4), 1);
  sprintf(hbuf, "%08lX", addr);   // emit hex chars into buffer for display
  SetAttribute(EDIT);             // prepare attribute for display
  AttrDisplayStr(hbuf);
  } // DisplayCurAddr

//----------------------------------------------------------------------------
//
//                          DisplayInvCurAddr
//
void DisplayInvCurAddr(int y, unsigned long addr)
  {
  char hbuf[9];

  _SetCursor((unsigned char)(y+4), 1);
  sprintf(hbuf, "%08lX", addr);   // emit hex chars into buffer for display
  SetAttribute(HILITE);           // prepare attribute for display
  AttrDisplayStr(hbuf);
  } // DisplayInvCurAddr

//@003------------------------------------------------------------------------
//
//                          DisplayFDHDSector
//
void DisplayFDHDSector(char *drv, int sec)
  {
  char sbuf[40];

  _SetCursor(21, 12);
  sprintf(sbuf, "Drive[%s]  Sector[%d]       ", argv_name, sec);
  SetAttribute(EDIT);             // prepare attribute for display
  AttrDisplayStr(sbuf);
  } // DisplayFDHDSector

//--------------------------end-of-module------------------------------------
