/*
   LIS3DSH_SPI.h

   Library to LIS3DSH_SPI AN3393_ LIS3DSH_ 3-axis digital output accelerometer SPI interface
   To follow the spec of AN3393_ LIS3DSH_ 3-axis digital output accelerometer.pdf
    ---

    Copyright (C) 2018/06/15  yang dongxiao (yangdongxiao666@gmail.com)    

*/

#pragma once
#ifndef _LIS3DSH_SPI_H
#define _LIS3DSH_SPI_H


#include <Arduino.h>

//#include <FS.h>
#include <ESPSoftwareSerial.h>
#include "ListArray.h"


extern "C" {
#include "miscCommon.h"
#include "byteSwap.h"
#include "AppleBase64.h"
}

#include "LIS3DSH_SPI_def.h"

//LIS3DSH_SPI CLASS BEGIN

#define CONST_APP_LIS3DSH_SPI_VERSION 180615
// header defining the interface of the source.

#define CONST_APP_LIS3DSH_SPI_RECV_FILE_NAME "/LIS3DSH_SPI.ini"
#define CONST_APP_LIS3DSH_SPI_RECV_FILE_SIZE 128

class LIS3DSH_SPI
{
  public:
    //data
    bool bSendBusyFlag = false;
        
    bool bStatChangeFlag; //状态变换指示器,这个是说本机状态改变
    uint8_t u8ParamChangeFlag; //一些参数条件发生了变化。例如：FSU改变了开机温度等设定。 

    short nSIOErrCount; //串口通信故障计数 
        
    unsigned long ulDataCheckTickInterval;
    unsigned long ulTimeStampTickInterval;
    // application data save cycle by minutes,default = 60 mins
    unsigned long ulSaveTick;
    //char bigBuff[CONST_APP_LIS3DSH_SPI_BASE64_BUFF_LEN+2];
    char *bigBuff;
    
    char timeStamp[LIS3DSH_TIME_STAMP_LEN];
    
    //device current status
    stDeviceDataInfo currStat;
    stDeviceDataInfo preStat;
    stDeviceDataInfo buffStat;
    
    stDeviceReport reportBuffer;
    
    //char sendbuffer[137];

    stDeviceDataInfo thresholdVal;
    
    short dataCount;
    
    uint16_t ctrl;
    uint16_t Output_DataRate ;
    uint16_t Axes_Enable;
    uint16_t SPI_Wire;
    uint16_t Self_Test;
    uint16_t Full_Scale;
    uint16_t Filter_BW;
    
    //function
    LIS3DSH_SPI();
	  bool begin(char *ptr);
    bool begin(char *ptr,uint16_t rate);
    void loop();
    bool devReady();
    short available(); //检测modbus是否有数据
    short getData(char *ptr);
    void setTimeStamp(char *YMDHMS);
    bool setFrameNum(uint32_t id);
    
    void ACCELERO_IO_Init();
    uint16_t ACCELERO_IO_WriteByte(uint16_t val1, uint16_t WriteAddr);
    
    //uint8_t CalcCheckSum(stDeviceDataInfo *dataPtr, short nLen);
    
    void spi_disable();
    void spi_enable();
    void spi_init();
    
    uint16_t getHigh(uint16_t data);
    uint16_t getLow(uint16_t data);
    uint16_t getBit(uint16_t data, uint16_t bitnum);
    
    uint16_t ReadID();
    uint16_t GetDataState();
    uint16_t ACCELERO_IO_ReadByte(uint16_t ReadAddr);
    uint16_t ACCELERO_IO_ReadData();
    uint16_t ReadACC();
    uint16_t package_data_b64();
    uint16_t package_data_0A0D();
    uint16_t package_data();
    
    uint16_t SPIx_WriteRead(uint16_t rwAddr);
    
    short decode_811_command(char *comPtr);
    
    bool bLoad_config();
    bool bSave_config();
    
    void debug_print_info();

    //功能调用
    
    //test only
  
  private:
    //data
    uint32_t selfID;
    
    bool deviceReady;
    uint8_t _ID;
    uint8_t CS_Pin = 4;// set pin 10 as the chip select
    uint8_t sclk_Pin = 14;// set pin 10 as the chip select
    uint8_t mosi_Pin = 12;// set pin 10 as the chip select
    uint8_t miso_Pin = 13;// set pin 10 as the chip select
    
    /*
    bool deviceReady;
    uint8_t _ID;
    uint8_t CS_Pin = 17;// set pin 10 as the chip select
    uint8_t sclk_Pin = 14;// set pin 10 as the chip select
    uint8_t mosi_Pin = 15;// set pin 10 as the chip select
    uint8_t miso_Pin = 16;// set pin 10 as the chip select
    */
   
    //func

    bool is_device_exist();
    
    uint16_t calc_CRC(uint8_t *in, short nLen);
    
    bool is_state_change(uint32_t currVal,uint32_t preVal,int16_t threshold);
    
};

//class end


#endif // _LIS3DSH_SPI_H

