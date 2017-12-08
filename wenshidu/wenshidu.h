/*
   weishidu.h

   The definition of weishidu

   通过dht11 获取温湿度的类 class

   2017  Steven Lian (steven.lian@gmail.com)


*/
#pragma once
#ifndef _WENSHIDU_H
#define _WENSHIDU_H

#include "dht.h"


extern "C"{
  #include "miscCommon.h"
}


#define dht_dpin 16

#define DHTLIB_OK 0
#define DHTLIB_ERROR_CHECKSUM -1
#define DHTLIB_ERROR_TIMEOUT -2


#define CONST_DHT_DEVICE_TYPE_DHT11 11
#define CONST_DHT_DEVICE_TYPE_DHT22 22
#define CONST_DHT_DELAY_TIME 250
#define CONST_DHT_TRY_TIMES 8

#define CONST_WENSHIDU_DEC_FORMAT_LENGTH 8
#define CONST_WENDU_CHANGE_ALARM_VALUE 3
#define CONST_SHIDU_CHANGE_ALARM_VALUE 10

#define CONST_WENDU_MAX_OFFSET_VAL 5

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
    char decWendu[CONST_WENSHIDU_DEC_FORMAT_LENGTH];
    char decShidu[CONST_WENSHIDU_DEC_FORMAT_LENGTH];

    //function
    short begin();
    short read();
    uint8_t devReady();
    
    void debug_print_info();
 
    bool decode_802_command(char *ptr);

  private:
    //data
    dht DHT;
    uint8_t deviceReady; //DHT设备是否准备好，=0,没有准备好， =11 DHT11设备， =22，DHT22设备
    
    short nWenduOffset;  //温度修正值 +1，说明实际温度比测量温度高1度
    short nShiduOffset;  //湿度修正值

    //function
    uint8_t isDHT11or22();
    bool _testDHT11or12(uint8_t type);

};

#endif // _WENSHIDU_H
