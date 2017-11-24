#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <ESP8266httpUpdate.h>

//#include "ESP8266ClientDB.h"

extern "C" {
#include "miscCommon.h"
}

#define DEBUG_MODE 1

#define CONST_WIFI_MODE WIFI_STA

#define CONST_APP_TAG_DEEP_SLEEP_TIME_LEN 1000*1000*60
#define CONST_APP_TRY_CONNECTION_TIME_LEN 1000*15

//shulong pin definition
#define CONST_LED_COLOR_RED_PIN 15
#define CONST_LED_COLOR_GREEN_PIN 5
#define CONST_LED_COLOR_BLUE_PIN 4

unsigned long glTryConnectionTick;
unsigned long glStateTransferTicks;
int gnStateTransferTicksCount;

int gnWiFiStat;
bool bConnectWiFi;// = false; connect to wifi status , true = connected, false= not connected
char wifiSSID[32] = "thisisnotausefulapdonottryagain";
char wifiPass[32] = "12345678901234567890abcdefg";

char macString[32] = "";

#define CONST_APP_VCC_REFERNCE 460/1023
#define CONTT_APP_BATTERY_LOW 370
short gnVCC ;

char finalSSID[64];

void vApp_read_vcc()
{
  short nT1;
  nT1 = analogRead(A0);
  gnVCC = nT1 * CONST_APP_VCC_REFERNCE;
  DBGPRINTF("\n A0:[%d] vcc: [%d] ", nT1, gnVCC);

}


int ICACHE_FLASH_ATTR nChange_WiFi_Mode(int stat)
{
  switch (stat)
  {
    case WL_DISCONNECTED:
      WiFi.disconnect();
      // strange issue, cannot reconnected by new password, add the following code based on https://github.com/esp8266/Arduino/issues/2186
      WiFi.persistent(false);
      WiFi.mode(WIFI_OFF);
      delay(5);
      WiFi.mode(CONST_WIFI_MODE);
      bConnectWiFi = false;
      break;
    default:
      break;
  }
  delay(20);
  return WiFi.status();
}



void get_mac_address(char *strP) {
  byte mac[6];
  char *strT;
  strT = strP;
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; ++i) {
    sprintf(strT, "%02X:", mac[i]);
    strT += 3;
  }
  strT--;
  *strT = 0;
}

void station_mode_setup()
{
  int nWiFiStat;
  SERIAL_DEBUG_BEGIN;
  DBGPRINTLN();
  DBGPRINTLN("-- SETUP DBG PRINT BEGIN --");
  nChange_WiFi_Mode(WL_DISCONNECTED);
  get_mac_address(macString);
  DBGPRINTF("MAC:[%s]\n", macString);
  DBGPRINTLN("-- SETUP DBG PRINT END --");
  pinMode(A0, INPUT);
  vApp_read_vcc();
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);
  if (gnVCC > CONTT_APP_BATTERY_LOW || gnVCC == 0 || DEBUG_MODE) {
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
    delay(250);
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
  }
  else {
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    delay(5);
  }

}


void station_mode_loop()
{
  short i;
  int nNetwork;
  if (gnVCC > CONTT_APP_BATTERY_LOW || gnVCC == 0 || DEBUG_MODE) {

    DBGPRINTLN("-- SCAN BEGIN --");
    nNetwork = WiFi.scanNetworks();
    DBGPRINTLN("-- SCAN END --");
    if (nNetwork <= 0) {
      DBGPRINTLN("-- no network found --");

    }
    else {
      short nFinalNo = 0;
      short nFinalRSSI = -200;
      DBGPRINTF("\n-- [%d] networks found --", nNetwork);
      for (i = 0; i < nNetwork; i++)
      {
        DBGPRINTF("\n No:[%d], SSID:[%s],RSSI:[%d]", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
        if (nFinalRSSI < WiFi.RSSI(i)) {
          nFinalNo = i;
          nFinalRSSI = WiFi.RSSI(i);
        }
      }

      DBGPRINTF("\n Final:[%d], SSID:[%s],RSSI:[%d]", nFinalNo, WiFi.SSID(nFinalNo).c_str(), WiFi.RSSI(nFinalNo));

      short nTryTimes = 20;
      while (nTryTimes > 0) {
        nTryTimes--;
        gnWiFiStat = WiFi.status();
        DBGPRINTF("\n--- gnWiFiStat [%d]", gnWiFiStat);

        if (gnWiFiStat != WL_CONNECTED)
        {
          switch (gnWiFiStat)
          {
            case WL_NO_SHIELD:
            case WL_SCAN_COMPLETED:
              break;
            case WL_NO_SSID_AVAIL:
            case WL_CONNECT_FAILED:
              gnStateTransferTicksCount = 0;
              //gchMainState = INIT_STATE;
              //nChange_WiFi_Mode(WL_DISCONNECTED);
              break;
            case WL_CONNECTION_LOST:
            case WL_IDLE_STATUS:
            case WL_DISCONNECTED:
              WiFi.begin(WiFi.SSID(nFinalNo).c_str(), wifiPass);
              DBGPRINTF(" WAIT CONNECTION STATE: SSID[%s]", WiFi.SSID(nFinalNo).c_str());
              //vReset_interval(glStateTransferTicks);
              glStateTransferTicks = ulReset_interval();

              break;
            default:
              break;
          }
        }
      }
    }
  }
  else {
    //battery low
    for (short i = 0; i < 6; i++)
    {
      digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
      delay(250);
      digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
      delay(250);

    }
  }

  if (ulGet_interval(glTryConnectionTick) >= CONST_APP_TRY_CONNECTION_TIME_LEN)
  {
    DBGPRINTLN("\n=== will in deep sleep mode===");
    //ESP.deepSleep(CONST_APP_TAG_DEEP_SLEEP_TIME_LEN);
    glTryConnectionTick = ulReset_interval();
  }

}

IPAddress myIP = WiFi.softAPIP();

void ap_mode_setup()
{
  int nWiFiStat;
  SERIAL_DEBUG_BEGIN;
  DBGPRINTLN();
  DBGPRINTLN("-- SETUP DBG PRINT BEGIN --");
  nChange_WiFi_Mode(WL_DISCONNECTED);
  get_mac_address(macString);
  DBGPRINTF("MAC:[%s]\n", macString);
  DBGPRINTLN("-- SETUP DBG PRINT END --");
  pinMode(A0, INPUT);
  vApp_read_vcc();
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);
  if (gnVCC > CONTT_APP_BATTERY_LOW || gnVCC == 0 || DEBUG_MODE) {
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
    delay(250);
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
  }
  else {
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    delay(5);
  }
  WiFi.softAP(wifiSSID, wifiPass);

  DBGPRINT("AP IP address: ");
  DBGPRINTLN(myIP);
}

void ap_mode_loop()
{
  if (gnVCC > CONTT_APP_BATTERY_LOW || gnVCC == 0 || DEBUG_MODE) {
    if (ulGet_interval(glTryConnectionTick) >= CONST_APP_TRY_CONNECTION_TIME_LEN)
    {
      DBGPRINTLN("\n=== will in deep sleep mode===");
      ESP.deepSleep(CONST_APP_TAG_DEEP_SLEEP_TIME_LEN);
      glTryConnectionTick = ulReset_interval();
    }
  }
  else {
    //battery low
    for (short i = 0; i < 3; i++) {
      digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
      delay(500);
      digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
      delay(500);

    }
    DBGPRINTLN("\n=== will in deep sleep mode===");
    ESP.deepSleep(CONST_APP_TAG_DEEP_SLEEP_TIME_LEN);
  }


}


void setup() {
  // put your setup code here, to run once:
  //station_mode_setup();
  ap_mode_setup();
}

void loop()
{
  // put your main code here, to run repeatedly:
  //station_mode_loop();
  ap_mode_loop();
}

