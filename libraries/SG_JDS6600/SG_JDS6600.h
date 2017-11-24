/*
   Library to support Sigal Generator, provied by 均测科技.
   taobao link 
   https://detail.tmall.com/item.htm?id=559945767309&cm_id=140105335569ed55e27b&abbucket=3
  
    ---

    Copyright (C) 2017/11/14  Steven Lian (steven.lian@gmail.com) 
   

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
#ifndef _SG_JDS6600_H
#define _SG_JDS6600_H


#include <Arduino.h>

//JDS6600 CLASS BEGIN
#include <FS.h>
#include <ESPSoftwareSerial.h>

#include "ListArray.h"

extern "C" {
#include "miscCommon.h"
}

#include "SG_JDS6600_def.h"

#define CONST_APP_SG_JDS6600_VERSION 171114
// header defining the interface of the source.
//#define APP_SERIAL Serial
#define APP_SERIAL SerialSG

// for debug purpose only
#define CONST_SG_RX_PORT D6         // GPIO 12 D6 -->--TX
#define CONST_SG_TX_PORT D5         // GPIO 14 D5 -->--RX

#define CONST_APP_SG_RECV_FILE_NAME "/SGConfig_JDS.ini"
#define CONST_APP_SG_RECV_FILE_SIZE 1024


#define CONST_APP_SG_CHANNEL_COUNT 2

class SG_JDS
{
  public:
    //data
    unsigned long devModel;
    unsigned long devSN;
    bool bSendBusyFlag = false;
    uint8_t au8sendBuff[CONST_APP_SG_RECV_SENDREV_BUFFLEN + 2];
    uint8_t au8recvBuff[CONST_APP_SG_RECV_SENDREV_BUFFLEN + 2];
    uint8_t u8recvLen;
    // application data save cycle by minutes,default = 60 mins
    unsigned long ulSaveTick;
    
    //device current status
    stChannelInfo currStat[CONST_APP_SG_CHANNEL_COUNT];
    stChannelInfo preStat[CONST_APP_SG_CHANNEL_COUNT];
    //以下是中间状态变量，给decode_stat_string使用
    stChannelInfo statBuf;
    
    bool bStatChangeFlag;

    //last sys command
    char achSysCommand[CONST_APP_SG_RECV_SYS_COMMAND_BUFFLEN + 1];
    //char achFMCommand[CONST_APP_SG_RECV_SENDREV_BUFFLEN];
    //reply data
    char achSIOReply[CONST_APP_SG_RECV_SENDREV_BUFFLEN];
    //
    //short errCount;

    //function
    SG_JDS();
    bool begin();
    void loop();
    bool devReady();
    void handle_SIO_reply_data(char *param);//处理FM返回数据
    void check_device_reply(); //检查FM是否有命令返回
   
    bool decode_762_command(char *ptr); //解释并执行服务器命令

    bool bLoad_config();
    bool bSave_config();

    void send_CMD_to_device(char *param);  //发送给device的命令到给device
    void send_CMD_to_device(short nLen, uint8_t *param);
    void trans_server_CMD(short channel, char *type, long val); //把服务器的命令转换为给FM的AT命令格式
    
    void debug_print_info();

  private:
    //data
    unsigned long ulRecvTimeOutTick;
    unsigned long ulSIOSendTick;
    unsigned long ulSIOQueryTick;
    unsigned long ulSIOQueryTickThreshold;
    unsigned long ulStatQueryTick;

    bool deviceReady;
    bool devStatusReady;
    uint8_t channelNum;
    long cmdValue;
    #define CONST_APP_SG_SERVER_CMD_LENGTH 6
    char cmdType[6];
    short devStatusCount;
    #define CONST_APP_SG_DEV_CHECK_LIST_LEN 30
    short anDevStatusCheckList[CONST_APP_SG_DEV_CHECK_LIST_LEN];
    
    //func
    void prepare_cmd_param(char *dataPtr,char *param);
    void prepare_cmd_param(char *dataPtr,short cmd,short actionID,long param);
    void prepare_cmd_param(char *dataPtr,short cmd,short actionID,long p1,long p2);
    void prepare_cmd_param(char *dataPtr,short cmd,short actionID,long p1, long p2,long p3,long p4,long p5);
    void restore_setting(); //恢复当前设置
    bool read_device_status();
    bool set_default_config();
    bool is_device_exist();

};

//class end


#endif // _SG_JDS6600_H

