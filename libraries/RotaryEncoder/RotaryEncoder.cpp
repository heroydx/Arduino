/*
  RotaryEncoder.cpp - Library for RotaryEncoder detection.
  Created by steven.lian@gmail.com
*/

#include "RotaryEncoder.h"

void RotaryEncoder::begin(short pinA,short pinB)
{
  begin(pinA,pinB, CONST_APP_ROTARY_ENCODER_EC11_ONE_PULSE);
}

void RotaryEncoder::begin(short pinA,short pinB, short ecType)
{
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  _pinA = pinA;
  _pinB = pinB;
  currEC11AStat=CONST_APP_ROTARY_ENCODER_DOWN_STATUS;
  preEC11AStat=CONST_APP_ROTARY_ENCODER_DOWN_STATUS;
  currEC11BStat=CONST_APP_ROTARY_ENCODER_DOWN_STATUS;
  preEC11BStat=CONST_APP_ROTARY_ENCODER_DOWN_STATUS;
  rotaryCount = 0;
  _ecType=ecType;
}

int RotaryEncoder::read()
{
  int ret;
  if (_ecType==CONST_APP_ROTARY_ENCODER_EC11_ONE_PULSE)
  {
    ret=read_one_pulse();
    rotaryCount+=ret;    
  }
  else
  {
    ret=read_two_pulse();
    rotaryCount+=ret;    
  }
  return ret;
}


int RotaryEncoder::read_one_pulse()
{
  int ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_NULL;
  if (ulGet_interval(detectionTick) > CONST_APP_ROTARY_ENCODER_DETECTION_TIME) {
    currEC11AStat = digitalRead(_pinA);
    currEC11BStat = digitalRead(_pinB);
    //Serial.printf("\n A[%d],B[%d]",currEC11AStat,currEC11BStat);
    
    // 当A发生跳变时采集B当前的状态，并将B与上一次的状态进行对比
    //以A为时钟，B为数据。正转时AB反相，反转时AB同相   
    
    if (currEC11AStat != preEC11AStat){
      //Serial.printf("\ndown[%d],up[%d]: currA[%d],preA[%d] currB[%d] preB[%d]",CONST_APP_ROTARY_ENCODER_DOWN_STATUS,CONST_APP_ROTARY_ENCODER_UP_STATUS,currEC11AStat,preEC11AStat,currEC11BStat,preEC11BStat);
      preEC11AStat=currEC11AStat;      
      preEC11BStat=currEC11BStat;      
      
      if (currEC11AStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS){
        //Serial.println("\ncurrEC11AStat=CONST_APP_ROTARY_ENCODER_DOWN_STATUS");
        if (currEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS){
          //Serial.printf("\n ret:[%d]",ret);
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_ADD;
        }
        else{
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_MIN;
          //Serial.printf("\n ret:[%d]",ret);
        }
      }
    }
    detectionTick=ulReset_interval();
  }
  return ret;
  
}

int RotaryEncoder::read_two_pulse()
{
  int ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_NULL;
  if (ulGet_interval(detectionTick) > CONST_APP_ROTARY_ENCODER_DETECTION_TIME) {
    currEC11AStat = digitalRead(_pinA);
    currEC11BStat = digitalRead(_pinB);

    // 当A发生跳变时采集B当前的状态，并将B与上一次的状态进行对比
    // 若A 0->1 时，B 1->0 正转；若A 1->0 时，B 0->1 正转
    // 若A 0->1 时，B 0->1 反转；若A 1->0 时，B 1->0 反转    
    if (currEC11AStat!=preEC11AStat){
      preEC11AStat=currEC11AStat;      
      preEC11BStat=currEC11BStat;      

      if (currEC11AStat==CONST_APP_ROTARY_ENCODER_UP_STATUS){
        if ((currEC11BStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS)&&(preEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_ADD;
        }
        if ((currEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS)&&(preEC11BStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_MIN;
        }
        
        //>>>>>>>>>>>>>>>>下面为正转一次再反转或反转一次再正转处理<<<<<<<<<<<<<<<<//
        if ((preEC11BStat==currEC11BStat)&&(currEC11BStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_ADD;          
        }
        
        if ((preEC11BStat==currEC11BStat)&&(currEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_MIN;          
        }
        
      }
      else{
        if ((currEC11BStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS)&&(preEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_MIN;
        }
        if ((currEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS)&&(preEC11BStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_ADD;
        }
        
        //>>>>>>>>>>>>>>>>下面为正转一次再反转或反转一次再正转处理<<<<<<<<<<<<<<<<//
        if ((preEC11BStat==currEC11BStat)&&(currEC11BStat==CONST_APP_ROTARY_ENCODER_DOWN_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_MIN;          
        }
        
        if ((preEC11BStat==currEC11BStat)&&(currEC11BStat==CONST_APP_ROTARY_ENCODER_UP_STATUS)){
          ret=CONST_APP_ROTARY_ENCODER_EC11_RTN_ADD;          
        }
        
      }
      
    }
    detectionTick=ulReset_interval();
  }
  return ret;
  
}

unsigned long RotaryEncoder::ulReset_interval()
{
  return millis();
}

unsigned long RotaryEncoder::ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;
}