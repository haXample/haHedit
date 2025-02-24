// hedit_tdes.cpp - C++ Developer source file.
// (c)2021 by helmut altmann

// 3DES (TDES, TDEA) Algorithm Encryption Modes for file encryption

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

#include <io.h>        // File open, close, access, etc.
#include <conio.h>     // For _putch(), _getch() ..
#include <string>      // printf, etc.

#include<sys/stat.h>   // For filesize
#include<iostream>     // I/O control
#include<fstream>      // File control

#include <windows.h>   // For console specific functions
      
using namespace std;

//-----------------------------------------------------------------------------
//
#define UCHAR unsigned char
#define UINT unsigned int
#define ULONG unsigned long int

#define FALSE  0
#define TRUE   1
#define ERR   -1

#define ISOPAD 0x80
#define PAD    0x00

#define ENCRYPT    0
#define DECIPHER   ENCRYPT ^ 0x01

#define TDES_ENCRYPT       15
#define TDES_DECIPHER      16
#define TDES_MAC           17
#define TDES_CBCE          18
#define TDES_CBCD          19
#define TDES_ECBE          20
#define TDES_ECBD          21
#define TDES_ECBDECIPHER   22
#define TDES_ECBENCRYPT    23

#define KEY_LENGTH 24
#define BLOCK_SIZE 8

#define COUNT_RATE 100000L

//----------------------------------------------------------------------------
//
//                          External declarations
//
extern "C" void desAlgorithm2(char* p1, char* p2, int);  // Assembler Module Interface
extern "C" void tdesKeyInit2(char* p, int, int);         // Assembler Module  Interface

extern void DebugPrintBuffer(char *, int);
extern void DebugStopBuf(unsigned char, unsigned long);

//ha//extern void ClearScreen();
//ha//extern void More();
//ha//extern void AnyKey();

extern char* open_failed;
extern char file_exists[];   // * vs []
extern char error_fsize[];
extern char error_keysize[];
extern char bytes_deciphered[];
extern char bytes_encrypted[];

extern unsigned char hedit_exe;
extern int i, j, bytesrd;
extern unsigned long ln, li, ls;;
extern UINT length;
extern long int srcFileSize;
extern UCHAR mode;

extern char icvblock[];
extern char inblock[], outblock[], lastblock[2*BLOCK_SIZE];

extern streampos pos;         // for seek test only
extern ofstream outfile;  

//----------------------------------------------------------------------------
//
//                          Global declarations
//
char TdesSignon[] = "TDES Crypto Utility, V1.00 (c)2021 by ha\n";

char tdes_keybuf[KEY_LENGTH] = {
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD, \
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD, \
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD
  }; // Provide space for key size of 192bits

char inbuf1[BLOCK_SIZE], inbuf2[BLOCK_SIZE];
char outbuf1[BLOCK_SIZE];

char tdes_keybuf1[KEY_LENGTH/3] = {
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD
  }; // Provide space for key size of 64bits
char tdes_keybuf2[KEY_LENGTH/3] = {
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD
  }; // Provide space for key size of 64bits
char tdes_keybuf3[KEY_LENGTH/3] = {
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD
  }; // Provide space for key size of 64bits

int padbytes, stepnr;
ULONG lj;

//-----------------------------------------------------------------------------
//
//                              TdesDoAlgorithmStealECB
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ciphertext stealing)
//
void TdesDoAlgorithmStealECB(ifstream &infile, ofstream &outfile, int _keymode)
  {
  int ciphStealing = FALSE;

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  do
    {
    if (ciphStealing == TRUE) break; // ciphStealing

    for (stepnr=1; stepnr<=3; stepnr++)
      {
      if (stepnr == 1)                                     // 1st step
        {
        if ((lj - ln) >= BLOCK_SIZE) bytesrd = BLOCK_SIZE; // Keep track of bytesrd,    
        else bytesrd = lj - ln;                            //  ifstream won't tell us   
        infile.read(inblock, bytesrd);

        // CIPHERTEXT STEALING:
        // We do not want to change filesizes, so we dont use padding.
        // The following special handling of the last block implements
        // "Ciphertext Stealing" if the last block is less than BLOCK_SIZE.
        // For the last 2 blocks: Des(Pn-1) = Cn||C' and Des(Pn||C') = Cn-1
        //  Note: Pn-1 = Plaintext of BLOCK_SIZE
        //        Pn = Last plaintext < BLOCK_SIZE
        //        C' = Ciphertext padded to Pn, stolen from previous block
        //        Cn-1 = New Ciphertext of BLOCK_SIZE for previous block
        //        Cn = Ciphertext < BLOCKSIZE from previous block, used last.
        //
        // Example: Encrypt Key = 12345678
        //          Pn-1 = 0A 0D 0A 0D 0A 0D 0A 0D   (EF 29 7C 97 61 5B 80 9E)
        //          Pn   = 0A 0D 0A 0D 0A
        //          C'   = 5B 80 9E
        //          Cn-1 = 91 03 D1 32 FA 54 C2 17
        //          Cn   = EF 29 7C 97 61
        //
        //  before: lastblock[] = 00 00 00 00 00 00 00 00 EF 29 7C 97 61 00 00 00
        //          inblock[]   = 0A 0D 0A 0D 0A 5B 80 9E
        //
        //  after:  lastblock[] = 91 03 D1 32 FA 54 C2 17 EF 29 7C 97 61
        //
        if (bytesrd < BLOCK_SIZE)
          {
          for (i = 0; i < bytesrd; i++) lastblock[BLOCK_SIZE + i] = outblock[i];
          for (i = bytesrd; i < BLOCK_SIZE; i++) inblock[i] = outblock[i];
          ciphStealing = TRUE;
          }
        } // end if(stepnr==1)

//ha//else if (stepnr == 2) kinit(tdes_keybuf2, _keymode^0x01);   // 2nd step

      else if (stepnr == 3)                                       // 3rd step
        {
        ln += (ULONG)bytesrd;
        } // end else if(stepnr==3)
      
      if (ciphStealing == TRUE)
        {
        desAlgorithm2(inblock, lastblock, stepnr);
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = lastblock[i];                                                  
        }
      else
        {
        desAlgorithm2(inblock, outblock, stepnr);                 // Perform the DES Algorithm
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];                                                   
        }

      if (stepnr == 3)
        {
        if (ciphStealing == TRUE)
          {
          outfile.seekp(0, ios::end);           // seek to the end of the file
          outfile.seekp(-BLOCK_SIZE, ios::cur); // back up 8 bytes
          outfile.write(lastblock, bytesrd + BLOCK_SIZE);
          }
        else outfile.write(outblock, bytesrd);
        }
      } // end for(stepnr)

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }                        
    } // end while
  while (ln < lj);
  } // TdesDoAlgorithmStealECB

//ha//DebugStop("bytesrd:",bytesrd);
//DebugStop("stepnr:", stepnr);
//DebugStop("while:", (int)ln);
//DebugPrintBuffer(inblock, 8);


//-----------------------------------------------------------------------------
//
//                              TdesDoAlgorithmIsoECB
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ISO Padding)
//
void TdesDoAlgorithmIsoECB(ifstream &infile, ofstream &outfile, int _keymode)
  {
  int isoPad = 0; j = 0;

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  do
    {
    if (isoPad == ISOPAD) break;

    for (stepnr=1; stepnr<=3; stepnr++)
      {
      if (stepnr == 1)                                     // 1st step
        {
        if ((lj - ln) >= BLOCK_SIZE) bytesrd = BLOCK_SIZE; // Keep track of bytesrd,    
        else bytesrd = lj - ln;                            //  ifstream won't tell us   
        infile.read(inblock, bytesrd);

        if (bytesrd == 0 && mode == TDES_ECBE)           
          {                                                // Filesize = N * BLOCK_SIZE:
          inblock[0]=ISOPAD;                               // Need to add a whole padding block
          for (i=1; i<BLOCK_SIZE; i++) inblock[i] = PAD;   // Does not apply to encrypted text
          padbytes = BLOCK_SIZE; bytesrd = BLOCK_SIZE;     //  which is always padded MOD(8)
          isoPad = ISOPAD;
          }
        else

        // ISO PADDING:
        // Using ISO padding we always increase the filesize.
        // The following handling of the last block implements ISO Padding.
        // For the last block: Des(Pn) = Cn||PB
        //  Note: Pn = Plaintext of <=BLOCK_SIZE
        //        Cn = Ciphertext > Pn, padded to BLOCKSIZE or appended with BLOCKSIZE.
        //
        // Example1: Encrypt Key = 12345678
        //           Pn      = 0A 0D 0A 0D 0A
        //           Pn||PB  = 0A 0D 0A 0D 0A 80 00 00
        //           Cn      = xx xx xx xx xx xx xx xx
        //
        // Example2: Encrypt Key = 12345678
        //           Pn      = 0A 0D 0A 0D 0A 12 34 45
        //           Pn||PB  = 0A 0D 0A 0D 0A 12 34 45 80 00 00 00 00 00 00 00    
        //           Cn      = xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
        //
        if (bytesrd < BLOCK_SIZE && mode == TDES_ECBE)
          {
          inblock[bytesrd] = ISOPAD;
          for (i=bytesrd+1; i<BLOCK_SIZE; i++) inblock[i] = PAD;
          padbytes = bytesrd; bytesrd = BLOCK_SIZE;        // Padded up to BLOCK_SIZE
          isoPad = ISOPAD;                                 // Stops when processing is complete
          }
        } // end if(stepnr==1)

//ha//else if (stepnr == 2) kinit(tdes_keybuf2, _keymode^0x01); // 2nd step

      else if (stepnr == 3)                                // 3rd step
        {
        ln += (ULONG)bytesrd;
        } // end else if

      desAlgorithm2(inblock, outblock, stepnr);               // Perform the DES Algorithm
      for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];  // Provide inblock for next stepnr                                                   

      if (stepnr == 3)  
        {
        if ((mode == TDES_ECBD) && (ln == lj)) // Last block requires special handling
          {                                    // Either a whole block of padding
          j=BLOCK_SIZE;                        //  or a partly padded block
          for (i=0; i<BLOCK_SIZE; i++)         //  is expected.
            {
            j--;
            if ((UCHAR)outblock[j] == ISOPAD)  // Check for ISO-PADDING
              {
              isoPad = ISOPAD;                 // ISOPAD found.
              break;                           // ISOPAD - break
              }
            } // end for

          if (isoPad != ISOPAD) j = bytesrd;   // ISOPAD not found - output the block
          outfile.write(outblock, j);          // Don't output ISOPAD if found 
          ln += ((ULONG)j - BLOCK_SIZE);       // Adjust (decipher) filesize count
          } //end if

        else outfile.write(outblock, bytesrd); // mode == TDES_ECBE
        } // end if(stepnr==3)
      } // end for(stepnr)

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end do while
  while (ln < lj);
  
  if ((mode == TDES_ECBE) && (isoPad == 0))  // Need to add a whole padding block
    {                                       // Does not apply to encrypted text
    inblock[0]=ISOPAD;                     //  which is always padded MOD(8)
    for (i=1; i<BLOCK_SIZE; i++) inblock[i] = PAD;

    desAlgorithm2(inblock, outblock, 1);                      // 1st step
    for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];

    desAlgorithm2(inblock, outblock, 2);                      // 2nd step
    for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];

    desAlgorithm2(inblock, outblock, 3);                      // 3rd step

    outfile.write(outblock, BLOCK_SIZE);
    ln += BLOCK_SIZE;                      // Adjust (decipher) filesize count
    }
  } // TdesDoAlgorithmIsoECB

//ha//DebugStop("bytesrd:",bytesrd);
//ha//DebugStop("isoPad:", isoPad);
//ha//DebugStop("j:", j);
//ha//DebugPrintBuffer(inblock, 8);


//------------------------------------------------------------------------------
//
//                          TdesDoAlgorithmStealCBCE
//
//  ENCRYPT - Cipher Block Chaining (Ciphertext Stealing)
//
void TdesDoAlgorithmStealCBCE(ifstream &infile, ofstream &outfile)
  {
  int ciphStealing = FALSE;

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  do
    {
    if (ciphStealing == TRUE) break;                   // ciphStealing -break

    if ((lj - ln) >= BLOCK_SIZE) bytesrd = BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = lj - ln;                            //  ifstream won't tell us   
    infile.read(inblock, bytesrd);                     // Read from input file

    if (bytesrd >= BLOCK_SIZE)
      for (i = 0; i < BLOCK_SIZE; i++) inblock[i] ^= icvblock[i]; // CBC: inbuf XOR ICV

    else if (bytesrd < BLOCK_SIZE)                                // Ciphertext stealing
      {
      for (i = 0; i < BLOCK_SIZE-bytesrd; i++) inblock[i+bytesrd] = PAD; // Pn* = Pn||0s (zero-padded) 
      for (i = 0; i < BLOCK_SIZE; i++) inblock[i] ^= outblock[i];        // Pn* XOR Cn-1*
      for (i = 0; i < BLOCK_SIZE; i++) outbuf1[i] = outblock[i];         // save Cn-1

      // CIPHERTEXT STEALING CBC ENCRYPT:
      // We do not want to change filesizes, so we dont use padding.
      // The following special handling of the last block implements
      // "Ciphertext Stealing" if the last block is less than BLOCK_SIZE.
      // For the last 2 blocks: Des(Pn-1) = Cn||C' and Des(Pn||C') = Cn-1
      //  Note: Pn-1      = Previous Plaintext of BLOCK_SIZE
      //        Pn        = Last plaintext < BLOCK_SIZE
      //        Pn*       = Last plaintext padded with zeros
      //        Cn-1      = Previous Ciphertext of Pn-1
      //        Cn-1*     = Ciphertext padded to Pn, stolen from previous block
      //        Cn-1(new) = New Ciphertext of BLOCK_SIZE for previous block
      //        Cn-1**    = Ciphertext < BLOCKSIZE from previous block, used last.
      //
      // Example: Encrypt Key = 12345678
      //          Pn-1   = 0A 0D 0A 0D 0A 0D 0A 0D
      //          Cn-1   = F5 AC DD BE 5F 21 C0 2B
      //          Pn     = 0A 0D 0A 0D 0A
      //          Cn-1*  = 21 C0 2B
      //          Cn-1** = F5 AC DD BE 5F
      //
      // Steps:   Pn* = Pn||0s = 0A 0D 0A 0D 0A 00 00 00  (zero-padded)
      //          Pn* ^ Cn-1   = FF A1 D7 B3 55 21 C0 2B  (inblock to be encrypted
      //          Cn-1(new)    = 72 F6 82 BA DA D8 88 91    yields a new previous Block)
      //          Cn-1**       = F5 AC DD BE 5F           (lastblock < BLOCK_SIZE)
      //
      //  before: lastblock[] = 00 00 00 00 00 00 00 00 F5 AC DD BE 5F 00 00 00
      //          inblock[]   = FF A1 D7 B3 55 21 C0 2B
      //
      //  after:  lastblock[] = 72 F6 82 BA DA D8 88 91 F5 AC DD BE 5F [Cn-1(new) || Cn-1**]
      //
      ciphStealing = TRUE;
      } // end else if

    // ---------------------------------------------
    // Performing the DES (i.e., Standard Algorithm)
    // ---------------------------------------------
    desAlgorithm2(inblock, outblock, 1);                                    // 1st step
    if (ciphStealing == TRUE)
      {
      // Build the last block(s) [Cn-1(new) || Cn-1**],
      //  where Cn-1(new) consists of the encrypted incomplete block of Pn
      //  and the stolen chunk Cn-1* which has been encrypted twice.
      //
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] = outblock[i];          // Cn-1(new)
      for (i = 0; i < bytesrd; i++) lastblock[BLOCK_SIZE + i] = outbuf1[i]; // Cn-1**
      }

    for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];                                                   
    desAlgorithm2(inblock, outblock, 2);                                    // 2nd step
    if (ciphStealing == TRUE)
      {
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] = outblock[i];          
      for (i = 0; i < bytesrd; i++) lastblock[BLOCK_SIZE + i] = outbuf1[i]; 
      }

    for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];                                                   
    desAlgorithm2(inblock, outblock, 3);                                    // 3rd step
    if (ciphStealing == TRUE)
      {
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] = outblock[i];          // Cn-1(new)
      for (i = 0; i < bytesrd; i++) lastblock[BLOCK_SIZE + i] = outbuf1[i]; // Cn-1**
      }

    ln += (ULONG)bytesrd;                            // Update counter total bytes read

    // ------------------------------------------------
    // Special processing after applying the TDES steps
    // ------------------------------------------------
    if (ciphStealing == FALSE)
      {
      outfile.write(outblock, bytesrd);                       // Write Ci..Cn
      for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = outblock[i]; // Update ICV
      }
    else if (ciphStealing == TRUE)                     
      {
      outfile.seekp(0, ios::end);                     // seek to end of the file
      outfile.seekp(-BLOCK_SIZE, ios::cur);           // back up 8 bytes
      outfile.write(lastblock, bytesrd + BLOCK_SIZE); // Write [Cn-1 || Cn]
      break;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }                        
    } // end do while
  while (ln < lj);
  } // TdesDoAlgorithmStealCBCE

//ha//DebugStop("bytesrd:",bytesrd);
//DebugStop("stepnr:", stepnr);
//DebugStop("while:", (int)ln);
//ha//DebugPrintBuffer(inblock, bytesrd);


//------------------------------------------------------------------------------
//
//                        TdesDoAlgorithmStealCBCD
//
//  DECIPHER - Cipher Block Chaining (Ciphertext Stealing)
//
void TdesDoAlgorithmStealCBCD(ifstream &infile, ofstream &outfile)
  {
  int ciphStealing = FALSE;

  for (i=0; i<BLOCK_SIZE; i++) inbuf2[i] = 0;   // IMPORTANT: Init-clear Cn-2 block

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  do
    {
    if (ciphStealing == TRUE) break;                   // ciphStealing - break

    if ((lj - ln) >= BLOCK_SIZE) bytesrd = BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = lj - ln;                            //  ifstream won't tell us   
    infile.read(inblock, bytesrd);                     // Read from input file

    if ((lj-ln) > 2*BLOCK_SIZE && (lj % BLOCK_SIZE) != 0) 
      for (i=0; i<BLOCK_SIZE; i++) inbuf2[i] = inblock[i]; // CBC save Cn-2 block

    if (bytesrd == BLOCK_SIZE)
      for (i=0; i<BLOCK_SIZE; i++) inbuf1[i] = inblock[i]; // CBC save 1st block

    else if (bytesrd < BLOCK_SIZE)
      {
      // CIPHERTEXT STEALING CBC DECIPHER:
      // We do not want to change filesizes, so we dont use padding.
      // The following special handling of the last block implements
      // "Ciphertext Stealing" if the last block is less than BLOCK_SIZE.
      // For the last 2 blocks: Des(Pn-1) = Cn||C' and Des(Pn||C') = Cn-1
      //  Note: Pn-1      = Previous Plaintext of BLOCK_SIZE
      //        Pn        = Last plaintext < BLOCK_SIZE
      //        Pn*       = Last plaintext padded with zeros
      //        Cn-1      = Previous Ciphertext of Pn-1
      //        Cn-1*     = Ciphertext padded to Pn, stolen from previous block
      //        Cn-1(new) = New Ciphertext of BLOCK_SIZE for previous block
      //        Cn-1**    = Ciphertext < BLOCKSIZE from previous block, used last.
      //
      // Example: Decipher Key = 12345678
      //          Cn-1(new) = 72 F6 82 BA DA D8 88 91  (Previous block)
      //          Cn-1*     = 21 C0 2B
      //          Cn-1**    = F5 AC DD BE 5F           (Last block)
      //
      // Steps:   Cn-1(new)                = 72 F6 82 BA DA D8 88 91  (partly encrypted twice)
      //          Pn* ^ Cn-1               = FF A1 D7 B3 55 21 C0 2B  (deciphered once)
      //          Cn-1 = Cn-1** || Cn-1*   = F5 AC DD BE 5F 21 C0 2B
      //          Must save Cn-1           =[F5 AC DD BE 5F 21 C0 2B]
      //          Pn-1                     = 0A 0D 0A 0D 0A 0D 0A 0D  (Pn-1 deciphered)
      //          Pn = (Pn* ^ Cn-1) ^ Cn-1 = 0A 0D 0A 0D 0A           (Pn lastblock deciphered)
      //
      //  before: lastblock[] = F5 AC DD BE 5F 21 C0 2B F5 AC DD BE 5F 21 C0 2B
      //  after:  outblock[]  = 0A 0D 0A 0D 0A 0D 0A 0D                          (Decipher)
      //
      //  before: lastblock[] = FF A1 D7 B3 55 21 C0 2B F5 AC DD BE 5F 21 C0 2B
      //  after:  lastblock[] = 0A 0D 0A 0D 0A 00 00 00 F5 AC DD BE 5F 21 C0 2B  (XOR)
      //
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] = lastblock[BLOCK_SIZE + i];
      for (i = 0; i < bytesrd; i++)    lastblock[i] = inblock[i];

      desAlgorithm2(lastblock, outblock, 1);                      // 1st step
      for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];

      desAlgorithm2(inblock, outblock, 2);                        // 2nd step
      for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];

      desAlgorithm2(inblock, outblock, 3);                        // 3rd step

      for (i = 0; i < BLOCK_SIZE; i++) outblock[i]  ^= inbuf2[i];
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] ^= lastblock[BLOCK_SIZE + i];

      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i+BLOCK_SIZE] = lastblock[i];  // swap Pn-1
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] = outblock[i];              // concatenate Pn chunk

      ciphStealing = TRUE;
      } // end else if                                                                

    // ---------------------------------------------
    // Performing the DES (i.e., Standard Algorithm)
    // ---------------------------------------------
    if (ciphStealing == FALSE)
      {
      desAlgorithm2(inblock, outblock, 1);             // 1st step
      if (bytesrd == BLOCK_SIZE && ciphStealing == FALSE)
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];
      }

    if (ciphStealing == FALSE)
      {
      desAlgorithm2(inblock, outblock, 2);             // 2nd step
      if (bytesrd == BLOCK_SIZE && ciphStealing == FALSE)
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];
      }

    if (ciphStealing == FALSE)
      {
      desAlgorithm2(inblock, outblock, 3);             // 3rd step
      if (bytesrd == BLOCK_SIZE && ciphStealing == FALSE)
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];
      }

    ln += (ULONG)bytesrd;                             // Update counter total bytes read
    
    // ------------------------------------------------
    // Special processing after applying the TDES steps
    // ------------------------------------------------
    if (ciphStealing == FALSE)
      {
      for (i=0; i<BLOCK_SIZE; i++) lastblock[BLOCK_SIZE+i] = outblock[i];  // Save Cn-1
      for (i=0; i<BLOCK_SIZE; i++) outblock[i] ^= icvblock[i]; // CBC specific XOR function
      for (i=0; i<BLOCK_SIZE; i++) icvblock[i]  = inbuf1[i];   // CBC copy 1st block
      outfile.write(outblock, bytesrd);
      }
    else
      {
      outfile.seekp(0, ios::end);                     // seek to end of the file
      outfile.seekp(-BLOCK_SIZE, ios::cur);           // back up 8 bytes
      outfile.write(lastblock, bytesrd + BLOCK_SIZE); // Write [Pn-1 || Pn]
      break;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }                        
    } // end do while
  while (ln < lj);
  } // TdesDoAlgorithmStealCBCD

//ha//DebugStop("bytesrd:",bytesrd);
//DebugStop("while:", (int)ln);                         
//ha//DebugPrintBuffer(inblock, bytesrd);
//DebugStop("Last write deciphered:", bytesrd + BLOCK_SIZE);      
//DebugPrintBuffer(lastblock, bytesrd + BLOCK_SIZE);


//------------------------------------------------------------------------------
//
//                              TdesDoAlgorithmIsoCBCE
//
//  ENCRYPT - Cipher Block Chaining (ISO Padding)
//
void TdesDoAlgorithmIsoCBCE(ifstream &infile, ofstream &outfile)
  {
  int isoPad = 0; j = 0;

  li = COUNT_RATE; ln = 0; lj = srcFileSize;

  if (mode == TDES_CBCE) lj += BLOCK_SIZE;
  while (ln < lj)
    {
    bytesrd = BLOCK_SIZE; // init bytesrd, filesize is at least BLOCK_SIZE
    if (isoPad == ISOPAD) break;

    for (stepnr=1; stepnr<=3; stepnr++)
      {
      if (stepnr == 1)                                          // 1st step
        {
        if (((lj-ln) == BLOCK_SIZE) && (mode == TDES_CBCE)) // Need to add a whole padding block
          {                                                // Does not apply to encrypted text
          inblock[0]=ISOPAD;                              //  which is always padded MOD(8)
          for (i=1; i<BLOCK_SIZE; i++) inblock[i] = PAD;
          }
        else
          infile.read(inblock, bytesrd);

        // ISO PADDING:
        // Using ISO padding we always increase the filesize.
        // The following handling of the last block implements ISO Padding.
        // For the last block: Des(Pn) = Cn||PB
        //  Note: Pn = Plaintext of <=BLOCK_SIZE
        //        Cn = Ciphertext > Pn, padded to BLOCKSIZE or appended with BLOCKSIZE.
        //
        // Example1: Encrypt Key = 12345678
        //           Pn      = 0A 0D 0A 0D 0A
        //           Pn||PB  = 0A 0D 0A 0D 0A 80 00 00
        //           Cn      = xx xx xx xx xx xx xx xx
        //                                                 
        // Example2: Encrypt Key = 12345678
        //           Pn      = 0A 0D 0A 0D 0A 12 34 45
        //           Pn||PB  = 0A 0D 0A 0D 0A 12 34 45 80 00 00 00 00 00 00 00    
        //           Cn      = xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
        //
        if ((lj-ln-BLOCK_SIZE) < BLOCK_SIZE && mode == TDES_CBCE)
          {
          bytesrd = (int)(lj-ln-BLOCK_SIZE);
          inblock[bytesrd] = ISOPAD;
          for (i=bytesrd+1; i<BLOCK_SIZE; i++) inblock[i] = PAD;
          bytesrd = BLOCK_SIZE;
          isoPad = ISOPAD;
          }
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] ^= icvblock[i]; // CBC; inbuf XOR ICV
        } // end if(stepnr==1)

      else if (stepnr == 2)
        {
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];  // 2nd step                                              
        }

      else if (stepnr == 3)                                     
        {
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];  // 3rd step                                                
        ln += (ULONG)bytesrd;
        } // end else if(stepnr==3)
      
        desAlgorithm2(inblock, outblock, stepnr);               // Perform the DES Algorithm
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];  // Provide inblock for next stepnr                                                   

      if (stepnr == 3)
        {
        outfile.write(outblock, bytesrd);
        for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = outblock[i]; // Update ICV
        }
      } // end for(stepnr)

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }                        
    } // end while
  } // TdesDoAlgorithmIsoCBCE

//ha//DebugStop("bytesrd:",bytesrd);
//DebugStop("stepnr:", stepnr);
//DebugStop("while:", (int)ln);
//DebugPrintBuffer(inblock, 8);


//------------------------------------------------------------------------------
//
//                         TdesDoAlgorithmIsoCBCD
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ISO Padding)
//
void TdesDoAlgorithmIsoCBCD(ifstream &infile, ofstream &outfile)
  {
  int isoPad = 0; j = 0;

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  
  while (ln < lj)
    {
    bytesrd = BLOCK_SIZE;   // Init bytesrd, filesize is at least BLOCK_SIZE
    if (isoPad == ISOPAD) break;

    for (stepnr=1; stepnr<=3; stepnr++)
      {
      if (stepnr == 1)                                           // 1st step
        {
        infile.read(inblock, bytesrd);
        if (bytesrd == BLOCK_SIZE)
          for (i=0; i<BLOCK_SIZE; i++) inbuf1[i] = inblock[i];   // CBC save 1st block
        } // end if(stepnr==1)

      else if (stepnr == 2)                                      // 2nd step
        {
        if (bytesrd == BLOCK_SIZE)
          for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];                                                   
        } // end else if(stepnr==2)

      else if (stepnr == 3)                                      // 3rd step
        {
        if (bytesrd == BLOCK_SIZE)
          for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];
        ln += (ULONG)bytesrd;
        } // end else if(stepnr==3)

      desAlgorithm2(inblock, outblock, stepnr);                  // Perform the DES Algorithm

      if (stepnr == 3)  
        {
        for (i=0; i<BLOCK_SIZE; i++) outblock[i] ^= icvblock[i]; // CBC specific XOR function
        for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = inbuf1[i];    // CBC copy 1st block

        // ISO PADDING:
        // Using ISO padding we always increase the filesize.
        // The following handling of the last block implements ISO Padding.
        // For the last block: Des(Pn) = Cn||PB
        //  Note: Pn = Plaintext of <=BLOCK_SIZE
        //        Cn = Ciphertext > Pn, padded to BLOCKSIZE or appended with BLOCKSIZE.
        //
        // Example1: Encrypt Key = 12345678
        //           Pn      = 0A 0D 0A 0D 0A
        //           Pn||PB  = 0A 0D 0A 0D 0A 80 00 00
        //           Cn      = xx xx xx xx xx xx xx xx
        //
        // Example2: Encrypt Key = 12345678
        //           Pn      = 0A 0D 0A 0D 0A 12 34 45
        //           Pn||PB  = 0A 0D 0A 0D 0A 12 34 45 80 00 00 00 00 00 00 00    
        //           Cn      = xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx
        //
        if ((mode == TDES_CBCD) && (ln == lj)) // Last block requires special handling
          {                                    // Either a whole block of padding
          j=BLOCK_SIZE;                        //  or a partly padded block
          for (i=0; i<BLOCK_SIZE; i++)
            {
            j--;
            if ((UCHAR)outblock[j] == ISOPAD)
              {
              isoPad = ISOPAD;                 // ISOPAD found.
              break;                           // ISOPAD - break
              }
            } // end for
          if (isoPad != ISOPAD) j = bytesrd;   // ISOPAD not found - output the block
          outfile.write(outblock, j);          // Don't output ISOPAD if found 
          } //end if

        else
          outfile.write(outblock, bytesrd);    // Write the whole Block
        } // end if(stepnr==3)
      } // end for(stepnr)

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);          // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end while
  
  if (mode == TDES_CBCD) ln -= bytesrd, ln += (ULONG)j;  // Adjust filesize count
  } // TdesDoAlgorithmIsoCBCD

//----------------------------------------------------------------------------
//
//                                TdesDoAlgorithmMac
//
// First step:  Two subkeys K1, K2 are generated from the key K.
// Second step: The input message is formatted into a sequence of complete blocks
//              in which the final block has been masked by a subkey.
// 
// There are two cases:
// If the message length is a positive multiple of the block size,
//  then the message is partitioned into complete blocks.
//  The final block is masked with the first subkey; in other words,
//  the final block in the partition is replaced
//  with the exclusive-OR of the final block with the FIRST subkey K1.
//  The resulting sequence of blocks is the formatted message
//  (no additional ISO Padding is applied).
// 
// If the message length is not a positive multiple of the block size,
//  then the message is partitioned into complete blocks
//  to the greatest extent possible, i.e., into a sequence of complete blocks
//  followed by a final bit string whose length is less than the block size.
//  A padding string is appended to this final bit string,
//  in particular, a single ‘1’ bit followed by the minimum number of ‘0’ bits,
//  possibly none, that are necessary to form a complete block (= ISO Padding).
//  The complete final block is masked, with the SECOND subkey K2.
//  The resulting sequence of blocks is the formatted message.
//
#define TDES_BLOCK_SIZE 8
 
void TdesDoAlgorithmMac(ifstream &infile, ofstream &outfile)
  {
  int msbFlag;

  char pszRB64[TDES_BLOCK_SIZE] = {0,0,0,0,0,0,0,0x1B};
  char pszZeroBlock[TDES_BLOCK_SIZE];
  char pszSubkeyK1[TDES_BLOCK_SIZE+1];  // Adding one 0-byte for ROL operation
  char pszSubkeyK2[TDES_BLOCK_SIZE];

  // Generating the subkeys K1 and K2
  //  
  //  Example 1:
  //  Block cipher is the TDES algorithm (Three Key TDEA):
  //    Key1 =       8aa83bf8 cbda1062
  //    Key2 =       0bc1bf19 fbb6cd58
  //    Key3 =       bc313d4a 371ca8b5  
  //
  //  Subkey K1, K2 Generation
  //    CIPHK(064) = C8 CC 74 E9 8A 73 29 A2  ok
  //    K1 =         91 98 E9 D3 14 E6 53 5F  ok
  //    K2 =         23 31 D3 A6 29 CC A6 A5  ok
  //
  // Example Mlen = 64:
  //    M =          6bc1bee2 2e409f96
  //    T =          b7a688e1 22ffaf95   ????  8E 8F 29 31 36 28 37 97
  //
  // Example Mlen = 160:
  //    M =          6bc1bee2 2e409f96 e93d7e11 7393172a
  //                 ae2d8a57
  //    T =          d32bcebe 43d23d80   ????  74 3D DB E0 CE 2D C2 ED
  //
  // Example Mlen = 256:
  //    M =          6bc1bee2 2e409f96 e93d7e11 7393172a
  //                 ae2d8a57 1e03ac9c 9eb76fac 45af8e51
  //    T =          33e6b109 2400eae5   ????  33 E6 B1 09 24 00 EA E5
  //

  //  Example 2:
  //  Block cipher is the TDES algorithm (Two Key TDEA):
  //    Key1 =       4cf15134 a2850dd5
  //    Key2 =       8a3d10ba 80570d38
  //    Key3 =       4cf15134 a2850dd5  
  //
  //  Subkey K1, K2 Generation
  //    CIPHK(064) = C7 67 9B 9F 6B 8D 7D 7A  ok
  //    K1 =         8E CF 37 3E D7 1A FA EF  ok
  //    K2 =         1D 9E 6E 7D AE 35 F5 C5  ok
  //
  // Example Mlen = 64:
  //    M =          6bc1bee2 2e409f96
  //    T =          bd2ebf9a 3ba00361   ????  4F F2 AB 81 3C 53 CE 83
  //
  // Example Mlen = 160:
  //    M =          6bc1bee2 2e409f96 e93d7e11 7393172a
  //                 ae2d8a57
  //    T =          8ea92435 b52660e0   ????  62 DD 1B 47 19 02 BD 4E
  //
  // Example Mlen = 256:
  //    M =          6bc1bee2 2e409f96 e93d7e11 7393172a
  //                 ae2d8a57 1e03ac9c 9eb76fac 45af8e51
  //    T =          31 B1 E4 31 DA BC 4E B8  ok
  
  pszSubkeyK1[TDES_BLOCK_SIZE+0] = 0;    // Ensure K1 last shifted bit = 0

  for (i=0; i<TDES_BLOCK_SIZE; i++)
    pszZeroBlock[i] = icvblock[i];   // Init Zero - block

  // K1
  desAlgorithm2(pszZeroBlock, pszSubkeyK1, 1);             // 1st step
  for (i=0; i<TDES_BLOCK_SIZE; i++) pszZeroBlock[i] = pszSubkeyK1[i];
  desAlgorithm2(pszZeroBlock, pszSubkeyK1, 2);             // 2nd step
  for (i=0; i<TDES_BLOCK_SIZE; i++) pszZeroBlock[i] = pszSubkeyK1[i];
  desAlgorithm2(pszZeroBlock, pszSubkeyK1, 3);             // 3rd step
  
  msbFlag = (UCHAR)pszSubkeyK1[0] & 0x80; // Set flag for XOR K1 later
  
  for (i=0; i<TDES_BLOCK_SIZE; i++)
    pszSubkeyK1[i] = (UCHAR)(pszSubkeyK1[i] << 1) | ((UCHAR)(pszSubkeyK1[i+1] & 0x80) >> 7);
                                                                                  
  if (msbFlag != 0)
    {
    for (i=0; i<TDES_BLOCK_SIZE; i++) pszSubkeyK1[i] ^= pszRB64[i];
    }

  // K2
  for (i=0; i<TDES_BLOCK_SIZE; i++)
    pszSubkeyK2[i] = (UCHAR)(pszSubkeyK1[i] << 1) | (UCHAR)((pszSubkeyK1[i+1] & 0x80) >> 7);

  msbFlag = (UCHAR)pszSubkeyK1[0] & 0x80; // Set flag for XOR K1 later
  if (msbFlag != 0)
    {
    for (i=0; i<TDES_BLOCK_SIZE; i++) pszSubkeyK2[i] ^= pszRB64[i];
    }

  DWORD dwCryptFileSize = srcFileSize;

  // CMAC 
  ln = 0; ls = dwCryptFileSize;  // init counters
  bytesrd = TDES_BLOCK_SIZE;     // init bytesrd, filesize is at least 8 bytes
  while (ln < ls)
    {
    if ((ls - ln) >= TDES_BLOCK_SIZE) bytesrd = TDES_BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = ls - ln;                                      //  ifstream won't tell us   

    infile.read(inblock, bytesrd);

    // Prepare the last block of TDES_BLOCK_SIZE in inblock:
    // K1) Message length is a positive multiple of the block size
    if ((ls - ln) == TDES_BLOCK_SIZE)
      {
      for (i=0; i<TDES_BLOCK_SIZE; i++) inblock[i] ^= pszSubkeyK1[i]; // Apply K1
      }
  
    // K2) Message length is not a positive multiple of the block size  
    else if ((ls - ln) < TDES_BLOCK_SIZE)
      {
      for (i=(int)(dwCryptFileSize % TDES_BLOCK_SIZE); i<TDES_BLOCK_SIZE; i++)
        {
        // CMAC Padding starts with 10000000b and continues with all bits zeroed
        if (i == (int)(dwCryptFileSize % TDES_BLOCK_SIZE)) inblock[i] = ISOPAD;      
        else inblock[i] = PAD;
        }
      for (i=0; i<TDES_BLOCK_SIZE; i++) inblock[i] ^= pszSubkeyK2[i]; // Apply K2
      }

    for (i=0; i<TDES_BLOCK_SIZE; i++) inblock[i] ^= icvblock[i]; // CBC: inbuf XOR ICV
    desAlgorithm2(inblock, outblock, 1);             // 1st step
    for (i=0; i<TDES_BLOCK_SIZE; i++) inblock[i] = outblock[i];  // Update next block
    desAlgorithm2(inblock, outblock, 2);             // 2nd step
    for (i=0; i<TDES_BLOCK_SIZE; i++) inblock[i] = outblock[i];  // Update next block
    desAlgorithm2(inblock, outblock, 3);             // 3rd step
    for (i=0; i<TDES_BLOCK_SIZE; i++) icvblock[i] = outblock[i]; // Update ICV w/ next block

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);       // Echo per COUNT_RATE
      li += COUNT_RATE;
      }

    ln += (ULONG)bytesrd;                   // Update counter total bytes read
    } // end while

  outfile.write(outblock, TDES_BLOCK_SIZE); // Emit the MAC to file
  } // TdesDoAlgorithmMac


//----------------------------------------------------------------------------
//
//                                TdesDisplayIcvblock
//
void TdesDisplayIcvblock()
  {
  printf("iv-block: ");
  for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)icvblock[i]);
  printf("\n");
  } // TdesDisplayIcvblock


//----------------------------------------------------------------------------
//
//                                TdesClearScreen
//
void TdesClearScreen()
  {
  system("cls");
  } //TdesClearScreen


//----------------------------------------------------------------------------
//
//                                 TdesAnyKey
//
void TdesAnyKey()
  {
  // This console program could be run by typing its name at the command prompt,
  // or it could be run by the user double-clicking it from Explorer.
  // And you want to know which case you’re in.

  // Check if invoked via Desktop
  DWORD procId = GetCurrentProcessId();
  DWORD count = GetConsoleProcessList(&procId, 1);
  if (count < 2)
    {
    printf("\nConsole application: TDES.EXE\n");
    system("cmd");                   // Keep the Console window
    exit(0);
    }

  // Invoked via Console
while (_kbhit() != 0) _getch();   // Flush key-buffer 
  printf("-- press any key --\n");
  _getch();
  } // TdesAnyKey


//----------------------------------------------------------------------------
//
//                                 TdesMore
//
void TdesMore()
  {
  while (_kbhit() != 0) _getch();   // flush key-buffer 
  printf("-- More --\r");
  _getch();
  TdesClearScreen();
  } // TdesMore


//-----------------------------------------------------------------------------
//
//                              TdesDisplayHelp
//
void TdesDisplayHelp()
  {
  printf(TdesSignon);                   // Display signon message

  printf("Encryption and decryption using the Triple DES Algorithm.\n\n");
  printf("Usage: '%s srcfile destfile [keyfile | /keystring] [options] [ivfile]",
                  (hedit_exe == FALSE) ? "TDES" : "HEDIT");
  printf("%s'\n", (hedit_exe == FALSE) ? "" : " /TDES");                                 
  printf("  srcfile    Input file (plain text or encrypted text >= 8 bytes).\n"
         "  destfile   Output file (after the algorithm has been applied).\n"
         "  ivfile     Input iv-file (Init Vector, optional for CBC modes).\n\n");

  printf("  keyfile    Input file containing the secret key.\n"
         "             The effective key length is 168 bits, i.e., parity bits\n"
         "             of the 'key' are ignored. Short keys will be zero-expanded.\n"
         "  /keystring To avoid a keyfile the key may be directly given\n"
         "             as a string of up to 24 ascii characters: e.g. /12345678...\n");
  printf("[options]\n");
  printf("  /ENCRYPT   Encrypts a file. The plaintext is TDES encrypted.\n"
         "             Mode: CBC with ciphertext stealing.\n\n");

  printf("  /DECIPHER  Deciphers an encrypted file. The TDES ciphertext is converted\n"
         "             into plaintext. Mode: CBC with ciphertext stealing.\n\n");

  printf("  /MAC       A Message Authentication Code (MAC) is calculated from srcfile.\n"
         "             The cryptographic signature is written to destfile, which can be\n"
         "             appended to the plaintext as a cryptographic signature.\n"
         "             Mode: (CMAC NIST SP 800-38B).\n");
  TdesMore();    // Press any key to continue
  
  printf("  /ECBENCRYPT  Encrypts a file. The plaintext is TDES encrypted.\n"
         "               Mode: ECB with ciphertext stealing.\n\n");

  printf("  /ECBDECIPHER Deciphers an encrypted file. The TDES ciphertext is converted\n"
         "               into plaintext. Mode: ECB with ciphertext stealing.\n\n");

  printf("  /CBCE     Encrypts a file. Mode: CBC with ISO/IEC 7816-4 padding.\n\n");

  printf("  /CBCD     Deciphers an encrypted file. Mode: CBC with ISO padding.\n\n");

  printf("  /ECBE     Encrypts a file. Mode: ECB with ISO/IEC 7816-4 padding.\n\n");

  printf("  /ECBD     Deciphers an encrypted file. Mode: ECB with ISO padding.\n\n");

  printf("This utility is fast. When encrypting files, always be careful\n"
         "about keeping your keys privately at a secure place.\n"
         "Never send an encrypted file and its secret key through the same channel.\n"
         "For example, if you sent the encrypted file and this utility via e-mail\n"
         " to a certain person, you should communicate the secret key via\n"
         " telephone or surface mail, addressing the entitled person.\n");
  if (hedit_exe == TRUE)
    {
    printf("\nNOTE: 'copy hedit.exe tdes.exe' to build a crypto utility for TDES only.\n\n");
    } 
  
TdesAnyKey();    // Press any key for exit
  } // TdesDisplayHelp

//----------------------------------------------------------------------------
//
//                    main: TdesCheckRunByExplorer
//
// This console program could be run by typing its name at the command prompt,
// or it could be run by the user double-clicking it from Explorer.
// And you want to know which case you’re in.
//
void TdesCheckRunByExplorer()
  {
  // Check if invoked via Desktop
  DWORD procId = GetCurrentProcessId();
  DWORD count = GetConsoleProcessList(&procId, 1);

  if (count < 2)       // Invoked via Desktop
    {
    TdesDisplayHelp();
    printf("\nConsole application: TDES.EXE\n");
    system("cmd");     // Keep the Console window open
    exit(0);           // (user may recoursively start the console app)
    }

  // When invoked via Console (nothing to do)
  } // TdesCheckRunByExplorer


//-----------------------------------------------------------------------------
//
//                              TdesMain
//
int TdesMain(int tdes_argc, char **tdes_argv)
  {
  int i;
  struct stat _stat;

  for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = 0x00;  // CBC Clear-Init ICV
  if (tdes_argc == 6)                                     // ICV file is present
    {
    tdes_argc--;
    stat(tdes_argv[5], &_stat);                         // Key provided by file
    if (_stat.st_size > BLOCK_SIZE)
      {
      printf(error_fsize, tdes_argv[5], BLOCK_SIZE);      // File length error, or non-existance
      exit(1);
      }
    //
    // Read the key and initialize the DES ICV for CBCE /CBCD modes.
    //
    ifstream Icvfile(tdes_argv[5], ios::binary | ios::in); //Open input binary file
    if (!Icvfile)
      {
      printf(open_failed, &tdes_argv[5]);
      exit(1);
      }
    Icvfile.read(icvblock, _stat.st_size);   // Copy the icv from file and display it
//ha//    printf("iv-block: ");
//ha//    for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)icvblock[i]);
//ha//    printf("\n");
    } // ReadFileIcv

  if (tdes_argc < 5)                    // Illegal parameter
    {
    TdesDisplayHelp();                  // Illegal parameter
    exit(0);
    }
  else if (_stricmp(tdes_argv[4], "/DECIPHER") == 0) mode = TDES_DECIPHER;
  else if (_stricmp(tdes_argv[4], "/ENCRYPT") == 0) mode = TDES_ENCRYPT;
  else if (_stricmp(tdes_argv[4], "/MAC") == 0) mode = TDES_MAC;
  else if (_stricmp(tdes_argv[4], "/CBCD") == 0) mode = TDES_CBCD;
  else if (_stricmp(tdes_argv[4], "/CBCE") == 0) mode = TDES_CBCE;
  else if (_stricmp(tdes_argv[4], "/ECBD") == 0) mode = TDES_ECBD;
  else if (_stricmp(tdes_argv[4], "/ECBE") == 0) mode = TDES_ECBE;
  else if (_stricmp(tdes_argv[4], "/ECBDECIPHER") == 0) mode = TDES_ECBDECIPHER;
  else if (_stricmp(tdes_argv[4], "/ECBENCRYPT") == 0) mode = TDES_ECBENCRYPT;
  else
    {
    TdesDisplayHelp();                  // Illegal parameter
    exit(0);
    }

  //
  // Determine the key mode
  //
  if (strncmp(&tdes_argv[3][0], "/", 1) == 0)          // Key via command line
    {
    if (strlen(&tdes_argv[3][1]) > KEY_LENGTH)                  
      {
      printf("ERROR: TDES-KEYSIZE > %d!\n", KEY_LENGTH);
      exit(1);
      }
    for (i=0; (UINT)i<strlen(&tdes_argv[3][1]); i++)    // Copy the key from command line
      {

      if ((UINT)i >= strlen(&tdes_argv[3][1])) break;   // Pad all short keys
      tdes_keybuf[i] = (tdes_argv[3][i+1] & 0xFF);
      if (tdes_keybuf[i] == 0xFFFFFFA0) tdes_keybuf[i] = 0xFF; // WIN10: **argv will return wrong chars if typed
      }                                                 //  on keypad (eg. Alt+"1 2 8" thru "2 5 5" 
    }

  else
    {
    stat(tdes_argv[3], &_stat);                         // Key provided by file
    if (_stat.st_size > KEY_LENGTH)
      {
      printf(error_keysize, tdes_argv[3], KEY_LENGTH);
      exit(1);
      }

    //
    // Read the key and initialize the DES key schedule.
    //
    ifstream keyfile(tdes_argv[3], ios::binary | ios::in); //Open input binary file
    if (!keyfile)
      {
      printf(open_failed, &tdes_argv[3]);
      exit(1);
      }
    keyfile.read(tdes_keybuf, _stat.st_size);  // Copy the key from file
    }

  // Distribute equal parts of 64bits of tdes_keybuf into tdes_keybuf1 thru tdes_keybuf3
  for (i=0; i<BLOCK_SIZE; i++)
    {
    tdes_keybuf1[i]=tdes_keybuf[i]; 
    tdes_keybuf2[i]=tdes_keybuf[i+8]; 
    tdes_keybuf3[i]=tdes_keybuf[i+16]; 
    }
//ha//printf("keylength: %dbits\n", _keylength);
//ha//printf("tdes_keybuf: ");
//ha//for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)tdes_keybuf[i]);
//ha//printf("\n");
  
  // ---------------------------------
  // Open source and destination files
  // ---------------------------------
  stat(tdes_argv[1], &_stat);                          // Check source file size
  srcFileSize = _stat.st_size;                    // Init source file size

  if (srcFileSize < BLOCK_SIZE)                           // Must be at least one block
    {
    printf(error_fsize, tdes_argv[1], BLOCK_SIZE);      // File length error, or non-existance
    exit(1);
    }

  ifstream infile(tdes_argv[1], ios::binary | ios::in); //Open input binary file
  if (!infile)
    {
    printf(open_failed, &tdes_argv[1]);
    exit(1);
    }
  
  if (_access(tdes_argv[2], 0) == 0)     // Check if outfile already exists
    {                               
    printf(file_exists, &tdes_argv[2]);   
    exit(1);
    }                               

  ofstream outfile(tdes_argv[2], ios::binary | ios::out); //Open output binary file
  if (!outfile)
    {
    printf(open_failed, &tdes_argv[2]);
    exit(1);
    }

  printf(TdesSignon);                   // Display signon message

  // -------------------------
  // Perform the DES algorithm
  // -------------------------
  ln = 0L; li = COUNT_RATE;
  switch(mode)
    {
    case TDES_ENCRYPT:
      tdesKeyInit2(tdes_keybuf1, ENCRYPT,  1);
      tdesKeyInit2(tdes_keybuf2, DECIPHER, 2);
      tdesKeyInit2(tdes_keybuf3, ENCRYPT,  3);
      TdesDisplayIcvblock();
      TdesDoAlgorithmStealCBCE(infile, outfile);
// DebugStop("TdesMain()", 0x55); ///////ha//////////////////////////////////
      printf(bytes_encrypted, ln);
      break;

    case TDES_DECIPHER:
      tdesKeyInit2(tdes_keybuf3, DECIPHER, 1);
      tdesKeyInit2(tdes_keybuf2, ENCRYPT,  2);
      tdesKeyInit2(tdes_keybuf1, DECIPHER, 3);
      TdesDisplayIcvblock();
      TdesDoAlgorithmStealCBCD(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case TDES_CBCE:
      tdesKeyInit2(tdes_keybuf1, ENCRYPT,  1);
      tdesKeyInit2(tdes_keybuf2, DECIPHER, 2);
      tdesKeyInit2(tdes_keybuf3, ENCRYPT,  3);
      TdesDisplayIcvblock();
      TdesDoAlgorithmIsoCBCE(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case TDES_CBCD:
      tdesKeyInit2(tdes_keybuf3, DECIPHER, 1);
      tdesKeyInit2(tdes_keybuf2, ENCRYPT,  2);
      tdesKeyInit2(tdes_keybuf1, DECIPHER, 3);
      TdesDisplayIcvblock();
      TdesDoAlgorithmIsoCBCD(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case TDES_ECBENCRYPT:
      tdesKeyInit2(tdes_keybuf1, ENCRYPT,  1);
      tdesKeyInit2(tdes_keybuf2, DECIPHER, 2);
      tdesKeyInit2(tdes_keybuf3, ENCRYPT,  3);
      TdesDoAlgorithmStealECB(infile, outfile, ENCRYPT);
      printf(bytes_encrypted, ln);
      break;

    case TDES_ECBDECIPHER:
      tdesKeyInit2(tdes_keybuf3, DECIPHER, 1);
      tdesKeyInit2(tdes_keybuf2, ENCRYPT,  2);
      tdesKeyInit2(tdes_keybuf1, DECIPHER, 3);
      TdesDoAlgorithmStealECB(infile, outfile, DECIPHER);
      printf(bytes_deciphered, ln);
      break;

    case TDES_ECBE:
      tdesKeyInit2(tdes_keybuf1, ENCRYPT,  1);
      tdesKeyInit2(tdes_keybuf2, DECIPHER, 2);
      tdesKeyInit2(tdes_keybuf3, ENCRYPT,  3);
      TdesDoAlgorithmIsoECB(infile, outfile, ENCRYPT);
      printf(bytes_encrypted, ln);
      break;

    case TDES_ECBD:
      tdesKeyInit2(tdes_keybuf3, DECIPHER, 1);
      tdesKeyInit2(tdes_keybuf2, ENCRYPT,  2);
      tdesKeyInit2(tdes_keybuf1, DECIPHER, 3);
      TdesDoAlgorithmIsoECB(infile, outfile, DECIPHER);
      printf(bytes_deciphered, ln);
      break;

    case TDES_MAC:
      tdesKeyInit2(tdes_keybuf1, ENCRYPT,  1);
      tdesKeyInit2(tdes_keybuf2, DECIPHER, 2);
      tdesKeyInit2(tdes_keybuf3, ENCRYPT,  3);
      TdesDisplayIcvblock();
      TdesDoAlgorithmMac(infile, outfile);
      printf("%lu bytes processed. MAC = [", ln);       // Display the MAC
      for (i=0; i<BLOCK_SIZE; i++) printf("%02X", (UCHAR)outblock[i]);
      printf("]\n%s: %d Bytes have been written.\n", tdes_argv[2], BLOCK_SIZE);
      break;

    default:
      printf("Illegal option.\n");
      break;
    } // end switch

  infile.close();
  outfile.close();
  if(!outfile.good())
    {
    cout << "Error occurred at writing time!" << endl;
    exit(1);
    }
  exit(0);
  } // TdesMain

//-------------------------- end of main module -------------------------------

//ha//DebugStop("bytesrd:",bytesrd);
//ha//DebugStop("isoPad:", isoPad);
//ha//DebugStop("j:", j);
//ha//DebugPrintBuffer(inblock, 8);


