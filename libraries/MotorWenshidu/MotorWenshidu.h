/*
   MotorWenshidu.h

   The definition of weishidu
   
   需要 wenshidu.h 和 Motor_DQ.h 
   通过温湿度控制马达控制的class

   2017  Steven Lian (steven.lian@gmail.com)


*/
#pragma once
#ifndef _MOTOR_WENSHIDU_H
#define _MOTOR_WENSHIDU_H

#include "wenshidu.h"
#include "Motor_DQ.h"

//Motor wendu control

#define CONST_APP_MOTOR_WENSHIDU_FILE_NAME "/motor_wenshidu_rule.data"
#define CONST_APP_MOTOR_WENSHIDU_FILE_SIZE 1024
#define CONST_APP_MOTOR_WENSHIDU_BIG_BUFF_SIZE 120

#define CONST_MIN_761_DATA_LEN 2
#define CONST_APP_MOTOR_SERVER_CMD_SPEED_PERCENTAGE 0
#define CONST_APP_MOTOR_SERVER_CMD_MAX_SPEED_VAL 1
#define CONST_APP_MOTOR_SERVER_CMD_ADJUST_PERCENTAGE 2
#define CONST_APP_MOTOR_SERVER_CMD_SPEED_VAL 3
#define CONST_APP_MOTOR_SERVER_CMD_WENSHIDU_RULE 4

#define CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM 10

#define CONST_APP_MOTOR_DEFAULT_RANGE_OFFSET 50



typedef struct {
  uint8_t wenduRuleNum;
  uint8_t shiduRuleNum;
  uint8_t wenduVal[CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM];
  short wenduSpeed[CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM];
  uint8_t shiduVal[CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM];
  short shiduSpeed[CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM];

} stMotorControlRule;


class MotorWenshidu
{
  public:
    //data
    uint8_t autoControlEnable;

    //func
    bool begin(motor_DQ *motor, wenshidu *wenshidu,char *dataPtr);
    void loop();

    bool decode_wenshidu_command(char *ptr);
    bool decode_761_command(char *ptr);

    void debug_print_rule();

    bool bLoad_config();
    bool bSave_config();

  private:
    //data
    char *dataBuffPtr;
    short nRangeOffset;
    motor_DQ *motorPtr;
    wenshidu *wenshiduPtr;

    stMotorControlRule controlRule;
    //func
    short get_right_speed();
    bool isInRange(short speed);
    void disable_auto_control();
};



#endif // _MOTOR_WENSHIDU_H
