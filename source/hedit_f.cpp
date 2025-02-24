// hedit_f.cpp - C++ Developer source file.
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

#include "hedit_cpp.h"      // include the common declarations

#include <conio.h>          // For _putch(), ..
#include <string>           // printf, etc.

//----------------------------------------------------------------------------
//                         external declarations
//----------------------------------------------------------------------------
extern void DebugStop(char *, int);
extern void DebugCursor_SHOW_POS(int, unsigned char,unsigned char,
                                      unsigned char,unsigned char);

extern void AttrDisplayUserGuide(char *); 
extern void ClrRow24();
extern void ReadyPrompt();
extern void BusyPrompt();
extern void AttrDisplayChar(unsigned char); 
extern void AttrDisplayHexChar(unsigned char);  
extern void ClrRowInvert();
extern int TmpFileWrite(int, unsigned char *, int);
extern int TmpFileRead(int, unsigned char *, int);
extern unsigned char get_key();
extern int update_chksum(unsigned char);
extern int display_file();
extern int set_block_boundary();
extern void SetColumnRow25(int);
extern void ClearRestOfRow(); 

extern unsigned char inbuf[256];
extern char f_buf[_SIZE];
extern char g_string[_LENGTH+1];
extern char str_rubtab[_LENGTH+1];
extern char sav_string[_LENGTH+1];
extern char sav_rubtab[_LENGTH+1];
extern char argv_name[_LENGTH+5];
extern char argv_sav[_LENGTH+5];

extern unsigned char key_code, key, hex_key, asc_key;

extern int i,j,k,l,m,n;
extern int x_pos, y_pos;
extern int repos_offset;
extern int n_sav, n_str;

extern unsigned int  g_bytesrd, bytwrtn, chksum;

extern unsigned char row24_flag;
extern int fh_tmp;
extern unsigned long int offset, g_offset, g_pos, f_pos, f_size, f_eof;
extern unsigned long int li, g_li;

//----------------------------------------------------------------------------
//                          function declarations
//----------------------------------------------------------------------------
unsigned long int TmpFileLseek(int fh, unsigned long offs, int org);

int line_editor(unsigned char linbuf[], unsigned char rubtab[],
                int *n_lth, unsigned char sw_str, unsigned char sw);

//----------------------------------------------------------------------------
//
//                          reposition_str_cur
//
int reposition_str_cur(unsigned char rubtab[])
  {
  k = 0;
  for (j=0;j<i;j++) k = k + rubtab[j];
  SetColumnRow25(k+repos_offset);
  return(TRUE);
  } // reposition_str_cur

//----------------------------------------------------------------------------
//
//                             string_cmp
//
int string_cmp(char *buf, char *str, int cnt)
  {
  int i;

  i = 0;
  while ((i < cnt) && (*buf == *str))
    {
    i++;
    buf++;
    str++;
    }

  if (i >= cnt) return(TRUE);
  else return(FALSE);
  } // string_cmp

//----------------------------------------------------------------------------
//
//                             string_cpy
//
int string_cpy(char *str2, char *str1, int cnt)
  {
  int i;

  i = 0;
  while (i < cnt)
    {
    *str2 = *str1;
    str2++; str1++;
    i++;
    } // end while
  return(TRUE);
  } // string_cpy

//----------------------------------------------------------------------------
//
//                             search_file
//
int search_file()
  {
  int f_bytesrd;

  BusyPrompt();

  if (n == 0) n = 1;                            // NUL-string handling

  li = offset + y_pos * 16 + x_pos/3 + 1;       // get cursor offset + 1
  j = FALSE;                                    // assume "not found"

  while ((li < f_eof) && !j)
    {
    g_pos = TmpFileLseek(fh_tmp,li,SEEK_SET);
    if ((f_bytesrd = TmpFileRead(fh_tmp, (unsigned char *)f_buf, _SIZE)) < n) break;

    i = 0;
    while ((i+n <= f_bytesrd) && !j)
      {
      while ((f_buf[i] != g_string[0]) && (i+n <= f_bytesrd)) i++;
      if ((i+n <= f_bytesrd) && string_cmp(&(char)f_buf[i], g_string, n)) j = TRUE;
      else i++;
      } // end while

    li = li + i;
    } // end while

  if (j)  offset = li;            // set offset to 1st byte of matched string
  return(j);                      // no change if string not found
  } // search_file

//----------------------------------------------------------------------------
//
//                            display_edlin
//
int display_edlin(unsigned char linbuf[], unsigned char rubtab[], unsigned char sw_str)
  {
  if (n == 0) ;                 // do nothing on NUL-String
  else
    {
    j = i;
    for (j=i; j<n; j++)
      {
      if (isprint(linbuf[j]))
        {
        AttrDisplayChar(linbuf[j]);
        rubtab[j] = 1;
        }
      else
        {
        AttrDisplayChar('\\');
        AttrDisplayHexChar(linbuf[j]);
        _putch('\b');
        rubtab[j] = 3;
        }
      }
    }                                                     
  if (sw_str) AttrDisplayChar('\"');

  ClrRowInvert();
  reposition_str_cur(rubtab);
  return(TRUE);
  } // display_edlin                                                                  

//----------------------------------------------------------------------------
//
//                               get_string
//
int get_string(unsigned char linbuf[], unsigned char rubtab[], int *n_lth, char sw_str)
  {
  i = 0;  n = *n_lth;
  display_edlin(linbuf, (unsigned char *)rubtab, sw_str);

  string_cpy(sav_string, (char *)linbuf,*n_lth);
  string_cpy((char *)sav_rubtab, (char *)rubtab,*n_lth);
  n_sav = *n_lth;

  if (n == 0)                 // init condition
    {
    linbuf[0] = 0;
    rubtab[0] = 0;
    }
  
  if (sw_str == 1)            //@001 <Ctrl-G> for Hex
    {
    ClrRow24();
    printf("<Ctrl-G> for Hex");
    reposition_str_cur(rubtab);
    }
  else                        // <HOME> to re-edit
    {
    ClrRow24();
    printf("<HOME> to re-edit");
    reposition_str_cur(rubtab);
    }

  key_code = _EXTENDED_KEYCODE; key = _CEND;           //@003 Simulate key "_CEND" and
  return(line_editor(linbuf,rubtab,n_lth,sw_str,0));   //@003  enter line_editor for other keys
  } // get_string

//----------------------------------------------------------------------------
//
//                             find_string
//
void find_string()
  {
  AttrDisplayUserGuide("Find {Cs} \"");

  if (get_string((unsigned char *)g_string, (unsigned char *)str_rubtab, &n_str, 1))
    {
    if (search_file())
      {
      x_pos = 3*((n-1)%16);  y_pos = (n-1)/16;  // cursor to end of string
      display_file();
      ClrRow24();
      }
    else
      {
      ClrRow24();
      printf("not found: \"");
      row24_flag = TRUE;
      i = 0;
      display_edlin((unsigned char *)g_string, (unsigned char *)str_rubtab, 1);
      }
    }
  else
    {
    ClrRow24();
    _putch(_BEEP);
    }

  ReadyPrompt();
  } // find_string

//----------------------------------------------------------------------------
//
//                             xfill_string
//
int xfill_string()
  {
  row24_flag = TRUE;
  AttrDisplayUserGuide("XFill \"");

  set_block_boundary();

  if (get_string((unsigned char *)g_string,(unsigned char *)str_rubtab,&n_str,1))
    {
    BusyPrompt();
    if (n==0) n++;      // NUL-string: dont divide by zero!
    for (i=0; i<(_SIZE/n)*n; i=i+n)
      {
      for (j=0; j<n; j++) f_buf[i+j] = g_string[j];
      }

    g_li = li-g_offset + 1;
    while (g_offset <= li)
      {
      g_pos = TmpFileLseek(fh_tmp,g_offset,SEEK_SET);
      if (g_li < (_SIZE/n)*n) g_bytesrd = (unsigned int)g_li;
      else g_bytesrd = (_SIZE/n) * n;
      g_li = g_li - g_bytesrd;

      bytwrtn = TmpFileWrite(fh_tmp, (unsigned char *)f_buf, g_bytesrd);

      g_offset = g_offset + g_bytesrd;
      } // end while

    update_chksum(_FULL_UPDATE);

    f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);
    display_file();
    return(TRUE);
    }
  else return(FALSE);
  } // xfill_string

//----------------------------------------------------------------------------
//
//                          insert_edchar
//
void insert_edchar(unsigned char linbuf[], unsigned char rubtab[], unsigned char sw_str)
  {
  unsigned char temp;       //@001 C7/C7++ internal error

  if (i >= _LENGTH-1) ;     // do nothing
  else
    {
    for (j=(_LENGTH-2); j>=i; j--)
      {
//@000      linbuf[j+1] = linbuf[j];      // compiler has problems here
//@000      rubtab[j+1] = rubtab[j];      // compiler has problems here
      temp = linbuf[j];     //@000
      linbuf[j+1] = temp;   //@000
      temp = rubtab[j];     //@000
      rubtab[j+1] = temp;   //@000
      }
    if (n < _LENGTH) n++;
    }
  } // end insert_edchar

//----------------------------------------------------------------------------
//
//                            delete_edchar
//
void delete_edchar(unsigned char linbuf[], unsigned char rubtab[], unsigned char sw_str)
  {
  unsigned char temp;           //@001 C7/C7++ internal error

  if (i >= n) ;                 // do nothing
  else
    {
    for (j=i; j<n; j++)
      {
      f_buf[j] = linbuf[j+1];
      temp = rubtab[j+1];       //@000 see above (compiler problems)
      rubtab[j] = temp;         //@000
      }
    for (j=i; j<n; j++)         //@000
      {                         //@000
      linbuf[j] = f_buf[j];     //@000
      }                         //@000
    if (n > 0) n--;
    }
  } // end delete_edchar

//----------------------------------------------------------------------------
//
//                            line_editor
//
int line_editor(unsigned char linbuf[], unsigned char rubtab[],
                int *n_lth, unsigned char sw_str, unsigned char sw)
  {
  unsigned char g_hex;
  int k, j;

//@003  ClrRow24();
  reposition_str_cur(rubtab);
  
  // normal key, delete
  if (sw) display_edlin(linbuf, (unsigned char *)rubtab, sw_str);  
  
  // else <HOME> to re-edit
  i = 0;
  while (key_code != _CR && key_code != _ESC)
    {
    if (key_code == _CNTL_R) key_code = _CNTL_G;  // <HEX> keys

    switch(key_code)                                                       
      {
      case _EXTENDED_KEYCODE:         //@001 for WIN10 console application
        switch(key)
          {                                              
          case _CHOME:
            i = 0;
            ClrRow24();                                                      //@003
            reposition_str_cur(rubtab);
            for (j=0; j<_LENGTH/2; j++) delete_edchar(linbuf,rubtab,sw_str); //@001
            display_edlin(linbuf, (unsigned char *)rubtab, sw_str);          //@001
            break;
          case _CEND:
            i = n;
            reposition_str_cur(rubtab);
            break;
          case _CLEFT:
            if (i > 0)
              {
              i--;
              reposition_str_cur(rubtab);
              }
            break;
          case _CRIGHT:
            if (i < n)
              {
              i++;
              reposition_str_cur(rubtab);
              }
            break;
          case _DELETE:
            delete_edchar(linbuf,rubtab,sw_str);
            display_edlin(linbuf, (unsigned char *)rubtab, sw_str);
            break;
          default:
            _putch(_BEEP);
            break;
          } // end switch(key)
        break;

      case _CNTL_G:
        if (sw_str)                           // only if string is edited
          {
          ClrRow24();
          printf("<HEX>");
          reposition_str_cur(rubtab);
          g_hex = 0;
          key_code = get_key();

          if (hex_key != 0xFF && i <= _LENGTH-1)
            {
            g_hex = g_hex | (hex_key * 16);
            key_code = get_key();
            if (hex_key != 0xFF)  g_hex = g_hex | hex_key;
            }
          if (hex_key == 0xFF || i > _LENGTH-1) _putch(_BEEP);
          else
            {
            insert_edchar(linbuf,rubtab,sw_str);
            linbuf[i] = g_hex;
            display_edlin(linbuf, (unsigned char *)rubtab, sw_str);
            i++;
            }

          ClrRow24();
          printf("<Ctrl-G> for Hex");         //@001 keeps it visible
          reposition_str_cur(rubtab);
          }
        else _putch(_BEEP);
        break;
      case _RUBOUT:
        if (i > 0 )
          {
          i--;
          reposition_str_cur(rubtab);
          delete_edchar(linbuf,rubtab,sw_str);
          display_edlin(linbuf, (unsigned char *)rubtab, sw_str);
          }
        break;
      default:
        if (asc_key != 0xFF && i <= _LENGTH-1)
          {
          insert_edchar(linbuf,rubtab,sw_str);
          if (sw_str) linbuf[i] = asc_key;    // store upper/lower case ASCII
          else linbuf[i] = key_code;          // store upper case ASCII
          display_edlin(linbuf, (unsigned char *)rubtab, sw_str);
          i++;
          reposition_str_cur(rubtab);
          }
        else _putch(_BEEP);
        break;
      } // end switch(key_code)

    key_code = get_key();
    } // end while

  if (key_code == _ESC)
    {
    string_cpy((char *)linbuf, (char *)sav_string,*n_lth);
    string_cpy((char *)rubtab,sav_rubtab,*n_lth);
    return(FALSE);
    }
  else
    {
    linbuf[n] = 0;                            // ensure string ends with NUL
    *n_lth = n;
    return(TRUE);
    }

  } // line_editor

//--------------------------end-of-module------------------------------------
//
//@001ClrRow24();
//@001printf("i=%02X, of=%lX, g_brd=%u f_bf[i]=%02X, j=%u",i,offset,f_bytesrd,f_buf[i],j);
//@001if((key_code = get_key()) == _ESC) break;
