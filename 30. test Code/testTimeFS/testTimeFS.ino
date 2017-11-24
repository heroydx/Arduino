#include <ArduinoJson.h>
#include <FS.h>
#include "Timenew.h"

extern "C"
{
  //#include "user_interface.h"
#include "os_type.h"
}

/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20161104
/*--------------------------------------------------------------------------------*/
#define DEBUG_SIO 1
//#define DEBUG_HTTP 1

#ifdef DEBUG_SIO
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

#define CONST_DEVICE_VERSION "9_906_20161107"
#define CONFIG_FILE_NAME "/config.json"
#define CONST_CONFIGFILE_SIZE 1024
#define CONST_APPLICATION_FILE_NAME "/application.data"
#define CONST_APPLICATION_FILE_SIZE 1024

//common struct,unicon definition
typedef struct  {
  String url;
  String addr;
  String port;
  String pushUrl;
  String udpAddr;
  String udpPort;
} stServerAddr;

typedef struct {
  String ssid;
  String pass;
} stWiFi;

typedef struct {
  String Version;
  String SWVN;
  String  HWVN;
  String  MAID;
  String  PID;
  String  CAT;
  stServerAddr server;
  stWiFi wifi;
} stDeviceCoreDataStruct;


stDeviceCoreDataStruct gsDeviceInfo = {
  CONST_DEVICE_VERSION,
  "LVSW_EN_V1.0.2",
  "LVHW_EN_V1.0.2",
  "9",
  "906",
  "wdSd",
  {
    "/py",
    "180.150.187.99",
    "80",
    "/ps",
    "255.255.255.255",
    "10000"

  },
  {
    "chakongerconfig",
    "12345678"
  },
};
typedef struct  {
  int nWenduInt;
  int nWenduDec;
  int nShiduInt;
  int nShiduDec;
  int nPreWenduInt;
  int nPreShiduInt;
} stApplicationData;

stApplicationData gsCurrent;

bool gbConfigOK = false;

// a time interrupt
os_timer_t ghTimer100ms;
unsigned long gl100msLoopTick = 0;/* a common 100 ms loop */

//LED global data

#define CONST_LED_CHANGE_COUNT 8

#define CONST_LED_STATUS_ALWAYSOFF 254
#define CONST_LED_STATUS_ALWAYSON 255

#define CONST_LED_STRUCT_LEN 5
#define CONST_LED_STATUS_SELFDEF 0
#define CONST_LED_STATUS_SMARTCONFIG 1
#define CONST_LED_STATUS_CONNECTING 2
#define CONST_LED_STATUS_SERVER 3
#define CONST_LED_STATUS_ONE 4

#define CONST_LED_COLOR_RED_PIN 13
#define CONST_LED_COLOR_GREEN_PIN 13
#define CONST_LED_COLOR_BLUE_PIN 13

#define CONST_LED_COLOR_RED_MASK 4
#define CONST_LED_COLOR_GREEN_MASK 2
#define CONST_LED_COLOR_BLUE_MASK 1

#define CONST_LED_LOOP_ALWAYS 255


uint8_t gaTestLED[] =
{
  CONST_LED_STATUS_ALWAYSON,
  CONST_LED_STATUS_ALWAYSOFF,
  CONST_LED_STATUS_SELFDEF,
  CONST_LED_STATUS_SMARTCONFIG,
  CONST_LED_STATUS_CONNECTING,
  CONST_LED_STATUS_SERVER,
  CONST_LED_STATUS_ONE,
};

uint8_t gu8LEDStatus = 0;
uint8_t gu8LEDLoopCount = 0;
uint8_t gu8LEDDispCount = 0;

typedef struct {
  uint8_t loopFlag;
  uint8_t onColor;
  uint8_t blinkCount;
  uint8_t blinkLoop[CONST_LED_CHANGE_COUNT];
} stLedDispStruct;

stLedDispStruct asLedDisplay[CONST_LED_STRUCT_LEN] = {
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {3, 6, 9, 19}}, // 300ms on, 300ms off, 300ms on, 1000ms off
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {2, 3, 5, 18}}, // 200ms on, 100ms off, 200ms on, 1200ms off
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 2, {5, 10}}, //500ms on, 500ms off
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 2, {10, 20}}, //1000ms on, 1000ms off
  {1, CONST_LED_COLOR_RED_MASK, 2, {5, 5}}, //500ms on, 500ms off
};

// to setup or change the LED display status, the status include:CONST_LED_STATUS_ALWAYSON,CONST_LED_STATUS_SMARTCONFIG,etc.
void vChange_LED_status(uint8_t u8Status)
{
  gu8LEDStatus = u8Status;
  gu8LEDLoopCount = 0;
  gu8LEDDispCount = 0;
  DBGPRINT("gu8LEDStatus ");
  DBGPRINTLN(u8Status);
  switch (u8Status)
  {
    case  CONST_LED_STATUS_ALWAYSON:
      DBGPRINTLN("CONST_LED_STATUS_ALWAYSON");
      break;
    case  CONST_LED_STATUS_ALWAYSOFF:
      DBGPRINTLN("CONST_LED_STATUS_ALWAYSOFF");
      break;
    case  CONST_LED_STATUS_SELFDEF:
      DBGPRINTLN("CONST_LED_STATUS_SELFDEF");
      break;
    case  CONST_LED_STATUS_SMARTCONFIG:
      DBGPRINTLN("CONST_LED_STATUS_SMARTCONFIG");
      break;
    case  CONST_LED_STATUS_CONNECTING:
      DBGPRINTLN("CONST_LED_STATUS_CONNECTING");
      break;
    case  CONST_LED_STATUS_SERVER:
      DBGPRINTLN("CONST_LED_STATUS_SERVER");
      break;
    case  CONST_LED_STATUS_ONE:
      DBGPRINTLN("CONST_LED_STATUS_ONE");
      break;
    default:
      DBGPRINTLN("DEFAULT");


  }
}

void vDisplay_LED_color(void)
{
  //DBGPRINTLN("\n----");
  // DBGPRINT("LED REAL gu8LEDStatus");
  //DBGPRINT("[");
  //DBGPRINT(gu8LEDStatus);
  //DBGPRINTLN("]");
  if (gu8LEDStatus == CONST_LED_STATUS_ALWAYSOFF)
  {
    //DBGPRINTLN("DISPLAY ALWAYS OFF");
    digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
    digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
    DBGPRINT("_");

  }
  else if  (gu8LEDStatus == CONST_LED_STATUS_ALWAYSON)
  {
    //DBGPRINTLN("DISPLAY ALWAYS ON");
    digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
    digitalWrite(CONST_LED_COLOR_BLUE_PIN, HIGH);
    DBGPRINT("T");
  }
  else
  {
    if (gu8LEDLoopCount < asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
      //if (gu8LEDLoopCount == asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
    {
      if ((gu8LEDDispCount % 2) == 0) {

        if (asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_RED_MASK)
          digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
        if (asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_GREEN_MASK)
          digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
        if (asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_BLUE_MASK)
          digitalWrite(CONST_LED_COLOR_BLUE_PIN, HIGH);
        //digitalWrite(CONST_LED_COLOR_RED_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_RED_MASK );
        //digitalWrite(CONST_LED_COLOR_GREEN_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_GREEN_MASK);
        //digitalWrite(CONST_LED_COLOR_BLUE_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_BLUE_MASK);
        DBGPRINT("T");
      }
      else
      {
        //DBGPRINTLN("DISPLAY OFF");
        digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
        digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
        digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
        DBGPRINT("_");
      }

    }
    else {
      gu8LEDDispCount++;
      if (gu8LEDDispCount >= asLedDisplay[gu8LEDStatus].blinkCount)
      {
        gu8LEDDispCount = 0;
        if (asLedDisplay[gu8LEDStatus].loopFlag != CONST_LED_LOOP_ALWAYS)
        {
          gu8LEDStatus = CONST_LED_STATUS_ALWAYSOFF;
          //DBGPRINTLN("!!!!!");
        }

      }
    }

    gu8LEDLoopCount++;
    if (gu8LEDLoopCount >= asLedDisplay[gu8LEDStatus].blinkLoop[asLedDisplay[gu8LEDStatus].blinkCount - 1])
    {
      gu8LEDLoopCount = 0;
      gu8LEDDispCount = 0;
      if (asLedDisplay[gu8LEDStatus].loopFlag != CONST_LED_LOOP_ALWAYS)
      {
        gu8LEDStatus = CONST_LED_STATUS_ALWAYSOFF;
        //DBGPRINTLN("!!!!!");
      }

    }
  }


  //DBGPRINTLN("!!!!!\n");
}


//the time struct, it is the timer system in this project.
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int wday;
  int timezone2; //2 x timezone , consider india GMT +7.5= 15
} LVTime;

LVTime gTime;
unsigned long glTimeTick;

//convert the HTTP header DATE month to int format
int str2month (char *strMonth)
{
  int month = 0;
  if (strcmp(strMonth, "Jan") == 0)
  {
    month = 1;
  }
  else if (strcmp(strMonth, "Feb") == 0)
  {
    month = 2;
  }
  else if (strcmp(strMonth, "Mar") == 0)
  {
    month = 3;
  }
  else if (strcmp(strMonth, "Apr") == 0)
  {
    month = 4;
  }
  else if (strcmp(strMonth, "May") == 0)
  {
    month = 5;
  }
  else if (strcmp(strMonth, "Jun") == 0)
  {
    month = 6;
  }
  else if (strcmp(strMonth, "Jul") == 0)
  {
    month = 7;
  }
  else if (strcmp(strMonth, "Aug") == 0)
  {
    month = 8;
  }
  else if (strcmp(strMonth, "Sep") == 0)
  {
    month = 9;
  }
  else if (strcmp(strMonth, "Oct") == 0)
  {
    month = 10;
  }
  else if (strcmp(strMonth, "Nov") == 0)
  {
    month = 11;
  }
  else if (strcmp(strMonth, "Dec") == 0)
  {
    month = 12;
  }
  return month;
}

//sync the date from server http header, normally, get the date from 3006 message
void vSync_time_fromServer(char achYMD[])
{
  int hh, mm, ss, dd, mon, yy;
  char buff[10];

  memcpy(buff, &achYMD[10], 3);
  buff[3] = 0;
  dd = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[14], 3);
  buff[3] = 0;
  mon = str2month(buff);

  memcpy(buff, &achYMD[18], 4);
  buff[4] = 0;
  yy = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[23], 2);
  buff[2] = 0;
  hh = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[26], 2);
  buff[2] = 0;
  mm = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[29], 2);
  buff[2] = 0;
  ss = ((String)(buff)).toInt();

  gTime.year = yy;
  gTime.month = mon;
  gTime.day = dd;
  gTime.hour = hh;
  gTime.minute = mm;
  gTime.second = ss;
  gTime.timezone2 = 8 * 2; //default
  setTime(hh, mm, ss, dd, mon, yy);
  vGet_time();
}


//sync the date from server http header, normally, get the date from 3006 message
void vSyncTime_998(String sYMDHMS)
{
  time_t t;
  int hh, mm, ss, dd, mon, yy, timezone2;
  yy = sYMDHMS.substring(0, 4).toInt();
  mon = sYMDHMS.substring(4, 6).toInt();
  dd = sYMDHMS.substring(6, 8).toInt();
  hh = sYMDHMS.substring(8, 10).toInt();
  mm = sYMDHMS.substring(10, 12).toInt();
  ss = sYMDHMS.substring(12, 14).toInt();
  timezone2 = sYMDHMS.substring(14, 16).toInt();

  gTime.timezone2 = timezone2;

  setTime(hh, mm, ss, dd, mon, yy);
  t = now() -  1800 * gTime.timezone2;
  setTime(t);
  t = now();

  gTime.year = year(t);
  gTime.month = month(t);
  gTime.day = day(t);
  gTime.hour = hour(t);
  gTime.minute = minute(t);
  gTime.second = second(t);
  gTime.timezone2 = 8 * 2; //default
}

//get current system time, used by application
void vGet_time() {
  time_t t;
  t = now();
  t += 1800 * gTime.timezone2;
  gTime.year = year(t);
  gTime.month = month(t);
  gTime.day = day(t);
  gTime.hour = hour(t);
  gTime.minute = minute(t);
  gTime.second = second(t);
  gTime.wday = weekday(t);
}


char serverHeader[12][100] = {
  "Date: Wed, 01 Jan 2016 05:38:59 GMT",
  "Date: Wed, 02 Feb 2016 05:38:59 GMT",
  "Date: Wed, 03 Mar 2016 05:38:59 GMT",
  "Date: Wed, 04 Apr 2016 05:38:59 GMT",
  "Date: Wed, 05 May 2016 05:38:59 GMT",
  "Date: Wed, 06 Jun 2016 05:38:59 GMT",
  "Date: Wed, 07 Jul 2016 05:38:59 GMT",
  "Date: Wed, 08 Aug 2016 05:38:59 GMT",
  "Date: Wed, 09 Sep 2016 05:38:59 GMT",
  "Date: Wed, 10 Oct 2016 05:38:59 GMT",
  "Date: Wed, 11 Nov 2016 05:38:59 GMT",
  "Date: Wed, 12 Dec 2016 05:38:59 GMT",
};


void vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}

unsigned long ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}

int gLoop, gMonth = 1;
int gnTestCount = 0;

void vFormat_filesystem()
{
  SPIFFS.format();
}


//callback every second
void vTimer_interrupt100ms(void *pArg)
{

  vDisplay_LED_color();

}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial.println("waiting for sync");

  os_timer_setfn(&ghTimer100ms, vTimer_interrupt100ms, NULL);
  os_timer_arm(&ghTimer100ms, 50, true);

  SPIFFS.begin();
  //vFormat_filesystem();

  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);
  digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
  digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
  digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
  delay(1000);
  digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
  //delay(1000);


  vSync_time_fromServer(serverHeader[0]);
  DBGPRINTLN("===Test LVTime===");
  DBGPRINTLN("vSyncTimeFromServer");
  DBGPRINT(gTime.year);
  DBGPRINT(gTime.month);
  DBGPRINT(gTime.day);
  DBGPRINT(gTime.hour);
  DBGPRINT(gTime.minute);
  DBGPRINT(gTime.second);
  DBGPRINTLN();
  DBGPRINTLN("setup end");
}

void loop() {
  // put your main code here, to run repeatedly:
  int i;

  // sync time
  if (ulGet_interval(glTimeTick) >= 1000)
  {
    char dispBuff[100];
    vGet_time();
    sprintf(dispBuff, "%04d-%02d-%02d %02d %02d:%02d:%02d", gTime.year, gTime.month, gTime.day, gTime.wday, gTime.hour, gTime.minute, gTime.second);
    DBGPRINTLN(dispBuff);
    vReset_interval(glTimeTick);
    //digitalClockDisplay();
    //delay(1000);
    gLoop++;
    //DBGPRINTLN("--load config file begin--");
    //bLoad_config();
    //DBGPRINTLN("--load config file end --");
  }

  if (gLoop >= 10)
  {
    gLoop = 0;
    vSync_time_fromServer(serverHeader[gMonth]);
    gMonth++;
    DBGPRINTLN("======BEGIN============");
    DBGPRINT("\nLED STATUS");
    DBGPRINT(" [");
    DBGPRINT(gnTestCount);
    DBGPRINT("] [");
    DBGPRINT(gaTestLED[gnTestCount]);
    DBGPRINTLN("]");

    //vChange_LED_status(gaTestLED[gnTestCount]);
    //vChange_LED_status(CONST_LED_STATUS_CONNECTING);
    vChange_LED_status(CONST_LED_STATUS_SMARTCONFIG);

    gnTestCount++;
    if (gnTestCount >= 7) {
      gnTestCount = 0;
    }
    // test save config
    gsDeviceInfo.wifi.ssid = String("SSID:") + String(gnTestCount);
    bSave_config();
    DBGPRINTLN("======END============");
  }
  if (gMonth >= 12) {
    gMonth = 0;
  }
  /*
    if (ulGet_interval(gl100msLoopTick) >= 100)
    {
      // sync time
      if (ulGet_interval(glTimeTick) >= 1000)
      {
        vGet_time();
        vReset_interval(glTimeTick);
      }
      // LED Control
      vDisplay_LED_color();
      vReset_interval(gl100msLoopTick);
    }
  */
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}


/* load config, return True==Success*/
bool bLoad_config()
{
  bool bRet = false;
  String sTemp;
  int nPos;
  File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file");
    return bRet;
  }
  //determine if the right config file
  sTemp = configFile.readStringUntil('\n');
  nPos = sTemp.indexOf(':');
  if (sTemp.substring(0, nPos) != "Version")
  {
    DBGPRINTLN("Config file is not belongs to this device");
    configFile.close();
    bSave_config();
    return bRet;
  }

  while (true)
  {
    sTemp = configFile.readStringUntil('\n');
    nPos = sTemp.indexOf(':');
    if (nPos <= 0)
      break;
    if (sTemp.substring(0, nPos) == "MAID")
    {
      gsDeviceInfo.MAID = sTemp.substring(nPos + 1);
    }
    else if (sTemp.substring(0, nPos) == "PID")
    {
      gsDeviceInfo.PID = sTemp.substring(nPos + 1);

    }
    else if (sTemp.substring(0, nPos) == "SSID")
    {
      gsDeviceInfo.wifi.ssid = sTemp.substring(nPos + 1);

    }
    else if (sTemp.substring(0, nPos) == "PASS")
    {
      gsDeviceInfo.wifi.pass = sTemp.substring(nPos + 1);

    }
    else if (sTemp.substring(0, nPos) == "SERVERADDR")
    {
      gsDeviceInfo.server.addr = sTemp.substring(nPos + 1);

    }
    else if (sTemp.substring(0, nPos) == "SERVERPORT")
    {
      gsDeviceInfo.server.port = sTemp.substring(nPos + 1);

    }
    else if (sTemp.substring(0, nPos) == "SERVERUDPADDR")
    {
      gsDeviceInfo.server.udpAddr = sTemp.substring(nPos + 1);

    }
    else if (sTemp.substring(0, nPos) == "SERVERUDPPORT")
    {
      gsDeviceInfo.server.udpPort = sTemp.substring(nPos + 1);

    }
  }

  // Real world application would store these values in some variables for later use

  DBGPRINT("Loaded MAID: ");
  DBGPRINTLN("[" + gsDeviceInfo.MAID + "]");
  DBGPRINT("Loaded PID: ");
  DBGPRINTLN("[" + gsDeviceInfo.PID + "]");

  DBGPRINT("Loaded ssid: ");
  DBGPRINTLN("[" + gsDeviceInfo.wifi.ssid + "]");
  DBGPRINT("Loaded password: ");
  DBGPRINTLN("[" + gsDeviceInfo.wifi.pass + "]");

  DBGPRINT("Loaded server_addr: ");
  DBGPRINTLN("[" + gsDeviceInfo.server.addr + "]");
  DBGPRINT("Loaded server_port: ");
  DBGPRINTLN("[" + gsDeviceInfo.server.port + "]");

  if (gsDeviceInfo.wifi.ssid != "" && gsDeviceInfo.wifi.pass != "" && gsDeviceInfo.server.addr != "" && gsDeviceInfo.server.port == "")
  {
    bRet = true;
  }
  configFile.close();

  DBGPRINTLN("Config ok");
  gbConfigOK = true;
  return bRet;
}

/* save config file, return True = success */
bool bSave_config()
{
  DBGPRINTLN("--save data--");
  DBGPRINTLN("ssid:" + gsDeviceInfo.wifi.ssid);
  //DBGPRINT(gsDeviceInfo.wifi.ssid);

  File configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }
  DBGPRINT("before print to file");
  configFile.print("Version:");
  configFile.println(gsDeviceInfo.Version);
  configFile.print("MAID:");
  configFile.println(gsDeviceInfo.MAID);
  configFile.print("PID:");
  configFile.println(gsDeviceInfo.PID);
  configFile.print("SSID:");
  configFile.println(gsDeviceInfo.wifi.ssid);
  configFile.print("PASS:");
  configFile.println(gsDeviceInfo.wifi.pass);
  configFile.print("SERVERADDR:");
  configFile.println(gsDeviceInfo.server.addr);
  configFile.print("SERVERPORT:");
  configFile.println(gsDeviceInfo.server.port);
  configFile.print("SERVERUDPADDR:");
  configFile.println(gsDeviceInfo.server.udpAddr);
  configFile.print("SERVERUDPPORT:");
  configFile.println(gsDeviceInfo.server.udpPort);

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}

/* load application data, return True==Success*/
bool bLoad_application_data()
{

  return true;
}

/* save application data, return True = success */
bool bSave_application_data()
{
  return true;
}



