/*
   LoRaMacDef.h
   
   The definition of LoRa struct 
   
   2017/06/12  Steven Lian (steven.lian@gmail.com)   
   
*/

#pragma once
#ifndef _LoRaMacDef_H
#define _LoRaMacDef_H

#include <arduino.h>


/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20170406
/*--------------------------------------------------------------------------------*/
//termianl, host master,host slave 终端模式，主机主模式，主机从模式
#define LORA_MAC_HOST_FLAG_CLIENT 0
#define LORA_MAC_HOST_FLAG_MASTER 1
#define LORA_MAC_HOST_FLAG_SLAVE 2

#define LORA_MAC_CHECK_HOST_MASTER_SLAVE_TIME_inMS (1000*30) //主机开机30秒检测是否有主机master存在，没有，自己变成主机master
#define LORA_MAC_CHECK_CONFIG_INFORMATION_TIME_inMS (1000*40) //终端开机40秒工作在默认配置频道，检测是否有频道配置机存在
#define LORA_MAC_CHECK_HOST_EXIST_TIME_inMS (1000*600) //终端检测是否有主机master存在

//common cmd list
#define LORA_CMD_NULL_CMD 0 //空命令,无需解释和执行
#define LORA_CMD_EXTEND_CMD 1    //扩展命令,用于避免两个字节的命令类型不够的情况
#define LORA_CMD_ACK_SUCCESS 2 //上次命令的应答，成功
#define LORA_CMD_ACK_FAIL 3 //上次命令的应答，失败

#define LORA_CMD_CONFIG_CHANNEL_BROARDCAST  10   //配置频道信息广播
#define LORA_CMD_HOST_INFO_BROARDCAST  11   //HOST 信息广播
#define LORA_CMD_REGISTRATION_CLEANUP  12   //服务登记清除，设备需要重新登记 
#define LORA_CMD_REGISTRATION_REQUEST  13   //服务登记请求，一个Terminal向router申请加入LoRa网络,同时会上传自己的唯一号 devID(IMEI),MAID(制造商代号)和PID(项目代号)
#define LORA_CMD_REGISTRATION_FEEDBACK  14   //服务登记回馈，一个Terminal向router申请加入LoRa网络,同时会下发内部地址 
#define LORA_CMD_REGISTRATION_REPORT  15   //服务登记状态报告，一个router向上级报告加入网络分配情况 
#define LORA_CMD_ADDR_REQUEST 16   //内部地址分配请求,一般是一个Terminal加入一个LoRa后,需要新分配或者获取已经分配的内部地址。
#define LORA_CMD_ADDR_FEEDBACK 17   //内部地址分配回馈,返回新分配或者获取已经分配的内部地址。
#define LORA_CMD_SYNC_CMD 18  //router 同步,Terminal发送的开始时间,以及,分组和顺序
#define LORA_CMD_SYNC_LOOP_BEGIN 19 // router 通知新一轮命令同步的开始。方便Terminal计算自己是否被router遗忘了
#define LORA_CMD_SYNC_RANDOM 20  //router 给出自由time slot,等需要发送设备请求，注册等信息的设备。 
#define LORA_CMD_SYNC_TIME 21   // router 同步时间
#define LORA_CMD_VERIFY_REQUEST 22 //要求验证对方设备的真伪，需要发送一组随机数,至少是8个字节的数据,
#define LORA_CMD_VERIFY_FEEDBACK 23 //验证反馈,回送对方8个字节的数据,和对应的MD5或者其他方式的hash结果,结果不少于8个字节。
#define LORA_CMD_DHCP_POOL_REQUEST 24 //host 主设备向上级服务器申请一个内部地址池
#define LORA_CMD_DHCP_POOL_FEEDBACK 25 //上级服务器对 host 主设备向申请一个内部地址池请求的回应,给一个段落(开始和结束)
#define LORA_CMD_ADDR_CHANGE 26 //对特定设备内部地址变更的通知,是对LORA_CMD_ADDR_REQUEST/LORA_CMD_ADDR_FEEDBACK的增强,格式和LORA_CMD_ADDR_FEEDBACK相同
#define LORA_CMD_FREQ_CHANGE 27 //对特定设备通信频率变更的通知,
#define LORA_CMD_ADDR_REJECT 28 //对特定设备拒绝服务的通知,
#define LORA_CMD_DATA_REPORT 29 //数据上报消息，或者是状态上报消息


//application cmd list
#define LORA_CMD_APP_EXTEND_CMD 256 //应用扩展命令集合
#define LORA_CMD_APP_FMRADIO 257 //FM Radion APPLICATION CMD 
#define LORA_CMD_APP_MOTOR 258 //电机控制应用 APPLICATION CMD 
#define LORA_CMD_APP_LIGHT 259 //路灯控制应用 APPLICATION CMD 
#define LORA_CMD_APP_METER 260 //路灯控制应用 APPLICATION CMD 


//data长度结构定义
#define LORA_MAC_ADDRESS_LEN 4
#define LORA_MAC_KEY_LEN 8
#define LORA_MAC_KEY_MD5 8
#define LORA_MAC_SYNC_LIST_MAX_LEN 8

#define LORA_MAC_DEFAULT_HOST_ADDR "\xC0\xA8\x00\x01"
#define LORA_MAC_BROADCAST_0000 "\x00\x00\x00\x00"
#define LORA_MAC_BROADCAST_FFFF "\xFF\xFF\xFF\xFF"
//uint8_t LORA_MAC_BROADCAST_FFFF[LORA_MAC_ADDRESS_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};

#define LORA_MAC_APPLICATION_MAX_DATA_LEN 24
// 4+4+1+1+2+16=12+24=36
#define LORA_MAC_UNION_MAX_DATA_LEN (12+LORA_MAC_APPLICATION_MAX_DATA_LEN)

// (LORA_MAC_UNION_MAX_DATA_LEN+2)*10*1000/2400=30*10*1000/2400=125ms 
// (LORA_MAC_UNION_MAX_DATA_LEN+2)*10*1000/2400=38*10*1000/2400=160ms 
#define LORA_MAC_TIME_SLOT_IN_MS 250 // 200 ms 根据 LORA_MAC_UNION_MAX_DATA_LEN 计算
#define LORA_MAC_TIME_SOLT_OFFSET_IN_MS 5 //第一个发送槽在多少时间后开始

#define LORA_MAC_SEND_BUFF_MAX_LENGTH 16
#define LORA_MAC_RECV_BUFF_MAX_LENGTH 5

#define LORA_MAC_HOST_DATA_LENGTH 248
#define LORA_MAC_HOST_DATA_ADDR_BEGIN 5

#define CONST_LoRa_NO_FEEDBACK_MAX_VAL 100 //没有反馈的最高计数值

#define LORA_MAC_AIR_TIME_SLOT_TYPE_NORMAL 0
#define LORA_MAX_AIR_TIME_SLOT_TYPE_EMERGENCY 1
#define LORA_MAC_AUTO_TOKEN_TIME_SLOT_TICK_LEN 6000 //LoRa自动查询间隔时间,单位毫秒,前提条件是发送队列空,而且发送窗口有效。 
#define LORA_MAC_TOKEN_STRATEGRY_BITLEN 16 //发送间隔策略参数的bitlen
#define LORA_MAC_TOKEN_STRATEGRY_DEFAULT_KEY 0xAAAA //默认发送策略
#define LORA_MAC_TOKEN_NORMAL_DEFAULT_LEN 8 //默认token个数
#define LORA_MAC_TOKEN_EMERGENCY_DEFAULT_LEN 8 //默认token个数
#define LORA_MAC_HOST_BROADCAST_TIME_LEN 10 //默认多少个token之后发送一个主机信息

#define LORA_MAC_DEFAULT_CONFIG_CHANNEL 410 //默认LoRa配置信道
//#define LORA_MAC_DEFAULT_WORK_CHANNEL 425 //默认LoRa配置信道
//#define LORA_MAC_DEFAULT_CONFIG_CHANNEL 410 //默认LoRa配置信道

// LORA_CMD_REGISTRATION_CLEANUP
// LORA_CMD_APP_EXTEND_CMD
// LORA_CMD_SYNC_LOOP_BEGIN
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  short exCMD;
} stDataCMD_00;

// LORA_CMD_CONFIG_CHANNEL_BROARDCAST
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  short channel01;
  short channel02;
  short channel11;
  short channel12;
} stDataCMD_LoRaChannel;

// LORA_CMD_HOST_INFO_BROARDCAST
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  unsigned long devID;
  //unsigned short MAID;
  //unsigned short PID;
  short maxCount;
  short addrCount;
  uint8_t addrBegin;
} stDataCMD_HostInfoBroardcast;

// LORA_CMD_REGISTRATION_REQUEST
// LORA_CMD_ADDR_REQUEST
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  unsigned long devID;
  unsigned short MAID;
  unsigned short PID;
  uint8_t key[LORA_MAC_KEY_LEN];
  //uint8_t val[LORA_MAC_KEY_MD5];
} stDataCMD_RegistrationRequest;


// LORA_CMD_REGISTRATION_REPORT
// LORA_CMD_ADDR_FEEDBACK
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  unsigned long devID;
  uint8_t addr[LORA_MAC_ADDRESS_LEN];
  uint8_t val[LORA_MAC_KEY_MD5];
} stDataCMD_RegistrationFeedback;

// LORA_CMD_SYNC_CMD
// LORA_CMD_SYNC_RANDOM
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  uint8_t groupAddr[LORA_MAC_ADDRESS_LEN - 1];
  uint8_t groupLen;
  uint8_t list[LORA_MAC_SYNC_LIST_MAX_LEN];
} stDataCMD_synCMD;

// LORA_CMD_SYNC_TIME
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  short year;
  short month;
  short day;
  short hour;
  short minute;
  short second;
  short wday;
  short timezone2; //2 x timezone , consider india GMT +7.5= 15
} stDataCMD_synTime;


// LORA_CMD_VERIFY_REQUEST
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  unsigned long devID;
  uint8_t key[LORA_MAC_KEY_LEN];
} stDataCMD_verifyRequestCMD;


// LORA_CMD_VERIFY_FEEDBACK
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  unsigned long devID;
  uint8_t val[LORA_MAC_KEY_MD5];
} stDataCMD_verifyFeedbackCMD;


// LORA_CMD_FREQ_CHANGE
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  short frequency;
} stDataCMD_frequencyChange;


// APPLICATION CMD
// LORA_CMD_APP_FMRADIO
// LORA_CMD_APP_DIANJI
typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t chksum;
  unsigned short CMD;
  uint8_t data[LORA_MAC_APPLICATION_MAX_DATA_LEN];
} stDataCMD_APP;


typedef union {
  stDataCMD_00 exCMD;
  stDataCMD_LoRaChannel configChannel;
  stDataCMD_HostInfoBroardcast hostInfo;
  stDataCMD_RegistrationRequest regRequest;
  stDataCMD_RegistrationFeedback regFeedback;
  stDataCMD_synCMD syncCMD;
  stDataCMD_synTime syncTime;
  stDataCMD_verifyRequestCMD verifyRegCMD;
  stDataCMD_verifyFeedbackCMD verifyFeedCMD;
  stDataCMD_APP app;
  uint8_t u8Data[LORA_MAC_UNION_MAX_DATA_LEN];
} stDataStreamUnion;

//#define CONST_LORA_DEVICE_ID_PREFIX 20170

//report to server data struct

#define CONST_LORA_REPORT_TYPE_REGISTRATION   1001
#define CONST_LORA_REPORT_TYPE_STATUS 2001
#define CONST_LORA_REPORT_TYPE_TODO 2002

//typedef struct {
//  unsigned long devID;
//  stDataStreamUnion content;
//} stLoRaDataServer;


//struct 定义请遵循long>short>char 的顺序安排。或者在其中填 uint8 保证对齐
typedef struct {
  unsigned long devID;
  unsigned short type;
  uint8_t addr[LORA_MAC_ADDRESS_LEN];
  unsigned short MAID;
  unsigned short PID;
} stLoRaRptRegistration;

typedef struct {
  unsigned long devID;
  unsigned short type;
  unsigned short CMD;
  //uint8_t addr[LORA_MAC_ADDRESS_LEN];
  uint8_t data[LORA_MAC_APPLICATION_MAX_DATA_LEN];
} stLoRaRptStatus;

typedef struct {
  unsigned long devID;
  unsigned short type;
  unsigned short CMD;
  //uint8_t addr[LORA_MAC_ADDRESS_LEN];
  uint8_t data[LORA_MAC_APPLICATION_MAX_DATA_LEN];
} stLoRaRptTODO;
 
typedef union {
  stLoRaRptRegistration reg;
  stLoRaRptStatus stat;
  stLoRaRptTODO todo;
  uint8_t u8Data[sizeof (stDataStreamUnion)];
} stLoRaDataServer;


#define CONST_LORA_HOST_DEFAULT_BATCH_LEN 8

#endif // _LoRaMacDef_H
