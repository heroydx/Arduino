#include <Motor_DQ.h>

//modified on 2017/5/11 by Steven Lian steven.lian@gmail.com

static SoftwareSerial motorSerial(CONST_MOTOR_DQ_RX_PORT, CONST_MOTOR_DQ_TX_PORT);//RX,TX

motor_DQ::motor_DQ()
{
  short i;
  for (i = 1; i < CONST_APP_MOTOR_DEFAULT_QUERY_CMD_LIST_LEN; i++) {
    queryCMDList[i] = CONST_APP_MOTOR_QUERY_CURR_CYCLE;
  }
  queryCMDList[0] = CONST_APP_MOTOR_QUERY_MAX_CYCLE;
  queryCMDListCount = 0;

  nRecvTimeOutInMS = CONST_APP_MOTOR_DEFAULT_QUERY_CYCLE_IN_MS;
  ulSIOQueryInMS = CONST_APP_MOTOR_DEFAULT_QUERY_TIME_OUT_IN_MS;
  deviceReady = false;
  rs485Busy = false;
}

bool motor_DQ::begin()
{
  return begin(CONST_APP_MOTOR_DEFAULT_ID);
}

bool motor_DQ::begin(uint8_t ID)
{
  bool ret = false;
  devID = ID;
  short nTimes = CONST_APP_MOTOR_DEFAULT_TRY_TIMES;
  APP_MOTOR_SERIAL.begin(CONST_APP_MOTOR_SIO_BAUD);
  while (nTimes > 0) {
    delay(1);
    deviceReady = is_motor_exist(ID);
    if (deviceReady) {
      break;
    }
    nTimes--;
  }
  return ret;
}

void motor_DQ::motor_loop()
{
  avaiable();
  delay(1);
  if ((ulGet_interval(ulSIOQueryTick) > ulSIOQueryInMS) && (rs485Busy == 0))
  {
    bufCommand.cmd = queryCMDList[queryCMDListCount];
    send_CMD(&bufCommand);
    //DBGPRINTF("\n motor_loop,[%d]-[%d]\n", queryCMDListCount, queryCMDList[queryCMDListCount]);

    queryCMDListCount++;
    if (queryCMDListCount >= CONST_APP_MOTOR_DEFAULT_QUERY_CMD_LIST_LEN) {
      queryCMDListCount = 0;
    }
    ulSIOQueryTick = ulReset_interval();
    delay(10);
  }

  if (ulGet_interval(ulOneSecondTick) > 1000) {
    //DBGPRINTF("\n check rs485Busy [%d]", rs485Busy);
    rs485Busy = false;
    ulOneSecondTick = ulReset_interval();
  }
}

bool motor_DQ::devReady()
{
  return deviceReady;
}


bool motor_DQ::avaiable()
{
  bool ret = false;
  //DBGPRINTLN("\n motor_DQ::avaiable()");
  if (APP_MOTOR_SERIAL.available()) {
    uint8_t chR;
    chR = APP_MOTOR_SERIAL.read();
    //DBGPRINTF("\n APP_MOTOR_SERIAL.available() %c",chR);
    switch (chR)
    {
      case CONST_APP_MOTOR_M_HEADER_CHAR:
        recvLen = 0;
        break;
      case CONST_APP_MOTOR_MP_END_CHAR:
        if (recvLen > 0)
        {
          //DBGPRINTLN("\n motor_DQ::avaiable() case CONST_APP_MOTOR_MP_END_CHAR");
          decod_CMD(recvBuff, &currStat);
          ret = true;
          recvBuff[recvLen + 1] = 0;
        }
        break;
      default:
        break;
    }
    recvBuff[recvLen] = chR;
    recvLen++;
    if (recvLen > CONST_APP_MOTOR_RECV_BUFFLEN) {
      recvLen = 0;
    }
  }
  return ret;
}

bool motor_DQ::send_cmd_to_device(char *pBuff)
{
  short i;
  //set 1 seoncd time out;
  rs485Busy = 1;
  ulOneSecondTick = ulReset_interval();

  for (i = 0; i < CONST_APP_MOTOR_MSG_LENG; i++) {
    APP_MOTOR_SERIAL.write(*(pBuff + i));
  }
}

bool motor_DQ::send_CMD(stMotorCMD_DQ *ptrCMD)
{
  memcpy(&currCommand, ptrCMD, sizeof (currCommand));
  encode_CMD(sendBuff, ptrCMD);
  send_cmd_to_device(sendBuff);
  return true;
}

bool motor_DQ::is_motor_exist(uint8_t ID)
{
  bool ret = false;
  //wait for 1 seconds to get data;
  unsigned long ulTick;
  short nLen = 0;
  //vReset_interval(ulTick);
  ulTick = ulReset_interval();
  currCommand.cmd = CONST_APP_MOTOR_QUERY_MAX_CYCLE;
  currCommand.ID = ID;
  encode_CMD(sendBuff, &currCommand);
  send_cmd_to_device(sendBuff);
  delay(5);

  DBGPRINTLN("wait for receive:");
  while (ulGet_interval(ulTick) < 1000) {
    delay(1);
    if (avaiable()) {
      ret = true;
      break;
    }
  }
  DBGPRINTLN("");
  DBGPRINTLN("finished");
  return ret;
}


uint8_t motor_DQ::motor_encode(uint8_t data)
{
  return data + 0x30;
}

uint8_t motor_DQ::motor_decode(uint8_t data)
{
  return data - 0x30;
}

void motor_DQ::byte_to_bcd(char data, char *str)//
{
  uint8_t nLow;
  uint8_t nHigh;
  nLow = data & 0xF;//低位和0xF（00001111）与，高位都变成了0
  nHigh = (data >> 4) & 0xF; //低位和0xF（00001111）与，高位都变成了0
  *str = nHigh;
  *(str + 1) = nLow;
  //Serial.printf("\n in byte %d,%d\n",(int) *str,(int) 1*(str+1));
}

void motor_DQ::short_to_bcd(short data, char *str)//求低四位元
{
  uint8_t nLow;
  uint8_t nHigh;
  nLow = data & 0xFF;
  nHigh = (data >> 8) & 0xFF;
  byte_to_bcd(nHigh, str);
  byte_to_bcd(nLow, str + 2);
}

uint8_t motor_DQ::bcd_to_byte(char *str)
{
  uint8_t ret;
  ret = *(str) << 4;
  ret |= *(str + 1);
  return ret;
}

short motor_DQ::bcd_to_short(char *str)
{
  short ret;
  ret = bcd_to_byte(str) << 8;
  ret |= bcd_to_byte(str + 2);
  return ret;
}

void motor_DQ::cal_check_sum(uint8_t *pBuff)
{
  uint8_t i;
  short  val;
  uint8_t sum1;
  uint8_t sum2;
  val = 0;
  for (i = 0; i < CONST_APP_MOTOR_CHKSUM_BEGIN_POS; i++) {
    //DBGPRINTF("%d ", pBuff[i]);
    val += pBuff[i];
  }
  sum1 = (val % 100) / 10;
  sum2 = val % 10;
  //Serial.printf("\n nLen [%d] val [%d], sum1 [%d],sum2 [%d]", nLen, val, sum1, sum2);
  *(pBuff + CONST_APP_MOTOR_CHKSUM_BEGIN_POS) = motor_encode(sum1);
  *(pBuff + CONST_APP_MOTOR_CHKSUM_BEGIN_POS + 1) = motor_encode(sum2);
}

bool motor_DQ::verify_check_sum(uint8_t *pBuff)
{
  uint8_t i;
  short  val;
  uint8_t sum1;
  uint8_t sum2;
  val = 0;
  for (i = 0; i < CONST_APP_MOTOR_CHKSUM_BEGIN_POS; i++) {
    //DBGPRINTF("%d ", pBuff[i]);
    val += pBuff[i];
  }
  sum1 = (val % 100) / 10;
  sum2 = val % 10;
  //Serial.printf("\n nLen [%d] val [%d], sum1 [%d],sum2 [%d]", nLen, val, sum1, sum2);
  if ((*(pBuff + CONST_APP_MOTOR_CHKSUM_BEGIN_POS) == motor_encode(sum1)) && ( *(pBuff + CONST_APP_MOTOR_CHKSUM_BEGIN_POS + 1) == motor_encode(sum2)))
    return true;
  else
    return false;
}

void motor_DQ::encode_CMD(char *pBuff, stMotorCMD_DQ * stPtr)
{
  //DBGPRINTF("\n===encode_CMD===");
  //DBGPRINTF("\nCMD [%d],ID [%d]\n", stPtr->cmd, stPtr->cmd);

  //strcpy(pBuff, "\x21\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x0D");//默认字符串
  memset(pBuff, 0, CONST_APP_MOTOR_MSG_LENG);
  switch (stPtr->cmd)
  {
    case CONST_APP_MOTOR_SAVE_EEPROM:
      pBuff[CONST_APP_MOTOR_CMD_BEGIN_POS] = CONST_APP_MOTOR_SAVE_EEPROM;
      break;
    case CONST_APP_MOTOR_QUERY_MAX_CYCLE:
      pBuff[CONST_APP_MOTOR_CMD_BEGIN_POS] = CONST_APP_MOTOR_QUERY_MAX_CYCLE;
      break;
    case CONST_APP_MOTOR_SET_MAX_CYCLE:
      pBuff[CONST_APP_MOTOR_CMD_BEGIN_POS] = CONST_APP_MOTOR_SET_MAX_CYCLE;
      short_to_bcd(stPtr->speed, pBuff + CONST_APP_MOTOR_SPEED_BEGIN_POS);
      byte_to_bcd(devID, pBuff + CONST_APP_MOTOR_ID2_BEGIN_POS);
      break;
    case CONST_APP_MOTOR_QUERY_CURR_CYCLE:
      pBuff[CONST_APP_MOTOR_CMD_BEGIN_POS] = CONST_APP_MOTOR_QUERY_CURR_CYCLE;
      break;
    case CONST_APP_MOTOR_SET_CURR_CYCLE:
      pBuff[CONST_APP_MOTOR_CMD_BEGIN_POS] = CONST_APP_MOTOR_SET_CURR_CYCLE;
      short_to_bcd(stPtr->speed, pBuff + CONST_APP_MOTOR_SPEED_BEGIN_POS);
      break;
    default:
      break;
  }

  //set default data;
  pBuff[CONST_APP_MOTOR_MODE_BEGIN_POS] = currStat.cycleMode;
  pBuff[CONST_APP_MOTOR_OFFDELAY_BEGIN_POS] = currStat.offDelay;
  pBuff[CONST_APP_MOTOR_ONDELAY_BEGIN_POS] = currStat.onDelay;
  //byte_to_bcd(currStat.ID, pBuff + CONST_APP_MOTOR_ID2_BEGIN_POS);
  //byte_to_bcd(devID, pBuff + CONST_APP_MOTOR_ID2_BEGIN_POS);

  //handle address:
  //byte_to_bcd(stPtr->ID, pBuff + CONST_APP_MOTOR_ID_BEGIN_POS);
  byte_to_bcd(devID, pBuff + CONST_APP_MOTOR_ID_BEGIN_POS);

  short i;
  for (i = 0; i < CONST_APP_MOTOR_MSG_LENG; i++)
  {
    *(pBuff + i) = motor_encode(*(pBuff + i));
  }
  //test purpose
  //strcpy(pBuff, "\x21\x30\x31\x33\x30\x37\x3D\x30\x30\x30\x30\x30\x30\x30\x30\x32\x39\x0D");//2000转每分钟的速度
  //strcpy(pBuff, "\x21\x30\x31\x33\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x39\x0D");//2000转每分钟的速度
  //head
  *(pBuff) = CONST_APP_MOTOR_P_HEADER_CHAR;
  //check sum
  cal_check_sum((uint8_t *)pBuff);
  //tail
  *(pBuff + CONST_APP_MOTOR_MSG_LENG - 1) = CONST_APP_MOTOR_MP_END_CHAR;
  //DBGPRINTF("\n encode_CMD:[%s]  --> ", pBuff);
  for (i = 0; i < CONST_APP_MOTOR_MSG_LENG; i++)
  {
    //DBGPRINTF("%02X ", *(pBuff + i));
  }
}


void motor_DQ::decod_CMD(char *pBuff, stMotorStat_DQ * stPtr)
{
  char chCMD;
  short i;
  //DBGPRINTF("\n decode_CMD:[%s]  --> ", pBuff);
  if (verify_check_sum((uint8_t *)pBuff)) {
    //DBGPRINT(" CHECK_SUM OK: ");
    //DBGPRINTF("%02X ", *(pBuff));
    for (i = 1; i < CONST_APP_MOTOR_MSG_LENG - 1; i++)
    {
      * (pBuff + i) = motor_decode(*(pBuff + i));
      //DBGPRINTF("%02X ", *(pBuff + i));
    }
    //DBGPRINTF("%02X ", *(pBuff + i));

    chCMD = pBuff[CONST_APP_MOTOR_CMD_BEGIN_POS];
    switch (chCMD)
    {
      case CONST_APP_MOTOR_SAVE_EEPROM:
        break;
      case CONST_APP_MOTOR_QUERY_MAX_CYCLE:
      case CONST_APP_MOTOR_SET_MAX_CYCLE:
        stPtr->maxSpeed = bcd_to_short(pBuff + CONST_APP_MOTOR_SPEED_BEGIN_POS);
        stPtr->cycleMode = *(pBuff + CONST_APP_MOTOR_MODE_BEGIN_POS);
        stPtr->onDelay = *(pBuff + CONST_APP_MOTOR_ONDELAY_BEGIN_POS);
        stPtr->offDelay = *(pBuff + CONST_APP_MOTOR_OFFDELAY_BEGIN_POS);
        stPtr->ID = bcd_to_byte(pBuff + CONST_APP_MOTOR_ID2_BEGIN_POS);
        stPtr->swVer = *(pBuff + CONST_APP_MOTOR_SW_VER_BEGIN_POS);
        stPtr->hwVer = *(pBuff + CONST_APP_MOTOR_HW_VER_BEGIN_POS);
        break;
      case CONST_APP_MOTOR_QUERY_CURR_CYCLE:
      case CONST_APP_MOTOR_SET_CURR_CYCLE:
        stPtr->speed = bcd_to_short(pBuff + CONST_APP_MOTOR_SPEED_BEGIN_POS);
        stPtr->dianliu = bcd_to_short(pBuff + CONST_APP_MOTOR_DIANLIU_BEGIN_POS);
        stPtr->errorCode = *(pBuff + CONST_APP_MOTOR_ERRCODE_BEGIN_POS);
        break;
    }
  }
  else {
    //DBGPRINT(" CHECK_SUM ERROR: ");
  }
}

void motor_DQ::debug_print_info()
{
  DBGPRINTLN("\n === Curr Stat begin ===");
  DBGPRINTF(" deviceReady [%d], rs485Busy [%d]", deviceReady, rs485Busy);
  DBGPRINTF(" ID [%d], swVer[%02X], hwVer[%02X]", currStat.ID, currStat.swVer, currStat.swVer);
  DBGPRINTF("\n speed [%d], max speed[%d]", currStat.speed, currStat.maxSpeed);
  DBGPRINTF("\n dianliu [%d], on[%d], off[%d] error[%d]", currStat.dianliu, currStat.onDelay, currStat.offDelay, currStat.errorCode);
  DBGPRINTLN("\n === Curr Stat end === \n");

}

