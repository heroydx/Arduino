/*
   Tower_FSUAir_def.h

   Library to support China Power FSU.
   To follow the spec of 基站智能空调控制器通讯协议 V1.0
    ---

    Copyright (C) 2017/11/15  Steven Lian (steven.lian@gmail.com)    

*/

#pragma once
#ifndef _Tower_FSUAir_H
#define _Tower_FSUAir_H


#include <Arduino.h>

//JDS6600 CLASS BEGIN
#include <FS.h>
#include <ESPSoftwareSerial.h>
//#include "ListArray.h"


extern "C" {
#include "miscCommon.h"
}

#include "Tower_FSUAir_def.h"

#define CONST_APP_Tower_FSUAir_VERSION 171115
// header defining the interface of the source.
#define APP_SERIAL SerialFSU

#define CONST_FSU_RX_PORT 4         // GPIO 4
#define CONST_FSU_TX_PORT 2         // GPIO 2

#define CONST_APP_FSU_RECV_FILE_NAME "/FSUConfig_AIR.ini"
#define CONST_APP_FSU_RECV_FILE_SIZE 1024


class TT_FSU
{
  public:
    //data
    bool bSendBusyFlag = false;
    
    uint8_t ADR; //对同类型设备的不同地址描述（1－254，0、255保留）
    
    uint8_t *bigBuffPtr; //extern big buff
    uint8_t *commBuffPtr; //extern big buff
    
    bool bStatChangeFlag; //状态变换指示器
    
    stFSUSIODesc sendStru; //发送结构体描述
    stFSUSIODesc recvStru; //接收结构体描述
    
    //uint8_t bigBuff[CONST_APP_FSU_RECV_BIG_BUFFLEN + 2];
    short nRecvLen;
    unsigned long ulRecvTimeOutTick;
    // application data save cycle by minutes,default = 60 mins
    unsigned long ulSaveTick;
    
    //device current status
    stBaseAirInfo currStat;
    stBaseAirInfo preStat;
    
    //function
    TT_FSU();
    bool begin(uint8_t addr,uint8_t *bufPtr1,uint8_t *bufPrt2);
    bool devReady();
    short available(); //检测是否FSU有数据
    void handle_SIO_reply_data(uint8_t *param);//处理FM返回数据
    bool bLoad_config();
    bool bSave_config();

    void send_CMD_to_FSU(short nLen, char *dataPtr);//发送给FSU的命令到给FSU
    
    void debug_print_info();

  private:
    //data
    bool deviceReady;
    uint8_t au8recvBuff[CONST_APP_FSU_RECV_SENDREV_BUFFLEN+2];

    //func
    bool calc_CHKSUM();
    bool calc_LENGTH();
    
    bool encode_send_format();
    bool decode_recv_format();    
    
    void prepare_FSU_CMD(); //把服务器的命令转换为给FM的AT命令格式
    bool read_config();
    bool is_device_exist();
    void restore_setting();
};

//class end


#endif // _Tower_FSUAir_H

