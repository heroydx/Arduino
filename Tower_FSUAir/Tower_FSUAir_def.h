/*
   Tower_FSUAir_def.h

   Library to support China Power FSU.
   To follow the spec of 基站智能空调控制器通讯协议 V1.0
    ---

    Copyright (C) 2017/11/15  Steven Lian (steven.lian@gmail.com)    
*/

#pragma once
#ifndef _Tower_FSUAir_DEF_H
#define _Tower_FSUAir_DEF_H


// header defining the interface of the source.

//起始位标志（START OF INFORMATION）
#define CONST_APP_FSU_SIO_SOI 0x7E
//结束码（END OF INFORMATION）
#define CONST_APP_FSU_SIO_EOI 0x0D
//通信协议版本号
#define CONST_APP_FSU_VER 0x10

//设备标识码（设备类型描述）空调控制器设备
#define CONST_APP_FSU_CID1 0x66

//CID2编码分配表
//获取模拟量量化后的数据（定点数）
#define CONST_APP_FSU_CID2_GET_ANALOG_DATA  0x42
//获取开关状态
#define CONST_APP_FSU_CID2_GET_SWITCH_STAT  0x43
//获取告警状态
#define CONST_APP_FSU_CID2_GET_WARNING_STAT 0x44
//遥控
#define CONST_APP_FSU_CID2_GET_REMOTE_CONTROL 0x45
//获取系统参数（定点数）
#define CONST_APP_FSU_CID2_GET_SYS_PARM 0x47
//设定系统参数（定点数）
#define CONST_APP_FSU_CID2_SET_SYS_PARM 0x49
//获取监测模块时间
#define CONST_APP_FSU_CID2_GET_TIME_INFO 0x4D
//设置监测模块时间
#define CONST_APP_FSU_CID2_SET_TIME_INFO 0x4E
//获取通信协议版本号
#define CONST_APP_FSU_CID2_GET_COMM_VER 0x4F
//获取设备地址
#define CONST_APP_FSU_CID2_GET_DEVID 0x50
//获取设备厂家信息
#define CONST_APP_FSU_CID2_GET_MAID 0x51
//空调控制器安装参数设置
#define CONST_APP_FSU_CID2_GET_DEV_PARM 0x80
//获取空调电量数据
#define CONST_APP_FSU_CID2_GET_METER_CURR 0x81
//读取空调电量历史数据信息
#define CONST_APP_FSU_CID2_GET_METER_HISTORY 0x86

//CID2中返回码RTN定义
//正常
#define CONST_APP_FSU_CID2_RTN_NORMAL 0x00
//VER错
#define CONST_APP_FSU_CID2_RTN_VER_ERROR 0x01
//CHKSUM错
#define CONST_APP_FSU_CID2_RTN_CHKSUM_ERROR 0x02
//LCHKSUM错
#define CONST_APP_FSU_CID2_RTN_LCHKSUM_ERROR 0x03
//CID2无效
#define CONST_APP_FSU_CID2_RTN_INVALID_CMD 0x04
//命令格式错
#define CONST_APP_FSU_CID2_RTN_FORMAT_ERROR 0x05
//无效数据
#define CONST_APP_FSU_CID2_RTN_INVALID_DATA 0x06
//红外学习成功
#define CONST_APP_FSU_CID2_RTN_IRDA_OK 0x80
//红外学习失败
#define CONST_APP_FSU_CID2_RTN_IRDA_FAIL 0x81

//表A.5　DATA_FLAG的形式
#define CONST_APP_FSU_DATA_FLAG_SWITCH_MASK 0xEF
#define CONST_APP_FSU_DATA_FLAG_WARN_MASK 0xFE

//表A.7　LENGTH的数据格式
#define CONST_APP_FSU_LENGTH_LEN 2
#define CONST_APP_FSU_LENGTH_LCHECKSUM_MASK_HIGH 0xF0
#define CONST_APP_FSU_LENGTH_LENID_MASK_HIGH 0x0F
#define CONST_APP_FSU_LENGTH_LENID_MASK_LOW 0xFF


//通信速率,buff等信息
#define CONST_APP_FSU_RECV_SIO_BAUD_DATA 9600
#define CONST_APP_FSU_RECV_SENDREV_BUFFLEN 256
#define CONST_APP_FSU_RECV_BIG_BUFFLEN 160
#define CONST_APP_FSU_RECV_SIO_DATA_BEGIN CONST_APP_FSU_SIO_SOI
#define CONST_APP_FSU_RECV_SIO_DATA_END CONST_APP_FSU_SIO_EOI


#define CONST_APP_FSU_RECV_SIO_ERROR_COUNT 5

#define CONST_APP_FSU_RECV_TIMEOUT_LEN 1000*2
#define CONST_APP_FSU_RECV_SIO_QUERY_INTERVAL  1000*60

#define CONST_APP_FSU_STAT_QUERY_INTERVAL 10*1000

#define CONST_APP_FSU_RECV_FREQ_MAX_SAVE_POS 20
#define CONST_APP_FSU_RECV_COMMAND_BUFF_LEN 10
#define CONST_APP_FSU_RECV_COMMAND_PARAM_LEN 10

#define CONST_APP_FSU_RECV_SAVE_CYCLE 1000*60*60


#define CONST_SHORT_INT_LENGTH 2
#define CONST_LONG_INT_LENGTH 4


//数据结构定义
typedef struct{
  //考虑sturct对齐原因，需要一个大小端转换代码
  uint8_t year[CONST_SHORT_INT_LENGTH];
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t second;
} stFSUDateTime;

typedef struct{
  uint8_t group;
  uint8_t type;
  uint8_t id;
  //time struct
  //考虑sturct对齐原因，需要一个大小端转换代码
  uint8_t year[CONST_SHORT_INT_LENGTH];
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t second;
  //考虑sturct对齐原因，需要一个大小端转换代码
  uint8_t dataI[CONST_SHORT_INT_LENGTH];
} stFSUCommandInfo;

typedef struct{
  uint8_t dataI[CONST_SHORT_INT_LENGTH];
  uint8_t dataFlag;
  uint8_t runState;
  uint8_t warnState;
  //time struct
  //考虑sturct对齐原因，需要一个大小端转换代码
  uint8_t year[CONST_SHORT_INT_LENGTH];
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t second;
  uint8_t dataType;
} stFSUDataInfo;

typedef struct{
  uint8_t length[CONST_APP_FSU_LENGTH_LEN];
} stFSU_LENGTHInfo;

typedef struct{
  unsigned long currMeter;
  unsigned long yesterdayMeter;
  short wendu;
  short shidu;
  uint8_t airSwitch; 
} stBaseAirInfo;

typedef struct{
  uint8_t SOI;
  uint8_t VER;
  uint8_t ADR;
  uint8_t CID1;
  uint8_t CID2;
  stFSU_LENGTHInfo LENGTH;
  uint8_t INFO;
  uint8_t CHKSUM;
  uint8_t EOI;
} stFSUSIODesc;

#endif // _Tower_FSUAir_DEF_H

