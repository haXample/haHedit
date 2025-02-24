// hedit.cpp - C++ Developer source file.
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

#include <sys\stat.h>       // For _open( , , S_IWRITE) needed for VC 2010
#include <fcntl.h>          // File Modes
#include <io.h>             // File open, close, etc.
#include <conio.h>          // For _putch(), ..
#include <string>           // printf, etc.

#include <windows.h>        // For console specific functions
#include <iostream>         //@004
#include <csignal>          //@004 Ctrl-C Handler

using namespace std;

//----------------------------------------------------------------------------
//                         function declarations
//----------------------------------------------------------------------------
unsigned long int TmpFileLseek(int fh, unsigned long offs, int org);
void DisplayRow24(char *format, ...);

//----------------------------------------------------------------------------
//                          external declarations
//----------------------------------------------------------------------------
extern void DebugStop(char *, int);
extern void DebugCursor_SHOW_POS(int, unsigned char,unsigned char,unsigned char,unsigned char);

extern int DesMain(int, char **);   //@002 Intgrated DES algorithm module
extern int AesMain(int, char **);   //@005 Intgrated AES algorithm module
extern int TdesMain(int, char **);  //@005 Intgrated TDES algorithm module

extern void put_fast_ch(unsigned char);
extern void put_fast_hex(unsigned char);
extern void SetRow25();
extern void ClrRow24();
extern int set_cursor(unsigned char);
extern int cur_home(unsigned char);
extern void cur_right(unsigned char);
extern void cur_left(unsigned char);
extern void cur_up(unsigned char);
extern void cur_down(unsigned char);
extern void RestoreVideoPage();
extern void SaveVideoPage();
extern void SetAttribute(unsigned char);
extern void AttrDisplayChar(unsigned char); 
extern void AttrDisplayUserGuide(char *);
extern void DisplayOffset(unsigned long);
extern void NormDisplayStart();
extern void BusyPrompt();
extern void ReadyPrompt();
extern void DefaultMenu();
extern void TagMenu();
extern void JumpMenu();
extern void SkipHexChar();
extern void RuboutStr();
extern void update_file(int);
extern int get_file(unsigned char);
extern int TmpFileRead(int, unsigned char *, int);
extern int file_block();
extern int quit();
extern int init_hedit(unsigned char);
extern int ExecDOSCommand();
extern int calculate();
extern void exchange();
extern void block_chksum(unsigned int *);
extern void clr_data_field();
extern void find_string();
extern int string_cmp(char *, char *, int);

extern int Int13hReadSector(char *, char *, int);        //@003
extern int Int13hWriteSector(char *, char *, int);       //@003
extern void DisplayFDHDSector(char *, int);              //@003

extern void AesCheckRunByExplorer();    //@005
extern void DesCheckRunByExplorer();    //@005
extern void TdesCheckRunByExplorer();   //@006
//----------------------------------------------------------------------------
//                         global declarations
//----------------------------------------------------------------------------
FILE *stream;

//---------------------------------------------------------------------------
//@001unsigned char siemens[] = "DOS Hedit V1.3  (c)Copyright 1986  Siemens AG";
//@001unsigned char siemens[] = "DOS Hedit V1.34 (c)1986-1994 Siemens AG      ";
//@001unsigned char siemens[] = "DOS Hedit V1.43 (c)1986-1994 Siemens Nixdorf ";
//@001unsigned char siemens[] = "DOS Hedit V1.44 (c)1986-1994 Siemens Nixdorf ";
char * siemens = "Hedit V1.5 (c)1986-2021 by ha ";

char * no_room = "no room for: %s";
char * open_failed = "open failed on: %s";
char * illegal_command = "illegal command";
char * enter_expr_dec = "Dec expression: ";
char * enter_expr_hex = "Hex expression: ";
char * calc_operators = "Calc operators: () + - * / %% & | ^ ~ >> <<";
char * illegal_expr = "illegal expression";
char * invalid_param = "invalid parameter";
char * hex = "Hex";
char * dec = "Dec";
char * enter_addr_hex = "Hex address: ";
char * block_update = "block update %s";
char * changes_lost = "all changes lost (y/[n])?";
char * evn = "/EVN";
char * has = " has%s";
char * have = " have%s";
char * been_written = " been written";
char * hedit_tmp = "HEDIT.TMP";

char copyright_msg[31] = {                                                                 
  'H'-38,'e'-39,'d'-40,'i'-41,'t'-42,' '-43,
  'V'-44,'1'-45,'.'-46,'5'-47,' '-48,
  '('-72,'c'-73,')'-74,'1'-75,'9'-76,'8'-77,'6'-78,'-'-79,
  '2'-80,'0'-81,'2'-82,'1'-83,' '-84,
  'b'-72,'y'-73,' '-74,'h'-75,'a'-76,' '-77, 0 };

char tmp_file[_LENGTH] = {* hedit_tmp};
char * con_name = "CON";
char * bak_suffix = ".BAK";
char * evn_suffix = ".EVN";
char * odd_suffix = ".ODD";
char * mrg_suffix = ".MRG";                       
char * hex_suffix = ".HEX";
char * bin_suffix = ".BIN";
char * raw_suffix = ".RAW";

unsigned char align_tab[16] = {8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9};

//---------------------------------------------------------------------------
unsigned char inbuf[256];
char f_buf[_SIZE];

char argv_name[_LENGTH+5];
char argv_sav[_LENGTH+5];
char g_name[_LENGTH+5];
char bak_name[_LENGTH+5];
char evn_name[_LENGTH+5];
char odd_name[_LENGTH+5];
char mrg_name[_LENGTH+5];
char drv_letter[_LENGTH+5];


char nam_rubtab[_LENGTH+1];

char cmd_line[_LENGTH+1];
char cmd_rubtab[_LENGTH+1];
char sav_string[_LENGTH+1];
char sav_rubtab[_LENGTH+1];
char g_string[_LENGTH+1];
char str_rubtab[_LENGTH+1];

char str_buf[_LENGTH];

unsigned char key_code, key, hex_key, dec_key, asc_key, hedit_mode, hedit_exe;

int n_sav, n_cmd, n_nam=0, n_str=0; // init to NUL-String, _LENGTH = 0

unsigned int g_bytesrd;
unsigned long g_offset, g_pos, g_size;
unsigned long g_li;

int g_fh, fh, fh_tmp, fh_mrg, fh_evn, fh_odd;
unsigned int evn_bytesrd, odd_bytesrd, bytesrd, bytwrtn;
unsigned long offset, f_pos, mem_pos, f_size, f_eof;
unsigned long li, ln, ls;

unsigned long tag_b_offset = _MAX;
unsigned long tag_c_offset = _MAX;
unsigned long tag_d_offset = _MAX;
int tag_b_x, tag_b_y, tag_c_x, tag_c_y, tag_d_x, tag_d_y;

int i,j,k,l,m,n;

int x_pos=0, y_pos=0, x_max=(3*16)-2, y_max=16-1, x_adj=12;
int repos_offset;
int dump_xs, dump_ys;
int drvSector=0;                    //@003

unsigned char ret_flag;
unsigned char backup_flag = FALSE;  // no backup file created yet
unsigned char update_flag = FALSE;
unsigned char xchg_flag = FALSE;
unsigned char hex_flag = TRUE;
unsigned char row24_flag = TRUE;
unsigned char row25_flag = TRUE;
unsigned char exit_flag = FALSE;
unsigned char rdy_flag = FALSE;

unsigned int copy_right, chksum, new_blk_chksum, old_blk_chksum;

char *p;

int exceptFlag = FALSE;         //@004
unsigned long exceptCode = 0L;  //@004


//----------------------------------------------------------------------------
//
//                                 display_file
//
int display_file()
  {
  int i,j,k;

  BusyPrompt();
  NormDisplayStart();

  for (i=0; i<256; i++) inbuf[i] = 0xFF; // Default - fill inbuf with 0FFh

  if (offset > f_eof)  offset = ((f_eof)/256L) * 256L;
  f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);

  bytesrd = TmpFileRead(fh_tmp,inbuf,256);
  block_chksum(&old_blk_chksum);

  li = offset;                          // save current offset
  l = align_tab[(int)(offset % 16L)];   // display '-' between byte 7 and 8
  k = 0;

  clr_data_field();               // written in assembler

  for (i=0; i<16; i++)
    {                                                                   
    if (li > f_eof) break;
    else
      {
      DisplayOffset(li);
      put_fast_ch(' ');           // written in assembler
      put_fast_ch(' ');
      put_fast_ch(' ');
      }

    // Hexprint
    for (j=0; j<16; j++)
      {
      if ((li + (unsigned long)(k % 16)) <= f_eof)
        {
        if (j == l)
          {
          put_fast_ch('\b');
          put_fast_ch('-');
          }
        put_fast_hex(inbuf[k]);    // written in assembler
        }
      else SkipHexChar();
      k++;
      }

    // ASCII-Print
    put_fast_ch(' ');              // written in assembler
    put_fast_ch(' ');
    k = k - 16;
    for (j=0; j<16; j++)
      {
      if ((li + (unsigned long)(k % 16)) > f_eof) break;
      else
        {
        if (isprint(inbuf[k]))  put_fast_ch(inbuf[k]);
        else put_fast_ch('.');
        }
      k++;
      }
    li = li + 16L;
    put_fast_ch(_CR);              // written in assembler
    put_fast_ch(_LF);
    }

  ReadyPrompt();
  set_cursor(_NORM_CURSOR);
  return(TRUE);
  }  // display_file


//@004------------------------------------------------------------------------
//
//                              signalHandler
//
void signalHandler(int signum) // Ctrl-C  Handler 
  {
  cout << "Interrupt Signal[" << signum << "] received. HEDIT aborted.\n";

  // Cleanup and close up stuff here not possible: Windows10 always aborts. 
  SetRow25();
  unlink(tmp_file);            // Delete tmp_file, if any exists
  SetAttribute(NORMAL);
  RestoreVideoPage();          // Restore default screen
  exit(signum);                // Terminate program
  } // signalHandler

//----------------------------------------------------------------------------
//
//                             get_key
//
unsigned char get_key()
  {
  unsigned char c;

  if ((c = _getch()) == _EXTENDED_KEYCODE)  key = _getch();

  if ((asc_key = c) < ' ' || asc_key > 0x7E) asc_key = 0xFF;

  c = toupper(c);

  if ((hex_key = c) >= '0' && hex_key <= '9')
    {
    hex_key = hex_key - '0';
    dec_key = hex_key;
    }
  else
    {
    dec_key = 0xFF;
    if (hex_key >= 'A' && hex_key <= 'F') hex_key = hex_key - '7';
    else  hex_key = 0xFF;
    }

  return (c);
  } // get_key

//----------------------------------------------------------------------------
//
//                             get_number
//
int get_number(unsigned long *l_number, unsigned char radix)
  {
  li = 0L;

  i = 0;
  while ((key_code = get_key()) != 0x0D && key_code != _ESC)
    {
    if ((hex_key != 0xFF && radix == 16 && i < 8) ||
        (dec_key != 0xFF && radix == 10 && i < 9))
      {
      i++;
      li = (li * radix) + hex_key;
      AttrDisplayChar(key_code);
      }
    else
      {
      if (key_code == _RUBOUT)
        {
        if (i > 0 )
          {
          i--;
          li = (li / radix);
          RuboutStr();
          }
        }
      else
        {
        if (key_code == _LF) ; // do nothing
        else _putch(_BEEP);
        }
      }
    } // end while

  if (key_code == _ESC)
    {
    _putch(_BEEP);  // no address change if ESC exit
    return(FALSE);
    }
  else
    {
    *l_number = li;
    return(TRUE);
    }
  } // get_number

//----------------------------------------------------------------------------
//
//                             get_address
//
void get_address(unsigned char sw)
  {
  AttrDisplayUserGuide(enter_addr_hex);

  if (get_number(&offset,RADIX_16))
    {
    if (offset > f_eof) offset = f_eof;

    if (sw)
      {
      x_pos = 3*(int)(offset%16L);
      y_pos = (int)((offset%256L))/16; // cur = new address
      offset = (offset/256L) * 256L;
      }
    else
      {
      x_pos = 0;  y_pos = 0;
      }
    display_file();
    }
  } // get_address

//----------------------------------------------------------------------------
//
//                             chk_integrity
//
int chk_integrity()
  {
  copy_right = 0;
  for (i=0;i<31;i++) copy_right = copy_right + copyright_msg[i];
//@001printf("\r\ncopy_right=%X, COPYRIGHTCS=%X\r\n",copy_right,COPYRIGHTCS); 
//@001while (getch() != ' ');                                                 
                                                                 
  if (copy_right != COPYRIGHTCS) return(FALSE);
  else
    {
    for (i=0;i<=10;i++) copyright_msg[i] = copyright_msg[i] + i+38;
    for (i=11;i<=23;i++) copyright_msg[i] = copyright_msg[i] + i+(72-11);
    for (i=24;i<30;i++) copyright_msg[i] = copyright_msg[i] + i+(72-24);
//@001printf("\ncopyright_msg=%s\n      siemens=%s\n",copyright_msg,siemens); 
//@001while (getch() != ' ');                                                 

    return(string_cmp(copyright_msg, siemens, 30));
    }
  return(TRUE);                                                              
  } // chk_integrity

//----------------------------------------------------------------------------
//
//                                 tag_set
//
void tag_set()
  {
  TagMenu();

  key_code = get_key();
  switch(key_code)
    {
    case 'B':
      tag_b_offset = offset;
      tag_b_x = x_pos; tag_b_y = y_pos;
      break;
    case 'C':
      tag_c_offset = offset;
      tag_c_x = x_pos; tag_c_y = y_pos;
      break;
    case 'D':
      tag_d_offset = offset;
      tag_d_x = x_pos; tag_d_y = y_pos;
      break;
    case _ESC:
      break;
    default:
      _putch(_BEEP);
      break;
    }
  } // tag_set

//----------------------------------------------------------------------------
//
//                                  get_tag
//
void get_tag(unsigned long tag_offset, int tag_x, int tag_y)
  {
  if (tag_offset != _MAX)
    {
    offset = tag_offset;
    x_pos = tag_x;  y_pos = tag_y;
    display_file();
    }
  else
    {
    ClrRow24();
    printf("no such tag");
    row24_flag = TRUE;
    _putch(_BEEP);
    }
  } // get_tag

//----------------------------------------------------------------------------
//
//                              start_of_file
//
int start_of_file()
  {
  offset = 0L;
  x_pos = 0; y_pos = 0;
  display_file();
  return(TRUE);
  } // start_of_file

//----------------------------------------------------------------------------
//
//                              end_of_file
//
int end_of_file()
  {
  offset = f_eof;
  x_pos = 3*(int)(offset%16L);
  y_pos = (int)(offset%256L)/16;    // cur = new address
  offset = (offset/256L) * 256L;
  display_file();
  return(TRUE);
  } // end_of_file

//----------------------------------------------------------------------------
//
//                               file_jump
//
void file_jump(unsigned char sw)
  {
  if (xchg_flag) update_file(_NO_CHANGE);
  JumpMenu();

  key_code = get_key();
  switch(key_code)
    {
    case 'A':
      get_address(sw);
      break;
    case 'B':
      get_tag(tag_b_offset,tag_b_x,tag_b_y);
      break;
    case 'C':
      get_tag(tag_c_offset,tag_c_x,tag_c_y);
      break;
    case 'D':
      get_tag(tag_d_offset,tag_d_x,tag_d_y);
      break;
    case 'E':
      end_of_file();
      break;
    case 'S':
      start_of_file();
      break;
    case _ESC:
      break;
    default:
      _putch(_BEEP);
      break;
    }
  }  // file_jump

//----------------------------------------------------------------------------
//
//                              previous_char
//
void previous_char()
  {
  if (offset >= 1L)
    {
    offset = offset - 1L;
    display_file();
    }
  } // previous_char

//----------------------------------------------------------------------------
//
//                                next_char
//
void next_char()
  {
  offset = offset + 1L;
  display_file();
  } // next_char

//----------------------------------------------------------------------------
//
//                              previous_row
//
void previous_row()
  {
  if (offset >= 16L)
    {
    offset = offset - 16L;
    display_file();
    }
  } // previous_row

//----------------------------------------------------------------------------
//
//                                next_row
//
void next_row()
  {
  offset = offset + 16L;
  display_file();
  } // next_row

//----------------------------------------------------------------------------
//
//                              previous_block
//
void previous_block()
  {
  if (hedit_mode == MOD_HDD)
    {                                                           //@003
    offset ^= 256;                                              //@003
    if (offset == 256)                                          //@003
      {                                                         //@003
      if (drvSector >0)                                         //@003
        {                                                       //@003
        drvSector--;                                            //@003
        Int13hReadSector(argv_name, (char *)f_buf, drvSector);  //@003
        DisplayFDHDSector(argv_name, drvSector);                //@003
        }                                                       //@003
      else offset = 0L;                                         //@003
      }                                                         //@003
    }                                                           //@003
  else                                                          //@003
    {                                                           //@003
    if (offset >= 256L) offset = offset - 256L;
    else  offset = 0L;
    }                                                           //@003
  display_file();
  } // previous_block

//----------------------------------------------------------------------------
//
//                                next_block
//
void next_block()
  {
  if (hedit_mode == MOD_HDD)                                    //@003
    {                                                           //@003
    offset ^= 256;                                              //@003
    if (offset != 256)                                          //@003
      {                                                         //@003
      drvSector++;                                              //@003
      Int13hReadSector(argv_name, (char *)f_buf, drvSector);    //@003
      DisplayFDHDSector(argv_name, drvSector);                  //@003
      }                                                         //@003
    }                                                           //@003
  else offset = offset + 256L;                                  //@003
//@003  offset = offset + 256L;
  display_file();
  } // next_block

//----------------------------------------------------------------------------
//
//                         check_extended_keys
//
void check_extended_keys(unsigned char sw)
  {
  switch(key)
    {
    case _CUP:
      cur_up(sw);
      break;
    case _CDOWN:
      cur_down(sw);
      break;
    case _CTRL_CUP:
      if (xchg_flag) update_file(_NO_CHANGE);
      previous_row();
      break;
    case _CTRL_CLEFT:
      if (xchg_flag) update_file(_NO_CHANGE);
      previous_char();
      break;
    case _CTRL_CDOWN:
      if (xchg_flag) update_file(_NO_CHANGE);
      next_row();
      break;
    case _CTRL_CRIGHT:
      if (xchg_flag) update_file(_NO_CHANGE);
      next_char();
      break;
    case _CLEFT:
      cur_left(sw);
      break;
    case _CRIGHT:
      cur_right(sw);
      break;
    case _CHOME:
      cur_home(sw);
      break;
    case _CEND:
      cur_home(sw);
      cur_left(sw);
      break;
    case _CTRL_PAGEUP:
      if (xchg_flag) update_file(_NO_CHANGE);
      start_of_file();
      break;
    case _PAGEUP:
      if (xchg_flag) update_file(_NO_CHANGE);
      previous_block();
      break;
    case _CTRL_PAGEDOWN:
      if (xchg_flag) update_file(_NO_CHANGE);
      end_of_file();
      break;
    case _PAGEDOWN:
      if (xchg_flag) update_file(_NO_CHANGE);
      next_block();
      break;
    default:
      _putch(_BEEP);
      break;
    }
  } // check_extended_keys

//----------------------------------------------------------------------------
//
//                              copy_filename
//
void copy_filename(char *file_name, char *argv_name)
  {
  p = argv_name;    // save argv_name pointer
  while (*p != 0)
    {
    *p = toupper(*p);
    p++;
    } // end while

  strcpy(file_name,argv_name);
  } // copy_filename

//----------------------------------------------------------------------------
//
//                              merge_file
//
unsigned int merge_file()
  {
  if (((fh_odd = open(odd_name,O_RDONLY|O_BINARY))==ERR) ||
      ((fh_evn = open(evn_name,O_RDONLY|O_BINARY))==ERR))
    {
    printf(open_failed,odd_name); printf(evn);
    return(FALSE);
    }

  ret_flag = TRUE;
  if ((fh_mrg = open(mrg_name,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR)
    {
    printf(open_failed,mrg_name);
    ret_flag = FALSE;
    }

  evn_bytesrd = TRUE;
  while (evn_bytesrd != 0 && ret_flag)
    {
    odd_bytesrd = read(fh_odd, (unsigned char *)&f_buf[_SIZE_MRG],_SIZE_MRG);
    evn_bytesrd = read(fh_evn, (unsigned char *)&f_buf[2*_SIZE_MRG],_SIZE_MRG);
    if (odd_bytesrd != evn_bytesrd)
      {
      if ((evn_bytesrd < odd_bytesrd) || (evn_bytesrd > odd_bytesrd+1))
        {
        printf("Error: %s%s - obviously not a splitted file!",odd_name, evn);
        ret_flag = FALSE;
        break;
        }
      }

    for (i=0; i<odd_bytesrd; i++) f_buf[(2*i)+1] = f_buf[_SIZE_MRG+i];
    for (i=0; i<evn_bytesrd; i++) f_buf[2*i] = f_buf[(2*_SIZE_MRG)+i];

    if (write(fh_mrg, (unsigned char *)f_buf,odd_bytesrd+evn_bytesrd) == ERR)
      {
      printf(no_room,mrg_name);
      ret_flag = FALSE;
      break;
      }
    } // end while

  close(fh_mrg);
  close(fh_odd);
  close(fh_evn);
  return(ret_flag);
  } // merge_file

//----------------------------------------------------------------------------
//
//                        main: HeditHelpMsg
//
void HeditHelpMsg()
  {
  printf("HEDIT filename | /MEMORY | [/MERGE filename] | [/HDD driveletter [SectorNr]]\n"
         "  filename    Pathname of the input file to be edited.\n"
         "  /MERGE      Merges any splitted 'filename.ODD/EVN' to 'filename.MRG'.\n"
         "  /M[EMORY]   Examines 4Gbyte of (unprotected) system memory space.\n"
         "  /HDD        Hard Drive sector editor. Usage e.g. [/HDD E: 0]\n"
         "  /DES        Explains usage of the integrated DES Crypto Module.\n"
         "  /TDES       Explains usage of the integrated TDES Crypto Module.\n"
         "  /AES        Explains usage of the integrated AES Crypto Module.\n");
  
#if (_MSC_VER >= 1900)
  // ... Do Visual Studio 2019 specific stuff
  // This console program could be run by typing its name at the command prompt,
  // or it could be run by the user double-clicking it from Explorer.
  // And you want to know which case you’re in.

  // Check if invoked via Desktop
  DWORD procId = GetCurrentProcessId();
  DWORD count = GetConsoleProcessList(&procId, 1);
  if (count < 2)
    {
    HANDLE hConsoleStd = GetStdHandle(STD_OUTPUT_HANDLE);

    // Resize console 80x25 with scrollbar
    //SMALL_RECT sConsoleWindow = {0, 0, 79, 24};  // Resizing console window: 80x25
    //SetConsoleWindowInfo(hConsoleStd, TRUE, &sConsoleWindow); // FALSE = ERROR 87!

    // Set console size 80x25 without scrollbar
    CONSOLE_SCREEN_BUFFER_INFOEX consolesize;
    consolesize.cbSize=sizeof(consolesize);
    GetConsoleScreenBufferInfoEx(hConsoleStd,&consolesize);
    COORD c;
    c.X = 80;
    c.Y = 25;
    consolesize.dwSize = c;
    consolesize.srWindow.Left   =  0;
    consolesize.srWindow.Right  = 80;
    consolesize.srWindow.Top    =  0;
    consolesize.srWindow.Bottom = 25;
    SetConsoleScreenBufferInfoEx(hConsoleStd, &consolesize);

    printf("\nConsole application: HEDIT.EXE\n");
    system("cmd");     // Keep the Console window open
    exit(0);           // (user may recoursively start the console app)
    }

  // When invoked via Console (nothing to do)
#endif
  } // HeditHelpMsg


#if (_MSC_VER == 1600)
// ... Do Visual Studio 2010 specific stuff
//----------------------------------------------------------------------------
//
//                        main: CheckRunByExplorer (XP only)
//
// This console program could be run by typing its name at the command prompt,
// or it could be run by the user double-clicking it from Explorer.
// And you want to know which case you’re in.
//
void CheckRunByExplorer()
  {
  // Check if invoked via Desktop

  DWORD procId = GetCurrentProcessId();
  DWORD count = GetConsoleProcessList(&procId, 1);
  if (count < 2)
    {
    // Resize console window 80x25, using console system command (XP only)
    system("MODE CON COLS=80 LINES=25");          // Sets window size (XP only)!
    HeditHelpMsg();                               // Signon help info
    printf("\nConsole application: HEDIT.EXE\n");
    system("cmd");     // Keep the Console window open
    exit(0);           // (user may recoursively start the console app)
    }

  // When invoked via Console (nothing to do)
  } // CheckRunByExplorer

#elif (_MSC_VER >= 1900)
// ... Do Visual Studio 2019 specific stuff
void CheckRunByExplorer()
  {
  }
#endif

//*****************************************************************************
//**                                                                         **
//** Funktion:       main                                                    **
//**                                                                         **
//** Abstract:       The command line is read and checked for the            **
//**                 input file name. If no filename was found an error      **
//**                 exit to DOS will be performed.                          **
//**                                                                         **
//** Extern Input:   int argc, char *argv[] from command line                **
//**                                                                         **
//** Extern Output:  none                                                    **
//**                                                                         **
//** PROCEDURES:                                                             **
//** -----------                                                             **
//** C-Library:      exit                                                    **
//**                 lseek                                                   **
//**                 open                                                    **
//**                 printf                                                  **
//**                 signal                                                  **
//**                 strncmp                                                 **
//**                 strcpy                                                  **
//**                 toupper                                                 **
//**                 unlink                                                  **
//**                                                                         **
//** Extern:         ....                                                    **
//** Public:         abort                                                   **
//**                 calculate                                               **
//**                 check_extended_keys                                     **
//**                 chk_integrity                                           **
//**                 display_file                                            **
//**                 file_jump                                               **
//**                 get_key                                                 **
//**                 get_number                                              **
//**                 get_address                                             **
//**                 next_block                                              **
//**                 previous_block                                          **
//**                                                                         **
//** ----------------------------------------------------------------------- **
//** Author:         Altmann,  KE ST 3,  D APC 1,  K WA T2,  SNI PC E SW 1,  **
//**                           SNI PC SB ESW 1                               **
//** Date:           Oct 1986; Oct 1987; Oct 1994; DEC 1995                  **
//** Version:        1.3                 1.37      1.42                      **
//**                                                                         **
//*****************************************************************************
int main(int argc, char *argv[])
  {
  hedit_exe = TRUE;             //@005 Assume the utility is named "HEDIT.EXE"

  strupr(argv[0]);              //@005 convert to upper case
  if (strstr(argv[0],"AES.EXE") != 0 || //@005
      strcmp(argv[0],"AES") == 0 ||     //@005
      strstr(argv[0],"AES") != 0)       //@005
    {                                   //@005
    hedit_exe = FALSE;                  //@005 Utility has been renamed "AES.EXE"
    AesCheckRunByExplorer();            //@005 XP only
    AesMain(argc, argv);                //@005 Run as AES.EXE
    exit(0);                            //@005                                                        
    }                                   //@005
  if (strstr(argv[0],"TDES.EXE") != 0 ||  //@006
      strcmp(argv[0],"TDES") == 0 ||      //@006
      strstr(argv[0],"TDES") != 0)        //@006
    {                                     //@006                                                                           
    hedit_exe = FALSE;                    //@006 Utility has been renamed "TDES.EXE"
    TdesCheckRunByExplorer();             //@006 XP only
    TdesMain(argc, argv);                 //@006 Run as TDES.EXE
    exit(0);                              //@006
    }                                     //@006
  if (strstr(argv[0],"DES.EXE") != 0 || //@005
      strcmp(argv[0],"DES") == 0 ||     //@005
      strstr(argv[0],"DES") != 0)       //@005
    {                                   //@005                                                                           
    hedit_exe = FALSE;                  //@005 Utility has been renamed "DES.EXE"
    DesCheckRunByExplorer();            //@005 XP only
    DesMain(argc, argv);                //@005 Run as DES.EXE
    exit(0);                            //@005
    }                                   //@005

  switch(argc)
    {
    case 1:
      printf("\r\n%s\n", siemens);          // No option - print signon message                        
      CheckRunByExplorer();                 // XP only
      HeditHelpMsg();
      exit(1);
      break;

    case 2:                                 // get argv_name, compose cmd_line
      if (strncmp(argv[1],"/?",2) == 0 ||
          stricmp(argv[1],"/MERGE") == 0 ||
          stricmp(argv[1],"/HDD") == 0)     // Help requested
        {
        CheckRunByExplorer();               // XP only
        HeditHelpMsg();
        exit(1);
        }
      if (stricmp(argv[1],"/DES") == 0)     //@002 DES Utility Help requested
        {                                   //@002
        argv[1],"/?";                       //@002 Force DES /? help menu
        DesMain(argc, argv);                //@002 Call DESfast.cpp
        exit(1);                            //@002
        }                                   //@002
      if (stricmp(argv[1],"/AES") == 0)     //@005 AES Utility Help requested
        {                                   //@005
        argv[1],"/?";                       //@005 Force AES /? help menu
        AesMain(argc, argv);                //@005 Call AES.cpp
        exit(1);                            //@005
        }                                   //@005
      if (stricmp(argv[1],"/TDES") == 0)    //@006 TDES Utility Help requested
        {                                   //@006
        argv[1],"/?";                       //@006 Force TDES /? help menu
        TdesMain(argc, argv);               //@006 Call TDES.cpp
        exit(1);                            //@006
        }                                   //@006
      n_cmd = (int)strlen(argv[1]);         // Other opttion: /M or filename, etc.
      copy_filename(cmd_line,argv[1]);
      break;
 
    case 3:                                 // Option and filname, compose cmd_line
      strcpy(cmd_line,argv[1]);             //@001
      n_cmd = (int)strlen(argv[1]);
      cmd_line[n_cmd] = ' ';                //@001 Truncate and force one space only
      cmd_line[n_cmd+1] = 0;                //@001 Terminate string after the space
      strcat(cmd_line,argv[2]);             //@001 Expected "/option space filepath"
      break;

    case 4:                                 //@002 Intgrated DES, AES & TDES algorithm modules
      strcpy(cmd_line,argv[1]);             //@001
      n_cmd = (int)strlen(argv[1]);
      cmd_line[n_cmd] = ' ';                //@003 Truncate and force one space only
      cmd_line[n_cmd+1] = 0;                //@003 Terminate string after the space
      strcat(cmd_line,argv[2]);             //@003 Expected "/option space filepath"
      strcat(cmd_line," ");                 //@003  or "/option drv sector"
      strcat(cmd_line,argv[3]);             //@003 
      strcpy(argv_sav,cmd_line);            //@003
      break;                                

    case 5:                                 //@002 Default: Intgrated DES algorithm module
      DesMain(argc, argv);                  //@002 Default: Call DESfast.cpp
      break;                                

    case 6:                                 //@005 Intgrated DES algorithm module
      argc = 5;                             //@005 Compatible with AES, DES % TDES modules
      if (stricmp(argv[5],"/AES") == 0)     //@005
        {                                   //@005 AES Utility
        AesMain(argc, argv);                //@005 Call AES.cpp
        }                                   //@005
      if (stricmp(argv[5],"/DES") == 0)     //@005
        {                                   //@005 DES Utility
        DesMain(argc, argv);                //@005 Call DES.cpp
        }                                   //@005
      if (stricmp(argv[5],"/TDES") == 0)    //@006
        {                                   //@006 TDES Utility
        TdesMain(argc, argv);               //@006 Call TDES.cpp
        }                                   //@006
      break;                                

    case 7:                                 //@005 Intgrated DES algorithm module
      argc = 6;                             //@005 Compatible with AES, DES % TDES modules
      if (stricmp(argv[6],"/AES") == 0)     //@005
        {                                   //@005 AES Utility
        AesMain(argc, argv);                //@005 Call AES.cpp
        }                                   //@005
      if (stricmp(argv[6],"/DES") == 0)     //@005
        {                                   //@005 DES Utility
        DesMain(argc, argv);                //@005 Call DES.cpp
        }                                   //@005
      if (stricmp(argv[6],"/TDES") == 0)    //@006
        {                                   //@006 TDES Utility
        TdesMain(argc, argv);               //@006 Call TDES.cpp
        }                                   //@006
      break;                                

    default:                                // Unknown option
      printf(invalid_param);
      exit(1);
      break;
    } // end switch

  if (n_cmd >= _LENGTH)                     // File path is too long for static
    {                                       //  buffers used here
    printf("Pathname too long\r\n");
    exit(1);
    }

  // Register signal SIGINT and signal handler  
  signal(SIGINT, signalHandler);            //@004 Set <CNTL-C> Handler
  SaveVideoPage();                          // save current screen

//----------------------------------------------------------------------------
//
//                     FILE_INIT: LABEL
//
  FILE_INIT:

  // check and open input file, copy into tmp_file, create bakup file
  if (exit_flag != INIT)
    {
    RestoreVideoPage();           // restore page to see error messages
    if (!init_hedit(_1ST_INIT))
      {
      printf("\n");               // next row
      exit(1);                    // open failed
      }
    ClrRow24();
    printf(copyright_msg);
    row24_flag = TRUE;
    }

  offset = 0L;
  cur_home(_NORM_CURSOR);
  display_file();

//----------------------------------------------------------------------------
//
//                     command interpreter
//
  exit_flag = FALSE;
  while (!exit_flag)
    {
    if (row25_flag) DefaultMenu();    // display default menu row
    set_cursor(_NORM_CURSOR);

    key_code = get_key();             // wait and get user key

    if (row24_flag) ClrRow24();
    switch (key_code)
      {
      case 'B':
        file_block();
        row25_flag = TRUE;
        break;
      case 'C':
        calculate();
        break;
      case 'F':
        find_string();
        break;
      case 'G':
        get_file(_FULL_READ);
        update_flag = TRUE;
        break;
      case 'J':
        file_jump(_NORM_ALIGNED);                 // normal address alignment
        break;
      case 'Q':
        exit_flag = quit();
        row24_flag = TRUE;
        row25_flag = TRUE;
        break;
      case 'T':
        tag_set();
        break;
      case 'X':
        exchange();
        update_flag = TRUE;
        break;
      case '!':
        ExecDOSCommand();
        break;
      case _ESC:
        ClrRow24();
        row25_flag = TRUE;
        break;
      case _EXTENDED_KEYCODE:          //@001 extended key with E0-prefix
        check_extended_keys(_NORM_CURSOR);
        break;
      case _CR:                        // do nothing
        break;
      case _LF:                        // do nothing
        break;
      default:
        _putch(_BEEP);
        ClrRow24();
        printf(illegal_command);
        row24_flag = TRUE;
        break;
      }
    if (exceptFlag == TRUE)                           //@004
      {                                               //@004
      ClrRow24();                                     //@004
      printf("Exception[%08X]: Memory access violation.", exceptCode);//@004
      row24_flag = TRUE;                              //@004
      }                                               //@004

    } // end while

  if (exit_flag == INIT) goto FILE_INIT;

  SetRow25();
  RestoreVideoPage();               // restore default screen
  exit(0);
  } // main

//--------------------------end-of-main-module-------------------------------

//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
//ha//{
//ha//printf("\n3: argv[0] = [%d] %s\ncmd_line = %s\nodd_name = %s\n\nPress ESC to continue..", argc, argv[0], cmd_line, odd_name); 
//ha//while (getch() != '\x1B');                                                  
//ha//}
//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
