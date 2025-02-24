// des.cpp - C++ Developer source file.

//*************************************************************************
// des.c
//
// Software Data Encryption Standard Implementation
// Version 2.4, February 25, 1985
//
// Permission is hereby given for non-commercial use.
//
// Copyright (c) 1977, 1984, 1985
// By Jim Gillogly, Lauren Weinstein, and Richard Outerbridge
//
//*************************************************************************

// Further development (2021 ha):
// Refurbished 2021, using Visual Studio 2010 (XP) and VS C++ 2019 (Windows 10)
// and transscribed into C++ language by helmut altmann.
//
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

#define DE  1

#define UCHAR unsigned char

//typedef char  BYTE; // BYTE = (vax) ? int : char

void p48init(),  desPermute(char *, int *, int *, char *);   //desInit(),

static char s12[4096], s34[4096], s56[4096], s78[4096];
long  p48a[256][2], p48b[256][2], p48c[256][2], p48d[256][2];

static long kn[32];

static int pmv[8] = { 64, 16,  4,  1, 128, 32,  8,   2};

static int pvs[8] = {  1,  2,  4,  8,  16, 32, 64, 128};

//------------------------------------------------------------------------
//
//                         desAlgorithm
//
void desAlgorithm(char *inblock, char *outblock)
  {
  register long *fmp, suba, subb, val, *h0L, *h1R, *keys;
  char _scratch[8];
  char *_small;       // Must be an L-Value (pointer to any address later)

  long swap[4];
  int  i;

  desPermute(inblock, pmv, pvs, _scratch);

  h0L = swap;
  h1R = &swap[4];
  _small = _scratch;

  while (h0L < h1R)
    {
    val  = ((*_small++) & 0377L) << 24;
    val |= ((*_small++) & 0377L) << 16;
    val |= ((*_small++) & 0377L) << 8;
    val |= (*_small++)  & 0377L;

    *h0L++ = ((val & 0x1L) << 23) | ((val >> 9) & 0x7C0000L) |
             ((val & 0x1F800000L) >> 11) | 
             ((val & 0x1F80000L)  >> 13) |
             ((val & 0x1F8000L)   >> 15);
    *h0L++ = ((val & 0x1F800L)    <<  7) |
             ((val & 0x1F80L)     <<  5) |
             ((val & 0x1F8L)      <<  3) |
             ((val & 0x1FL)       <<  1) |
             ((val >> 31) & 0x1L);
    }

  keys = kn;
  h0L = swap;
  h1R = &swap[2];

  for (i = 0; i < 16; i++)
    {
    val = *keys++ ^ *h1R++;
    fmp = &p48a[ s12[(val >> 12)] & 0377 ][0];
    suba = *fmp++;
    subb = *fmp;
    fmp = &p48b[ s34[(val & 07777L)] & 0377 ][0];
    suba |= *fmp++;
    subb |= *fmp;
    val = *keys++ ^ *h1R++;
    fmp = &p48c[ s56[(val >> 12)] & 0377 ][0];
    suba |= *fmp++;
    subb |= *fmp;
    fmp = &p48d[ s78[(val & 07777L)] & 0377 ][0];
    suba |= *fmp++;
    subb |= *fmp;
    *h0L++ ^= suba;
    *h0L++ ^= subb;

    if (i & 1) h0L = swap;
    else h1R = swap;
    }

  val = *h0L;
  *h0L++ = *h1R;
  *h1R++ = val;
  val = *h0L;
  *h0L = *h1R;
  *h1R = val;
  h0L = swap;
  h1R = &swap[4];
  _small = _scratch;

  while (h0L < h1R)
    {
    *_small++ = (UCHAR)((*h0L & 036000000L) >> 15) | (UCHAR)((*h0L & 0360000L) >> 13);      //@0001
    *_small++ = (UCHAR)((*h0L & 03600L) >> 3)      | (UCHAR)((*h0L & 036L) >> 1);         //@0001
    h0L++;
    }

  desPermute(_scratch, pvs, pmv, outblock);
  } // End desAlgorithm

//-----------------------------------------------------------------------------------------------
//
static UCHAR s1[64] = {                  // S[1] << 4 
       224, 64,208, 16, 32,240,176,128, 48,160, 96,192, 80,144,  0,112,
         0,240,112, 64,224, 32,208, 16,160, 96,192,176,144, 80, 48,128,
        64, 16,224,128,208, 96, 32,176,240,192,144,112, 48,160, 80,  0,
       240,192,128, 32, 64,144, 16,112, 80,176, 48,224,160,  0, 96,208
};

static UCHAR s3[64] = {                  // S[3] << 4 
       160,  0,144,224, 96, 48,240, 80, 16,208,192,112,176, 64, 32,128,
       208,112,  0,144, 48, 64, 96,160, 32,128, 80,224,192,176,240, 16,
       208, 96, 64,144,128,240, 48,  0,176, 16, 32,192, 80,160,224,112,
        16,160,208,  0, 96,144,128,112, 64,240,224, 48,176, 80, 32,192
};

static UCHAR s5[64] = {                  // S[5] << 4 
        32,192, 64, 16,112,160,176, 96,128, 80, 48,240,208,  0,224,144,
       224,176, 32,192, 64,112,208, 16, 80,  0,240,160, 48,144,128, 96,
        64, 32, 16,176,160,208,112,128,240,144,192, 80, 96, 48,  0,224,
       176,128,192,112, 16,224, 32,208, 96,240,  0,144,160, 64, 80, 48
};

static UCHAR s7[64] = {                  // S[7] << 4 
        64,176, 32,224,240,  0,128,208, 48,192,144,112, 80,160, 96, 16,
       208,  0,176,112, 64,144, 16,160,224, 48, 80,192, 32,240,128, 96,
        16, 64,176,208,192, 48,112,224,160,240, 96,128,  0, 80,144, 32,
        96,176,208,128, 16, 64,160,112,144, 80,  0,240,224, 32, 48,192
};

static UCHAR s2[64] = {                  // S[2] 
        15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
         3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
         0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
        13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9
};

static UCHAR s4[64] = {                  // S[4] 
         7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
        13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
        10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
         3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14
};

static UCHAR s6[64] = {                  // S[6] 
        12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
        10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
         9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
         4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
};

static UCHAR s8[64] = {                  // S[8] 
        13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
         1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
         7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
         2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

static UCHAR p48i[] = {
        24, 15,  6, 19, 20, 28, 20, 28, 11, 27, 16,  0,
        16,  0, 14, 22, 25,  4, 25,  4, 17, 30,  9,  1,
         9,  1,  7, 23, 13, 31, 13, 31, 26,  2,  8, 18,
         8, 18, 12, 29,  5, 21,  5, 21, 10,  3, 24, 15
};

static int bytebit[] = {0200,0100,040,020,010,04,02,01};    // bit 0 is left-most in byte

static long bigbyte[] = {
        0x800000L, 0x400000L, 0x200000L, 0x100000L,
        0x80000L,  0x40000L,  0x20000L,  0x10000L,
        0x8000L,   0x4000L,   0x2000L,   0x1000L,
        0x800L,    0x400L,    0x200L,    0x100L,
        0x80L,     0x40L,     0x20L,     0x10L,
        0x8L,      0x4L,      0x2L,      0x1L
};

static int Ignited = 0;

void desInit()
  {
  register int  j, left, right;

  if (Ignited) return;

  for (j = 0; j < 4096; j++)
    {
    left  = ((j >> 6) & 040) | ((j >> 2) & 020) | ((j >> 7) & 017);
    right = (j & 040) | ((j << 4) & 020) | ((j >> 1) & 017);
    s12[j] = s1[left] | s2[right];
    s34[j] = s3[left] | s4[right];
    s56[j] = s5[left] | s6[right];
    s78[j] = s7[left] | s8[right];
    }

  p48init();
  Ignited = 1;
  } // end desInit

//----------------------------------------------------------------------
//
// Use the key schedule specified in the Standard (ANSI X3.92-1981).    
//
static char pc1[] = {                   // permuted choice table (key)  
        56, 48, 40, 32, 24, 16,  8,
         0, 57, 49, 41, 33, 25, 17,
         9,  1, 58, 50, 42, 34, 26,
        18, 10,  2, 59, 51, 43, 35,
        62, 54, 46, 38, 30, 22, 14,
         6, 61, 53, 45, 37, 29, 21,
        13,  5, 60, 52, 44, 36, 28,
        20, 12,  4, 27, 19, 11,  3
};

static char totrot[] = {                // number left rotations of pc1 
  1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28
};

static char pc2[] = {                   // permuted choice key (table)  
        13, 16, 10, 23,  0,  4,
         2, 27, 14,  5, 20,  9,
        22, 18, 11,  3, 25,  7,
        15,  6, 26, 19, 12,  1,
        40, 51, 30, 36, 46, 54,
        29, 39, 50, 44, 32, 47,
        43, 48, 38, 55, 33, 52,
        45, 41, 49, 35, 28, 31
};

//------------------------------------------------------------------------
//
//                                desKeyInit
//
void desKeyInit(char  *key, int edf)          // initialize key schedule array
  {
  register int  i, j, l, m, n;
  char    pc1m[56], pcr[56];

  if (!Ignited) desInit();

//printf("keybuf(desKeyInit): %02X %02X %02X %02X %02X %02X %02X %02X \n",
//       key[0], key[1], key[2], key[3],
//       key[4], key[5], key[6], key[7]);

  for (j = 0; j < 56; j++)
    {                               // convert pc1 to bits of key   
    l = pc1[j];                   // integer bit location         
    m = l & 07;               // find bit                     
    pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
    }

  for (i = 0; i < 16; i++)
    {                             // key chunk for each iteration 
    if (edf == DE) m = (15 - i) << 1;
    else m = i << 1;

    n = m + 1;
    kn[m] = kn[n] = 0L;

    for (j = 0; j < 56; j++)  // rotate pc1 the right amount  
    pcr[j] =
    pc1m[(l = j+totrot[i]) < (j < 28 ? 28 : 56) ? l : l-28];

    // rotate left and right halves independently   

    for (j = 0; j < 24; j++)
      {
      if (pcr[pc2[j]])      kn[m] |= bigbyte[j];
      if (pcr[pc2[j + 24]]) kn[n] |= bigbyte[j];
      }
    }
  } // end desKeyInit

//----------------------------------------------------------------------
//
static void p48init()   // initialize 32-bit permutation 
  {
  int l, j, k, i;
  long (*pfill)[2];    // Pointer to a 2-dimensional array: (*p)[n]
                       //  *p points to 1st array-element,
                       //  [n] must match the number of 2nd array-elements

  for (i = 0; i < 4; i++)
    { // each input byte position     
  
    switch (i)
      {
      default:
        break;

      case 0:
        pfill = p48a;
        break;

      case 1:
        pfill = p48b;
        break;

      case 2:
        pfill = p48c;
        break;

      case 3:
        pfill = p48d;
        break;
      } // end switch

    // Performance: Fall thru and continue
    //  with initalized array pointer (*pfill)[2]
    //  which now can be used just as usual pfill[j][k] 
    for (j = 0; j < 256; j++)
      {
      for (k = 0; k < 2; k++)
        pfill[j][k] = 0L;

      for (k = 0; k < 24; k++)
        {
        l = p48i[k];
        if ((l >> 3) != i) continue;

        if (!(j & bytebit[l & 07])) continue;

        pfill[j][0] |= bigbyte[k];
        }

      for (k = 24; k < 48; k++)
        {
        l = p48i[k];
        if ((l >> 3) != i) continue;

        if (!(j & bytebit[l & 07])) continue;

        pfill[j][1] |= bigbyte[k - 24];
        }
      } // end for(i)
    }
  } // end p48init

//----------------------------------------------------------------------
//
void desPermute(char *inblock, int *test, int *vals, char *outblock)
  {
  char  *cp, *eop, *eip;
  int *dp;

  eop = &outblock[8];
  eip = &inblock[8];

  for (cp = outblock; cp < eop;) *cp++ = 0;

  while (inblock < eip)
    {
    cp = outblock;
    dp = test;

    while (cp < eop)
      {
      if (*inblock & *dp++)
      *cp |= *vals;
      cp++;
      }

    inblock++;
    vals++;
    }
  } // end desPermute

