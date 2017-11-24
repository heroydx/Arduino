#include <arduino.h>

#include "LoRaMAC.h"
#include "FM_ZCJ_SM.h"
#include "miscLED.h"

/***********************************************************************
   DEBUG MODIFY FILES
   1. miscCommon.h
     //#define DEBUG_SIO Serial--> remove quote
   2. FM_ZCJ_SM.h
     //#define APP_SERIAL Serial-->  #define APP_SERIAL SerialFM
   3. FM_ZCJ_SM.cpp
     static SoftwareSerial SerialFM(CONST_FM_ZCJ_RX_PORT, CONST_FM_ZCJ_TX_PORT); // RX, TX 8266 GPIO 4,2
 **********************************************************************/

#define SIO_BAUD 74880

// status value
#define FM_INIT_STATE 0
#define FM_SMARTCONFIG_START_STATE 1
#define FM_SMARTCONFIG_DONE_STATE 2
#define FM_CONFIG_START_STATE 3
#define FM_CONFIG_DONE_STATE 4
#define FM_WAIT_CONNECTION_STATE 5
#define FM_CONNECTED_STATE 6
#define FM_FACTORY_TEST_FINISH_LOOP 7

#define NEED_REGULAR_CONNECTION_FUNTION 1
#ifdef NEED_REGULAR_CONNECTION_FUNTION
#define CONST_CONNINTERVAL_TICK 125*1000
#else
#define CONST_CONNINTERVAL_TICK 20000*1000
#endif
#define CONST_CONNINTERVAL_SHORT_TICK 5000

#define CONST_MIN_POST_INTERVAL_TICK 1000

#define CONST_ADDR_LENGTH 16+16
#define CONST_WIFI_SSID_LENGTH 64
#define CONST_WIFI_PASS_LENGTH 64

#define CONST_SYS_VERSION_LENGTH 64
#define CONST_SYS_MAID_LENGTH 5
#define CONST_SYS_PID_LENGTH 5
#define CONST_SYS_CAT_LENGTH 5
#define CONST_MAC_ADDR_LEN 6
#define CONST_MAC_ADDR_ASC_LEN 17

typedef struct {
  char Version[CONST_SYS_VERSION_LENGTH];
  char SWVN[CONST_SYS_VERSION_LENGTH];
  char  HWVN[CONST_SYS_VERSION_LENGTH];
  short MAID;
  short PID;
  uint8_t bssid[CONST_MAC_ADDR_LEN + 1];
  uint8_t macAddr[CONST_MAC_ADDR_LEN + 1];
  uint8_t userMac[CONST_MAC_ADDR_LEN + 1];
  char udpAddr[CONST_ADDR_LENGTH + 1];
  unsigned int udpPort;
  short channel;
  char ssid[CONST_WIFI_SSID_LENGTH + 1];
  char pass[CONST_WIFI_PASS_LENGTH + 1];
} stFMDeviceCoreDataStruct;

stFMDeviceCoreDataStruct gsFMDevInfo;

short gFMState;

LoRaMAC LoRaMacData;
FM_ZCJ FMDevice;
miscLED LEDControl;

stDataStreamUnion appSendData;

unsigned long gl5secondTick = 0;
unsigned long glStateTransferTicks = ulReset_interval();
int gnStateTransferTicksCount = 0;

uint8_t gu8SeqNum;

#define CONST_CONFIG_TEST_SWITCH_TICK_inMS 1000*10
unsigned long glTestSwitchTick = 0; //测试目的,每10秒切换一下音量和数据，以后不用
short gnTestSwitchFlag = 1;

#define CONST_SEND_CONFIG_INFO_FM_VOL 10
#define CONST_SEND_CONFIG_INFO_TICK_inMS 500

#define CONST_LORA_BAND_433_LOW 410
#define CONST_LORA_BAND_433_HIGH 441
#define CONST_LORA_BAND_FM_ADD_VAL 500

unsigned long glSendConfigTicks;

//发送LoRa config info 状态信息
void send_config_info(short channel)
{
  DBGPRINTLN("\n---------- send_config_info begin ------------");
  appSendData.configChannel.CMD = LORA_CMD_CONFIG_CHANNEL_BROARDCAST;
  memcpy(appSendData.configChannel.sourceAddr, LoRaMacData.meAddr, LORA_MAC_ADDRESS_LEN);
  memcpy(appSendData.configChannel.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
  appSendData.configChannel.chksum = LoRaMacData.chksum;
  appSendData.configChannel.seq = gu8SeqNum++;
  appSendData.configChannel.channel01 = channel;
  appSendData.configChannel.channel02 = channel;
  DBGPRINTLN("--------------DBG config information begin ------------");
  LoRaMacData.debug_print_self();
  LoRaMacData.debug_print_union(appSendData.u8Data);
  LoRaMacData.config_send_data(appSendData.u8Data);
  DBGPRINTLN("--------------DBG config information end ------------");
  DBGPRINTLN("---------- send_config_info end ------------");
}

void  disp_LoRa_channel()
{
  short nLoRaChannel;
  short dispChannel;
  short nCurrFMFrequency;
  uint8_t currStat, currVol;
  nLoRaChannel = LoRaMacData.workChannel;
  nCurrFMFrequency = FMDevice.currStat.nFrequency;
  currStat = FMDevice.currStat.u8Stat;
  DBGPRINTLN("");
  DBGPRINTF("\n LoRa Channel:[%d]", nLoRaChannel);
  //DBGPRINTF("\n FM: stat:[%d],freq:[%d]", currStat, nCurrFMFrequency);
  DBGPRINTF("\n FM: stat:[%d],freq:[%d],vol:[%d]", currStat, nCurrFMFrequency, currVol);

  //display current LoRa Channel and turn on
  dispChannel = nLoRaChannel + CONST_LORA_BAND_FM_ADD_VAL;
  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = dispChannel;
  delay(10);
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(100);
  //FM radio: turn off
  FMDevice.allCommand.CMD.command = FM_PLAY_PAUSE;
  FMDevice.allCommand.CMD.param = 0;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(6000);

  //restore setting
  // set freq and turn on
  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = nCurrFMFrequency;
  delay(10);
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(100);
  if (currStat == 0) {
    //turn off
    FMDevice.allCommand.CMD.command = FM_PLAY_PAUSE;
    FMDevice.allCommand.CMD.param = 0;
    FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  }
}

void check_disp_channel()
{
  short nCurrFMFrequency;
  uint8_t currStat;
  uint8_t currVol;
  nCurrFMFrequency = FMDevice.currStat.nFrequency;
  currStat = FMDevice.currStat.u8Stat;
  currVol = FMDevice.currStat.u8Vol;
  DBGPRINTLN("");
  //DBGPRINTF("\n LoRa Disp Channel:[%d]", dispChannel);
  DBGPRINTF("\n FM: stat:[%d],freq:[%d],vol:[%d]", currStat, nCurrFMFrequency, currVol);
  /*
    if (nCurrFMFrequency <= (CONST_LORA_BAND_433_LOW + CONST_LORA_BAND_FM_ADD_VAL) || nCurrFMFrequency > (CONST_LORA_BAND_433_HIGH + CONST_LORA_BAND_FM_ADD_VAL)) {
      nCurrFMFrequency = (CONST_LORA_BAND_433_LOW + 5 + CONST_LORA_BAND_FM_ADD_VAL);
      set_disp_channel(nCurrFMFrequency);
    }
  */
  set_disp_status();
  delay(100);
}


void set_disp_channel(short dispChannel)
{
  short nCurrFMFrequency;
  uint8_t currStat;
  if (dispChannel <= (CONST_LORA_BAND_433_LOW + CONST_LORA_BAND_FM_ADD_VAL) || dispChannel > (CONST_LORA_BAND_433_HIGH + CONST_LORA_BAND_FM_ADD_VAL)) {
    dispChannel = (CONST_LORA_BAND_433_LOW + 5 + CONST_LORA_BAND_FM_ADD_VAL);
  }
  DBGPRINTF("\n LoRa Disp Channel:[%d]", dispChannel);

  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = dispChannel;
  delay(10);
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
}

void set_disp_vol(short vol)
{
  DBGPRINTF("\n LoRa Disp vol:[%d]", vol);

  FMDevice.allCommand.CMD.command = FM_SET_VOL;
  FMDevice.allCommand.CMD.param = vol;
  delay(10);
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
}

void set_disp_status()
{
  FMDevice.allCommand.CMD.command = FM_STATUS;
  delay(10);
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
}


void test_switch_channel_vol()
{
  if (ulGet_interval(glTestSwitchTick) > CONST_CONFIG_TEST_SWITCH_TICK_inMS) {
    if (gnTestSwitchFlag > 0) {
      DBGPRINTF("\n Switch to send: [%d],[%d]", 925, 16);
      set_disp_channel(925);
      delay(100);
      set_disp_vol(16);
    }
    else {
      DBGPRINTF("\n Switch to send: [%d],[%d]", 915, 14);
      set_disp_channel(915);
      delay(100);
      set_disp_vol(14);
    }
    delay(100);
    set_disp_status();
    gnTestSwitchFlag = -gnTestSwitchFlag;
    glTestSwitchTick = ulReset_interval();
  }
}


void func_FM_CONFIG_DONE_STATE()
{
  unsigned long tick;
  short nColCount = 0;
  LEDControl.change_LED_status(CONST_LED_STATUS_CONNECTING);
  DBGPRINTF("\nLoRa Channel [%d]\n", gsFMDevInfo.channel);
  LoRaMacData.begin(true, gsFMDevInfo.channel);
  LoRaMacData.set_host_mode(LORA_MAC_HOST_FLAG_MASTER);
  LoRaMacData.debug_print_self();
  DBGPRINTF("\n freeheap:[%d]\n", ESP.getFreeHeap());

  // determine if the lora and FM is in status
  if (!LoRaMacData.devReady()) {
    DBGPRINTLN("LoRa device is NOT ready!");
    gFMState = FM_INIT_STATE;
  }

  set_disp_vol(CONST_SEND_CONFIG_INFO_FM_VOL - 1);
  delay(100);
  set_disp_status();
  disp_LoRa_channel();
}

void func_FM_CONNECTED_STATE()
{
  short nCurrFMFrequency;
  short channel;
  FMDevice.FM_loop();
  //LoRaMacData.available();
  if (ulGet_interval(glSendConfigTicks) > CONST_SEND_CONFIG_INFO_TICK_inMS) {
    //判断当前音量的值
    if (FMDevice.currStat.u8Vol > CONST_SEND_CONFIG_INFO_FM_VOL) {
      //send config status
      nCurrFMFrequency = FMDevice.currStat.nFrequency;
      channel = nCurrFMFrequency - CONST_LORA_BAND_FM_ADD_VAL;
      if (channel > (CONST_LORA_BAND_433_LOW + 5) && channel < CONST_LORA_BAND_433_HIGH) {
        DBGPRINTLN("\n--send config status");
        DBGPRINTF("\n vol:[%d],channel:[%d]", FMDevice.currStat.u8Vol, channel);
        LEDControl.change_LED_status(CONST_LED_STATUS_FAST_FLASH);
        send_config_info(channel);
      }
      else {
        DBGPRINTLN("\n frequency is not in range");
      }
    }
    else {
      LEDControl.change_LED_status(CONST_LED_STATUS_VERY_SLOW_FLASH);
    }
    check_disp_channel();
    glSendConfigTicks = ulReset_interval();
  }
  //debug purpose only
  //test_switch_channel_vol();
}

void setup()
{
  if (!SPIFFS.begin())
  {

    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  // Open serial communications and wait for port to open:
#ifdef DEBUG_SIO
  SERIAL_DEBUG_BEGIN
  while (!DEBUG_SIO) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  DBGPRINTLN();
  DBGPRINTLN();
  DBGPRINTLN("Serial Port for debug is ready on : ");
  DBGPRINTLN(SIO_BAUD);
#endif

  gsFMDevInfo.MAID = 8;
  gsFMDevInfo.PID = 2;

  LEDControl.begin();
  gFMState = FM_INIT_STATE;

  // set the data rate for the SoftwareSerial port
  FMDevice.begin();

}

void loop() // run over and over
{
  if (ulGet_interval(gl5secondTick) > 5000) {
    DBGPRINTLN("---- main loop ----");
    gl5secondTick = ulReset_interval();
  }

  LEDControl.LED_loop();

  switch (gFMState)
  {
    case FM_INIT_STATE:
      DBGPRINTLN("---- FM_INIT_STATE ----");
      DBGPRINTLN("\r\nWait for Smartconfig");
      LEDControl.change_LED_status(CONST_LED_STATUS_SMARTCONFIG);
      gFMState = FM_SMARTCONFIG_START_STATE;
      break;
    case FM_SMARTCONFIG_START_STATE:
      gFMState = FM_SMARTCONFIG_DONE_STATE;
      break;
    case FM_SMARTCONFIG_DONE_STATE:
      gFMState = FM_CONFIG_START_STATE;
      break;
    case FM_CONFIG_START_STATE:
      gsFMDevInfo.channel = LORA_MAC_DEFAULT_CONFIG_CHANNEL;
      DBGPRINTF("\n gsFMDevInfo.channel: [%d]", gsFMDevInfo.channel);
      gFMState = FM_CONFIG_DONE_STATE;
      break;
    case FM_CONFIG_DONE_STATE:
      func_FM_CONFIG_DONE_STATE();
      gFMState = FM_CONNECTED_STATE;
      break;
    case FM_WAIT_CONNECTION_STATE:
      break;
    case FM_CONNECTED_STATE:
      func_FM_CONNECTED_STATE();
      break;
    case FM_FACTORY_TEST_FINISH_LOOP:
      break;
    default:
      break;
  }
}


