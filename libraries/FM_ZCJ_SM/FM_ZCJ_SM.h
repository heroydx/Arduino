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
#ifndef _FM_ZCJ_SM_H
#define _FM_ZCJ_SM_H


#include <Arduino.h>

//FM RADIO CLASS BEGIN
#include <FS.h>
//#include "ListArray.h"
#include <ESPSoftwareSerial.h>

extern "C" {
#include "miscCommon.h"
}

#include "FM_ZCJ_SM_def.h"

#define CONST_APP_FM_ZCJ_VERSION 171002
// header defining the interface of the source.
#define APP_SERIAL Serial
//#define APP_SERIAL SerialFM

// for debug purpose only
#define CONST_FM_ZCJ_RX_PORT 12         // GPIO D6 --> FM 11--TX
#define CONST_FM_ZCJ_TX_PORT 14         // GPIO D5 --> FM 12--RX

#define CONST_APP_FM_RECV_FILE_NAME "/FMConfig_ZCJ.ini"
#define CONST_APP_FM_RECV_FILE_SIZE 1024


//to FM Radio FIFO command buff
//为了简化设计,考虑到命令队列不长,之间用数组模拟,并且用移动后方数据的方式。 Begin
class FMCommand
{
  public:
    FMCommand();
    uint8_t push(uint8_t command, short param, uint8_t flag);
    uint8_t lpush(uint8_t command, short param, uint8_t flag);
    uint8_t pop(stFMCommand *spCommand);
    uint8_t rpop(stFMCommand *spCommand);
    uint8_t available();
  private:
    stFMCommand gasCommand[CONST_APP_FM_RECV_COMMAND_BUFF_LEN];
    uint8_t gu8CommandPos;
};


enum ATCOMMAND {
  FM_SET_FRE,
  FM_FRE_DOWN,
  FM_FRE_UP,
  FM_PLAY_PAUSE,
  FM_SET_VOL,
  FM_VOL_DOWN,
  FM_VOL_UP,
  FM_SET_BANK,
  FM_SET_CAMPUS,
  FM_SET_DSP,
  FM_SET_SN_THR,
  FM_RESET,
  FM_STATUS,
  FM_SCAN,
  FM_SCAND,
  FM_SCANU,
  FM_SCAMSTOP,
  FM_CHNUM,
  FM_CHDOWN,
  FM_CHUP,
  FM_SET_ALL,
  FM_STAT_REPORT,
  FM_SAVE_FREQ,
  FM_OP_SAVE_REEQ,
};



class FM_ZCJ
{
  public:
    //data
    bool bSendBusyFlag = false;
    uint8_t au8sendBuff[CONST_APP_FM_RECV_SENDREV_BUFFLEN + 2];
    uint8_t au8recvBuff[CONST_APP_FM_RECV_SENDREV_BUFFLEN + 2];
    //uint8_t bigBuff[CONST_APP_FM_RECV_BIG_BUFFLEN + 2];
    uint8_t u8recvLen;
    unsigned long ulRecvTimeOutTick;
    unsigned long ulFMSIOQueryTick;
    unsigned long ulFMStatQueryTick;
    // application data save cycle by minutes,default = 60 mins
    unsigned long ulFMSaveTick;
    
    //FM device current status
    stFMDeviceStat currStat;
    //以下是中间状态变量，给decode_stat_string使用
    stFMDeviceStat statBuf;

    char *dataBuffPtr;//extern big buff 
    
    bool bStatChangeFlag;
    uint8_t u8statString[CONST_APP_FM_STAT_STRING_LENGTH]; //系统状态字符串
    uint8_t u8preStatString[CONST_APP_FM_STAT_STRING_LENGTH]; //以前的系统状态字符串
    //uint8_t u8statBuff[CONST_APP_FM_STAT_STRING_LENGTH]; //系统状态字符串缓冲区，主要接收来自client的报告，然后转发到服务器。

    //FM Radio Frequency range;
    short nFreqHigh = CONST_APP_FM_RECV_FREQUENCY_HIGH;
    short nFreqLow = CONST_APP_FM_RECV_FREQUENCY_LOW;
    uint8_t u8freqPos;
    uint8_t u8freqNum;
    char achFMDeviceVer[CONST_APP_FM_RECV_FM_DEVICE_VER_BUFFLEN];
    bool bSendSaveFreq;
    //last sys command
    char achSysCommand[CONST_APP_FM_RECV_SYS_COMMAND_BUFFLEN + 1];
    //last to FM command
    FMCommandUnion currCommand;
    FMCommandUnion allCommand;// buff for command struct 一个命令缓冲结构
    //char achFMCommand[CONST_APP_FM_RECV_SENDREV_BUFFLEN];
    //reply data
    char achFMReply[CONST_APP_FM_RECV_SENDREV_BUFFLEN];
    //
    //short errCount;

    //function
    FM_ZCJ();
    bool begin();
    void FM_loop();
    bool devReady();
    void handle_SIO_reply_data(char *param);//处理FM返回数据
    void push_CMD_to_FM_queue();//把给FM的命令放到发送队列
    void check_FM_reply(); //检查FM是否有命令返回
    void handle_Server_to_FM_command(char achParam[]); //处理服务器给FM的命令,兼容非loRa系统的命令。
    //void trans_Server_to_LoRa_FM_command(char achParam[]); //把服务器给FM的命令,转换成loRa系统的命令，LoRa HOST使用。
    short trans_Server_to_Inter_FM_command(char achParam[],short *npFMCommand,short *npFMParam); //把服务器给FM的命令,转换成内部系统的命令。
    void handle_host_to_FM_command(uint8_t data[]); //处理LoRa给FM的命令，LoRa Client使用。    //把原来服务发送的字符串命令，转换为内部命令格式， 返回值>0,成功，=0没有命令

    void report_LoRaHost_stat(); //发送当前状态给LoRa Host.
    void encode_stat_string(uint8_t *u8Ptr); //编码系统状态字符串
    void decode_stat_string(uint8_t *u8Ptr); //解码系统状态字符串
    void decode_stat_json(char *strPtr); //转换到JSON格式字符串
    void copy2_rpt_data(stFMDeviceRptStat *spStat,uint8_t *u8Ptr); //复制到report格式


    //保存的FM频率列表函数
    short freq_curr(); //当前频率
    short freq_next(); //下一个频率
    short freq_prev(); //上一个频率
    uint8_t freq_add(short freq); //增加一个
    uint8_t freq_del(); //删除当前
    uint8_t freq_del(short freq); //删除给定频率
    uint8_t freq_clear(); //清除所有频率
    bool freq_set_send(); //设置发送保存的频率列表
    bool freq_clear_send(); //清除发送保存的频率列表标志
    uint8_t freq_num(); //统计当前保存的频率数量
    uint8_t freq_sort(int8_t softFlag); //排序当前保存的频率列表

    bool bLoad_config();
    bool bSave_config();

    void send_CMD_to_FM(short nType, short nParam);//发送给FM的命令到给FM
    
    void debug_print_info();

  private:
    //data
    bool deviceReady;
    //FM Radio Frequency saving array
    short anSaveFreq[CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS + 2];

    //func
    void exec_FM_command(short cmd,short param);
    void restore_setting(); //恢复当前设置
    void prepare_FM_CMD(short nType, short nParam, char *pBuff); //把服务器的命令转换为给FM的AT命令格式
    void check_cmd_for_stat(short nType); //部分命令执行后，会默认把开关设置为开。
    bool read_config();
    bool is_FM_exist();

};

//class end


#endif // _FM_ZCJ_SM_H

