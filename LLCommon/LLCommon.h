#pragma once
#ifndef LLCommon_H
#define LLCommon_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

#include "ESP8266ClientDB.h"

#include "ListArray.h"
#include <FS.h>
#include "Timenew.h"

#include "miscLED.h"

extern "C" {
#include "simpleEncode.h"
#include "miscCommon.h"
}

//Modified by Steven Lian on 2017/05/31
/*
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

//define data

#define CONST_LLCOMMON_VERSION 170910

/*====================================================================================*/
//COMMON_PART_BEGIN Ver=20161206
/*====================================================================================*/

/*--------------------------------------------------------------------------------*/
//COMMON_DATA_PART_BEGIN
/*--------------------------------------------------------------------------------*/
//different project should use the different Version data
#define CONST_STANDARD_SIZE 1
#define CONST_SINGLE_SIZE 1
#define CONST_DOUBLE_SIZE 2

//define with irda function
//#define WITH_IRDA_FUNCTION 1
//#ifdef WITH_IRDA_FUNCTION

//#endif

// to define if need the PUSH function
#define NEED_PUSH_FUNCTION 1
#ifdef NEED_PUSH_FUNCTION

#endif


#define NEED_REGULAR_CONNECTION_FUNTION 1

#ifdef NEED_REGULAR_CONNECTION_FUNTION
#define CONST_CONNINTERVAL_TICK 125*1000
#else
#define CONST_CONNINTERVAL_TICK 20000*1000
#endif

#define CONST_CONNINTERVAL_SHORT_TICK 1000

#define CONST_MIN_POST_INTERVAL_TICK 500

//some time paramater,very important

#define CONST_MIN_DELAY_TIME_FOR_PUSH_DATA 50
#define CONST_MIN_DELAY_TIME_FOR_POST_DATA 250


//common struct,unicon definition
#define CONST_URL_LENGTH 32
#define CONST_ADDR_LENGTH 100

#define CONST_DEFAULT_SERVER_URL_DATA "/dev"
#define CONST_DEFAULT_SERVER_ADDR_DATA "106.75.105.138"
#define CONST_DEFAULT_SERVER_PORT_DATA 80
#define CONST_DEFAULT_PUSH_URL_DATA "/devps"
#define CONST_DEFAULT_UDP_ADDR_DATA 10080
#define CONST_DEFAULT_BIND_ADDR_DATA "255.255.255.255"
#define CONST_DEFAULT_BIND_SEND_PORT_DATA 7788
#define CONST_DEFAULT_BIND_RECV_PORT_DATA 8877

#define CONST_DEFAULT_WIFI_SSID_DATA "HiWiFi_BUPT"
#define CONST_DEFAULT_WIFI_PASS_DATA "8765432101"

#define CONST_APP_LORA_MAX_REPORT_PKG_LEN 50

typedef struct  {
  char url[CONST_URL_LENGTH ];
  char addr[CONST_ADDR_LENGTH + 1];
  unsigned short port;
  char pushUrl[CONST_URL_LENGTH ];
  char udpAddr[CONST_ADDR_LENGTH + 1];
  unsigned short udpPort;
  char bindAddr[CONST_ADDR_LENGTH + 1];
  unsigned short bindPort;
  unsigned short recvPort;
  unsigned short loraChannel;
} stServerAddr;

#define CONST_WIFI_SSID_LENGTH 64
#define CONST_WIFI_PASS_LENGTH 64

typedef struct {
  char ssid[CONST_WIFI_SSID_LENGTH + 1];
  char pass[CONST_WIFI_PASS_LENGTH + 1];
} stWiFi;

#define CONST_SYS_VERSION_LENGTH 64
#define CONST_SYS_MAID_LENGTH 5
#define CONST_SYS_PID_LENGTH 5
#define CONST_SYS_CAT_LENGTH 5
#define CONST_MAC_ADDR_LEN 6
#define CONST_MAC_ADDR_ASC_LEN 17

typedef struct {
  char Version[CONST_SYS_VERSION_LENGTH];
  char SWVN[CONST_SYS_VERSION_LENGTH];
  char HWVN[CONST_SYS_VERSION_LENGTH];
  char MAID[CONST_SYS_MAID_LENGTH + 1];
  char PID[CONST_SYS_PID_LENGTH + 1];
  char CAT[CONST_SYS_CAT_LENGTH + 1];
  uint8_t bssid[CONST_MAC_ADDR_LEN + 1];
  uint8_t macAddr[CONST_MAC_ADDR_LEN + 1];
  uint8_t userMac[CONST_MAC_ADDR_LEN + 1];
  unsigned short SUBPID;
  stServerAddr server;
  stWiFi wifi;
} stDeviceCoreDataStruct;


//define command data type
#define CONST_CMD_LENGTH 4
#define CONST_CMD_2001_STR "2001"

#define CONST_CMD_2001 2001
#define CONST_CMD_2002 2002
#define CONST_CMD_1001 1001
#define CONST_CMD_9999 9999

//double check gnTcpCommandBusy time lengh = 10 seconds
#define CONST_CMD_TCP_DOUBLE_CHECK_TIME_LEN 10000

// define loop main status
#define CONST_WIFI_MODE WIFI_STA
#define INIT_STATE          0
#define SMARTCONFIG_START_STATE   1
#define SMARTCONFIG_DONE_STATE    2
#define LV_CONFIG_START_STATE   3
#define LV_CONFIG_DONE_STATE    4
#define WAIT_CONNECTION_STATE   5
#define WAIT_FOR_THIRD_PARTY_DEVICE 6 
#define CONNECTED_STATE       7
#define FACTORY_TEST_FINISH_LOOP     8


//buffer,file etc length
#define CONST_CONFIGFILE_SIZE 1024
#define CONST_UDP_BUFF_SIZE 512
#define CONST_PUSH_BUFF_SIZE 512
#define CONST_BIG_BUFF_SIZE 1024*4

#define CONFIG_FILE_NAME "/config.json"
#define IMEI_FILE_NAME "/imei.json"
#define HWVERSION_FILE_NAME "/hwversion.data"

#define CONST_SEED_LENGTH 6

#define CONST_IMEI_LENGTH 20



typedef union {
  char *chpData; /* used for all internal sprintf, tcp communication etc.*/
  unsigned char *chupData;
  uint8_t *u8pData;
  short  *npData;
  long  *lpData;
  unsigned long  *ulpData;
} stBigBuffUnion;


//tick data
#define CONST_TRANSFER_TICK_LEN 1000
#define CONST_SMARTCONFIG_SWITCH 60 //1 minute switch from smart config to connect
#define CONST_UDPCONFIG_SWITCH 5
#define CONST_SMATCONFIG_REBOOT_COUNT 60*5 //5 minutes 重启设备

// a time interrupt
//os_timer_t ghTimer100ms;

//error handle
#define CONST_MAX_WIFI_FAIL 5

#define CONST_MAX_NET_WRITE 3
#define CONST_MAX_NET_POSTFAIL 3
#define CONST_MAX_NET_PUSHFAIL 5


//TCP UDP data



//the time struct, it is the timer system in this project.
typedef struct {
  short year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t wday;
  int8_t timezone2; //2 x timezone , consider india GMT +7.5= 15
  char timeString[20];
  bool synced;
} LVTime;



//commmand list struct
#define CONST_SERVER_COMMAND_LIST_LEN 40

typedef struct {
  short command;
  unsigned long tick;
  unsigned long beginTick;
  unsigned long endTick;
} stLVCommmandList;

#define CONST_REBOOT_HOUR 2

//是否是开机 0-- 非开机或者开机结束, 1= 开机(上电或者内部重启)
#define CONST_POWERON_FLAG_NO 0
#define CONST_POWERON_FLAG_YES 1
//#define CONST_POWERON_TYPE_JUMP 2
//#define CONST_POWERON_TYPE_RESTART 3


//encode and decode 
#define _DEF_COMM_CODING_TYPE_LENGTH  12

#define _DEF_COMM_CODING_TYPE_JSON  "JSON"
#define _DEF_COMM_CODING_TYPE_BCD  "BCD"
#define _DEF_COMM_CODING_TYPE_B6400  "B64.00"
#define _DEF_COMM_CODING_TYPE_B6401  "B64.01"
#define _DEF_COMM_CODING_TYPE_B6402  "B64.02"
#define _DEF_COMM_CODING_TYPE_B64C1  "B64.C1"

/*--------------------------------------------------------------------------------*/
//COMMON_DATA_PART_END
/*--------------------------------------------------------------------------------*/

class LLCommon
{
  public:
    //data
    char IMEI[CONST_IMEI_LENGTH + 1];//=devID device ID
    uint8_t IMEILen;

    uint8_t SN;

    unsigned long glLastCommandTick;
    unsigned long glConnIntervalTick;

    //device info
    stDeviceCoreDataStruct gsDeviceInfo;  // the device common information, include Server info, wifi info, etc.
    stBigBuffUnion gBigBuff, gCommandBuff; // the buffer for application, gBigBuff is for input, the gCommandBuff is for output

    short gnServerCommand;// = 0; the next command to server refer to 3001 command struct
    //这是一个发送到服务器的命令的队列，可以支持多个命令的储存和控制
    stLVCommmandList sServerCommand;

    uint8_t gnExecActionID;// = 0; the next actionID to server
    uint8_t gnWiFiStat;// = 0; Current WiFi connection Status

    unsigned char gchMainState;// 0 INIT_STATE  initial state, status machine flag

    //uint8_t gnSendDatatStaus;//=0,the data is empty
    uint8_t gu8PowerOn;//=0,the machine is power on;

    bool gbConfigOK;// = false; the flag for reading config file, true=success, false=failure

    bool gbSmartConfigFlag;//开机是否有过SmartConfig, 1==是, 0==没有);

    bool bConnectWiFi;// = false; connect to wifi status , true = connected, false= not connected
    
    bool bKeepConnection;// default=true,如果通信失败则重启; =false,失败不重启。 


    LVTime gTime;// = {2016,  11,  16,  12, 0, 0, 0, -1}; system time
    LVTime gPreTime;

    ESP8266Client *ghpTcpCommand; // the TCP connection to server
#ifdef NEED_PUSH_FUNCTION
    ESP8266Client *ghpTcpPush;  // the TCP push connection channel to server
#endif
    ESP8266Client *ghpUdpLocal; // the local UDP connection

    char codingType[_DEF_COMM_CODING_TYPE_LENGTH]; // encode & decode type

    //int anFreeHeapInfo[20];

    //function
    LLCommon(); //constructor
    LLCommon(int nType); //constructor
    virtual ~LLCommon(); // de-constructor
    void vInit_common_data(int nType);

    void mainsetup(); // the main setup function for arduino,must be called in setup
    void mainloop();  // the main loop function for arduino,must be called in loop
    //convert the HTTP header DATE month to int format

    //time and interval control
    void vSync_time_fromServer(char achYMD[]);//sync time from server, default timezone = GMT +8;
    void vSyncTime_998(char * achYMDHMS);// sync time from server with timezone;
    void vGet_time();// get system time, current time data in gTime;
    
    //void vReset_interval(unsigned long &resetTick); // reset the interval time in ms  data;
    //unsigned long ulGet_interval(unsigned long checkTick); //get the time tick from last reset
    //void vReset_microseconds(unsigned long &resetTick);  // reset the interval time in us data
    //unsigned long ulGet_microseconds(unsigned long checkTick);// get the time tick from last reset (us)
    void vTimer_interrupt100ms(void *pArg); // a 100 ms  regular call function
    // debug
    void debug_print_time();
    void debug_print_wifi_status();
    void debug_print_info();
    //void debug_print_serverCommand();


    //WiFi control
    int nChange_WiFi_Mode(int stat);

    //network, http ...
    int nCommonPOST(char achData[]); // post a string to server
    int nCommonPOST(char achData[], bool encodeFlag); // post a string to server
    void vRefresh_data_status(); // send a update data of 3001
    void push_cmd(short cmd);
    void push_cmd(short cmd, short beginTime, short endTime);
    short pop_cmd();
    //void vRefresh_data_status(int seconds); // send a update data of 3001 after seoncds
    
    //encode & decode function
    short decode_data(char achData[],char achBuff[]);
    short encode_data(char  *codingType,short nLen, char achData[],char achBuff[]);
    
    unsigned short get_version();
    void vSystem_restart(short reason);


  protected:

  private:
    // data
    unsigned short version;
    
    char gachServerSeed[CONST_SEED_LENGTH + 1]; //Server Seeds

    int gnStateTransferTicksCount ;
    unsigned long glStateTransferTicks ;
    unsigned long glMinPostIntervalTick;

    //20 minutes 20分钟
#define CONST_ACTIONID_909_TICK_LEN 1000*60*20
    unsigned long glActionID909Tick;

    unsigned long gl100msLoopTick ;/* a common 100 ms loop */

    unsigned long ulTimeSecondTick;

    unsigned long ulMinPOSTTicks;

#define CONST_FORCE_POST_CHECK_TIME_TICK_COUNT 3
#define CONST_FORCE_POST_CHECK_TIME_TICK_LENGTH 1000*60*30
    unsigned long ulForcePOSTCheckTimeInMs;
    unsigned long ulForcePOSTCheckTick;

    unsigned long ulDoubleCheckTCPTick;
    //short nDoubleCheckTCPFlagCount;

#ifdef NEED_PUSH_FUNCTION
#define PUSH_TIMEOUT_INTERVAL   60000L
    unsigned long glLastPushTick;
    uint8_t gnTcpPushBusy ;
#endif

    //int gnStatusRelay ;

    short gnWiFiFailCount ;

    short gnNetPostFailCount;
    short gnNetPushFailCount;
    //TCP 连接的等待时长 最多10秒
#define CONST_TCP_COMMAND_BUSY_TIME_OUT 1
    short gnTcpCommandBusy ; //即表明tcp通道占用,也是发送后检测post回馈消息的标志。收到回馈后清零。

    unsigned int guLenHttp;
    size_t gnLenPush;
    //uint8_t data_push[PUSH_DATA_SIZE];
    size_t gnLenCommand;
    //uint8_t data_command[COMMAND_DATA_SIZE];


    uint8_t gu8LEDStatus;// = 0;
    //uint8_t gu8LEDPreStatus = 0;
    uint8_t gu8LEDLoopCount;// = 0;
    uint8_t gu8LEDDispCount;// = 0;
    
    stLedDispStruct asLedDisplay[CONST_LED_STRUCT_LEN];
    //  = {
    //      {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {3, 6, 9, 19}}, // 300ms on, 300ms off, 300ms on, 1000ms off
    //      {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {2, 4, 6, 18}}, // 200ms on, 200ms off, 200ms on, 1200ms off
    //      {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {4, 8, 12, 16 }}, //400ms on, 400ms off,400ms on, 400ms off
    //      {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 2, {10, 20}}, //1000ms on, 1000ms off
    //      {1, CONST_LED_COLOR_RED_MASK, 2, {2, 8}}, //500ms on, 500ms off
    //      {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {5, 10, 15, 20}}, // 500ms on, 500ms off, 500ms on, 500ms off
    //    };

    short gnTimeSyncUpdate;// = 0;

    short gnRestartMin;// = 0;// the highest bit is the flag of reboot
    short nRestartReason;
    

    //int content_length = 0;


    //function
    bool func_cmd_901(char achParam[], char achInst[]);
    bool func_cmd_904(char achParam[], char achInst[]);
    bool func_cmd_905(char achParam[], char achInst[]);


    void func_INIT_STATE();
    void func_SMARTCONFIG_START_STATE();
    void func_SMARTCONFIG_DONE_STATE();
    void func_LL_CONFIG_START_STATE();
    void func_LL_CONFIG_DONE_STATE();
    void func_WAIT_FOR_THIRD_PARTY_DEVICE();
    void func_WAIT_CONNECTION_STATE();
    void func_CONNECTED_STATE();
    void func_FACTORY_TEST_FINISH_LOOP();
    
    void func_100ms_loop();
    void func_second_loop();
    void func_minute_loop();
    void func_hour_loop();
    void func_day_loop();

    // string format, convert, ...
    short str2month(char *strMonth);


    //LED control
    void vDisplay_LED_color(void);

    //smartconfig, network config
    void vLVConfig_udp_send(ESP8266Client *hUDP);

    //WiFi control
    void vDetermine_NetworkStatus(int nRet);
    //network, http ...
    //int nCommonPOST( char achUrl[], char achAddr[], char achData[], ESP8266Client *hTCP);
    bool bSend_registration(bool firstReg, char achSeed[]);
    bool bSend_registration(char achReqID[],bool firstReg, char achSeed[]);
#ifdef NEED_PUSH_FUNCTION
    bool bSend_Push();
#endif
    bool bHandle_recvLine_main(char achData[]);
    bool bHandle_recvLine_date(char achData[]);
    bool bHandle_recvLine_body(char achData[]);
    
    int nhandle_server_response(uint8_t *data, size_t len);
    bool bExec_serverCommand(int nActionID, char  achParam[], char achInst[]);

    //config file control
    bool bLoad_config();
    bool bSave_config();
    bool bLoad_IMEI();
    bool bSave_IMEI();
    bool bLoad_HWVersion();
    bool bSave_HWVersion();
#define CONST_ENCODE_STRING_LEN 20
    bool bEncode(char *achpData);
    

};


extern miscLED LEDControl;
//void vGenerate_IMEI();
//void vApplication_setup_call();
void vThird_party_setup_call();
void vApplication_wait_loop_call();
void vApplication_connected_loop_call();
void vApplication_read_data();
void application_POST_call(int cmd);
int nExec_application_Command(int nActionID, char  achParam[], char achInst[]);
int nExec_transfer_Command(char achData[]);
bool bSend_status();
bool bSend_feedback_OK();
bool bLoad_application_data();
bool bSave_application_data();
#endif