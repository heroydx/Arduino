#include <slipcrc8.h>

int slip_encode(unsigned char *outBuf, unsigned char *inBuf, int nLen)
{
  int i;
  int nCount;
  unsigned char chT;
  nCount = 0;

  //begin
  outBuf[nCount++] = CONST_SLIP_END;
  

  for (i = 0; i < nLen; i++) {
    chT = inBuf[i];
    switch (chT)
    {
      case CONST_SLIP_END:
        outBuf[nCount++] = CONST_SLIP_ESC;
        outBuf[nCount++] = CONST_SLIP_ESC_END;
        break;
      case CONST_SLIP_ESC:
        outBuf[nCount++] = CONST_SLIP_ESC;
        outBuf[nCount++] = CONST_SLIP_ESC_ESC;
        break;
      default:
        outBuf[nCount++] = chT;
    }
  }

  //crc8
  chT=crc8(inBuf,nLen);
  switch (chT)
  {
    case CONST_SLIP_END:
      outBuf[nCount++] = CONST_SLIP_ESC;
      outBuf[nCount++] = CONST_SLIP_ESC_END;
      break;
    case CONST_SLIP_ESC:
      outBuf[nCount++] = CONST_SLIP_ESC;
      outBuf[nCount++] = CONST_SLIP_ESC_ESC;
      break;
    default:
      outBuf[nCount++] = chT;
  }  
  //end
  outBuf[nCount++] = CONST_SLIP_END;
  return nCount;
}

int slip_decode(unsigned char *outBuf, unsigned char *inBuf, int nLen)
{
  int i;
  int nCount;
  unsigned char chT;
  nCount = 0;
  i = 0;
  while (i < nLen)
  {
    chT = inBuf[i];
    i++;
    switch (chT)
    {
      case CONST_SLIP_END:
        if (nCount == 0) {
          continue;
        }
        break;
      case CONST_SLIP_ESC:
        chT = inBuf[i];
        i++;
        switch (chT)
        {
          case CONST_SLIP_ESC_END:
            outBuf[nCount++] = CONST_SLIP_END;
            break;
          case CONST_SLIP_ESC_ESC:
            outBuf[nCount++] = CONST_SLIP_ESC;
            break;
          default:
            break;
        }
        break;
      default:
        outBuf[nCount++] = chT;
        break;
    }
  }
  nCount--;
  chT = crc8(outBuf, nCount);
  if (chT != outBuf[nCount]) {
    nCount = -nCount;
  }

  return nCount;
}


/* please read copyright-notice at EOF */

//#include <stdint.h>

#define CRC8INIT    0x00
#define CRC8POLY    0x18              //0X18 = X^8+X^5+X^4+X^0

unsigned char crc8( unsigned char *data, int number_of_bytes_in_data )
{
  unsigned char  crc;
  int loop_count;
  unsigned char  bit_counter;
  unsigned char  b;
  unsigned char  feedback_bit;

  crc = CRC8INIT;

  for (loop_count = 0; loop_count != number_of_bytes_in_data; loop_count++)
  {
    b = data[loop_count];

    bit_counter = 8;
    do {
      feedback_bit = (crc ^ b) & 0x01;

      if ( feedback_bit == 0x01 ) {
        crc = crc ^ CRC8POLY;
      }
      crc = (crc >> 1) & 0x7F;
      if ( feedback_bit == 0x01 ) {
        crc = crc | 0x80;
      }

      b = b >> 1;
      bit_counter--;

    } while (bit_counter > 0);
  }

  return crc;
}

/*
  This code is from Colin O'Flynn - Copyright (c) 2002
  only minor changes by M.Thomas 9/2004

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

