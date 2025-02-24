// hedit_x.cpp - C++ Developer source file.
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
#include <string>           // Printf, etc.

//----------------------------------------------------------------------------
//                         external declarations
//----------------------------------------------------------------------------
extern void AttrDisplayChar(unsigned char); 
extern void AttrDisplayHexChar(unsigned char);  
extern void AttrDisplayUserGuide(char *); 
extern void XchangeMenu();
extern unsigned char get_key();
extern void update_file(int);
extern int display_file();
extern void DisplayCurAddr(int, unsigned long);
extern void DisplayInvCurAddr(int, unsigned long);
extern void check_extended_keys(unsigned char);
extern void ScreenPos(int, int);
extern void InvScreenPos(int, int);
extern void BlockCursor();


extern unsigned char inbuf[256];

extern unsigned char key_code, hex_key, asc_key;
extern unsigned char xchg_flag, hex_flag, rdy_flag;

extern int i;
extern int x_pos, y_pos, x_max, y_max, x_adj, dump_xs, dump_ys;

extern unsigned long offset, g_offset, f_size, f_eof;

//----------------------------------------------------------------------------
//
//                          cur_invalid
//
int cur_invalid()
  {
  int i;

  if (hex_flag) i = y_pos * 16 + x_pos/3;     // hex-area selected
  else i = y_pos * 16 + x_pos;                // ascii area selected

  if ((offset + (unsigned long)i) <= f_eof) return (FALSE);   // cursor valid
  else return (TRUE);
  } // end cur_invalid

//----------------------------------------------------------------------------
//
//                             set_cursor
//
int set_cursor(unsigned char sw)
  {
  int i;

  if (cur_invalid())
    {
    y_pos = (int)((f_eof - offset)/16L);                     
    if (x_adj == 12) x_pos = 3*(int)((f_eof - offset)%16L);  //ha// hex-area selected
    else x_pos = (int)((f_eof - offset)%16L);                //ha// ascii area selected
    }
  if (x_adj == 12) i = y_pos * 16 + x_pos/3;  // hex-area selected
  else i = y_pos * 16 + x_pos;                // ascii area selected

  DisplayInvCurAddr(y_pos,offset+(unsigned long)i); // cursor address

  switch(sw)
    {
    case _NORM_CURSOR:
      InvScreenPos(y_pos,x_pos+x_adj);
      break;
    case _BLOCK_CURSOR:
      if ((g_offset>=offset)&&(g_offset<(offset+256L)))
        {
        dump_xs = 3*(int)((g_offset-offset)%16L);
        dump_ys = (int)((g_offset-offset)/16L);
        InvScreenPos(dump_ys,dump_xs+x_adj);
        BlockCursor();
        }
      InvScreenPos(y_pos,x_pos+x_adj);
      BlockCursor();
      break;
    default:
      break;
    }
  return(TRUE);
  } // set_cursor

//----------------------------------------------------------------------------
//
//                             dump_cur_remove
//
int dump_cur_remove()
  {
  int i;

  i = dump_ys * 16 + dump_xs/3;
  if ( (g_offset >= offset) &&
       (g_offset <= (offset+256L)) &&
       ((offset + (unsigned long)i) <= f_eof))
    {
    ScreenPos(dump_ys,dump_xs+x_adj);
    AttrDisplayHexChar(inbuf[i]);
    printf("\b\b");
    }

  ScreenPos(y_pos,x_pos+x_adj);
  i = y_pos * 16 + x_pos/3;
  AttrDisplayHexChar(inbuf[i]);
  printf("\b\b");
  return(TRUE);
  } // dump_cur_remove

//----------------------------------------------------------------------------
//
//                           cur_addr_remove
//
int cur_addr_remove()
  {
  DisplayCurAddr(y_pos,offset+(unsigned long)(16*y_pos));
  return(TRUE);
  } // cur_addr_remove

//----------------------------------------------------------------------------
//
//                             cur_up
//
void cur_up(unsigned char sw)
  {
  if (sw) dump_cur_remove();
  cur_addr_remove();
  if (y_pos > 0 ) y_pos--;
  else y_pos = y_max;

  set_cursor(sw);
  } // cur_up

//----------------------------------------------------------------------------
//
//                              cur_down
//
void cur_down(unsigned char sw)
  {
  if (sw) dump_cur_remove();
  cur_addr_remove();
  y_pos++;
  if ((y_pos > y_max) || cur_invalid())  y_pos = 0;

  set_cursor(sw);
  } // cur_down

//----------------------------------------------------------------------------
//
//                             cur_left
//
void cur_left(unsigned char sw)
  {
  if (sw) dump_cur_remove();
  if (x_pos > 0) x_pos--;
  else
    {
    x_pos = x_max;      // x_max = 16*3-2 = 46
    if (sw) x_pos--;    // 46%3 = 1, has to be adjusted
    cur_up(_NORM_CURSOR);  // block cursor has already been removed on entry
    }
  if (hex_flag && x_pos % 3 == 2)  x_pos--;
  if (sw && x_pos % 3 == 1) x_pos--;

  set_cursor(sw);
  } // cur_left

//----------------------------------------------------------------------------
//
//                             cur_home
//
int cur_home(unsigned char sw)
  {
  if (sw) dump_cur_remove();
  cur_addr_remove();
  x_pos = 0; y_pos = 0;

  set_cursor(sw);
  return(TRUE);
  } // cur_home

//----------------------------------------------------------------------------
//
//                             cur_right
//
void cur_right(unsigned char sw)
  {
  if (sw) dump_cur_remove();
  x_pos++;
  if (sw && x_pos % 3 == 1) x_pos = x_pos + 2;
  if (x_pos > x_max)
    {
    x_pos = 0;
    cur_down(_NORM_CURSOR);   // block cursor has already been removed on entry
    }
  if (hex_flag && x_pos % 3 == 2) x_pos++;
  if (cur_invalid()) cur_home(_NORM_CURSOR);

  set_cursor(sw);
  } // cur_right

//----------------------------------------------------------------------------
//
//                               xchg_hex
//
void xchg_hex()
  {
  AttrDisplayUserGuide("[Exchange Hex]");     // prompt user and set cursor

  set_cursor(_NORM_CURSOR);

  while ((key_code = get_key()) != _ESC && key_code != _CR)
    {
    if (key_code == _EXTENDED_KEYCODE)
      {
      check_extended_keys(_NORM_CURSOR);
      set_cursor(_NORM_CURSOR);
      }

    else                                      // exchanging the hex-numbers
      {
      if (hex_key != 0xFF && (!cur_invalid()))
        {
        xchg_flag = TRUE;

        // calculate inbuf index
        i = y_pos * 16 + x_pos/3;

        if (x_pos % 3 == 1) inbuf[i] = (inbuf[i] & 0xF0) | hex_key;
        else inbuf[i] = (inbuf[i] & 0x0F) | (hex_key * 16);
        AttrDisplayChar(key_code);

        // ASCII-Print
        x_adj = (62 + x_pos/3) - x_pos;
        set_cursor(_NORM_CURSOR);
        if (isprint(inbuf[i]))  AttrDisplayChar(inbuf[i]);
        else AttrDisplayChar('.');

        // re-position hex
        x_adj = 12;
        cur_right(_NORM_CURSOR);
        }
      else _putch(_BEEP);
      }
    } // end while

  } // xchg_hex

//----------------------------------------------------------------------------
//
//                             xchg_ascii
//
void xchg_ascii()
  {
  AttrDisplayUserGuide("[Exchange ASCII]");   // prompt user and set cursor

  // set cursor to proper ascii position

  hex_flag = FALSE;
  x_pos = x_pos/3;  x_max = 16 - 1;
  x_adj = 62;
  set_cursor(_NORM_CURSOR);

  while ((key_code = get_key()) != _ESC && key_code != _CR)
    {
    if (key_code == _EXTENDED_KEYCODE)
      {
      check_extended_keys(_NORM_CURSOR);
      set_cursor(_NORM_CURSOR);
      }

    else                                      // exchanging the ASCII-chars
      {
      if (asc_key != 0xFF && (!cur_invalid()))
        {
        xchg_flag = TRUE;

        // calculate inbuf index
        i = (y_pos * 16) + x_pos;
        inbuf[i] = asc_key;
        AttrDisplayChar(asc_key);

        // Hex-Print
        x_adj = (12 + x_pos * 3) - x_pos;
        set_cursor(_NORM_CURSOR);
        AttrDisplayHexChar(inbuf[i]);

        // re-position ASCII
        x_adj = 62;
        cur_right(_NORM_CURSOR);
        }
      else _putch(_BEEP);
      }
    } // end while

  // re-position hex
  x_pos = x_pos * 3;  x_max = (3*16) - 2;
  x_adj = 12;
  hex_flag = TRUE;
  } // xchg_ascii

//----------------------------------------------------------------------------
//
//                             exchange
//
void exchange()
  {
  XchangeMenu();

  rdy_flag = FALSE;
  while (!rdy_flag)
    {
    switch(get_key())
      {
      case 'H':
        xchg_hex();
        rdy_flag = TRUE;
        break;
      case 'A':
        xchg_ascii();
        rdy_flag = TRUE;
        break;
      case _ESC:
        rdy_flag = TRUE;
        break;
      default:
        _putch(_BEEP);
        break;
      }
    } // end while

  if (xchg_flag)
    {
    update_file(_CHANGE);
    display_file();
    }
  } // exchange

//--------------------------end-of-module------------------------------------
