#include <SoftwareSerial.h>
extern "C" {
#include "miscCommon.h"
}


SoftwareSerial motorSerial(2, 4);//RX,TX

uint8_t ascii(int hex)
{
  uint8_t cc;
  cc = 0x30;
  switch (hex)
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      cc = 0x30 + hex;
      break;
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      cc = 'A' + hex - 0x0A;
  }
  //Serial.printf("hex=[%x]\n",hex);
  //Serial.printf("cc=[%x]\n",cc);
  return cc;
}

uint8_t ascii2hex(uint8_t cc)
{
  uint8_t hex;
  hex = 0x00;
  switch (cc)
  {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      hex = cc - 0x30;
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      hex = cc - 'a' + 0x0A;
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      hex = cc - 'A' + 0x0A;
  }
  return hex;
}

void vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen) {
  int i, k = 0;
  char chT;
  for (i = 0; i < nLen; i++) {
    chT = pBuff[i] >> 4;
    dispP[k] = ascii(chT);
    k++;
    chT = pBuff[i] & 0xF;
    dispP[k] = ascii(chT);
    k++;
    dispP[k] = ' ';
    k++;
  }
  dispP[k] = 0;
}

void print_hex_data(char *title, uint8_t *ptr, short nLen)
{
  short i;
  DBGPRINTF("\n%s", title);
  for (i = 0; i < nLen; i++)
  {
    DBGPRINTF("%02X ", *(ptr + i));
  }
}


#define SIO_BAUD 74880
char dispBuff[100];

#define APP_MOTOR_SERIAL motorSerial
#define CONST_APP_MOTOR_SIO_BAUD 2400

#define CONST_APP_MOTOR_SAVE_EEPROM 14
#define CONST_APP_MOTOR_QUERY_MAX_CYCLE 0
#define CONST_APP_MOTOR_SET_MAX_CYCLE 1
#define CONST_APP_MOTOR_QUERY_CURR_CYCLE 2
#define CONST_APP_MOTOR_SET_CURR_CYCLE 3

#define CONST_APP_MOTOR_ID_BEGIN_POS 1
#define CONST_APP_MOTOR_CMD_BEGIN_POS 3
#define CONST_APP_MOTOR_SPEED_BEGIN_POS 4
#define CONST_APP_MOTOR_MODE_BEGIN_POS 8
#define CONST_APP_MOTOR_DIANLIU_BEGIN_POS 8
#define CONST_APP_MOTOR_OFFDELAY_BEGIN_POS 9
#define CONST_APP_MOTOR_ONDELAY_BEGIN_POS 10
#define CONST_APP_MOTOR_ERRCODE_BEGIN_POS 12
#define CONST_APP_MOTOR_ID2_BEGIN_POS 11
#define CONST_APP_MOTOR_CHKSUM_BEGIN_POS 15
#define CONST_APP_MOTOR_SW_VER_BEGIN_POS 13
#define CONST_APP_MOTOR_HW_VER_BEGIN_POS 14

#define CONST_APP_MOTOR_MSG_LENG 18


#define CONST_APP_MOTOR_M_HEADER_CHAR '#'
#define CONST_APP_MOTOR_P_HEADER_CHAR '!'
#define CONST_APP_MOTOR_MP_END_CHAR '\xd'

#define CONST_APP_MOTOR_SEND_BUFFLEN 24
#define CONST_APP_MOTOR_RECV_BUFFLEN 24

#define CONST_APP_MOTOR_DEFAULT_QUERY_CYCLE_IN_MS 6000
#define CONST_APP_MOTOR_DEFAULT_QUERY_TIME_OUT_IN_MS 3000
#define CONST_APP_MOTOR_DEFAULT_QUERY_CMD_LIST_LEN 4

#define CONST_APP_MOTOR_DEFAULT_CYCLE_SPEED 200
#define CONST_APP_MOTOR_DEFAULT_CYCLE_MAX_SPEED 500
#define CONST_APP_MOTOR_DEFAULT_CYCLE_MODE 0
#define CONST_APP_MOTOR_DEFAULT_ON_DELAY 1
#define CONST_APP_MOTOR_DEFAULT_OFF_DELAY 1

#define CONST_APP_MOTOR_DEFAULT_SET_ID  '\xFF'

#define CONST_APP_MOTOR_DEFAULT_TRY_TIMES 5

#define CONST_APP_MOTOR_DEFAULT_ID '\xFF'

typedef struct {
  short ID;
  short speed;
  short maxSpeed;
  short cycleMode;
  short dianliu;
  char errorCode;
  char onDelay;
  char offDelay;
  char swVer;
  char hwVer;
} stMotorStat_DQ;

typedef struct {
  char cmd;
  short ID;
  short speed;
  short cycleMode;
  //char onDelay;
  //char offDelay;
} stMotorCMD_DQ;


class motor_DQ
{
  public:
    //data
    stMotorStat_DQ currStat;
    stMotorStat_DQ statBuf;

    stMotorCMD_DQ bufCommand; // buff for command struct 一个命令缓冲结构

    char recvBuff[CONST_APP_MOTOR_RECV_BUFFLEN];
    char sendBuff[CONST_APP_MOTOR_SEND_BUFFLEN];
    uint8_t recvLen;
    unsigned long nRecvTimeOutInMS;
    unsigned long ulRecvTimeOutTick;

    //fuction
    motor_DQ();
    bool begin();
    bool begin(uint8_t ID);
    bool devReady();
    void motor_loop();

    bool send_cmd(stMotorCMD_DQ *ptrCMD);

    void debug_print_info();

  private:
    //data
    bool deviceReady;
    uint8_t rs485Busy;
    uint8_t devID;
    stMotorCMD_DQ currCommand;
    unsigned long ulSIOQueryTick;
    unsigned long ulSIOQueryInMS;
    char queryCMDList[CONST_APP_MOTOR_DEFAULT_QUERY_CMD_LIST_LEN];
    uint8_t queryCMDListCount;
    unsigned long ulOneSecondTick;

    //function
    bool avaiable();
    bool is_motor_exist(uint8_t ID);
    uint8_t motor_encode(uint8_t data);
    uint8_t motor_decode(uint8_t data);
    void byte_to_bcd(char data, char *str);
    void short_to_bcd(short data, char *str);
    uint8_t bcd_to_byte(char *str);
    short bcd_to_short(char *str);
    void cal_check_sum(uint8_t *pBuff);
    bool verify_check_sum(uint8_t *pBuff);
    void encode_CMD(char *pBuff, stMotorCMD_DQ *stPtr);
    void decod_CMD(char *pBuff, stMotorStat_DQ *stPtr);
    bool send_cmd_to_device(char *pBuff);
};

//class end

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
    send_cmd(&bufCommand);
    DBGPRINTF("\n motor_loop,[%d]-[%d]", queryCMDListCount, queryCMDList[queryCMDListCount]);

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
  if (APP_MOTOR_SERIAL.available()) {
    uint8_t chR;
    chR = APP_MOTOR_SERIAL.read();
    switch (chR)
    {
      case CONST_APP_MOTOR_M_HEADER_CHAR:
        recvLen = 0;
        break;
      case CONST_APP_MOTOR_MP_END_CHAR:
        if (recvLen > 0)
        {
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

bool motor_DQ::send_cmd(stMotorCMD_DQ *ptrCMD)
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

  //DBGPRINTLN("wait for receive:");
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
  DBGPRINTF("\n encode_CMD:[%s]  --> ", pBuff);
  for (i = 0; i < CONST_APP_MOTOR_MSG_LENG; i++)
  {
    DBGPRINTF("%02X ", *(pBuff + i));
  }
}


void motor_DQ::decod_CMD(char *pBuff, stMotorStat_DQ * stPtr)
{
  char chCMD;
  short i;
  DBGPRINTF("\n decode_CMD:[%s]  --> ", pBuff);
  if (verify_check_sum((uint8_t *)pBuff)) {
    DBGPRINT(" CHECK_SUM OK: ");
    DBGPRINTF("%02X ", *(pBuff));
    for (i = 1; i < CONST_APP_MOTOR_MSG_LENG - 1; i++)
    {
      * (pBuff + i) = motor_decode(*(pBuff + i));
      DBGPRINTF("%02X ", *(pBuff + i));
    }
    DBGPRINTF("%02X ", *(pBuff + i));

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
    DBGPRINT(" CHECK_SUM ERROR: ");
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


motor_DQ LLMotorData;
unsigned gulSecondTicks;
unsigned gulRecvTimeOutTick = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(SIO_BAUD);
  LLMotorData.begin();
  if (LLMotorData.devReady()) {
    DBGPRINTLN("\n ready!");
  }
  else {
    DBGPRINTLN("\n Not ready!");
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  short nSIOava;
  LLMotorData.motor_loop();

  nSIOava = Serial.available();
  if (nSIOava > 0) {
    char chT;
    uint8_t nLen;
    chT = Serial.read();
    if (chT != 0) {
      Serial.printf("\n == Recv Serail Command: [ % c] == \n", chT);
    }
    switch (chT)
    {
      case '1':
        Serial.printf("\n Query Max Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_QUERY_MAX_CYCLE;
        LLMotorData.send_cmd(&LLMotorData.bufCommand);
        break;
      case '2':
        Serial.printf("\n SET Max Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_MAX_CYCLE;
        LLMotorData.bufCommand.speed = 3000;
        LLMotorData.send_cmd(&LLMotorData.bufCommand);
        break;
      case '3':
        Serial.printf("\n Query Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_QUERY_CURR_CYCLE;
        LLMotorData.send_cmd(&LLMotorData.bufCommand);
        break;
      case '4':
        Serial.printf("\n Set Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        LLMotorData.bufCommand.speed = 300;
        LLMotorData.send_cmd(&LLMotorData.bufCommand);
        break;
      case '5':
        Serial.printf("\n Set Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        LLMotorData.bufCommand.speed = 600;
        LLMotorData.send_cmd(&LLMotorData.bufCommand);
        break;
      case '0':
        Serial.printf("\n Set Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        LLMotorData.bufCommand.speed = 00;
        LLMotorData.send_cmd(&LLMotorData.bufCommand);
        break;
      case 'p':
      case 'P':
        LLMotorData.debug_print_info();
        chT = 0;
        break;
      case '\x0A':
        chT = 0;
      default:
        chT = 0;
        break;
    }

    if (chT != 0) {
      Serial.println();
      Serial.println(LLMotorData.sendBuff);
      vHexString_2ascii((uint8_t *)LLMotorData.sendBuff, dispBuff, CONST_APP_MOTOR_MSG_LENG);
      Serial.println(dispBuff);
    }
  }

  if (ulGet_interval(gulSecondTicks) > 5000) {
    //Serial.println(nSeqCount);
    /*
      Serial.printf("\n---- - recv len [ % d] ------ -\n", recvBuffLen);
      for (int i = 0; i < CONST_APP_MOTOR_MSG_LENG; i++) {
      Serial.printf("[ % 02X]", recvBuff[i]);
      }
      Serial.println("------------------------------ -");
    */
    LLMotorData.debug_print_info();
    gulSecondTicks = ulReset_interval();
  }

}
