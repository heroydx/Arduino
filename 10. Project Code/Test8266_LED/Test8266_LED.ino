/*在同一wifi环境下，通过web对esp8266开发板的GPIO 0对应的led灯进行控制 */
/*代码参考：http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-3-control-an-led-from-a-web-page-using-station-mode-st/ */

#include <ESP8266WiFi.h>

// change these values to match your network
char ssid[] = "**********";       //  your network SSID (name)
char pass[] = "**********";    // your network password

WiFiServer server(80);

String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/><meta charset='utf-8'><style>body {font-size:140%;} #main {display: table; margin: auto;  padding: 0 10px 0 10px; } h2,{text-align:center; } .button { padding:10px 10px 10px 10px; width:100%;  background-color: #4CAF50; font-size: 120%;}</style><title>LED Control</title></head><body><div id='main'><h2>LED Control</h2>";
String html_2 = "";
String html_4 = "</div></body></html>";

String request = "";
int LED_Pin = D0;

void setup()
{
  pinMode(LED_Pin, OUTPUT);

  Serial.begin(115200);
  delay(500);
  Serial.println(F("Serial started at 9600"));
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print(F("Connecting to "));  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");    delay(500);
  }
  Serial.println("");
  Serial.println(F("[CONNECTED]"));
  Serial.print("[IP ");
  Serial.print(WiFi.localIP());
  Serial.println("]");

  // start a server
  server.begin();
  Serial.println("Server started");

} // void setup()



void loop()
{

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)  {
    return;
  }

  // Read the first line of the request
  request = client.readStringUntil('\r');

  if       ( request.indexOf("LEDON") > 0 )  {
    digitalWrite(LED_Pin, HIGH);
  }
  else if  ( request.indexOf("LEDOFF") > 0 ) {
    digitalWrite(LED_Pin, LOW);
  }


  // Get the LED pin status and create the LED status message
  if (digitalRead(LED_Pin) == HIGH)
  {
    // the LED is off so the button needs to say turn it off
    html_2 = "<form id='F1' action='LEDOFF'><input class='button' type='submit' value='Turn on the LED' ></form><br>";
  }
  else
  {
    // the LED is on so the button needs to say turn it on
    html_2 = "<form id='F1' action='LEDON'><input class='button' type='submit' value='Turn off the LED' ></form><br>";
  }


  client.flush();

  client.print( header );
  client.print( html_1 );
  client.print( html_2 );
  client.print( html_4);

  delay(5);
  // The client will actually be disconnected when the function returns and 'client' object is detroyed

} // void loop()
