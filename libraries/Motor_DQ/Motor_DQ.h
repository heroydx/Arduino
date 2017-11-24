/*
   Motor_DQ.h

   Library to support Power Motor, provied by taiwan DQ.
   
   http://www.bigbest.com.tw/webls-zh-tw/index.html
   
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
#ifndef _Motor_DQ_H
#define _Motor_DQ_H


#include <Arduino.h>

//FM RADIO CLASS BEGIN
#include <FS.h>
//#include "ListArray.h"
#include <ESPSoftwareSerial.h>

extern "C" {
#include "miscCommon.h"
}

#include "Motor_DQ_def.h"

#define APP_MOTOR_SERIAL motorSerial

#define CONST_MOTOR_DQ_RX_PORT 4
#define CONST_MOTOR_DQ_TX_PORT 2

class motor_DQ
{
  public:
    //data
    stMotorStat_DQ currStat;
    stMotorStat_DQ statBuf;

    stMotorCMD_DQ bufCommand; // buff for command struct 一个命令缓冲结构

    char recvBuff[CONST_APP_MOTOR_RECV_BUFFLEN];
    char sendBuff[CONST_APP_MOTOR_SEND_BUFFLEN];
    uint8_t recvLen;
    unsigned long nRecvTimeOutInMS;
    unsigned long ulRecvTimeOutTick;

    //fuction
    motor_DQ();
    bool begin();
    bool begin(uint8_t ID);
    bool devReady();
    void motor_loop();

    bool send_CMD(stMotorCMD_DQ *ptrCMD);

    void debug_print_info();

  private:
    //data
    bool deviceReady;
    uint8_t rs485Busy;
    uint8_t devID;
    stMotorCMD_DQ currCommand;
    unsigned long ulSIOQueryTick;
    unsigned long ulSIOQueryInMS;
    char queryCMDList[CONST_APP_MOTOR_DEFAULT_QUERY_CMD_LIST_LEN];
    uint8_t queryCMDListCount;
    unsigned long ulOneSecondTick;

    //function
    bool avaiable();
    bool is_motor_exist(uint8_t ID);
    uint8_t motor_encode(uint8_t data);
    uint8_t motor_decode(uint8_t data);
    void byte_to_bcd(char data, char *str);
    void short_to_bcd(short data, char *str);
    uint8_t bcd_to_byte(char *str);
    short bcd_to_short(char *str);
    void cal_check_sum(uint8_t *pBuff);
    bool verify_check_sum(uint8_t *pBuff);
    void encode_CMD(char *pBuff, stMotorCMD_DQ *stPtr);
    void decod_CMD(char *pBuff, stMotorStat_DQ *stPtr);
    bool send_cmd_to_device(char *pBuff);
};

//class end

#endif // _Motor_DQ_H

