/*在NodeMCU搭建了TCP server端，PC为client端与server进行简单通信*/
/*参考代码：https://zhuanlan.zhihu.com/p/27668432 */
/* 连接路由器并设置静态IP */
/* 参考代码：https://github.com/esp8266/Arduino/issues/1959 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Hardcode WiFi parameters as this isn't going to be moving around.
const char* ssid = "BUPT_J2_1776"; //填入自己的WiFi名
const char* password = "bupt12345678"; //WiFi密码

// Start a TCP Server on port 5045
WiFiServer server(5045); //端口5045，自定义（避免公用端口）
WiFiClient client;

char data[1500];
int ind = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin ( 115200 );
  WiFi.begin(ssid, password);
  Serial.println("");
  //Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    //检查WiFi连接状态
    delay(500);
    Serial.print(".");
  }

  // config static IP
  IPAddress ip(192, 168, 1, 200); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 0, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);

  Serial.print("Connected to "); Serial.println(ssid);
  Serial.print("IP Address: "); Serial.println(WiFi.localIP()); //串口监视器显示IP地址

  // Start the TCP server
  server.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected())
  {
    //try to connect to a new client
    Serial.print(WiFi.localIP()); Serial.println(" try again....");
    client = server.available();
  }
  else
  {
    if (client.available() > 0)
    {
      //Serial.println("Connected to client");
      Serial.print(WiFi.localIP()); Serial.println(" listenning....");

      while (client.available())
      {
        data[ind] = client.read(); //读取client端发送的字符
        ind++;
      }
      client.flush();

      for (int j = 0; j < ind; j++)
      {
        Serial.print(data[j]);
      }

      Serial.print("\n");
      ind = 0;
      client.print("OK! Got your request."); //在client端回复
    }

  }

}
