/*
   FM_ZCJ.h

   Library to support FM Radio module, provied by ZCJ.
   taobao link:
   https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-10758737972.16.3NGwHv&id=45300076747

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

#include <Arduino.h>

//FM RADIO CLASS BEGIN
#include <FS.h>
//#include "ListArray.h"
#include <ESPSoftwareSerial.h>

extern "C" {
#include "miscCommon.h"
}

// header defining the interface of the source.
#ifndef _FM_ZCJ_H
#define _FM_ZCJ_H


//#define APP_SERIAL Serial
#define APP_SERIAL SerialFM

#define CONST_FM_ZCJ_RX_PORT 12
#define CONST_FM_ZCJ_TX_PORT 14

#define CONST_APP_FM_RECV_FILE_NAME "/FMConfig_ZCJ.ini"
#define CONST_APP_FM_RECV_FILE_SIZE 1024

#define CONST_APP_FM_RECV_SIO_BAUD_DATA 38400
#define CONST_APP_FM_RECV_SENDREV_BUFFLEN 80
#define CONST_APP_FM_RECV_BIG_BUFFLEN 160
#define CONST_APP_FM_RECV_SYS_COMMAND_BUFFLEN 20
#define CONST_APP_FM_RECV_SIO_DATA_END '\n'

#define CONST_APP_FM_RECV_SIO_ERROR_COUNT 5

#define CONST_RECV_TIMEOUT_LEN 1000*2
#define CONST_APP_FM_RECV_SIO_QUERY_INTERVAL  1000*60

#define CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS 20
#define CONST_APP_FM_RECV_COMMAND_BUFF_LEN 10
#define CONST_APP_FM_RECV_COMMAND_PARAM_LEN 10


#define CONST_APP_FM_RECV_CMD_FLAG_SYSTEM 0
#define CONST_APP_FM_RECV_CMD_FLAG_WEBAPP 1
#define CONST_APP_FM_RECV_CMD_FLAG_IRDA 2
#define CONST_APP_FM_RECV_CMD_FLAG_KEY 3

//application data structure

//频率2=byte，开关(1bit)+设备有效(1bit)+校园(1bit)+音量(5bit)=1 byte,背景灯 1 byte,静噪 1bit+ 阈值+5bit =1 byte 
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
#define CONST_APP_FM_RECV_DEFAULT_FREQUENCY 900
//volume: 0-30
#define CONST_APP_FM_RECV_MIN_VOL 0
#define CONST_APP_FM_RECV_MAX_VOL 30
#define CONST_APP_FM_RECV_DEFAULT_VOL 15
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
typedef struct  {
  short command;
  short param;
  uint8_t flag;//system command =0,web/app command=1;
  //char param[CONST_APPLICATION_COMMAND_PARAM_LEN + 1];
} stFMCommand;

typedef union {
  stFMCommand CMD;
  unsigned char u8Data[sizeof(stFMCommand)];
} FMCommandUnion;



//to FM Radio FIFO command buff
//为了简化设计,考虑到命令队列不长,之间用数组模拟,并且用移动后方数据的方式。 Begin
class FMCommand
{
  public:
    FMCommand();
    uint8_t push(short command, short param, uint8_t flag);
    uint8_t lpush(short command, short param, uint8_t flag);
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
};



class FM_ZCJ
{
  public:
    //data
    bool bSendBusyFlag = false;
    uint8_t au8sendBuff[CONST_APP_FM_RECV_SENDREV_BUFFLEN + 2];
    uint8_t au8recvBuff[CONST_APP_FM_RECV_SENDREV_BUFFLEN + 2];
    uint8_t bigBuff[CONST_APP_FM_RECV_BIG_BUFFLEN + 2];
    uint8_t u8recvLen;
    unsigned long ulRecvTimeOutTick;
    unsigned long ulFMSIOQueryTick;
    // application data save cycle by minutes,default = 60 mins
    unsigned long ulFMSaveTick;
    short nFrequency;
    uint8_t u8Vol;
    uint8_t u8Stat;
    uint8_t u8Campus;
    uint8_t u8Blk;
    uint8_t u8SN_THR;
    uint8_t u8DSP;
    uint8_t u8statString[CONST_APP_FM_STAT_STRING_LENGTH]; //系统状态字符串

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
    FMCommandUnion allCommand;
    //char achFMCommand[CONST_APP_FM_RECV_SENDREV_BUFFLEN];
    //reply data
    char achFMReply[CONST_APP_FM_RECV_SENDREV_BUFFLEN];
    //
    short errCount;

    //function
    FM_ZCJ();
    bool begin();
    void FM_loop();
    void handle_FM_replay_data(char *param);//处理FM返回数据
    void push_CMD_to_FM_queue();//把给FM的命令放到发送队列
    void check_FM_reply(); //检查FM是否有命令返回
    void handle_Server_to_FM_command(char achParam[]); //处理服务器给FM的命令。
    void encode_stat_string(); //编码系统状态字符串
    void decode_stat_string(); //解码系统状态字符串
    void decode_stat_json(char *strPtr); //转换到JSON格式字符串

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

  private:
    //data
    //FM Radio Frequency saving array
    short anSaveFreq[CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS + 2];

    //func

    void restore_setting(); //恢复当前设置
    void prepare_FM_CMD(short nType, short nParam, char *pBuff); //把服务器的命令转换为给FM的AT命令格式
    void send_CMD_to_FM(short nType, short nParam);//发送给FM的命令到给FM
    bool read_config();

};

//class end


#endif // _FM_ZCJ_H

