#include <LVwenshiduIrda.h>

//modified by steven.lian@gmail.com 2017/7/26

//class functions LoRaHost


unsigned short wenshidu::get_version()
{
  return version;
}


short wenshidu::begin(LVIrda *irPtr, char *dataPtr)
{
  version=CONST_APP_WENSHI_VERSION;

  valid = false;
  wenduEnable = 0;
  shiduEnable = 0;

  nWenduInt = 0;
  nWenduDec = 0;
  nShiduInt = 0;
  nShiduDec = 0;
  nPreWenduInt = 0;
  nPreShiduInt = 0;
  decWendu[0] = 0;
  decShidu[0] = 0;
  lastCommandType = 0;
  lastCommandCount = 0;
  deviceReady = 0;
  currPower = 0;
  recordPos = 0;
  
  nWenduOffset=0;
  nShiduOffset=0;
  nWenduAdjust=0;
  nShiduAdjust=0;

  sameCMDReactionCount = CONST_APP_WENSHI_MAX_SAME_COMMAND_COUNT;
  ulCheckActionCycleTimeInMS = CONST_CHECK_ACTION_CYCLE;

  bLastCommanIfActived = false; //最后一个命令是否正确执行，true==yes

  chWenduLowOperator = ' '; //判断方式">","<",...
  chWenduHighOperator = ' ';
  chShiduLowOperator = ' ';
  chShiduHighOperator = ' ';


  DBGPRINTLN("\n==wenshidu::begin==");
  irdaPtr = irPtr;
  dataBuffPtr = dataPtr;
  if (bLoad_config()) {
    valid = true;
  }
  else {
    valid = false;
  }

  pinMode(dht_dpin, INPUT);
  deviceReady = isDHT11or22();

  if (deviceReady == 0) {
    valid = false;
  }
  
  //确保开机以后就检查是否温湿度符合要求
  ulCheckActionCycleTick= -CONST_CHECK_ACTION_CYCLE;
  
}


bool wenshidu::_testDHT11or12(uint8_t type)
{
  bool ret = false;
  int chk;
  DBGPRINTF("Try dht%d\n", type);
  switch (type)
  {
    case CONST_DHT_DEVICE_TYPE_DHT11:
      chk = DHT.read11(dht_dpin); //
      break;
    case CONST_DHT_DEVICE_TYPE_DHT22:
      chk = DHT.read22(dht_dpin); //
      break;
    default:
      chk = DHTLIB_ERROR_TIMEOUT;
      break;
  }
  switch (chk)
  {
    case DHTLIB_OK:
      DBGPRINTLN("OK");
      ret = true;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      DBGPRINTLN("Checksum error");
      ret = false;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      DBGPRINTLN("Time out error");
      ret = false;
      break;
    default:
      DBGPRINTLN("Unknown error");
      ret = false;
      break;
  }
  return ret;
}

uint8_t wenshidu::isDHT11or22()
{
  uint8_t ret = 0;
  bool dht11Ready, dht22Ready;
  short tryTimes = CONST_DHT_TRY_TIMES;
  while (tryTimes > 0)
  {
    delay(CONST_DHT_DELAY_TIME);
    dht22Ready = _testDHT11or12(CONST_DHT_DEVICE_TYPE_DHT22);
    delay(CONST_DHT_DELAY_TIME);
    dht11Ready = _testDHT11or12(CONST_DHT_DEVICE_TYPE_DHT11);
    if ((dht22Ready == true) && (dht11Ready == false))
    {
      ret = CONST_DHT_DEVICE_TYPE_DHT22;
    }
    else if (dht11Ready == true) {
      ret = CONST_DHT_DEVICE_TYPE_DHT11;
    }
    else {
      ret = 0;
    }
    tryTimes--;
    DBGPRINTF("\nret [%d]\n", ret);

    if (ret > 0) {
      break;
    }
  }
  return ret;
}


uint8_t wenshidu::devReady()
{
  return deviceReady;
}

//检查是否收到数据
short wenshidu::read()
{
  short ret = 0;
  int nT1, nT2;
  if (deviceReady > 0)
  {

    switch (deviceReady) {
      case CONST_DHT_DEVICE_TYPE_DHT11:
        nT1 = DHT.read11(dht_dpin);
        break;
      case CONST_DHT_DEVICE_TYPE_DHT22:
        nT1 = DHT.read22(dht_dpin);
        break;
      default:
        break;

    }
    //DHT.read22(dht_dpin);
    if (DHT.temperature < 100.0 && DHT.temperature > -30.0 && DHT.humidity >= 0.0 && DHT.humidity <= 100.0)
    {
      //keep old data;
      nPreWenduInt = nWenduInt;
      nPreShiduInt = nShiduInt;

      nWenduInt = DHT.temperature;
      nWenduDec = (DHT.temperature - nWenduInt) * 100 ;
      nWenduInt += nWenduOffset;
      nWenduInt += nWenduAdjust;
      nShiduInt = DHT.humidity;
      nShiduDec = (DHT.humidity - nShiduInt) * 100 ;
      nShiduInt += nShiduOffset;
      nShiduInt += nShiduAdjust;
      nT1 = abs(nWenduInt - nPreWenduInt);
      nT2 = abs(nShiduInt - nPreShiduInt);
      sprintf(decWendu, "%d.%d", nWenduInt, nWenduDec);
      sprintf(decShidu, "%d.%d", nShiduInt, nShiduDec);

      if (( nT1 >= CONST_WENDU_CHANGE_ALARM_VALUE) || ( nT2 >= CONST_SHIDU_CHANGE_ALARM_VALUE))
      {
        ret = 2;
      }
      else {
        ret = 1;
      }
      //检查是否现在温湿度是否需要有相应的设备操作
      check_status_action();
      //检查相应操作的结果，主要是判断压缩机是否工作
      check_action_result();
    }
  }

  return ret;
}


bool wenshidu::if_need_to_take_action(uint8_t commandType)
{
  bool ret = true;
  DBGPRINTF("\n if_need_to_take_action [curr : %d, last : %d] ", commandType, lastCommandType);
  if (commandType == lastCommandType) {
    DBGPRINTF(" curr=last; codever [%d,%d] bLastCommanIfActived [%d] ", cmdCodeVer, CONST_APP_WENSHI_803_CODE_VER_NORMAL,bLastCommanIfActived);
    if ((cmdCodeVer == CONST_APP_WENSHI_803_CODE_VER_NORMAL) ||
        (cmdCodeVer == CONST_APP_WENSHI_803_CODE_VER_PINGPANG && bLastCommanIfActived==false )) {
      //若干个命令以后可以重复执行
      lastCommandCount++;
      DBGPRINTF(" lastCommandCount [%d]", lastCommandCount);
      if (lastCommandCount <= sameCMDReactionCount) {
        ret = false;
      }
      else {
        lastCommandCount = 0;
      }
    }
    else{
      ret =false;
    }
  }
  else {
      lastCommandType = commandType;
      lastCommandCount = 0;
  }
  DBGPRINTF(" ret [%d]\n", ret);
  return ret;
}

short wenshidu::get_wenduOffset()
{
  return nWenduOffset;  
}

short wenshidu::get_shiduOffset()
{
  return nShiduOffset;    
}

short wenshidu::get_wenduAdjust()
{
  return nWenduAdjust;  
}

short wenshidu::get_shiduAdjust()
{
  return nShiduAdjust;    
}


void wenshidu::add_record(uint8_t type)
{
  //record 
  record[recordPos]=type;
  recordPos++;
  if(recordPos>=CONST_APP_WENSHIDU_CMD_RECORD_LIST_LEN){
    recordPos=0;
  }
}

void wenshidu::clean_record()
{
  //record 
  memset(record,0,CONST_APP_WENSHIDU_CMD_RECORD_LIST_LEN);
  recordPos=0;
}


//检查温湿度状态， 判断是否需要发送命令，以及命令类型
void wenshidu::check_status_action()
{
  if ((ulGet_interval(ulCheckActionCycleTick) < ulCheckActionCycleTimeInMS)) {
    return;
  }

  DBGPRINTF("\ncheck_status_action() %d", valid);


  if ((ulGet_interval(ulCheckActionCycleTick) > ulCheckActionCycleTimeInMS) && (valid))
  {
    short i;
    ulCheckActionCycleTick = ulReset_interval();
    if (nWenduInt < nWenduLow) {
      //温度低于要求
      if (if_need_to_take_action(CONST_APP_WENSHI_CMD_TYPE_WENDU_LOW) && wenduEnable)
      {
        DBGPRINTF("\nWendu [%d] < [%d],will send [%s]", nWenduInt, nWenduLow, wenduLowActionString[0]);
        for (i = 0; i < nWenduActionLen; i++) {
          irdaPtr->irsend6(wenduLowActionString[i], dataBuffPtr);
        }
        bLastCommanIfActived = false;
        ulActivedCheckTick = ulReset_interval();
        add_record(CONST_APP_WENSHI_CMD_TYPE_WENDU_LOW);
      }

    }
    else if (nWenduInt > nWenduHigh) {
      //温度高于要求
      if (if_need_to_take_action(CONST_APP_WENSHI_CMD_TYPE_WENDU_HIGH)&& wenduEnable)
      {
        DBGPRINTF("\nWendu [%d] > [%d],will send [%s]", nWenduInt, nWenduHigh, wenduHighActionString[0]);
        for (i = 0; i < nWenduActionLen; i++) {
          irdaPtr->irsend6(wenduHighActionString[i], dataBuffPtr);
        }
        bLastCommanIfActived = false;
        ulActivedCheckTick = ulReset_interval();
        add_record(CONST_APP_WENSHI_CMD_TYPE_WENDU_HIGH);
      }

    }
    else {
      //温度满足要求的情况下，可以调节湿度
      if (nShiduInt < nShiduLow) {
        //湿度低于要求
        if (if_need_to_take_action(CONST_APP_WENSHI_CMD_TYPE_SHIDU_LOW) && shiduEnable)
        {
          DBGPRINTF("\nShidu [%d] < [%d],will send [%s]", nShiduInt, nShiduLow, shiduLowActionString[0]);
          for (i = 0; i < nShiduActionLen; i++) {
            irdaPtr->irsend6(shiduLowActionString[i], dataBuffPtr);
          }
          bLastCommanIfActived = false;
          ulActivedCheckTick = ulReset_interval();
          add_record(CONST_APP_WENSHI_CMD_TYPE_SHIDU_LOW);
        }

      }
      else if (nShiduInt > nShiduHigh) {
        //湿度高于要求
        if (if_need_to_take_action(CONST_APP_WENSHI_CMD_TYPE_SHIDU_HIGH) && shiduEnable)
        {
          DBGPRINTF("\nShidu [%d] > [%d],will send [%s]", nShiduInt, nShiduHigh, shiduHighActionString[0]);
          for (i = 0; i < nShiduActionLen; i++) {
            irdaPtr->irsend6(shiduHighActionString[i], dataBuffPtr);
          }
          bLastCommanIfActived = false;
          ulActivedCheckTick = ulReset_interval();
          add_record(CONST_APP_WENSHI_CMD_TYPE_SHIDU_HIGH);
         
        }
      }
    }
  }
}

//判断一段时间内是否有压缩机工作过
bool wenshidu::check_operator_power(bool status, char chOperator, unsigned short nPower, unsigned short nThreshold)
{
  bool ret = false;
  switch (chOperator)
  {
    case '<':
      //主要检测是否关机或者吹风状态。
      //如果有一次大于门限，说明压缩机没有关
      if (nPower > nThreshold)
        ret = false;
      else
        ret = true;
      break;
    case '>':
      //空调压缩机工作的时候，功率一般>200,只要有过>200的功率，表明，压缩机工作了,因此只需要检测一次>nThreshold,就说明命令有效
      if (status) {
        //功率大于过门限
        ret = true;
      }
      else {
        if (nPower > nThreshold)
          ret = true;
      }
      break;
    default:
      break;
  }
  return ret;

}

//检查上个命令是否执行，主要是通过检测功率和比较相应命令的功率要求
void wenshidu::check_action_result()
{
  if ((ulGet_interval(ulActivedCheckTick) < ulActivedCheckTimeInMs)) {
    return;
  }
  switch (lastCommandType)
  {
    case CONST_APP_WENSHI_CMD_TYPE_WENDU_LOW:
      bLastCommanIfActived = check_operator_power(bLastCommanIfActived, chWenduLowOperator, currPower, nWenduLowPowerVal);
      break;
    case CONST_APP_WENSHI_CMD_TYPE_WENDU_HIGH:
      bLastCommanIfActived = check_operator_power(bLastCommanIfActived, chWenduHighOperator, currPower, nWenduHighPowerVal);
      break;
    case CONST_APP_WENSHI_CMD_TYPE_SHIDU_LOW:
      bLastCommanIfActived = check_operator_power(bLastCommanIfActived, chShiduLowOperator, currPower, nShiduLowPowerVal);
      break;
    case CONST_APP_WENSHI_CMD_TYPE_SHIDU_HIGH:
      bLastCommanIfActived = check_operator_power(bLastCommanIfActived, chShiduHighOperator, currPower, nShiduHighPowerVal);
      break;
    default:
      break;
  }
}


//802 修正温度值，+3 把测量值+3度，-2 把测量值 -2度
// 也可以同时调整湿度值， 格式:3;30 or 3
bool wenshidu::decode_802_command(char *ptr)
{
  bool ret = false;
  short nT1;
  nT1 = strlen(ptr);
  if (nT1 > 0)
  {
    char *spWendu,*spShidu,*spStr;
    spWendu=ptr;
    spStr=strchr(ptr,';');
    if (spStr!=NULL){
      *spStr=0;
      spShidu=spStr+1;
      nT1=atoi(spShidu);
      if (abs(nT1) <= CONST_SHIDU_MAX_OFFSET_VAL)
      {
        nShiduOffset = nT1;
        ret = true;
      } 
    }
    nT1 = atoi(spWendu);
    if (abs(nT1) <= CONST_WENDU_MAX_OFFSET_VAL)
    {
      nWenduOffset = nT1;
      ret = true;
    }
  }
  return ret;
}

//802 修正温度值，+3 把测量值+3度，-2 把测量值 -2度
// 也可以同时调整湿度值， 格式:3;30 or 3
bool wenshidu::decode_805_command(char *ptr)
{
  bool ret = false;
  short nT1;
  nT1 = strlen(ptr);
  if (nT1 > 0)
  {
    char *spWendu,*spShidu,*spStr;
    spWendu=ptr;
    spStr=strchr(ptr,';');
    if (spStr!=NULL){
      *spStr=0;
      spShidu=spStr+1;
      nT1=atoi(spShidu);
      if (abs(nT1) <= CONST_SHIDU_MAX_OFFSET_VAL)
      {
        nShiduAdjust = nT1;
        ret = true;
      } 
    }
    nT1 = atoi(spWendu);
    if (abs(nT1) <= CONST_WENDU_MAX_OFFSET_VAL)
    {
      nWenduAdjust = nT1;
      ret = true;
    }
  }
  return ret;
}



/*
  803 command sample
  格式:
  类型;红外固定码;温度控制是否有效(6=有效):低温:高温:低温红外命令条数:红外命令,:高温红外命令条数:红外命令,;湿度控制是否有效(6=有效):低湿度:高湿度:低湿度红外命令条数:红外命令,:高湿度命令条数:红外命令,;
  如果类型是2,有检测控制命令是否有效的功率检测范围数据
  
  类型1
  1;2601029411fe1002460246020246022c06024602881300;6:24:25:1:0230004DB2DE2107F80000,:1:0130004DB2F00F07F80000,;6:50:70:1:0230004DB2FD0227D80000,:1:0230004DB2F80713EC0000,;
  解释:
  类型1,<24 >25度控制，湿度<50或者 >70控制
  
  类型2
  2;2601029411fe1002460246020246022c06024602881300;6:24:25:1:0230004DB2DE2107F80000,:1:0130004DB2F00F07F80000,;6:50:70:1:0230004DB2FD0227D80000,:1:0230004DB2F80713EC0000,;60:<:200:>:200:<:200:>:200:;
  解释:
  类型2,<24 >25度控制，湿度<50或者 >70控制
  检测时间60秒后，低温命令出现<200瓦说明有效，高温命令>200瓦说明有效，低湿度命令出现<200瓦说明有效，高湿度命令>200瓦说明有效
  
*/
bool wenshidu::decode_803_command(char *ptr)
{
  bool ret = false;
  short nT1;
  short nT2;
  short i;
  char *spStart;
  char *spData;
  if (deviceReady == 0) {
    deviceReady = isDHT11or22();
  }

  //deviceReady=1;
  
  if (deviceReady) {

    nT1 = strlen(ptr);
    if (nT1 <= CONST_MIN_803_DATA_LEN)
      return ret;

    spStart = ptr;
    //code version;
    spStart = get_short_split(spStart, &nT1, ';');
    if (spStart == NULL) {
      return ret;
    }

    cmdCodeVer = nT1;

    DBGPRINTF("\nCODE Version: [%d]", cmdCodeVer);

    if (cmdCodeVer == CONST_APP_WENSHI_803_CODE_VER_NORMAL || cmdCodeVer == CONST_APP_WENSHI_803_CODE_VER_PINGPANG) {
      //code version 1 format

      //fixcode;
      spStart = get_str_split(spStart, &spData, ';');
      if (spStart == NULL) {
        return ret;
      }

      DBGPRINTF("\nFIXCODE: [%s]", spData);

      nT1 = strlen(spData);
      if (nT1 < CONST_IRDA_MIN_FIXCODELEN) {
        return ret;
      }

      //irdaPtr->fixcode(spData);

      //wendu
      //action type
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nTYPE:[%d]", nT1);
      if (nT1 == 6) {
        wenduEnable = 1;
      }
      else {
        wenduEnable = 0;
      }
      DBGPRINTF("\wenduEnable:[%d]", wenduEnable);

      //wendu low
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nwenduLow:[%d]", nT1);
      if ((nT1 < CONST_WENDU_LOW_VAL) || (nT1 > CONST_WENDU_HIGH_VAL))
      {
        return ret;
      }
      nWenduLow = nT1;

      //wendu high
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nwenduHigh:[%d]", nT1);
      if ((nT1 < CONST_WENDU_LOW_VAL) || (nT1 > CONST_WENDU_HIGH_VAL))
      {
        return ret;
      }
      nWenduHigh = nT1;

      //low irda command count
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nirda command:[%d]", nT1);
      if ((nT1 <= 0) || (nT1 > CONST_MAX_IRDA_SUB_COMMAND_LEN))
      {
        return ret;
      }
      //low command
      for (i = 0; i < nT1; i++) {
        spStart = get_str_split(spStart, &spData, ',');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nlowWendu command[%s]", spData);
        if (strlen(spData) <= 0) {
          return ret;
        }
        strncpy(wenduLowActionString[i], spData, CONST_IRDA_SUB_COMMAND_LENGTH);
      }

      //分割符号
      spStart = strchr(spStart, ':');
      if (spStart == NULL) {
        return ret;
      }
      spStart++;

      //high irda command count
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nirda command:[%d]", nT1);
      if ((nT1 <= 0) || (nT1 > CONST_MAX_IRDA_SUB_COMMAND_LEN))
      {
        return ret;
      }
      nWenduActionLen = nT1;

      //high command
      for (i = 0; i < nT1; i++) {
        spStart = get_str_split(spStart, &spData, ',');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nhighWendu command[%s]", spData);
        if (strlen(spData) <= 0) {
          return ret;
        }
        strncpy(wenduHighActionString[i], spData, CONST_IRDA_SUB_COMMAND_LENGTH);
      }

      //分割符号
      spStart = strchr(spStart, ';');
      if (spStart == NULL) {
        return ret;
      }
      spStart++;

      //shidu
      //action type
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nTYPE:[%d]", nT1);
      if (nT1 == 6) {
        shiduEnable = 1;
      }
      else {
        shiduEnable = 0;
      }
      DBGPRINTF("\shiduEnable:[%d]", shiduEnable);

      //shidu low
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nshiduLow:[%d]", nT1);
      if ((nT1 < CONST_SHIDU_LOW_VAL) || (nT1 > CONST_SHIDU_HIGH_VAL))
      {
        return ret;
      }
      nShiduLow = nT1;

      //wendu high
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nwenduHigh:[%d]", nT1);
      if ((nT1 < CONST_SHIDU_LOW_VAL) || (nT1 > CONST_SHIDU_HIGH_VAL))
      {
        return ret;
      }
      nShiduHigh = nT1;

      //low irda command count
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nirda command:[%d]", nT1);
      if ((nT1 < 0) || (nT1 > CONST_MAX_IRDA_SUB_COMMAND_LEN))
      {
        return ret;
      }
      //low command
      for (i = 0; i < nT1; i++) {
        spStart = get_str_split(spStart, &spData, ',');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nlowShidu command[%s]", spData);
        if (strlen(spData) <= 0) {
          return ret;
        }
        strncpy(shiduLowActionString[i], spData, CONST_IRDA_SUB_COMMAND_LENGTH);
      }

      spStart = strchr(spStart, ':');
      if (spStart == NULL) {
        return ret;
      }
      spStart++;

      //high irda command count
      spStart = get_short_split(spStart, &nT1, ':');
      if (spStart == NULL) {
        return ret;
      }
      DBGPRINTF("\nirda command:[%d]", nT1);
      if ((nT1 < 0) || (nT1 > CONST_MAX_IRDA_SUB_COMMAND_LEN))
      {
        return ret;
      }
      nShiduActionLen = nT1;

      //high command
      for (i = 0; i < nT1; i++) {
        spStart = get_str_split(spStart, &spData, ',');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nhighShidu command[%s]", spData);
        if (strlen(spData) <= 0) {
          return ret;
        }
        strncpy(shiduHighActionString[i], spData, CONST_IRDA_SUB_COMMAND_LENGTH);
      }

      //code Version ==2
      if (cmdCodeVer == CONST_APP_WENSHI_803_CODE_VER_PINGPANG)
      {
        char chT;
        //分割符号
        spStart = strchr(spStart, ';');
        if (spStart == NULL) {
          return ret;
        }
        spStart++;

        //60:<:200:>200:<:200:>:200;

        //检测时间
        spStart = get_short_split(spStart, &nT1, ':');
        if (spStart == NULL) {
          return ret;
        }
        ulActivedCheckTimeInMs = nT1 * 1000;
        DBGPRINTF("\nCheck Times:[%d]", ulActivedCheckTimeInMs);

        //wendu low operator
        spStart = get_char_split(spStart, &chT, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nWenduLow operator:[%c]", chT);
        chWenduLowOperator = chT;

        //wendu low power val
        spStart = get_short_split(spStart, &nT1, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nWenduLow power value:[%d]", nT1);
        nWenduLowPowerVal = nT1;

        //wendu high operator
        spStart = get_char_split(spStart, &chT, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nWenduHigh operator:[%c]", chT);
        chWenduHighOperator = chT;

        //wendu High power val
        spStart = get_short_split(spStart, &nT1, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nWenduHigh power value:[%d]", nT1);
        nWenduHighPowerVal = nT1;

        //Shidu low operator
        spStart = get_char_split(spStart, &chT, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nShiduLow operator:[%c]", chT);
        chShiduLowOperator = chT;

        //shidu low power val
        spStart = get_short_split(spStart, &nT1, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nShiduLow power value:[%d]", nT1);
        nShiduLowPowerVal = nT1;

        //shidu high operator
        spStart = get_char_split(spStart, &chT, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nShiduHigh operator:[%c]", chT);
        chShiduHighOperator = chT;

        //Shidu high power val
        spStart = get_short_split(spStart, &nT1, ':');
        if (spStart == NULL) {
          return ret;
        }
        DBGPRINTF("\nShiduHigh power value:[%d]", nT1);
        nShiduHighPowerVal = nT1;
      }
    }

    //if (wenduEnable || shiduEnable) {}
    
    autoControlEnable=wenduEnable+shiduEnable;
    DBGPRINTF("\nautoControlEnable:[%d]", autoControlEnable);

    ret = true;
    lastCommandType = 0;
    lastCommandCount = 0;
    valid = true;
    bSave_config();

    DBGPRINTF("\n--END--%s", "");
  }
  else {
    //device is not ready
    valid = false;
  }

  return ret;
}

//
bool wenshidu::decode_804_command(char *ptr)
{
  bool ret = false;
  short nT1;
  char *spStart, *spEnd;
  spStart = ptr;
  nT1 = strlen(ptr);
  if (nT1 > 0)
  {
    if (*(spStart + nT1 - 1) == ';') {
      spStart = get_short_split(spStart, &nT1, ';');
    }
    else {
      nT1 = atoi(spStart);

    }
  }
  if (nT1 > 0) {
    ulCheckActionCycleTimeInMS = nT1 * 1000;
    ret = true;
  }
  return ret;
}

char * wenshidu::get_str_split(char *ptr, char **spData, char chSplit)
{
  return get_str_split_with_len(ptr, spData, chSplit, CONST_IRDA_SUB_COMMAND_LENGTH);
}


//乒乓键类型检测,通过功率检测
unsigned short ICACHE_FLASH_ATTR wenshidu::set_curr_power(unsigned short power)
{
  currPower = power;
  return currPower;
}


void ICACHE_FLASH_ATTR wenshidu::debug_print_info()
{
  DBGPRINTLN("\n === wenshidu curr Stat begin ===");
  DBGPRINTF("\n Ready [%d], valid [%d] wendu[%d],shidu[%d] cmdCodeVer[%d]", deviceReady, valid, wenduEnable, shiduEnable, cmdCodeVer);
  DBGPRINTF(" wendu [%d.%d], shidu [%d.%d]", nWenduInt, nWenduDec, nShiduInt, nShiduDec);
  DBGPRINTF("\n WenduLow [%d], WenduHigh [%d]",  nWenduLow, nWenduHigh);
  DBGPRINTF("  ShiduLow [%d], ShiduHigh [%d]",  nShiduLow, nShiduHigh);
  DBGPRINTF("\n WenduLow [%c %d], WenduHigh [%c %d]", chWenduLowOperator, nWenduLowPowerVal, chWenduHighOperator, nWenduHighPowerVal);
  DBGPRINTF("  ShiduLow [%c %d], ShiduHigh [%c %d]", chShiduLowOperator, nShiduLowPowerVal, chShiduHighOperator, nShiduHighPowerVal);
  DBGPRINTLN("\n === wenshidu Curr Stat end === \n");
}

/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR wenshidu::bLoad_config()
{
  bool bRet = false;
  short nLen;
  char *spTemp;
  char *bigBuff;
  //uint8_t bigBuff[CONST_APP_TEMP_BIG_BUFFLEN + 2];
  short nPos;
  bigBuff = dataBuffPtr;

  File configFile = SPIFFS.open(CONST_APP_WENSHIDU_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s", CONST_APP_WENSHIDU_FILE_NAME);
    return bRet;
  }

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuff, CONST_APP_WENSHIDU_FILE_SIZE);
    if (nLen <= 0)
      break;
    bigBuff[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bigBuff, ':');
    //    DBGPRINTF("\nbLoadConfig bigBuff [%s]",bigBuff);
    if (spTemp == NULL)
      break;//not found;
    spTemp++;
    //    DBGPRINTF("\nbLoadConfig spTemp [%s]",spTemp);
    
    //cmdCodeVer
    if (memcmp(bigBuff, "cdv", 3) == 0) {
      cmdCodeVer = atoi(spTemp);
    }

    //wendu
    if (memcmp(bigBuff, "wde", 3) == 0) {
      wenduEnable = atoi(spTemp);
    }
    if (memcmp(bigBuff, "wdl", 3) == 0) {
      nWenduLow = atoi(spTemp);
    }
    if (memcmp(bigBuff, "wdh", 3) == 0) {
      nWenduHigh = atoi(spTemp);
    }
    if (memcmp(bigBuff, "wdc", 3) == 0) {
      nWenduActionLen = atoi(spTemp);
    }

    if (memcmp(bigBuff, "wla", 3) == 0) {
      *(spTemp + 1) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_MAX_IRDA_SUB_COMMAND_LEN)) {
        strncpy(wenduLowActionString[nPos], spTemp + 2, CONST_IRDA_SUB_COMMAND_LENGTH);
      }
    }

    if (memcmp(bigBuff, "wha", 3) == 0) {
      *(spTemp + 1) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_MAX_IRDA_SUB_COMMAND_LEN)) {
        strncpy(wenduHighActionString[nPos], spTemp + 2, CONST_IRDA_SUB_COMMAND_LENGTH);
      }
    }

    //shidu
    if (memcmp(bigBuff, "sde", 3) == 0) {
      shiduEnable = atoi(spTemp);
    }
    if (memcmp(bigBuff, "sdl", 3) == 0) {
      nShiduLow = atoi(spTemp);
    }
    if (memcmp(bigBuff, "sdh", 3) == 0) {
      nShiduHigh = atoi(spTemp);
    }
    if (memcmp(bigBuff, "sdc", 3) == 0) {
      nShiduActionLen = atoi(spTemp);
    }
    //wendu offset
    if (memcmp(bigBuff, "wdf", 3) == 0) {
      nWenduOffset = atoi(spTemp);
    }
    if (memcmp(bigBuff, "sdf", 3) == 0) {
      nShiduOffset = atoi(spTemp);
    }
    //wendu offset
    if (memcmp(bigBuff, "wda", 3) == 0) {
      nWenduAdjust = atoi(spTemp);
    }
    if (memcmp(bigBuff, "sda", 3) == 0) {
      nShiduAdjust = atoi(spTemp);
    }

    
    if (memcmp(bigBuff, "sla", 3) == 0) {
      *(spTemp + 1) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_MAX_IRDA_SUB_COMMAND_LEN)) {
        strncpy(shiduLowActionString[nPos], spTemp + 2, CONST_IRDA_SUB_COMMAND_LENGTH);
      }
    }

    if (memcmp(bigBuff, "sha", 3) == 0) {
      *(spTemp + 1) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_MAX_IRDA_SUB_COMMAND_LEN)) {
        strncpy(shiduHighActionString[nPos], spTemp + 2, CONST_IRDA_SUB_COMMAND_LENGTH);
      }
    }

    //power check data

    if (memcmp(bigBuff, "owl", 3) == 0) {
      chWenduLowOperator = *spTemp;
    }    
    if (memcmp(bigBuff, "owh", 3) == 0) {
      chWenduHighOperator = *spTemp;
    }
    if (memcmp(bigBuff, "osl", 3) == 0) {
      chShiduLowOperator = *spTemp;
    }    
    if (memcmp(bigBuff, "osh", 3) == 0) {
      chShiduHighOperator = *spTemp;
    }
  
    if (memcmp(bigBuff, "vwl", 3) == 0) {
      nWenduLowPowerVal = atoi(spTemp);
    }
    if (memcmp(bigBuff, "vwh", 3) == 0) {
      nWenduHighPowerVal = atoi(spTemp);
    }
    if (memcmp(bigBuff, "vsl", 3) == 0) {
      nShiduLowPowerVal = atoi(spTemp);
    }
    if (memcmp(bigBuff, "vsh", 3) == 0) {
      nShiduHighPowerVal = atoi(spTemp);
    }

    autoControlEnable=wenduEnable+shiduEnable;

    bRet = true;
  }

  // Real world application would store these values in some variables for later use
  DBGPRINTF("\nLoaded cmdCodeVer : [%d]", cmdCodeVer);
  DBGPRINTF("\nLoaded autoControlEnable: [%d]", autoControlEnable);
  DBGPRINTF("\nLoaded wenduEnable: [%d]", wenduEnable);
  DBGPRINTF("\nLoaded wendu low  : [%d]", nWenduLow);
  DBGPRINTF("\nLoaded wendu High : [%d]", nWenduHigh);
  DBGPRINTF("\nLoaded wendu count: [%d]", nWenduActionLen);
  for (nPos = 0; nPos < nWenduActionLen; nPos++) {
    DBGPRINTF("\nLoaded wendu strL : [%s]", wenduLowActionString[nPos]);
  }
  for (nPos = 0; nPos < nWenduActionLen; nPos++) {
    DBGPRINTF("\nLoaded wendu strH : [%s]", wenduHighActionString[nPos]);
  }

  DBGPRINTF("\nLoaded shiduEnable: [%d]", shiduEnable);
  DBGPRINTF("\nLoaded shidu low  : [%d]", nShiduLow);
  DBGPRINTF("\nLoaded shidu High : [%d]", nShiduHigh);
  DBGPRINTF("\nLoaded shidu count: [%d]", nShiduActionLen);
  for (nPos = 0; nPos < nWenduActionLen; nPos++) {
    DBGPRINTF("\nLoaded shidu strL : [%s]", shiduLowActionString[nPos]);
  }
  for (nPos = 0; nPos < nWenduActionLen; nPos++) {
    DBGPRINTF("\nLoaded shidu strH : [%s]", shiduHighActionString[nPos]);
  }
    

  configFile.close();
  DBGPRINTLN("Application Config ok");
  return bRet;
}


/* save application data, return True = success */
bool ICACHE_FLASH_ATTR wenshidu::bSave_config()
{
  char *bigBuff;
  //uint8_t bigBuff[CONST_APP_TEMP_BIG_BUFFLEN + 2];
  short i;
  bigBuff = dataBuffPtr;

  DBGPRINTLN("\n--save application data--");
  File configFile = SPIFFS.open(CONST_APP_WENSHIDU_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_WENSHIDU_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }

  configFile.print("cdv:");
  configFile.println(cmdCodeVer);
  configFile.print("wde:");
  configFile.println(wenduEnable);
  configFile.print("wdl:");
  configFile.println(nWenduLow);
  configFile.print("wdh:");
  configFile.println(nWenduHigh);
  configFile.print("wdc:");
  configFile.println(nWenduActionLen);

  for (i = 0; i < nWenduActionLen; i++) {
    sprintf((char *)bigBuff, "wla:%1d:%s", i, wenduLowActionString[i]);
    configFile.println((char *)bigBuff);
  }
  for (i = 0; i < nWenduActionLen; i++) {
    sprintf((char *)bigBuff, "wha:%1d:%s", i, wenduHighActionString[i]);
    configFile.println((char *)bigBuff);
  }

  configFile.print("sde:");
  configFile.println(shiduEnable);
  configFile.print("sdl:");
  configFile.println(nShiduLow);
  configFile.print("sdh:");
  configFile.println(nShiduHigh);
  configFile.print("sdc:");
  configFile.println(nShiduActionLen);

  for (i = 0; i < nShiduActionLen; i++) {
    sprintf((char *)bigBuff, "sla:%1d:%s", i, shiduLowActionString[i]);
    configFile.println((char *)bigBuff);
  }
  for (i = 0; i < nWenduActionLen; i++) {
    sprintf((char *)bigBuff, "sha:%1d:%s", i, shiduHighActionString[i]);
    configFile.println((char *)bigBuff);
  }
  
  //wendu nWenduOffset
  configFile.print("wdf:");
  configFile.println(nWenduOffset);

  //shidu nShiduOffset
  configFile.print("sdf:");
  configFile.println(nShiduOffset);

  //wendu nWenduOffset
  configFile.print("wda:");
  configFile.println(nWenduAdjust);

  //shidu nShiduOffset
  configFile.print("sda:");
  configFile.println(nShiduAdjust);
  
  //power check data
  configFile.print("owl:");
  configFile.println(chWenduLowOperator);
  configFile.print("owh:");
  configFile.println(chWenduHighOperator);
  configFile.print("osl:");
  configFile.println(chShiduLowOperator);
  configFile.print("osh:");
  configFile.println(chShiduHighOperator);

  configFile.print("vwl:");
  configFile.println(nWenduLowPowerVal);
  configFile.print("vwh:");
  configFile.println(nWenduHighPowerVal);
  configFile.print("vsl:");
  configFile.println(nShiduLowPowerVal);
  configFile.print("vsh:");
  configFile.println(nShiduHighPowerVal);
  

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}

