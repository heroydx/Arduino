/*
   Motor_DQ_def.h

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
#ifndef _Motor_DQ_DEF_H
#define _Motor_DQ_DEF_H


// header defining the interface of the source.

#define CONST_APP_MOTOR_SIO_BAUD 2400

#define CONST_APP_MOTOR_SAVE_EEPROM 14
#define CONST_APP_MOTOR_QUERY_MAX_CYCLE 0
#define CONST_APP_MOTOR_SET_MAX_CYCLE 1
#define CONST_APP_MOTOR_QUERY_CURR_CYCLE 2
#define CONST_APP_MOTOR_SET_CURR_CYCLE 3

#define CONST_APP_MOTOR_ID_BEGIN_POS 1
#define CONST_APP_MOTOR_CMD_BEGIN_POS 3
#define CONST_APP_MOTOR_SPEED_BEGIN_POS 4
#define CONST_APP_MOTOR_MODE_BEGIN_POS 8
#define CONST_APP_MOTOR_DIANLIU_BEGIN_POS 8
#define CONST_APP_MOTOR_OFFDELAY_BEGIN_POS 9
#define CONST_APP_MOTOR_ONDELAY_BEGIN_POS 10
#define CONST_APP_MOTOR_ERRCODE_BEGIN_POS 12
#define CONST_APP_MOTOR_ID2_BEGIN_POS 11
#define CONST_APP_MOTOR_CHKSUM_BEGIN_POS 15
#define CONST_APP_MOTOR_SW_VER_BEGIN_POS 13
#define CONST_APP_MOTOR_HW_VER_BEGIN_POS 14

#define CONST_APP_MOTOR_MSG_LENG 18


#define CONST_APP_MOTOR_M_HEADER_CHAR '#'
#define CONST_APP_MOTOR_P_HEADER_CHAR '!'
#define CONST_APP_MOTOR_MP_END_CHAR '\xd'

#define CONST_APP_MOTOR_SEND_BUFFLEN 24
#define CONST_APP_MOTOR_RECV_BUFFLEN 24

#define CONST_APP_MOTOR_DEFAULT_QUERY_CYCLE_IN_MS 6000
#define CONST_APP_MOTOR_DEFAULT_QUERY_TIME_OUT_IN_MS 3000
#define CONST_APP_MOTOR_DEFAULT_QUERY_CMD_LIST_LEN 4

#define CONST_APP_MOTOR_DEFAULT_CYCLE_SPEED 200
#define CONST_APP_MOTOR_DEFAULT_CYCLE_MAX_SPEED 500
#define CONST_APP_MOTOR_DEFAULT_CYCLE_MODE 0
#define CONST_APP_MOTOR_DEFAULT_ON_DELAY 1
#define CONST_APP_MOTOR_DEFAULT_OFF_DELAY 1

#define CONST_APP_MOTOR_DEFAULT_SET_ID  '\xFF'

#define CONST_APP_MOTOR_DEFAULT_TRY_TIMES 5

#define CONST_APP_MOTOR_DEFAULT_ID '\xFF'

typedef struct {
  short ID;
  short speed;
  short maxSpeed;
  short cycleMode;
  short dianliu;
  char errorCode;
  char onDelay;
  char offDelay;
  char swVer;
  char hwVer;
} stMotorStat_DQ;

typedef struct {
  char cmd;
  short ID;
  short speed;
  short cycleMode;
  //char onDelay;
  //char offDelay;
} stMotorCMD_DQ;



#endif // _Motor_DQ_DEF_H

