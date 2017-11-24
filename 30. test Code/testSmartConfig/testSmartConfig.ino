#include <Arduino.h>
//#include <GDBStub.h>
#include <ESP8266WiFi.h>
//#include <ArduinoJson.h>
#include <ESP8266httpUpdate.h>
//#include <exception>

#include "Timenew.h"
#include "ESP8266ClientDB.h"


extern "C"
{
  //#include "user_interface.h"
#include "os_type.h"
}


/*====================================================================================*/
//DEBUG_PART_BEGIN Ver=20161104
/*====================================================================================*/
#define DEBUG_SIO 1

#ifdef DEBUG_SIO
#define SERIALBEGIN Serial.begin(74880);
#define DBGPRINT(__VA_ARGS__) \
  Serial.print(__VA_ARGS__)
#define DBGPRINTLN(__VA_ARGS__) \
  Serial.println(__VA_ARGS__)
#define DBGPRINTF(fmt,...) \
  Serial.printf(fmt,__VA_ARGS__)
#else
#define DBGPRINT(__VA_ARGS__)
#define DBGPRINTLN(__VA_ARGS__)
#define DBGPRINTF(fmt,...)
#endif
char dispBuff[200];
/*====================================================================================*/
//DEBUG_PART_END
/*====================================================================================*/


/*====================================================================================*/
//COMMON_PART_BEGIN Ver=20161118
/*====================================================================================*/

/*--------------------------------------------------------------------------------*/
//COMMON_DATA_PART_BEGIN
/*--------------------------------------------------------------------------------*/
#define CONST_WIFI_MODE WIFI_STA
#define INIT_STATE          0
#define SMARTCONFIG_START_STATE   1
#define SMARTCONFIG_DONE_STATE    2
#define LV_CONFIG_START_STATE   3
#define LV_CONFIG_DONE_STATE    4
#define WAIT_CONNECTION_STATE   5
#define CONNECTED_STATE       6


//tick data
#define CONST_TRANSFER_TICK_LEN 1000
#define CONST_SMARTCONFIG_SWITCH 60 //1 minute switch from smart config to connect
#define CONST_UDPCONFIG_SWITCH 5
#define CONST_SMATCONFIG_REBOOT_COUNT 60*5 //5 minutes 重启设备

unsigned long glStateTransferTicks = 0;
short gnStateTransferTicksCount;
int gnCommandBusy = 0;

unsigned long glLastCommandTick;

unsigned char gchMainState = INIT_STATE;   // 0 INIT_STATE  initial state
bool bConnectWiFi = false;
short gnWiFiStat;

#define CONST_UDP_BUFF_SIZE 256
ESP8266Client *ghpUdpLocal;

unsigned int gnBundPort = 7788;
const char * gsBoundServer = "255.255.255.255";
unsigned int gnRecvPort = 8877;

#define SSID_LENGTH 64

char gSSID[SSID_LENGTH];
char gPASSWORD[SSID_LENGTH];
unsigned char gMAC[8];

char IMEI[22] = "1234567890";
char SWVN[22] = "SWVN123";
char HWVN[22] = "HWVN123";

char gBigBuff[512];

/*--------------------------------------------------------------------------------*/
//COMMON_DATA_PART_END
/*--------------------------------------------------------------------------------*/

void vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}
unsigned long ulReset_interval()
{
  return millis();
}

unsigned long ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}

//nChange_WiFi_Mode(WL_DISCONNECTED);
int nChange_WiFi_Mode(int stat)
{
  switch (stat)
  {
    case WL_DISCONNECTED:
      WiFi.disconnect();
      // strange issue, cannot reconnected by new password, add the following code based on https://github.com/esp8266/Arduino/issues/2186
      WiFi.persistent(false);
      WiFi.mode(WIFI_OFF);
      delay(100);
      WiFi.mode(CONST_WIFI_MODE);
      bConnectWiFi = false;
      break;
    default:
      break;
  }
  delay(400);
  return WiFi.status();
}

void vLVConfig_udp_send(ESP8266Client *hUDP)
{
  sprintf(gBigBuff, "{\"state\":\"offLine\",\"devSn\":\"%s\",\"swvn\":\"%s\",\"hwvn\":\"%s\",\"time\":\"60\"}", IMEI, SWVN, HWVN);
  hUDP->write((unsigned char *) gBigBuff, strlen(gBigBuff));

}

/*====================================================================================*/
//COMMON_PART_END
/*====================================================================================*/



/*====================================================================================*/
//MAIN_PART_BEGIN Ver=20161116
/*====================================================================================*/

//The setup function is called once at startup of the sketch
void setup()
{
  // Add your initialization code here

#ifdef DEBUG_SIO
  SERIALBEGIN
#endif

  DBGPRINTLN();
  DBGPRINTLN("--DBG PRINT--");
  ghpUdpLocal = new ESP8266Client(UDP, CONST_UDP_BUFF_SIZE);

}


void loop()
{
  //Add your repeated code here


  switch (gchMainState)
  {
    case INIT_STATE:
      WiFi.mode(CONST_WIFI_MODE);
      DBGPRINTLN("\r\nWait for Smartconfig");
      //smartconfig_set_type(SC_TYPE_ESPTOUCH); //LV only support ESPTOUCH
      //smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);   // both ESPTouch and AirKiss
      WiFi.beginSmartConfig();
      vReset_interval(glStateTransferTicks);
      gchMainState = SMARTCONFIG_START_STATE;
      break;

    case SMARTCONFIG_START_STATE:
      if (WiFi.smartConfigDone())
      {
        DBGPRINTLN("SmartConfig Success");
        DBGPRINT("SSID:");
        DBGPRINTLN( WiFi.SSID().c_str());
        DBGPRINT("PASS:");
        DBGPRINTLN( WiFi.psk().c_str());
        DBGPRINT("It costs:");
        DBGPRINT(ulGet_interval(glStateTransferTicks) / 1000);
        DBGPRINTLN("s");
        DBGPRINTLN("SmartConfig Success");
        strcpy(gSSID , WiFi.SSID().c_str());
        strcpy(gPASSWORD , WiFi.psk().c_str());
        WiFi.macAddress(gMAC);

        DBGPRINTF("\n\rSSID:%s", gSSID);
        DBGPRINTF("\n\rPASS:%s", gPASSWORD);
        DBGPRINTLN();

        WiFi.begin(gSSID, gPASSWORD);
        bConnectWiFi = true;

        gchMainState = SMARTCONFIG_DONE_STATE;

        vReset_interval(glStateTransferTicks);
        gchMainState = SMARTCONFIG_DONE_STATE;
      }
      else
      {
        if (ulGet_interval(glStateTransferTicks) % 1000 == 0)
        {
          vReset_interval(glStateTransferTicks);
          DBGPRINT("s");
          while ((ulGet_interval(glStateTransferTicks) % 1000) == 0)
            ;
        }
        else if (ulGet_interval(glStateTransferTicks) > 300000)
        { //Time out
          WiFi.stopSmartConfig();
          gchMainState = INIT_STATE;
          ESP.restart();
        }
      }
      break;
    case SMARTCONFIG_DONE_STATE:
      gnWiFiStat = WiFi.status();
      if (gnWiFiStat != WL_CONNECTED)
      {
        if (ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN)
        {
          //vReset_interval(glStateTransferTicks);
          glStateTransferTicks = ulReset_interval();
          gnStateTransferTicksCount++;
          DBGPRINT("c");
          //while ((ulGet_interval(glStateTransferTicks) % 1000) == 0)
          ;
        }

        if (gnStateTransferTicksCount > CONST_SMARTCONFIG_SWITCH)
        { //Not connected? Strange go to smartconfig again
          DBGPRINTLN();
          //vReset_interval(glStateTransferTicks);
          glStateTransferTicks = ulReset_interval();

          gnStateTransferTicksCount = 0;
          nChange_WiFi_Mode(WL_DISCONNECTED);
          DBGPRINTLN("Smartconfig SSID/PASS ERROR,CANNOT CONNETCT TO WIFI");
          gchMainState = INIT_STATE;
        }
      }
      else
      { //connected
        DBGPRINTLN("\r\nWiFi connected");
        DBGPRINTLN("IP address: ");
        DBGPRINTLN(WiFi.localIP());

        bConnectWiFi = false;

        //vReset_interval(glStateTransferTicks);
        glStateTransferTicks = ulReset_interval();

        gnStateTransferTicksCount = 0;
        gchMainState = LV_CONFIG_START_STATE;

        DBGPRINTLN("Starting UDP");
        //  ghUdpLocal.begin(gsDeviceInfo.server.recvPort);
        ghpUdpLocal->setLocalPort(gnRecvPort);
        ghpUdpLocal->connect(gsBoundServer, gnBundPort);
        short sendCount = 5;
        while (sendCount) {
          vLVConfig_udp_send(ghpUdpLocal);
          delay(10);
          sendCount--;
        }
      }
      delay(1);

      break;
    case LV_CONFIG_START_STATE:
      DBGPRINTLN("\nWait for UDP response!");
      delay(500);
      gchMainState = LV_CONFIG_START_STATE;
      break;

    case LV_CONFIG_DONE_STATE:
      ESP.restart();
      break;

    default:        //return to controlled state
      gchMainState = INIT_STATE;
      break;
  }
}

/*====================================================================================*/
//MAIN_PART_END
/*====================================================================================*/


