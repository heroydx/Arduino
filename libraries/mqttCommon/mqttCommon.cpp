#include "mqttCommon.h"

//Modified by Steven Lian on 2017/06/29
/*
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

#ifdef DEBUG_SIO
char dispBuff[200];
#endif
//ESP8266Client ghTcpCommand(TCP, CONST_BIG_BUFF_SIZE);
//ESP8266Client ghTcpPush(TCP, CONST_PUSH_BUFF_SIZE);
//ESP8266Client ghUdpLocal(UDP, CONST_UDP_BUFF_SIZE);

static ListArray serverCommand(CONST_SERVER_COMMAND_LIST_LEN, sizeof(stLVCommmandList));
static stLVCommmandList filterData;

static void private_filter_server_command()
{
  short i;
  short nLen;
  short cmd = 0;
  nLen = serverCommand.len();
  if (nLen < 2)
    return;
  for (i = 0; i < nLen; i++)
  {
    short pos;
    // to check repeat data ; only check 3001 command
    pos = serverCommand.lpop(&filterData);
    //DBGPRINTF("\n=== filter [%d] pos [%d],len[%d] head[%d],tail[%d]",filterData.command,pos,serverCommand.len(),serverCommand.head,serverCommand.tail);

    if (cmd == CONST_CMD_3001) {
      if ((filterData.command != CONST_CMD_3001) || (filterData.beginTick != 0) || (filterData.endTick != 0)) {
        //if ((filterData.command != CONST_CMD_3001)) {
        //not 3001 or special 3001
        serverCommand.rpush(&filterData);
      }
      //=3001, trash
    }
    else {
      //if (filterData.command == CONST_CMD_3001 && filterData.beginTick==0) {
      if (filterData.command == CONST_CMD_3001 ) {
        //first 3001
        cmd = CONST_CMD_3001;
      }
      serverCommand.rpush(&filterData);
    }
  }
}

/*--------------------------------------------------------------------------------*/
//COMMON_FUCTION_PART_BEGIN
/*--------------------------------------------------------------------------------*/

LVCommon::LVCommon()
{
  //初始化数据，内存分批比例是1倍
  vInit_common_data(1);
}

LVCommon::LVCommon(int nType)
{

  switch (nType)
  {
    case CONST_DOUBLE_SIZE:
      //初始化数据，内存分批比例是2倍
      nType = 2;
      break;
    case 0:
    default:
      //初始化数据，内存分批比例是1倍
      nType = 1;
      break;

  }
  vInit_common_data(nType);

};


// init all common data
void ICACHE_FLASH_ATTR LVCommon::vInit_common_data(int nType)
{
  int i;
  gchMainState = INIT_STATE;   // 0 INIT_STATE  initial state
  gnServerCommand = 0;
  gnExecActionID = 0;
  //gnSendDatatStaus = 0;
  //gu8PowerOn = 0;
  gu8PowerOn = CONST_POWERON_FLAG_NO;

  //device info
  memset(&gsDeviceInfo, 0, sizeof(stDeviceCoreDataStruct));

  strcpy(gsDeviceInfo.server.url, CONST_DEFAULT_SERVER_URL_DATA);
  strcpy(gsDeviceInfo.server.addr, CONST_DEFAULT_SERVER_ADDR_DATA);
  gsDeviceInfo.server.port = CONST_DEFAULT_SERVER_PORT_DATA;
  strcpy(gsDeviceInfo.server.pushUrl, CONST_DEFAULT_PUSH_URL_DATA);
  strcpy(gsDeviceInfo.server.udpAddr, CONST_DEFAULT_BIND_ADDR_DATA);
  gsDeviceInfo.server.udpPort = CONST_DEFAULT_UDP_ADDR_DATA;
  strcpy(gsDeviceInfo.server.bindAddr, CONST_DEFAULT_BIND_ADDR_DATA);
  gsDeviceInfo.server.bindPort = CONST_DEFAULT_BIND_SEND_PORT_DATA;
  gsDeviceInfo.server.recvPort = CONST_DEFAULT_BIND_RECV_PORT_DATA;

  strcpy(gsDeviceInfo.wifi.ssid, CONST_DEFAULT_WIFI_SSID_DATA);
  strcpy(gsDeviceInfo.wifi.pass, CONST_DEFAULT_WIFI_PASS_DATA);

  //strncpy(LVDATA.gsDeviceInfo.Version, "", CONST_SYS_VERSION_LENGTH);
  //strncpy(LVDATA.gsDeviceInfo.SWVN, "LVSW", CONST_SYS_VERSION_LENGTH);
  //strncpy(LVDATA.gsDeviceInfo.HWVN, "LVSW", CONST_SYS_VERSION_LENGTH);
  //strncpy(LVDATA.gsDeviceInfo.MAID, "9", CONST_SYS_MAID_LENGTH);
  //strncpy(LVDATA.gsDeviceInfo.PID, "906", CONST_SYS_PID_LENGTH);
  //strncpy(LVDATA.gsDeviceInfo.CAT, "wdSd", CONST_SYS_CAT_LENGTH);

  gbConfigOK = false;

  gnWiFiStat = 0;

  bConnectWiFi = false;

  gTime.year = 2016;
  gTime.month = 11;
  gTime.day = 16;
  gTime.hour = 12;
  gTime.minute = 0;
  gTime.second = 0;
  gTime.wday = 0;
  gTime.timezone2 = 8*2;
  gTime.synced = false;

  i = 0;
  //DBGPRINT("Free heap before ghpTcpCommand: ");
  //DBGPRINTLN( ESP.getFreeHeap());
  //anFreeHeapInfo[i] = ESP.getFreeHeap();
  i++;
  ghpTcpCommand = new ESP8266Client(TCP, (CONST_BIG_BUFF_SIZE + 256) * nType);
  //DBGPRINT("Free heap after ghpTcpCommand: ");
  //DBGPRINTLN( ESP.getFreeHeap());
  //anFreeHeapInfo[i] = ESP.getFreeHeap();
  i++;
#ifdef NEED_PUSH_FUNCTION
  ghpTcpPush = new ESP8266Client(TCP, CONST_PUSH_BUFF_SIZE);
  //DBGPRINT("Free heap after ghpTcpPush: ");
  //DBGPRINTLN( ESP.getFreeHeap());
  //anFreeHeapInfo[i] = ESP.getFreeHeap();
  i++;
#endif

  ghpUdpLocal = new ESP8266Client(UDP, CONST_UDP_BUFF_SIZE * nType);
  //DBGPRINT("Free heap after ghpUdpLocal: ");
  //DBGPRINTLN( ESP.getFreeHeap());
  //anFreeHeapInfo[i] = ESP.getFreeHeap();
  i++;

  gBigBuff.chpData = new char [CONST_BIG_BUFF_SIZE * nType];
  //DBGPRINT("Free heap after gBigBuff: ");
  //DBGPRINTLN( ESP.getFreeHeap());
  //anFreeHeapInfo[i] = ESP.getFreeHeap();
  i++;

  gCommandBuff.chpData = new char [CONST_BIG_BUFF_SIZE * nType];
  //DBGPRINT("Free heap after gCommandBuff: ");
  //DBGPRINTLN( ESP.getFreeHeap());
  //anFreeHeapInfo[i] = ESP.getFreeHeap();
  i++;

  strcpy(gachServerSeed, "12345"); //Server Seeds
  //CONST_LED_STATUS_SELFDEF
  asLedDisplay[CONST_LED_STATUS_SELFDEF].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_SELFDEF].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_SELFDEF].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_SELFDEF].blinkLoop[0] = 3;
  asLedDisplay[CONST_LED_STATUS_SELFDEF].blinkLoop[1] = 6;
  asLedDisplay[CONST_LED_STATUS_SELFDEF].blinkLoop[2] = 9;
  asLedDisplay[CONST_LED_STATUS_SELFDEF].blinkLoop[3] = 19;

  //CONST_LED_STATUS_SMARTCONFIG
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].blinkLoop[0] = 2;
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].blinkLoop[1] = 4;
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].blinkLoop[2] = 6;
  asLedDisplay[CONST_LED_STATUS_SMARTCONFIG].blinkLoop[3] = 18;

  //CONST_LED_STATUS_CONNECTING
  asLedDisplay[CONST_LED_STATUS_CONNECTING].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_CONNECTING].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_CONNECTING].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_CONNECTING].blinkLoop[0] = 4;
  asLedDisplay[CONST_LED_STATUS_CONNECTING].blinkLoop[1] = 8;
  asLedDisplay[CONST_LED_STATUS_CONNECTING].blinkLoop[2] = 12;
  asLedDisplay[CONST_LED_STATUS_CONNECTING].blinkLoop[3] = 16;

  //CONST_LED_STATUS_SERVER
  asLedDisplay[CONST_LED_STATUS_SERVER].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_SERVER].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_SERVER].blinkCount = 2;
  asLedDisplay[CONST_LED_STATUS_SERVER].blinkLoop[0] = 10;
  asLedDisplay[CONST_LED_STATUS_SERVER].blinkLoop[1] = 20;

  //CONST_LED_STATUS_ONE
  asLedDisplay[CONST_LED_STATUS_ONE].loopFlag = 1;
  asLedDisplay[CONST_LED_STATUS_ONE].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_SERVER].blinkCount = 2;
  asLedDisplay[CONST_LED_STATUS_SERVER].blinkLoop[0] = 2;
  asLedDisplay[CONST_LED_STATUS_SERVER].blinkLoop[1] = 8;

  //CONST_LED_STATUS_TEST_FINISH
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].blinkLoop[0] = 5;
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].blinkLoop[1] = 10;
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].blinkLoop[2] = 15;
  asLedDisplay[CONST_LED_STATUS_TEST_FINISH].blinkLoop[3] = 20;
}


LVCommon::~LVCommon()
{

  delete ghpTcpCommand;
#ifdef NEED_PUSH_FUNCTION
  delete ghpTcpPush;
#endif
  delete ghpUdpLocal;

  delete gBigBuff.chpData;
  delete gCommandBuff.chpData;

}

//把月字符串，转换成数字， 例如： Jan=1,Feb=2
short ICACHE_FLASH_ATTR LVCommon::str2month (char *strMonth)
{
  short month = 0;
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
void ICACHE_FLASH_ATTR LVCommon::vSync_time_fromServer(char achYMD[])
{
  short yy;
  short hh, mm, ss, dd, mon;
  char buff[10];

  memcpy(buff, &achYMD[10], 3);
  buff[3] = 0;
  dd = (short) atoi(buff);

  memcpy(buff, &achYMD[14], 3);
  buff[3] = 0;
  mon = str2month(buff);

  memcpy(buff, &achYMD[18], 4);
  buff[4] = 0;
  yy = (short) atoi(buff);

  memcpy(buff, &achYMD[23], 2);
  buff[2] = 0;
  hh = (short)atoi(buff);

  memcpy(buff, &achYMD[26], 2);
  buff[2] = 0;
  mm = (short) atoi(buff);

  memcpy(buff, &achYMD[29], 2);
  buff[2] = 0;
  ss = (short)atoi(buff);

  gTime.year = yy;
  gTime.month = mon;
  gTime.day = dd;
  gTime.hour = hh;
  gTime.minute = mm;
  gTime.second = ss;
  if((gTime.timezone2<-24)||(gTime.timezone2>24))
    gTime.timezone2 = 8 * 2; //default

  //DBGPRINTF("\n%d %d %d %d %d %d tz:%d", gTime.year, gTime.month, gTime.day, gTime.hour, gTime.minute, gTime.second,gTime.timezone2);

  setTime(hh, mm, ss, dd, mon, yy);
  vGet_time();
  //DBGPRINTF("\n%d %d %d %d %d %d tz:%d", gTime.year, gTime.month, gTime.day, gTime.hour, gTime.minute, gTime.second,gTime.timezone2);
  gTime.synced = true;
}


//sync the date from server http header, normally, get the date from 3006 message
void ICACHE_FLASH_ATTR LVCommon::vSyncTime_998(char * achYMDHMS)
{
  time_t t;
  int yy;
  short hh, mm, ss, dd, mon, timezone2;
  /*
    yy = sYMDHMS.substring(0, 4).toInt();
    mon = sYMDHMS.substring(4, 6).toInt();
    dd = sYMDHMS.substring(6, 8).toInt();
    hh = sYMDHMS.substring(8, 10).toInt();
    mm = sYMDHMS.substring(10, 12).toInt();
    ss = sYMDHMS.substring(12, 14).toInt();
    timezone2 = sYMDHMS.substring(14, 16).toInt();
  */
  timezone2 = (short)atoi(&achYMDHMS[14]);
  achYMDHMS[14] = '\0';
  ss = (short)atoi(&achYMDHMS[12]);
  achYMDHMS[12] = '\0';
  mm = (short)atoi(&achYMDHMS[10]);
  achYMDHMS[10] = '\0';
  dd = (short) atoi(&achYMDHMS[8]);
  achYMDHMS[8] = '\0';
  hh = (short) atoi(&achYMDHMS[6]);
  achYMDHMS[6] = '\0';
  mon = (short)atoi(&achYMDHMS[4]);
  achYMDHMS[4] = '\0';
  yy = (short) atoi(achYMDHMS);

  gTime.timezone2 = timezone2;

  setTime(hh, mm, ss, dd, mon, yy);
  //t = now() -  1800 * gTime.timezone2;
  //一个timezone2 是30分钟=1800秒
  t = now() -  1800 * gTime.timezone2;
  setTime(t);
  t = now();

  gTime.year = year(t);
  gTime.month = month(t);
  gTime.day = day(t);
  gTime.hour = hour(t);
  gTime.minute = minute(t);
  gTime.second = second(t);
  if((timezone2<-24)||(timezone2>24))
    gTime.timezone2 = 8 * 2; //default GMT8
  gTime.synced = true;
}

//get current system time, used by application
void ICACHE_FLASH_ATTR LVCommon::vGet_time()
{
  time_t t;
  //copy to preTime
  memcpy(&gPreTime, &gTime, sizeof(gTime));
  t = now();
  t += 1800 * gTime.timezone2;
  gTime.year = year(t);
  gTime.month = month(t);
  gTime.day = day(t);
  gTime.hour = hour(t);
  gTime.minute = minute(t);
  gTime.second = second(t);
  gTime.wday = weekday(t);
  sprintf(gTime.timeString, "%04d%02d%02d%02d%02d%02d", gTime.year, gTime.month, gTime.day, gTime.hour, gTime.minute, gTime.second);
}

/*
   conver a hex data to ascii format
   ex: 0x1--> 0x31 ('1')
*/
uint8_t ICACHE_FLASH_ATTR LVCommon::ascii(int hex)
{
  uint8_t cc;
  cc = 0x30;
  switch (hex)
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      cc = 0x30 + hex;
      break;
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      cc = 'A' + hex - 0x0A;
  }
  //DBGPRINTF("hex=[%x]\n",hex);
  //DBGPRINTF("cc=[%x]\n",cc);
  return cc;
}


/*
   conver a ascii data to int format
   ex: 0x31('1')--> 0x1 ('1')
*/
uint8_t ICACHE_FLASH_ATTR LVCommon::ascii_hex(uint8_t cc)
{
  uint8_t hex;
  hex = 0x00;
  switch (cc)
  {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      hex = cc - 0x30;
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      hex = cc - 'a' + 0x0A;
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      hex = cc - 'A' + 0x0A;
  }
  return hex;
}

//return the pointer to trimed string;
char * ICACHE_FLASH_ATTR LVCommon::strTrim(char *str)
{
  char *p = str;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    p ++;
  str = p;
  p = str + strlen(str) - 1;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    -- p;
  *(p + 1) = 0;
  return str;
}

// return a char position in a char *
int ICACHE_FLASH_ATTR LVCommon::strchrPos(char achData[], char chData)
{
  int nRet = -1, pos = 0;
  while (1)
  {
    if (achData[pos] == '\0')
      break;
    if (achData[pos] == chData)
    {
      nRet = pos;
      break;
    }
    pos++;
  }
  return nRet;
}


void ICACHE_FLASH_ATTR LVCommon::debug_print_time()
{
#ifdef DEBUG_SIO
  sprintf(dispBuff, "%04d-%02d-%02d %02d %02d:%02d:%02d", gTime.year, gTime.month, gTime.day, gTime.wday, gTime.hour, gTime.minute, gTime.second);
  DBGPRINTLN(dispBuff);
#endif
}


void ICACHE_FLASH_ATTR LVCommon::debug_print_wifi_status()
{
#ifdef DEBUG_SIO
  /*
    WL_CONNECTED: assigned when connected to a WiFi network;
    WL_NO_SHIELD: assigned when no WiFi shield is present;
    WL_IDLE_STATUS: it is a temporary status assigned when WiFi.begin() is called and remains active until the number of attempts expires (resulting in WL_CONNECT_FAILED) or a connection is established (resulting in WL_CONNECTED);
    WL_NO_SSID_AVAIL: assigned when no SSID are available;
    WL_SCAN_COMPLETED: assigned when the scan networks is completed;
    WL_CONNECT_FAILED: assigned when the connection fails for all the attempts;
    WL_CONNECTION_LOST: assigned when the connection is lost;
    WL_DISCONNECTED: assigned when disconnected from a network;
  */

  switch (gnWiFiStat)
  {
    case WL_CONNECTED:
      sprintf(dispBuff, "[%d],WL_CONNECTED", gnWiFiStat);
      break;
    case WL_NO_SHIELD:
      sprintf(dispBuff, "[%d],WL_NO_SHIELD", gnWiFiStat);
      break;
    case WL_IDLE_STATUS:
      sprintf(dispBuff, "[%d],WL_IDLE_STATUS", gnWiFiStat);
      break;
    case WL_NO_SSID_AVAIL:
      sprintf(dispBuff, "[%d],WL_NO_SSID_AVAIL", gnWiFiStat);
      break;
    case WL_SCAN_COMPLETED:
      sprintf(dispBuff, "[%d],WL_SCAN_COMPLETED", gnWiFiStat);
      break;
    case WL_CONNECT_FAILED:
      sprintf(dispBuff, "[%d],WL_CONNECT_FAILED", gnWiFiStat);
      break;
    case WL_CONNECTION_LOST:
      sprintf(dispBuff, "[%d],WL_CONNECTION_LOST", gnWiFiStat);
      break;
    case WL_DISCONNECTED:
      sprintf(dispBuff, "[%d],WL_DISCONNECTED", gnWiFiStat);
      break;
    default:
      break;
  }
  DBGPRINTLN(dispBuff);
#endif
}


//nChange_WiFi_Mode(WL_DISCONNECTED);
int ICACHE_FLASH_ATTR LVCommon::nChange_WiFi_Mode(int stat)
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

// to setup or change the LED display status, the status include:CONST_LED_STATUS_ALWAYSON,CONST_LED_STATUS_SMARTCONFIG,etc.
void ICACHE_FLASH_ATTR LVCommon::vChange_LED_status(uint8_t u8Status)
{
  if (gu8LEDStatus != u8Status)
  {
    gu8LEDStatus = u8Status;
    gu8LEDLoopCount = 0;
    gu8LEDDispCount = 0;
  }
}

void ICACHE_FLASH_ATTR LVCommon::vLED_ON(uint8_t onColor)
{
  if (onColor & CONST_LED_COLOR_RED_MASK)
    digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
  if (onColor & CONST_LED_COLOR_GREEN_MASK)
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
  if (onColor & CONST_LED_COLOR_BLUE_MASK)
    digitalWrite(CONST_LED_COLOR_BLUE_PIN, HIGH);
}

void ICACHE_FLASH_ATTR LVCommon::vLED_ON()
{
  if (CONST_LED_COLOR_RED_MASK)
    digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);

}

void ICACHE_FLASH_ATTR LVCommon::vLED_OFF()
{
  digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
}

void ICACHE_FLASH_ATTR LVCommon::vLED_Blink()
{
  vLED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
  delay(50);
  vLED_OFF();
}

void ICACHE_FLASH_ATTR LVCommon::vDisplay_LED_color(void)
{
  if (gu8LEDStatus == CONST_LED_STATUS_ALWAYSOFF)
  {
    //DBGPRINTLN("DISPLAY ALWAYS OFF");
    vLED_OFF();
    //DBGPRINT("_");

  }
  else if  (gu8LEDStatus == CONST_LED_STATUS_ALWAYSON)
  {
    //DBGPRINTLN("DISPLAY ALWAYS ON");
    vLED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
    //DBGPRINT("T");
  }
  else
  {
    if (gu8LEDLoopCount < asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
      //if (gu8LEDLoopCount == asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
    {
      if ((gu8LEDDispCount % 2) == 0)
      {
        vLED_ON(asLedDisplay[gu8LEDStatus].onColor);
        //digitalWrite(CONST_LED_COLOR_RED_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_RED_MASK );
        //digitalWrite(CONST_LED_COLOR_GREEN_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_GREEN_MASK);
        //digitalWrite(CONST_LED_COLOR_BLUE_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_BLUE_MASK);
        //DBGPRINT("T");
      }
      else
      {
        //DBGPRINTLN("DISPLAY OFF");
        digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
        digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
        digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
        //DBGPRINT("_");
      }
      //DBGPRINT(gu8LEDDispCount);

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
}


void ICACHE_FLASH_ATTR LVCommon::vLVConfig_udp_send(ESP8266Client *hUDP)
{
  sprintf(gBigBuff.chpData, "{\"state\":\"offLine\",\"devSn\":\"%s\",\"swvn\":\"%s\",\"hwvn\":\"%s\",\"time\":\"60\"}", IMEI, gsDeviceInfo.SWVN, gsDeviceInfo.HWVN);
  hUDP->write((unsigned char *) gBigBuff.chpData, strlen(gBigBuff.chpData));

}

void ICACHE_FLASH_ATTR LVCommon::vDetermine_NetworkStatus(int nRet)
{
  WiFi.status() != WL_CONNECTED;
  //gchMainState = WAIT_CONNECTION_STATE;
  //ESP.restart();
  vSystem_restart(CONST_SYS_RESTART_REASON_NETWORK_ERROR);
}
;

int ICACHE_FLASH_ATTR LVCommon::nCommonPOST(char achData[])
{
  nCommonPOST(achData, true);
  //nCommonPOST(achData,false);
}

int ICACHE_FLASH_ATTR LVCommon::nCommonPOST(char achData[], bool encodeFlag)
{
  int ret;
  //int nTryTimes = CONST_MAX_NET_WRITE;
  //DBGPRINTLN("achData:");
  //DBGPRINTLN(achData);
  if (encodeFlag && gTime.synced == true)
  {
    bEncode(achData);
  }
  sprintf(gCommandBuff.chpData, "POST %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n",
          gsDeviceInfo.server.url, gsDeviceInfo.server.addr, strlen(achData));
  //DBGPRINTLN("gCommandBuff.chpData");
  //DBGPRINTLN(gCommandBuff.chpData);
  //DBGPRINTLN(strlen(gCommandBuff.chpData));
  strcat(gCommandBuff.chpData, achData);
  DBGPRINTLN("==postReq BEGIN==");
  //DBGPRINTLN("gCommandBuff.chpData");
  //DBGPRINTLN(gCommandBuff.chpData);
  ret = ghpTcpCommand->write((unsigned char *)gCommandBuff.chpData, strlen(gCommandBuff.chpData));
  //DBGPRINTF("\n--- ret = ghpTcpCommand->write [%d]",ret);
  if (ret == 0) {
    //gnServerCommand = 0;
  }
  else {
    gnNetPostFailCount++;
    //temporary solution
    //gnServerCommand = 0;
  }
  delay(50);
  //vReset_interval(glMinPostIntervalTick);
  //glMinPostIntervalTick = ulReset_interval();
  DBGPRINTLN("==postReq END==");
  ulForcePOSTCheckTick = ulReset_interval();
  return ret;
}


bool ICACHE_FLASH_ATTR LVCommon::bSend_3006(bool firstReg, char achSeed[])
{
  int ret;
  byte  *bssid;
  //byte mac[6];
  char chReg;
  DBGPRINTLN("--send 3006--");

  bssid = WiFi.BSSID(); //WiFi.BSSID(bssid);
  memcpy(gsDeviceInfo.bssid, bssid, CONST_MAC_ADDR_LEN);
  WiFi.macAddress(gsDeviceInfo.macAddr);

  if (firstReg)
  {
    chReg = 'y';
  }
  else
  {
    chReg = 'x';
  }
  //int nSeed = random(10000, 60000);
  //sprintf (gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"devVER\":\"%s\",\"MAID\":\"%s\",\"PID\":\"%s\",\"WifiMac\":\"%02X-%02X-%02X-%02X-%02X-%02X\",\"RegInfo\":\"%c\",\"ROMVersion\":\"%s\",\"HWVersion\":\"%s\",\"mac\":\"%02X-%02X-%02X-%02X-%02X-%02X\",\"seed\":\"%s\"}",
  //         CONST_CMD_3006, IMEI, gsDeviceInfo.Version, gsDeviceInfo.MAID, gsDeviceInfo.PID, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], chReg, gsDeviceInfo.SWVN, gsDeviceInfo.HWVN,
  //         gsDeviceInfo.macAddr[0], gsDeviceInfo.macAddr[1], gsDeviceInfo.macAddr[2],  gsDeviceInfo.macAddr[3], gsDeviceInfo.macAddr[4], gsDeviceInfo.macAddr[5], achSeed);
  sprintf (gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"MAID\":\"%s\",\"PID\":\"%s\",\"WifiMac\":\"%02X-%02X-%02X-%02X-%02X-%02X\",\"RegInfo\":\"%c\",\"ROMVersion\":\"%s\",\"HWVersion\":\"%s\",\"mac\":\"%02X-%02X-%02X-%02X-%02X-%02X\",\"boot\":\"%d\",\"seed\":\"%s\"}",
           CONST_CMD_3006, IMEI, gsDeviceInfo.MAID, gsDeviceInfo.PID, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], chReg, gsDeviceInfo.SWVN, gsDeviceInfo.HWVN,
           gsDeviceInfo.macAddr[0], gsDeviceInfo.macAddr[1], gsDeviceInfo.macAddr[2],  gsDeviceInfo.macAddr[3], gsDeviceInfo.macAddr[4], gsDeviceInfo.macAddr[5], nRestartReason, achSeed);

  // This will send the POST request to the server
  DBGPRINTLN("debug send 3006 gBigBuff.achData:");
  DBGPRINTLN(gBigBuff.chpData);
  ret = nCommonPOST(gBigBuff.chpData, true);
  //reset boot reason
  nRestartReason = CONST_SYS_RESTART_REASON_NORMAL_BOOT;
  bSave_config();

  DBGPRINTLN("--end--");

  return true;
}

#ifdef NEED_PUSH_FUNCTION
bool ICACHE_FLASH_ATTR LVCommon::bSend_Push()
{
  int ret;
  DBGPRINTLN("--send_Push--");
  // We now create a URI for the request

  sprintf(gCommandBuff.chpData, "GET %s?devID=%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", gsDeviceInfo.server.pushUrl, IMEI, gsDeviceInfo.server.addr);
  ret = ghpTcpPush->write((unsigned char *) gCommandBuff.chpData, strlen(gCommandBuff.chpData));
  //DBGPRINTF("send_Push:write length=[%d]\n", ret);
  DBGPRINTLN("--end--");

  return true;
}
#endif


int ICACHE_FLASH_ATTR LVCommon::nGet_json_pos(char achData[], char achKey[], char **spPos)
{
  int nRet = -1;
  char *spStart, *spEnd;
  int nLen;
  nLen = strlen(achKey);
  spStart = strstr(achData, achKey);
  if (spStart != NULL)
  {
    spStart += 1 + nLen;
    if (*(spStart) == ':')
    {
      spStart++;
      if ((*spStart) == '\"')
      {
        //string
        spStart++;
        spEnd = strchr((spStart), '\"');
      }
      else if ((*spStart) == '[')
      {
        //list
        spStart++;
        spEnd = strchr((spStart), ']');
        if (spEnd==NULL){
          return -1;
        }
      }
      else if ((*spStart) == '{')
      {
        //dict
        spStart++;
        spEnd = strchr((spStart), '}');
        if (spEnd==NULL){
          return -1;
        }        
      }
      else
      {
        spEnd = strchr((spStart), ',');
      }
      *spPos = spStart;
      nRet = spEnd - spStart;
    }

  }

  return nRet;
}

bool ICACHE_FLASH_ATTR LVCommon::bHandle_recvLine_body(char achData[])
{
  //push message
  if (memcmp(achData, CONST_CMD_3001_STR, CONST_CMD_LENGTH) == 0)
  {
    DBGPRINTLN("==get push message==");
    //gnServerCommand = CONST_CMD_3001;
    push_cmd(CONST_CMD_3001);

    return true;
  }
  else if (achData[0] == '{')
  {
    bool bValidCommand = false;

    char *spCMD, *spDEVID, *spDevTodo, *spActionID, *spParam, *spInst, *spInterval;
    int nCMDLen, nDEVIDLen, nDevTodoLen, nActionIDLen, nParamLen, nInstLen, nIntervalLen;
    int nCMDVal, nIntervalVal;
    char *spPos;
    //int nTEMPCount=0;

    DBGPRINTLN("------------DBG CMD-------------");
    DBGPRINTLN(achData);

    nCMDLen = nGet_json_pos(achData, "CMD", &spCMD);
    //DBGPRINTF("\n nGet_json_pos:CMD [%d]",nCMDLen);
    if (nCMDLen > 0)
    {
      char buff[CONST_CMD_LENGTH + 2];
      memcpy(buff, spCMD, CONST_CMD_LENGTH);
      buff[CONST_CMD_LENGTH] = 0;
      nCMDVal = atoi(buff);
    }
    gnNetPostFailCount = 0;
    switch (nCMDVal)
    {
      case CONST_CMD_3001:
        //gnNetPostFailCount--;
        nDEVIDLen = nGet_json_pos(achData, "devID", &spDEVID);
        nIntervalLen = nGet_json_pos(achData, "connInterval", &spInterval);
        nDevTodoLen = nGet_json_pos(achData, "devTodo", &spDevTodo);
        //DBGPRINT(" devID, connInterval,devTodo");
        if ((nDEVIDLen > 0) && (nCMDLen > 0) && (nIntervalLen > 0))
        {
          //split all string;
          *(spCMD + nCMDLen) = '\0';
          *(spDEVID + nDEVIDLen) = '\0';
          *(spInterval + nIntervalLen) = '\0';
          nIntervalVal = atoi(spInterval);
          //DBGPRINT("Interval:");
          //DBGPRINTLN(nIntervalVal);
          if (nIntervalVal > 1)
          {
            glConnIntervalTick = (long) nIntervalVal * 1000;
          }
        }
        
        if (nDevTodoLen<=0){
          if (nDevTodoLen==0){
            DBGPRINT("!!! nothing do to  !!!");          
          }
          else{
            DBGPRINT("!!! nDevTodoLen Error !!!");
          }
          break;
        }

        *(spDevTodo + nDevTodoLen) = '\0';        

        //DBGPRINTLN(spCMD);
        //DBGPRINTLN(spDEVID);
        //DBGPRINTLN(spInterval);
        //DBGPRINTLN(spDevTodo);
        //devtodo handle;
        spPos = spDevTodo;

        while (nDevTodoLen > 0)
        {
          char *spActionID, *spParam, *spInst;
          int nActionIDLen, nParamLen, nInstLen;
          int nActionID;

          spPos = strchr(spPos, '{');
          if (spPos == NULL)
          {
            break;
          }
          //nTEMPCount++;
          //DBGPRINTLN("====== DEVTODO  BEGIN =====");
          //DBGPRINTLN(nTEMPCount);
          //DBGPRINTLN("====== DEVTODO  END =====");

          nActionIDLen = nGet_json_pos(spPos, "actionID", &spActionID);


          if (nActionIDLen <= 0)
          {
            bValidCommand = false;
            break;
          }

          nParamLen = nGet_json_pos(spPos, "param", &spParam);

          nInstLen = nGet_json_pos(spPos, "inst", &spInst);
          
          //DBGPRINT(" before strchr(spPos, '}')");

          spPos = strchr(spPos, '}');
          
          if (spPos == NULL) {
            DBGPRINT(" after strchr(spPos, '}') break");
            break;
          }
          *spPos = '\0';
          spPos++;

          bValidCommand = true;
          if (bValidCommand)
          {
            char achNull[2] = "";
            *(spActionID + nActionIDLen) = '\0';
            if (nParamLen > 0)
              *(spParam + nParamLen) = '\0';
            else
              spParam = achNull;

            if (nInstLen > 0)
              *(spInst + nInstLen) = '\0';
            else
              spInst = achNull;

            DBGPRINTF("\n actionID:[%s], param [%s], inst[%s]", spActionID, spParam, spInst);
            //DBGPRINTLN(nActionIDLen);
            //DBGPRINTLN(spActionID);
            //DBGPRINTLN(nParamLen);
            //DBGPRINTLN(spParam);
            //DBGPRINTLN(nInstLen);
            //DBGPRINTLN(spInst);

            nActionID = atoi(spActionID);
            bExec_serverCommand(nActionID, spParam, spInst);
            //vChange_LED_status(CONST_LED_STATUS_ONE);
            //vLED_Blink();
          }
        }
        break;

      case CONST_CMD_3006:
        break;

      case CONST_CMD_9999:
        break;

      default:
        break;
    }
    DBGPRINTLN("------------DBG END-------------");
    //DBGPRINTLN("Finish Command!");
  }
  else  //error
  {
    DBGPRINTLN("Get Body Error");
    DBGPRINTLN(achData);
    return false;
  }

}

bool ICACHE_FLASH_ATTR LVCommon::bHandle_recvLine_date(char achData[])
{
  //Now use server time to update time display

  char buff[10];
  int hh;

  memcpy(buff, &achData[23], 2);
  buff[2] = 0;
  hh = atoi(buff);
  hh += (gTime.timezone2 / 2);
  if (hh >= 24)
    hh -= 24;
  if (hh < 0)
    hh += 24;

  if (hh != gTime.hour || gnTimeSyncUpdate)
  {
    DBGPRINTF("==sync time== [%s] ",achData);
    vSync_time_fromServer(achData);
    gnTimeSyncUpdate = 0;
  }

  vGet_time();
  //DBGPRINTF("time string [%s] ",gTime.timeString);

  return true;
}



bool ICACHE_FLASH_ATTR LVCommon::bHandle_recvLine_main(char achData[])
{
  bool ret = true;
  if (memcmp(achData, "HTTP/", 5) == 0)
  {
    if (memcmp(&achData[9], "200", 3) != 0)
    {
      char buff[10];
      memcpy(buff, &achData[9], 3);
      buff[3] = '\0';
      //DBGPRINTF("\nHTTP Status Code: [%s]\n", buff );
      ret =true;
    }
  }
  else if (memcmp(achData, "Server:", 7) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Date:", 5) == 0)
  {
    ret = bHandle_recvLine_date(achData);
  }
  else if (memcmp(achData, "Content-Type:", 13) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Last-Modified:", 14) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Etag:", 5) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Vary:", 5) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Content-Length:", 15) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Connection:", 11) == 0)
  {
    ret = true;
  }
  else if (memcmp(achData, "Transfer-Encoding:", 18) == 0)
  {
    ret = true;
  }
  else if (achData[0] == '\0') // End HTTP Header
  {
    ret = true;
  }
  else    // body
  {
    ret = bHandle_recvLine_body(achData);
  }
  return ret;
}


int ICACHE_FLASH_ATTR LVCommon::nhandle_server_response(uint8_t *data, size_t len)
{
  int posStart;
  int posEnd;
  int pos;
  bool ret;

  //DBGPRINTF("\nServerResponse: [%s][%d]\n",data,len);
  posStart = 0;
  posEnd = 0;
  for (pos = 0; pos < len; pos++)
  {
    if ((data[pos] == '\r') || (pos == len - 1))
    {
      posEnd = pos;
      if (pos == len - 1)
      {
        posEnd++;
      }
      data[posEnd] = '\0';
      //DBGPRINT(posStart);
      //DBGPRINT("-");
      //DBGPRINT(posEnd);
      //DBGPRINTF("\nparse Data:[%s]",&gCommandBuff.chpData[posStart]);

      ret = bHandle_recvLine_main(&gCommandBuff.chpData[posStart]);
      posStart = pos + 2;
      if (!ret)
        return false;
      
    }
  }
  return true;
}

void ICACHE_FLASH_ATTR LVCommon::push_cmd(short cmd, short beginTime, short endTime)
{
  short pos;
  sServerCommand.command = cmd;
  sServerCommand.tick = ulReset_interval();
  sServerCommand.beginTick = (unsigned long) beginTime * 1000L;
  sServerCommand.endTick = (unsigned long) endTime * 1000L;
  pos = serverCommand.rpush(&sServerCommand);
  DBGPRINTF("\n=== push_cmd [%d] pos [%d],len[%d] head[%d],tail[%d]",cmd,pos,serverCommand.len(),serverCommand.head,serverCommand.tail);
}

void ICACHE_FLASH_ATTR LVCommon::push_cmd(short cmd)
{
  push_cmd(cmd, 0, 0);
}


short ICACHE_FLASH_ATTR LVCommon::pop_cmd()
{
  short ret = 0;
  short pos;
  //DBGPRINTF("\n=== serverCommand len =[%d] ", serverCommand.len());
  private_filter_server_command();
  pos = serverCommand.lpop(&sServerCommand);
  //DBGPRINTF("\n=== pop [%d] pos [%d],len[%d] head[%d],tail[%d]",filterData.command,pos,serverCommand.len(),serverCommand.head,serverCommand.tail);
  //DBGPRINTF("serverCommand=[%d]\n", sServerCommand.command);
  if (sServerCommand.beginTick != 0 || sServerCommand.endTick != 0)
  {
    //时间判断;
    if (ulGet_interval(sServerCommand.tick) < sServerCommand.beginTick) {
      //时间没有到，重新放回队列
      serverCommand.rpush(&sServerCommand);
      ret = 0;
    }
    else {
      if (ulGet_interval(sServerCommand.tick) < sServerCommand.endTick) {
        //合适，去执行这个命令
        ret = sServerCommand.command;
      }
      else {
        //过期，抛弃这个命令
        ret = 0;
      }
    }
  }
  else {
    ret = sServerCommand.command;
  }
  //DBGPRINTF(" [%d]", ret);
  return ret;
}



void ICACHE_FLASH_ATTR LVCommon::vRefresh_data_3001()
{
  /*
    glLastCommandTick = millis() - glConnIntervalTick - 100;
    if (glLastCommandTick<0){
    glLastCommandTick+=0xffffffff;
    }
  */
  push_cmd(CONST_CMD_3001);
}

void LVCommon::vTimer_interrupt100ms(void *pArg)
{

  vDisplay_LED_color();

}

bool ICACHE_FLASH_ATTR LVCommon::func_cmd_901(char  achParam[], char achInst[])
{
  bool ret = true;
  push_cmd(CONST_CMD_9999);
  {
    t_httpUpdate_return httpRet;
    int pos, pos2, nLen;
    char *spStart, *spEnd;
    nLen = strlen(achParam);
    pos2 = 0;
    for (pos = 0; pos < nLen; pos += 2)
    {
      achParam[pos2] = ((ascii_hex(achParam[pos]) * 16 + ascii_hex(achParam[pos + 1]))) & 0xFF;
      pos2++;
    }
    achParam[pos2] = '\0';
    DBGPRINT("param=[");
    DBGPRINT(achParam);
    DBGPRINTLN("]");
    nRestartReason=CONST_SYS_RESTART_REASON_SW_UPGRADE;
    bSave_config();
    delay(50);
    httpRet = ESPhttpUpdate.update(achParam);
    switch (httpRet)
    {
      case HTTP_UPDATE_FAILED:
        DBGPRINTF("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        DBGPRINTLN("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        DBGPRINTLN("HTTP_UPDATE_OK");
        delay(1000);
        //ESP.restart();
        vSystem_restart(CONST_SYS_RESTART_REASON_SW_UPGRADE);
        break;
    }
  }
  //gnServerCommand = CONST_CMD_9999;
  return ret;
}



bool ICACHE_FLASH_ATTR LVCommon::func_cmd_904(char  achParam[], char achInst[])
{
  bool ret = true;
  push_cmd(CONST_CMD_9999);
  gsDeviceInfo.wifi.ssid[0] = '\0';
  gsDeviceInfo.wifi.pass[0] = '\0';
  gbConfigOK=false;
  bSave_config();
  nChange_WiFi_Mode(WL_DISCONNECTED);
  gchMainState = INIT_STATE;
  return ret;
}


bool ICACHE_FLASH_ATTR LVCommon::func_cmd_905(char  achParam[], char achInst[])
{
  bool ret = false;
  int pos, pos2, nLen;
  char *spStart, *spEnd;
  char *spAddr, *spPort, *spSSID, *spPass;
  nLen = strlen(achParam);
  pos2 = 0;
  for (pos = 0; pos < nLen; pos += 2)
  {
    achParam[pos2] = ((ascii_hex(achParam[pos]) * 16 + ascii_hex(achParam[pos + 1]))) & 0xFF;
    pos2++;
  }
  achParam[pos2] = '\0';

  DBGPRINTF("ActiodID param:[%s]\n", achParam);

  spStart = achParam;
  spEnd = strchr(spStart, ':');
  if (spEnd == NULL)
    //break;
    return ret;
  *(spEnd) = '\0';
  spEnd++;
  spAddr = spStart;
  spStart = spEnd;

  spEnd = strchr(spStart, '@');
  if (spEnd == NULL)
    //break;
    return ret;
  *(spEnd) = '\0';
  spEnd++;
  spPort = spStart;
  spStart = spEnd;

  spEnd = strchr(spStart, ':');
  if (spEnd == NULL)
    //break;
    return ret;
  *(spEnd) = '\0';
  spEnd++;
  spSSID = spStart;
  spPass = spEnd;

  strcpy(gsDeviceInfo.server.addr, spAddr);
  gsDeviceInfo.server.port = atoi(spPort);
  strcpy(gsDeviceInfo.wifi.ssid, spSSID);
  strcpy(gsDeviceInfo.wifi.pass, spPass);

  DBGPRINTLN(gsDeviceInfo.server.addr);
  DBGPRINTLN(gsDeviceInfo.server.port);
  DBGPRINTLN(gsDeviceInfo.wifi.ssid);
  DBGPRINTLN(gsDeviceInfo.wifi.pass);

  //bSave_config();
  //DBGPRINTLN("will restart in 1 seconds");
  //delay(1000);

  gchMainState = WAIT_CONNECTION_STATE;
  nChange_WiFi_Mode(WL_DISCONNECTED);
  ret = true;
  return ret;
}


bool ICACHE_FLASH_ATTR LVCommon::bExec_serverCommand(int nActionID, char  achParam[], char achInst[])
{
  vLED_ON();
  gnExecActionID = nActionID;

  DBGPRINTF("\nexe_serverCommand=[%d]",nActionID);

  switch (nActionID)
  {
    case 901:
      push_cmd(CONST_CMD_9999);
      func_cmd_901(achParam, achInst);
      break;

    case 904:
      push_cmd(CONST_CMD_9999);
      func_cmd_904(achParam, achInst);
      break;

    case 905:
      push_cmd(CONST_CMD_9999);
      func_cmd_905(achParam, achInst);
      break;

    case 908:
      //gnServerCommand = CONST_CMD_9999;
      push_cmd(CONST_CMD_9999);
      vRefresh_data_3001();
      break;

    //test complted command
    case 909:
      push_cmd(CONST_CMD_9999);
      glActionID909Tick = ulReset_interval();
      nChange_WiFi_Mode(WL_DISCONNECTED);
      gchMainState = FACTORY_TEST_FINISH_LOOP;
      break;

    case 998:
      push_cmd(CONST_CMD_9999);
      vSyncTime_998(achParam);
      //gnServerCommand = CONST_CMD_9999;
      break;

    default:
      break;

  }
  //gnServerCommand = nExec_application_Command(nActionID, achParam, achInst);
  nExec_application_Command(nActionID, achParam, achInst);
  delay(50);
  vLED_OFF();
  return true;
}


/* load config, return True==Success*/
bool ICACHE_FLASH_ATTR LVCommon::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s", CONFIG_FILE_NAME);
    return bRet;
  }
  //determine if the right config file
  nLen = configFile.readBytesUntil('\n', gBigBuff.chpData, CONST_BIG_BUFF_SIZE);
  if (nLen <= CONST_BIG_BUFF_SIZE && nLen > 1)
  {
    gBigBuff.chpData[nLen - 1] = '\0'; //trim
    spTemp = strchr(gBigBuff.chpData, ':');
    spTemp++;
    DBGPRINTLN(gBigBuff.chpData);
    //DBGPRINTLN(gsDeviceInfo.Version);
    //DBGPRINTLN(spTemp);

    if (memcmp(gBigBuff.chpData, "Version", 7) || memcmp(spTemp, gsDeviceInfo.Version, strlen(gsDeviceInfo.Version)))
    {
      DBGPRINTLN("Config file is not belongs to this device");
      DBGPRINTLN(spTemp);
      DBGPRINTLN(gsDeviceInfo.Version);
      //防止升级错误导致的断网，因此屏蔽下面几行代码
      //configFile.close();
      //bSave_config();
      //return bRet;
    }

    while (true)
    {
      nLen = configFile.readBytesUntil('\n', gBigBuff.chpData, CONST_BIG_BUFF_SIZE);
      if (nLen <= 0)
        break;
      gBigBuff.chpData[nLen - 1] = '\0'; //trim
      spTemp = strchr(gBigBuff.chpData, ':');
      if (spTemp == NULL)
        break;//not found;
      spTemp++;

      if (memcmp(gBigBuff.chpData, "MAID", 4) == 0)
      {
        //strncpy(gsDeviceInfo.MAID, spTemp, CONST_SYS_MAID_LENGTH);
      }
      else if (memcmp(gBigBuff.chpData, "PID", 3) == 0)
      {
        //strncpy(gsDeviceInfo.PID, spTemp, CONST_SYS_PID_LENGTH);
      }
      else if (memcmp(gBigBuff.chpData, "SSID", 4) == 0)
      {
        strncpy(gsDeviceInfo.wifi.ssid, spTemp, CONST_WIFI_SSID_LENGTH);
      }
      else if (memcmp(gBigBuff.chpData, "PASS", 4) == 0)
      {
        strncpy(gsDeviceInfo.wifi.pass, spTemp, CONST_WIFI_PASS_LENGTH);
      }
      else if (memcmp(gBigBuff.chpData, "SERVERADDR", 10) == 0)
      {
        strncpy(gsDeviceInfo.server.addr, spTemp, CONST_ADDR_LENGTH);
      }
      else if (memcmp(gBigBuff.chpData, "SERVERPORT", 10) == 0)
      {
        gsDeviceInfo.server.port = atoi(spTemp);
      }
      else if (memcmp(gBigBuff.chpData, "SERVERUDPADDR", 13) == 0)
      {
        strncpy(gsDeviceInfo.server.udpAddr, spTemp, CONST_ADDR_LENGTH);
      }
      else if (memcmp(gBigBuff.chpData, "SERVERUDPPORT", 13) == 0)
      {
        gsDeviceInfo.server.udpPort = atoi(spTemp);
      }
      else if (memcmp(gBigBuff.chpData, "BOOT", 4) == 0)
      {
        nRestartReason = atoi(spTemp);
      }

    }

    // Real world application would store these values in some variables for later use
    DBGPRINTF("Loaded MAID:[%s]\n", gsDeviceInfo.MAID);
    DBGPRINTF("Loaded PID:[%s]\n", gsDeviceInfo.PID);
    DBGPRINTF("Loaded ssid:[%s]\n", gsDeviceInfo.wifi.ssid);
    DBGPRINTF("Loaded password:[%s]\n", gsDeviceInfo.wifi.pass);
    DBGPRINTF("Loaded server_addr:[%s]\n", gsDeviceInfo.server.addr);
    DBGPRINTF("Loaded server_port:[%d]\n", gsDeviceInfo.server.port);
    DBGPRINTF("Loaded nRestartReason:[%d]\n", nRestartReason);

    if (strlen(gsDeviceInfo.wifi.ssid) && strlen(gsDeviceInfo.wifi.pass) && strlen(gsDeviceInfo.server.addr) && (gsDeviceInfo.server.port))
    {
      DBGPRINTLN("loaded data valid!");
      bRet = true;
    }
    configFile.close();
  }
  //DBGPRINTLN("Config ok");
  return bRet;
}


/* save config file, return True = success */
bool ICACHE_FLASH_ATTR LVCommon::bSave_config()
{
  DBGPRINTLN("--save data--");
  DBGPRINTF("ssid:[%s]\n", gsDeviceInfo.wifi.ssid);
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
  //DBGPRINT("before print to file");
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

  configFile.print("BOOT:");
  configFile.println(nRestartReason);

  configFile.close();
  DBGPRINTLN("-- end --");
  return true;
}


/* load IMEI, return True==Success*/
bool ICACHE_FLASH_ATTR LVCommon::bLoad_IMEI()
{
  bool bRet = false;
  int nLen;
  File configFile = SPIFFS.open(IMEI_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open IMEI file");
    return bRet;
  }
  //determine if the right config file
  nLen = configFile.readBytesUntil('\n', gBigBuff.chpData, CONST_BIG_BUFF_SIZE);
  if (nLen <= CONST_BIG_BUFF_SIZE && nLen > 10)
  {
    gBigBuff.chpData[nLen - 1] = '\0'; //trim
    strncpy(IMEI, gBigBuff.chpData, CONST_IMEI_LENGTH);

    // Real world application would store these values in some variables for later use
    DBGPRINTF("Loaded IMEI:[%s]\n", IMEI);
    bRet = true;
  }
  configFile.close();
  //DBGPRINTLN("Config ok");
  return bRet;
}


/* save IMEI file, return True = success */
bool ICACHE_FLASH_ATTR LVCommon::bSave_IMEI()
{
  DBGPRINTLN("--save IMEI data--");
  DBGPRINTF("IMEI:[%s]\n", IMEI);
  //DBGPRINT(gsDeviceInfo.wifi.ssid);

  File configFile = SPIFFS.open(IMEI_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open IMEI file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(IMEI_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open IMEI file for writing");
      return false;
    }
  }
  //DBGPRINT("before print to file");
  //configFile.print("IMEI:");
  configFile.println(IMEI);
  configFile.close();
  DBGPRINTLN("-- end --");
  return true;
}



/* load HWVN, return True==Success*/
bool ICACHE_FLASH_ATTR LVCommon::bLoad_HWVersion()
{
  bool bRet = false;
  int nLen;
  File configFile = SPIFFS.open(HWVERSION_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open HWVersion file");
    return bRet;
  }
  //determine if the right config file
  nLen = configFile.readBytesUntil('\n', gBigBuff.chpData, CONST_BIG_BUFF_SIZE);
  if (nLen <= CONST_BIG_BUFF_SIZE && nLen > 10)
  {
    gBigBuff.chpData[nLen - 1] = '\0'; //trim
    strncpy(gsDeviceInfo.HWVN, gBigBuff.chpData, CONST_SYS_VERSION_LENGTH);

    // Real world application would store these values in some variables for later use
    DBGPRINTF("Loaded HWVN:[%s]\n", gsDeviceInfo.HWVN);
    bRet = true;
  }
  configFile.close();
  //DBGPRINTLN("Config ok");
  return bRet;
}


/* save HWVN file, return True = success */
bool ICACHE_FLASH_ATTR LVCommon::bSave_HWVersion()
{
  DBGPRINTLN("--save HWVN data--");
  DBGPRINTF("HWVN:[%s]\n", gsDeviceInfo.HWVN);
  //DBGPRINT(gsDeviceInfo.wifi.ssid);

  File configFile = SPIFFS.open(HWVERSION_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open HWVN file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(HWVERSION_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open HWVN file for writing");
      return false;
    }
  }
  //DBGPRINT("before print to file");
  //configFile.print("IMEI:");
  configFile.println(gsDeviceInfo.HWVN);
  configFile.close();
  DBGPRINTLN("-- end --");
  return true;
}



bool ICACHE_FLASH_ATTR LVCommon::bEncode(char *achpData)
{
  char *spStart, *spEnd, *spSprintPos;
  char source[CONST_ENCODE_STRING_LEN + 1], encodeData[CONST_ENCODE_STRING_LEN * 2 + 1];
  int nLen, nLen2;
  int nSeed;
  int i = 0;
  //strcpy(gTime.timeString,"20161205214301");
  nLen = strlen(achpData);
  spSprintPos = achpData + nLen - 1;
  if (nLen > (CONST_BIG_BUFF_SIZE - 200))
    return false;
  // first char
  nLen2 = nGet_json_pos(achpData, "devID", &spStart);
  if (nLen2 > 0)
  {
    source[i] = *(spStart + nLen2 - 1);
    i++;
  }
  // second char
  nLen2 = strlen(gTime.timeString);
  if (nLen2 > 0)
  {
    spStart = gTime.timeString;
    source[i] = *(spStart + nLen2 - 1);
    i++;
  }
  // 3rd 4th char
  nLen2 = nGet_json_pos(achpData, "CMD", &spStart);
  if (nLen2 > 0)
  {
    source[i] = *(spStart + nLen2 - 2);
    i++;
    source[i] = *(spStart + nLen2 - 1);
    i++;
  }
  //nSeed=randomData();
  nSeed = random(10000, 60000);
  sprintf(&source[i], "%05d", nSeed);
  source[4] = 0;
  encodeWithSeed(source, encodeData, nSeed);
  //gTime.timeString
  sprintf(spSprintPos, ",\"SVER\":1,\"SCRTY\":1,\"COEXIST\":1,\"seed1\":\"%05d\",\"seed2\":\"%s\",\"DateTime\":\"%s\"}", nSeed, encodeData, gTime.timeString);
  //printf ("result:%s",achpData);

  return true;
}
;

void ICACHE_FLASH_ATTR LVCommon::vSystem_restart(short reason)
{
  nRestartReason = reason;
  bSave_config();
  delay(200);

  ESP.restart();
}


/*--------------------------------------------------------------------------------*/
//COMMON_FUCTION_PART_END
/*--------------------------------------------------------------------------------*/

/*====================================================================================*/
//COMMON_PART_END
/*====================================================================================*/




/*====================================================================================*/
//MAIN_PART_BEGIN Ver=20161126
/*====================================================================================*/

//The setup function is called once at startup of the sketch
void LVCommon::mainsetup()
{
  // Add your initialization code here
  version=CONST_LVCOMMON_VERSION;
  DBGPRINTLN();
  DBGPRINTLN("--DBG PRINT--");
  //gu8PowerOn = 1;
  gu8PowerOn = CONST_POWERON_FLAG_YES;

  //init a 100 ms timer interrupt
  //os_timer_setfn(&ghTimer100ms, vTimer_interrupt100ms, NULL);
  //os_timer_arm(&ghTimer100ms, 100, true);

  //LED PIN MODE
  DBGPRINTF("RED PIN:[%d]", CONST_LED_COLOR_RED_PIN);
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);

  // to generate the IMEI number based on chipid
  //vGenerate_IMEI();
  //sprintf(IMEI, "%s%010d", CONST_IMEI_PREFIX, ESP.getChipId());
  gnRestartMin = ESP.getChipId() % 56;

  DBGPRINTF("\nIMEI:[%s]\n", IMEI);
  //application call
  //vApplication_setup_call();// to init third party data

  DBGPRINTLN(gsDeviceInfo.SWVN);
  /*
    if (!SPIFFS.begin())
    {
    DBGPRINTLN("Failed to mount file system");
    return;
    }
  */
  gbConfigOK = bLoad_config();

  if (!bLoad_IMEI())
  {
    gbConfigOK = false;
    bSave_IMEI();
  }

  if (!bLoad_HWVersion())
  {
    bSave_HWVersion();
  }

  if (gbConfigOK)
  {
    gchMainState = WAIT_CONNECTION_STATE;
  }

  gbSmartConfigFlag = false;

#ifdef NEED_PUSH_FUNCTION
  //vReset_interval(glLastPushTick);
  glLastPushTick = ulReset_interval();
#endif
  //glMinPostIntervalTick = CONST_MIN_POST_INTERVAL_TICK;
  glConnIntervalTick = CONST_CONNINTERVAL_TICK;

  //to init random data
  randomSeed(micros());

}

unsigned short LVCommon::get_version()
{
  return version;
}


void LVCommon::func_INIT_STATE()
{
  WiFi.mode(CONST_WIFI_MODE);
  gnWiFiStat = WiFi.status();
  if (gnWiFiStat != WL_CONNECTED)
  {
    DBGPRINTLN("\r\nWait for Smartconfig");
    vChange_LED_status(CONST_LED_STATUS_SMARTCONFIG);
    //smartconfig_set_type(SC_TYPE_ESPTOUCH); //LV only support ESPTOUCH
    //smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);   // both ESPTouch and AirKiss
    WiFi.beginSmartConfig();
    gbSmartConfigFlag = true;
    //vReset_interval(glStateTransferTicks);
    glStateTransferTicks = ulReset_interval();
    gnStateTransferTicksCount = 0;
    gchMainState = SMARTCONFIG_START_STATE;
  }
  else
  {
    WiFi.disconnect();
  }
  delay(1);


}

void LVCommon::func_SMARTCONFIG_START_STATE()
{
  if (WiFi.smartConfigDone())
  {
    WiFi.stopSmartConfig();
    DBGPRINTLN("SmartConfig Success");
    strcpy(gsDeviceInfo.wifi.ssid , WiFi.SSID().c_str());
    strcpy(gsDeviceInfo.wifi.pass , WiFi.psk().c_str());
    WiFi.macAddress(gsDeviceInfo.macAddr);

    DBGPRINTF("\n\rSSID:%s", gsDeviceInfo.wifi.ssid);
    DBGPRINTF("\n\rPASS:%s", gsDeviceInfo.wifi.pass);
    DBGPRINTLN();

    WiFi.begin(gsDeviceInfo.wifi.ssid, gsDeviceInfo.wifi.pass);
    bConnectWiFi = true;

    //vReset_interval(glStateTransferTicks);
    glStateTransferTicks = ulReset_interval();

    gnStateTransferTicksCount = 0;
    gchMainState = SMARTCONFIG_DONE_STATE;
  }
  else
  {
    if (ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN)
    {
      //vReset_interval(glStateTransferTicks);
      glStateTransferTicks = ulReset_interval();
      gnStateTransferTicksCount++;
      DBGPRINT("s");
      //while ((ulGet_interval(glStateTransferTicks) % 1000) == 0);
    }
    else if (gbConfigOK && gnStateTransferTicksCount > CONST_SMARTCONFIG_SWITCH)
    { //Time out switch from smartconfig to try connection mode
      DBGPRINTLN();
      //vReset_interval(glStateTransferTicks);
      glStateTransferTicks = ulReset_interval();
      gnStateTransferTicksCount = 0;
      WiFi.stopSmartConfig();
      gchMainState = WAIT_CONNECTION_STATE;
    }
  }
  delay(1);

}

void LVCommon::func_SMARTCONFIG_DONE_STATE()
{
  //show_led_color(2, 2);
  vChange_LED_status(CONST_LED_STATUS_CONNECTING);
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
    ghpUdpLocal->setLocalPort(gsDeviceInfo.server.recvPort);
    ghpUdpLocal->connect(gsDeviceInfo.server.bindAddr, gsDeviceInfo.server.bindPort);
    vLVConfig_udp_send(ghpUdpLocal);
  }
  delay(1);

}

void LVCommon::func_LV_CONFIG_START_STATE()
{
  if (!ghpUdpLocal->available())
  {
    if ((ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN))
    {
      //vReset_interval(glStateTransferTicks);
      glStateTransferTicks = ulReset_interval();
      gnStateTransferTicksCount++;
      DBGPRINT("b");
      vLVConfig_udp_send(ghpUdpLocal);
      //while ((ulGet_interval(glStateTransferTicks) % 1000) == 0)
      ;    //Wait
    }

    if (gnStateTransferTicksCount >= CONST_UDPCONFIG_SWITCH)
    { //Not connected? Strange go to smartconfig again
      //vReset_interval(glStateTransferTicks);
      glStateTransferTicks = ulReset_interval();
      gnStateTransferTicksCount = 0;
      gchMainState = INIT_STATE;
      ghpUdpLocal->stop();
    }
  }
  else
  {
    short len;
    char *spDevID, *spAddr, *spPort, *spSeed, *spMac;
    short nDevIDLen, nAddrLen, nPortLen, nSeedLen, nMacLen;
    
    char *spLoRaAddr,*spLoRaPort,*spLoRaChannel;
    short nLoRaAddrLen,nLoRaPortLen,nLoRaChannelLen;
    
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
      *(spDevID + nDevIDLen) = '\0';
      *(spAddr + nAddrLen) = '\0';
      *(spPort + nPortLen) = '\0';
      *(spSeed + nSeedLen) = '\0';

      strcpy(gsDeviceInfo.server.addr, spAddr);
      gsDeviceInfo.server.port = atoi(spPort);
      strcpy(gachServerSeed, spSeed);
      //gachServerSeed = String(spSeed);

      DBGPRINTLN(gachServerSeed);

      DBGPRINT("Bound Server Addr:");
      DBGPRINTLN(gsDeviceInfo.server.addr);
      DBGPRINT("Bound Server Port:");
      DBGPRINTLN(gsDeviceInfo.server.port);

      ghpUdpLocal->stop();
      //vReset_interval(glStateTransferTicks);
      glStateTransferTicks = ulReset_interval();

      gnStateTransferTicksCount = 0;
      gchMainState = LV_CONFIG_DONE_STATE;
      if (nMacLen == CONST_MAC_ADDR_ASC_LEN) {
        short i;
        *(spMac + (nMacLen)) = '\0';
        for (i = 0; i < CONST_MAC_ADDR_LEN; i++) {
          gsDeviceInfo.userMac[i] = ascii_hex(*(spMac)) * 16 + ascii_hex(*(spMac + 1));
          spMac += 3;
        }
      }
      
      //LoRa Part
      nLoRaAddrLen = nGet_json_pos(gBigBuff.chpData, "LoRaIP", &spLoRaAddr);
      nLoRaPortLen = nGet_json_pos(gBigBuff.chpData, "LoRaPort", &spLoRaPort);
      nLoRaChannelLen = nGet_json_pos(gBigBuff.chpData, "LoRaChan", &spLoRaChannel);
      if ((nLoRaAddrLen > 0) && (nLoRaPortLen > 0) && (nLoRaChannelLen > 0)){
        *(spLoRaAddr + nLoRaAddrLen) = '\0';
        *(spLoRaPort + nLoRaPortLen) = '\0';
        *(spLoRaChannel + nLoRaChannelLen) = '\0';
        DBGPRINTF("\nLoRa addr:[%s]",spLoRaAddr);
        DBGPRINTF("\nLoRa port:[%s]",spLoRaPort);
        DBGPRINTF("\nLoRa chan:[%s]",spLoRaChannel);
        DBGPRINTLN("");
        /*
        strcpy(gsDeviceInfo.server.addr, spLoRaAddr);
        gsDeviceInfo.server.port = atoi(spLoRaPort);
        gsDeviceInfo.server.loraChannel = atoi(spLoRaPort);
        */
      }
    }
    else
    {
      DBGPRINTLN("Failed to parse config file");
      //Strange data error. Repeat send again
      //break;
      return;
    }
  }
  delay(1);

}

void LVCommon::func_LV_CONFIG_DONE_STATE()
{
  vChange_LED_status(CONST_LED_STATUS_CONNECTING);

  DBGPRINTF("SSID:%s\r\n", gsDeviceInfo.wifi.ssid);
  DBGPRINTF("PSW:%s\r\n", gsDeviceInfo.wifi.pass);
  DBGPRINTF("SVR:%s\r\n", gsDeviceInfo.server.addr);
  DBGPRINTF("SPT:%d\r\n", gsDeviceInfo.server.port);

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
  delay(100);
  //delay(10);
  if (!ghpTcpPush->connect(gsDeviceInfo.server.addr, gsDeviceInfo.server.port))
  {
    DBGPRINT("\nStart push failed");
    gnNetPushFailCount++;
    gnTcpPushBusy = 0;
  }
  else
  {
    DBGPRINT("\nStart push...");
    gnNetPushFailCount = 0;
    gnTcpPushBusy = 1;
    delay(100);
    //delay(10);
    bSend_Push();
    delay(100);
    //delay(10);
  }
#endif

  //Now config OK.
  //vReset_interval(glLastCommandTick);
  glLastCommandTick = ulReset_interval();

  gchMainState = CONNECTED_STATE;
  delay(1);


}

void LVCommon::func_WAIT_CONNECTION_STATE()
{
  //DBGPRINTLN("WAIT CONNECTION STATE:");
  vChange_LED_status(CONST_LED_STATUS_CONNECTING);
  gnWiFiStat = WiFi.status();

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
        gchMainState = INIT_STATE;
        nChange_WiFi_Mode(WL_DISCONNECTED);
        break;
      case WL_CONNECTION_LOST:
      case WL_IDLE_STATUS:
      case WL_DISCONNECTED:
        if (!bConnectWiFi)
        {
          WiFi.begin(gsDeviceInfo.wifi.ssid, gsDeviceInfo.wifi.pass);
          DBGPRINTLN("WAIT CONNECTION STATE:");
          DBGPRINTLN(gsDeviceInfo.wifi.ssid);
          DBGPRINTLN(gsDeviceInfo.wifi.pass);
          bConnectWiFi = true;
          //vReset_interval(glStateTransferTicks);
          glStateTransferTicks = ulReset_interval();

        }
        break;
      default:
        break;

    }

    if ((ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN))
    {
      //vReset_interval(glStateTransferTicks);
      glStateTransferTicks = ulReset_interval();
      gnStateTransferTicksCount++;
      DBGPRINT("c");
    }

    if ( gnStateTransferTicksCount > CONST_SMARTCONFIG_SWITCH)
    { //Need to switch to Smart Config
      DBGPRINTLN("=====SWITCH BEGIN====");
      DBGPRINT(gnStateTransferTicksCount);
      DBGPRINT("|");
      DBGPRINT(ulGet_interval(glStateTransferTicks));
      DBGPRINTLN("=====SWITCH END====");
      gnStateTransferTicksCount = 0;
      gchMainState = INIT_STATE;
      nChange_WiFi_Mode(WL_DISCONNECTED);
    }

  }
  else
  {
    //Connected
    vChange_LED_status(CONST_LED_STATUS_SERVER);
    bConnectWiFi = false;
    DBGPRINTLN("WiFi connected");
    DBGPRINT("IP address: ");
    DBGPRINTLN(WiFi.localIP());  //WiFi.localIP()

    if (!ghpTcpCommand->connect(gsDeviceInfo.server.addr, gsDeviceInfo.server.port))
    {
      DBGPRINTLN("connection failed");
      if (ulGet_interval(glStateTransferTicks) >= CONST_TRANSFER_TICK_LEN)
      { //Still failed : Server down? Need to do smartconfig to change to another server?
        gchMainState = INIT_STATE;
      }
      //break; //Continue to try next time as router is not connected to internet or server is down
      return;
    }

    gnTcpCommandBusy = 1;
    ulDoubleCheckTCPTick = ulReset_interval();

    //gnTcpCommandBusy =  CONST_TCP_COMMAND_BUSY_TIME_OUT;
    delay(100);
    //delay(10);
    if (gu8PowerOn == CONST_POWERON_FLAG_YES)
    {
      if (gbConfigOK)
        bSend_3006(false, gachServerSeed);
      else
        bSend_3006(true, gachServerSeed);
      gu8PowerOn = CONST_POWERON_FLAG_NO;
    }
    else {
      //没有利用3006清除过gnTcpCommandBusy状态
      //gnTcpCommandBusy = 0;
      bSend_3001();
    }

    DBGPRINTLN("bSave_config()");
    bSave_config();
    //vRefresh_data_3001();
    delay(100);
    //delay(10);

#ifdef NEED_PUSH_FUNCTION
    DBGPRINTLN("Start Push");
    if (!ghpTcpPush->connect(gsDeviceInfo.server.addr, gsDeviceInfo.server.port))
    {
      DBGPRINTLN("connection failed");
      gnNetPushFailCount++;
    }
    else
    {
      gnNetPushFailCount = 0;
    }
    gnTcpPushBusy = 1;
    delay(100);
    //delay(10);
    bSend_Push();
    delay(100);
    //delay(10);
#endif
    //vReset_interval(glLastCommandTick);
    glLastCommandTick = ulReset_interval();

    //init application wait loop call
    vApplication_wait_loop_call();

    gchMainState = CONNECTED_STATE;
    vChange_LED_status(CONST_LED_STATUS_ALWAYSOFF);
  }

  delay(1);

}

void LVCommon::func_CONNECTED_STATE()
{

  gnLenCommand = 0;
#ifdef NEED_PUSH_FUNCTION
  gnLenPush = 0;
  if (gnTcpPushBusy == 0 || !ghpTcpPush->connected()
      || ulGet_interval(glLastPushTick) > PUSH_TIMEOUT_INTERVAL)
  {
    //asm("break 0,0");  //For debug purpose, break point
    gnTcpPushBusy = 0;
    if (ghpTcpPush->connected())
    {
      DBGPRINTLN("Push timeout but still connected! Disconnect");
      ghpTcpPush->stop();
      delay(5);
    }
    else
    {
      if (ghpTcpPush->available() > 0)
      {
        int nRet;
        char *spTemp;
        gnLenPush = ghpTcpPush->read(gCommandBuff.u8pData, ghpTcpPush->available());

        gCommandBuff.chpData[gnLenPush] = 0;

        nRet = nhandle_server_response(gCommandBuff.u8pData, gnLenPush);
        gnLenPush = 0;
      }
      else {
        //DBGPRINTLN("---------------------------------------");
      }

      //DBGPRINTLN("Push disconnected");
    }
    //vReset_interval(glLastPushTick);
    glLastPushTick = ulReset_interval();

    if (!ghpTcpPush->reconnect())
    {
      //DBGPRINTLN("Push reconnection failed");
      gnNetPushFailCount++;
      //break;
      return;
    }
    else
    {
      gnNetPushFailCount = 0;
    }
    delay(10);
    DBGPRINTLN("Start Push: ");
    gnTcpPushBusy = 1;
    bSend_Push();
    delay(100);
    //delay(10);
    //break;
    return;
  }

  guLenHttp = ghpTcpPush->available();
  //tick = millis();
  if (guLenHttp > 0)
  {
    int nRet;
    //delay(50);
    delay(CONST_MIN_DELAY_TIME_FOR_PUSH_DATA);
    DBGPRINTF("Push Available: [%d]\r\n", guLenHttp);
    //DBGPRINTF("Push available len [%d]\n",guLenHttp);

    gnLenPush = ghpTcpPush->read(gCommandBuff.u8pData, ghpTcpPush->available());

    gCommandBuff.chpData[gnLenPush] = 0;

    nRet = nhandle_server_response(gCommandBuff.u8pData, gnLenPush);

    //if (nRet)
    {
      gnTcpPushBusy = 0;
      gnLenPush = 0;

    }
  }
#endif

  if (gnTcpCommandBusy)
  {
    //检测post回馈消息
    guLenHttp = ghpTcpCommand->available();
    //tick = millis();

    if (guLenHttp > 0)
    {
      int nRet;
      //delay(50);
      //等待，确保读到数据
      //delay(200);
      
      delay(CONST_MIN_DELAY_TIME_FOR_POST_DATA);
      
      DBGPRINTF("Command Available: [%d] freeheap[%d] \r\n", guLenHttp,ESP.getFreeHeap());
      //

      gnLenCommand = ghpTcpCommand->read(gCommandBuff.u8pData,  ghpTcpCommand->available());

      gCommandBuff.chpData[gnLenCommand] = 0;

      //DBGPRINTF("Command read data:(%s)\n",gCommandBuff.chpData);

      nRet = nhandle_server_response(gCommandBuff.u8pData, gnLenCommand);

      //if (nRet)
      {
        //收到POST回馈清除标记
        gnTcpCommandBusy = 0;
        gnLenCommand = 0;

        //vReset_interval(glLastCommandTick);
        glLastCommandTick = ulReset_interval();
        delay(10);
      }

    }

  }

  //DBGPRINTF("\n before ServerCommand.isEmpty[%d] [%d]",serverCommand.isEmpty(),gnServerCommand);
  //if ((!serverCommand.isEmpty()) && gnServerCommand == 0 && ulGet_interval(ulMinPOSTTicks) > CONST_MIN_POST_INTERVAL_TICK) {
  if ((!serverCommand.isEmpty()) && gnServerCommand == 0 ) {
    gnServerCommand = pop_cmd();
    if (gnServerCommand) {
      DBGPRINTF("\n gnServerCommand [%d],gnTcpCommandBusy[%d]", gnServerCommand, gnTcpCommandBusy);
      //if (gnServerCommand!=0)
        //ulMinPOSTTicks = ulReset_interval();
    }
  }

  //if ((gnServerCommand) && (gnTcpCommandBusy == 0)&& (ulGet_interval(glMinPostIntervalTick) > CONST_MIN_POST_INTERVAL_TICK))
  if ((gnServerCommand) && (gnTcpCommandBusy == 0))
    //if ((!serverCommand.isEmpty()) && (gnTcpCommandBusy == 0))
  {

    if (ghpTcpCommand->connected())
    {
      DBGPRINTLN("Send server command connected!");
      ghpTcpCommand->stop();
    }

    if (!ghpTcpCommand->reconnect())
    {
      DBGPRINTLN("Command connection failed");
      //break;
      return;
    }
    else
    {
      gnTcpCommandBusy = 1;
      ulDoubleCheckTCPTick = ulReset_interval();

      //gnTcpCommandBusy = CONST_TCP_COMMAND_BUSY_TIME_OUT;
      //switch (gnServerCommand)
      switch (gnServerCommand)
      {
        case CONST_CMD_3001:
          bSend_3001();
          //vReset_interval(glLastCommandTick);
          glLastCommandTick = ulReset_interval();
          gnServerCommand = 0;
          break;
        case CONST_CMD_9999:
          bSend_9999();
          gnServerCommand = 0;
          //nDoubleCheck9999FlagCount = 0;
          break;
        default:
          application_POST_call(gnServerCommand);
          gnServerCommand = 0;
          break;
      }
    }
  }


  //if ((gnServerCommand == 0)
  //&& (ulGet_interval(glLastCommandTick) > glConnIntervalTick))
  //&& (ulGet_interval(glMinPostIntervalTick) > CONST_MIN_POST_INTERVAL_TICK))
  if ((ulGet_interval(glLastCommandTick) > glConnIntervalTick))
  {
    //gnServerCommand = CONST_CMD_3001;
    push_cmd(CONST_CMD_3001);

    //gnTcpCommandBusy = 0;
    //vReset_interval(glLastCommandTick);
    glLastCommandTick = ulReset_interval();

  }

  //double check gnTcpCommandBusy status
  if ((nDoubleCheckTCPFlagCount) &&
      (ulGet_interval(ulDoubleCheckTCPTick) > CONST_CMD_TCP_DOUBLE_CHECK_TIME_LEN))
  {
    gnTcpCommandBusy = 0;
  }

  vApplication_connected_loop_call();

  /*
        if (gnSendDatatStaus == 0)
        {
          vApplication_connected_loop_call();
        }
        else
        {
          glConnIntervalTick = CONST_CONNINTERVAL_SHORT_TICK;
        }

  */
  delay(1);

}

void LVCommon::func_FACTORY_TEST_FINISH_LOOP()
{
  if (ulGet_interval(glActionID909Tick) > CONST_ACTIONID_909_TICK_LEN) {
    //ESP.restart();
    vSystem_restart(CONST_SYS_RESTART_REASON_FACTORY_TEST_TIME_OUT);
  }
  //vChange_LED_status(CONST_LED_STATUS_TEST_FINISH);
  //测试成功LED变化，开:3000,关：1000,开：500,关：500
  vLED_OFF();
  delay(1000);
  vLED_ON();
  delay(2000);
  vLED_OFF();
  delay(250);
  vLED_ON();
  delay(250);
  vLED_OFF();
  delay(250);
  vLED_ON();
  delay(250);
  vLED_OFF();
  delay(250);
  vLED_ON();
  delay(250);

}

void LVCommon::debug_print_info()
{
  //DBGPRINT("DATE:");
  //DBGPRINT(gPreTime.timeString);
  //DBGPRINT("-");
  //DBGPRINTLN(gTime.timeString);
  //DBGPRINTLN("----------------check last command tick begin --");
  //DBGPRINT("gnTcpCommandBusy:");
  //DBGPRINT(gnTcpCommandBusy);
  //DBGPRINTF("\n------ serverCommand [%d]", serverCommand.len());

  //DBGPRINT("|gnServerCommand:");
  //DBGPRINT(gnServerCommand);
  //DBGPRINT("|glLastCommandTick:");
  //DBGPRINT(ulGet_interval(glLastCommandTick));
  //DBGPRINT("|glConnIntervalTick:");
  //DBGPRINTLN(glConnIntervalTick);
  //DBGPRINT("|glStateTransferTicks:");
  //DBGPRINTLN(glStateTransferTicks);
  //DBGPRINT("|gnStateTransferTicksCount:");
  //DBGPRINTLN(gnStateTransferTicksCount);
  //DBGPRINT("|gu8LEDStatus:");
  //DBGPRINTLN(gu8LEDStatus);
  //DBGPRINTLN("--------------check last command tick end --");
  //debug_print_time();
  //debug_print_wifi_status();
}


void LVCommon::func_100ms_loop()
{
  // LED Control
  vDisplay_LED_color();
}

void LVCommon::func_second_loop()
{
  //force post check
  ulForcePOSTCheckTimeInMs = MAX(CONST_FORCE_POST_CHECK_TIME_TICK_LENGTH, glConnIntervalTick * CONST_FORCE_POST_CHECK_TIME_TICK_COUNT);

  if (ulGet_interval(ulForcePOSTCheckTick) > ulForcePOSTCheckTimeInMs) {
    DBGPRINTLN("\n ulForcePOSTCheckTick time out restart!!!");
    vSystem_restart(CONST_SYS_RESTART_REASON_NOPOST_TIME_OUT);
  }
  //TCP COMMAND BUSY TIME OUT
  //if (gnTcpCommandBusy > 0) {
  //gnTcpCommandBusy--;
  //}
  // current time and preTime
  vGet_time();
  //second change
  // determine network status
  if ((gnNetPostFailCount > 0) || (gnNetPushFailCount > 0))
  {
    vChange_LED_status(CONST_LED_STATUS_CONNECTING);
  }
  else
  {
    if (gchMainState == CONNECTED_STATE)
      vChange_LED_status(CONST_LED_STATUS_ALWAYSOFF);
  }

  if ((gnNetPostFailCount > CONST_MAX_NET_POSTFAIL) || (gnNetPushFailCount > CONST_MAX_NET_PUSHFAIL))
    //if ((gnNetPostFailCount > CONST_MAX_NET_POSTFAIL))
  {
    //network error change to WAIT_CONNECTION_STATE
    //gchMainState = WAIT_CONNECTION_STATE;
    //nChange_WiFi_Mode(WL_DISCONNECTED);
    //ESP.restart();
    vSystem_restart(CONST_SYS_RESTART_REASON_NETWORK_ERROR);
  }
  else if (gnNetPostFailCount > 0)
  {
    vRefresh_data_3001();
    delay(10);
  }

  debug_print_info();

}

void LVCommon::func_minute_loop()
{
  //reboot
  if ((gTime.hour == CONST_REBOOT_HOUR) && (gTime.minute == gnRestartMin))
  {
    delay(100);
    vSystem_restart(CONST_SYS_RESTART_REASON_REGULAR_REBOOT);

  }
  gnWiFiStat = WiFi.status();
  /*
    if (gnWiFiStat != WL_CONNECTED)
    {
    gnWiFiFailCount++;
    if (gnWiFiFailCount > CONST_MAX_WIFI_FAIL)
    {
      //ESP.restart();
      vSystem_restart(CONST_SYS_RESTART_REASON_WIFI_FAIL);
    }
    }
    else
    {
    gnWiFiFailCount == 0;
    }
  */
}

void LVCommon::func_hour_loop()
{
  //clean the fail count
  gnNetPostFailCount = 0;

}

void LVCommon::func_day_loop()
{
  gnTimeSyncUpdate = 1;

}


void LVCommon::mainloop()
{

  // a common 100 us loop function, for led, sync time, ...
  if (ulGet_interval(gl100msLoopTick) >= 100)
  {
    func_100ms_loop();
    // sync time
    if (ulGet_interval(ulTimeSecondTick) >= 1000)
    {
      func_second_loop();
      //minute change
      if (gTime.minute != gPreTime.minute)
      {
        func_minute_loop();
        // hour change
        if (gTime.hour != gPreTime.hour)
        {
          func_hour_loop();
          // day change
          if (gTime.day != gPreTime.day)
          {
            func_day_loop();
          }
        }
      }
      //vReset_interval(ulTimeSecondTick);
      ulTimeSecondTick = ulReset_interval();
    }
    //vReset_interval(gl100msLoopTick);
    gl100msLoopTick = ulReset_interval();

  }

  switch (gchMainState)
  {
    case INIT_STATE:
      func_INIT_STATE();
      break;

    case SMARTCONFIG_START_STATE:
      func_SMARTCONFIG_START_STATE();
      break;

    case SMARTCONFIG_DONE_STATE:
      func_SMARTCONFIG_DONE_STATE();
      break;

    case LV_CONFIG_START_STATE:
      func_LV_CONFIG_START_STATE();
      break;

    case LV_CONFIG_DONE_STATE:
      func_LV_CONFIG_DONE_STATE();
      break;

    case WAIT_CONNECTION_STATE:
      func_WAIT_CONNECTION_STATE();
      break;

    case CONNECTED_STATE:
      func_CONNECTED_STATE();
      break;

    case FACTORY_TEST_FINISH_LOOP:
      func_FACTORY_TEST_FINISH_LOOP();
      break;

    default:        //return to controlled state
      gchMainState = INIT_STATE;
      break;
  }
}

/*====================================================================================*/
//MAIN_PART_END
/*====================================================================================*/

