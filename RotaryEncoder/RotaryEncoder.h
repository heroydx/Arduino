/*
  RotaryEncoder.h - Library for EC11 RotaryEncoder detection.
  Created by steven.lian@gmail.com
*/
#ifndef _ROTARY_ENCODER_H
#define _ROTARY_ENCODER_H

#include "Arduino.h"

// EC11 类型
#define CONST_APP_ROTARY_ENCODER_EC11_ONE_PULSE 0
#define CONST_APP_ROTARY_ENCODER_EC11_TWP_PULSE 1

// 编码器扫描结果参数定义
#define CONST_APP_ROTARY_ENCODER_EC11_RTN_NULL 0
#define CONST_APP_ROTARY_ENCODER_EC11_RTN_ADD 1
#define CONST_APP_ROTARY_ENCODER_EC11_RTN_MIN -1

//检测按键的周期,默认是5毫秒。
#define CONST_APP_ROTARY_ENCODER_DETECTION_TIME 2

//默认检测按键是高电平是有效
#define CONST_APP_ROTARY_ENCODER_UP_STATUS HIGH
#define CONST_APP_ROTARY_ENCODER_DOWN_STATUS LOW

class RotaryEncoder
{
  public:
    //func
    void begin(short pinA,short pinB);
    void begin(short pinA,short pinB, short ecType);
    int read();
    int rotaryCount;
  private:
    //data
    short currEC11AStat; //current stat, true=press
    short preEC11AStat;
    short currEC11BStat; //current stat, true=press
    short preEC11BStat;
    short _pinA;
    short _pinB;
    short _ecType;
    unsigned long detectionTick;
    //func
    int read_one_pulse();
    int read_two_pulse();
    unsigned long ulReset_interval();
    unsigned long ulGet_interval(unsigned long checkTick);
};

#endif
