#include <MotorWenshidu.h>



bool MotorWenshidu::begin(motor_DQ *motor, wenshidu *wenshidu,char *dataPtr)
{
  motorPtr = motor;
  wenshiduPtr = wenshidu;
  dataBuffPtr=dataPtr;
  nRangeOffset = CONST_APP_MOTOR_DEFAULT_RANGE_OFFSET;
  bLoad_config();
}

void MotorWenshidu::loop()
{
  short nSpeed;
  nSpeed = get_right_speed();
  if (!isInRange(nSpeed))
  {
    motorPtr->bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
    motorPtr->bufCommand.speed = nSpeed;
    motorPtr->send_CMD(&motorPtr->bufCommand);
    //DBGPRINTF("\n === current speed [%d] is not in range, take action!", nSpeed);
  }
  else{
    //DBGPRINTF("\n === current speed [%d] is in range, nothing to do!", nSpeed);    
  }
}

short  MotorWenshidu::get_right_speed()
{
  short ret;
  short i;
  short wdSpeed = 0, sdSpeed = 0;
  ret = motorPtr->currStat.speed;
  //DBGPRINTF("\n==  motorPtr.currStat.speed:[%d]", ret);
  autoControlEnable = controlRule.shiduRuleNum + controlRule.wenduRuleNum;
  if (autoControlEnable>0)
  {
    //check shidu
    for (i = 0; i < controlRule.shiduRuleNum; i++)
    {
      if (wenshiduPtr->nShiduInt < controlRule.shiduVal[i]) {
        sdSpeed = controlRule.shiduSpeed[i];
      }
    }
    //DBGPRINTF("\n== Shidu check speed result:[%d]", sdSpeed);

    //check wendu
    for (i = 0; i < controlRule.wenduRuleNum; i++)
    {
      if (wenshiduPtr->nWenduInt < controlRule.wenduVal[i]) {
        wdSpeed = controlRule.wenduSpeed[i];
      }
    }
    //DBGPRINTF("\n== Wendu check speed result:[%d]", wdSpeed);

    //
    if (sdSpeed > wdSpeed) {
      ret = sdSpeed;
    }
    else {
      ret = wdSpeed;
    }

  }
  if (ret < 0)
    ret = 0;
  if (ret > motorPtr->currStat.maxSpeed)
    ret = motorPtr->currStat.maxSpeed;
  //DBGPRINTF("\n== final check speed result:[%d]", ret);

  return ret;
}


bool MotorWenshidu::isInRange(short speed)
{
  if ((motorPtr->currStat.speed) > (speed - nRangeOffset) &&
      (motorPtr->currStat.speed) < (speed + nRangeOffset))
    return true;
  else
    return false;
}


bool MotorWenshidu::decode_wenshidu_command(char *ptr)
{
  bool ret = false;
  short nT1;
  short i;
  char *spStart;
  char *spData;
  short cmdCodeVer;
  stMotorControlRule buffRule;

  spStart = ptr;

  spStart = get_short_split(spStart, &nT1, ';');

  cmdCodeVer = nT1;

  //DBGPRINTF("\nCODE Version: [%d]", cmdCodeVer);

  if (cmdCodeVer == 1 || cmdCodeVer == 2) {
    //code version 1 format

    //wendu
    //wendu control rule number 0-->nothing to do;
    spStart = get_short_split(spStart, &nT1, ':');
    if (spStart == NULL || nT1 < 0 || nT1 > CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM) {
      return ret;
    }
    buffRule.wenduRuleNum = nT1;
    for (i = 0; i < buffRule.wenduRuleNum; i++) {
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL || nT1 < 0) {
        return ret;
      }
      buffRule.wenduVal[i] = nT1;
      spStart = get_short_split(spStart, &nT1, ',');
      if (spStart == NULL || nT1 < 0) {
        return ret;
      }
      buffRule.wenduSpeed[i] = nT1;
    }

    spStart = get_short_split(spStart, &nT1, ';');
    if (spStart == NULL || nT1 < 0) {
      return ret;
    }

    //shidu
    //shidu control rule number 0-->nothing to do;
    spStart = get_short_split(spStart, &nT1, ':');
    if (spStart == NULL || nT1 < 0 || nT1 > CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM) {
      return ret;
    }
    buffRule.shiduRuleNum = nT1;
    for (i = 0; i < buffRule.shiduRuleNum; i++) {
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL || nT1 < 0) {
        return ret;
      }
      buffRule.shiduVal[i] = nT1;
      spStart = get_short_split(spStart, &nT1, ',');
      if (spStart == NULL || nT1 < 0) {
        return ret;
      }
      buffRule.shiduSpeed[i] = nT1;
    }

  }

  ret = true;
  memcpy(&controlRule, &buffRule, sizeof(controlRule));

  //DBGPRINTF("\n--END--%s", "");
}

void MotorWenshidu::disable_auto_control()
{
  controlRule.shiduRuleNum=0;
  controlRule.wenduRuleNum=0;
  autoControlEnable=controlRule.shiduRuleNum+controlRule.wenduRuleNum;  
  bSave_config();

}

bool MotorWenshidu::decode_761_command(char *ptr)
{
  bool ret = false;
  short nT1;
  char *spStart;

  nT1 = strlen(ptr);
  if (nT1 <= CONST_MIN_761_DATA_LEN)
    return ret;

  spStart = ptr;
  //code version;
  spStart = get_short_split(spStart, &nT1, ':');
  if (spStart == NULL) {
    return ret;
  }

  //DBGPRINTF("\nCMD Type: [%d]", nT1);

  switch (nT1)
  {
    case CONST_APP_MOTOR_SERVER_CMD_SPEED_PERCENTAGE:
      spStart = get_short_split(spStart, &nT1, ';');
      if (nT1 >= 0) {
        disable_auto_control();
        
        motorPtr->bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        motorPtr->bufCommand.speed = (motorPtr->currStat.maxSpeed / 100 * nT1);
        motorPtr->send_CMD(&motorPtr->bufCommand);
        ret = true;
      }
      break;
    case CONST_APP_MOTOR_SERVER_CMD_MAX_SPEED_VAL:
      spStart = get_short_split(spStart, &nT1, ';');
      if (nT1 > 0) {
        motorPtr->bufCommand.cmd = CONST_APP_MOTOR_SET_MAX_CYCLE;
        motorPtr->bufCommand.speed = nT1;
        motorPtr->send_CMD(&motorPtr->bufCommand);
        ret = true;
      }
      break;
    case CONST_APP_MOTOR_SERVER_CMD_ADJUST_PERCENTAGE:
      disable_auto_control();
      
      spStart = get_short_split(spStart, &nT1, ';');
      nT1 = motorPtr->currStat.maxSpeed / 100 * nT1;
      nT1 = motorPtr->currStat.speed + nT1;
      if (nT1 > motorPtr->currStat.maxSpeed)
        nT1 = motorPtr->currStat.maxSpeed;
      if (nT1 < 0)
        nT1 = 0;
      motorPtr->bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
      motorPtr->bufCommand.speed = (nT1);
      motorPtr->send_CMD(&motorPtr->bufCommand);
      ret = true; 
      break;
    case CONST_APP_MOTOR_SERVER_CMD_SPEED_VAL:
      spStart = get_short_split(spStart, &nT1, ';');
      if (nT1 >= 0) {
        disable_auto_control();
        
        motorPtr->bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        motorPtr->bufCommand.speed = (nT1);
        motorPtr->send_CMD(&motorPtr->bufCommand);
        ret = true;
      }
      break;
    case CONST_APP_MOTOR_SERVER_CMD_WENSHIDU_RULE:
      ret = decode_wenshidu_command(spStart);
      if (ret)
      {
        debug_print_rule();
      }
      break;
    default:
      break;

  }

  return ret;
}


/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR MotorWenshidu::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  //uint8_t bigBuff[CONST_APP_MOTOR_WENSHIDU_BIG_BUFF_SIZE + 2];

  File configFile = SPIFFS.open(CONST_APP_MOTOR_WENSHIDU_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s",CONST_APP_MOTOR_WENSHIDU_FILE_NAME);
    return bRet;
  }

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', dataBuffPtr, CONST_APP_MOTOR_WENSHIDU_BIG_BUFF_SIZE);
    DBGPRINTF("\n%d %s",nLen,dataBuffPtr);
    if (nLen <= 0)
      break;
    dataBuffPtr[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)dataBuffPtr, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    // wenduRuleNum
    if (memcmp(dataBuffPtr, "wdn", 3) == 0) {
      controlRule.wenduRuleNum = atoi(spTemp);
    }
    
    //wenduVal
    if (memcmp(dataBuffPtr, "wdv", 3) == 0) {
      int nPos;
      *(spTemp + 3) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM)) {
        controlRule.wenduVal[nPos] = atoi(spTemp + 4);
        //DBGPRINTF("\n====== Read: %s  %s  %s %d  %d",dataBuffPtr,spTemp,spTemp + 4,nPos,controlRule.wenduVal[nPos]);
      }
    }

    //wenduSpeed
    if (memcmp(dataBuffPtr, "wds", 3) == 0) {
      int nPos;
      *(spTemp + 3) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM)) {
        controlRule.wenduSpeed[nPos] = atoi(spTemp + 4);
      }
    }

    // shiduRuleNum
    if (memcmp(dataBuffPtr, "sdn", 3) == 0) {
      controlRule.shiduRuleNum = atoi(spTemp);

    }
    
    //shiduVal
    if (memcmp(dataBuffPtr, "sdv", 3) == 0) {
      int nPos;
      *(spTemp + 3) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM)) {
        controlRule.shiduVal[nPos] = atoi(spTemp + 4);
      }
    }

    //shiduSpeed
    if (memcmp(dataBuffPtr, "sds", 3) == 0) {
      int nPos;
      *(spTemp + 3) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_APP_MOTOR_WENSHIDU_MAX_RULE_NUM)) {
        controlRule.shiduSpeed[nPos] = atoi(spTemp + 4);
      }
    }

  }
  configFile.close();
  DBGPRINTLN("\nMotorWenshidu Config ok");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR MotorWenshidu::bSave_config()
{
  
  //uint8_t bigBuff[CONST_APP_MOTOR_WENSHIDU_BIG_BUFF_SIZE + 2];

  //DBGPRINTLN("--save application data--");
  File configFile = SPIFFS.open(CONST_APP_MOTOR_WENSHIDU_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_MOTOR_WENSHIDU_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }
  
  int i;
  
  // wenduRuleNum  
  configFile.print("wdn:");
  configFile.println(controlRule.wenduRuleNum);

  //wenduVal
  for (i = 0; i < controlRule.wenduRuleNum; i++) {
    sprintf((char *)dataBuffPtr, "wdv:%03d:%d", i, controlRule.wenduVal[i]);
    configFile.println((char *)dataBuffPtr);    
  }

  //wenduSpeed
  for (i = 0; i < controlRule.wenduRuleNum; i++) {
    sprintf((char *)dataBuffPtr, "wds:%03d:%d", i, controlRule.wenduSpeed[i]);
    configFile.println((char *)dataBuffPtr);    
  }

  // shiduRuleNum  
  configFile.print("sdn:");
  configFile.println(controlRule.shiduRuleNum);

  //shiduVal
  for (i = 0; i < controlRule.wenduRuleNum; i++) {
    sprintf((char *)dataBuffPtr, "sdv:%03d:%d", i, controlRule.shiduVal[i]);
    configFile.println((char *)dataBuffPtr);    
  }

  //shiduSpeed
  for (i = 0; i < controlRule.wenduRuleNum; i++) {
    sprintf((char *)dataBuffPtr, "sds:%03d:%d", i, controlRule.shiduSpeed[i]);
    configFile.println((char *)dataBuffPtr);    
  }
  
  configFile.close();
  //DBGPRINTLN(" -- end");
  return true;
}



void MotorWenshidu::debug_print_rule()
{
  short i;
  DBGPRINT("\n=== control rule ===");
  DBGPRINTF("\n wenduRuleNum [%d]", controlRule.wenduRuleNum);
  for (i = 0; i < controlRule.wenduRuleNum; i++)
  {
    DBGPRINTF("\n wenduVal     [%d] wenduSpeed [%d]", controlRule.wenduVal[i],controlRule.wenduSpeed[i]);
  }
  DBGPRINTF("\n\n shiduRuleNum [%d]", controlRule.shiduRuleNum);
  for (i = 0; i < controlRule.shiduRuleNum; i++)
  {
    DBGPRINTF("\n shiduVal     [%d] shiduSpeed [%d]", controlRule.shiduVal[i],controlRule.shiduSpeed[i]);
  }
}
