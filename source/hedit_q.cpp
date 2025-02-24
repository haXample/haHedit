// hedit_q.cpp - C++ Developer source file.
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

//----------------------------------------------------------------------------
//                         Function & Variable declarations
//----------------------------------------------------------------------------
void update_file(int);
extern unsigned long int TmpFileLseek(int fh, unsigned long offs, int org);
unsigned long int PCIBiosPresent();

extern int drvSector;
extern char *str;

unsigned int ioidx, iodat, iolen;
unsigned int pcibus, pcifct;

unsigned char upitype;

//----------------------------------------------------------------------------
//                         external declarations
//----------------------------------------------------------------------------
extern char * no_room;
extern char * open_failed;
extern char * invalid_param;
extern char * has;
extern char * have;
extern char * been_written;
extern char * changes_lost;
extern char * illegal_command;
extern char * evn;
extern char * block_update;

extern void DebugStop(char *, int);
extern void DebugPrintBuffer(char *, int);

extern void SetSpaceAttrib(unsigned char);
extern void ClrRow24();
extern void copy_filename(char *, char *);
extern int TmpFileWrite(int, unsigned char *, int);
extern int TmpFileRead(int, unsigned char *, int);
extern int TmpFileClose();
extern int TmpFileUnlink();
extern int TmpFileOpen();
extern unsigned int merge_file();
extern void DisplayMode(unsigned char);
extern void DisplayEOF(unsigned long);
extern void DisplayChksum(unsigned int);
extern void DisplaySize(unsigned long);
extern void BusyPrompt();
extern void SetAttribute(unsigned char);
extern void AttrDisplayStr(char *);
extern void AttrDisplayUserGuide(char *); 
extern int DisplayEditFilename(char *);
extern void QuitMenu(); 
extern void ReadyPrompt();
extern int set_cursor(unsigned char);
extern unsigned char get_key();
extern int string_cpy(char *, char *, int);
extern int string_cmp(char *, char *, int);
extern int chk_filename(char c[], int *);
extern void InitScreen(); 
extern int display_file();
extern int get_string(unsigned char[], unsigned char[], int *, char);
extern int get_g_name(unsigned char);
extern int chk_integrity();

extern int Int13hReadSector(char *, char *, int);
extern int Int13hWriteSector(char *, char *, int);

extern char tmp_file[];

extern char * bak_suffix;
extern char * evn_suffix;
extern char * odd_suffix;
extern char * mrg_suffix;
extern char * hex_suffix;
extern char * hedit_tmp;

extern unsigned char inbuf[256];
extern char f_buf[_SIZE];

extern char bak_name[_LENGTH+5];
extern char evn_name[_LENGTH+5];
extern char odd_name[_LENGTH+5];
extern char mrg_name[_LENGTH+5];

extern char *g_string;

extern char argv_name[_LENGTH+5];
extern char argv_sav[_LENGTH+5];
extern char g_name[_LENGTH+5];                                     
extern unsigned char nam_rubtab[_LENGTH+1];
extern char sav_string[_LENGTH+1];
extern char sav_rubtab[_LENGTH+1];
extern char cmd_line[_LENGTH+1];
extern char cmd_rubtab[_LENGTH+1];

extern char *p;

extern unsigned char key_code;

extern int i,j,k,l,m,n;
extern int n_sav, n_nam, n_cmd;
extern int x_pos, y_pos;

extern unsigned char backup_flag, hedit_mode;
extern unsigned char update_flag, xchg_flag, rdy_flag, row24_flag, row25_flag;

extern int fh, g_fh, fh_tmp, fh_evn, fh_odd;
extern unsigned int bytesrd, g_bytesrd, bytwrtn;
extern unsigned long hddbytesRd, hddbytesWr;


extern unsigned int chksum, new_blk_chksum, old_blk_chksum;

extern unsigned long int li, g_offset, offset, mem_pos, f_pos, f_size, f_eof;

//----------------------------------------------------------------------------
//
//                             YesNoPrompt
//
int YesNoPrompt()
  {
  putch(_BEEP);
  if ((key_code = get_key()) == 'Y') return(TRUE);
  else
    {
    ClrRow24();
    return(FALSE);
    }
  } // YesNoPrompt

//--------------------------------------------------------------------------
//
//                              asc2hex
//
unsigned char asc2hex(unsigned char *asc_str)
  {
  unsigned char _c;

  _c = toupper(*asc_str);
  if (_c >= '0' && _c <= '9') _c = _c - '0';
  else if (_c >= 'A' && _c <= 'F') _c = _c - '7';
  else _c = 0xFF;
  return (_c);
  }  // asc2hex

//ha////--------------------------------------------------------------------------
//ha////
//ha////                              asc2hex
//ha////
//ha//unsigned char asc2hex(char *asc_str)
//ha//  {
//ha//  unsigned char _c=0;
//ha//
//ha//  _c = toupper(*asc_str);
//ha//  if (_c >= '0' && _c <= '9') _c = _c - '0';
//ha//  else if (_c >= 'A' && _c <= 'F') _c = _c - '7';
//ha//  else _c = 0xFF;
//ha//  return (_c);
//ha//  }  // asc2hex

//--------------------------------------------------------------------------
//
//                              asc2int
//
//ha//unsigned long asc2int(char *asc_str)
//ha//  {
//ha//  unsigned long rval = 0L;
//ha//  unsigned char digit = 0, i = 0;
//ha//
//ha//  while (*asc_str >= '0' && (*asc_str <= '9'))      // loop until end of string
//ha//    {
//ha//    rval = (rval*(RADIX_10) + (unsigned long)(*asc_str-'0'));  // calculate the constant
//ha//    asc_str++;                                       // next ascii char
//ha//    } // end while
//ha//  return(rval);
//ha//  }  // asc2int


//---------------------------------------------------------------------------
//
//                      AscHex2Bin
//
// Example: Converts "KY RM 33 34 62 61 45 46 35 34" into "0034baED54"
//
int AscHex2Bin(char *_inbuf, char *_outbuf, int _bytesrd) 
  {
  char _tmpbuf[2];
  int j, _byteswr;

  _byteswr =0,  j = 0;
  for (i=0; i < _bytesrd; i++)
    {
//ha//DebugStop("_inbuf", i);
//ha//DebugPrintBuffer(&_inbuf[i], 2);
//    if (asc2hex((unsigned char *)&_inbuf[i+j]) != 0xFF && !j && (_inbuf[i] != ' '))
    if (!j && (_inbuf[i] != ' '))
      {
      _tmpbuf[0] = (char)asc2hex((unsigned char *)&_inbuf[i]);
      _tmpbuf[1] = (char)asc2hex((unsigned char *)&_inbuf[i+1]);
//ha//DebugStop("_tmpbuf", 0);
//ha//DebugPrintBuffer(&_tmpbuf[0], 2);
      if (((unsigned char)_tmpbuf[0] == 0xFF) || ((unsigned char)_tmpbuf[1] == 0xFF))
        {                      
        _outbuf[_byteswr] = 0x00;
        }                                      
      else _outbuf[_byteswr] = (_tmpbuf[0] <<4) | _tmpbuf[1];
      j = 1;
      _byteswr++;
      }
    else j = 0;
//ha//DebugStop("_outbuf", _byteswr);
//ha//DebugPrintBuffer(&_outbuf[_byteswr], 1);
    }
//ha//printf("bytesrd = %d, byteswr = %d\n", _bytesrd, _byteswr);
//ha//DebugPrintBuffer(_outbuf, _byteswr);
  return (_byteswr);
  } // AscHex2Bin

          
//--------------------------------------------------------------------------
//
//                            get_hexnr
//
int get_hexnr(unsigned int *hexnr, char *asc_str, char brkchr)
  {
  unsigned char c;
  unsigned int h = 0, i = 0;

  while (asc_str[i] != brkchr)
    {
    if ((c=asc2hex((unsigned char *)&asc_str[i])) > 0x0F) break;
    h = (h * 16) + c;
    i++;
    } // while

  if (c > 0x0F) return(FALSE);
  else
    {                                                   
    *hexnr = h;
    return(TRUE);
    }
  } // get_hexnr

//----------------------------------------------------------------------------
//
//                          GetCmdlineRange
//
int GetCmdlineRange(unsigned char sw, char *cmdline, unsigned int *start,
                    unsigned int *end, unsigned int *len)
  {
  if ((p=strpbrk(cmdline,",")) == 0) return(FALSE);         // find (xxxx,
  i = p - cmdline;                                          // next str
  if (!get_hexnr(start,cmdline,',')) return(FALSE);         // convert hexnr

  if ((p=strpbrk(&cmdline[i+1],")")) == 0) return(FALSE);   // find (xxxx,xxxx)
  j = p - cmdline;                                          // next str
  if (!get_hexnr(end,&cmdline[i+1],')')) return(FALSE);     // convert hexnr

  if (cmdline[j+1] != '=') *len = 256;                      // default length
  else
    {
    if (!get_hexnr(len,&cmdline[j+2],0)) return(FALSE);     // convert hexnr
    }
  if (sw == 0)
    {
    if ((*end <= *start) || (*len >= 0xFFFE) ||
        (*start >= 0xFFFE) || (*end >= 0xFFFE) || (*len == 0))
      {
      return(FALSE);
      }
    else return(TRUE);
    }
  else return(TRUE);
  } // GetCmdlineRange

//----------------------------------------------------------------------------
//
//                             ChkCmdlineOption
//
int ChkCmdlineOption(char *cmdline, char *cmpstr)
  {                               // either 1st letter or full option name
  if (((strlen(cmdline) > 1) &&   // should be given at the cmd_line
      !string_cmp(cmdline,cmpstr,strlen(cmpstr))) ||
      (strlen(cmdline) > strlen(cmpstr)))
    {
    printf(invalid_param);        // invalid option
    return(FALSE);
    }
  return(TRUE);                   // valid option
  } // ChkCmdlineOption

//----------------------------------------------------------------------------
//
//                            SetFilenameSuffix
//
int SetFilenameSuffix(char *filename, char *suffix)
  {
  i = 0;                                    // allow ..\..\..\pathname
  if ((p = strrchr(filename,'\\')) != 0) i = (p - filename);
  if ((p = strrchr(&filename[i],'.')) != 0) i = (p - filename);
  else i = strlen(filename);
  strcpy(&filename[i], suffix);
  return(TRUE);
  } // SetFilenameSuffix

//----------------------------------------------------------------------------
//
//                             InitFileEdit
//
int InitFileEdit(unsigned char sw)
  {
  // check and open input file, copy into tmp_file, _CReate bakup file
  if (!chk_filename(argv_name, &n_cmd)) return(FALSE);

  if ((fh = open(argv_name,O_RDONLY|O_BINARY))==ERR) // open input file
    {

INIT_FILE_ABORT:                             //@005

    printf(open_failed,argv_name);
    return(FALSE);
    }
  f_size = lseek(fh,0L,SEEK_END);             // get input file size

  // check if File is empty                   //@005
  if (f_size == 0) goto INIT_FILE_ABORT;      //@005

  if (_QUIT_INIT) TmpFileClose();             // open/re-open tmp_file
  if (TmpFileOpen() == FALSE) return(FALSE);

  // copy file to tmp_file and calculate file checksum
  chksum = 0;             
  f_pos = lseek(fh,0L,SEEK_SET);

  while (bytesrd = read(fh,f_buf,_SIZE))
    {
    for (i=0;i<bytesrd;i++) chksum = chksum + f_buf[i];
    if ((bytwrtn = TmpFileWrite(fh_tmp, (unsigned char *)f_buf,bytesrd)) == ERR)
      {
      printf(no_room,tmp_file);
      TmpFileUnlink();
      return(FALSE);
      }
    } // end while

  close(fh);                                  // close input file

  strcpy(bak_name, argv_name);                // _CReate backup filename
  SetFilenameSuffix(bak_name, bak_suffix);
  return(TRUE);
  } // InitFileEdit

//----------------------------------------------------------------------------
//
//                             init_hedit
//
int init_hedit(unsigned char sw)
  {
  unsigned int i, j, k;

  if ((sw == _1ST_INIT) && !chk_integrity())        // check HEDIT copyright
    {                                               // stop it and give any
    printf(open_failed,argv_name);                  //  dummy error message.
    return(FALSE);
    }

  if ((p = strpbrk(cmd_line,"/")) != NULL)          // Check for options
    {
    i = p - cmd_line;                               // cmd_line = p="/merge test001_abc" --> i=0
    if ((p = strpbrk(cmd_line," ")) == NULL)        // NO spaces found (else only 1 space at most)
      {
      j = i;
      string_cpy(argv_name, cmd_line, j);
      argv_name[j] = 0;
      }
    else  // p points to the space deliniter (one space at most)
      {                                             // p=" test1_abc"
      p++;                                          // p="test1_abc"  --> Skip space   
      string_cpy(argv_name, p, strlen(p));          // strlen(p)=9
      argv_name[strlen(p)] = 0;                     // Terminate string
      }

    switch(toupper(cmd_line[i+1]))                  // cmdline option parser
      {
      case 'M':                                     // options /MEMORY & /MERGE
        if (string_cmp(&cmd_line[i+1],"MERGE",5) ||
            string_cmp(&cmd_line[i+1],"merge",5))   // @001 no case sensitivity
          {
          hedit_mode = MOD_FILE;                    // file editor
          cmd_line[i+6] = 0;  n_cmd = 6;            // truncate cmd_line "/MERGE"
          copy_filename(mrg_name,argv_name);
          copy_filename(odd_name,argv_name);
          copy_filename(evn_name,argv_name);

          i = 0;                                    // allow ..\..\..\pathname
          if ((p = strrchr(argv_name,'\\')) != 0) i = (p - argv_name);
          if ((p = strrchr(&argv_name[i],'.')) != 0)
            {
            p[0] = 0;
            printf("%s.suffix not expected!\n"
                   "Usage: 'HEDIT /MERGE %s'", argv_name, argv_name);
            return(FALSE);
            }
          else
            {
            i = strlen(mrg_name);
            strcpy(&mrg_name[i],mrg_suffix);        // create the filenames
            strcpy(&odd_name[i],odd_suffix);
            strcpy(&evn_name[i],evn_suffix);
            if (!merge_file()) return(FALSE);       // merge ODD/EVN to .MRG
            strcpy(argv_name,mrg_name);             // fake input filename
            if (!InitFileEdit(sw)) return(FALSE);   // open files for edit
            }
          } // "/MERGE"

        else                                  // option /MEMORY:  4G flat space
          {
          hedit_mode = MOD_MEMORY;
          if (!ChkCmdlineOption(&cmd_line[i+1],"MEMORY")) return(FALSE);
          cmd_line[i+2] = 0; n_cmd = i+2;     // truncate cmd_line

          if (sw == _QUIT_INIT) TmpFileClose();
          if (TmpFileOpen() == FALSE) return(FALSE);
          f_size = TmpFileLseek(fh_tmp,0L,SEEK_END); // get input file size
          }
        break;
    
      case 'H':                               // @003 /HDD hard disk sector edit mode
        if (!strncmp(&cmd_line[1],"HDD",3) || 
            !strncmp(&cmd_line[1],"hdd",3))
          { 
          hedit_mode = MOD_HDD;
          cmd_line[4] = 0;  n_cmd = 4;        // Truncate cmd_line "/HDD"
          argv_name[2] =0;                    // argv_name = drive_letter
                                              
          while (cmd_line[i+5] != ' ') i++;   // Skip cmd_line to sector number
          drvSector = atoi(&cmd_line[i+1+5]); // Calculate sector number

          if (Int13hReadSector(argv_name, f_buf, drvSector) == ERR) 
            {
            printf("Hard Disk read error.");
            return(FALSE);
            }
          }
        else
          {
          printf(invalid_param);
          return(FALSE);
          }
        break;  // @003 'HDD'

      default:
        printf(invalid_param);
        return(FALSE);
        break;
      } // end switch
   }                                      

  else
    {
    hedit_mode = MOD_FILE;            // default mode: file editor
    n_nam = 0;                        // clr filename buffers
    copy_filename(argv_name,cmd_line);
    if (!InitFileEdit(sw)) return(FALSE);
    }

  InitScreen();                       // for all modes: init screen display
  DisplayMode(hedit_mode);
  DisplaySize(f_size);
  if (f_size != 0xFFFFFFFF) f_eof = f_size - 1L;      // normal
  else f_eof = f_size;                                // 4G boundary wraps
  DisplayEOF(f_eof);
  DisplayChksum(chksum);
  return(TRUE);
  } // init_hedit

//----------------------------------------------------------------------------
//
//                          CalcBlkChksum
//
void CalcBlkChksum()
  {
  unsigned int chksum = 0, bytesrd;
  unsigned long li, lj;

  lj = (unsigned long)(y_pos*16 + x_pos/3) + offset;

  if (g_offset <= lj)
    {
    li = lj - g_offset;
    f_pos = TmpFileLseek(fh_tmp, g_offset, SEEK_SET);
    }
  else
    {
    li = g_offset - lj;
    f_pos = TmpFileLseek(fh_tmp, lj, SEEK_SET);
    }

  li++;
  while (li/(unsigned long)_SIZE != 0)
    {
    TmpFileRead(fh_tmp, (unsigned char *)f_buf, _SIZE);
    for (i=0; i<_SIZE; i++) chksum += f_buf[i];
    li -= (unsigned long)_SIZE;
    }
  if (li != 0)
    {
    bytesrd = TmpFileRead(fh_tmp, (unsigned char *)f_buf, (unsigned int)li);
    for (i=0; i<bytesrd; i++) chksum += f_buf[i];
    }

  f_pos = TmpFileLseek(fh_tmp, offset, SEEK_SET);
  ClrRow24();
  printf("Block Checksum (%08lX,%08lX) = %04X", g_offset, lj, chksum);
  } // end CalcBlkChksum

//----------------------------------------------------------------------------
//
//                           block_chksum
//
void block_chksum(unsigned int *blk_chksum)
  {
  *blk_chksum = 0;
  for (i=0;i<256;i++) *blk_chksum = *blk_chksum + (unsigned int)inbuf[i];
  } // block_chksum

//----------------------------------------------------------------------------
//
//                          update_chksum
//
int update_chksum(unsigned char sw)
  {
  if (hedit_mode == MOD_MEMORY) return(FALSE);

  if (sw)                               // calculate the total file chksum
    {
    f_pos = TmpFileLseek(fh_tmp,0L,SEEK_SET);  // re-position tmp_file pointer
    chksum = 0;
    while (g_bytesrd = TmpFileRead(fh_tmp,(unsigned char *)f_buf,_SIZE))
      {
      for (i=0; i<g_bytesrd; i++) chksum = chksum + f_buf[i];
      } // end while
    }
  else                                  // only current block changed chksum
    {
    block_chksum(&new_blk_chksum);
    chksum = chksum - old_blk_chksum + new_blk_chksum;
    old_blk_chksum = 0;
    new_blk_chksum = 0;
    }

  DisplayChksum(chksum);
  return(TRUE);
  } // update_chksum

//----------------------------------------------------------------------------
//
//                             update_file
//
void update_file(int sw)
  {
  row24_flag = TRUE;
  ClrRow24();

  f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);  // get current file position for update

  // update temp file
  if ((bytwrtn = TmpFileWrite(fh_tmp,inbuf,bytesrd)) == ERR)
    {
    printf(block_update,"failed");
    abort();
    }
  f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);  // get current file position
  if (sw) printf(block_update,"completed");
  update_chksum(_BLOCK_UPDATE);
  xchg_flag = FALSE;
  } // update_file

//----------------------------------------------------------------------------
//
//                                 quit_write
//
int quit_write()
  {
  if (get_g_name(_OUTPUT_FILE))  // Output file
    {
    BusyPrompt();
    ClrRow24();
    printf("%s",g_name);

    if ((g_fh = open(g_name,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR)  //@001
      {
      ClrRow24();
      printf(open_failed,g_name);
      return (FALSE);
      }
    else
      {
      f_pos = TmpFileLseek(fh_tmp,0L,SEEK_SET);
      while (g_bytesrd = TmpFileRead(fh_tmp, (unsigned char*)f_buf, _SIZE))
        {
        if ((bytwrtn = write(g_fh, (unsigned char*)f_buf, g_bytesrd)) == ERR)
          {
          ClrRow24();
          printf(no_room,g_name);
          close(g_fh);
          unlink(g_name);
          return(FALSE);
          }
        } // end while

      close(g_fh);
      printf(has,been_written);
      return(TRUE);
      }
    }
  return (FALSE);
  } // quit_write

//----------------------------------------------------------------------------
//
//                             quit_exit
//
int quit_exit(unsigned char sw)
  {
  BusyPrompt();
  ClrRow24();

  switch(hedit_mode)
    {
    case MOD_MEMORY:
      TmpFileUnlink();                     // delete tmp_file
      return(TRUE);
      break;

      case MOD_HDD:                        //@003
        f_pos = TmpFileLseek(fh_tmp,0L,SEEK_SET);
        TmpFileRead(fh_tmp,(unsigned char*)f_buf,(int)f_size);
        if (update_flag)
          {
          AttrDisplayUserGuide("Write changed sector to Hard Disk (y/[n])?");
          if (YesNoPrompt())
            {
            AttrDisplayUserGuide("HDD may become unusable! Abort / Write (a/[w])?");
            if ((key_code = get_key()) == 'W')
              {
              if (Int13hWriteSector(argv_name, f_buf, drvSector) == ERR)
                {
                printf("Hard Disk write error.");
                }
              } // 2nd query
            } // 1st query
          }
        close(fh_tmp);
        unlink(tmp_file);                  // delete tmp_file
        return(TRUE);
        break;
      
    case MOD_FILE:                         // default
    default:
      printf("%s",argv_name);

//@001      if (!backup_flag)
//@001        {
//@001        unlink(bak_name);                 // delete bak_file, if any exists
//@001        rename(argv_name,bak_name);       // backup input file <oldname,newname>
//@001        backup_flag = TRUE;
//@001        }

      fh = open(argv_name,O_WRONLY|O_BINARY|O_TRUNC,S_IWRITE);

      f_pos = TmpFileLseek(fh_tmp,0L,SEEK_SET);
      while (g_bytesrd = TmpFileRead(fh_tmp, (unsigned char*)f_buf, _SIZE))
        {
        if ((bytwrtn = write(fh, (unsigned char*)f_buf, g_bytesrd)) == ERR)
          {
          ClrRow24();
          printf(no_room,argv_name);
          close(fh);
          unlink(argv_name);
          rename(bak_name,argv_name);     // restore input file <oldname,newname>
          backup_flag = FALSE;
          return(FALSE);
          }
        } // end while

      close(fh);

      if (sw) TmpFileUnlink();            // delete tmp_file
      else f_pos = TmpFileLseek(fh_tmp,offset,SEEK_SET);

      printf(has,been_written);
      return(TRUE);
      break;
    } // end switch
  } // quit_exit

//----------------------------------------------------------------------------
//
//                             quit_init
//
int quit_init()
  {
  unsigned char tmp;

  if (update_flag)
    {
    AttrDisplayUserGuide(changes_lost);
    if (!YesNoPrompt()) return(FALSE);
    }

  ClrRow24();
  AttrDisplayUserGuide("enter [file [/option]]: ");

  if (hedit_mode == MOD_HDD) strcpy(cmd_line, argv_sav);  //@003 display complete cmd                                         //@003
  if (hedit_mode == MOD_MEMORY) strcpy(cmd_line, "/M");   //@003 display current option                                         //@003

  n_cmd = strlen(cmd_line);                               //@003 length of complete cmdline 
  if (get_string((unsigned char *)cmd_line, (unsigned char *)cmd_rubtab, &n_cmd,0))
    {
    ClrRow24();
//@003    strcpy(argv_sav,argv_name);       
    strcpy(argv_sav,cmd_line);        //@003 save current cmd_line
    tmp = hedit_mode;                 // save current mode

    if (init_hedit(_QUIT_INIT))
      {
      update_flag = FALSE;
      return(TRUE);
      }
    else
      {
      n_cmd = n_sav;
      string_cpy((char *)cmd_line,(char *)sav_string,n_sav);
      string_cpy((char *)cmd_rubtab, (char *)sav_rubtab, n_sav);
      strcpy(argv_name,argv_sav);
      hedit_mode = tmp;               // restore current mode
      return(FALSE);
      }
    }
  else return(FALSE);
  } // quit_init

//----------------------------------------------------------------------------
//
//                             quit-abort
//
int quit_abort()
  {
  if (update_flag)
    {
    AttrDisplayUserGuide(changes_lost);
    if (!YesNoPrompt()) return(FALSE);
    }

  TmpFileUnlink();                     // delete tmp_file
  BusyPrompt();
  ClrRow24();
  return(TRUE);
  } // quit_abort

//----------------------------------------------------------------------------
//
//                             write_f_buf
//
int write_f_buf(unsigned char odd_evn, unsigned int fh_odd_evn)
  {
  unsigned int bytesread;

  bytesread = g_bytesrd;
  j = odd_evn;
  if (j == EVN && (bytesread % 2 != 0)) bytesread++; // make even if not

  for (i=0;i<bytesread/2;i++)
    {
    f_buf[i+_SIZE/2] = f_buf[j];
    j = j + 2;
    }

  if ((bytwrtn = write(fh_odd_evn,(unsigned char *)&f_buf[_SIZE/2],bytesread/2)) == ERR)
    {
    ClrRow24();
    printf(no_room,odd_name); printf(evn);
    close(fh_evn);
    close(fh_odd);
    unlink(evn_name);
    unlink(odd_name);
    return(FALSE);
    }
  else return(TRUE);
  } // write_f_buf

//----------------------------------------------------------------------------
//
//                                 hex_asc
//
unsigned char hex_asc(unsigned char hexchar)
  {
  if (hexchar <= 9) return(hexchar + '0');
  else return((hexchar-0x0A) + 'A');
  } // hex_asc

//----------------------------------------------------------------------------
//
//                                 put_byte
//
void put_byte(unsigned char *buf, unsigned char byte)
  {
  chksum += byte;

  *buf = hex_asc((unsigned char)(byte >> 4));
  buf++;
  *buf = hex_asc((unsigned char)(byte & 0x0F));
  } // put_byte

//----------------------------------------------------------------------------
//
//                             quit_bin2hex
//
// Converts a binary input file into INTEL MCS-86 Object file record format.
// The Intel 16-bit Hexadecimal Object file record format has a 9-character,
// (4-field) prefix that defines the start of record, byte count, load address,
// and record type, followed by the record and a 2-character checksum suffix.
// The four record types are:
//    00 = data record, including offset address
//    01 = end of file record
//    02 = extended segment address record
//         (added to the offset to determine the absolute load address)
//    03 = start record, not used
// BIN2HEX source [destination] [/C:count | /MCS-85]
//   /C:count   Specifies the number of data bytes per record,
//              the valid range is 1..32 (default := 16).
//   /MCS-85    This mode will not use the extended record type 02, implicating
//              that the size of the binary input file may not exceed 64K.
//
#define DATBLK 256/2          // number hex lines per extended segment record
#define DATFLD 16*2           // number of bytes per hex line

int quit_bin2hex()
  {
  unsigned int hexix=0, hex_seg=0, hex_off=0, datcnt;
  unsigned char *hexbuf = (unsigned char *)&f_buf[DATBLK*DATFLD];      // [(1+2+4+2+(2*DATFLD)+2+1+1)]

  strcpy(g_name,bak_name);            // suggest a default name *.HEX
  i = strlen(bak_name);
  strcpy(&g_name[i-4],hex_suffix);
  n_nam = strlen(g_name);             // init default name length for re-edit
  if (!get_g_name(_OUTPUT_FILE))                 // 1: Output file
    {
    n_nam = 0;                        // invalidate g_name buffer
    return(FALSE);                    // default name may be re-edited
    }

  BusyPrompt();
  ClrRow24();

  if ((strstr(argv_name,hex_suffix) != 0) ||
      ((g_fh = open(g_name,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR))  //@001
    {                                                                         //@001
    printf(open_failed,g_name);
    return(FALSE);
    }

  printf("%s",g_name);
  f_pos = TmpFileLseek(fh_tmp,0L,SEEK_SET);
  while ((g_bytesrd = TmpFileRead(fh_tmp, (unsigned char *)f_buf,DATBLK*DATFLD)))
    {
    if (f_size > 0x10000L)
      {
      hexbuf[0] =':';  chksum=0;            // init extended address rec type
      put_byte(&hexbuf[1],0x02);            // byte count for segment field
      put_byte(&hexbuf[3],0x00);            // address field always 0
      put_byte(&hexbuf[5],0x00);
      put_byte(&hexbuf[7],0x02);            // record type 02 identifier
      put_byte(&hexbuf[9],(unsigned char)(hex_seg >> 8));  // edit segment field address
      put_byte(&hexbuf[11],(unsigned char)(hex_seg & 0xFF));
      put_byte(&hexbuf[13],(unsigned char)(-chksum));      // checksum byte
      hexbuf[15] = '\r';                    // skip new line in hex file
      hexbuf[16] = '\n';
      hexix = (7*2)+3;                      // init index into hexbuf
      hex_off=0;                            // reset hex_offset
      }
    else hexix=0;                           // reset hex index

    for (i=0; i<DATBLK; i++)                // block count
      {
      if (g_bytesrd >= DATFLD)
        {
        datcnt = DATFLD;
        g_bytesrd -= DATFLD;
        }
      else datcnt = g_bytesrd;

      hexbuf[hexix++] = ':';  chksum=0;
      put_byte(&hexbuf[hexix],(unsigned char)datcnt); // byte cnt of data field
      hexix += 2;                                     // update index cnt
      put_byte(&hexbuf[hexix],(unsigned char)((i*DATFLD+hex_off)>>8)); // offset range
      hexix += 2;                                     // update index cnt
      put_byte(&hexbuf[hexix],(unsigned char)(i*DATFLD+hex_off));
      hexix += 2;                                     // update index cnt
      put_byte(&hexbuf[hexix],0x00);                  // record type 00 identifier
      hexix += 2;                                     // update index cnt

      for (j=0; j<datcnt; j++)
        {
        put_byte(&hexbuf[hexix],f_buf[(DATFLD*i)+j]); // data
        hexix += 2;                                   // update index cnt
        }

      put_byte(&hexbuf[hexix],(unsigned char)(-chksum));    // checksum byte
      hexix += 2;                                   // update index cnt
      hexbuf[hexix++] = '\r';                       // skip new line in hex file
      hexbuf[hexix++] = '\n';

      if (datcnt < DATFLD) break;
      } // end segment block

    if (write(g_fh,hexbuf,hexix) == ERR)
      {
      ClrRow24();
      printf(no_room,g_name);
      close(g_fh);
      unlink(g_name);                           // delete hex_file
      return(FALSE);
      }
                                                // DATBLK =256
    hex_seg += (DATFLD*DATBLK >> 4);            // next segment address
    hex_off += DATFLD*DATBLK;                   // only used if MCS-85
    } // end while

  if (write(g_fh,":00000001FF\r\n",13) == ERR)
    {
    ClrRow24();
    printf(no_room,g_name);
    close(g_fh);
    unlink(g_name);                             // delete hex_file
    return(FALSE);
    }
  else
    {
    close(g_fh);
    printf(has,been_written);
    return(TRUE);
    }
  } // convert_binfile

//----------------------------------------------------------------------------
//
//                             quit-split
//
int quit_split()
  {
  if (f_size % 2 != 0)
    {
    ClrRow24();
    printf("size of input file not even, continue? (y or [n])");
    if (!YesNoPrompt()) return(FALSE);
    }

  BusyPrompt();
  ClrRow24();

  strcpy(evn_name,bak_name);
  strcpy(odd_name,bak_name);
  i = strlen(bak_name);
  strcpy(&evn_name[i-4],evn_suffix);
  strcpy(&odd_name[i-4],odd_suffix);

  if (((fh_evn = open(evn_name,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR) ||   //@001
      ((fh_odd = open(odd_name,O_RDWR|O_BINARY|O_TRUNC|O_CREAT,S_IWRITE))==ERR))     //@001
    {
    printf(open_failed,odd_name); printf(evn);
    return(FALSE);
    }

  li = 0;
  i = strlen(argv_name);
  if (argv_name[i-4] == '.')               // skip "EXE" pre-record, if any
    {
    if (string_cmp(&argv_name[i-4],".EXE",4))
      {
      li = 512L;                           // skip "EXE" pre-record (=512 bytes)
      printf("EXE2BIN ");
      }
    }

  printf("%s%s",odd_name,evn);
  f_pos = TmpFileLseek(fh_tmp,li,SEEK_SET);
  while (g_bytesrd = TmpFileRead(fh_tmp, (unsigned char *)f_buf,_SIZE/2))
    {
    if (!write_f_buf(EVN,fh_evn)) return(FALSE);
    if (!write_f_buf(ODD,fh_odd)) return(FALSE);
    } // end while

  close(fh_evn);
  close(fh_odd);
  printf(have,been_written);
  return(TRUE);
  } // quit_split

//----------------------------------------------------------------------------
//
//                                   quit
//
int quit()
  {
  row25_flag = TRUE;                          // prepare Row25
  rdy_flag = FALSE;

  ClrRow24();
  if (hedit_mode != MOD_FILE) printf("no input file");
  else
    {
    DisplayEditFilename(NULL);                // NULL-Pointer: clear filename
    SetAttribute(NORMAL);                     // @001
    AttrDisplayStr("Editing ");               // @001
    AttrDisplayStr(argv_name);                // @001
    }

  while (!rdy_flag)
    {
    ReadyPrompt();                            // tell user to enter choice
    if (row25_flag) QuitMenu();               // print quit menu

    set_cursor(_NORM_CURSOR);
    key_code = get_key();                     // wait and get key from user

    switch (key_code)
      {
      case 'A':
        rdy_flag = TRUE;
        if (hedit_mode == MOD_MEMORY)          //@001
          {
          TmpFileUnlink();                     // delete tmp_file
          return(TRUE);
          }
        else if (quit_abort()) return(TRUE);
        break;
      case 'B':
        if (hedit_mode == MOD_FILE)
          {
          update_file(_NO_CHANGE);
          quit_bin2hex();
          }
        else putch(_BEEP);
        break;
      case 'E':
        update_file(_NO_CHANGE);
        if (quit_exit(_DELETE_TMP_FILE)) return(TRUE);
        break;
      case 'I':
        rdy_flag = TRUE;
        if (quit_init()) return(INIT);
        break;
      case 'S':
        if (hedit_mode == MOD_FILE)
          {
          update_file(_NO_CHANGE);
          quit_split();
          }
        else putch(_BEEP);
        break;
      case 'U':
        if (hedit_mode == MOD_FILE)
          {
          if (xchg_flag)
            {
            update_file(_CHANGE);
            display_file();
            }
          else update_file(_NO_CHANGE);
          if (quit_exit(_KEEP_TMP_FILE)) update_flag = FALSE;
          }
        else putch(_BEEP);
        break;
      case 'W':
        update_file(_NO_CHANGE);
        if (quit_write()) update_flag = FALSE;
        row25_flag = TRUE;
        break;
      case _LF:              // _CR/_LF: Allows termination in batch files
      case _CR:
      case _ESC:
        rdy_flag = TRUE;
        ClrRow24();
        break;
      default:
        putch(_BEEP);
        ClrRow24();
        printf(illegal_command);
        break;
      }
    } // end while

  if (hedit_mode == MOD_FILE) DisplayEditFilename(argv_name);
  ReadyPrompt();
  return(FALSE);
  } // quit

//--------------------------end-of-module------------------------------------

//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
//ha//{
//ha//printf("\ninit_hedit\n argv_name = %s\ncmd_line = %s\nodd_name = %s\n\nPress ESC to continue..", argv_name, cmd_line, odd_name);  
//ha//while (getch() != '\x1B');                                                  
//ha//}
//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---

//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
//ha//{
//ha//printf("\ninit_hedit [3]\n argv_name = %s\ncmd_line = %s\np = %s\nstrlen(p) = %d\nPress ESC to continue..", argv_name, cmd_line, p, strlen(p)); 
//ha//while (getch() != '\x1B');                                                  
//ha//}
//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---

//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---
//ha//{
//ha//printf("\ninit_hedit [2]\n argv_name = %s\ncmd_line = %s\np = %s\ni = %d, j = %d\nPress ESC to continue..", argv_name, cmd_line, p, i,j); 
//ha//while (getch() != '\x1B');                                                  
//ha//}
//ha////ha////---DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG------DEBUG---

