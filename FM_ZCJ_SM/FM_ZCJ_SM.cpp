#include <FM_ZCJ_SM.h>

//modified on 2017/4/2 by Steven Lian steven.lian@gmail.com

static SoftwareSerial SerialFM(CONST_FM_ZCJ_RX_PORT, CONST_FM_ZCJ_TX_PORT); //
//ListArray gsFMDataList(CONST_APP_FM_RECV_COMMAND_BUFF_LEN, sizeof(FMCommandUnion));

static FMCommand gsFMDataList;

FMCommand::FMCommand()
{
  memset(gasCommand, 0, (sizeof(stFMCommand) * CONST_APP_FM_RECV_COMMAND_BUFF_LEN));
  gu8CommandPos = 0;
};

uint8_t ICACHE_FLASH_ATTR FMCommand::lpush(uint8_t command, short param, uint8_t flag)
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


uint8_t ICACHE_FLASH_ATTR FMCommand::push(uint8_t command, short param, uint8_t flag)
{
  return lpush(command, param, flag);
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
  dataBuffPtr=NULL;
}

bool FM_ZCJ::begin()
{
  bool ret=false;
  bStatChangeFlag=false;
  //  Serial.begin(CONST_APP_FM_RECV_SIO_BAUD_DATA);
  APP_SERIAL.begin(CONST_APP_FM_RECV_SIO_BAUD_DATA);
  while (!APP_SERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  };
  DBGPRINTLN("\n\n");
  DBGPRINTLN("--FM RADIO INIT BEGIN--");


  short nTimes = 5;
  while (nTimes > 0) {
    delay(1);
    deviceReady = is_FM_exist();
    if (deviceReady) {
      DBGPRINTLN("FMRADIO is ready!!!");
      break;
    }
    nTimes--;
  }

  if (!bLoad_config()) {
    currStat.u8Stat = CONST_APP_FM_RECV_DEFAULT_STAT;
    currStat.nFrequency = CONST_APP_FM_RECV_DEFAULT_FREQUENCY;
    currStat.u8Vol = CONST_APP_FM_RECV_DEFAULT_VOL;
    currStat.u8Campus = CONST_APP_FM_RECV_DEFAULT_SN_CAMPUS;
    currStat.u8Blk = CONST_APP_FM_RECV_DEFAULT_BLK;
    currStat.u8SN_THR = CONST_APP_FM_RECV_DEFAULT_SN_THR;
    currStat.u8DSP = CONST_APP_FM_RECV_DEFAULT_SN_DSP;
  }

  restore_setting();
  return ret;

}


bool FM_ZCJ::is_FM_exist()
{
  bool ret = false;
  //准备并发送查询状态数据
  allCommand.CMD.command = FM_SET_BANK;
  allCommand.CMD.param = CONST_APP_FM_RECV_DEFAULT_BLK;
  send_CMD_to_FM(allCommand.CMD.command, allCommand.CMD.param);

  //wait for 2 seconds to get data;
  unsigned long ulTick;
  //vReset_interval(ulTick);
  ulTick = ulReset_interval();
  DBGPRINTLN("wait for receive:");
  while (ulGet_interval(ulTick) < 2000) {
    //DBGPRINT("fc");
    if (APP_SERIAL.available()) {
      uint8_t chR;
      chR = APP_SERIAL.read();
      if (chR == 'B'){
        ret=true;
        break;
      }
    }
    delay(5);
  }
  while(APP_SERIAL.available())
  {
    APP_SERIAL.read();
  }
  DBGPRINTLN("\nis_FM_exist done");
  return ret;
}


bool FM_ZCJ::devReady()
{
  return deviceReady;
}



bool FM_ZCJ::read_config()
{
  bool ret = true;
  //准备并发送查询状态数据
  allCommand.CMD.command=FM_STATUS;
  allCommand.CMD.param=0;
  send_CMD_to_FM(allCommand.CMD.command,allCommand.CMD.param);
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
  //DBGPRINTLN("-- -sort-- -function-- -");
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    //DBGPRINTF(" % 6d", anSaveFreq[i]);
  }
  //DBGPRINTLN("\n-- -");
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
    //DBGPRINTF("\n j: [ % d]\n", j);
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
  //DBGPRINTLN("-- -sort-- -function-- -");
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    //DBGPRINTF(" % 6d", anSaveFreq[i]);
  }
  //DBGPRINTLN("\n-- -");
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

  gsFMDataList.push(FM_SET_FRE, currStat.nFrequency, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_VOL, currStat.u8Vol, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_CAMPUS, currStat.u8Campus, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_STATUS, 0, CONST_APP_FM_RECV_CMD_FLAG_SYSTEM);

}

void ICACHE_FLASH_ATTR FM_ZCJ::handle_SIO_reply_data(char *param)
{
  int nLen;
  char *spTemp;
  spTemp = param;
  nLen = strlen(spTemp);
  //DBGPRINTF("\n**-- in Handle Replay Data: % d  freeheap: % d--**", nLen, ESP.getFreeHeap());
  if (nLen < 1) {
    return;
  }
  *(spTemp + nLen - 1) = '\0';

  if (memcmp(spTemp, "ERR", 3) == 0)
  {
    //error
    currStat.errCount ++;
  }
  //stat on/off
  else if (memcmp(spTemp, "PAUS", 4) == 0)  {
    currStat.u8Stat = 0;
    currStat.errCount = 0;
  }
  else if (memcmp(spTemp, "PLAY", 4) == 0) {
    currStat.u8Stat = 1;
    currStat.errCount = 0;
  }
  //frequency
  else if ((memcmp(spTemp, "FRE=", 4) == 0) && (nLen > 4)) {
    short nT1;
    nT1 = atoi(spTemp + 4);
    if ((nT1 >= nFreqLow) && (nT1 <= nFreqHigh)) {
      currStat.nFrequency = nT1;
    }
    currStat.errCount = 0;
  }
  //volume
  else if ((memcmp(spTemp, "VOL=", 4) == 0) && (nLen > 4)) {
    short nT1;
    nT1 = atoi(spTemp + 4);
    if ((nT1 >= CONST_APP_FM_RECV_MIN_VOL) && (nT1 <= CONST_APP_FM_RECV_MAX_VOL)) {
      currStat.u8Vol = nT1;
      //DBGPRINTF("\n--Convert VOL: % d  % d\n", nT1, gsCurrent.u8Vol);
    }
    currStat.errCount = 0;
  }
  //backgroud light  BANK=04s\n
  else if ((memcmp(spTemp, "BANK=", 5) == 0) && (nLen > 5)) {
    short nT1;
    *(spTemp + 7) = '\0';
    nT1 = atoi(spTemp + 5);
    //DBGPRINTLN(nT1);
    if ((nT1 >= CONST_APP_FM_RECV_MIN_BLK) && (nT1 <= CONST_APP_FM_RECV_MAX_BLK)) {
      currStat.u8Blk = nT1;
    }
    currStat.errCount = 0;
  }
  //Campus
  else if ((memcmp(spTemp, "CAMPUS_ON", 9) == 0) || (memcmp(spTemp, "CAMPOS_ON", 9) == 0)) {
    currStat.u8Campus = 1;
    currStat.errCount = 0;
  }
  else if ((memcmp(spTemp, "CAMPUS_OFF", 10) == 0) || (memcmp(spTemp, "CAMPOS_OFF", 9) == 0)) {
    currStat.u8Campus = 0;
    currStat.errCount = 0;
  }
  //DSP SN on/off
  else if (memcmp(spTemp, "SN_OFF", 6) == 0) {
    currStat.u8DSP = 0;
    currStat.errCount = 0;
  }
  else if (memcmp(spTemp, "SN_ON", 5) == 0) {
    currStat.u8DSP = 1;
    currStat.errCount = 0;
  }
  //DSP threold
  else if ((memcmp(spTemp, "SN_THR=", 7) == 0) && (nLen > 7)) {
    short nT1;
    nT1 = atoi(spTemp + 7);
    if ((nT1 >= CONST_APP_FM_RECV_SN_MIN_THR) && (nT1 <= CONST_APP_FM_RECV_SN_MAX_THR)) {
      currStat.u8SN_THR = nT1;
    }
    currStat.errCount = 0;
  }
  //reset response
  else if (memcmp(spTemp, "OK", 2) == 0) {
    currStat.errCount = 0;
  }
  //RM radio device ver:PCB_NUMBE:LCD_FM_RX_ENC_V1.9
  else if (memcmp(spTemp, "PCB_NUM", 7) == 0) {
    strncpy(achFMDeviceVer, spTemp, CONST_APP_FM_RECV_FM_DEVICE_VER_BUFFLEN);
    currStat.errCount = 0;
  }
  //status query header/tailer
  else if (memcmp(spTemp, "/***", 4) == 0) {
    currStat.errCount = 0;
  }
  //status query tailer
  else if (memcmp(spTemp, "Thank", 5) == 0) {
    currStat.errCount = 0;
  }
  else {
    currStat.errCount++;
  }
  //DBGPRINTF("\n**-- in Handle Replay Data END: %d--**", nLen);
  encode_stat_string(u8statString);
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
      //DBGPRINTLN("**--Receive SIO data--**");
      //DBGPRINTF("[%s]", au8recvBuff);
      strcpy(achFMReply, (char *) au8recvBuff);
      u8recvLen = 0;
      handle_SIO_reply_data(achFMReply);
    }
  }
  delay(1);
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
      //sprintf(pBuff, "AT+FRE=%d\n", nParam);
      sprintf(pBuff, "AT<FRE>=%d\n", nParam);
      break;
    case  FM_FRE_DOWN:
      //strcpy(pBuff, "AT+FRED\n");
      strcpy(pBuff, "AT<FRED>\n");
      break;
    case  FM_FRE_UP:
      //strcpy(pBuff, "AT+FREU\n");
      strcpy(pBuff, "AT<FREU>\n");
      break;
    case  FM_PLAY_PAUSE:
      //strcpy(pBuff, "AT+PAUS\n");
      strcpy(pBuff, "AT<PAUS>\n");
      break;
    case  FM_SET_VOL:
      //sprintf(pBuff, "AT+VOL=%02d\n", nParam);
      sprintf(pBuff, "AT<VOL>=%02d\n", nParam);
      break;
    case  FM_VOL_DOWN:
      //strcpy(pBuff, "AT+VOLD\n");
      strcpy(pBuff, "AT<VOLD>\n");
      break;
    case  FM_VOL_UP:
      //strcpy(pBuff, "AT+VOLU\n");
      strcpy(pBuff, "AT<VOLU>\n");
      break;
    case  FM_SET_BANK:
      //sprintf(pBuff, "AT+BANK=%02d\n", nParam);
      sprintf(pBuff, "AT<BANK>=%02d\n", nParam);
      break;
    case  FM_SET_CAMPUS:
      //sprintf(pBuff, "AT+CAMPUS=%d\n", nParam);
      sprintf(pBuff, "AT<CAMPUS>=%d\n", nParam);
      break;
    case  FM_SET_DSP:
      //?
      //sprintf(pBuff, "AT+SN=%d\n", nParam);
      sprintf(pBuff, "AT<SN>=%d\n", nParam);
      break;
    case  FM_SET_SN_THR:
      //sprintf(pBuff, "AT+SN_THR=%02d\n", nParam);
      sprintf(pBuff, "AT<SN_THR>=%02d\n", nParam);
      break;
    case  FM_RESET:
      //strcpy(pBuff, "AT+CR\n");
      strcpy(pBuff, "AT<CR>\n");
      break;
    case  FM_STATUS:
      //strcpy(pBuff, "AT+RET\n");
      strcpy(pBuff, "AT<RET>\n");
      break;
    case FM_SCAN:
      strcpy(pBuff, "AT<SCAN>\n");
      break;
    case FM_SCAND:
      strcpy(pBuff, "AT<SCAND>\n");
      break;
    case FM_SCANU:
      strcpy(pBuff, "AT<SCANU>\n");
      break;
    case FM_SCAMSTOP:
      strcpy(pBuff, "AT<SCANSTOP>\n");
      break;
    case FM_CHNUM:
      sprintf(pBuff, "AT<CH>=%02d\n",nParam);
      break;
    case FM_CHDOWN:
      strcpy(pBuff, "AT<CHD>\n");
      break;
    case FM_CHUP:
      strcpy(pBuff, "AT<CHU>\n");
      break;
    default:
      strcpy(pBuff, "");
      break;
  }

};

void ICACHE_FLASH_ATTR FM_ZCJ::check_cmd_for_stat(short nType)
{
  switch (nType)
  {
    case  FM_SET_FRE:
    case  FM_FRE_DOWN:
    case  FM_FRE_UP:
    case  FM_SET_VOL:
    case  FM_VOL_DOWN:
    case  FM_VOL_UP:
      currStat.u8Stat=1;
    default:
      break;
  }
}


void ICACHE_FLASH_ATTR FM_ZCJ::send_CMD_to_FM(short nType, short nParam)
{
  int i;
  int nLen;

  prepare_FM_CMD(nType, nParam, (char *) au8sendBuff);
  check_cmd_for_stat(nType);
  nLen = strlen((char *) au8sendBuff);
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(au8sendBuff[i]);
  }
  bSendBusyFlag = true;
#ifdef DEBUG_SIO
  //DBGPRINT("\nSend:");
  //DBGPRINT((char *)au8sendBuff);
  //DBGPRINTLN("------------------");
#endif
}


//把原来服务发送的字符串命令，转换为内部命令格式， 返回值>0,成功，=0没有命令
short ICACHE_FLASH_ATTR FM_ZCJ::trans_Server_to_Inter_FM_command(char achParam[], short *npFMCommand, short *npFMParam)
{
  short ret = 0;
  short nCMDType;
  short nCMDVal;
  char *spTemp;
  spTemp = strchr(achParam, ':');
  if (spTemp != NULL) {
    //DBGPRINTF("\n-- %s %s---\n", achParam, spTemp);
    *spTemp = '\0';
    spTemp++;
    nCMDType = (short) atoi(achParam);
    if (nCMDType != 1) {
      //格式 0:1 or 8:1039
      nCMDVal = (short) atoi(spTemp);
    }
    else {
      //格式 1:A:1B:4B0-->

    }

    short nFMCommand = -1;
    short nFMParam = 0;
    //bool bIsFMRadioCommand = true;

    switch (nCMDType) {
      case 0:
        //开关命令
        //on off handle
        nFMCommand = FM_PLAY_PAUSE;
        nFMParam=nCMDVal;
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
      nFMCommand=FM_SAVE_FREQ;
      nFMParam=nCMDVal;
      break;
    case 7:
      //存储频率控制
      if (nCMDVal > 0 && nCMDVal <= CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS) {
        nFMCommand = FM_OP_SAVE_REEQ;
        nFMParam = nCMDVal-1;
      }
      break;
      case 8:
        //频率设置
        nFMCommand = FM_SET_FRE;
        nFMParam = nCMDVal;
        break;
      case 9:
        //背景灯延时设置
        //DBGPRINTF("nFMParam:%d", nFMParam);
        if (nFMParam >= CONST_APP_FM_RECV_MIN_BLK && nFMParam <= CONST_APP_FM_RECV_MAX_BLK) {
          nFMCommand = FM_SET_BANK;
          nFMParam = nCMDVal;
        }
        break;
      case 10:
        //静噪比设置
        //DBGPRINTF("nFMParam:%d", nFMParam);
        if (nFMParam >= CONST_APP_FM_RECV_SN_MIN_THR && nFMParam <= CONST_APP_FM_RECV_SN_MAX_THR) {
          nFMCommand = FM_SET_SN_THR;
          nFMParam = nCMDVal;
        }
        break;
      default:
        break;

    }
    if (nFMCommand >= 0) {
      //DBGPRINTLN("nFMCOMMAND>=0");
      *npFMCommand=nFMCommand;
      *npFMParam=nFMParam;
      ret=1;
    }
    //DBGPRINTLN("--------------------");
    //DBGPRINTF("nCMDType: %d  nCMDVal: %d || nFMCommand: %d  nFMParam:%d \n", nCMDType, nCMDVal, nFMCommand, nFMParam);
  }
  //DBGPRINTF("\n RET: %d",ret);
  return ret;
}


void ICACHE_FLASH_ATTR FM_ZCJ::exec_FM_command(short cmd, short param)
{
  //DBGPRINTLN("\n--exec_FM_command--");
  switch (cmd)
  {
    case FM_PLAY_PAUSE:
      cmd = -1;
      if (currStat.u8Stat != param) {
        cmd = FM_PLAY_PAUSE;
      }
      break;
    case FM_SAVE_FREQ:
      //存储频率控制
      cmd = -1;
      switch (param)
      {
        case 0:
          freq_clear();
          break;
        case 1:
          freq_add(currStat.nFrequency);
          break;
        case 2:
          freq_del(currStat.nFrequency);
          break;
        case 3:
          freq_set_send();
          break;
        case 4:
          /*
            allCommand.CMD.bStatChangeFlag = FM_SET_FRE;
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
    case FM_OP_SAVE_REEQ:
      //存储频率控制
      cmd = FM_SET_FRE;
      param = anSaveFreq[param];
      break;
    default:
      break;
  }


  if (cmd >= 0) {
    //DBGPRINTLN("cmd>=0");
    /*
      allCommand.CMD.command = cmd;
      allCommand.CMD.param = param;
      allCommand.CMD.flag=CONST_APP_FM_RECV_CMD_FLAG_WEBAPP;
      gsFMDataList.lpush(allCommand.u8Data);
    */
    gsFMDataList.push(cmd, param, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
  }
  //DBGPRINTLN("--------------------");
  //DBGPRINTF("cmd: %d  param:%d\n", cmd, param);

}


void ICACHE_FLASH_ATTR FM_ZCJ::handle_Server_to_FM_command(char achParam[])
{
  short nFMCommand = -1;
  short nFMParam = 0;
  //nFMParam=trans_Server_to_Inter_FM_command(achParam, &nFMCommand, &nFMParam);
  //DBGPRINTF("\n=========result:trans_Server_to_Inter_FM_command [%d]",nFMParam);
  if (trans_Server_to_Inter_FM_command(achParam, &nFMCommand, &nFMParam)>0) {
    //DBGPRINTLN("---will call exec_FM_command(nFMCommand,nFMParam) ---");
    exec_FM_command(nFMCommand,nFMParam);
  }
}

/* 处理来自服务器的命令,设置FMRadio的参数,例如,开关音量等 */
void ICACHE_FLASH_ATTR FM_ZCJ::handle_host_to_FM_command(uint8_t data[])
{
  memcpy(allCommand.u8Data, data, sizeof(allCommand.u8Data));
  switch (allCommand.CMD.command)
  {
    case FM_SET_ALL:
      //老的数据格式,部分数据以bit方式合并,目前改成新的格式
      //decode_stat_string(allCommand.stat.u8statString);
      statBuf.nFrequency = allCommand.stat.nFrequency;
      statBuf.u8Vol = allCommand.stat.u8Vol;
      statBuf.u8Stat = allCommand.stat.u8Stat;
      statBuf.u8Campus = allCommand.stat.u8Campus;
      statBuf.u8Blk = allCommand.stat.u8Blk;
      statBuf.u8DSP = allCommand.stat.u8DSP;
      statBuf.u8SN_THR = allCommand.stat.u8SN_THR;
      
      // frequency
      gsFMDataList.push(FM_SET_FRE, statBuf.nFrequency, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      // vol
      gsFMDataList.push(FM_SET_VOL, statBuf.u8Vol, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      // campus
      gsFMDataList.push(FM_SET_CAMPUS, statBuf.u8Campus, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      //blk
      gsFMDataList.push(FM_SET_BANK, statBuf.u8Blk, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      //DSP
      gsFMDataList.push(FM_SET_DSP, statBuf.u8DSP, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      //SN
      gsFMDataList.push(FM_SET_SN_THR, statBuf.u8SN_THR, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      //switch
      if (currStat.u8Stat != statBuf.u8Stat) {
        gsFMDataList.push(FM_PLAY_PAUSE, statBuf.nFrequency, CONST_APP_FM_RECV_CMD_FLAG_WEBAPP);
      }
      break;
    default:
      exec_FM_command(allCommand.CMD.command, allCommand.CMD.param);
      break;
  }
}






//编码系统状态字符串
void ICACHE_FLASH_ATTR FM_ZCJ::encode_stat_string(uint8_t *u8Ptr)
{
  uint8_t nT1;
  nT1 = currStat.nFrequency & 0xFF;
  u8Ptr[0] = nT1;
  nT1 = ((currStat.nFrequency >> 8) & 0xFF);
  u8Ptr[1] = nT1;

  nT1 = (currStat.u8Stat << 7) & CONST_APP_FM_STAT_STRING_SWITCH_MASK;

  if (currStat.errCount > 0) {
    nT1 |= CONST_APP_FM_STAT_STRING_VALID_MASK;
  }

  nT1 |= (currStat.u8Campus << 5) & CONST_APP_FM_STAT_STRING_CAMPUS_MASK ;
  nT1 |= currStat.u8Vol & CONST_APP_FM_STAT_STRING_VOL_MASK ;
  u8Ptr[2] = nT1;

  u8Ptr[3] = currStat.u8Blk;

  nT1 = (currStat.u8DSP << 7) & CONST_APP_FM_STAT_STRING_DSP_MASK;
  nT1 |= currStat.u8SN_THR & CONST_APP_FM_STAT_STRING_SN_MASK;

  u8Ptr[4] = nT1;
};

//解码系统状态字符串
void ICACHE_FLASH_ATTR FM_ZCJ::decode_stat_string(uint8_t *u8Ptr)
{
  uint8_t nT1;
  //DBGPRINTF("\n in decode_stat_string [%02X %02X %02X %02X %02X %02X]",*u8Ptr,*(u8Ptr+1),*(u8Ptr+2),*(u8Ptr+3),*(u8Ptr+4),*(u8Ptr+5));
  statBuf.nFrequency = u8Ptr[1];
  statBuf.nFrequency = statBuf.nFrequency << 8;
  statBuf.nFrequency |= u8Ptr[0];
  //DBGPRINTF("\n nFrequency [%d]",statBuf.nFrequency);

  nT1 = u8Ptr[2];
  statBuf.u8Stat = (nT1 & CONST_APP_FM_STAT_STRING_SWITCH_MASK) >> 7;
  //DBGPRINTF("\n u8Stat [%d]",statBuf.u8Stat);
  statBuf.errCount = (nT1 & CONST_APP_FM_STAT_STRING_VALID_MASK) >> 6;
  //DBGPRINTF("\n errCount [%d]",statBuf.errCount);
  statBuf.u8Campus = (nT1 & CONST_APP_FM_STAT_STRING_CAMPUS_MASK) >> 5;
  //DBGPRINTF("\n u8Campus [%d]",statBuf.u8Campus);
  statBuf.u8Vol = nT1 & CONST_APP_FM_STAT_STRING_VOL_MASK;
  //DBGPRINTF("\n u8Vol [%d]",statBuf.u8Vol);

  statBuf.u8Blk = u8Ptr[3];
  //DBGPRINTF("\n u8Blk [%d]",statBuf.u8Blk);

  nT1 = u8Ptr[4];
  statBuf.u8DSP = (nT1 & CONST_APP_FM_STAT_STRING_DSP_MASK) >> 7;
  //DBGPRINTF("\n u8DSP [%d]",statBuf.u8DSP);
  statBuf.u8SN_THR = nT1 & CONST_APP_FM_STAT_STRING_SN_MASK;
  //DBGPRINTF("\n u8SN_THR [%d]",statBuf.u8SN_THR);
}

void ICACHE_FLASH_ATTR FM_ZCJ::copy2_rpt_data(stFMDeviceRptStat *spStat,uint8_t *u8Ptr)
{
  decode_stat_string(u8Ptr);
  spStat->nFrequency=statBuf.nFrequency;
  spStat->u8Stat=statBuf.u8Stat;
  spStat->u8Campus=statBuf.u8Campus;
  spStat->u8Vol=statBuf.u8Vol;
  spStat->u8Blk=statBuf.u8Blk;
  spStat->u8DSP=statBuf.u8DSP;
  spStat->u8SN_THR=statBuf.u8SN_THR;
  
}

//转换到JSON格式字符串
void ICACHE_FLASH_ATTR FM_ZCJ::decode_stat_json(char *strPtr)
{
  short nT1;
  short frequency;
  uint8_t stat, err, campus, vol, blk, dsp, snthr;

  frequency = u8statString[1];
  frequency = frequency << 8;
  frequency |= u8statString[0];

  nT1 = u8statString[2];
  stat = (nT1 & CONST_APP_FM_STAT_STRING_SWITCH_MASK) >> 7;
  err = (nT1 & CONST_APP_FM_STAT_STRING_VALID_MASK) >> 6;
  campus = (nT1 & CONST_APP_FM_STAT_STRING_CAMPUS_MASK) >> 5;
  vol = nT1 & CONST_APP_FM_STAT_STRING_VOL_MASK;

  blk = u8statString[3];

  nT1 = u8statString[4];
  dsp = (nT1 & CONST_APP_FM_STAT_STRING_DSP_MASK) >> 7;
  snthr = nT1 & CONST_APP_FM_STAT_STRING_SN_MASK;
  sprintf(strPtr, "{\"STAT\":%d,\"frequency\":%d,\"err\":%d,\"campus\":%d,\"vol\":%d,\"blk\":%d,\"DSP\":%d,\"SN_THR\":%d,}", \
          stat, frequency, err, campus, vol, blk, dsp, snthr);

}




void ICACHE_FLASH_ATTR FM_ZCJ::FM_loop()
{
  check_FM_reply();
  
  if ((ulGet_interval(ulRecvTimeOutTick) > CONST_APP_FM_RECV_TIMEOUT_LEN)) {
    if (bSendBusyFlag) {
      bSendBusyFlag = false;
      currStat.errCount++;
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
      //DBGPRINTLN("----Prepare to send data---------");
      //DBGPRINTF("\n %d   %d\n", allCommand.CMD.command, allCommand.CMD.param);
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
  
  //query current FM Status
  if ((ulGet_interval(ulFMStatQueryTick) > CONST_APP_FM_STAT_QUERY_INTERVAL))
  {
    read_config();
    if (memcmp(u8preStatString,u8statString,CONST_APP_FM_STAT_STRING_LENGTH)){
      bStatChangeFlag=true;
      memcpy(u8preStatString,u8statString,CONST_APP_FM_STAT_STRING_LENGTH);
      bSave_config();
    }
    ulFMStatQueryTick = ulReset_interval();
  }

  
  if ((ulGet_interval(ulFMSIOQueryTick) > CONST_APP_FM_RECV_SIO_QUERY_INTERVAL))
  {
    push_CMD_to_FM_queue();
    ulFMSIOQueryTick = ulReset_interval();
  }
  delay(1);
}

/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR FM_ZCJ::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  uint8_t bigBuff[CONST_APP_FM_RECV_BIG_BUFFLEN + 2];

  File configFile = SPIFFS.open(CONST_APP_FM_RECV_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s",CONST_APP_FM_RECV_FILE_NAME);
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
      currStat.u8Stat = atoi(spTemp);
    }
    if (memcmp(bigBuff, "fre", 3) == 0) {
      currStat.nFrequency = atoi(spTemp);
    }
    if (memcmp(bigBuff, "vol", 3) == 0) {
      currStat.u8Vol = atoi(spTemp);
    }
    if (memcmp(bigBuff, "cam", 3) == 0) {
      currStat.u8Campus = atoi(spTemp);
    }
    if (memcmp(bigBuff, "blk", 3) == 0) {
      currStat.u8Blk = atoi(spTemp);
    }
    if (memcmp(bigBuff, "snt", 3) == 0) {
      currStat.u8SN_THR = atoi(spTemp);
    }
    if (memcmp(bigBuff, "dsp", 3) == 0) {
      currStat.u8DSP = atoi(spTemp);
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
  DBGPRINT( currStat.u8Stat );
  DBGPRINTLN("]");
  DBGPRINT("Loaded frequency: ");
  DBGPRINT("[" );
  DBGPRINT( currStat.nFrequency );
  DBGPRINTLN("]");
  
  configFile.close();
  DBGPRINTLN("Application Config ok");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR FM_ZCJ::bSave_config()
{
  
  uint8_t bigBuff[CONST_APP_FM_RECV_BIG_BUFFLEN + 2];

  DBGPRINTLN("--save FMRadio config data--");
  File configFile = SPIFFS.open(CONST_APP_FM_RECV_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_FM_RECV_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTF("Failed to open config file :%s",CONST_APP_FM_RECV_FILE_NAME);
      return false;
    }
  }

  configFile.print("sta:");
  configFile.println(currStat.u8Stat);
  configFile.print("fre:");
  configFile.println(currStat.nFrequency);
  configFile.print("vol:");
  configFile.println(currStat.u8Vol);
  configFile.print("cam:");
  configFile.println(currStat.u8Campus);
  configFile.print("blk:");
  configFile.println(currStat.u8Blk);
  configFile.print("snt:");
  configFile.println(currStat.u8SN_THR);
  configFile.print("dsp:");
  configFile.println(currStat.u8DSP);

  int i;
  for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
    sprintf((char *)bigBuff, "sav:%02d:%d", i, anSaveFreq[i]);
    configFile.println((char *)bigBuff);
  }

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}


void FM_ZCJ::debug_print_info()
{
  short i;
  DBGPRINTLN("\n==** FM info Begin ==**");
  DBGPRINTF("  --stat:[%d] freq:[%d] vol:[%d]\n",currStat.u8Stat,currStat.nFrequency,currStat.u8Vol);
  DBGPRINTF("  --camp:[%d] bank:[%d] DSP:[%d] SN:[%d]\n",currStat.u8Campus,currStat.u8Blk,currStat.u8DSP,currStat.u8SN_THR);
  DBGPRINTLN("\n==** Host Data info END ==**");
}
