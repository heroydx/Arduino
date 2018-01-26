/*UDP收发实验，固定IP地址，固定端口 */
/*参考代码：https://github.com/esp8266/Arduino/blob/master/doc/esp8266wifi/udp-examples.rst */
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

extern "C" {
#include "miscCommon.h"
}

const char* ssid = "BUPT_J2_1776";
const char* password = "bupt12345678";
const int MAX = 1535;
int currTimeSlot = 0;

const char* host = "193.168.0.198";//server ip
unsigned int remoteUdpPort_outcome = 20001;  // local port to listen on
//unsigned int remoteUdpPort_outcome = 20002;

WiFiUDP UdpPulse;
WiFiUDP UdpListen;
WiFiUDP UdpSend;

unsigned int pulseUdpPort = 20000;  // local port to listen on
unsigned int listenUdpPort = 20001;  // local port to listen on
unsigned int sendUdpPort = 20002;  // local port to listen on

unsigned long pulseTimeTick = 0;

char incomingPacket[255];  // buffer for incoming packets
char  replyPacekt[128];  // a reply string to send back


void setup()
{
  // config static IP
  IPAddress ip(193, 168, 0, 201); // the desired IP Address
  IPAddress gateway(193, 168, 0, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);

  //Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.localIP()); //串口监视器显示IP地址

  Serial.begin(115200);
  Serial.println();

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  UdpPulse.begin(pulseUdpPort);
  UdpListen.begin(listenUdpPort);
  UdpSend.begin(sendUdpPort);

  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), listenUdpPort);
}

void loop()
{
  if (ulGet_interval(pulseTimeTick) > 1000) {
    // send back a reply, to the IP address and port we got the packet from
    sprintf(replyPacekt, "{\"CMD\":\"LINK16REQ\",\"timeSlot\":%d}", currTimeSlot);
    currTimeSlot++;
    Serial.printf("\n Sending..... [%s]",replyPacekt);
    if (currTimeSlot > MAX) {
      currTimeSlot = 0;
    }
    UdpPulse.beginPacket(host, pulseUdpPort);
    UdpPulse.write(replyPacekt);
    UdpPulse.endPacket();
    pulseTimeTick = ulReset_interval();
  }


  int packetSize = UdpListen.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("\n Received %d bytes from %s, port %d\n", packetSize, UdpListen.remoteIP().toString().c_str(), UdpListen.remotePort());
    int nLen = UdpListen.read(incomingPacket, 255);
    if (nLen > 0)
    {
      incomingPacket[nLen] = 0;
      UdpSend.beginPacket(host, sendUdpPort);
      UdpSend.write(incomingPacket);
      UdpSend.endPacket();
      Serial.printf("\n data:[%d]",incomingPacket);
    }

    Serial.printf("UDP packet contents: %s\n", incomingPacket);
  }

}
