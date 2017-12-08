#include <FM_ZCJ.h>

//modified on 2017/4/2 by Steven Lian steven.lian@gmail.com

SoftwareSerial SerialFM(CONST_FM_ZCJ_RX_PORT, CONST_FM_ZCJ_TX_PORT); // RX, TX 8266 GPIO 4,2
//ListArray gsFMDataList(CONST_APP_FM_RECV_COMMAND_BUFF_LEN, sizeof(FMCommandUnion));

FMCommand gsFMDataList;

FMCommand::FMCommand()
{
  memset(gasCommand, 0, (sizeof(stFMCommand) * CONST_APP_FM_RECV_COMMAND_BUFF_LEN));
  gu8CommandPos = 0;
};

uint8_t ICACHE_FLASH_ATTR FMCommand::lpush(short command, short param, uint8_t flag)
{
  uint8_t ret;
  if (gu8CommandPos < (CONST_APP_FM_RECV_COMMAND_BUFF_LEN - 1)) {
    gasCommand[gu8CommandPos].command = command;
    gasCommand[gu8CommandPos].param = param;
    gu8CommandPos++;
    ret = gu8CommandPos;
  }
  else {
    //full
    uint8_t ret = -1;
  }
  return ret;
}


uint8_t ICACHE_FLASH_ATTR FMCommand::push(short command, short param, uint8_t flag)
{
   return lpush(command,param,flag);
}


uint8_t ICACHE_FLASH_ATTR FMCommand::rpop(stFMCommand *spCommand)
{
  uint8_t ret;
  stFMCommand *spTemp;
  spTemp = gasCommand;
  if (gu8CommandPos > 0) {
    spCommand->command = spTemp->command;
    spCommand->param = spTemp->param;
    gu8CommandPos--;
    memcpy(spTemp, (spTemp + 1), (sizeof(stFMCommand) * gu8CommandPos));
    ret = gu8CommandPos;
  }
  else {
    //no command
    uint8_t ret = -1;
  }
  return ret;
}


uint8_t ICACHE_FLASH_ATTR FMCommand::pop(stFMCommand *spCommand)
{
  return rpop(spCommand);
}

uint8_t ICACHE_FLASH_ATTR FMCommand::available()
{
  return gu8CommandPos;
}


FM_ZCJ::FM_ZCJ()
{
  if (!bLoad_config()) {
    u8Stat = CONST_APP_FM_RECV_DEFAULT_STAT;
    nFrequency = CONST_APP_FM_RECV_DEFAULT_FREQUENCY;
    u8Vol = CONST_APP_FM_RECV_DEFAULT_VOL;
    u8Campus = CONST_APP_FM_RECV_DEFAULT_SN_CAMPUS;
    u8Blk = CONST_APP_FM_RECV_DEFAULT_BLK;
    u8SN_THR = CONST_APP_FM_RECV_DEFAULT_SN_THR;
    u8DSP = CONST_APP_FM_RECV_DEFAULT_SN_DSP;
  }
  restore_setting();
}

bool FM_ZCJ::begin()
{
  //  Serial.begin(CONST_APP_FM_RECV_SIO_BAUD_DATA);
  APP_SERIAL.begin(CONST_APP_FM_RECV_SIO_BAUD_DATA);
  while (!APP_SERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  };
  DBGPRINTLN("\n\n");
  DBGPRINTLN("--FM RADIO INIT BEGIN--");
  /*
    if (read_config()) {
    DBGPRINTLN("FMRADIO is ready!!!");
    }
  */
}

bool FM_ZCJ::read_config()
{
  bool ret = false;
  short i;
  short nLen;
  //准备并发送查询状态数据
  prepare_FM_CMD(FM_STATUS, 0 , (char *) au8sendBuff);
  nLen = strlen((char *) au8sendBuff);

  DBGPRINTF("len:%d, send:%s", nLen, (char *) au8sendBuff);
  APP_SERIAL.write('\n');
  delay(1);
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(au8sendBuff[i]);
  }

  //wait for 2 seconds to get data;
  unsigned long ulTick;
  //vReset_interval(ulTick);
  ulTick = ulReset_interval();
  DBGPRINTLN("wait for receive:");
  u8Campus = 20;
  while (ulGet_interval(ulTick) < 2000) {
    //DBGPRINT("fc");
    check_FM_reply();
    if ((u8Campus == 0) || (u8Campus == 1)) {
      ret = true;
      break;
    }
    delay(5);
  }
  DBGPRINTLN("");
  DBGPRINTLN("finished");
  return ret;
}


short ICACHE_FLASH_ATTR FM_ZCJ::freq_curr()
{
  return anSaveFreq[u8freqPos];
}

short ICACHE_FLASH_ATTR FM_ZCJ::freq_next()
{
  short ret;
  short i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    u8freqPos++;
    if (u8freqPos >= CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS) {
      u8freqPos = 0;
    }
    if (anSaveFreq[u8freqPos] > 0) {
      ret = anSaveFreq[u8freqPos];
      break;
    }
  };
  return ret;
}

short ICACHE_FLASH_ATTR FM_ZCJ::freq_prev()
{
  short ret;
  short i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    u8freqPos--;
    if (u8freqPos < 0 ) {
      u8freqPos = CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS;
    }
    if (anSaveFreq[u8freqPos] > 0) {
      ret = anSaveFreq[u8freqPos];
      break;
    }
  };
  return ret;
}

uint8_t ICACHE_FLASH_ATTR FM_ZCJ::freq_add(short freq)
{
  short i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    u8freqPos++;
    if (u8freqPos >= CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS) {
      u8freqPos = 0;
    }
    if (anSaveFreq[u8freqPos] == 0) {
      break;//find a empty position
    }
  };
  anSaveFreq[u8freqPos] = freq;
  return u8freqPos;
}

uint8_t ICACHE_FLASH_ATTR FM_ZCJ::freq_del()
{
  short i;
  anSaveFreq[u8freqPos] = 0;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[i] > 0) {
      u8freqPos = i;
      break;
    }
  }
  freq_num();
  return u8freqPos;
}

uint8_t ICACHE_FLASH_ATTR FM_ZCJ::freq_del(short freq)
{
  short i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[i] == freq) {
      anSaveFreq[i] = 0;
    }
  }
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[i] > 0) {
      u8freqPos = i;
      break;
    }
  }
  freq_num();
  return u8freqPos;
}

uint8_t ICACHE_FLASH_ATTR FM_ZCJ::freq_clear()
{
  short i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    anSaveFreq[i] = 0;
  }
  return freq_num();
}

uint8_t ICACHE_FLASH_ATTR FM_ZCJ::freq_num()
{
  short i;
  u8freqNum = 0;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[u8freqPos] > 0) {
      u8freqNum++;
    }
  };
  return u8freqNum;
  ;
}
//为了简化设计,考虑到数组较短,采用简单排序方式
uint8_t ICACHE_FLASH_ATTR FM_ZCJ::freq_sort(int8_t softFlag)
{
  uint8_t i, j;
  short nT1;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    for (j = i; j < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; j++) {
      nT1 = anSaveFreq[i];
      //去掉重复数据
      if (i != j) {
        if (anSaveFreq[i] == anSaveFreq[j]) {
          anSaveFreq[j] = 0;
        }
      }
      //从大到小排序
      if (softFlag > 0) {
        if (anSaveFreq[i] < anSaveFreq[j]) {
          nT1 = anSaveFreq[j];
          anSaveFreq[j] = anSaveFreq[i];
          anSaveFreq[i] = nT1;
        }
      }
      else {
        //从小到大排序
        if (anSaveFreq[i] > anSaveFreq[j]) {
          nT1 = anSaveFreq[j];
          anSaveFreq[j] = anSaveFreq[i];
          anSaveFreq[i] = nT1;
        }
      }
    }
  }
  DBGPRINTLN("-- -sort-- -function-- -");
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    DBGPRINTF(" % 6d", anSaveFreq[i]);
  }
  DBGPRINTLN("\n-- -");
  if (softFlag <= 0) {
    j = 0;
    for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
      if (anSaveFreq[i] == 0) {
        j++;
      }
      else {
        break;
      }
    }
    DBGPRINTF("\n j: [ % d]\n", j);
    //move data
    for (i = 0; i < (CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS - j); i++)
    {
      anSaveFreq[i] = anSaveFreq[i + j];
    }
    //clean restdata
    for (i = 0; i < j; i++)
    {
      anSaveFreq[i + CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS - j] = 0;
    }
  }
  DBGPRINTLN("-- -sort-- -function-- -");
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    DBGPRINTF(" % 6d", anSaveFreq[i]);
  }
  DBGPRINTLN("\n-- -");
  u8freqPos = 0;
  return freq_num();
}

bool ICACHE_FLASH_ATTR FM_ZCJ::freq_set_send()
{
  bSendSaveFreq = true;
  return bSendSaveFreq;
}


bool ICACHE_FLASH_ATTR FM_ZCJ::freq_clear_send()
{
  bSendSaveFreq = false;
  return bSendSaveFreq;
}

void ICACHE_FLASH_ATTR FM_ZCJ::restore_setting()
{
  /*
  allCommand.CMD.command = FM_PLAY_PAUSE;
  allCommand.CMD.param = FM_PLAY_PAUSE;
  allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
  gsFMDataList.lpush(allCommand.u8Data);

  allCommand.CMD.command = FM_SET_FRE;
  allCommand.CMD.param = nFrequency;
  allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
  gsFMDataList.lpush(allCommand.u8Data);

  allCommand.CMD.command = FM_SET_VOL;
  allCommand.CMD.param = u8Vol;
  allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
  gsFMDataList.lpush(allCommand.u8Data);

  allCommand.CMD.command = FM_SET_CAMPUS;
  allCommand.CMD.param = u8Campus;
  allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
  gsFMDataList.lpush(allCommand.u8Data);
  
  allCommand.CMD.command = FM_STATUS;
  allCommand.CMD.param = 0;
  allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
  gsFMDataList.lpush(allCommand.u8Data);
  */
  
  gsFMDataList.push(FM_SET_FRE, nFrequency, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_VOL, u8Vol, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_CAMPUS, u8Campus, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_STATUS, 0, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  
}

void ICACHE_FLASH_ATTR FM_ZCJ::handle_FM_replay_data(char *param)
{
  int nLen;
  char *spTemp;
  spTemp = param;
  nLen = strlen(spTemp);
  DBGPRINTF("\n**-- in Handle Replay Data: % d  freeheap: % d--**", nLen, ESP.getFreeHeap());
  if (nLen < 1) {
    return;
  }
  *(spTemp + nLen - 1) = '\0';

  if (memcmp(spTemp, "ERR", 3) == 0)
  {
    //error
    errCount ++;
  }
  //stat on/off
  else if (memcmp(spTemp, "PAUS", 4) == 0)  {
    u8Stat = 0;
    errCount = 0;
  }
  else if (memcmp(spTemp, "PLAY", 4) == 0) {
    u8Stat = 1;
    errCount = 0;
  }
  //frequency
  else if ((memcmp(spTemp, "FRE = ", 4) == 0) && (nLen > 4)) {
    short nT1;
    nT1 = atoi(spTemp + 4);
    if ((nT1 >= nFreqLow) && (nT1 <= nFreqHigh)) {
      nFrequency = nT1;
    }
    errCount = 0;
  }
  //volume
  else if ((memcmp(spTemp, "VOL = ", 4) == 0) && (nLen > 4)) {
    short nT1;
    nT1 = atoi(spTemp + 4);
    if ((nT1 >= CONST_APP_FM_RECV_MIN_VOL) && (nT1 <= CONST_APP_FM_RECV_MAX_VOL)) {
      u8Vol = nT1;
      //DBGPRINTF("\n--Convert VOL: % d  % d\n", nT1, gsCurrent.u8Vol);
    }
    errCount = 0;
  }
  //backgroud light  BANK=04s\n
  else if ((memcmp(spTemp, "BANK = ", 5) == 0) && (nLen > 5)) {
    short nT1;
    *(spTemp + 7) = '\0';
    nT1 = atoi(spTemp + 5);
    DBGPRINTLN(nT1);
    if ((nT1 >= CONST_APP_FM_RECV_MIN_BLK) && (nT1 <= CONST_APP_FM_RECV_MAX_BLK)) {
      u8Blk = nT1;
    }
    errCount = 0;
  }
  //Campus
  else if ((memcmp(spTemp, "CAMPUS_ON", 9) == 0) || (memcmp(spTemp, "CAMPOS_ON", 9) == 0)) {
    u8Campus = 1;
    errCount = 0;
  }
  else if ((memcmp(spTemp, "CAMPUS_OFF", 10) == 0) || (memcmp(spTemp, "CAMPOS_OFF", 9) == 0)) {
    u8Campus = 0;
    errCount = 0;
  }
  //DSP SN on/off
  else if (memcmp(spTemp, "SN_OFF", 6) == 0) {
    u8DSP = 0;
    errCount = 0;
  }
  else if (memcmp(spTemp, "SN_ON", 5) == 0) {
    u8DSP = 1;
    errCount = 0;
  }
  //DSP threold
  else if ((memcmp(spTemp, "SN_THR = ", 7) == 0) && (nLen > 7)) {
    short nT1;
    nT1 = atoi(spTemp + 7);
    if ((nT1 >= CONST_APP_FM_RECV_SN_MIN_THR) && (nT1 <= CONST_APP_FM_RECV_SN_MAX_THR)) {
      u8SN_THR = nT1;
    }
    errCount = 0;
  }
  //reset response
  else if (memcmp(spTemp, "OK", 2) == 0) {
    errCount = 0;
  }
  //RM radio device ver:PCB_NUMBE:LCD_FM_RX_ENC_V1.9
  else if (memcmp(spTemp, "PCB_NUM", 7) == 0) {
    strncpy(achFMDeviceVer, spTemp, CONST_APP_FM_RECV_FM_DEVICE_VER_BUFFLEN);
    errCount = 0;
  }
  //status query header/tailer
  else if (memcmp(spTemp, "/***", 4) == 0) {
    errCount = 0;
  }
  //status query tailer
  else if (memcmp(spTemp, "Thank", 5) == 0) {
    errCount = 0;
  }
  else {
    errCount++;
  }
  DBGPRINTF("\n**-- in Handle Replay Data END: %d--**", nLen);
  encode_stat_string();
}


void ICACHE_FLASH_ATTR FM_ZCJ::check_FM_reply()
{
  if (APP_SERIAL.available()) {
    uint8_t chR;
    chR = APP_SERIAL.read();
    //DBGPRINT(chR);
    au8recvBuff[u8recvLen++] = chR;
    if (u8recvLen > CONST_APP_FM_RECV_SENDREV_BUFFLEN) {
      u8recvLen = 0;
    }
    if (chR == '\n') {
      //got respose/reply data;
      au8recvBuff[u8recvLen++] = '\0';
      bSendBusyFlag = false;
      DBGPRINTLN("**--Receive SIO data--**");
      DBGPRINTF("[%s]", au8recvBuff);
      strcpy(achFMReply, (char *) au8recvBuff);
      u8recvLen = 0;
      handle_FM_replay_data(achFMReply);
    }
  }
}


void ICACHE_FLASH_ATTR FM_ZCJ::push_CMD_to_FM_queue()
{
  // regular check FM radio machine status
  /*
  allCommand.CMD.command = FM_STATUS;
  allCommand.CMD.param = 0;
  allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
  gsFMDataList.lpush(allCommand.u8Data);
  */
  gsFMDataList.push(FM_STATUS, 0, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);

}


void ICACHE_FLASH_ATTR FM_ZCJ::prepare_FM_CMD(short nType, short nParam, char *pBuff)
{
  switch (nType)
  {
    case  FM_SET_FRE:
      sprintf(pBuff, "AT+FRE=%d\n", nParam);
      break;
    case  FM_FRE_DOWN:
      strcpy(pBuff, "AT+FRED\n");
      break;
    case  FM_FRE_UP:
      strcpy(pBuff, "AT+FREU\n");
      break;
    case  FM_PLAY_PAUSE:
      strcpy(pBuff, "AT+PAUS\n");
      break;
    case  FM_SET_VOL:
      sprintf(pBuff, "AT+VOL=%02d\n", nParam);
      break;
    case  FM_VOL_DOWN:
      strcpy(pBuff, "AT+VOLD\n");
      break;
    case  FM_VOL_UP:
      strcpy(pBuff, "AT+VOLU\n");
      break;
    case  FM_SET_BANK:
      sprintf(pBuff, "AT+BANK=%02d\n", nParam);
      break;
    case  FM_SET_CAMPUS:
      sprintf(pBuff, "AT+CAMPUS=%d\n", nParam);
      break;
    case  FM_SET_DSP:
      sprintf(pBuff, "AT+SN=%d\n", nParam);
      break;
    case  FM_SET_SN_THR:
      sprintf(pBuff, "AT+SN_THR=%02d\n", nParam);
      break;
    case  FM_RESET:
      strcpy(pBuff, "AT+CR\n");
      break;
    case  FM_STATUS:
      strcpy(pBuff, "AT+RET\n");
      break;
    default:
      strcpy(pBuff, "");
      break;
  }

};


void ICACHE_FLASH_ATTR FM_ZCJ::send_CMD_to_FM(short nType, short nParam)
{
  int i;
  int nLen;

  prepare_FM_CMD(nType, nParam, (char *) au8sendBuff);
  nLen = strlen((char *) au8sendBuff);
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(au8sendBuff[i]);
  }
  bSendBusyFlag = true;
#ifdef DEBUG_SIO
  DBGPRINT("\nSend:");
  DBGPRINT((char *)au8sendBuff);
  DBGPRINTLN("------------------");
#endif
}


void ICACHE_FLASH_ATTR FM_ZCJ::handle_Server_to_FM_command(char achParam[])
{
  short nCMDType;
  short nCMDVal;
  char *spTemp;
  spTemp = strchr(achParam, ':');
  if (spTemp == NULL) {
    return;
  }
  DBGPRINTF("\n-- %s %s---\n", achParam, spTemp);
  *spTemp = '\0';
  spTemp++;
  nCMDType = (short) atoi(achParam);
  if (nCMDType!=1){
    //格式 0:1 or 8:1039
     nCMDVal = (short) atoi(spTemp); 
  }
  else{
    //格式 1:A:1B:4B0-->
    
  }

  short nFMCommand = -1;
  short nFMParam = 0;
  //bool bIsFMRadioCommand = true;
  
  switch (nCMDType) {
    case 0:
    //开关命令
      //on off handle
      if (u8Stat != nCMDVal) {
        nFMCommand = FM_PLAY_PAUSE;
      }
      break;
    case 1:
    //状态和直接信息设置（16进制BCD)
      break;
    case 2:
    //设置参数命令
      switch (nCMDVal)
      {
        case 0:
          //reset FM device
          nFMCommand = FM_RESET;
          break;
        case 1:
          //query FM device status
          nFMCommand = FM_STATUS;
          break;
        case 2:
          //campuse on
          nFMCommand = FM_SET_CAMPUS;
          nFMParam = 1;
          break;
        case 3:
          //campus off
          nFMCommand = FM_SET_CAMPUS;
          nFMParam = 0;
          break;
        case 4:
          //DSP SN ON
          nFMCommand = FM_SET_DSP;
          nFMParam = 1;
          break;
        case 5:
          //DSP SN OFF
          nFMCommand = FM_SET_DSP;
          nFMParam = 0;
          break;
        default:
          break;
      }
      break;
    case 3:
    //音量加减
      switch (nCMDVal)
      {
        case 0:
          nFMCommand = FM_VOL_DOWN;
          break;
        case 1:
          nFMCommand = FM_VOL_UP;
          break;
      }
      break;
    case 4:
    //音量直接设置
      if (nCMDVal >= CONST_APP_FM_RECV_MIN_VOL && nCMDVal <= CONST_APP_FM_RECV_MAX_VOL) {
        nFMCommand = FM_SET_VOL;
        nFMParam = nCMDVal;
      }
      break;
    case 5:
    //频率加减
      switch (nCMDVal)
      {
        case 0:
          nFMCommand = FM_FRE_DOWN;
          break;
        case 1:
          nFMCommand = FM_FRE_UP;
          break;
      }
      break;
    case 6:
    //存储频率控制
      switch (nCMDVal)
      {
        case 0:
          freq_clear();
          break;
        case 1:
          freq_add(nFrequency);
          break;
        case 2:
          freq_del(nFrequency);
          break;
        case 3:
          freq_set_send();
          break;
        case 4:
        /*
          allCommand.CMD.command = FM_SET_FRE;
          allCommand.CMD.param = freq_prev();
          allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
          gsFMDataList.lpush(allCommand.u8Data);
  */
          gsFMDataList.push(FM_SET_FRE, freq_prev(), CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
          break;
        case 5:
        /*
          allCommand.CMD.command = FM_SET_FRE;
          allCommand.CMD.param = freq_next();
          allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_SYSTEM;
          gsFMDataList.lpush(allCommand.u8Data);
*/
          gsFMDataList.push(FM_SET_FRE, freq_next(), CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
          break;
        case 6:
          freq_sort(-1);
          break;
        case 7:
          freq_sort(1);
          break;
        default:
          break;
      }
      break;
    case 7:
    //存储频率控制
      if (nCMDVal > 0 && nCMDVal <= CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS) {
        nFMCommand = FM_SET_FRE;
        nFMParam = anSaveFreq[nCMDVal - 1];
      }
      break;
    case 8:
    //频率设置
      nFMCommand = FM_SET_FRE;
      nFMParam = nCMDVal;
      break;
    case 9:
    //背景灯延时设置
      DBGPRINTF("nFMParam:%d", nFMParam);
      if (nFMParam >= CONST_APP_FM_RECV_MIN_BLK && nFMParam <= CONST_APP_FM_RECV_MAX_BLK) {
        nFMCommand = FM_SET_BANK;
        nFMParam = nCMDVal;
      }
      break;
    case 10:
    //静噪比设置
      DBGPRINTF("nFMParam:%d", nFMParam);
      if (nFMParam >= CONST_APP_FM_RECV_SN_MIN_THR && nFMParam <= CONST_APP_FM_RECV_SN_MAX_THR) {
        nFMCommand = FM_SET_SN_THR;
        nFMParam = nCMDVal;
      }
      break;
    default:
      break;

  }
  if (nFMCommand >= 0) {
    DBGPRINTLN("nFMCOMMAND>=0");
    /*
    allCommand.CMD.command = nFMCommand;
    allCommand.CMD.param = nFMParam;
    allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_WEBAPP;
    gsFMDataList.lpush(allCommand.u8Data);
    */
    gsFMDataList.push(nFMCommand, nFMParam, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
  }
  DBGPRINTLN("--------------------");
  DBGPRINTF("nCMDType: %d  nCMDVal: %d || nFMCommand: %d  nFMParam:%d\n", nCMDType, nCMDVal, nFMCommand, nFMParam);
}

//编码系统状态字符串
void ICACHE_FLASH_ATTR FM_ZCJ::encode_stat_string()
{
  uint8_t nT1;
  nT1=nFrequency&0xFF;
  u8statString[0]=nT1;
  nT1=((nFrequency>>8)&0xFF);
  u8statString[1]=nT1;
  
  nT1=(u8Stat<<7) & CONST_APP_FM_STAT_STRING_SWITCH_MASK;
  
  if (errCount>0){
    nT1|=CONST_APP_FM_STAT_STRING_VALID_MASK;
  }
  
  nT1|=(u8Campus<<5) & CONST_APP_FM_STAT_STRING_CAMPUS_MASK ;
  nT1|=u8Vol & CONST_APP_FM_STAT_STRING_VOL_MASK ;
  u8statString[2]=nT1;
  
  u8statString[3]=u8Blk;
  
  nT1=(u8DSP<<7) & CONST_APP_FM_STAT_STRING_DSP_MASK;
  nT1|=u8SN_THR & CONST_APP_FM_STAT_STRING_SN_MASK;
  
  u8statString[4]=nT1;
};

//解码系统状态字符串
void ICACHE_FLASH_ATTR FM_ZCJ::decode_stat_string()
{
  uint8_t nT1;
  nFrequency=u8statString[1];
  nFrequency=nFrequency<<8;
  nFrequency|=u8statString[0];
  
  nT1=u8statString[2];
  u8Stat=(nT1 & CONST_APP_FM_STAT_STRING_SWITCH_MASK)>>7;
  errCount=(nT1 & CONST_APP_FM_STAT_STRING_VALID_MASK)>>6;
  u8Campus=(nT1 & CONST_APP_FM_STAT_STRING_CAMPUS_MASK)>>5;
  u8Vol=nT1 & CONST_APP_FM_STAT_STRING_VOL_MASK;
  
  u8Blk=u8statString[3];
  
  nT1=u8statString[4];
  u8DSP=(nT1 & CONST_APP_FM_STAT_STRING_DSP_MASK)>>7;
  u8SN_THR=nT1 & CONST_APP_FM_STAT_STRING_SN_MASK;
}

//转换到JSON格式字符串
void ICACHE_FLASH_ATTR FM_ZCJ::decode_stat_json(char *strPtr)
{
  short nT1;
  short frequency;
  uint8_t stat,err,campus,vol,blk,dsp, snthr;
  
  frequency=u8statString[1];
  frequency=frequency<<8;
  frequency|=u8statString[0];
  
  nT1=u8statString[2];
  stat=(nT1 & CONST_APP_FM_STAT_STRING_SWITCH_MASK)>>7;
  err=(nT1 & CONST_APP_FM_STAT_STRING_VALID_MASK)>>6;
  campus=(nT1 & CONST_APP_FM_STAT_STRING_CAMPUS_MASK)>>5;
  vol=nT1 & CONST_APP_FM_STAT_STRING_VOL_MASK;
  
  blk=u8statString[3];
  
  nT1=u8statString[4];
  dsp=(nT1 & CONST_APP_FM_STAT_STRING_DSP_MASK)>>7;
  snthr=nT1 & CONST_APP_FM_STAT_STRING_SN_MASK;
  sprintf(strPtr,"{\"STAT\":%d,\"frequency\":%d,\"err\":%d,\"campus\":%d,\"vol\":%d,\"blk\":%d,\"DSP\":%d,\"SN_THR\":%d,}",\
  stat,frequency,err,campus,vol,blk, dsp,snthr);
  
}
 



void ICACHE_FLASH_ATTR FM_ZCJ::FM_loop()
{
  check_FM_reply();
  if ((ulGet_interval(ulRecvTimeOutTick) > CONST_RECV_TIMEOUT_LEN)) {
    if (bSendBusyFlag) {
      bSendBusyFlag = false;
      errCount++;
    }
  }

//  if (gsFMDataList.count()) {
  if (gsFMDataList.available()) {
    if (!bSendBusyFlag) {
      unsigned char *spTemp;
      spTemp = allCommand.u8Data;
      //gsFMDataList.rpop(spTemp);
      gsFMDataList.rpop(&allCommand.CMD);
      if (allCommand.CMD.flag != CONST_APP_FM_RECV_CMD_FLAG_SYSTEM) {
        memcpy((void *) currCommand.u8Data, spTemp, sizeof(currCommand));
      }
      DBGPRINTLN("----Prepare to send data---------");
      DBGPRINTF("\n %d   %d\n", allCommand.CMD.command, allCommand.CMD.param);
      //send data;
      send_CMD_to_FM(allCommand.CMD.command, allCommand.CMD.param);
      bSave_config();
      //receive buff clean
      u8recvLen = 0;
      //reset timeout tick
      ulRecvTimeOutTick = ulReset_interval();
    }
  }

  // save application data
  if ((ulGet_interval(ulFMSaveTick) > CONST_APP_FM_RECV_SAVE_CYCLE))
  {
    bSave_config();
    ulFMSaveTick = ulReset_interval();
  }

  if ((ulGet_interval(ulFMSIOQueryTick) > CONST_APP_FM_RECV_SIO_QUERY_INTERVAL))
  {
    push_CMD_to_FM_queue();
    ulFMSIOQueryTick = ulReset_interval();
  }

}

/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR FM_ZCJ::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;

  File configFile = SPIFFS.open(CONST_APP_FM_RECV_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open application data file");
    return bRet;
  }

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuff, CONST_APP_FM_RECV_BIG_BUFFLEN);
    if (nLen <= 0)
      break;
    bigBuff[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bigBuff, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bigBuff, "sta", 3) == 0) {
      u8Stat = atoi(spTemp);
    }
    if (memcmp(bigBuff, "fre", 3) == 0) {
      nFrequency = atoi(spTemp);
    }
    if (memcmp(bigBuff, "vol", 3) == 0) {
      u8Vol = atoi(spTemp);
    }
    if (memcmp(bigBuff, "cam", 3) == 0) {
      u8Campus = atoi(spTemp);
    }
    if (memcmp(bigBuff, "blk", 3) == 0) {
      u8Blk = atoi(spTemp);
    }
    if (memcmp(bigBuff, "snt", 3) == 0) {
      u8SN_THR = atoi(spTemp);
    }
    if (memcmp(bigBuff, "dsp", 3) == 0) {
      u8DSP = atoi(spTemp);
    }

    if (memcmp(bigBuff, "sav", 3) == 0) {
      int nPos;
      *(spTemp + 2) = '\0';
      nPos = atoi(spTemp);
      if ((nPos >= 0) && (nPos < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS)) {
        anSaveFreq[nPos] = atoi(spTemp + 3);
      }
    }
  }
  // Real world application would store these values in some variables for later use
  DBGPRINT("Loaded stat: ");
  DBGPRINT("[" );
  DBGPRINT( u8Stat );
  DBGPRINTLN("]");
  DBGPRINT("Loaded frequency: ");
  DBGPRINT("[" );
  DBGPRINT( nFrequency );
  DBGPRINTLN("]");

  configFile.close();
  DBGPRINTLN("Application Config ok");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR FM_ZCJ::bSave_config()
{
  DBGPRINTLN("--save application data--");
  File configFile = SPIFFS.open(CONST_APP_FM_RECV_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_FM_RECV_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }
  configFile.print("sta:");
  configFile.println(u8Stat);
  configFile.print("fre:");
  configFile.println(nFrequency);
  configFile.print("vol:");
  configFile.println(u8Vol);
  configFile.print("cam:");
  configFile.println(u8Campus);
  configFile.print("blk:");
  configFile.println(u8Blk);
  configFile.print("snt:");
  configFile.println(u8SN_THR);
  configFile.print("dsp:");
  configFile.println(u8DSP);

  int i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    sprintf((char *)bigBuff, "sav:%02d:%d", i, anSaveFreq[i]);
    configFile.println((char *)bigBuff);
  }

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}


