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

unsigned long glsecondTick = 0;
unsigned long glStateTransferTicks = ulReset_interval();
int gnStateTransferTicksCount = 0;

//发送FM Radio 状态信息
void send_stat_info()
{
  DBGPRINTLN("\n---------- send_stat_info ------------");
  // current FM radio status
  FMDevice.allCommand.stat.command = FM_STAT_REPORT;
  //老的数据格式,部分数据以bit方式合并,目前改成新的格式
  //memcpy(FMDevice.allCommand.stat.u8statString, FMDevice.u8statString, sizeof(FMDevice.u8statString));
  FMDevice.allCommand.stat.nFrequency=FMDevice.currStat.nFrequency;
  FMDevice.allCommand.stat.u8Vol=FMDevice.currStat.u8Vol;
  FMDevice.allCommand.stat.u8Stat=FMDevice.currStat.u8Stat;
  FMDevice.allCommand.stat.u8Campus=FMDevice.currStat.u8Campus;
  FMDevice.allCommand.stat.u8Blk=FMDevice.currStat.u8Blk;
  FMDevice.allCommand.stat.u8SN_THR=FMDevice.currStat.u8SN_THR;
  FMDevice.allCommand.stat.u8DSP=FMDevice.currStat.u8DSP;

  appSendData.app.CMD = LORA_CMD_APP_FMRADIO;
  memcpy(appSendData.app.sourceAddr, LoRaMacData.meAddr, LORA_MAC_ADDRESS_LEN);
  memcpy(appSendData.app.destAddr, LoRaMacData.hostAddr, LORA_MAC_ADDRESS_LEN);
  memcpy(appSendData.app.data, FMDevice.allCommand.u8Data, sizeof(FMDevice.allCommand));
  DBGPRINTLN("\n--------------DBG stat information ------------");
  LoRaMacData.debug_print_union(appSendData.u8Data);
  LoRaMacData.send_data(appSendData.u8Data);
}

void handle_recv_LoRa_data()
{
  short CMD;
  short nLen;
  nLen = sizeof (stDataStreamUnion);

  CMD = LoRaMacData.recvData.exCMD.CMD;
  //public Part
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      //DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

      //DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
      //simpleHash((char *) LoRaMacData.key, (char *)  LoRaMacData.recvData.regFeedback.val, LoRaMacData.devID, LORA_MAC_KEY_LEN);

      //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
      //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);
      //LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);

      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_REGISTRATION_FEEDBACK ==");
      break;
    case LORA_CMD_ADDR_REQUEST:
      // DBGPRINTLN("\n== LORA_CMD_ADDR_REQUEST ==");
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_ADDR_FEEDBACK ==");
      break;
    case LORA_CMD_SYNC_CMD:
      DBGPRINTLN("\n== LORA_CMD_SYNC_CMD ==");
      //DBGPRINTLN("\n--------- will send stat information -----------");
      //发送FM Radio 状态信息
      send_stat_info();
      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINTLN("\n== LORA_CMD_SYNC_TIME ==");
      break;
    case LORA_CMD_SYNC_RANDOM:
      DBGPRINTLN("\n== LORA_CMD_SYNC_RANDOM ==");
      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_REQUEST ==");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_FEEDBACK ==");
      break;
    case LORA_CMD_APP_FMRADIO:
      DBGPRINTLN("\n== LORA_CMD_APP_FMRADIO ==");
      //FMDevice.handle_Server_to_FM_command((char *)LoRaMacData.recvData.app.data);
      FMDevice.handle_host_to_FM_command((unsigned char *)LoRaMacData.recvData.app.data);
      break;
    default:
      DBGPRINTLN("\n== application call ==");
      //application call to call external application API
      DBGPRINTF("\napp CMD: [%d]\n", CMD);
      break;
  }
}

void  disp_LoRa_channel()
{
  short nLoRaChannel;
  short dispChannel;
  short nCurrFMFrequency;
  uint8_t currStat;
  nLoRaChannel = LoRaMacData.workChannel;
  nCurrFMFrequency = FMDevice.currStat.nFrequency;
  currStat = FMDevice.currStat.u8Stat;
  DBGPRINTLN("");
  DBGPRINTF("\n LoRa workChannel Channel:[%d] configChannel:[%d]", nLoRaChannel, LoRaMacData.configChannel);
  DBGPRINTF("\n FM: stat:[%d],freq:[%d]", currStat, nCurrFMFrequency);

  //display current LoRa Channel and turn on
  dispChannel = nLoRaChannel + 500;
  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = dispChannel;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(100);
  //FM radio: turn off
  FMDevice.allCommand.CMD.command = FM_PLAY_PAUSE;
  FMDevice.allCommand.CMD.param = 0;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);

  delay(3000);

  //restore setting
  // set freq and turn on
  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = nCurrFMFrequency;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(100);
  if (currStat == 0) {
    //turn off
    FMDevice.allCommand.CMD.command = FM_PLAY_PAUSE;
    FMDevice.allCommand.CMD.param = 0;
    FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  }
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
  DBGPRINTLN("\n=====< LLFMLoRaCient >=====");
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
  if (ulGet_interval(glsecondTick) > 5000) {
    DBGPRINTLN("---- main loop ----");
    glsecondTick = ulReset_interval();
  }

  LEDControl.LED_loop();

  switch (gFMState)
  {
    case FM_INIT_STATE:
      DBGPRINTLN("---- FM_INIT_STATE ----");
      /*
        WiFi.mode(CONST_WIFI_MODE);
        gnWiFiStat = WiFi.status();
        if (gnWiFiStat != WL_CONNECTED)
        {
      */
      DBGPRINTLN("\r\nWait for Smartconfig");
      LEDControl.change_LED_status(CONST_LED_STATUS_SMARTCONFIG);
      /*
        WiFi.beginSmartConfig();
        glStateTransferTicks = ulReset_interval();
        gnStateTransferTicksCount = 0;
      */
      gFMState = FM_SMARTCONFIG_START_STATE;
      /*
        }
        else
        {
        WiFi.disconnect();
        }
      */
      break;
    case FM_SMARTCONFIG_START_STATE:
      DBGPRINTLN("== FM_SMARTCONFIG_START_STATE ==");
      //disp_LoRa_channel();
      /*
        if (WiFi.smartConfigDone())
        {
        WiFi.stopSmartConfig();
        DBGPRINTLN("SmartConfig Success");
        strcpy(gsDeviceInfo.wifi.ssid , WiFi.SSID().c_str());
        strcpy(gsDeviceInfo.wifi.pass , WiFi.psk().c_str());
        WiFi.macAddress(gsDeviceInfo.macAddr);

        DBGPRINTF("\n\rSSID: % s", gsDeviceInfo.wifi.ssid);
        DBGPRINTF("\n\rPASS: % s", gsDeviceInfo.wifi.pass);
        DBGPRINTLN();

        WiFi.begin(gsDeviceInfo.wifi.ssid, gsDeviceInfo.wifi.pass);
        bConnectWiFi = true;

        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount = 0;
        gchMainState = SMARTCONFIG_DONE_STATE;
        }
        else
        {
        if (ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN)
        {
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount++;
        DBGPRINT("s");
        //while ((ulGet_interval(glStateTransferTicks) % 1000) == 0);
        }
        else if (gbConfigOK && gnStateTransferTicksCount > CONST_SMARTCONFIG_SWITCH)
        { //Time out switch from smartconfig to try connection mode
        DBGPRINTLN();
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount = 0;
        WiFi.stopSmartConfig();
        gchMainState = WAIT_CONNECTION_STATE;
        }
        }
      */

      gFMState = FM_SMARTCONFIG_DONE_STATE;
      break;
    case FM_SMARTCONFIG_DONE_STATE:
      DBGPRINTLN("== FM_SMARTCONFIG_DONE_STATE == ");
      /*
        vChange_LED_status(CONST_LED_STATUS_CONNECTING);
        gnWiFiStat = WiFi.status();
        if (gnWiFiStat != WL_CONNECTED)
        {
        if (ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN)
        {
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount++;
        DBGPRINT("c");
        //while ((ulGet_interval(glStateTransferTicks) % 1000) == 0)
        ;
        }

        if (gnStateTransferTicksCount > CONST_SMARTCONFIG_SWITCH)
        { //Not connected? Strange go to smartconfig again
        DBGPRINTLN();
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount = 0;
        nChange_WiFi_Mode(WL_DISCONNECTED);
        DBGPRINTLN("Smartconfig SSID / PASS ERROR, CANNOT CONNETCT TO WIFI");
        gchMainState = INIT_STATE;
        }
        }
        else
        { //connected
        DBGPRINTLN("\r\nWiFi connected");
        DBGPRINTLN("IP address: ");
        DBGPRINTLN(WiFi.localIP());

        bConnectWiFi = false;

        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount = 0;

        DBGPRINTLN("Starting UDP");
        //  ghUdpLocal.begin(gsDeviceInfo.server.recvPort);
        ghpUdpLocal->setLocalPort(gsDeviceInfo.server.recvPort);
        ghpUdpLocal->connect(gsDeviceInfo.server.bundAddr, gsDeviceInfo.server.bundPort);
        vLVConfig_udp_send(ghpUdpLocal);
        gchMainState = LV_CONFIG_START_STATE;
        }
      */
      gFMState = FM_CONFIG_START_STATE;
      DBGPRINTLN("");
      break;
    case FM_CONFIG_START_STATE:
      /*
        if (!ghpUdpLocal->available())
        {
        if ((ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN))
        {
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount++;
        DBGPRINT("b");
        vLVConfig_udp_send(ghpUdpLocal);
        //while ((ulGet_interval(glStateTransferTicks) % 1000) == 0)
        ;    //Wait
        }

        if (gnStateTransferTicksCount >= CONST_UDPCONFIG_SWITCH)
        { //Not connected? Strange go to smartconfig again
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount = 0;
        gchMainState = INIT_STATE;
        ghpUdpLocal->stop();
        }
        }
        else
        {
        int len;
        char *spDevID, *spAddr, *spPort, *spSeed, *spMac;
        int nDevIDLen, nAddrLen, nPortLen, nSeedLen, nMacLen;
        len = ghpUdpLocal->read((unsigned char *) gBigBuff.chpData, CONST_UDP_BUFF_SIZE);
        gBigBuff.chpData[len] = 0;
        DBGPRINTLN("receive UDP");
        DBGPRINTLN(gBigBuff.chpData);
        nDevIDLen = nGet_json_pos(gBigBuff.chpData, "devSn", &spDevID);
        nAddrLen = nGet_json_pos(gBigBuff.chpData, "domain", &spAddr);
        nPortLen = nGet_json_pos(gBigBuff.chpData, "port", &spPort);
        nSeedLen = nGet_json_pos(gBigBuff.chpData, "seed", &spSeed);
        nMacLen = nGet_json_pos(gBigBuff.chpData, "mac", &spMac);
        if ((nDevIDLen > 0) && (nAddrLen > 0) && (nPortLen > 0) && (nSeedLen > 0))
        {
        (spDevID + nDevIDLen) = '\0';
        (spAddr + nAddrLen) = '\0';
        (spPort + nPortLen) = '\0';
        (spSeed + nSeedLen) = '\0';

        strcpy(gsDeviceInfo.server.addr, spAddr);
        gsDeviceInfo.server.port = atoi(spPort);
        strcpy(gachServerSeed, spSeed);
        //gachServerSeed = String(spSeed);

        DBGPRINTLN(gachServerSeed);

        DBGPRINT("Bound Server Addr: ");
        DBGPRINTLN(gsDeviceInfo.server.addr);
        DBGPRINT("Bound Server Port: ");
        DBGPRINTLN(gsDeviceInfo.server.port);

        ghpUdpLocal->stop();
        vReset_interval(glStateTransferTicks);
        gnStateTransferTicksCount = 0;
        gchMainState = LV_CONFIG_DONE_STATE;
        if (nMacLen == CONST_MAC_ADDR_ASC_LEN) {
        short i;
        (spMac + (nMacLen)) = '\0';
        for (i = 0; i < CONST_MAC_ADDR_LEN; i++) {
        gsDeviceInfo.userMac[i] = ascii_hex(*(spMac)) * 16 + ascii_hex(*(spMac + 1));
        spMac += 3;
        }
        }
        }
        else
        {
        DBGPRINTLN("Failed to parse config file");
        //Strange data error. Repeat send again
        break;
        }
        }
      */
      //gsFMDevInfo.channel = LORA_MAC_DEFAULT_CONFIG_CHANNEL;
      gFMState = FM_CONFIG_DONE_STATE;
      break;
    case FM_CONFIG_DONE_STATE:
      {
        unsigned long tick;
        short nColCount = 0;
        DBGPRINTLN("== FM_CONFIG_DONE_STATE == ");
        LEDControl.change_LED_status(CONST_LED_STATUS_CONNECTING);
        DBGPRINTF("\nLoRa Channel [ % d]\n", gsFMDevInfo.channel);
        //LoRaMacData.begin(false, gsFMDevInfo.channel);
        LoRaMacData.begin(false);
        LoRaMacData.debug_print_self();
        LoRaMacData.registration(gsFMDevInfo.MAID, gsFMDevInfo.PID);
        DBGPRINTF("\n freeheap: [ % d]\n", ESP.getFreeHeap());

        // determine if the lora and FM is in status
        if (!LoRaMacData.devReady()) {
          gFMState = FM_INIT_STATE;
        }

        disp_LoRa_channel();

        tick = 0;

        while (true)
        {
          short nLen;
          LEDControl.LED_loop();

          LEDControl.change_LED_status(CONST_LED_STATUS_FAST_FLASH);

          LoRaMacData.registration(gsFMDevInfo.MAID, gsFMDevInfo.PID);
          //DBGPRINTLN("\nLoRaMAC::registration(short maid, short pid)\n");
          //LoRaMacData.debug_print_union(LoRaMacData.emergencyData.u8Data);

          nLen = LoRaMacData.available();
          if (nLen) {
            DBGPRINTF("\n--LoRa received Data: [%d]", nLen);
            LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);

            if (LoRaMacData.is_to_me_msg()) {
              DBGPRINT("--is_to_me_msg \n");
              handle_recv_LoRa_data();
            }
            LoRaMacData.debug_print_self();
            DBGPRINTLN("");

          }
          if (LoRaMacData.linkReady == true) {
            DBGPRINTF("\nLoRaMacData.linkReady == true % d", LoRaMacData.linkReady);
            LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);
            delay(5);
            break;
          }
          if (ulGet_interval(tick) > 1000) {
            DBGPRINT("r");
            //nColCount++;
            if (nColCount > 60) {
              DBGPRINTLN();
              disp_LoRa_channel();
              nColCount = 0;
              //LoRaMacData.debug_print_self();
            }
            tick = ulReset_interval();
            delay(2);//必须保留
          }
        }

        /*
          bSave_config();
          //there is bug in here, connect return 4, not 0.
          while (!ghpTcpCommand->connect(gsDeviceInfo.server.addr, gsDeviceInfo.server.port))
          {
          //gchMainState = WAIT_CONNECTION_STATE;
          //break;
          delay(10);
          DBGPRINTLN("Can't connect, retry!");
          }

          delay(50);
          bSend_3006(true, gachServerSeed);

          gbConfigOK = true;

          #ifdef NEED_PUSH_FUNCTION
          //delay(100);
          delay(10);
          if (!ghpTcpPush->connect(gsDeviceInfo.server.addr, gsDeviceInfo.server.port))
          {
          DBGPRINTLN("Start push failed");
          gnNetPushFailCount++;
          gnTcpPushBusy = 0;
          }
          else
          {
          DBGPRINTLN("Start push");
          gnNetPushFailCount = 0;
          gnTcpPushBusy = 1;
          //delay(100);
          delay(10);
          bSend_Push();
          //delay(100);
          delay(10);
          }
          #endif

          //Now config OK.
          vReset_interval(glLastCommandTick);
        */
        gFMState = FM_CONNECTED_STATE;
      }
      break;
    case FM_WAIT_CONNECTION_STATE:
      break;
    case FM_CONNECTED_STATE:
      LEDControl.change_LED_status(CONST_LED_STATUS_VERY_SLOW_FLASH);
      {
        short nLen;
        //nLen = LoRaDev.available();
        nLen = LoRaMacData.available();
        if (nLen) {
          LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);
          DBGPRINTLN("\n--LoRa received Data");
          DBGPRINTLN(nLen);
          handle_recv_LoRa_data();
          LoRaMacData.debug_print_self();

        }
        //
        FMDevice.FM_loop();
      }
      break;
    case FM_FACTORY_TEST_FINISH_LOOP:
      break;
    default:
      break;
  }

}


