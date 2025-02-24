// hedit_tdes.cpp - C++ Developer source file.
// (c)2021 by helmut altmann

// DES Algorithm Encryption Modes for file encryption

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

#define ENCRYPT             0
#define DECIPHER            1
#define BUILD_ENCRYPT_KEY   2
#define BUILD_DECIPHER_KEY  3
#define PERMUTE_KEY         4
#define FEED_KEY            5

#define DES_ENCRYPT  15
#define DES_DECIPHER 16
#define DES_MAC      17
#define DES_CBCE     18
#define DES_CBCD     19
#define DES_ECBE     20
#define DES_ECBD     21
#define DES_ECBDECIPHER 22
#define DES_ECBENCRYPT  23

#define KEY_LENGTH 8
#define BLOCK_SIZE 8

#define COUNT_RATE 100000L

//----------------------------------------------------------------------------
//
//                          External declarations
//
#ifdef DES_AES_QUICK
  extern void desAlgorithm (char*, char*);      // C++ Module Interface
  extern void desKeyInit(char*, int);           // C++ Module Interface
#else
  extern "C" void desAlgorithm (char*, char*);  // ASM Module Interface
  extern "C" void desKeyInit(char*, int);       // ASM Module Interface
#endif

extern void DebugStop(char *, int);

extern char * open_failed;

extern unsigned char hedit_exe;
extern int i, j;
extern unsigned long ln, li, lj, ls;;

extern char inbuf1[BLOCK_SIZE], inbuf2[BLOCK_SIZE];
extern char outbuf1[BLOCK_SIZE];

 
//----------------------------------------------------------------------------
//
//                          Global declarations
//
char * signon           = "DES Crypto Utility, V2.00 (c)1997-2021 by ha\n";

char file_exists[]      = "ERROR %s: FILE ALREADY EXISTS.\n";
char error_fsize[]      = "ERROR %s: FILE SIZE <> %d?\n";
char error_keysize[]    = "ERROR %s: KEY SIZE <> %d?\n";
char bytes_deciphered[] = "%lu bytes deciphered.\n";
char bytes_encrypted[]  = "%lu bytes encrypted.\n";

char icvblock[BLOCK_SIZE];
char inblock[BLOCK_SIZE], outblock[BLOCK_SIZE], lastblock[2*BLOCK_SIZE];
char keybuf[KEY_LENGTH] = {
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD
  }; // Provide space for key size of 64bits

UCHAR mode;
int bytesrd;
UINT length;
long int srcFileSize;

ofstream outfile; 
streampos pos;    // for seek test only

//-----------------------------------------------------------------------------
//
//                        DesDoAlgorithmStealECB
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ciphertext stealing)
//
void DesDoAlgorithmStealECB(ifstream &infile, ofstream &outfile)
  {
  li = COUNT_RATE;  ls = srcFileSize;

  bytesrd = BLOCK_SIZE; // init bytesrd, filesize is at least 8 bytes
  while (ln < ls)
    {
    if ((ln+BLOCK_SIZE) > ls) bytesrd = (int)(ls % BLOCK_SIZE);
    ln += (ULONG)bytesrd;

    infile.read(inblock, bytesrd);

    if (bytesrd == BLOCK_SIZE)
      {
      desAlgorithm(inblock, outblock);
      outfile.write(outblock, bytesrd);
      }

    //
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
    else
      {
      for (i = 0; i < bytesrd; i++) lastblock[BLOCK_SIZE + i] = outblock[i];
      for (i = bytesrd; i < BLOCK_SIZE; i++) inblock[i] = outblock[i];

      desAlgorithm (inblock, lastblock);    //@Am0003
      outfile.seekp(0, ios::end);           // seek to the end of the file
      outfile.seekp(-BLOCK_SIZE, ios::cur); // back up 8 bytes
      outfile.write(lastblock, bytesrd + BLOCK_SIZE);
      }

    //pos = infile.tellg();
    //cout << "infile: The file pointer is now at location " << pos << endl;
    //pos = outfile.tellp();
    //cout << "outfile: The file pointer is now at location " << pos << ;

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end while

  } // DesDoAlgorithmStealECB


//-----------------------------------------------------------------------------
//
//                          DesDoAlgorithmIsoECB
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ISO Padding)
//
void DesDoAlgorithmIsoECB(ifstream &infile, ofstream &outfile)
  {
  int isoPad = 0; j = 0;

  li = COUNT_RATE;  ls = srcFileSize;

  bytesrd = BLOCK_SIZE; // init
  while (ln < ls)
    {
    if ((ln+BLOCK_SIZE) > ls) bytesrd = (int)(ls % BLOCK_SIZE);
    ln += (ULONG)bytesrd;

    infile.read(inblock, bytesrd);                                            

    if (bytesrd == BLOCK_SIZE)
      {
      desAlgorithm(inblock, outblock);

      if (mode == DES_ECBE)
        outfile.write(outblock, BLOCK_SIZE);     // Write all blocks

      else if ((mode == DES_ECBD) && (ln != ls))
        outfile.write(outblock, BLOCK_SIZE);     // Write all blocks, except the last block

      else if ((mode == DES_ECBD) && (ln == ls)) // Last block requires special handling
        {                                        // Either a whole block of padding
        j=BLOCK_SIZE;                            //  or a partly padded block
                                                 
        for (i=0; i<BLOCK_SIZE; i++)
          {
          j--;
          if (outblock[j] == 0xFFFFFF80)         // ISOPAD rendered as long int ????
            {                                    
            break;
            }
          }

        outfile.write(outblock, j);              // Write until ISOPAD
        ln += ((ULONG)j - BLOCK_SIZE);           // Adjust (decipher) filesize count
        break;                                   // Stop while loop if ISOPAD
        }
      }

    //
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
    else // bytesrd < BLOCK_SIZE
      {
      //
      // The last block is padded.
      //
      inblock[bytesrd] = ISOPAD;
      for (i=bytesrd+1; i<BLOCK_SIZE; i++) inblock[i] = PAD;

      desAlgorithm (inblock, outblock);

      outfile.write(outblock, BLOCK_SIZE);
      ln += BLOCK_SIZE-bytesrd;
      isoPad = 1;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);      // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end while

  if ((mode == DES_ECBE) && (isoPad == 0)) // Need to add a whole padding block
    {                                      // Does not apply to encrypted text
    inblock[0]=ISOPAD;                     //  which is always padded MOD(8)
    for (i=1; i<BLOCK_SIZE; i++) inblock[i] = PAD;

    desAlgorithm (inblock, outblock);

    outfile.write(outblock, BLOCK_SIZE);
    ln += BLOCK_SIZE;                      // Adjust (decipher) filesize count
    }
  } // DesDoAlgorithmIsoECB


//------------------------------------------------------------------------------
//
//                          DesDoAlgorithmStealCBCE
//
//  ENCRYPT - Cipher Block Chaining (Ciphertext Stealing)
//               
void DesDoAlgorithmStealCBCE(ifstream &infile, ofstream &outfile)
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
      //        Pn = Last plaintext < BLOCK_SIZE
      //        Pn*       = Last plaintext padded with zeros
      //        Cn-1      = Previous Ciphertext of Pn-1
      //        Cn-1*     = Ciphertext padded to Pn, stolen from previous block
      //        Cn-1(new) = New Ciphertext of BLOCK_SIZE for previous block
      //        Cn-1**    = Ciphertext < BLOCKSIZE from previous block, used last.
      //
      // Example: Encrypt Key = 12345678
      //          Pn-1   = 0A 0D 0A 0D 0A 0D 0A 0D
      //          Cn-1   = F5 AC DD BE 5F 21 C0 2B
      //          Pn   = 0A 0D 0A 0D 0A
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
    desAlgorithm(inblock, outblock);
    if (ciphStealing == TRUE)
      {
      // Build the last block(s) [Cn-1(new) || Cn-1**],
      //  where Cn-1(new) consists of the encrypted incomplete block of Pn
      //  and the stolen chunk Cn-1* which has been encrypted twice.
      //
      for (i = 0; i < BLOCK_SIZE; i++) lastblock[i] = outblock[i];          // Cn-1(new)
      for (i = 0; i < bytesrd; i++) lastblock[BLOCK_SIZE + i] = outbuf1[i]; // Cn-1**
      }

    ln += (ULONG)bytesrd;                            // Update counter total bytes read

    // -----------------------------------------
    // Special processing after applying the DES
    // -----------------------------------------
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
  } // DesDoAlgorithmStealCBCE


//------------------------------------------------------------------------------
//
//                          DesDoAlgorithmStealCBCD
//
//  DECIPHER - Cipher Block Chaining (Ciphertext Stealing)
//
void DesDoAlgorithmStealCBCD(ifstream &infile, ofstream &outfile)
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
      //        Pn = Last plaintext < BLOCK_SIZE
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

      desAlgorithm(lastblock, outblock);

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
      desAlgorithm(inblock, outblock);
      if (bytesrd == BLOCK_SIZE && ciphStealing == FALSE)
        for (i=0; i<BLOCK_SIZE; i++) inblock[i] = outblock[i];
      }

    ln += (ULONG)bytesrd;                             // Update counter total bytes read
    
    // -----------------------------------------
    // Special processing after applying the DES 
    // -----------------------------------------
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
  } // DesDoAlgorithmStealCBCD

//ha//DebugStop("bytesrd:",bytesrd);
//DebugStop("while:", (int)ln);                         
//ha//DebugPrintBuffer(inblock, bytesrd);
//DebugStop("Last write deciphered:", bytesrd + BLOCK_SIZE);      
//DebugPrintBuffer(lastblock, bytesrd + BLOCK_SIZE);


//------------------------------------------------------------------------------
//
//                              DesDoAlgorithmIsoCBCE
//
//  ENCRYPT - Cipher Block Chaining (ISO Padding)
//
void DesDoAlgorithmIsoCBCE(ifstream &infile, ofstream &outfile)
  {
  li = COUNT_RATE;  ls = srcFileSize; ln = 0;
  int isoPad = PAD;

  //
  // CBC specific initial chaining vector init function
  //
  for (i=0; i<BLOCK_SIZE; i++) outblock[i] = 0;               // CBC Init ICV

  bytesrd = BLOCK_SIZE; // init
  while (ln < ls)
    {
    if ((ln+BLOCK_SIZE) > ls) bytesrd = (int)(ls % BLOCK_SIZE);
    ln += (ULONG)bytesrd;

    infile.read(inblock, bytesrd);

    if (bytesrd == BLOCK_SIZE)
      {
      //
      // CBC specific XOR function
      //
      for (i=0; i<BLOCK_SIZE; i++) inblock[i] ^= icvblock[i]; // CBC; inbuf XOR ICV

      desAlgorithm(inblock, outblock);
      outfile.write(outblock, bytesrd);

      for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = outblock[i]; // Update ICV
      }

    //
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
    else
      {
      //
      // The last block is padded.
      //
      inblock[bytesrd] = ISOPAD;
      for (i=bytesrd+1; i<BLOCK_SIZE; i++) inblock[i] = PAD;

      //
      // CBC specific XOR function
      //
      for (i=0; i<BLOCK_SIZE; i++) inblock[i] ^= outblock[i]; // CBC XOR

      desAlgorithm (inblock, outblock);
      outfile.write(outblock, BLOCK_SIZE);
      ln += BLOCK_SIZE-bytesrd;
      isoPad = 1;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end while

  //
  // Encrypt and decipher modes must be handled differently.
  // Encrypt: If the srcFileSize is a multiple of BLOCK_SIZE we must append
  //          a whole block of ISO padding.
  // Decipher: Nothing to do, no final check required.
  //
  if (isoPad == 0)                         // Need to add a whole padding block
    {
    inblock[0]=ISOPAD;
    for (i=1; i<BLOCK_SIZE; i++) inblock[i] = PAD;

    //
    // CBC specific XOR function
    //
    for (i=0; i<BLOCK_SIZE; i++) inblock[i] ^= outblock[i]; // CBC XOR

    desAlgorithm (inblock, outblock);
    outfile.write(outblock, BLOCK_SIZE);
    ln += BLOCK_SIZE;                      // Adjust filesize count
    }
  } // DesDoAlgorithmIsoCBCE

//-----------------------------------------------------------------------------
//
//                              DesDoAlgorithmIsoCBCD
//
//  DECIPHER - Cipher Block Chaining (ISO Padding)
//
void DesDoAlgorithmIsoCBCD(ifstream &infile, ofstream &outfile)
  {
  li = COUNT_RATE;  ls = srcFileSize;

  //
  // CBC specific initial chaining vector init function
  //
  bytesrd = BLOCK_SIZE; // init
  do
    {
    infile.read(inblock, bytesrd);
    ln += (ULONG)bytesrd;

    desAlgorithm(inblock, outblock);
    //
    // CBC specific XOR function
    //
    for (i=0; i<BLOCK_SIZE; i++) outblock[i] ^= icvblock[i]; // CBC XOR
    for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = inblock[i];   // CBC copy

    //
    // Encrypt and decipher modes must be handled differently.
    // Encrypt: Since "bytesrd==BLOCK_SIZE" we just write outblock to dstfile.
    // Decipher: Since ISO padding was applied to the enrypted file, it is
    //           guaranteed that the srcFileSize is a multiple of BLOCK_SIZE.
    //           However, we should remove the ISO padding from the deciphered
    //           plaintext, which is done here.
    //
    if (ln != srcFileSize) outfile.write(outblock, BLOCK_SIZE);

    else                      // (ln == srcFileSize)
      {                       // Remove ISO padding from plaintext before fwrite
      j = BLOCK_SIZE;         // Assume a whole block of padding
      for (i=0; i<BLOCK_SIZE; i++)
        {
        j--;
        if (outblock[j] == 0xFFFFFF80)  // Stop at ISOPAD (rendered as long int ????)
          {
          break;
          }
        }

      outfile.write(outblock, j);       // Write until ISOPAD
      break;                            // Stop while loop if ISOPAD
      }                                 // (discard ISOPAD)

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);   // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    }
  while (ln < ls);  // end do while

  ln -= bytesrd; ln += (ULONG)j;        // Adjust (decipher) filesize count
  } // DesDoAlgorithmIsoCBCD


//----------------------------------------------------------------------------
//
//                                DesDoAlgorithmMac
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
#define DES_BLOCK_SIZE 8
 
void DesDoAlgorithmMac(ifstream &infile, ofstream &outfile)
  {
  int msbFlag;

  char pszRB64[DES_BLOCK_SIZE] = {0,0,0,0,0,0,0,0x1B};
  char pszZeroBlock[DES_BLOCK_SIZE];
  char pszSubkeyK1[DES_BLOCK_SIZE+1]; // Adding one 0-byte for ROL operation
  char pszSubkeyK2[DES_BLOCK_SIZE];
  
  // Generating the subkeys K1 and K2
  //  
  //  Example:
  //  Block cipher is the DES algorithm:
  //    Key =        8aa83bf8 cbda1062
  //
  //  Subkey K1, K2 Generation
  //    CIPHK(064) = DA FF D1 15 C4 DC F5 3E
  //    K1 =         85 FF A2 2B 89 B9 EA 67
  //    K2 =         6B FF 44 57 13 73 D4 D%
  //
  // Example Mlen = 64:
  //    M =          6bc1bee2 2e409f96
  //    T =          20 37 34 C0 22 B2 26 C8
  //
  // Example Mlen = 160:
  //    M =          6bc1bee2 2e409f96 e93d7e11 7393172a
  //                 ae2d8a57
  //    T =          E3 A8 DD 10 1A 7B CB B5
  //
  // Example Mlen = 256:
  //    M =          6bc1bee2 2e409f96 e93d7e11 7393172a
  //                 ae2d8a57 1e03ac9c 9eb76fac 45af8e51
  //    T =          59 EB 8D B9 78 9D AF C7

  pszSubkeyK1[DES_BLOCK_SIZE+0] = 0; // Ensure K1 last shifted bit = 0

  for (i=0; i<DES_BLOCK_SIZE; i++)
    pszZeroBlock[i] = icvblock[i];   // Init Zero - block

  // K1
  desAlgorithm(pszZeroBlock, pszSubkeyK1);

  msbFlag = (UCHAR)pszSubkeyK1[0] & 0x80; // Set flag for XOR K1 later

  for (i=0; i<DES_BLOCK_SIZE; i++)
    {
    pszSubkeyK1[i] = pszSubkeyK1[i] << 1;
    pszSubkeyK1[i] = pszSubkeyK1[i] | ((pszSubkeyK1[i+1] & 0x80) >> 7);
    }                                                                             

  if (msbFlag != 0)
    {
    for (i=0; i<DES_BLOCK_SIZE; i++) pszSubkeyK1[i] ^= pszRB64[i];
    }

  // K2
  for (i=0; i<DES_BLOCK_SIZE; i++)
    pszSubkeyK2[i] = (pszSubkeyK1[i] << 1) | ((pszSubkeyK1[i+1] & 0x80) >> 7);
  
  msbFlag = (UCHAR)pszSubkeyK1[0] & 0x80; // Set flag for XOR K1 later
  if (msbFlag != 0)
    {
    for (i=0; i<DES_BLOCK_SIZE; i++) pszSubkeyK2[i] ^= pszRB64[i];
    }

  // Prepare the last block of DES_BLOCK_SIZE in inblock:
  DWORD dwCryptFileSize = srcFileSize;
    
  // CMAC 
  ln = 0; ls = dwCryptFileSize;    // init counters
  bytesrd = DES_BLOCK_SIZE;        // init bytesrd, filesize is at least 8 bytes
  while (ln < ls)
    {
    if ((ls - ln) >= DES_BLOCK_SIZE) bytesrd = DES_BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = ls - ln;                                    //  ifstream won't tell us   

    infile.read(inblock, bytesrd);

    // K1) Message length is a positive multiple of the block size
    if ((ls - ln) == DES_BLOCK_SIZE)
      {
      for (i=0; i<DES_BLOCK_SIZE; i++) inblock[i] ^= pszSubkeyK1[i];
      }

    // K2) Message length is not a positive multiple of the block size  
    else if ((ls - ln) < DES_BLOCK_SIZE)
      {
      for (i=(int)(dwCryptFileSize % DES_BLOCK_SIZE); i<DES_BLOCK_SIZE; i++)
        {
        // CMAC Padding starts with 10000000b and continues with all bits zeroed
        if (i == (int)(dwCryptFileSize % DES_BLOCK_SIZE)) inblock[i] = ISOPAD;       
        else  inblock[i] = PAD;
        }
      for (i=0; i<DES_BLOCK_SIZE; i++) inblock[i] ^= pszSubkeyK2[i];
      }

    for (i=0; i<DES_BLOCK_SIZE; i++) inblock[i] ^= icvblock[i]; // CBC: inbuf XOR  ICV
    desAlgorithm(inblock, outblock);
    for (i=0; i<DES_BLOCK_SIZE; i++) icvblock[i] = outblock[i]; // Update ICV w/ next block

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);       // Echo per COUNT_RATE
      li += COUNT_RATE;
      }

    ln += (ULONG)bytesrd;                   // Update counter total bytes read
    } // end while

  outfile.write(outblock, DES_BLOCK_SIZE);  // Emit the MAC to file
  } // DesDoAlgorithmMac


//-----------------------------------------------------------------------------
//
//                              OutfileXorInfile
//
//  For use in CBC Test-Batchfiles:  Outfile ^= infile
//  For simplicity no Error-checking  on file streams is done here!
//
void OutfileXorInfile(char *_outfile, char *_infile)
    {
    char inbuf[BLOCK_SIZE], outbuf[BLOCK_SIZE];

    ifstream tmpfile(_outfile, ios::binary | ios::in); // Open input binary file
    tmpfile.read(outbuf, BLOCK_SIZE);  //Fill the outbuffer from file and display it

    ifstream infile(_infile, ios::binary | ios::in); // Open input binary file
    infile.read(inbuf, BLOCK_SIZE);  //Fill the inbuffer from file and display it

//ha///DebugPrintBuffer(inbuf, BLOCK_SIZE);
    for (i=0; i<BLOCK_SIZE; i++) outbuf[i] ^= inbuf[i]; // CBC XOR
//ha//DebugPrintBuffer(outbuf, BLOCK_SIZE);

    ofstream outfile(_outfile, ios::binary | ios::out); //Open output binary file
    outfile.write(outbuf, BLOCK_SIZE);

    tmpfile.close();
    infile.close();
    outfile.close();
    exit(0);
    }  // OutfileXorInfile


//----------------------------------------------------------------------------
//
//                                DisplayIcvblock
//
void DisplayIcvblock()
  {
  printf("iv-block: ");
  for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)icvblock[i]);
  printf("\n");
  } // DisplayIcvblock

//----------------------------------------------------------------------------
//
//                                ClearScreen
//
void ClearScreen()
  {
  system("cls");
  } // ClearScreen


//----------------------------------------------------------------------------
//
//                                 AnyKey
//
void AnyKey()
  {
  while (_kbhit() != 0) _getch();   // Flush key-buffer 
  printf("-- press any key --\n");
  _getch();
  } // AnyKey

                                                        
//----------------------------------------------------------------------------
//
//                                 More
//
void More()
  {
  while (_kbhit() != 0) _getch();   // flush key-buffer 
  printf("-- more --\r");
  _getch();
  ClearScreen();
  } // More


//-----------------------------------------------------------------------------
//
//                              DisplayHelp
//
void DisplayHelp()
  {
  printf(signon);  // Display DesSignon message

  printf("Performs encryption and decryption using the Data Encryption Standard.\n\n");
  
  printf("Usage: '%s srcfile destfile [keyfile | /keystring] [options] [ivfile]",
                  (hedit_exe == FALSE) ? "DES" : "HEDIT");
  printf("%s'\n", (hedit_exe == FALSE) ? "" : " /DES");                                
  printf("  srcfile    Input file (plain text or encrypted text >= 8 bytes).\n"
         "  destfile   Output file (after the algorithm has been applied).\n"
         "  ivfile     Input iv-file (Init Vector, optional for CBC modes).\n\n");

  printf("  keyfile    Input file containing the secret key.\n"
         "             The key can be 8 bytes max. The effective key length is\n"
         "             56 bits, i.e., parity bits of the 'key' are ignored.\n"
         "  /keystring To avoid a keyfile the key may be directly given\n"
         "             as a string of 8 ascii characters: e.g. /12345678.\n");
  printf("[options]\n");
  printf("  /ENCRYPT  Encrypts a file. The plaintext is DES encrypted.\n"
         "            Mode: CBC with ciphertext stealing.\n\n");

  printf("  /DECIPHER Deciphers an encrypted file. The DES ciphertext is converted\n"
         "            into plaintext. Mode: CBC with ciphertext stealing.\n\n");

  printf("  /MAC      A Message Authentication Code (MAC) is calculated from srcfile.\n"
         "            The cryptographic signature is written to destfile, that can be\n"
         "            appended to the plaintext as a cryptographic signature.\n"
         "            Mode: (CMAC NIST SP 800-38B).\n");
  More();          // Press any key to continue
  
  printf("  /ECBENCRYPT  Encrypts a file. The plaintext is DES encrypted.\n"
         "               Mode: ECB with ciphertext stealing.\n\n");

  printf("  /ECBDECIPHER Deciphers an encrypted file. The DES ciphertext is converted\n"
         "               into plaintext. Mode: ECB with ciphertext stealing.\n\n");

  printf("  /CBCE     Encrypts a file. Mode: CBC with ISO/IEC 7816-4 padding.\n\n");

  printf("  /CBCD     Deciphers an encrypted file. Mode: CBC with ISO padding.\n\n");

  printf("  /ECBE     Encrypts a file. Mode: ECB with ISO/IEC 7816-4 padding.\n\n");

  printf("  /ECBD     Deciphers an encrypted file. Mode: ECB with ISO padding.\n\n");

  printf("  /XOR  Additional option. Usage: 'DES outfile infile /XOR'\n"
         "        May be used in batches to perform 'outfile ^= infile'\n\n");

  printf("This utility is very fast! When encrypting files, always be careful\n"
         " about keeping your keys privately at a secure place.\n"
         " Never send an encrypted file and its secret key through the same channel.\n"
         " For example, if you sent the encrypted file and this utility via e-mail\n"
         " to a certain person, you should communicate the secret key via\n"
         " telephone or surface mail, addressing the entitled person.\n");
  if (hedit_exe == TRUE)
    {
    printf("\nNOTE: 'copy hedit.exe des.exe' to build a crypto utility for DES only.\n\n");
    } 
  
  AnyKey();        // Press any key for exit
  } // DisplayHelp

//----------------------------------------------------------------------------
//
//                    main: DesCheckRunByExplorer
//
// This console program could be run by typing its name at the command prompt,
// or it could be run by the user double-clicking it from Explorer.
// And you want to know which case you’re in.
//
void DesCheckRunByExplorer()
  {
  // Check if invoked via Desktop
  DWORD procId = GetCurrentProcessId();
  DWORD count = GetConsoleProcessList(&procId, 1);

  if (count < 2)       // Invoked via Desktop
    {
    DisplayHelp();
    printf("\nConsole application: DES.EXE\n");
    system("cmd");     // Keep the Console window open
    exit(0);           // (user may recoursively start the console app)
    }

  // When invoked via Console (nothing to do)
  } // DesCheckRunByExplorer

//-----------------------------------------------------------------------------
//
//                              DesMain
//
int DesMain(int des_argc, char **des_argv)   //@0003 Interfacing HEDIT
  {
  int i;
  struct stat _stat;

  if ((des_argc == 4) && _stricmp(des_argv[3], "/XOR") == 0)  // XOR files requested
    {
    stat(des_argv[1], &_stat);                           // Input file status
    if (_stat.st_size != BLOCK_SIZE) ;
    OutfileXorInfile(des_argv[1], des_argv[2]);
    }

  for (i=0; i<BLOCK_SIZE; i++) icvblock[i] = 0x00;  // CBC Clear-Init ICV
  if (des_argc == 6)                                    // ICV file is present
    {
    des_argc--;
    stat(des_argv[5], &_stat);                          // Key provided by file
    if (_stat.st_size > BLOCK_SIZE)
      {
      printf(error_fsize, des_argv[5], BLOCK_SIZE);     // File length error, or non-existance
      exit(1);
      }
    //
    // Read the key and initialize the DES ICV for CBCE /CBCD modes.
    //
    ifstream Icvfile(des_argv[5], ios::binary | ios::in); //Open input binary file
    if (!Icvfile)
      {
      printf(open_failed, des_argv[5]);
      exit(1);
      }
    Icvfile.read(icvblock, _stat.st_size);   // Copy the icv from file and display it
//ha//    printf("iv-block: ");
//ha//    for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)icvblock[i]);
//ha//    printf("\n");
    } // ReadFileIcv

  if (des_argc < 5)                     // Illegal parameter
    {
    DisplayHelp();                      // Illegal parameter
    return(0);
    }      
  else if (_stricmp(des_argv[4], "/DECIPHER") == 0) mode = DES_DECIPHER;
  else if (_stricmp(des_argv[4], "/ENCRYPT") == 0) mode = DES_ENCRYPT;
  else if (_stricmp(des_argv[4], "/MAC") == 0) mode = DES_MAC;
  else if (_stricmp(des_argv[4], "/CBCD") == 0) mode = DES_CBCD;
  else if (_stricmp(des_argv[4], "/CBCE") == 0) mode = DES_CBCE;
  else if (_stricmp(des_argv[4], "/ECBD") == 0) mode = DES_ECBD;
  else if (_stricmp(des_argv[4], "/ECBE") == 0) mode = DES_ECBE;
  else if (_stricmp(des_argv[4], "/ECBDECIPHER") == 0) mode = DES_ECBDECIPHER;
  else if (_stricmp(des_argv[4], "/ECBENCRYPT") == 0) mode = DES_ECBENCRYPT;
  else
    {
    DisplayHelp();                      // Illegal parameter
    return(0);
    }
//printf("des_argv[3]: %02X %02X %02X %02X %02X %02X %02X %02X \n",
//    argv[3][0], argv[3][1], argv[3][2], argv[3][3],
//    argv[3][4], argv[3][5], argv[3][6], argv[3][7]);
//DebugStop(des_argv[3], 0x55);

  //
  // Determine the key mode
  //
  if (strncmp(&des_argv[3][0], "/", 1) == 0)          // Key via command line
    {
    if (strlen(&des_argv[3][1]) > KEY_LENGTH)                 
      {
      printf("ERROR: DES-KEYSIZE > %d!\n", KEY_LENGTH);
      exit(1);
      }
    for (i=0; (UINT)i<strlen(&des_argv[3][1]); i++)   // Copy the key from command line
      {
      if ((UINT)i >= strlen(&des_argv[3][1])) break;  // Pad all short keys
      keybuf[i] = (des_argv[3][i+1] & 0xFF);
      if (keybuf[i] == 0xFFFFFFA0) keybuf[i] = 0xFF; // WIN10: **argv will return wrong chars if typed
      }                                              //  on keypad (eg. Alt+"1 2 8" thru "2 5 5"  

    }

  else
    {
    stat(des_argv[3], &_stat);                        // Key provided by file
    if (_stat.st_size > KEY_LENGTH)
      {
      printf(error_keysize, des_argv[3], KEY_LENGTH);
      exit(1);
      }

    //
    // Read the key and initialize the DES key schedule.
    //
    ifstream keyfile(des_argv[3], ios::binary | ios::in); //Open input binary file
    if (!keyfile)
      {
      printf(open_failed, des_argv[3]);
      exit(1);
      }
    keyfile.read(keybuf, _stat.st_size);            // Copy the key from file
    }

  // ---------------------------------
  // Open source and destination files
  // ---------------------------------
  stat(des_argv[1], &_stat);                        // Check source file size
  srcFileSize = _stat.st_size;                      // Init source file size

  if (srcFileSize < BLOCK_SIZE)                     // Must be at least one block
    {
    printf(error_fsize, des_argv[1], BLOCK_SIZE);   // File length error, or non-existance
    exit(1);
    }

  ifstream infile(des_argv[1], ios::binary | ios::in); //Open input binary file
  if (!infile)
    {
    printf(open_failed, des_argv[1]);
    exit(1);
    }
  
  if (_access(des_argv[2], 0) == 0)     // Check if outfile already exists
    {                               
    printf(file_exists, des_argv[2]); 
    exit(1);
    }

  ofstream outfile(des_argv[2], ios::binary | ios::out); //Open output binary file
  if (!outfile)
    {
    printf(open_failed, des_argv[2]);
    exit(1);
    }

  printf(signon);                   // Display signon message

  // -------------------------
  // Perform the DES algorithm
  // -------------------------
  ln = 0L; li = COUNT_RATE;
  switch(mode)
    {
    case DES_ENCRYPT:
      DisplayIcvblock();
      desKeyInit(keybuf, ENCRYPT);
      DesDoAlgorithmStealCBCE(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case DES_DECIPHER:
      DisplayIcvblock();
      desKeyInit(keybuf, DECIPHER);
      DesDoAlgorithmStealCBCD(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case DES_ECBENCRYPT:
      desKeyInit(keybuf, ENCRYPT);
      DesDoAlgorithmStealECB(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case DES_ECBDECIPHER:
      desKeyInit(keybuf, DECIPHER);
      DesDoAlgorithmStealECB(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case DES_CBCE:
      DisplayIcvblock();
      desKeyInit(keybuf, ENCRYPT);
      DesDoAlgorithmIsoCBCE(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case DES_CBCD:
      DisplayIcvblock();
      desKeyInit(keybuf, DECIPHER);
      DesDoAlgorithmIsoCBCD(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case DES_ECBE:
      desKeyInit(keybuf, ENCRYPT);
      DesDoAlgorithmIsoECB(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case DES_ECBD:
      desKeyInit(keybuf, DECIPHER);
      DesDoAlgorithmIsoECB(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case DES_MAC:
      DisplayIcvblock();
      desKeyInit(keybuf, ENCRYPT);
      DesDoAlgorithmMac(infile, outfile);
      printf("%lu bytes processed. MAC = [", ln);     // Display the MAC
      for (i=0; i<BLOCK_SIZE; i++) printf("%02X", (UCHAR)outblock[i]);
      printf("]\n%s: %d Bytes have been written.\n", des_argv[2], BLOCK_SIZE );
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
  } // DesMain

//-------------------------- end of main module -------------------------------
