// hedit_b.cpp - C++ Developer source file.
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

using namespace std;

//----------------------------------------------------------------------------
//                         external declarations
//----------------------------------------------------------------------------
extern char tmp_file[];
extern char* con_name;

extern char *p;
extern int drvSector;

extern char argv_name[_LENGTH+5];
extern char g_name[_LENGTH+5];
extern char nam_rubtab[_LENGTH+1];
extern int n_nam, n_str;

extern char g_string[_LENGTH+1];
extern char str_rubtab[_LENGTH+1];

extern unsigned char inbuf[256];
extern char f_buf[_SIZE];

extern unsigned char key_code, key, hex_key, asc_key;

extern int i,j,k,l,m,n;
extern int x_pos, y_pos, dump_xs, dump_ys;
extern unsigned int chksum;

extern unsigned char update_flag, xchg_flag, ret_flag, hedit_mode;
extern unsigned char row24_flag, row25_flag, rdy_flag;

extern int g_fh, fh_tmp;
extern unsigned int g_bytesrd, bytwrtn;

extern unsigned long g_offset, g_pos, g_size, mem_pos, offset, f_pos, f_size, f_eof;
extern unsigned long int ln, li, g_li;

extern char * mptr; //@004

extern char * siemens;
extern char * illegal_command;
extern char * open_failed;
extern char * hedit_tmp;
extern char * enter_addr_hex;
extern char * no_room;
extern char * has;
extern char * have;
extern char * been_written;
extern char * bin_suffix;
extern char * raw_suffix;
extern char bak_name[];

extern void DebugStop(char *, int);
extern void DebugStopBuf(unsigned char, unsigned long);

extern void ClrRow24();
extern void ReadyPrompt();
extern void BusyPrompt();
extern void BlockMenu();
extern void PutMenu();
extern void CalcBlkChksum();
extern void AttrDisplayChar(unsigned char); 
extern void AttrDisplayHexChar(unsigned char);  
extern void AttrDisplayUserGuide(char *); 
extern void SaveVideoPage();
extern void RestoreVideoPage();
extern void DisplayMode(unsigned char mod);
extern void DisplaySize(unsigned long size);
extern void DisplayEOF(unsigned long size);
extern void DisplayChksum(unsigned int chksum);
extern void SkipOffsetAddr();
extern void SkipHexChar();
extern int SkipColumn(unsigned char);
extern void InvDisplayStart();
extern void TabAsciiField();
extern void InitScreen();
extern int set_cursor(unsigned char);
extern int display_file();
extern void update_file(int);
extern int get_string(unsigned char[], unsigned char[], int *, char);
extern int xfill_string();
extern void find_string();
extern void file_jump(unsigned char);
extern unsigned char get_key();
extern int dump_cur_remove();
extern void check_extended_keys(unsigned char);
extern int get_number(unsigned long *, unsigned char);
extern int update_chksum(unsigned char);

extern int ReadMemory(unsigned char *, char *, int);  //@004
extern int WriteMemory(unsigned char *, char *, int); //@004
extern int AscHex2Bin(char *, char *, int);

extern FILE *stream;

extern unsigned char row24_flag;
extern int exceptFlag;                    //@004
extern unsigned long exceptCode, offset;  //@004

//----------------------------------------------------------------------------
//
//                            set_block_boundary
//
int set_block_boundary()
  {
  li = y_pos*16 + x_pos/3 + offset;   // end block address

  if (li < g_offset)
    {
    g_li = li;                        // xchg g_offset,li (li >= g_offset)
    li = g_offset;
    g_offset = g_li;
    }
  if (li >= f_size) li = f_size-1;
  return(TRUE);
  } // set_block_boundary

//----------------------------------------------------------------------------
//
//                             TmpFileClose
//
int TmpFileClose()
  {
  if (hedit_mode == MOD_MEMORY) return(TRUE);   // no temp file required
  close(fh_tmp);
  return(TRUE);                                 // close tmp_file
  } // TmpFileClose

//----------------------------------------------------------------------------
//
//                             TmpFileUnlink
//
int TmpFileUnlink()
  {
  if (hedit_mode == MOD_MEMORY) return(TRUE);   // no temp file required

  close(fh_tmp);
  unlink(tmp_file);                             // delete tmp_file
  return(TRUE);                                 // close tmp_file
  } // TmpFileUnlink

//----------------------------------------------------------------------------
//
//                             TmpFileOpen
//
int TmpFileOpen()
  {
//  char *p, *q, t[_LENGTH + 4];
  char *p, *q, *t;
  char __mallocBuf[256]; //@001

  if (hedit_mode == MOD_MEMORY) return(TRUE);    // no temp file required


  // check and get the environment variable settings

//@001  if (((p = (char *)getenv("TMP")) == NULL) &&   // retrieve the temporary env path
//@001      ((q = (char *)getenv("TEMP")) == NULL))
  if (TRUE) //@001
    {
//@001    t = malloc(_LENGTH + 4);
    t = __mallocBuf;
    strcpy(t, hedit_tmp);         // use current path
    }
  else
    {
    if (p != NULL)
      {
//@001      t = malloc(strlen(p) + _LENGTH + 4);
      t = __mallocBuf;
      strcpy(__mallocBuf, p);
      strcat(__mallocBuf, "\\");      //@001 => "C:\Users\_ha\AppData\Local\Temp\" 
      strcat(__mallocBuf, hedit_tmp); //@001 => "C:\Users\_ha\AppData\Local\Temp\HEDIT.TMP" 
      }
    else // use *q settings
      {
//@001      t = malloc(strlen(q) + _LENGTH + 4);
      t = __mallocBuf;
      strcpy(t, q);
      strcat(t, "\\");
      strcat(t, hedit_tmp);
      }
    }
  strcpy(tmp_file, t);            // Init temporary filename
//DebugStop(tmp_file, 0);         //@001 => "C:\Users\_ha\AppData\Local\Temp\HEDIT.TMP"
//@001  free(t);


  if ((fh_tmp = open(tmp_file, O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR)
    {
    // Try current path
    strcpy(tmp_file, hedit_tmp);  // Init temporary filename
    if ((fh_tmp = open(tmp_file, O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR)
      {
      printf(open_failed,tmp_file);
      return(FALSE);
      }
    return(TRUE);
    }
  else
    {
    return(TRUE);     
    }
  } // TmpFileOpen

//----------------------------------------------------------------------------
//
//                           TmpFileLseek
//
unsigned long TmpFileLseek(int fh, unsigned long offs, int org)
  {
  switch(hedit_mode)
    {
    case MOD_MEMORY:                              // 4G flat memory space edit
      if (offs == 0L && org == SEEK_END)
        {
        mem_pos = 0xFFFFFFFFL;                    //@004
        }
      else mem_pos = offs;
      return(mem_pos);
      break;

    case MOD_HDD:                                 //@001 HDD edit mode
    case MOD_FILE:                                // file edit mode
    default:
      return(lseek(fh, offs, org));
      break;
    } // end switch
  return(TRUE);
  } // TmpFileLseek

//----------------------------------------------------------------------------
//
//                            TmpFileRead
//
int TmpFileRead(int fh, unsigned char *buf, int cnt)
  {
  int bytrd=cnt;                      //@001
  char * mptr=0L;                     //@004
//DebugStopBuf(1, (unsigned long)mptr);

  switch(hedit_mode)
    {
    case MOD_MEMORY:
      if (offset == 0L)                       //@004
        {                                     //@004
#if (_MSC_VER == 1600)
        // ... Do VC10/Visual Studio 2010 specific stuff
        // Start somewhere within us (XP), i.e. at EXE-Header "MZ..."
        // [ = (siemens) - (00F90000h - 00FC0394h) = 00030394h]
        mptr = ((siemens) - 0x30394) - 0x1000 + mem_pos; //@004
#elif (_MSC_VER >= 1900)
        // ... Do Visual Studio 2019 specific stuff
        // [ = (siemens) - (00BA0000h - 00BDA240h) = 0003A240h]
        mptr = ((siemens) - 0x3A240) - 0xf270 + mem_pos; //@004 
#endif
        offset = (unsigned long)mptr;         //@004
        }                                     //@004
      else mptr = mptr + offset;              //@004
      ReadMemory(buf,mptr,bytrd);             // Written in Assembler
      mem_pos += (unsigned long)bytrd;        //@001 move position ptr
      return(bytrd);                          // return the actual bytes read
      break;

    case MOD_HDD:                             //@001 HDD sector edit
    case MOD_FILE:                            // file edit mode
    default:
      return(read(fh, buf, cnt));
      break;
    } // end switch

  } // TmpFileRead

//----------------------------------------------------------------------------
//
//                           TmpFileWrite
//
int TmpFileWrite(int fh, unsigned char *buf, int cnt)
  {
  int bytwr=cnt;
  char * mptr=0L;     //@004

  switch(hedit_mode)
    {
    case MOD_MEMORY:                          // 4G flat memory space edit
      if (offset == 0L)                       //@004
        {                                     //@004
#if (_MSC_VER == 1600)
        // ... Do VC10/Visual Studio 2010 specific stuff
        mptr = (siemens-0x2C384-0x3010) +mem_pos;     //@004 start somewhere within us (XP)
#elif (_MSC_VER >= 1900)
        // ... Do Visual Studio 2019 specific stuff
        mptr = (siemens-0x38240+0x2000) +mem_pos;     //@004 start somewhere within us
#endif
        offset = (unsigned long)mptr;         //@004
        }                                     //@004
      else mptr = mptr + offset;              //@004
      WriteMemory(buf,mptr,bytwr);            // Written in Assembler
      mem_pos += (unsigned long)bytwr;        // move position ptr
      return(bytwr);
      break;

    case MOD_FILE:                            // file edit mode
    default:
      return(write(fh, buf, cnt));
      break;
    } // end switch

  } // TmpFileWrite

//----------------------------------------------------------------------------
//
//                           chk_filename
//
int chk_filename(char f_name[], int *n_lth)
  {
  j = 0;

//DebugStop(f_name, 0x55);
  if ((p = strchr(&f_name[j],'/')) != 0)
    {
    j = (p - f_name);
    f_name[j]=0;
    }
//DebugStop(f_name, 56);
                                              // allow ..\..\..\pathname
  if ((p = strrchr(&f_name[j],':')) != 0) j = (p - f_name);
  if ((p = strrchr(&f_name[j],'\\')) != 0) j = (p - f_name);
  if ((p = strrchr(&f_name[j],'.')) != 0)     // check suffix length
    {
    j = p - f_name;
    f_name[j+4] = 0;                          // truncate suffix to 3 chars max
    *n_lth = strlen(f_name);
    }

  // "CON" filename activates console, special treatment required   //@001
  if ((strncmp(&f_name[j], "CO", 2) == 0) && (f_name[j+2] == 'N'))  //@001
    {
    ClrRow24();
    printf(".. Console is already open.",con_name);                 //@001
    _putch(_BEEP);
    return(FALSE);
    }                          

  return(TRUE);
  } // chk_filename

//----------------------------------------------------------------------------
//
//                           display_g_file
//
void display_g_file()
  {
  BusyPrompt();
  InvDisplayStart();

  l = y_pos * 16 + x_pos/3;

  k = 0;
  for (i=0; i<16; i++)
    {
    SkipOffsetAddr();            // skip offset display

  // Hexprint
    for (j=0; j<16; j++)
      {
      if (k > l || k >= g_bytesrd)  SkipHexChar();
      else  AttrDisplayHexChar(inbuf[k]);
      k++;
      }

  // ASCII-Print
    TabAsciiField();
    k = k - 16;
    for (j=0; j<16; j++)
      {
      if (k > l || k >= g_bytesrd) SkipColumn(1);     // skip to next column
      else
        {
        if (isprint(inbuf[k]))  AttrDisplayChar(inbuf[k]);
        else AttrDisplayChar('.');
        }
      k++;
      }
    printf("\r\n");             // next row
    }

  ReadyPrompt();
  }  // display_g_file

//----------------------------------------------------------------------------
//
//                            ExecDOSCommand
//
int ExecDOSCommand()
  {
  AttrDisplayUserGuide("DOS Command: \"");

  if (get_string((unsigned char *)g_string, (unsigned char *)str_rubtab, &n_str, 1))
    {
    RestoreVideoPage();                 // restore default screen
    system(g_string);
    printf("\npress any key ...\n");
    getch();
    SaveVideoPage();                    // save system screen
    InitScreen();                       // for all modes: init screen display
    DisplayMode(hedit_mode);
    DisplaySize(f_size);
    if (f_size != 0xFFFFFFFF) f_eof = f_size - 1L;      // normal
    else f_eof = f_size;                                // 4G boundary wraps
    DisplayEOF(f_eof);
    DisplayChksum(chksum);
    display_file();
    }
  else
    {
    ClrRow24();
    _putch(_BEEP);
    }

  ReadyPrompt();
  return(TRUE);
  } // ExecDOSCommand

//----------------------------------------------------------------------------
//
//                             get_g_name
//
int get_g_name(unsigned char sw)
  {
  if (sw)
    {
    AttrDisplayUserGuide("Output file: ");
    }
  else
    {
    AttrDisplayUserGuide("Input file: ");
    }

  if (!get_string((unsigned char *)g_name, (unsigned char *)nam_rubtab,&n_nam,0)) return(FALSE);
  else  return(chk_filename(g_name, &n_nam));
  return(chk_filename(g_name, &n_nam));
  } // get_g_name

//----------------------------------------------------------------------------
//
//                             get_g_address
//
int get_g_address()
  {
  unsigned long g_offset;

  AttrDisplayUserGuide(enter_addr_hex);

  g_offset = 0L;
  if (get_number(&g_offset,RADIX_16))
    {
    g_size = lseek(g_fh, 0L, SEEK_END);  // get file size

    if (g_offset >= g_size)
      {
      ClrRow24();
      printf("exceeded EOF in get file at: %08lX",g_size);
      return(FALSE);
      }
    else
      {
      g_pos = lseek(g_fh, g_offset, SEEK_SET);
      return(TRUE);
      }
    }
  else return(FALSE);
  } // get_g_address

//----------------------------------------------------------------------------
//
//                               ScrambleFile
//
void ScrambleFile()
  {
  unsigned char scrmbl;              // temporary for scramble algorithm

  BusyPrompt();
  ClrRow24();

  set_block_boundary();

  g_li = li-g_offset + 1;
  ln = g_offset;
  g_pos = TmpFileLseek(fh_tmp,ln,SEEK_SET);
  while ((g_bytesrd = TmpFileRead(fh_tmp,(unsigned char *)f_buf,_SIZE)) && (ln <= li))
    {
    if (g_li < (unsigned long)g_bytesrd) g_bytesrd = (int)g_li;
    g_li = g_li - (unsigned long)g_bytesrd;
    for (i=0; i<g_bytesrd; i++)      // algorithm: bits[76543210] -> [01234567]
      {
      scrmbl = 0;                    // init scramble value
      if (f_buf[i] & 0x01) scrmbl |= 0x80;
      if (f_buf[i] & 0x02) scrmbl |= 0x40;
      if (f_buf[i] & 0x04) scrmbl |= 0x20;
      if (f_buf[i] & 0x08) scrmbl |= 0x10;
      if (f_buf[i] & 0x10) scrmbl |= 0x08;
      if (f_buf[i] & 0x20) scrmbl |= 0x04;
      if (f_buf[i] & 0x40) scrmbl |= 0x02;
      if (f_buf[i] & 0x80) scrmbl |= 0x01;
      f_buf[i] = scrmbl;             // write scrambled value
      }
    g_pos = TmpFileLseek(fh_tmp,ln,SEEK_SET);
    bytwrtn = TmpFileWrite(fh_tmp,(unsigned char *)f_buf,g_bytesrd);

    ln += (unsigned long)g_bytesrd;
    } // end while

  update_chksum(_FULL_UPDATE);

  f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);

  display_file();
  } // ScrambleFile

//----------------------------------------------------------------------------
//
//                               xfill_file
//
void xfill_file()
  {
  BusyPrompt();
  set_block_boundary();

  g_li = li-g_offset + 1;
  ln = g_li;
  while ((g_bytesrd = read(g_fh,f_buf,_SIZE)) && (g_offset <= li))
    {
    g_pos = TmpFileLseek(fh_tmp,g_offset,SEEK_SET);
    if (g_li < (unsigned long)g_bytesrd) g_bytesrd = (unsigned int)g_li;
    g_li = g_li - g_bytesrd;

    bytwrtn = TmpFileWrite(fh_tmp,(unsigned char *)f_buf,g_bytesrd);

    g_offset = g_offset + g_bytesrd;
    } // end while

  update_chksum(_FULL_UPDATE);

  ln = ln - g_li;
  offset = g_offset - 1;
  x_pos = 3*(offset%16); y_pos = (offset%256)/16;
  offset = (offset/256)*256;
  f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);

  display_file();
  } // xfill_file

//----------------------------------------------------------------------------
//
//                               xget_file
//
int xget_file()
  {
  BusyPrompt();
  i = y_pos * 16 + x_pos/3;
  g_bytesrd = read(g_fh,inbuf,i+1);
  xchg_flag = TRUE;
  display_g_file();
  ln = (unsigned long)g_bytesrd;
  ReadyPrompt();
  return(TRUE);
  } // xget_file

//----------------------------------------------------------------------------
//
//                               get_file
//
int get_file(unsigned char sw)
  {
  row24_flag = TRUE;
  ClrRow24();

  if (get_g_name(_INPUT_FILE))  // Input file
    {
    if ((g_fh = open(g_name,O_RDONLY|O_BINARY))==ERR)
      {
      ClrRow24();
      printf(open_failed,g_name);
      return(FALSE);
      }
    else
      {
      if (get_g_address())
        {
        if (sw) xfill_file();
        else xget_file();
        ClrRow24();
        printf("%lu byte(s) read from: %s",ln,g_name,g_offset,li,g_li);
        close (g_fh);
        return(TRUE);
        }
      else
        {
        close (g_fh);
        return(FALSE);
        }
      }
    }
  else return(FALSE);
  } // get_file

//----------------------------------------------------------------------------
//
//                                dump_buffer
//
int dump_buffer()
  {
  i = 0;

  if ((g_bytesrd <16) && (g_bytesrd != 0)) g_bytesrd = g_bytesrd + 16;
  while ((i+16 <= g_bytesrd) && (g_offset+i <= li))
    {
    if (li <= 0xFFFF)  fprintf(stream,"%04lX ",g_offset+i);
    else  fprintf(stream,"%08lX ",g_offset+i);

    // Hex Dump
    k=0;
    while ( k < (g_offset+i)%16)
      {
      fprintf(stream,"   ");
      k++;
      } // end while

    for (j=k; j<16; j++)
      {
      if (g_offset+i <= li)
        {
        if (j == 8) fprintf(stream,"-%02X",f_buf[i] & 0xFF);  //@001 "& 0xFF" required
        else fprintf(stream," %02X",f_buf[i] & 0xFF);         //@001 "& 0xFF" required
        }
      else fprintf(stream,"   ");
      i++;
      }

    // ASCII Dump
    fprintf(stream,"  ");
    for (j=0; j<k; j++) fprintf(stream," ");

    i = i - (16-k);
    for (j=k; j<16; j++)
      {
      if (g_offset+i <= li)
        {
        if (isprint(f_buf[i])) fprintf(stream,"%c",f_buf[i]);
        else fprintf(stream,".");
        i++;
        }
      }
    fprintf(stream,"\n");
    } // end while

  if (ferror(stream))
    {
    ClrRow24();
    printf(no_room,g_name);
    return(ERR);
    }
  else return(i);
  } // dump_buffer


//----------------------------------------------------------------------------
//
//                                dump_file
//
unsigned char dump_file()
  {
  row24_flag = TRUE;
  ret_flag = FALSE;

  set_block_boundary();

  if (get_g_name(_OUTPUT_FILE))  // Output file
    {
    if ((stream = fopen(g_name,"w")) == NULL)
      {
      ClrRow24();
      printf(open_failed,g_name);
      }
    else
      {
      BusyPrompt();
      ClrRow24();
      printf("%s",g_name);

      // set large I/O-buffer
      setvbuf(stream, &f_buf[_SIZE/7], _IOFBF, _SIZE-_SIZE/7);

      while (g_offset <= li)
        {
        g_pos = TmpFileLseek(fh_tmp,g_offset,SEEK_SET);
        if ((g_bytesrd = TmpFileRead(fh_tmp, (unsigned char *)f_buf,_SIZE/7)) == 0) break;
        if ((n = dump_buffer()) == ERR) break;
        g_offset = g_offset + n;
        } // end while

      if (g_offset > li)
        {
        printf(has,been_written);    // dump finished
        ret_flag = TRUE;
        }

      fclose(stream);
      // re-position tmp_file pointer
      f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET); 
      }
    }
  return(ret_flag);
  } // dump_file

//----------------------------------------------------------------------------
//
//                                 put_file
//
unsigned char put_file()
  {
  char t_buf[_SIZE/2];
  int t_byteswr, _putMode;

  row24_flag = TRUE;
  ret_flag = FALSE;

  set_block_boundary();
  
//@005--Begin-----------------------------------------------
  ClrRow24();
  ReadyPrompt();                            // tell user to enter choice
  PutMenu();                                // print quit menu
  set_cursor(_NORM_CURSOR);
  key_code = get_key();                     // wait and get key from user

  switch (key_code)
    {
    case 'R':
      _putMode = _PUT_RAW;
      strcpy(g_name,bak_name);            // suggest a default name *.HEX
      i = strlen(bak_name);
      strcpy(&g_name[i-4],raw_suffix);
      n_nam = strlen(g_name);             // init default name length for re-edit
      break;
    case 'A':
      _putMode = _PUT_ASC_HEX2BIN;
      strcpy(g_name,bak_name);            // suggest a default name *.HEX
      i = strlen(bak_name);
      strcpy(&g_name[i-4],bin_suffix);
      n_nam = strlen(g_name);             // init default name length for re-edit
      break;
    default:
      putch(_BEEP);
      ClrRow24();
      printf(illegal_command);
      break;
    }
//@005--End-------------------------------------------------

  if (get_g_name(_OUTPUT_FILE))           // Output file
    {
    if ((g_fh = open(g_name,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR)
      {
      ClrRow24();
      printf(open_failed,g_name);
      }
    else
      {
      BusyPrompt();
      ClrRow24();
      printf("%s",g_name);

      g_li = li-g_offset + 1L;   // total size
      while (g_offset <= li)
        {
        TmpFileLseek(fh_tmp,g_offset,SEEK_SET);
        if (g_li < _SIZE) g_bytesrd = TmpFileRead(fh_tmp,(unsigned char *)f_buf,(int)g_li);
        else g_bytesrd = TmpFileRead(fh_tmp,(unsigned char *)f_buf,_SIZE);
        if (g_bytesrd == 0) break;

        g_offset += (unsigned long)g_bytesrd;
        g_li -= (unsigned long)g_bytesrd;

//@005--Begin-----------------------------------------------
        if (_putMode == _PUT_ASC_HEX2BIN)
          {
          t_byteswr = AscHex2Bin(f_buf, t_buf, g_bytesrd);
          if (write(g_fh,t_buf,t_byteswr) == ERR)
            {
            ClrRow24();
            printf(no_room,g_name);
            close(g_fh);
            unlink(g_name);
            break;
            }
          }
        else if (_putMode ==_PUT_RAW)
          {
          if (write(g_fh, f_buf, g_bytesrd) == ERR)
            {
            ClrRow24();
            printf(no_room,g_name);
            close(g_fh);
            unlink(g_name);
            break;
            }
          }
//@005--End-------------------------------------------------

        g_offset = g_offset + (unsigned long)g_bytesrd;
        } // end while

      if (g_offset > li)
        {
        printf(has,been_written);  // put finished
        ret_flag = TRUE;
        }

      close(g_fh);
      f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);  // re-position tmp_file pointer
      }
    }
  return(ret_flag);
  } // put_file

//----------------------------------------------------------------------------
//
//                                 file_block
//
int file_block()
  {
  if (xchg_flag)
    {
    update_file(_NO_CHANGE);
    display_file();
    }

  ClrRow24();

  g_offset = y_pos*16 + x_pos/3 + offset;   // current seek pointer in file
  if (x_pos%3 == 1) x_pos--;
  dump_xs = x_pos; dump_ys = y_pos;

  row25_flag = TRUE;
  rdy_flag = FALSE;
  while (!rdy_flag)
    {
    ReadyPrompt();               // tell user to enter choice
    if (row25_flag) BlockMenu(); // print block menu

    set_cursor(_BLOCK_CURSOR);
    key_code = get_key();

    if (row24_flag) ClrRow24();
    switch (key_code)
      {
      case 'C':
        CalcBlkChksum();
        break;
      case 'D':
        rdy_flag = dump_file();
        break;
      case 'F':
        find_string();
        break;
      case 'G':
        rdy_flag = get_file(_BLOCK_READ);
        update_flag = TRUE;
        break;
      case 'J':
        file_jump(_BLOCK_ALIGNED);            // block aligned
        break;
      case 'P':
        rdy_flag = put_file();     
        row25_flag = TRUE;
        break;
      case 'S':
        ScrambleFile();
        update_flag = TRUE;
        break;
      case 'X':
        rdy_flag = xfill_string();
        update_flag = TRUE;
        break;
      case _LF:                 // CR/LF: Allows termination in batch files
      case _CR:
      case _ESC:
        rdy_flag = TRUE;
        ClrRow24();             // preserve Block Checksum display (if any)
        break;
      case _EXTENDED_KEYCODE:   // extended key with NUL-prefix
        check_extended_keys(_BLOCK_CURSOR);
        break;
      default:
        _putch(_BEEP);
        ClrRow24();                                            
        printf(illegal_command);
        row24_flag = TRUE;
        break;
      } // end switch
    if (exceptFlag == TRUE)                           //@004
      {                                               //@004
      ClrRow24();                                     //@004
      printf("Exception[%08X]: Memory access violation.", exceptCode );//@004
      row24_flag = TRUE;                              //@004
      }                                               //@004
    } // end while

  dump_cur_remove();
  display_file(); //ha//
  ReadyPrompt();
  return(TRUE);
  } // file_block()

//--------------------------end-of-module------------------------------------
//
//@001ClrRow24();
//@001printf("i=%02X, of=%lX, g_brd=%u f_bf[i]=%02X, j=%u",i,offset,f_bytesrd,f_buf[i],j);
//@001key_code = get_key();
