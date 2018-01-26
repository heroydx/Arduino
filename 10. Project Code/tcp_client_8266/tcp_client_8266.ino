/*在NodeMCU搭建了TCP client端，进行简单通信*/
/*参考代码：https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiClient/WiFiClient.ino */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Hardcode WiFi parameters as this isn't going to be moving around.
const char* ssid = "BUPT_J2_1776"; //填入自己的WiFi名
const char* password = "bupt12345678"; //WiFi密码

const char* host = "192.168.2.120";//server ip

// Start a TCP Server on port 5045
WiFiServer server(5045); //端口5045，自定义（避免公用端口）
WiFiClient client;

char data[1500];
int ind = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  Serial.println("");
  //Wait for connection
  while(WiFi.status() != WL_CONNECTED) { //检查WiFi连接状态
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.localIP()); //串口监视器显示IP地址

  // Start the TCP server
  server.begin();
}

//int value = 0;

void loop() {
  delay(5000);
  //++value;

  //server = 192.168.2.119; //以后改为自动获取对方ip
  Serial.print(WiFi.localIP());Serial.println("connecting to host.....");
  //Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  //const int httpPort = 80;
  const int Port = 5045;
//  if (!client.connect(host, httpPort)) {
//    Serial.println("connection failed");
//    return;
//  }
  if (!client.connect(host, Port)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "hello";
  //url += streamId;
  //url += "?private_key=";
  //url += privateKey;
  //url += "&value=";
  //url += value;
  
  Serial.print("Requesting: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}
