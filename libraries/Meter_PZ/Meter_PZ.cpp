#include <Meter_PZ.h>

//modified on 2017/4/2 by Steven Lian steven.lian@gmail.com

//static SoftwareSerial meterSerial(CONST_METER_DQ_RX_PORT, CONST_METER_DQ_TX_PORT);//RX,TX

Meter_DQ::Meter_DQ()
{

}

unsigned short Meter_DQ::get_version()
{
  return version;
}


bool Meter_DQ::begin(char *bigBuffPtr)
{
  bool ret = false;
  short nTimes = CONST_APP_METER_DEFAULT_TRY_TIMES;
  version=CONST_METER_PZ_VERSION;
  gnSIOCommandNum=0;
  APP_METER_SERIAL.begin(CONST_APP_METER_SIO_BAUD_DATA);
  while (!APP_METER_SERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  bFirstReadDianliangFlag = true;
  
  dataBuffPtr=bigBuffPtr;

  set_default_ip();
  delay(5);
  
  while (nTimes > 0) {
    delay(1);
    deviceReady = is_meter_exist();
    if (deviceReady) {
      break;
    }
    nTimes--;
  }
  return ret;
}

bool Meter_DQ::devReady()
{
  return deviceReady;
}

bool Meter_DQ::is_meter_exist()
{
  bool ret = false;
  //wait for 1 seconds to get data;
  unsigned long ulTick;
  short nLen = 0;
  //vReset_interval(ulTick);
  ulTick = ulReset_interval();
  send_cmd_to_device(CONST_APP_METER_DIANYA_FLAG);
  delay(5);

  DBGPRINTLN("wait for receive:");
  while (ulGet_interval(ulTick) < 1000) {
    delay(1);
    if (available()) {
      ret = true;
      break;
    }
  }
  DBGPRINTLN("\n is_meter_exist");
  return ret;
}

bool Meter_DQ::available()
{
  bool ret = false;
  // receive the SIO data from meter
  while (APP_METER_SERIAL.available()) {
    // DBGPRINTLN("APP_METER_SERIAL.available");
    recvBuff[recvBuffLen++] = APP_METER_SERIAL.read();
    if (recvBuffLen == 1) //to determine if the data is correct.
    {
      switch (recvBuff[0])
      {
        case CONST_APP_METER_DIANYA_RECE:
        case CONST_APP_METER_DIANLIU_RECE:
        case CONST_APP_METER_GONGLV_RECE:
        case CONST_APP_METER_DIANLIANG_RECE:
        case CONST_APP_METER_POWERFACTOR_RECE:
        case CONST_APP_METER_FREQUENCY_RECE:
        case CONST_APP_METER_ZERO_RECE:
        case CONST_APP_METER_SETIP_RECE:
          break;
        default:
          // the first byte is wrong, reset;
          recvBuffLen = 0;
      }
    }
    if (recvBuffLen == CONST_APP_METER_SIO_DATA_LEN) {
      decode_CMD(recvBuff);
      recvBuffLen = 0;
      ret = true;
      break;
    } 
  }
  return ret;
}

void Meter_DQ::clean_SIO()
{
  //clean SIO data
  while (APP_METER_SERIAL.available()) {
    APP_METER_SERIAL.read();
  }
  recvBuffLen = 0;
    
  delay(2);

}

bool Meter_DQ::read()
{
  bool ret = false;
  uint8_t nCMD;

  ret = available();

  //send request
  if ((ulGet_interval(ulSIOQueryTick) > CONST_APP_METER_SIO_QUERY_INTERVAL))
  {
    // 2017/3/25 end

    switch (gnSIOCommandNum)
    {
      case 0:
      //dianya
        nCMD = CONST_APP_METER_DIANYA_FLAG;
        break;

      case 1:
      case 4:
      //dianliu
        nCMD = CONST_APP_METER_DIANLIU_FLAG;
        break;

      case 2:
      case 5:
      //gonglv
        nCMD = CONST_APP_METER_GONGLV_FLAG;
        break;

      case 3:
      //DIANLIANG
        nCMD = CONST_APP_METER_DIANLIANG_FLAG;
        break;
      default:
        nCMD = CONST_APP_METER_GONGLV_FLAG;
        break;

    }
    
    //Serial.printf("\n gnSIOCommandNum [%d] nCMD [%d]",gnSIOCommandNum, nCMD);
    
    send_cmd_to_device(nCMD);

    gnSIOCommandNum++;
    if (gnSIOCommandNum >= CONST_APP_METER_SIO_COMMAND_LOOP)
    {
      gnSIOCommandNum = 0;
    }
    ulSIOQueryTick = ulReset_interval();
  }

  return ret;

}


uint8_t Meter_DQ::check_sum(uint8_t *pBuff, int nLen)
{
  uint8_t i, val = 0;
  for (i = 0; i < nLen; i++) {
    val += pBuff[i];
  }
  return (val & 0xFF);
}


void Meter_DQ::encode_CMD(int nType, uint8_t *pBuff)
{
  char chSum = 0;
  if (nType < 0 || nType >= CONST_APP_METER_SIO_COMMAND_COUNT)
    nType = 0;

  pBuff[0] = 0xB0+nType;
  pBuff[1] = 0xC0; //192.168.1.1
  pBuff[2] = 0xA8;
  pBuff[3] = 0x01;
  pBuff[4] = 0x01;
  pBuff[5] = 0x00;
  pBuff[6] = check_sum(pBuff, CONST_APP_METER_SIO_DATA_LEN - 1);
};

void Meter_DQ::decode_CMD(uint8_t data[])
{
  unsigned short i, nD0, nD1, nD2, nD3;
  long lData, lMeter;
  uint8_t checkSum;
  //DBGPRINTLN("decode_CMD BEGIN");
  checkSum = check_sum(data, CONST_APP_METER_SIO_DATA_LEN - 1);
  if (checkSum == data[CONST_APP_METER_SIO_DATA_LEN - 1])
  {
    nD0 = (short) data[0];
    nD1 = (short) data[1];
    nD2 = (short) data[2];
    nD3 = (short) data[3];

    switch (nD0)
    {
      case CONST_APP_METER_DIANYA_RECE:
        //电压 个位数1位 %d.%d
        nWDianyaInt = nDianyaInt;
        nDianyaInt = (nD1 << 8 ) + nD2;
        nDianyaDec = nD3;
        break;
      case CONST_APP_METER_DIANLIU_RECE:
        //电流 个位数2位 %d.%2d
        if(CONST_APP_METER_POWER_RATIO>1){
          //>1 需要放大
          nD1=nD3*CONST_APP_METER_POWER_RATIO;
          nD3=nD1 % 100;
          nD0=nD1 / 100;
          nD2=nD2*CONST_APP_METER_POWER_RATIO+nD0;
        }
        if(CONST_APP_METER_POWER_RATIO<0){
          //<0，需要缩小
          nD0 = abs(CONST_APP_METER_POWER_RATIO);
          nD1=nD2 % nD0;
          nD3=nD3 / nD0 + nD1*(100/nD0);
          nD2=nD2 / nD0;
        }
        nDianliuInt = nD2;
        nDianliuDec = nD3;
        break;
      case CONST_APP_METER_GONGLV_RECE:
        //功率 个位数0位
        nWGonglv = nGonglv;
        nGonglv = (nD1 << 8) + nD2;

        if(CONST_APP_METER_POWER_RATIO>1){
          //>1 需要放大
          nGonglv=nGonglv*CONST_APP_METER_POWER_RATIO;
        }
        if(CONST_APP_METER_POWER_RATIO<0){
          //<0，需要缩小
          nGonglv=nGonglv/abs(CONST_APP_METER_POWER_RATIO);
        }

        anLastGonglv[nLastGonglvPos] = nGonglv;
        nLastGonglvPos++;
        if (nLastGonglvPos > CONST_APP_METER_GONGLV_LAST_MAX_LEN) {
          nLastGonglvPos = 0;
        }
        cal_stdev();

        break;
      case CONST_APP_METER_DIANLIANG_RECE:
        //电量 个位数0位
        lMeter = (((long) nD1) << 16) + (((long) nD2) << 8) + (long) nD3;

        if(CONST_APP_METER_POWER_RATIO>1){
          //>1 需要放大
          lMeter =lMeter*CONST_APP_METER_POWER_RATIO;
        }
        if(CONST_APP_METER_POWER_RATIO<0){
          //<0，需要缩小
          lMeter = lMeter/abs(CONST_APP_METER_POWER_RATIO);
        }
        
        if (lMeter==0){
          //避免串口读数错误
          lMeter=lLastMeterDianliang;
        }
        
        lCurrMeterDianliang = lMeter;
        if (bFirstReadDianliangFlag)
        {
          bFirstReadDianliangFlag = false;
          lMeterDianliang = lMeter;
          lLastMeterDianliang = lMeterDianliang;
          lYMeterDianliang = lMeterDianliang;
          break;
        }

        // the following code is to prevent the data error BEGIN
        lData = lMeter - lLastMeterDianliang;
        if (lData < 0) {
          lData += 0xFFFFFF;
        }
        if (lData >= CONST_APP_METER_SIO_DIANLIANG_MAX_ERROR)
        {
          //data error
          failCount ++;
          failTotal ++;
          break;
        }
        //end
        failCount = 0;
        lData = lMeter - lYMeterDianliang;
        if (lData < 0) {
          lData += 0xFFFFFF;
        }
        lLastMeterDianliang = lMeterDianliang;
        lMeterDianliang = lMeter;

        //calculate the right dianliang
        nDianliangInt = (short) (lData / 1000);
        nDianliangDec = (short) (lData % 1000);
        break;
    }
  }

  if (!bFirstReadDianliangFlag)
  {
    if (failCount > CONST_APP_METER_SIO_ERROR_COUNT)
    {
      lYMeterDianliang = lMeterDianliang;
      failCount = 0;
      //ESP.restart();
    }
  }
  //DBGPRINTLN("decode_CMD END");
}

//原方案计算功率均方差，目的是找出设备的稳定工作频率
//改进方案，计算超出10%平均值的功率个数，默认不超过2个（包括），这个功率值就是稳定功率值
void Meter_DQ::cal_stdev()
{
  short i;
  short avg;
  unsigned long sum;
  short val;
  sum=0;
  for (i=0;i<CONST_APP_METER_GONGLV_LAST_MAX_LEN;i++)
  {
    sum+=(anLastGonglv[i]+CONST_AIR_CONDITION_GONGLV_DIV_VAL-1);
  }
  avg=sum/CONST_APP_METER_GONGLV_LAST_MAX_LEN;
  stAvg=avg;
  /* 原方案计算功率均方差，目的是找出设备的稳定工作频率
  avg/=CONST_AIR_CONDITION_GONGLV_DIV_VAL*CONST_AIR_CONDITION_GONGLV_EXT_VAL;
  if (avg>0)
  {
    sum=0;
    for (i=0;i<CONST_APP_METER_GONGLV_LAST_MAX_LEN;i++)
    {
      val=(anLastGonglv[i]+CONST_AIR_CONDITION_GONGLV_DIV_VAL-1)/CONST_AIR_CONDITION_GONGLV_DIV_VAL*CONST_AIR_CONDITION_GONGLV_EXT_VAL;
      sum+=(val-avg)*(val-avg);
    }
    stdev=sqrt((double)sum);    
  }
  */
  //改进方案，计算超出10%平均值的功率个数，默认不超过2个（包括），这个功率值就是稳定功率值
  val=avg/CONST_APP_STDEV_DIVIDE;
  stdev=0;
  for (i=0;i<CONST_APP_METER_GONGLV_LAST_MAX_LEN;i++)
  {
    if (abs(anLastGonglv[i]-avg)>val){
      stdev++;
    }
  }
  
}


void Meter_DQ::send_cmd_to_device(uint8_t nType)
{
  short i;
  encode_CMD(nType, sendBuff);

  clean_SIO();

  for (i = 0; i < CONST_APP_METER_SIO_DATA_LEN; i++) {
    //mySerial.write(sendBuff[i]);
    //Serial.write(sendBuff[i]);
    APP_METER_SERIAL.write(sendBuff[i]);
    //DBGPRINTF("%02X ", sendBuff[i]);

  }
}

void Meter_DQ::set_default_ip()
{
  send_cmd_to_device(CONST_APP_METER_SETIP_FLAG);
}

void Meter_DQ::set_daingliang_zero()
{
  send_cmd_to_device(CONST_APP_METER_ZERO_FLAG);
}



/* load application data, return True==Success*/
bool Meter_DQ::bLoad_config()
{
  bool ret = false;
  int nLen;
  char *spTemp;
  
  char *dataPtr;
  dataPtr=dataBuffPtr;
  if (dataPtr==NULL)
    return ret;

  File configFile = SPIFFS.open(CONST_APP_METER_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s",CONST_APP_METER_FILE_NAME);
    return ret;
  }
  
  //determine if the right config file
  while (true)
  {
    nLen = configFile.readBytesUntil('\n', dataPtr, CONST_APP_METER_BIG_BUFF_SIZE);
    if (nLen <= 0)
      break;
    dataPtr[nLen - 1] = '\0'; //trim
    spTemp = strchr(dataPtr, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;
    if (memcmp(dataPtr, "YMeterDianliang", 15) == 0)
    {
      //gsCurrent.lYMeterDianliang = atol(spTemp);
    }
  }
  
  // Real world application would store these values in some variables for later use
  DBGPRINT("Loaded YMeterDianliang: ");
  DBGPRINT("[" );
  DBGPRINT( lYMeterDianliang );
  DBGPRINTLN("]");
  configFile.close();
  DBGPRINTLN("Application Config ok");

  return ret;
}

/* save application data, return True = success */
bool Meter_DQ::bSave_config()
{
  bool ret=false;
  char *dataPtr;
  dataPtr=dataBuffPtr;
  if (dataPtr==NULL)
    return ret;

  DBGPRINTLN("--save application data--");
  File configFile = SPIFFS.open(CONST_APP_METER_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_METER_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }

  configFile.print("YMeterDianliang:");
  configFile.println(lYMeterDianliang);

  configFile.close();
  DBGPRINTLN(" -- end");
  return ret;
}


