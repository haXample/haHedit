// hedit_aes.cpp - C++ Developer source file.
// (c)2021 by helmut altmann

// AES Algorithm Encryption Modes for file encryption

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

#define AES_ENCRYPT  15
#define AES_DECIPHER 16
#define AES_MAC      17
#define AES_CBCE     18
#define AES_CBCD     19
#define AES_ECBE     20
#define AES_ECBD     21
#define AES_ECBDECIPHER 22
#define AES_ECBENCRYPT  23

#define KEY_LENGTH 16
#define BLOCK_SIZE 16

#define COUNT_RATE 100000L

//----------------------------------------------------------------------------
//
//                          External declarations
//
#ifdef DES_AES_QUICK
  extern void aesAlgorithm (char*, char*, int);      // C++ Module Interface
  extern void aesKeyInit(char*, int, int);           // C++ Module Interface
#else
  extern "C" void aesAlgorithm (char*, char*, int);  // ASM Module Interface
  extern "C" void aesKeyInit(char*, int, int);       // ASM Module Interface
#endif

extern void DebugStop(char *, int);
extern void ClearScreen();
extern void More();
extern void AnyKey();

extern char * open_failed;    //[] vs *
extern char file_exists[];    //[] vs *
extern char error_fsize[];    //[] vs *
extern char error_keysize[];
extern char bytes_deciphered[];
extern char bytes_encrypted[];

extern char inbuf1[], inbuf2[];
extern char outbuf1[];

extern unsigned char hedit_exe, mode;
extern int i, j, bytesrd;
extern unsigned long ln, li, lj, ls;
extern long int srcFileSize;

extern streampos pos;         // for seek test only
extern ofstream outfile;  

extern void DebugPrintBuffer(char *, int);
extern void DebugStopBuf(unsigned char, unsigned long);

//----------------------------------------------------------------------------
//
//                          Global declarations
//
char AesSignon[]   = "AES Crypto Utility, V1.00 (c)2021 by ha\n";

char AesIcvblock[BLOCK_SIZE];
char AesInblock[BLOCK_SIZE], AesOutblock[BLOCK_SIZE], AesLastblock[2*BLOCK_SIZE];
char AesKeybuf[2*KEY_LENGTH] = {
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD, \
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD, \
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD, \
  PAD, PAD, PAD, PAD, PAD, PAD, PAD, PAD
  }; // Provide space for key size of 256bits

char AesInbuf1[BLOCK_SIZE], AesInbuf2[BLOCK_SIZE];
char AesOutbuf1[BLOCK_SIZE];

int _keylength = 128;  // AES default keysize = 128 bits


//-----------------------------------------------------------------------------
//
//                              AesDoAlgorithmStealECB
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ciphertext stealing)
//
void AesDoAlgorithmStealECB(ifstream &infile, ofstream &outfile, int _mode)
  {
  li = COUNT_RATE;  ls = srcFileSize;

  bytesrd = BLOCK_SIZE; // init bytesrd, filesize is at least 16 bytes
  while (ln < ls)
    {
    if ((ln+BLOCK_SIZE) > ls) bytesrd = (int)(ls % BLOCK_SIZE);
    ln += (ULONG)bytesrd;

    infile.read(AesInblock, bytesrd);

    if (bytesrd == BLOCK_SIZE)
      {
      aesAlgorithm(AesInblock, AesOutblock, _mode);
      outfile.write(AesOutblock, bytesrd);
      }

    //
    // CIPHERTEXT STEALING:
    // We do not want to change filesizes, so we dont use padding.
    // The following special handling of the last block implements
    // "Ciphertext Stealing" if the last block is less than BLOCK_SIZE.
    // For the last 2 blocks: Aes(Pn-1) = Cn||C' and Aes(Pn||C') = Cn-1
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
    //  before: AesLastblock[] = 00 00 00 00 00 00 00 00 EF 29 7C 97 61 00 00 00
    //          AesInblock[]   = 0A 0D 0A 0D 0A 5B 80 9E
    //
    //  after:  AesLastblock[] = 91 03 D1 32 FA 54 C2 17 EF 29 7C 97 61
    //
    else
      {
      for (i = 0; i < bytesrd; i++) AesLastblock[BLOCK_SIZE + i] = AesOutblock[i];
      for (i = bytesrd; i < BLOCK_SIZE; i++) AesInblock[i] = AesOutblock[i];

      aesAlgorithm (AesInblock, AesLastblock, _mode); //@Am0001
      outfile.seekp(0, ios::end);               // seek to the end of the file
      outfile.seekp(-BLOCK_SIZE, ios::cur);     // back up 16 bytes
      outfile.write(AesLastblock, bytesrd + BLOCK_SIZE);
      }

//      pos = infile.tellg();
//      cout << "infile: The file pointer is now at location " << pos << endl;
//      pos = outfile.tellp();
//      cout << "outfile: The file pointer is now at location " << pos << ;

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end while

  } // AesDoAlgorithmStealECB


//-----------------------------------------------------------------------------
//
//                              AesDoAlgorithmIsoECB
//
//  ENCRYPT/DECIPHER - Electronic Code Book (ISO Padding)
//
void AesDoAlgorithmIsoECB(ifstream &infile, ofstream &outfile, int _mode)
  {
  int isoPad = 0; j = 0;

  li = COUNT_RATE;  ls = srcFileSize;

  bytesrd = BLOCK_SIZE; // init
  while (ln < ls)
    {

    if ((ln+BLOCK_SIZE) > ls) bytesrd = (int)(ls % BLOCK_SIZE);
    ln += (ULONG)bytesrd;

    infile.read(AesInblock, bytesrd);

    if (bytesrd == BLOCK_SIZE)
      {
      aesAlgorithm(AesInblock, AesOutblock, _mode);

      if (mode == AES_ECBE) outfile.write(AesOutblock, BLOCK_SIZE); // Write all 16-byte blocks
      else if ((mode == AES_ECBD) && (ln != ls))
        {
        outfile.write(AesOutblock, BLOCK_SIZE);  // Write all 16-byte blocks, except the last block
        }

      else if ((mode == AES_ECBD) && (ln == ls)) // Last block requires special handling
        {                                        // Either a whole block of padding
        j=BLOCK_SIZE;                            //  or a partly padded block
                                                 
        for (i=0; i<BLOCK_SIZE; i++)
          {
          j--;
          if (AesOutblock[j] == 0xFFFFFF80)      // ISOPAD rendered as long int ????
            {                                 
            break;
            }
          }

        outfile.write(AesOutblock, j);           // Write until ISOPAD
        ln += ((ULONG)j - BLOCK_SIZE);           // Adjust (decipher) filesize count
        break;                                   // Stop while loop if ISOPAD
        }
      }

    //
    // ISO PADDING:
    // Using ISO padding we always increase the filesize.
    // The following handling of the last block implements ISO Padding.
    // For the last block: Aes(Pn) = Cn||PB
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
      AesInblock[bytesrd] = ISOPAD;
      for (i=bytesrd+1; i<BLOCK_SIZE; i++) AesInblock[i] = PAD;

      aesAlgorithm (AesInblock, AesOutblock, _mode);

      outfile.write(AesOutblock, BLOCK_SIZE);
      isoPad = 1;
      ln += BLOCK_SIZE-bytesrd;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end while

  if ((mode == AES_ECBE) && (isoPad == 0))   // Need to add a whole padding block
    {                                        // Does not apply to encrypted text
    AesInblock[0]=ISOPAD;                    //  which is always padded MOD(16)
    for (i=1; i<BLOCK_SIZE; i++) AesInblock[i] = PAD;

    aesAlgorithm (AesInblock, AesOutblock, _mode);

    outfile.write(AesOutblock, BLOCK_SIZE);
    ln += BLOCK_SIZE;                        // Adjust (decipher) filesize count
    }
  } // AesDoAlgorithmIsoECB


//------------------------------------------------------------------------------
//
//                              AesDoAlgorithmStealCBCE
//
//  ENCRYPT - Cipher Block Chaining (Ciphertext Stealing)
//               
void AesDoAlgorithmStealCBCE(ifstream &infile, ofstream &outfile)
  {
  int ciphStealing = FALSE;

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  do
    {
    if (ciphStealing == TRUE) break;                   // ciphStealing -break

    if ((lj - ln) >= BLOCK_SIZE) bytesrd = BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = lj - ln;                            //  ifstream won't tell us   
    infile.read(AesInblock, bytesrd);                  // Read from input file

    if (bytesrd >= BLOCK_SIZE)
      for (i = 0; i < BLOCK_SIZE; i++) AesInblock[i] ^= AesIcvblock[i]; // CBC: inbuf XOR ICV

    else if (bytesrd < BLOCK_SIZE)                     // Ciphertext stealing
      {
      for (i = 0; i < BLOCK_SIZE-bytesrd; i++) AesInblock[i+bytesrd] = PAD; // Pn* = Pn||0s (zero-padded) 
      for (i = 0; i < BLOCK_SIZE; i++) AesInblock[i] ^= AesOutblock[i];     // Pn* XOR Cn-1*
      for (i = 0; i < BLOCK_SIZE; i++) AesOutbuf1[i] = AesOutblock[i];      // save Cn-1

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
      //          Pn* ^ Cn-1   = FF A1 D7 B3 55 21 C0 2B  (AesInblock to be encrypted
      //          Cn-1(new)    = 72 F6 82 BA DA D8 88 91    yields a new previous Block)
      //          Cn-1**       = F5 AC DD BE 5F           (AesLastblock < BLOCK_SIZE)
      //
      //  before: AesLastblock[] = 00 00 00 00 00 00 00 00 F5 AC DD BE 5F 00 00 00
      //          AesInblock[]   = FF A1 D7 B3 55 21 C0 2B
      //
      //  after:  AesLastblock[] = 72 F6 82 BA DA D8 88 91 F5 AC DD BE 5F [Cn-1(new) || Cn-1**]
      //
      ciphStealing = TRUE;
      } // end else if

    // ---------------------------------------------
    // Performing the AES (i.e., Standard Algorithm)
    // ---------------------------------------------
    aesAlgorithm(AesInblock, AesOutblock, ENCRYPT);                               // 1st step
    if (ciphStealing == TRUE)
      {
      // Build the last block(s) [Cn-1(new) || Cn-1**],
      //  where Cn-1(new) consists of the encrypted incomplete block of Pn
      //  and the stolen chunk Cn-1* which has been encrypted twice.
      //
      for (i = 0; i < BLOCK_SIZE; i++) AesLastblock[i] = AesOutblock[i];          // Cn-1(new)
      for (i = 0; i < bytesrd; i++) AesLastblock[BLOCK_SIZE + i] = AesOutbuf1[i]; // Cn-1**
      }

    ln += (ULONG)bytesrd;                            // Update counter total bytes read

    // -----------------------------------------
    // Special processing after applying the AES
    // -----------------------------------------
    if (ciphStealing == FALSE)
      {
      outfile.write(AesOutblock, bytesrd);                          // Write Ci..Cn
      for (i=0; i<BLOCK_SIZE; i++) AesIcvblock[i] = AesOutblock[i]; // Update ICV
      }
    else if (ciphStealing == TRUE)                     
      {
      outfile.seekp(0, ios::end);                        // seek to end of the file
      outfile.seekp(-BLOCK_SIZE, ios::cur);              // back up 8 bytes
      outfile.write(AesLastblock, bytesrd + BLOCK_SIZE); // Write [Cn-1 || Cn]
      break;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end do while
  while (ln < lj);
  } // AesDoAlgorithmStealCBCE

//------------------------------------------------------------------------------
//
//                              AesDoAlgorithmStealCBCD
//
//  DECIPHER - Cipher Block Chaining (Ciphertext Stealing)
//
void AesDoAlgorithmStealCBCD(ifstream &infile, ofstream &outfile)
  {
  int ciphStealing = FALSE;

  for (i=0; i<BLOCK_SIZE; i++) AesInbuf2[i] = 0;   // IMPORTANT: Init-clear Cn-2 block

  li = COUNT_RATE; ln = 0; lj = srcFileSize;
  do
    {
    if (ciphStealing == TRUE) break;                   // ciphStealing - break

    if ((lj - ln) >= BLOCK_SIZE) bytesrd = BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = lj - ln;                            //  ifstream won't tell us   
    infile.read(AesInblock, bytesrd);                  // Read from input file

    if ((lj-ln) > 2*BLOCK_SIZE && (lj % BLOCK_SIZE) != 0) 
      for (i=0; i<BLOCK_SIZE; i++) AesInbuf2[i] = AesInblock[i]; // CBC save Cn-2 block

    if (bytesrd == BLOCK_SIZE)
      for (i=0; i<BLOCK_SIZE; i++) AesInbuf1[i] = AesInblock[i]; // CBC save 1st block

    else if (bytesrd < BLOCK_SIZE)
      {
      // CIPHERTEXT STEALING CBC DECIPHER:
      // We do not want to change filesizes, so we dont use padding.
      // The following special handling of the last block implements
      // "Ciphertext Stealing" if the last block is less than BLOCK_SIZE.
      // For the last 2 blocks: Aes(Pn-1) = Cn||C' and Aes(Pn||C') = Cn-1
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
      //          Pn = (Pn* ^ Cn-1) ^ Cn-1 = 0A 0D 0A 0D 0A           (Pn AesLastblock deciphered)
      //
      //  before: AesLastblock[] = F5 AC DD BE 5F 21 C0 2B F5 AC DD BE 5F 21 C0 2B
      //  after:  AesOutblock[]  = 0A 0D 0A 0D 0A 0D 0A 0D                           (Decipher)
      //
      //  before: AesLastblock[] = FF A1 D7 B3 55 21 C0 2B F5 AC DD BE 5F 21 C0 2B
      //  after:  AesLastblock[] = 0A 0D 0A 0D 0A 00 00 00 F5 AC DD BE 5F 21 C0 2B   (XOR)
      //
      for (i = 0; i < BLOCK_SIZE; i++) AesLastblock[i] = AesLastblock[BLOCK_SIZE + i];
      for (i = 0; i < bytesrd; i++)    AesLastblock[i] = AesInblock[i];

      aesAlgorithm(AesLastblock, AesOutblock, DECIPHER);

      for (i = 0; i < BLOCK_SIZE; i++) AesOutblock[i]  ^= AesInbuf2[i];
      for (i = 0; i < BLOCK_SIZE; i++) AesLastblock[i] ^= AesLastblock[BLOCK_SIZE + i];

      for (i = 0; i < BLOCK_SIZE; i++) AesLastblock[i+BLOCK_SIZE] = AesLastblock[i];  // swap Pn-1
      for (i = 0; i < BLOCK_SIZE; i++) AesLastblock[i] = AesOutblock[i];              // concatenate Pn chunk

      ciphStealing = TRUE;
      } // end else if                                                                

    // ---------------------------------------------
    // Performing the AES (i.e., Standard Algorithm)
    // ---------------------------------------------
    if (ciphStealing == FALSE)
      {
      aesAlgorithm(AesInblock, AesOutblock, DECIPHER);
      if (bytesrd == BLOCK_SIZE && ciphStealing == FALSE)
        for (i=0; i<BLOCK_SIZE; i++) AesInblock[i] = AesOutblock[i];
      }

    ln += (ULONG)bytesrd;                             // Update counter total bytes read
    
    // ------------------------------------------------
    // Special processing after applying the TDES steps
    // ------------------------------------------------
    if (ciphStealing == FALSE)
      {
      for (i=0; i<BLOCK_SIZE; i++) AesLastblock[BLOCK_SIZE+i] = AesOutblock[i];  // Save Cn-1
      for (i=0; i<BLOCK_SIZE; i++) AesOutblock[i] ^= AesIcvblock[i]; // CBC specific XOR function
      for (i=0; i<BLOCK_SIZE; i++) AesIcvblock[i]  = AesInbuf1[i];   // CBC copy 1st block
      outfile.write(AesOutblock, bytesrd);
      }
    else
      {
      outfile.seekp(0, ios::end);                        // seek to end of the file
      outfile.seekp(-BLOCK_SIZE, ios::cur);              // back up 8 bytes
      outfile.write(AesLastblock, bytesrd + BLOCK_SIZE); // Write [Pn-1 || Pn]
      break;
      }

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);  // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    } // end do while
  while (ln < lj);
  } // AesDoAlgorithmStealCBCD


//------------------------------------------------------------------------------
//
//                              AesDoAlgorithmIsoCBCE
//
//  ENCRYPT - Cipher Block Chaining (ISO Padding)
//
void AesDoAlgorithmIsoCBCE(ifstream &infile, ofstream &outfile)
  {
  li = COUNT_RATE;  ls = srcFileSize; ln = 0;
  int isoPad = PAD;

  //
  // CBC specific initial chaining vector init function
  //
  for (i=0; i<BLOCK_SIZE; i++) AesOutblock[i] = 0;       // CBC Init ICV

  bytesrd = BLOCK_SIZE; // init
  while (ln < ls)
    {
    if ((ln+BLOCK_SIZE) > ls) bytesrd = (int)(ls % BLOCK_SIZE);
    ln += (ULONG)bytesrd;

    infile.read(AesInblock, bytesrd);

    if (bytesrd == BLOCK_SIZE)
      {
      //
      // CBC specific XOR function
      //
      for (i=0; i<BLOCK_SIZE; i++) AesInblock[i] ^= AesIcvblock[i]; // CBC; inbuf XOR ICV

      aesAlgorithm(AesInblock, AesOutblock, ENCRYPT);
      outfile.write(AesOutblock, bytesrd);

        for (i=0; i<BLOCK_SIZE; i++) AesIcvblock[i] = AesOutblock[i]; // Update ICV
     }

    //
    // ISO PADDING:
    // Using ISO padding we always increase the filesize.
    // The following handling of the last block implements ISO Padding.
    // For the last block: Aes(Pn) = Cn||PB
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
      AesInblock[bytesrd] = ISOPAD;
      for (i=bytesrd+1; i<BLOCK_SIZE; i++) AesInblock[i] = PAD;

      //
      // CBC specific XOR function
      //
      for (i=0; i<BLOCK_SIZE; i++) AesInblock[i] ^= AesOutblock[i]; // CBC XOR

      aesAlgorithm (AesInblock, AesOutblock, ENCRYPT);
      outfile.write(AesOutblock, BLOCK_SIZE);
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
    AesInblock[0]=ISOPAD;
    for (i=1; i<BLOCK_SIZE; i++) AesInblock[i] = PAD;

    //
    // CBC specific XOR function
    //
    for (i=0; i<BLOCK_SIZE; i++) AesInblock[i] ^= AesOutblock[i]; // CBC XOR

    aesAlgorithm (AesInblock, AesOutblock, ENCRYPT);
    outfile.write(AesOutblock, BLOCK_SIZE);
    ln += BLOCK_SIZE;                      // Adjust filesize count
    }
  } // AesDoAlgorithmIsoCBCE

//-----------------------------------------------------------------------------
//
//                              AesDoAlgorithmIsoCBCD
//
//  DECIPHER - Cipher Block Chaining (ISO Padding)
//
void AesDoAlgorithmIsoCBCD(ifstream &infile, ofstream &outfile)
  {
  li = COUNT_RATE;  ls = srcFileSize;

  //
  // CBC specific initial chaining vector init function
  //
  bytesrd = BLOCK_SIZE; // init
  do
    {
    infile.read(AesInblock, bytesrd);
    ln += (ULONG)bytesrd;

    aesAlgorithm(AesInblock, AesOutblock, DECIPHER);
    //
    // CBC specific XOR function
    //
    for (i=0; i<BLOCK_SIZE; i++) AesOutblock[i] ^= AesIcvblock[i]; // CBC XOR
    for (i=0; i<BLOCK_SIZE; i++) AesIcvblock[i] = AesInblock[i];   // CBC copy

    //
    // Encrypt and decipher modes must be handled differently.
    // Encrypt: Since "bytesrd==BLOCK_SIZE" we just write AesOutblock to dstfile.
    // Decipher: Since ISO padding was applied to the enrypted file, it is
    //           guaranteed that the srcFileSize is a multiple of BLOCK_SIZE.
    //           However, we should remove the ISO padding from the deciphered
    //           plaintext, which is done here.
    //
    if (ln != srcFileSize) outfile.write(AesOutblock, BLOCK_SIZE);

    else                      // (ln == srcFileSize)
      {                       // Remove ISO padding from plaintext before write
      j = BLOCK_SIZE;         // Assume a whole block of padding
      for (i=0; i<BLOCK_SIZE; i++)
        {
        j--;
        if (AesOutblock[j] == 0xFFFFFF80) // Stop at ISOPAD (rendered as long int ????)
          {
          break;
          }
        }

        outfile.write(AesOutblock, j);    // Write until ISOPAD
        break;                            // Stop while loop if ISOPAD
      }                                   // (discard ISOPAD)

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);     // Echo per COUNT_RATE
      li += COUNT_RATE;
      }
    }
  while (ln < ls);  // end do while

  ln -= bytesrd; ln += (ULONG)j;  // Adjust (decipher) filesize count
  } // AesDoAlgorithmIsoCBCD


//----------------------------------------------------------------------------
//
//                        AesDoAlgorithmMac
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
#define AES_BLOCK_SIZE 16
 
void AesDoAlgorithmMac(ifstream &infile, ofstream &outfile)
  {
  int msbFlag;

  UCHAR pszRB128[AES_BLOCK_SIZE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x87};
  char pszZeroBlock[AES_BLOCK_SIZE];
  char pszSubkeyK1[AES_BLOCK_SIZE+1]; // Adding one 0-byte for ROL operation
  char pszSubkeyK2[AES_BLOCK_SIZE];

  // Generating the subkeys K1 and K2
  //  
  //  Example:
  //  Block cipher is the AES algorithm with the following 128 bit key K:
  //    K =           2b7e1516 28aed2a6 abf71588 09cf4f3c
  //
  //  Subkey K1, K2 Generation
  //    CIPHK(0128) = 7df76b0c 1ab899b3 3e42f047 b91b546f
  //    K1 =          fbeed618 35713366 7c85e08f 7236a8de
  //    K2 =          f7ddac30 6ae266cc f90bc11e e46d513b
  
  //
  //  Example Mlen = 128:
  //    M =           6bc1bee2 2e409f96 e93d7e11 7393172a
  //    T =           070a16b4 6b4d4144 f79bdd9d d04a287c
  //
  //    T(Key192) =   9e99a7bf 31e71090 0662f65e 617c5184
  //
  //    T(Key256) =   28a7023f 452e8f82 bd4bf28d 8c37c35c
  //
  //  Example Mlen = 320:
  //    M =           6bc1bee2 2e409f96 e93d7e11 7393172a
  //                  ae2d8a57 1e03ac9c 9eb76fac 45af8e51
  //                  30c81c46 a35ce411
  //    T =           dfa66747 de9ae630 30ca3261 1497c827
  // 
  //    T(Key192) =   8a1de5be 2eb31aad 089a82e6 ee908b0e
  //
  //    T(Key256) =   aaf3d8f1 de5640c2 32f5b169 b9c911e6
  //
  //  Example Mlen = 512:
  //    M =           6bc1bee2 2e409f96 e93d7e11 7393172a
  //                  ae2d8a57 1e03ac9c 9eb76fac 45af8e51
  //                  30c81c46 a35ce411 e5fbc119 1a0a52ef
  //                  f69f2445 df4f9b17 ad2b417b e66c3710
  //    T =           51f0bebf 7e3b9d92 fc497417 79363cfe
  // 
  //    T(Key192) =   a1d5df0e ed790f79 4d775896 59f39a11
  //
  //    T(Key256) =   e1992190 549f6ed5 696a2c05 6c315410

  pszSubkeyK1[AES_BLOCK_SIZE+0] = 0;   // Ensure K1 last shifted bit = 0

  for (i=0; i<AES_BLOCK_SIZE; i++)
    pszZeroBlock[i] = AesIcvblock[i];  // Init Zero - block

  // K1
  aesAlgorithm(pszZeroBlock, pszSubkeyK1, ENCRYPT);

  msbFlag = (UCHAR)pszSubkeyK1[0] & 0x80; // Set flag for XOR K1 later

  for (i=0; i<AES_BLOCK_SIZE; i++)
    {
    pszSubkeyK1[i] = pszSubkeyK1[i] << 1;
    pszSubkeyK1[i] = pszSubkeyK1[i] | ((pszSubkeyK1[i+1] & 0x80) >> 7);
    }                                                                             

  if (msbFlag != 0)
    {
    for (i=0; i<AES_BLOCK_SIZE; i++) pszSubkeyK1[i] ^= pszRB128[i];
    }

  // K2
  for (i=0; i<AES_BLOCK_SIZE; i++)
    pszSubkeyK2[i] = (pszSubkeyK1[i] << 1) | ((pszSubkeyK1[i+1] & 0x80) >> 7);
  
  msbFlag = (UCHAR)pszSubkeyK1[0] & 0x80; // Set flag for XOR K1 later
  if (msbFlag != 0)
    {
    for (i=0; i<AES_BLOCK_SIZE; i++) pszSubkeyK2[i] ^= pszRB128[i];
    }

  // Prepare the last block of AES_BLOCK_SIZE in inblock:
  DWORD dwCryptFileSize = srcFileSize;
    
  // CMAC 
  ln = 0; ls = dwCryptFileSize;    // init counters
  bytesrd = AES_BLOCK_SIZE;        // init bytesrd, filesize is at least 8 bytes
  while (ln < ls)
    {
    if ((ls - ln) >= AES_BLOCK_SIZE) bytesrd = AES_BLOCK_SIZE; // Keep track of bytesrd,    
    else bytesrd = ls - ln;                                    //  ifstream won't tell us   

    infile.read(AesInblock, bytesrd);

    // K1) Message length is a positive multiple of the block size
    if ((ls - ln) == AES_BLOCK_SIZE)
      {
      for (i=0; i<AES_BLOCK_SIZE; i++) AesInblock[i] ^= pszSubkeyK1[i];
      }

    // K2) Message length is not a positive multiple of the block size  
    else if ((ls - ln) < AES_BLOCK_SIZE)
      {
      for (i=(int)(dwCryptFileSize % AES_BLOCK_SIZE); i<AES_BLOCK_SIZE; i++)
        {
        // CMAC Padding starts with 10000000b and continues with all bits zeroed
        if (i == (int)(dwCryptFileSize % AES_BLOCK_SIZE)) AesInblock[i] = ISOPAD;      
        else AesInblock[i] = PAD;
        }
      for (i=0; i<AES_BLOCK_SIZE; i++) AesInblock[i] ^= pszSubkeyK2[i];
      }
   
    for (i=0; i<AES_BLOCK_SIZE; i++) AesInblock[i] ^= AesIcvblock[i]; // CBC: inbuf XOR  ICV
    aesAlgorithm(AesInblock, AesOutblock, ENCRYPT);
    for (i=0; i<AES_BLOCK_SIZE; i++) AesIcvblock[i] = AesOutblock[i]; // Update ICV w/ next block

    if (ln / li)
      {
      printf("%lu KB\r", ln / 1024L);          // Echo per COUNT_RATE
      li += COUNT_RATE;
      }

    ln += (ULONG)bytesrd;                      // Update counter total bytes read
    } // end while

  outfile.write(AesOutblock, AES_BLOCK_SIZE);  // Emit the MAC to file
  } // AesDoAlgorithmMac


//----------------------------------------------------------------------------
//
//                                AesDisplayIcvblock
//
void AesDisplayIcvblock()
  {
  printf("iv-block: ");
  for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)AesIcvblock[i]);
  printf("\n");
  } // AesDisplayIcvblock

//-----------------------------------------------------------------------------
//
//                              AesDisplayHelp
//
void AesDisplayHelp()                                               
  {
  printf(AesSignon);  // Display AesSignon message

  printf("Performs encryption and decryption using the Advanced Encryption Standard.\n\n");
  
  printf("Usage: '%s srcfile destfile [keyfile | /keystring] [options] [ivfile]",
                  (hedit_exe == FALSE) ? "AES" : "HEDIT");
  printf("%s'\n", (hedit_exe == FALSE) ? "" : " /AES");                                
  printf("  srcfile    Input file (plain text or encrypted text >= 16 bytes).\n"
         "  destfile   Output file (after the algorithm has been applied).\n"
         "  ivfile     Input iv-file (Init Vector, optional for CBC modes).\n\n");

  printf("  keyfile    Input file containing the secret key.\n"
         "             The key can be 32 bytes max (keysize = 128,192,256 bits).\n"
         "             Short keys will be zero-expanded.\n"
         "  /keystring To avoid a keyfile the key may be directly given as\n"
         "             a string of up to 32 ascii characters: e.g. /1234567890...\n");
  printf("[options]\n");
  printf("  /ENCRYPT   Encrypts a file. The plaintext is AES encrypted.\n"
         "             Mode: CBC with ciphertext stealing.\n\n");

  printf("  /DECIPHER  Deciphers an encrypted file. The AES ciphertext is converted\n"
         "             into plaintext. Mode: CBC with ciphertext stealing.\n\n");

  printf("  /MAC       A Message Authentication Code (MAC) is calculated from srcfile.\n"
         "             The cryptographic signature is written to destfile, that can be\n"
         "             appended to the plaintext as a cryptographic signature.\n"
         "             Mode: (CMAC NIST SP 800-38B).\n");
  More();             // Press any key to continue
  
  printf("  /ECBENCRYPT  Encrypts a file. The plaintext is AES encrypted.\n"
         "               Mode: ECB with ciphertext stealing.\n\n");

  printf("  /ECBDECIPHER Deciphers an encrypted file. The AES ciphertext is converted\n"
         "               into plaintext. Mode: ECB with ciphertext stealing.\n\n");

  printf("  /CBCE     Encrypts a file. Mode: CBC with ISO/IEC 7816-4 padding.\n\n");

  printf("  /CBCD     Deciphers an encrypted file. Mode: CBC with ISO padding.\n\n");

  printf("  /ECBE     Encrypts a file. Mode: ECB with ISO/IEC 7816-4 padding.\n\n");

  printf("  /ECBD     Deciphers an encrypted file. Mode: ECB with ISO padding.\n\n");

  printf("This utility is very fast! When encrypting files, always be careful\n"
         "about keeping your keys privately at a secure place.\n"
         "Never send an encrypted file and its secret key through the same channel.\n"
         "For example, if you sent the encrypted file and this utility via e-mail\n"
         " to a certain person, you should communicate the secret key via\n"
         " telephone or surface mail, addressing the entitled person.\n");
  if (hedit_exe == TRUE)
    {
    printf("\nNOTE: 'copy hedit.exe aes.exe' to build a crypto utility for AES only.\n\n");
    } 

  AnyKey();           // Press any key for exit
  } // AesDisplayHelp

//----------------------------------------------------------------------------
//
//                        main: AesCheckRunByExplorer
//
// This console program could be run by typing its name at the command prompt,
// or it could be run by the user double-clicking it from Explorer.
// And you want to know which case you’re in.
//
void AesCheckRunByExplorer()
  {
  // Check if invoked via Desktop
  DWORD procId = GetCurrentProcessId();
  DWORD count = GetConsoleProcessList(&procId, 1);

  if (count < 2)       // Invoked via Desktop
    {
    AesDisplayHelp();
    printf("\nConsole application: AES.EXE\n");
    system("cmd");     // Keep the Console window open
    exit(0);           // (user may recoursively start the console app)
    }

  // When invoked via Console (nothing to do)
  } // AesCheckRunByExplorer

//-----------------------------------------------------------------------------
//
//                              AesMain
//
int AesMain(int aes_argc, char **aes_argv) //@0003 Interfacing HEDIT
  {
  int i;
  struct stat _stat;

  for (i=0; i<BLOCK_SIZE; i++) AesIcvblock[i] = 0x00;  // CBC Clear-Init ICV
  if (aes_argc == 6)                                    // ICV file is present
    {
    aes_argc--;
    stat(aes_argv[5], &_stat);                          // Key provided by file
    if (_stat.st_size > BLOCK_SIZE)
      {
      printf(error_fsize, aes_argv[5], BLOCK_SIZE);     // File length error, or non-existance
      exit(1);
      }
    //
    // Read the key and initialize the AES ICV for CBCE /CBCD modes.
    //
    ifstream Icvfile(aes_argv[5], ios::binary | ios::in); //Open input binary file
    if (!Icvfile)
      {
      printf(open_failed, aes_argv[5]);
      exit(1);
      }
    Icvfile.read(AesIcvblock, _stat.st_size);  // Copy the icv from file and display it
//ha//    printf("iv-block: ");
//ha//    for (i=0; i<BLOCK_SIZE; i++) printf("%02X ", (UCHAR)AesIcvblock[i]);
//ha//    printf("\n");
    } // ReadFileIcv
                                                                               
  if (aes_argc < 5)                        // Illegal parameter
    {
    AesDisplayHelp();                      // Illegal parameter
    exit(1);
    }      
  else if (_stricmp(aes_argv[4], "/DECIPHER") == 0) mode = AES_DECIPHER;
  else if (_stricmp(aes_argv[4], "/ENCRYPT") == 0) mode = AES_ENCRYPT;
  else if (_stricmp(aes_argv[4], "/MAC") == 0) mode = AES_MAC;
  else if (_stricmp(aes_argv[4], "/CBCD") == 0) mode = AES_CBCD;
  else if (_stricmp(aes_argv[4], "/CBCE") == 0) mode = AES_CBCE;
  else if (_stricmp(aes_argv[4], "/ECBD") == 0) mode = AES_ECBD;
  else if (_stricmp(aes_argv[4], "/ECBE") == 0) mode = AES_ECBE;
  else if (_stricmp(aes_argv[4], "/ECBDECIPHER") == 0) mode = AES_ECBDECIPHER;
  else if (_stricmp(aes_argv[4], "/ECBENCRYPT") == 0) mode = AES_ECBENCRYPT;
  else
    {                                                                  
    AesDisplayHelp();                      // Illegal parameter
    exit(1);
    }
                                                        
  //
  // Determine the key mode
  //
  if (strncmp(&aes_argv[3][0], "/", 1) == 0)    // Key via command line
    {
    if (strlen(&aes_argv[3][1]) > 2*KEY_LENGTH)                 
      {
      printf("ERROR: AES-KEYSIZE > %d!\n", 2*KEY_LENGTH);
      exit(1);
      }
    for (i=0; (UINT)i<strlen(&aes_argv[3][1]); i++)    // Copy the key from command line
      {
      if ((UINT)i >= strlen(&aes_argv[3][1])) break;   // Pad all short keys
      AesKeybuf[i] = (aes_argv[3][i+1] & 0xFF);
      // WIN10: **argv will return wrong chars if typed
      //  on keypad (eg. Alt+"1 2 8" thru "2 5 5"
      if (AesKeybuf[i] == 0xFFFFFFA0) AesKeybuf[i] = 0xFF; 
      }                                                    

    // Determine the _keylength
    // ( 8-1) / 8 = 0 R 7  _keylength = 128bits
    // (16-1) / 8 = 1 R 7  _keylength = 128bits = 2*64 bits
    // (24-1) / 8 = 2 R 7  _keylength = 192bits = 3*64 bits
    // (32-1) / 8 = 3 R 7  _keylength = 256bits = 4*64 bits
    (strlen(&aes_argv[3][1]) - 1) / 8 ? _keylength = (((strlen(&aes_argv[3][1])-1) / 8)+1) * 64 : _keylength = 128;  
    } // end if

  else
    {
    stat(aes_argv[3], &_stat);                          // Key provided by file
    if ((UCHAR)_stat.st_size > 2*KEY_LENGTH)
      {
      printf(error_keysize, aes_argv[3], 2*KEY_LENGTH);
      exit(1);
      }

    //
    // Read the key and initialize the AES key schedule.
    //
    ifstream keyfile(aes_argv[3], ios::binary | ios::in); //Open input binary file
    if (!keyfile)
      {
      printf(open_failed, aes_argv[3]);
      exit(1);
      }
    keyfile.read(AesKeybuf, _stat.st_size);  // Copy the key from file

    // Determine the _keylength
    // ( 8-1) / 8 = 0 R 7  _keylength = 128bits 
    // (16-1) / 8 = 1 R 7  _keylength = 128bits = 2*64 bits
    // (24-1) / 8 = 2 R 7  _keylength = 192bits = 3*64 bits
    // (32-1) / 8 = 3 R 7  _keylength = 256bits = 4*64 bits
    (_stat.st_size - 1) / 8 ? _keylength = (((_stat.st_size - 1) / 8)+1) * 64 : _keylength = 128;  
    } // end else

  // ---------------------------------
  // Open source and destination files
  // ---------------------------------
  stat(aes_argv[1], &_stat);                      // Check source file size
  srcFileSize = _stat.st_size;                    // Init source file size

  if (srcFileSize < BLOCK_SIZE)                   // Must be at least one block
    {
    printf(error_fsize, aes_argv[1], BLOCK_SIZE); // File length error, or non-existance
    exit(1);
    }

  ifstream infile(aes_argv[1], ios::binary | ios::in);  //Open input binary file
  if (!infile)
    {
    printf(open_failed, aes_argv[1]);
    exit(1);
    }
  
  if (_access(aes_argv[2], 0) == 0)     // Check if outfile already exists
    {                               
    printf(file_exists, aes_argv[2]);   
    exit(1);
    }                               

  ofstream outfile(aes_argv[2], ios::binary | ios::out);  //Open output binary file
  if (!outfile)
    {
    printf(open_failed, aes_argv[2]);
    exit(1);
    }

  printf(AesSignon);                   // Display AesSignon message

  // -------------------------
  // Perform the AES algorithm
  // -------------------------
  ln = 0L; li = COUNT_RATE;
  switch(mode)
    {
    case AES_ENCRYPT:
      AesDisplayIcvblock();
      aesKeyInit(AesKeybuf, _keylength, ENCRYPT);
      AesDoAlgorithmStealCBCE(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case AES_DECIPHER:
      AesDisplayIcvblock();
      aesKeyInit(AesKeybuf, _keylength, DECIPHER);
      AesDoAlgorithmStealCBCD(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case AES_ECBENCRYPT:
      aesKeyInit(AesKeybuf, _keylength, ENCRYPT);
      AesDoAlgorithmStealECB(infile, outfile, ENCRYPT);
      printf(bytes_encrypted, ln);
      break;

    case AES_ECBDECIPHER:
      aesKeyInit(AesKeybuf, _keylength, DECIPHER);
      AesDoAlgorithmStealECB(infile, outfile, DECIPHER);
      printf(bytes_deciphered, ln);
      break;

    case AES_CBCE:
      AesDisplayIcvblock();
      aesKeyInit(AesKeybuf, _keylength, ENCRYPT);
      AesDoAlgorithmIsoCBCE(infile, outfile);
      printf(bytes_encrypted, ln);
      break;

    case AES_CBCD:
      AesDisplayIcvblock();
      aesKeyInit(AesKeybuf, _keylength, DECIPHER);
      AesDoAlgorithmIsoCBCD(infile, outfile);
      printf(bytes_deciphered, ln);
      break;

    case AES_ECBE:
      aesKeyInit(AesKeybuf, _keylength, ENCRYPT);
      AesDoAlgorithmIsoECB(infile, outfile, ENCRYPT);
      printf(bytes_encrypted, ln);
      break;

    case AES_ECBD:
      aesKeyInit(AesKeybuf, _keylength, DECIPHER);
      AesDoAlgorithmIsoECB(infile, outfile, DECIPHER);
      printf(bytes_deciphered, ln);
      break;

    case AES_MAC:
      AesDisplayIcvblock();
      aesKeyInit(AesKeybuf, _keylength, ENCRYPT);
      AesDoAlgorithmMac(infile, outfile);
      printf("%lu bytes processed. MAC = [", ln);       // Display the MAC
      for (i=0; i<BLOCK_SIZE; i++) printf("%02X", (UCHAR)AesOutblock[i]);
      printf("]\n%s: %d Bytes have been written.\n", aes_argv[2], BLOCK_SIZE);
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
  } // AesMain

//-------------------------- end of main module -------------------------------
