/*
  Button.cpp - Library for button detection.
  Created by steven.lian@gmail.com
*/

#include "Button.h"

void Button::begin(short pin)
{
  begin(pin, CONST_APP_BUTTON_DETECTION_TIME);
}

void Button::begin(short pin, short detectionMS)
{
  pinMode(pin, INPUT);
  _pin = pin;
  currStat=CONST_APP_BUTTON_DOWN_STATUS;
  preStat=CONST_APP_BUTTON_DOWN_STATUS;
  switchStatus = 0;
  pushCount = 0;
}

int Button::read()
{
  int ret;
  if (ulGet_interval(detectionTick) > CONST_APP_BUTTON_DETECTION_TIME) {
    currStat = digitalRead(_pin);
    //Serial.printf("\n pin:[%d]",currStat);
    if (currStat == CONST_APP_BUTTON_ACTION_STATUS) {
      //按键按下,判断次数避免误判
      pushCount++;
      //Serial.printf("\n push:[%d]",pushCount);
      if (pushCount >= CONST_APP_BUTTON_DETECTION_COUNT) {
        //按键按下时间满足要求
        //Serial.printf("\n currStat:[%d] preStat[%d]", currStat,preStat);
        if ((currStat != preStat)) {
          preStat = currStat;
          //Serial.printf("\n switchStatus:[%d]", pushCount);
          switchStatus = pushCount;
        }
      }
    }
    else {
      //恢复preStat的初始状态，否则以后不会检测了
      preStat=CONST_APP_BUTTON_DOWN_STATUS;
      pushCount = 0;
      switchStatus = 0;
    }
    detectionTick = ulReset_interval();
  }
  ret = switchStatus;
  switchStatus = 0;
  return ret;
}

unsigned long Button::ulReset_interval()
{
  return millis();
}

unsigned long Button::ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;
}