/*
   LoRa_AS62.h

   Library to support AS62 LoRa chipset.

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
#include <FS.h>
#include <ESPSoftwareSerial.h>

extern "C" {
  //#include "AppleBase64.h"
#include "slipcrc8.h"
#include "simpleEncode.h"
#include "miscCommon.h"
}


// header defining the interface of the source.
#ifndef _LORA_AS62_H
#define _LORA_AS62_H

#define CONST_LORA_AS62_VERSION 171008
#define CONST_LORA_HARDWARE_DEVICE "AS62"

//ESP
#define CONST_LORA_AS62_RX_PORT 4  //D2
#define CONST_LORA_AS62_TX_PORT 2  //D4
#define CONST_LORA_AS62_M0_PORT 16
#define CONST_LORA_AS62_M1_PORT 16
//#define CONST_LORA_AS62_M1_PORT 5
//#define CONST_LORA_AS62_AUX_PORT 12

//AS62 communication mode
#define CONST_LORA_AS62_COMM_MODE_NORMAL 0
#define CONST_LORA_AS62_COMM_MODE_WAKEUP 1
#define CONST_LORA_AS62_COMM_MODE_SAVING 2
#define CONST_LORA_AS62_COMM_MODE_SLEEP 3

#define CONST_LORA_AS62_DEFAULT_CHANNEL 425
#define CONST_LORA_AS62_BAND_433_LOW 410
#define CONST_LORA_AS62_BAND_433_HIGH 441

#define LORA_Serial LORASerial
#define CONST_LORA_DEVICE_TYPE_AS62 1
#define CONST_LORA_COMMUICATION_MAX_DATALEN 40
#define CONST_LORA_COMMUNICATION_BUFFSIZE CONST_LORA_COMMUICATION_MAX_DATALEN*2+4
#define CONST_LORA_COMMUNICATION_RECV_BUFF_COUNT 5
#define CONST_LORA_COMMUNICATION_BEGIN_CHAR '$'
#define CONST_LORA_COMMUNICATION_END_CHAR '#'


#define CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN 6
#define CONST_LORA_AS62_SIO_BAUD_RATE_NORMAL 9600
#define CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT 9600
#define CONST_LORA_AS62_AIR_BAUD_RATE 2400

#define CONST_LORA_AS62_AIR_DELAY_TIME (1000/(CONST_LORA_AS62_AIR_BAUD_RATE/10))

#define CONST_LORA_AS62_CONFIG_FILENAME "/LoRaConfig_AS62.ini"


class LoRa_AS62
{
  public:
    //data
    //SoftwareSerial SerialPort(CONST_LORA_AS62_RX_PORT, CONST_LORA_AS62_TX_PORT); // RX, TX 8266 GPIO 4,2

    uint8_t deviceType;// Lora device type; default is AS62
    short sendChannel;//send channel
    short recvChannel;//received channel

    short recvReady;

    short loraBandRate;
    short sioBandRate;
    unsigned short address;
    //function
    LoRa_AS62();
    virtual ~LoRa_AS62();

    bool begin();
    bool begin(short addr,short channel, short airRate);
    bool isReady();
    //short set(short,addr,short channel, short airRate);
    short available();
    uint8_t busy(); //true if could not send
    short send(uint8_t *u8Ptr, short nLen);
    short get(uint8_t *u8Ptr);
    short set_config();
    
    bool is_valid_channel(short channel);

    unsigned short get_version();
    
    bool bLoad_config();
    bool bSave_config();

  private:
    //data
    unsigned short version;

    bool bReady;
    uint8_t u8Busy;
    unsigned long airSendTime;
    unsigned long gulBusyTick;
    uint8_t recvCHAN;
    uint8_t sendCHAN;
    uint8_t M0Port;
    uint8_t M1Port;
    
    uint8_t sendBuf[CONST_LORA_COMMUNICATION_BUFFSIZE + 2];
    short sendLen;
    uint8_t recvBuf[CONST_LORA_COMMUNICATION_BUFFSIZE + 2];
    short recvLen;
    uint8_t simpleEncodeBuf[CONST_LORA_COMMUICATION_MAX_DATALEN+8];
    
    // HEAD,ADDH,ADDL,SPEED,CHAN,OPTION
    bool init_device();
    uint8_t configData[CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN];
    void prepare_configData();
    void set_comm_mode(uint8_t mode);
    uint8_t get_SPEED(short SIORate, short AirRate);
    uint8_t get_CHAN(short channel);
    short read_config(uint8_t *u8Ptr);
    void reset();

};




#endif // _LORA_AS62_H

