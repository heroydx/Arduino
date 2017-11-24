// Very basic Spiffs example, writing 10 strings to SPIFFS filesystem, and then read them back
// For SPIFFS doc see : https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md
// Compiled in Arduino 1.6.7. Runs OK on Wemos D1 ESP8266 board.

#include "FS.h"

#define CONST_LED_COLOR_RED_PIN 13

void LED_BLINK(int onDelay, int offDelay, int count)
{
  int i;
  for (i = 0; i < count; i++)
  {
    digitalWrite(CONST_LED_COLOR_RED_PIN, 1);
    delay(onDelay);
    digitalWrite(CONST_LED_COLOR_RED_PIN, 0);
    delay(offDelay);
  }
}

void setup()
{
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  Serial.begin(74880);
  LED_BLINK(1000, 1000, 2);
  Serial.println();
  Serial.println();
  Serial.println("\nVery basic Spiffs example, writing 10 lines to SPIFFS filesystem, and then read them back");
  SPIFFS.begin();
  // Next lines have to be done ONLY ONCE!!!!!When SPIFFS is formatted ONCE you can comment these lines out!!
  Serial.println("Please wait 30 secs for SPIFFS to be formatted");
  //LED_BLINK(2000, 2000, 2);
  SPIFFS.format();
  Serial.println("Spiffs formatted");
  LED_BLINK(500, 500, 5);
}

void loop()
{
  Serial.println("test FS");
  LED_BLINK(1000, 1000, 1);
}

char fileName[] = "/config.json";
void realloop() {

  // open file for writing

  char buff[100];
  File f = SPIFFS.open(fileName, "w");
  if (!f)
  {
    Serial.println("file open failed");
  }
  Serial.println("====== Writing to SPIFFS file =========");
  // write 10 strings to file
  for (int i = 1; i <= 10; i++)
  {
    sprintf(buff, "Millis():%d", millis());
    //f.print("Millis():");
    //f.println(millis());
    //Serial.println(millis());
    f.println(buff);
    Serial.println(buff);
  }

  f.close();

  // open file for reading
  f = SPIFFS.open(fileName, "r");
  if (!f)
  {
    Serial.println("file open failed");
  }
  Serial.println("====== Reading from SPIFFS file =======");
  // write 10 strings to file
  for (int i = 1; i <= 10; i++)
  {
    int nPos;
    String s = f.readStringUntil('\n');
    //Serial.println(s);
    nPos = s.indexOf(':');
    Serial.print(nPos);
    Serial.print("[");
    Serial.print(s.substring(0, nPos));
    Serial.print("][");
    Serial.print(s.substring(nPos + 1));
    Serial.print("]");

    Serial.print(i);
    Serial.print(":");
    Serial.println(s);
  }

  // wait a few seconds before doing it all over again

  Serial.print(":");
  delay(1000);

}
