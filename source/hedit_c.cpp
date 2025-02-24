// hedit_c.cpp - C++ Developer source file.
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
extern char tmp_file[];

extern unsigned char inbuf[256];
extern unsigned char f_buf[_SIZE];

extern char bak_name[_LENGTH+5];
extern char evn_name[_LENGTH+5];
extern char odd_name[_LENGTH+5];
extern char mrg_name[_LENGTH+5];

extern char argv_name[_LENGTH+5];
extern char argv_sav[_LENGTH+5];
extern char g_name[_LENGTH+5];
extern unsigned char nam_rubtab[_LENGTH+1];
extern char sav_string[_LENGTH+1];
extern unsigned char sav_rubtab[_LENGTH+1];
extern char cmd_line[_LENGTH+1];
extern unsigned char cmd_rubtab[_LENGTH+1];

extern char *p;

extern unsigned char key_code;

extern int i,j,k,l,m,n;
extern int n_sav, n_nam, n_cmd;
extern int x_pos, y_pos;

extern unsigned char backup_flag, hedit_mode;
extern unsigned char update_flag, xchg_flag, rdy_flag, row24_flag, row25_flag;

extern unsigned int fh, fh_tmp, fh_evn, fh_odd;
extern int g_fh, bytesrd, g_bytesrd, bytwrtn;
extern unsigned int ln, chksum, new_blk_chksum, old_blk_chksum;

extern unsigned long int li, g_offset, offset, mem_pos, f_pos, f_size;

extern void ClrRow24();
extern void AttrDisplayUserGuide(char *);
extern void CalcMenu(); 
extern unsigned char get_key();
extern int get_string(unsigned char[], unsigned char[], int *, char);

//----------------------------------------------------------------------------
//                          global variables
//----------------------------------------------------------------------------
extern void DebugCursor_SHOW_POS(int, unsigned char, unsigned char,
                                      unsigned char, unsigned char);

extern char * enter_expr_dec;
extern char * enter_expr_hex;
extern char * calc_operators;
extern char * illegal_expr;

static char *str;
static int error;
unsigned char unary = 0;
int n_calc = 0;
char calc_rubtab[_LENGTH+1];
char calcbuf[_LENGTH+1];

//----------------------------------------------------------------------------
//                         function declarations
//----------------------------------------------------------------------------
unsigned long Expr(unsigned char radix);

//----------------------------------------------------------------------------
//
//                               ValidDigit
//
int ValidDigit(unsigned char radix)
  {
  if (*str >= '0' && *str <= '9') return(TRUE);
  else
    {
    if ((radix == RADIX_16) && (*str >= 'A' && *str <= 'F')) return(TRUE);
    else return(FALSE);
    }
  } // ValidDigit

//----------------------------------------------------------------------------
//
//                                 Constant
//
unsigned long Constant(unsigned char radix)
  {
  unsigned long rval = 0L;
  unsigned char digit = 0, i = 0;

  if (!ValidDigit(radix)) error = 1;              // error break if invalid char
  while (*str && ValidDigit(radix))               // loop until end of string
    {
    if (*str >= '0' && *str <= '9') digit = *str - '0';
    else digit = *str - '7';
    rval = (rval*(unsigned long)radix) + (unsigned long)digit;    // calculate the constant
    str++;                                        // next ascii char
    i++;                                          // count number of ascii chars
    } // end while

  switch (*str)                 // check the syntax of allowed operators
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '|':
    case '&':
    case '^':
    case '%':
    case '<':
    case '>':
    case '(':
    case ')':
    case ' ':                     // allow spaces
    case 0:                       // end-of-string
      break;
    default:
      error = 1;                  // illegal char in expression
      break;
      } // end switch

  if (radix == RADIX_16 && i > 8) error = 4;      // ascii constant too long
  if (radix == RADIX_10 && i > 10) error = 4;     // ascii constant too long
  return(rval);
  } // Constant

//----------------------------------------------------------------------------
//
//                                Factor
//
unsigned long Factor(unsigned char radix)
  {
  unsigned long rval = 0L;
  unsigned char u;

  while (*str == ' ') str++;          // skip leading spaces
                                      // save and skip any unary: '-' or '~'
  if (((u = *str) == '~') || (u == '-')) str++;

  if (*str != '(') rval = Constant(radix);    // push the next constant
  else
    {                                 // handle expresion in parentheses
    str++;                            // skip leading parenthesis
    rval = Expr(radix);               // evaluate and push next expression
    if (*str == ')') str++;           // expression must have termination here
    else  error = 2;                  // printf("unbalanced parenthesis");
    }

  switch (u)                          // return the evaluated expression term
    {
    case '-':                         // unary: negative value must be returned
      unary = u;                      // set global unary
      return(~rval+1L);               // negate value
      break;
    case '~':                         // unary: binary invert
      unary = u;                      // set global unary
      return(~rval);                  // invert value
    default:
      return(rval);                   // return the factor value
      break;
    } // end switch
  } // Factor

//----------------------------------------------------------------------------
//
//                               Expr
//
// Expression <expr> Evaluation is done left to right and can be controlled by
// parentheses. Expressions may contain:
// {()0123456789ABCDEF} {+ - * / %} {^ & | ~ >> <<} {Space}
//
// The following grammar is applied:
// <expr>   ::=    <factor>
//     | <factor> + <expr>
//     | <factor> - <expr>
//     | <factor> * <expr>
//     | <factor> / <expr>
//     | <factor> % <expr>
//     | <factor> ^ <expr>
//     | <factor> & <expr>
//     | <factor> | <expr>
//     | <factor> ~ <expr>
//     | <factor> >> <expr>
//     | <factor> << <expr>
//
// <factor>   ::=     ( <expr> )
//     | -( <expr> )
//     |  <constant>
//     | -<constant>
//
// <constant> ::=  ASCII-str containing ('0'..'F')
//
unsigned long Expr(unsigned char radix)
  {
  unsigned long lval;

  lval=Factor(radix);             // get the next factor

  while (*str == ' ') str++;      // skip leading spaces

  do                              // bnf grammar, left-to-right, no precedence
    {
    switch (*str)                 // this is the list of available operators
      {
      case '+': str++; lval += Factor(radix); break;
      case '-': str++; lval -= Factor(radix); break;
      case '*': str++; lval *= Factor(radix); break;
      case '/': str++; lval /= Factor(radix); break;
      case '|': str++; lval |= Factor(radix); break;
      case '&': str++; lval &= Factor(radix); break;
      case '^': str++; lval ^= Factor(radix); break;
      case '%': str++; lval %= Factor(radix); break;
      case '<':
        str++;
        if (*str != '<') error = 3;   // illegal operator, '<<' expected
        str++; lval <<= Factor(radix);
        break;
      case '>':
        str++;
        if (*str != '>') error = 3;   // illegal operator, '>>' expected
        str++; lval >>= Factor(radix);
        break;
      default:
        while (*str == ' ') str++;    // skip trailing spaces
        return(lval);                 // expression evaluated: break loop.
        break;
      } // end switch
    } while (TRUE);                   // end expression evaluation loop
  } // Expr

//----------------------------------------------------------------------------
//
//                               calculate
//
int calculate()
  {
  unsigned long value;

  ClrRow24();
  CalcMenu();

  unary = ' ';                            // reset global unary
  error = 0;                              // clear flag "illegal expr"
  switch(get_key())                       // get the user's choice
    {
    case 'H':
      AttrDisplayUserGuide(enter_expr_hex);
      if (!get_string((unsigned char *)calcbuf, (unsigned char *)calc_rubtab, &n_calc, 0)) return(FALSE);
      str = calcbuf;
      value = Expr(RADIX_16);
      ClrRow24();
      if (error != 0 || *str != 0) printf(illegal_expr);
      else printf("  [%lu] = %08lXh",value,value);
      break;
    case 'D':
      AttrDisplayUserGuide(enter_expr_dec);
      if (!get_string((unsigned char *)calcbuf, (unsigned char *)calc_rubtab, &n_calc, 0)) return(FALSE);
      str = calcbuf;
      value = Expr(RADIX_10);
      ClrRow24();
      if (error != 0 || *str != 0) printf(illegal_expr);
      else
        {
        if (unary == '-') printf("  %c%lu = [%08lXh]",unary,~value+1,value);
        else printf("  %lu = [%08lXh]",value,value);
        }
      break;
    case '?':
      ClrRow24();
      printf(calc_operators);
      row24_flag = TRUE;
      break;
    case _ESC:
      break;
    default:
      _putch(_BEEP);
      break;
    } // end switch
  return(TRUE);
  } // calculate

//--------------------------end-of-module------------------------------------
