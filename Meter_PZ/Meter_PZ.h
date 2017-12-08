/*
   Meter_PZ.h

   Library to support Power Meter module, provied by PZ.
   taobao link 
   https://item.taobao.com/item.htm?id=45589922100&_u=b1im8oic076
   
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
#ifndef _Meter_PZ_H
#define _Meter_PZ_H


#include <Arduino.h>

//FM RADIO CLASS BEGIN
#include <FS.h>
//#include "ListArray.h"
#include <ESPSoftwareSerial.h>

extern "C" {
#include "miscCommon.h"
}


#include "Meter_PZ_def.h"

#define CONST_METER_PZ_VERSION 170821

// header defining the interface of the source.
//#define APP_METER_SERIAL meterSerial
//#define APP_METER_SERIAL Serial1
#define APP_METER_SERIAL Serial

#define CONST_METER_DQ_RX_PORT 12
#define CONST_METER_DQ_TX_PORT 14

#define CONST_APP_STDEV_DIVIDE 10 //除数，如果=10就是10% 如果是5 就是20%

#define CONST_APP_METER_FILE_NAME "/meter_dq.data"
#define CONST_APP_METER_FILE_SIZE 1024
#define CONST_APP_METER_BIG_BUFF_SIZE 100

class Meter_DQ
{
  public:
    //data
    bool bFirstReadDianliangFlag;
    unsigned short failCount;
    unsigned short failTotal;

    unsigned short nDianliangInt;
    unsigned short nDianliangDec;
    unsigned short nDianyaInt;
    unsigned short nDianyaDec;
    unsigned short nDianliuInt;
    unsigned short nDianliuDec;
    unsigned short nGonglv;
    unsigned long lMeterDianliang;

    //warning data
    unsigned short nWDianyaInt;
    unsigned short nWGonglv;

    //pre hour data
    unsigned short preHour;
    unsigned short nHDianliangInt;
    unsigned short nHDianliangDec;
    unsigned long lHMeterDianliang;

    //yesterday data
    unsigned short preDay;
    unsigned short nYDianliangInt;
    unsigned short nYDianliangDec;
    unsigned long lYMeterDianliang;

    unsigned long lLastMeterDianliang;
    unsigned long lCurrMeterDianliang;

    unsigned short anLastGonglv[CONST_APP_METER_GONGLV_LAST_MAX_LEN];
    short nLastGonglvPos;
    unsigned short stdev; 
    unsigned short stAvg;

    char *dataBuffPtr;//extern big buff

    //func
    Meter_DQ();
    bool begin(char *bigBuffPtr);
    bool read();
    bool available();

    bool devReady();
    
    void set_default_ip();
    void set_daingliang_zero();
    
    unsigned short get_version();

    bool bLoad_config();
    bool bSave_config();

    //void debug_print_info();
    uint8_t recvBuff[CONST_APP_METER_SIO_DATA_LEN + 8];
    uint8_t recvBuffLen;

    uint8_t sendBuff[CONST_APP_METER_SENDREV_BUFFLEN + 2];

    
  private:
    //data
    unsigned short version;
    bool deviceReady;
    
    
    /*
    uint8_t gu8SIOCommandHeader[CONST_APP_METER_SIO_COMMAND_COUNT] = {
      CONST_APP_METER_DIANYA_SEND, CONST_APP_METER_DIANLIU_SEND, CONST_APP_METER_GONGLV_SEND,
      CONST_APP_METER_DIANLIANG_SEND,CONST_APP_METER_POWERFACTOR_SEND,CONST_APP_METER_FREQUENCY_SEND,
      CONST_APP_METER_ZERO_SEND,CONST_APP_METER_SETIP_SEND
    };
    */
    
    short gnSIOCommandNum;
    unsigned long ulSIOQueryTick;

    //func
    void clean_SIO();
    bool is_meter_exist();
    void send_cmd_to_device(uint8_t nType);
    void decode_CMD(uint8_t data[]);
    uint8_t check_sum(uint8_t *pBuff, int nLen);
    void encode_CMD(int nType, uint8_t *pBuff);
    void cal_stdev();

};

//class end


#endif // _Meter_PZ_H

