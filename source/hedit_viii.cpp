// hedit_viii.cpp - C++ Developer source file.
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

//#include <exception>        // For try, throw, catch
#include <excpt.h>          // for EXCEPTION_ACCESS_VIOLATION
#include <windows.h>        // C++ Windows10 console general definitions

using namespace std;
      
//----------------------------------------------------------------------------
//                          external declarations
//----------------------------------------------------------------------------
extern void DebugStop(char *, int);
extern void DebugStopBuf(unsigned char, unsigned long);

extern int exceptFlag;
extern unsigned long exceptCode;
 
//----------------------------------------------------------------------------
//                          global declarations
//----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//                          filter
//
// exceptions_try_except_Statement.cpp
// Example of try-except and try-finally statements
//
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep)
  {
  if (code == EXCEPTION_ACCESS_VIOLATION)
    {
    return EXCEPTION_EXECUTE_HANDLER;
    }
  else
    {
    return EXCEPTION_CONTINUE_SEARCH;
    };
  } // filter


//------------------------------------------------------------------------------
//
//                          ReadMemory
//
// This procedure interfaces the following 'C' function:
//
//   int ReadMemory(UCHAR *buf, ULONG mem_pos, int cnt);
//
//  INBUF EQU WORD  PTR [bp+4]  ; 1st parameter: address of inbuf[]
//  OFFS  EQU DWORD PTR [bp+6]  ; 2nd parameter: offset within 4G memory space
//  CNT   EQU WORD  PTR [bp+10] ; 3rd parameter: bytes to be read
//
//int ReadMemory(unsigned char *buf, unsigned long *mptr, int mbytrd)
int ReadMemory(unsigned char *buf, char *mptr, int mbytrd)
  {
  int i;

  __try
    {
    __try
      {  // May cause an access violation exception (4G flat memory space edit)
      for (i=0; i<mbytrd; i++) buf[i] = *(mptr++);  // read here something
      }
    __finally
      {
      if (AbnormalTermination()) exceptFlag = TRUE; // Tell outside world 
      else exceptFlag = FALSE;                      // Reset execption status
      }
    }
  __except(filter(GetExceptionCode(), GetExceptionInformation()))
    {
    exceptCode = EXCEPTION_ACCESS_VIOLATION;        // Tell outside world
    }
  return(mbytrd);                                   // return actual bytes read
  } // ReadMemory 


//------------------------------------------------------------------------------
//
//                        WriteMemory
//
// This procedure interfaces the following 'C' function:
//
//   int WriteMemory(UCHAR *buf, ULONG mem_pos, int cnt);
//
//  INBUF EQU WORD  PTR [bp+4]  ; 1st parameter: address of inbuf[]
//  OFFS  EQU DWORD PTR [bp+6]  ; 2nd parameter: offset within 4G memory space
//  CNT   EQU WORD  PTR [bp+10] ; 3rd parameter: bytes to be written
//
int WriteMemory(unsigned char *buf, char *mptr, int mbytwr)
  {
  int i;

  __try
    {
    __try
      {  // May cause an access violation exception (4G flat memory space edit)
      for (i=0; i<mbytwr; i++) *(mptr++) = buf[i];  // write memory
      }
    __finally
      {
      if (AbnormalTermination()) exceptFlag = TRUE; // Tell outside world 
      else exceptFlag = FALSE;                      // Reset execption status
      }
    }
  __except(filter(GetExceptionCode(), GetExceptionInformation()))
    {
    exceptCode = EXCEPTION_ACCESS_VIOLATION;        // Tell outside world
    }
  return(mbytwr);                                   // return actual bytes written
  } // WriteMemory  

//--------------------------end-of-module------------------------------------





                              