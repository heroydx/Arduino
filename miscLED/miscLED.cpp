#include "miscLED.h"


miscLED::miscLED()
{
  begin();
} 

void ICACHE_FLASH_ATTR miscLED::begin()
{
  //LED PIN MODE
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);
  ul100msLoopTick = 0;

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

  //CONST_LED_STATUS_FAST_FLASH
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].blinkLoop[0] = 2;
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].blinkLoop[1] = 4;
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].blinkLoop[2] = 6;
  asLedDisplay[CONST_LED_STATUS_FAST_FLASH].blinkLoop[3] = 8;

  //CONST_LED_STATUS_MID_FLASH
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].blinkLoop[0] = 3;
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].blinkLoop[1] = 6;
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].blinkLoop[2] = 9;
  asLedDisplay[CONST_LED_STATUS_MID_FLASH].blinkLoop[3] = 12;

  //CONST_LED_STATUS_SLOW_FLASH
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].blinkLoop[0] = 6;
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].blinkLoop[1] = 12;
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].blinkLoop[2] = 18;
  asLedDisplay[CONST_LED_STATUS_SLOW_FLASH].blinkLoop[3] = 24;

    //CONST_LED_STATUS_VERY_SLOW_FLASH
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].loopFlag = CONST_LED_LOOP_ALWAYS;
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].onColor = CONST_LED_COLOR_RED_MASK;
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].blinkCount = 4;
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].blinkLoop[0] = 10;
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].blinkLoop[1] = 20;
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].blinkLoop[2] = 30;
  asLedDisplay[CONST_LED_STATUS_VERY_SLOW_FLASH].blinkLoop[3] = 40;

}

void ICACHE_FLASH_ATTR miscLED::LED_loop(void)
{
  if (ulGet_interval(ul100msLoopTick) >= 100)
  {
    //DBGPRINTLN("display_LED_color");
    display_LED_color();
    ul100msLoopTick = ulReset_interval();
  }
}

// to setup or change the LED display status, the status include:CONST_LED_STATUS_ALWAYSON,CONST_LED_STATUS_SMARTCONFIG,etc.
void ICACHE_FLASH_ATTR miscLED::change_LED_status(uint8_t u8Status)
{
  if (gu8LEDStatus != u8Status)
  {
    gu8LEDStatus = u8Status;
    gu8LEDLoopCount = 0;
    gu8LEDDispCount = 0;
  }
}

uint8_t ICACHE_FLASH_ATTR miscLED::get_LED_status()
{
  return gu8LEDStatus;
}


void ICACHE_FLASH_ATTR miscLED::LED_ON(uint8_t onColor)
{
  if (onColor & CONST_LED_COLOR_RED_MASK)
    digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
  if (onColor & CONST_LED_COLOR_GREEN_MASK)
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
  if (onColor & CONST_LED_COLOR_BLUE_MASK)
    digitalWrite(CONST_LED_COLOR_BLUE_PIN, HIGH);
}

void ICACHE_FLASH_ATTR miscLED::LED_ON()
{
  if (CONST_LED_COLOR_RED_MASK)
    digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);

}

void ICACHE_FLASH_ATTR miscLED::LED_OFF()
{
  digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
}

void ICACHE_FLASH_ATTR miscLED::LED_Blink()
{
  LED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
  delay(CONST_LED_COLOR_BLINK_DELAY);
  LED_OFF();
}

void ICACHE_FLASH_ATTR miscLED::LED_poweron()
{
  for(int i=0;i<CONST_LED_POWERON_LOOP_COUNT;i++)
  {
    LED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
    delay(CONST_LED_COLOR_50MS_DELAY);
    LED_OFF();
    delay(CONST_LED_COLOR_BLINK_DELAY);
  }
}


void ICACHE_FLASH_ATTR miscLED::display_LED_color(void)
{
  //DBGPRINTF("\ndisplay_LED_color:[%d]",gu8LEDStatus);
  if (gu8LEDStatus == CONST_LED_STATUS_ALWAYSOFF)
  {
    //DBGPRINTLN("DISPLAY ALWAYS OFF");
    LED_OFF();
    //DBGPRINT("_");

  }
  else if  (gu8LEDStatus == CONST_LED_STATUS_ALWAYSON)
  {
    //DBGPRINTLN("DISPLAY ALWAYS ON");
    LED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
    //DBGPRINT("T");
  }
  else
  {
    if (gu8LEDLoopCount < asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
      //if (gu8LEDLoopCount == asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
    {
      if ((gu8LEDDispCount % 2) == 0)
      {
        LED_ON(asLedDisplay[gu8LEDStatus].onColor);
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
          //DBGPRINTF("!!!!!1(asLedDisplay[gu8LEDStatus].loopFlag [%d] != CONST_LED_LOOP_ALWAYS)[%d]",asLedDisplay[gu8LEDStatus].loopFlag,CONST_LED_LOOP_ALWAYS);
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
        //DBGPRINTF("!!!!!2(asLedDisplay[gu8LEDStatus].loopFlag != CONST_LED_LOOP_ALWAYS)[%d]",asLedDisplay[gu8LEDStatus].loopFlag,CONST_LED_LOOP_ALWAYS);
      }

    }
  }
  delay(1);
}

