#include <miscLED.h>



miscLED LEDControl;
unsigned long glSecondLoopTick;
int count = 0;

short statusCount = 0;
uint8_t gu8LED_status[] = {
  //CONST_LED_STATUS_CONNECTING,
  //CONST_LED_STATUS_SMARTCONFIG,
  //CONST_LED_STATUS_SERVER,
  CONST_LED_STATUS_FAST_FLASH,
  CONST_LED_STATUS_MID_FLASH,
  CONST_LED_STATUS_SLOW_FLASH
};

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(78440);
  LEDControl.begin();
  Serial.println("----Setup----");
  LEDControl.change_LED_status(CONST_LED_STATUS_VERY_SLOW_FLASH);
  Serial.printf("\nLED Status:[%d]", LEDControl.get_LED_status());
}


void loop()
{
  LEDControl.LED_loop();
  // put your main code here, to run repeatedly:
  // a common 100 us loop function, for led, sync time, ...
  if (ulGet_interval(glSecondLoopTick) >= 1000)
  {
    glSecondLoopTick = ulReset_interval();
    count++;
    if (count % 10 == 0)
    {
      Serial.println();
      Serial.print("stat:");
      Serial.println(statusCount);
      LEDControl.change_LED_status(gu8LED_status[statusCount]);
      Serial.printf("\nLED Status:[%d]", LEDControl.get_LED_status());
      statusCount++;
      if (statusCount > sizeof(gu8LED_status)) {
        statusCount = 0;
      }
    }
    else if (count % 10 == 0)
    {
      Serial.print(".");
    }
  }
}
