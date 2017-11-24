#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ESP8266httpUpdate.h>

#include "ESP8266Client.h"

ESP8266Client ghTcpCommand(TCP);


/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20161029
/*--------------------------------------------------------------------------------*/
#define DEBUG_SIO 1
#define DEBUG_HTTP 1

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


//common struct,unicon definition
typedef struct  {
  String url;
  String addr;
  String port;
  String pushUrl;
  String udpAddr;
  String udpPort;
} stServerAddr;

#ifdef DEBUG_HTTP
stServerAddr gsHttpLog = {
  "/uwsgitest",
  "123.59.151.230",
  "80"
};

ESP8266Client ghTcpHttpLog(TCP);

char gachDebugDataBuff[400] = "";

void vConnect_httpLogServer()
{
  while (1)
  {
    int rtn;
    rtn = ghTcpHttpLog.connect(gsHttpLog.addr.c_str(), gsHttpLog.port.toInt());
    DBGPRINT("TCPHTTPLOG");
    DBGPRINTLN(rtn);
    if (rtn == 0)
      break;
    delay(10);
    DBGPRINTLN("Can't connect to http log server, retry!");
  }
  delay(50);
  //String testMsg="http log server connected!";
  //DBGPRINTLN(testMsg);
  //nCommon_logPOST(gsHttpLog,testMsg);
}

int nCommon_logPOST( stServerAddr *spAddr, String sData )
{
  int ret;
  String postReq =
    String("POST ") + spAddr->url + " HTTP/1.1\r\n" + "Host: " + spAddr->addr
    + "\r\n"
    + "Connection: close\r\nContent-Type: application/x-www-form-urlencoded\r\n"
    + "Content-Length: " + sData.length() + "\r\n\r\n";
  postReq += sData;

  DBGPRINTLN("==postReq==");
  DBGPRINTLN(postReq);
  ret = ghTcpHttpLog.write((unsigned char *) postReq.c_str(), postReq.length());
  return ret;
}


int nHttp_print()
{
  int rtn;

  String buff;
  DBGPRINTLN("===HTTP TEST BEGIN===");
  DBGPRINTLN(gsHttpLog.url);
  DBGPRINTLN(gsHttpLog.addr);
  DBGPRINTLN(gachDebugDataBuff);
  DBGPRINTLN("===HTTP TEST END===");
  buff = String (gachDebugDataBuff);
  if (!ghTcpHttpLog.connected())
  {
    int loopCount = 10;
    while (!ghTcpHttpLog.reconnect() && loopCount > 0)
    {
      //gchMainState = WAIT_CONNECTION_STATE;
      //break;
      delay(10);
      loopCount--;
      DBGPRINTLN("Can't connect http log server, retry!");
    }
  }
  rtn = nCommon_logPOST(&gsHttpLog, buff);
  return rtn;
}

#endif

//common struct,unicon definition

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

#define CONST_DEVICE_VERSION "121"

stDeviceCoreDataStruct gsDeviceInfo = {
  CONST_DEVICE_VERSION,
  "LVSW_EN_V1.0.0",
  "LVHW_EN_V1.0.0",
  "9",
  "905",
  "meTe",
  {
    "/dbAEA0",
    "ss1.chakonger.net.cn",
    "80",
    "/ps",
    "255.255.255.255",
    "10000"

  },
  {
    "jylx",
    "zgkxydx123456"
  },
};



int nCommonPOST( String sUrl, String sAddr, String sData )
{
  int ret;
  String postReq =
    String("POST ") + sUrl + " HTTP/1.1\r\n" + "Host: " + sAddr
    + "\r\n"
    + "Connection: close\r\nContent-Type: application/x-www-form-urlencoded\r\n"
    + "Content-Length: " + sData.length() + "\r\n\r\n";
  postReq += sData;

  DBGPRINTLN("==postReq==");
  DBGPRINTLN(postReq);
  ret = ghTcpCommand.write((unsigned char *) postReq.c_str(), postReq.length());
  return ret;
}
unsigned long getInterval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}

unsigned long glStateTransferTicks;

String gsData = "{\"CMD\":\"AEA0\",\"devID\":\"800012345678\",\"actionID\":\"2\",\"inst\":\"0\", \"param\":\"123\", \"exeTime\":\"\",  \"seed1\":\"\",\"seed2\":\"\"}";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  WiFi.begin(gsDeviceInfo.wifi.ssid.c_str(), gsDeviceInfo.wifi.pass.c_str());
  DBGPRINTLN(gsDeviceInfo.wifi.ssid.c_str());
  DBGPRINTLN(gsDeviceInfo.wifi.pass.c_str());

}

int gchMainState = 0;
unsigned int guLenHttp;

#define COMMAND_DATA_SIZE 2048
size_t len_command;
uint8_t data_command[COMMAND_DATA_SIZE];

bool gbSendCommand = false;

void loop()
{
#ifdef DEBUG_HTTP
  vConnect_httpLogServer();
#endif

#ifdef DEBUG_HTTP
  sprintf(gachDebugDataBuff, "DEBUG:TEST HTTP LOG");
  nHttp_print();
#endif

#ifdef DEBUG_HTTP
  sprintf(gachDebugDataBuff,"DEBUG:%s", gsDeviceInfo.wifi.pass.c_str());
  nHttp_print();
#endif
  switch (gchMainState)
  {
    case 0:
      // put your main code here, to run repeatedly:
      if (WiFi.status() != WL_CONNECTED)
      {
        if ((getInterval(glStateTransferTicks) % 1000) == 0)
        {
          DBGPRINT("c");
          while ((getInterval(glStateTransferTicks) % 1000) == 0)
            ;
        }
      }
      else
      {
        while (!ghTcpCommand.connect(gsDeviceInfo.server.addr.c_str(), gsDeviceInfo.server.port.toInt()))
        {
          //gchMainState = WAIT_CONNECTION_STATE;
          //break;
          delay(10);
          DBGPRINTLN("Can't connect, retry!");
        }
        DBGPRINTLN("TCP Connected");
        gchMainState = 1;
      }
      break;
    case 1:

      if (!ghTcpCommand.connected()) 
      {
        //while (!ghTcpCommand.connect(gsDeviceInfo.server.addr.c_str(), gsDeviceInfo.server.port.toInt()))
        while (!ghTcpCommand.reconnect())
        {
          //gchMainState = WAIT_CONNECTION_STATE;
          //break;
          delay(10);
          DBGPRINTLN("Can't connect, retry!");
        }
      }
      nCommonPOST(gsDeviceInfo.server.url, gsDeviceInfo.server.addr, gsData);
      delay(200);

      guLenHttp = ghTcpCommand.available();
      if (guLenHttp > 0)
      {
        delay(50);
        DBGPRINTF("Command Available: [%d]\r\n", guLenHttp);

        len_command = ghTcpCommand.read(data_command,  ghTcpCommand.available());

        data_command[len_command] = 0;

        DBGPRINTF("Command read data:(%s)\n", data_command);
        char buff[40];
        char *testStr=strstr((char *) data_command,"Date");
        memcpy(buff,testStr,35);
        buff[35]='\0';
        DBGPRINTLN(buff);

      }
      delay(10 * 1000);
      
      break;


  }

}
