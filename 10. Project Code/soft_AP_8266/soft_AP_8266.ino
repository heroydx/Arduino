#include <ESP8266WiFi.h>
void setup() {
  Serial.begin ( 115200 );
  Serial.println("");
  // 设置内网
  IPAddress softLocal(192,168,128,1);
  IPAddress softGateway(192,168,128,1);
  IPAddress softSubnet(255,255,255,0);
  WiFi.softAPConfig(softLocal, softGateway, softSubnet);
  String apName = ("ESP8266_"+(String)ESP.getChipId());
  const char *softAPName = apName.c_str();
  WiFi.softAP(softAPName, "adminadmin");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.print("softAPName: ");
  Serial.println(apName);
}
 
void loop() {
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

