/*
   lvweishiduIrda.h

   The definition of weishidu

   通过dht11 获取温湿度的类 并通过红外控制空调温度的 class

   2017/05/06 Steven Lian (steven.lian@gmail.com)


*/
#pragma once
#ifndef _WENSHIDU_H
#define _WENSHIDU_H

#include <FS.h>
#include "lvIrda.h"
#include "dht.h"

extern "C"{
  #include "miscCommon.h"
}

#define CONST_APP_WENSHI_VERSION 170730
#define dht_dpin 16

#define DHTLIB_OK 0
#define DHTLIB_ERROR_CHECKSUM -1
#define DHTLIB_ERROR_TIMEOUT -2

#define CONST_DHT_DEVICE_TYPE_DHT11 11
#define CONST_DHT_DEVICE_TYPE_DHT22 22
#define CONST_DHT_DELAY_TIME 250
#define CONST_DHT_TRY_TIMES 8

#define CONST_APP_WENSHIDU_FILE_NAME "/wenshiduconfig.ini"
#define CONST_APP_WENSHIDU_FILE_SIZE 1024
#define CONST_APP_TEMP_BIG_BUFFLEN 112

#define CONST_WENSHIDU_DEC_FORMAT_LENGTH 8
#define CONST_WENDU_CHANGE_ALARM_VALUE 3
#define CONST_SHIDU_CHANGE_ALARM_VALUE 10
#define CONST_MAX_IRDA_SUB_COMMAND_LEN 2
#define CONST_IRDA_SUB_COMMAND_LENGTH 100

#define CONST_WENDU_MAX_OFFSET_VAL 5
#define CONST_SHIDU_MAX_OFFSET_VAL 50

#define CONST_WENDU_LOW_VAL -100
#define CONST_WENDU_HIGH_VAL 100

#define CONST_SHIDU_LOW_VAL 0
#define CONST_SHIDU_HIGH_VAL 100

#define CONST_APP_WENSHI_CMD_TYPE_WENDU_LOW 1
#define CONST_APP_WENSHI_CMD_TYPE_WENDU_HIGH 2
#define CONST_APP_WENSHI_CMD_TYPE_SHIDU_LOW 3
#define CONST_APP_WENSHI_CMD_TYPE_SHIDU_HIGH 4

#define CONST_APP_WENSHI_MAX_SAME_COMMAND_COUNT 5

#define CONST_APP_WENSHI_803_CODE_VER_NORMAL 1
#define CONST_APP_WENSHI_803_CODE_VER_PINGPANG 2


//#define CONST_CHECK_ACTION_CYCLE 1000L*60*6  //默认6分钟检测一次温度判断是否需要有动作
//#define CONST_CHECK_ACTION_CYCLE 1000L*30  //测试0.5分钟检测一次温度判断是否需要有动作
#define CONST_CHECK_ACTION_CYCLE 1000L*60*2  //默认2分钟检测一次温度判断是否需要有动作

#define CONST_MIN_803_DATA_LEN 6+12

#define CONST_APP_WENSHIDU_CMD_RECORD_LIST_LEN 10

class wenshidu
{
  public:
    //data
    short nWenduInt;
    short nWenduDec;
    short nShiduInt;
    short nShiduDec;
    short nPreWenduInt;
    short nPreShiduInt;
    char decWendu[CONST_WENSHIDU_DEC_FORMAT_LENGTH]; //dec格式温度值
    char decShidu[CONST_WENSHIDU_DEC_FORMAT_LENGTH];

    uint8_t autoControlEnable;
    
    uint8_t record[CONST_APP_WENSHIDU_CMD_RECORD_LIST_LEN+2];
    uint8_t recordPos;
    
    //function
    unsigned short get_version();
    //为了节省内存， 由外部函数定义缓冲区，并给出缓冲区指针
    short begin(LVIrda *irPtr,char *dataPtr);
    short read();
    uint8_t devReady();

    
    //温度校准
    bool decode_802_command(char *ptr);
    //温度校准
    bool decode_805_command(char *ptr);
    //温湿度范围控制指令下发
    bool decode_803_command(char *ptr);
    //温湿度检查控制周期 ulCheckActionCycleTick 的调整
    bool decode_804_command(char *ptr);

    //乒乓键类型检测
    unsigned short set_curr_power(unsigned short power); //需要外部程序把当前功率给出
    
    void clean_record();
    short get_wenduOffset();
    short get_shiduOffset();
    
    short get_wenduAdjust();
    short get_shiduAdjust();
    
    void debug_print_info();
    
    bool bLoad_config();
    bool bSave_config();

  private:
    //data
    dht DHT;

    unsigned short version;

    uint8_t deviceReady; //DHT设备是否准备好，=0,没有准备好， =11 DHT11设备， =22，DHT22设备
    
    short nWenduOffset;  //温度修正值 +1，说明实际温度比测量温度高1度
    short nShiduOffset;  //湿度修正值

    short nWenduAdjust;  //温度控制修正值 +1，说明实际温度比测量温度高1度
    short nShiduAdjust;  //湿度控制修正值
    
    //803 wendu shidu control command 
    uint8_t cmdCodeVer; //803命令版本1，非乒乓键类， 2，乒乓键类型
    
    bool valid; //控制是否打开， 如果为false,无需控制
    uint8_t wenduEnable; //温度控制开关
    uint8_t shiduEnable; //湿度控制开关
    uint8_t lastCommandType; //a record for last command
    uint8_t lastCommandCount; //count for same command;
    
    short sameCMDReactionCount; //重复若干次以后重新执行同样的命令
    unsigned long ulCheckActionCycleTick;
    unsigned long ulCheckActionCycleTimeInMS;
    
    LVIrda *irdaPtr;

    char *dataBuffPtr;//extern big buff 

    short fixCodeLen[CONST_IRDA_MAX_FIXCODELEN];
    
    short nWenduLow;
    short nWenduHigh;
    short nWenduActionLen;
    char wenduLowActionString[CONST_MAX_IRDA_SUB_COMMAND_LEN][CONST_IRDA_SUB_COMMAND_LENGTH];    
    char wenduHighActionString[CONST_MAX_IRDA_SUB_COMMAND_LEN][CONST_IRDA_SUB_COMMAND_LENGTH];    
    
    short nShiduLow;
    short nShiduHigh;
    short nShiduActionLen;
    char shiduLowActionString[CONST_MAX_IRDA_SUB_COMMAND_LEN][CONST_IRDA_SUB_COMMAND_LENGTH];
    char shiduHighActionString[CONST_MAX_IRDA_SUB_COMMAND_LEN][CONST_IRDA_SUB_COMMAND_LENGTH];

    //乒乓键类型检测,通过功率检测
    unsigned short currPower;
    
    bool bLastCommanIfActived; //最后一个命令是否正确执行，true==yes

    char chWenduLowOperator; //判断方式">","<",...
    char chWenduHighOperator;
    char chShiduLowOperator;
    char chShiduHighOperator;
    
    short nWenduLowPowerVal; //判断的功率值
    short nWenduHighPowerVal;
    short nShiduLowPowerVal;
    short nShiduHighPowerVal;
    
    unsigned long ulActivedCheckTimeInMs; //命令多长时间后检查。
    unsigned long ulActivedCheckTick; //命令多长时间后检查。
    
    //function
    uint8_t isDHT11or22();
    bool _testDHT11or12(uint8_t type);
    bool if_need_to_take_action(uint8_t commandType);
    void check_status_action();
    void check_action_result();
    bool check_operator_power(bool status,char chOperator,unsigned short nPower,unsigned short nThreshold);
    
    char *get_str_split(char *ptr, char **spData, char chSplit);
    
    void add_record(uint8_t type);
    
};

#endif // _WENSHIDU_H
