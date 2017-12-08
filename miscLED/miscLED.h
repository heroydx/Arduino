#pragma once
#ifndef _MISCLED_H
#define _MISCLED_H

#include <Arduino.h>


extern "C" {
#include "miscCommon.h"
}


//LED global data
#define CONST_LED_CHANGE_COUNT 8

#define CONST_LED_COLOR_RED_PIN 13
#define CONST_LED_COLOR_GREEN_PIN 13
#define CONST_LED_COLOR_BLUE_PIN 13

#define CONST_LED_COLOR_BLINK_DELAY 100
#define CONST_LED_COLOR_50MS_DELAY 50
#define CONST_LED_POWERON_LOOP_COUNT 10

#define CONST_LED_COLOR_RED_MASK 4
#define CONST_LED_COLOR_GREEN_MASK 2
#define CONST_LED_COLOR_BLUE_MASK 1

#define CONST_LED_LOOP_ALWAYS 255

#define CONST_LED_STATUS_ALWAYSOFF 254
#define CONST_LED_STATUS_ALWAYSON 255

#define CONST_LED_STRUCT_LEN 10 // LED status count 

#define CONST_LED_STATUS_SELFDEF 0
#define CONST_LED_STATUS_SMARTCONFIG 1
#define CONST_LED_STATUS_CONNECTING 2
#define CONST_LED_STATUS_SERVER 3
#define CONST_LED_STATUS_ONE 4
#define CONST_LED_STATUS_TEST_FINISH 5
#define CONST_LED_STATUS_FAST_FLASH 6
#define CONST_LED_STATUS_MID_FLASH 7
#define CONST_LED_STATUS_SLOW_FLASH 8
#define CONST_LED_STATUS_VERY_SLOW_FLASH 9



typedef struct {
  uint8_t loopFlag;
  uint8_t onColor;
  uint8_t blinkCount;
  uint8_t blinkLoop[CONST_LED_CHANGE_COUNT];
} stLedDispStruct;

class miscLED
{
  public:
    //LED control
    miscLED();
    void begin();
    void LED_loop();
    void change_LED_status(uint8_t u8Status);
    void LED_ON(uint8_t onColor);
    void LED_ON();
    void LED_OFF();
    void LED_Blink();
    void LED_poweron();
    void display_LED_color(void);
    uint8_t get_LED_status();

  private:
    unsigned long ul100msLoopTick = 0;
    uint8_t gu8LEDStatus = 0; // = 0;
    //uint8_t gu8LEDPreStatus = 0;
    uint8_t gu8LEDLoopCount = 0; // = 0;
    uint8_t gu8LEDDispCount = 0; // = 0;

    stLedDispStruct asLedDisplay[CONST_LED_STRUCT_LEN];
};

#endif
