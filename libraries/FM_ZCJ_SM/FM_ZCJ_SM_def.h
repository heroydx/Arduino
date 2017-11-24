/*
   FM_ZCJ_SM.h

   Library to support FM Radio module, provied by ZCJ.
   taobao link 
   https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-10758737972.22.3NGwHv&id=43379080309
   https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-10758737972.25.3NGwHv&id=43328850623

    ---

    Copyright (C) 2017  Steven Lian (steven.lian@gmail.com)
   

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef _FM_ZCJ_SM_DEF_H
#define _FM_ZCJ_SM_DEF_H


// header defining the interface of the source.

#define CONST_APP_FM_RECV_SIO_BAUD_DATA 38400
#define CONST_APP_FM_RECV_SENDREV_BUFFLEN 80
#define CONST_APP_FM_RECV_BIG_BUFFLEN 160
#define CONST_APP_FM_RECV_SYS_COMMAND_BUFFLEN 20
#define CONST_APP_FM_RECV_SIO_DATA_END '\n'

#define CONST_APP_FM_RECV_SIO_ERROR_COUNT 5

#define CONST_APP_FM_RECV_TIMEOUT_LEN 1000*2
#define CONST_APP_FM_RECV_SIO_QUERY_INTERVAL  1000*60

#define CONST_APP_FM_STAT_QUERY_INTERVAL 5*1000

#define CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS 20
#define CONST_APP_FM_RECV_COMMAND_BUFF_LEN 10
#define CONST_APP_FM_RECV_COMMAND_PARAM_LEN 10


#define CONST_APP_FM_RECV_CMD_FLAG_SYSTEM 0
#define CONST_APP_FM_RECV_CMD_FLAG_WEBAPP 1
#define CONST_APP_FM_RECV_CMD_FLAG_IRDA 2
#define CONST_APP_FM_RECV_CMD_FLAG_KEY 3

//application data structure

#define CONST_APP_FM_CMD_STRING_LENGTH 12


//频率 = 2 byte，开关(1bit)+设备有效(1bit)+校园(1bit)+音量(5bit)=1 byte,背景灯 1 byte,静噪 1bit+ 阈值+5bit =1 byte 
#define CONST_APP_FM_STAT_STRING_LENGTH 6
#define CONST_APP_FM_STAT_STRING_SWITCH_MASK 0x80
#define CONST_APP_FM_STAT_STRING_VALID_MASK 0x40
#define CONST_APP_FM_STAT_STRING_CAMPUS_MASK 0x20
#define CONST_APP_FM_STAT_STRING_VOL_MASK 0x1F
#define CONST_APP_FM_STAT_STRING_DSP_MASK 0x80
#define CONST_APP_FM_STAT_STRING_SN_MASK 0x1F



//FM radio machine is pingpang key, our system: 1=on,0=off
#define CONST_APP_FM_RECV_DEFAULT_STAT 1
//campus=on: 760-1080(MHz) campus =off: 870-1080(MHz)
#define CONST_APP_FM_RECV_FREQUENCY_HIGH 1080
#define CONST_APP_FM_RECV_FREQUENCY_CAMPUS_LOW 760
#define CONST_APP_FM_RECV_FREQUENCY_LOW 870
#define CONST_APP_FM_RECV_DEFAULT_FREQUENCY 925
//volume: 0-30
#define CONST_APP_FM_RECV_MIN_VOL 0
#define CONST_APP_FM_RECV_MAX_VOL 30
#define CONST_APP_FM_RECV_DEFAULT_VOL 10
//campus=0,off, 1=on
#define CONST_APP_FM_RECV_DEFAULT_SN_CAMPUS 0
//backgroud light on time: 00-99
#define CONST_APP_FM_RECV_MIN_BLK 0
#define CONST_APP_FM_RECV_MAX_BLK 99
#define CONST_APP_FM_RECV_DEFAULT_BLK 10
//DSP SN 静噪 0=off, 1=on;
#define CONST_APP_FM_RECV_DEFAULT_SN_DSP 0
//DSP SN 静噪 阈值  00-20
#define CONST_APP_FM_RECV_SN_MIN_THR 0
#define CONST_APP_FM_RECV_SN_MAX_THR 20
#define CONST_APP_FM_RECV_DEFAULT_SN_THR 10


#define CONST_APP_FM_RECV_FM_DEVICE_VER_BUFFLEN 60

#define CONST_APP_FM_RECV_SAVE_CYCLE 1000*60*60

typedef struct{
  short nFrequency;
  uint8_t u8Vol;
  uint8_t u8Stat;
  uint8_t u8Campus;
  uint8_t u8Blk;
  uint8_t u8SN_THR;
  uint8_t u8DSP;  
  short errCount;
} stFMDeviceStat;

typedef struct{
  short timeStamp;
  short nFrequency;
  uint8_t u8Vol;
  uint8_t u8Stat;
  uint8_t u8Campus;
  uint8_t u8Blk;
  uint8_t u8SN_THR;
  uint8_t u8DSP;  
} stFMDeviceRptStat;

typedef struct  {
  uint8_t command;
  uint8_t flag;//system command =0,web/app command=1;
  short param;
  //char param[CONST_APPLICATION_COMMAND_PARAM_LEN + 1];
} stFMCommand;


typedef struct  {
  uint8_t command;
  uint8_t flag;//system command =0,web/app command=1;
  //uint8_t u8statString[CONST_APP_FM_STAT_STRING_LENGTH];
  //char param[CONST_APPLICATION_COMMAND_PARAM_LEN + 1];
  short nFrequency;
  uint8_t u8Vol;
  uint8_t u8Stat;
  uint8_t u8Campus;
  uint8_t u8Blk;
  uint8_t u8SN_THR;
  uint8_t u8DSP;  
} stFMStat;


typedef union {
  stFMCommand CMD;
  stFMStat stat;
  unsigned char u8Data[sizeof(stFMStat)];
} FMCommandUnion;



#endif // _FM_ZCJ_SM_DEF_H

