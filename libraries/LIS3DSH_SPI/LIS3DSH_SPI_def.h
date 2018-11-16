/*
   LIS3DSH_SPI_def.h

   Library to LIS3DSH_SPI AN3393_ LIS3DSH_ 3-axis digital output accelerometer SPI interface
   To follow the spec of AN3393_ LIS3DSH_ 3-axis digital output accelerometer.pdf
    ---

    Copyright (C) 2018/06/15  yang dongxiao (yangdongxiao666@gmail.com)    
*/

#pragma once
#ifndef _LIS3DSH_SPI_def_H
#define _LIS3DSH_SPI_def_H


// header defining the interface of the source.
#define  LIS3DSH_OUT_T_ADDR  0x0C  /*tempreture address*/


#define  LIS3DSH_DATARATE_POWERDOWN  0x00 //#/* Power Down Mode*/
#define  LIS3DSH_DATARATE_3_125  0x10 //#/* 3.125 Hz Normal Mode */
#define  LIS3DSH_DATARATE_6_25  0x20 //#/* 6.25 Hz Normal Mode */
#define  LIS3DSH_DATARATE_12_5  0x30 //#/* 12.5 Hz Normal Mode */
#define  LIS3DSH_DATARATE_25  0x40 //#/* 25 Hz Normal Mode */
#define  LIS3DSH_DATARATE_50  0x50 //#/* 50 Hz Normal Mode */
#define  LIS3DSH_DATARATE_100  0x60 //#/* 100 Hz Normal Mode */
#define  LIS3DSH_DATARATE_400  0x70 //#/* 400 Hz Normal Mode */
#define  LIS3DSH_DATARATE_800  0x80 //#/* 800 Hz Normal Mode */
#define  LIS3DSH_DATARATE_1600  0x90 //#/* 1600 Hz Normal Mode */


#define  LIS3DSH_STATUS_ADDR  0x27
#define  LIS3DSH_OUT_X_L_ADDR  0x28
#define  LIS3DSH_OUT_X_H_ADDR  0x29
#define  LIS3DSH_OUT_Y_L_ADDR  0x2A
#define  LIS3DSH_OUT_Y_H_ADDR  0x2B
#define  LIS3DSH_OUT_Z_L_ADDR  0x2C
#define  LIS3DSH_OUT_Z_H_ADDR  0x2D


#define  LIS3DSH_FULLSCALE_2  0x00 //#/* 2 g */
#define  LIS3DSH_FULLSCALE_4  0x08 //#/* 4 g */
#define  LIS3DSH_FULLSCALE_6  0x10 //#/* 6 g */
#define  LIS3DSH_FULLSCALE_8  0x18 //#/* 8 g */
#define  LIS3DSH_FULLSCALE_16  0x20 //#/* 16 g */
#define  LIS3DSH__FULLSCALE_SELECTION  0x38
#define  LIS3DSH_SENSITIVITY_0_06G  0.06
#define  LIS3DSH_SENSITIVITY_0_12G  0.12
#define  LIS3DSH_SENSITIVITY_0_18G  0.18
#define  LIS3DSH_SENSITIVITY_0_24G  0.24
#define  LIS3DSH_SENSITIVITY_0_73G  0.73
#define  LIS3DSH_CTRL_REG4_ADDR  0x20
#define  LIS3DSH_CTRL_REG5_ADDR  0x24
#define  LIS3DSH_CTRL_REG6_ADDR  0x25
#define  LIS3DSH_BOOT_NORMALMODE  0x00
#define  LIS3DSH_BOOT_FORCED  0x80
#define  LIS3DSH_X_ENABLE  0x01
#define  LIS3DSH_Y_ENABLE  0x02
#define  LIS3DSH_Z_ENABLE  0x04
#define  LIS3DSH_XYZ_ENABLE  0x07
#define  LIS3DSH_SERIALINTERFACE_4WIRE  0x00
#define  LIS3DSH_SERIALINTERFACE_3WIRE  0x01
#define  LIS3DSH_SELFTEST_NORMAL  0x00
#define  LIS3DSH_SELFTEST_P  0x02
#define  LIS3DSH_SELFTEST_M  0x04
#define  LIS3DSH_FILTER_BW_800  0x00
#define  LIS3DSH_FILTER_BW_40  0x08
#define  LIS3DSH_FILTER_BW_200  0x10
#define  LIS3DSH_FILTER_BW_50  0x18
#define  LIS3DSH_ST1_1_ADDR  0x40
#define  LIS3DSH_ST1_2_ADDR  0x41
#define  LIS3DSH_ST1_3_ADDR  0x42
#define  LIS3DSH_ST1_4_ADDR  0x43
#define  LIS3DSH_ST1_5_ADDR  0x44
#define  LIS3DSH_ST1_6_ADDR  0x45
#define  LIS3DSH_ST1_7_ADDR  0x46
#define  LIS3DSH_ST1_8_ADDR  0x47
#define  LIS3DSH_ST1_9_ADDR  0x48
#define  LIS3DSH_ST1_10_ADDR  0x49
#define  LIS3DSH_ST1_11_ADDR  0x4A
#define  LIS3DSH_ST1_12_ADDR  0x4B
#define  LIS3DSH_ST1_13_ADDR  0x4C
#define  LIS3DSH_ST1_14_ADDR  0x4D
#define  LIS3DSH_ST1_15_ADDR  0x4E
#define  LIS3DSH_ST1_16_ADDR  0x4F
#define  LIS3DSH_TIM4_1_ADDR  0x50
#define  LIS3DSH_TIM3_1_ADDR  0x51
#define  LIS3DSH_TIM2_1_L_ADDR  0x52
#define  LIS3DSH_TIM2_1_H_ADDR  0x53
#define  LIS3DSH_TIM1_1_L_ADDR  0x54
#define  LIS3DSH_TIM1_1_H_ADDR  0x55
#define  LIS3DSH_THRS2_1_ADDR  0x56
#define  LIS3DSH_THRS1_1_ADDR  0x57
#define  IS3DSH_MASK1_B_ADDR  0x59
#define  LIS3DSH_MASK1_A_ADDR  0x5A
#define  LIS3DSH_SETT1_ADDR  0x5B
#define  LIS3DSH_PR1_ADDR  0x5C
#define  LIS3DSH_TC1_L_ADDR  0x5D
#define  LIS3DSH_TC1_H_ADDR  0x5E
#define  LIS3DSH_OUTS1_ADDR  0x5F
#define  LIS3DSH_ST2_1_ADDR  0x60
#define  LIS3DSH_ST2_2_ADDR  0x61
#define  LIS3DSH_ST2_3_ADDR  0x62
#define  LIS3DSH_ST2_4_ADDR  0x63
#define  LIS3DSH_ST2_5_ADDR  0x64
#define  LIS3DSH_ST2_6_ADDR  0x65
#define  LIS3DSH_ST2_7_ADDR  0x66
#define  LIS3DSH_ST2_8_ADDR  0x67
#define  LIS3DSH_ST2_9_ADDR  0x68
#define  LIS3DSH_ST2_10_ADDR  0x69
#define  LIS3DSH_ST2_11_ADDR  0x6A
#define  LIS3DSH_ST2_12_ADDR  0x6B
#define  LIS3DSH_ST2_13_ADDR  0x6C
#define  LIS3DSH_ST2_14_ADDR  0x6D
#define  LIS3DSH_ST2_15_ADDR  0x6E
#define  LIS3DSH_ST2_16_ADDR  0x6F
#define  LIS3DSH_MASK2_A_ADDR  0x7A
#define  LIS3DSH_SETT2_ADDR  0x7B

//threshold value
#define CONST_APP_LIS3DSH_SPI_THRESHOLD_AX 5000
#define CONST_APP_LIS3DSH_SPI_THRESHOLD_AY 5000
#define CONST_APP_LIS3DSH_SPI_THRESHOLD_AZ 5000

#define CONST_APP_LIS3DSH_SPI_DEFAULT_DATA_CHECK_TICKS 1
#define CONST_APP_LIS3DSH_SPI_BIG_BUFF_LEN 128
#define CONST_APP_LIS3DSH_SPI_BASE64_BUFF_LEN 2048

#define RECEIVE_BUFFER_COUNT 40
#define SEND_BUFFER_COUNT 3
#define SEND_DATA_BATCH_COUNT (RECEIVE_BUFFER_COUNT/2)
#define LIS3DSH_TIME_STAMP_LEN 20

//数据结构定义
typedef struct{
  uint32_t microSeconds;
  short year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
}stTimeStamp;

typedef struct{
  //uint16_t relMicroSeconds;   //相对时间
  uint8_t AX_H;
  uint8_t AX_L;
  uint8_t AY_H;
  uint8_t AY_L;
  uint8_t AZ_H;
  uint8_t AZ_L;
} stDeviceDataInfo;

typedef struct {
  uint8_t frameBegin[2];
  uint8_t frameLen[2];
  uint8_t frameType;
  uint8_t frameFill;
  uint8_t frameNum[8];
  stDeviceDataInfo frameData[SEND_DATA_BATCH_COUNT];
  uint16_t frameCRC;
  uint8_t frameEnd[2];
} stDeviceReport;
#endif // _LIS3DSH_SPI_def_H

