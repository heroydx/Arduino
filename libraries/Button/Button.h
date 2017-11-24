/*
  Button.h - Library for button detection.
  Created by steven.lian@gmail.com
*/
#ifndef _BUTTON_H
#define _BUTTON_H

#include "Arduino.h"

//检测按键的周期,默认是10毫秒。
#define CONST_APP_BUTTON_DETECTION_TIME 10
#define CONST_APP_BUTTON_DETECTION_COUNT 3

//默认检测按键是高电平是有效
#define CONST_APP_BUTTON_ACTION_STATUS LOW
#define CONST_APP_BUTTON_DOWN_STATUS HIGH

class Button
{
  public:
    //func
    void begin(short pin);
    void begin(short pin, short detectionMS);
    int read();
  private:
    //data
    int switchStatus;
    short int currStat; //current stat, true=press
    short int preStat;
    int pushCount;
    short _pin;
    unsigned long detectionTick;
    //func
    unsigned long ulReset_interval();
    unsigned long ulGet_interval(unsigned long checkTick);
};

#endif
