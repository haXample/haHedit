// hedit_vii.cpp - C++ Developer source file.
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

#include <stdio.h>
#include <conio.h>
#include <string>           // For printf, etc.

#include <io.h>             // File open, close, etc.
#include <iostream>                           
#include <windows.h>        // C++ Windows10 console general definitions

using namespace std;
      
//----------------------------------------------------------------------------
//                          external declarations
//----------------------------------------------------------------------------
extern void DebugStop(char *, int);

extern int TmpFileWrite(int, unsigned char *, int);
extern int TmpFileClose();
extern int TmpFileOpen();
extern unsigned long TmpFileLseek(int fh, unsigned long offs, int org);

extern char * no_room;
extern char tmp_file[];

extern int fh_tmp;
extern char f_buf[_SIZE];
extern unsigned long f_size;

//----------------------------------------------------------------------------
//                          global declarations
//----------------------------------------------------------------------------
unsigned long hddbytesRd, hddbytesWr;   // unsigned long  = DWORD
HANDLE hDrive = NULL;
int numSector = 0;                      // Initial sector 0

//char * PhysicalDrive = "\\\\.\\";   // strcat does not work = "\\.\E:",0 (see below)
//char PhysicalDrive[] = "\\\\.\\";    // strcat OK
char PhysicalDrive[7] = { 0x5C,0x5C,'.',0x5C,'0',':',0 }; // OK = "\\.\E:",0

//------------------------------------------------------------------------------
//
//                    _Int13hReadSector
//
//   This procedure interfaces the following 'C' function:
//    int Int13hReadSector(UCHAR drive, UCHAR *buf, ULONG offset);
//
int Int13hReadSector(char *drive, char *buf, int numSector)
  {
  PhysicalDrive[4] = drive[0];                 // = Volume name (see above)
  PhysicalDrive[5] = drive[1];                 // = Colon

  // When opening a volume or removable media drive
  //  (for example, a floppy disk drive or flash memory thumb drive),
  //  the PhysicalDrive string should be the following form: \\.\X:.

  hDrive = CreateFile(PhysicalDrive,            // Drive to open, eg. "\\\\.\\I:"
                      GENERIC_READ,             // Access mode = READ
                      FILE_SHARE_READ | FILE_SHARE_WRITE,   // Share Mode
                      NULL,                     // Security Descriptor
                      OPEN_EXISTING,            // How to create
                      0,                        // File attributes
                      NULL);                    // Handle to template

  if (hDrive == INVALID_HANDLE_VALUE)
    {
    printf("ERROR [%u]: Drive access denied.\n", GetLastError());
    return ERR;
    }

  SetFilePointer(hDrive, numSector*512, NULL, FILE_BEGIN);
  if (!ReadFile(hDrive, buf, 512, &hddbytesRd, NULL))
    {
    printf("ReadFile: %u\n", GetLastError());
    return ERR;
    }

//ha//  CloseHandle(hDrive);
  close(fh_tmp);
  if (TmpFileOpen() == FALSE) return(FALSE);

  if ((TmpFileWrite(fh_tmp, (unsigned char *)f_buf, hddbytesRd)) == ERR)
    {
    printf(no_room,tmp_file);
    close(fh_tmp);
    unlink(tmp_file);
    return(FALSE);
    }
  // get input file size
  f_size = TmpFileLseek(fh_tmp,0L,SEEK_END);      

  return 0;
  }  // _Int13hReadSector


//------------------------------------------------------------------------------
//
//                        _Int13hWriteSector
//
//    This procedure interfaces the following 'C' function:
//     int Int13hWriteSector(UCHAR drive, UCHAR *buf, ULONG offset);
//
// BOOL WriteFile(
//  HANDLE       hFile,
//  LPCVOID      lpBuffer,
//  DWORD        nNumberOfhddbytesToWrite,
//  LPDWORD      lpNumberOfhddbytesWr,
//  LPOVERLAPPED lpOverlapped);
//
int Int13hWriteSector(char *drive, char *buf, int numSector)
  {
  PhysicalDrive[4] = drive[0];                  // = Volume name (see above)
  PhysicalDrive[5] = drive[1];                  // = Colon

  hDrive = CreateFile(PhysicalDrive,            // Drive to open eg. "\\\\.\\C:"
                      GENERIC_WRITE,            // Access mode = WRITE
                      FILE_SHARE_READ | FILE_SHARE_WRITE,   // Share Mode
                      NULL,                     // Security Descriptor
                      OPEN_EXISTING,            // How to create
                      0,                        // File attributes
                      NULL);                    // Handle to template

  if (hDrive == INVALID_HANDLE_VALUE) return ERR;

  SetFilePointer(hDrive, numSector*512, NULL, FILE_BEGIN);
  if (!WriteFile(hDrive, buf, 512, &hddbytesWr, NULL))  return ERR;

  CloseHandle(hDrive);
  return 0;
  } // _Int13hWriteSector
 
//--------------------------end-of-module------------------------------------





                              