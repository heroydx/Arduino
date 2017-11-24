#include "Tower_FSUAir.h"

//modified on 2017/11/14 by Steven Lian steven.lian@gmail.com

static SoftwareSerial SerialFSU(CONST_FSU_RX_PORT, CONST_FSU_TX_PORT); //
//ListArray gsFMDataList(CONST_APP_FSU_RECV_COMMAND_BUFF_LEN, sizeof(FMCommandUnion));



TT_FSU::TT_FSU()
{
}

bool TT_FSU::begin(uint8_t addr,uint8_t *bufPtr1,uint8_t *bufPrt2)
{
  bool ret=false;
  ADR = addr;
  bigBuffPtr = bufPtr1;
  commBuffPtr = bufPrt2;
  
  
  bStatChangeFlag=false;
  //  Serial.begin(CONST_APP_FSU_RECV_SIO_BAUD_DATA);
  APP_SERIAL.begin(CONST_APP_FSU_RECV_SIO_BAUD_DATA);
  while (!APP_SERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  };
  DBGPRINTLN("\n\n");
  DBGPRINTLN("--FSU INIT BEGIN--");


  short nTimes = 5;
  while (nTimes > 0) {
    delay(1);
    deviceReady = is_device_exist();
    if (deviceReady) {
      DBGPRINTLN("FSU is ready!!!");
      break;
    }
    nTimes--;
  }

  if (!bLoad_config()) {
    currStat.currMeter = 0;
    currStat.yesterdayMeter = 0;
    currStat.wendu = 0;
    currStat.shidu = 0;
    currStat.airSwitch = 0;
  }

  restore_setting();
  return ret;

}


bool TT_FSU::is_device_exist()
{
  bool ret = true;
  return ret;
}


bool TT_FSU::devReady()
{
  return deviceReady;
}



bool TT_FSU::read_config()
{
  bool ret = true;
  //准备并发送查询状态数据
  return ret;
}

void ICACHE_FLASH_ATTR TT_FSU::restore_setting()
{
}

void ICACHE_FLASH_ATTR TT_FSU::handle_SIO_reply_data(uint8_t *param)
{

}


short ICACHE_FLASH_ATTR TT_FSU::available()
{
  short ret=0;
  if (APP_SERIAL.available()) {
    uint8_t chR;
    chR = APP_SERIAL.read();
    //DBGPRINT(chR);
    au8recvBuff[nRecvLen++] = chR;
    //判断是否是开始
    if ((chR==CONST_APP_FSU_RECV_SIO_DATA_BEGIN)
      ||(nRecvLen > CONST_APP_FSU_RECV_SENDREV_BUFFLEN)) {
      nRecvLen = 0;
    }
    //判断是否是结束
    if (chR == CONST_APP_FSU_RECV_SIO_DATA_END) {
      //got respose/reply data;
      au8recvBuff[nRecvLen++] = '\0';
      bSendBusyFlag = false;
      DBGPRINTLN("**--Receive SIO data--**");
      DBGPRINTF("[%s]", au8recvBuff);
      ret=nRecvLen;
      delay(1);
      handle_SIO_reply_data(au8recvBuff);
      nRecvLen=0;
    }
  }
  delay(1);
}

void ICACHE_FLASH_ATTR TT_FSU::send_CMD_to_FSU(short nLen, char *dataPtr)
{
  int i;
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(dataPtr[i]);
  }
}


/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR TT_FSU::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;

  File configFile = SPIFFS.open(CONST_APP_FSU_RECV_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s",CONST_APP_FSU_RECV_FILE_NAME);
    return bRet;
  }

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuffPtr, CONST_APP_FSU_RECV_BIG_BUFFLEN);
    if (nLen <= 0)
      break;
    bigBuffPtr[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bigBuffPtr, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bigBuffPtr, "ADR", 3) == 0) {
      ADR = atoi(spTemp);
    }
  }
  
  // Real world application would store these values in some variables for later use
  DBGPRINT("Loaded stat: ");
  DBGPRINT("[" );
  DBGPRINT( ADR );
  DBGPRINTLN("]");
  
  configFile.close();
  DBGPRINTLN("Application Config ok");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR TT_FSU::bSave_config()
{
  
  uint8_t bigBuffPtr[CONST_APP_FSU_RECV_BIG_BUFFLEN + 2];

  DBGPRINTLN("--save FSU config data--");
  File configFile = SPIFFS.open(CONST_APP_FSU_RECV_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_FSU_RECV_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTF("Failed to open config file :%s",CONST_APP_FSU_RECV_FILE_NAME);
      return false;
    }
  }

  configFile.print("ADR:");
  configFile.println(ADR);

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}


void TT_FSU::debug_print_info()
{
}
