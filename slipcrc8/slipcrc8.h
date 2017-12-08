#pragma once
#ifndef SLIPCRC8_H
#define SLIPCRC8_H

#define CONST_SLIP_END             0300                    // 0xC0 indicates end of packet
#define CONST_SLIP_ESC             0333                    // 0xDB indicates byte stuffing
#define CONST_SLIP_ESC_END         0334                    // 0xDC ESC ESC_END means END data byte
#define CONST_SLIP_ESC_ESC         0335                    // 0xDD ESC ESC_ESC means ESC data byte

int slip_encode(unsigned char *outBuf, unsigned char *inBuf, int nLen);
int slip_decode(unsigned char *outBuf, unsigned char *inBuf, int nLen);
unsigned char crc8( unsigned char *data, int number_of_bytes_in_data );

#endif
