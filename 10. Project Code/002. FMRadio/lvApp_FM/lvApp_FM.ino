#include <FS.h>

//#define DEBUG_SIO Serial

#include "LVCommon.h"


/*================================================================================*/
//MAIN_PART_BEGIN
/*================================================================================*/
//DON'T CHANGE THE MAIN PART CODE

LVCommon LVDATA;

#ifdef DEBUG_SIO
extern char dispBuff[];
#endif


void setup()
{
#ifdef DEBUG_SIO
  SERIALBEGIN
#endif
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  vGenerate_IMEI();
  vApplication_setup_call();
  LVDATA.mainsetup();
  check_sys_data();
}

void loop()
{
  LVDATA.mainloop();
}

/*================================================================================*/
//MAIN_PART_END
/*================================================================================*/

/*====================================================================================*/
//APPLICATION_PART_BEGIN Ver=20161116
/*====================================================================================*/

/*--------------------------------------------------------------------------------*/
//APPLICATION_DATA_PART_BEGIN
/*--------------------------------------------------------------------------------*/
// 不要修改下面的CONST_DEVICE_VERSION的值,一类设备对应于一个值,修改这个设备的值,会造成软件升级失败
#define CONST_DEVICE_VERSION "9_121_20170310"
/* 软件版本解释：
 * LVSW_GATE_A_S01_V1.0.1_20170301
 * 软件_设备类型_子类型_子参数_版本_日期
 * 例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1), 
 * 硬件版本解释：
 * LVHW_GATE_4M1M_V1.0.0_20161116
 * 软件_设备类型_内存选择参数_版本_日期
 */
#define CONST_DEVICE_SWVN "LVSW_FM_V1.0.0_20170310"
#define CONST_DEVICE_HWVN "LVHW_FM_4M1M_V1.0.0_20170310"
#define CONST_DEVICE_MAID "9"
#define CONST_DEVICE_PID "121"
#define CONST_DEVICE_CAT "fmRa"


//#define CONST_IMEI_PREFIX "8019980908"
//#define CONST_IMEI_PREFIX "8116111401"
#define CONST_IMEI_PREFIX_1 "81170"
#define CONST_IMEI_PREFIX_2 "31001"

/*
  #if DEBUG_SIO == Serial
  #define APP_SERIAL Serial1
  #else
  #define APP_SERIAL Serial
  #endif
*/
//#include <arduino.h>
#include <SoftwareSerial.h>
//SoftwareSerial mySerial(10, 11); // RX D10, TX D11 Nano
//SoftwareSerial mySerial(19, 20); // RX, TX 8266 GPIO 4,5
SoftwareSerial mySerial(13, 14); // RX, TX 8266 GPIO 13，14?

#define APP_SERIAL Serial

#define CONST_APPLICATION_FILE_NAME "/application.data"
#define CONST_APPLICATION_FILE_SIZE 1024

#define CONST_APPLICATION_SIO_BAUD_DATA 38400
#define CONST_APPLICATION_SENDREV_BUFFLEN 80
#define CONST_APPLICATION_SYS_COMMAND_BUFFLEN 20
#define CONST_APPLICATION_SIO_DATA_END '\n'

#define CONST_APPLICATION_SIO_ERROR_COUNT 5

bool bSendBusyFlag = false;
uint8_t gau8sendBuff[CONST_APPLICATION_SENDREV_BUFFLEN + 2];
uint8_t gau8recvBuff[CONST_APPLICATION_SENDREV_BUFFLEN + 2];
uint8_t gu8recvLen;

#define CONST_RECV_TIMEOUT_LEN 1000*2
unsigned long glRecvTimeOutTick;
#define CONST_APPLICATION_SIO_QUERY_INTERVAL  1000*60
unsigned long glSIOQueryTick;

#define CONST_APPLICATION_FREQ_MAX_SAVE_POS 20
#define CONST_APPLICATION_COMMAND_BUFF_LEN 10
#define CONST_APPLICATION_COMMAND_PARAM_LEN 10

enum ATCOMMAND {
  FM_SET_FRE,
  FM_FRE_DOWN,
  FM_FRE_UP,
  FM_PLAY_PAUSE,
  FM_SET_VOL,
  FM_VOL_DOWN,
  FM_VOL_UP,
  FM_SET_BANK,
  FM_SET_CAMPUS,
  FM_SET_DSP,
  FM_SET_SN_THR,
  FM_RESET,
  FM_STATUS,
};

#define CONST_APPLICATION_CMD_FLAG_SYSTEM 0
#define CONST_APPLICATION_CMD_FLAG_WEBAPP 1
#define CONST_APPLICATION_CMD_FLAG_IRDA 2
#define CONST_APPLICATION_CMD_FLAG_KEY 3

typedef struct  {
  short command;
  short param;
  uint8_t flag;//system command =0,web/app command=1;
  //char param[CONST_APPLICATION_COMMAND_PARAM_LEN + 1];
} stFMCommand;


//to FM Radio FIFO command buff
//为了简化设计,考虑到命令队列不长,之间用数组模拟,并且用移动后方数据的方式。 Begin
class FMCommand
{
  public:
    FMCommand();
    uint8_t push(short command, short param, uint8_t flag);
    uint8_t pop(stFMCommand *spCommand);
    uint8_t available();
  private:
    stFMCommand gasCommand[CONST_APPLICATION_COMMAND_BUFF_LEN];
    uint8_t gu8CommandPos;
};

FMCommand::FMCommand()
{
  memset(gasCommand, 0, (sizeof(stFMCommand) * CONST_APPLICATION_COMMAND_BUFF_LEN));
  gu8CommandPos = 0;
};

uint8_t ICACHE_FLASH_ATTR FMCommand::push(short command, short param, uint8_t flag)
{
  uint8_t ret;
  if (gu8CommandPos < (CONST_APPLICATION_COMMAND_BUFF_LEN - 1)) {
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

uint8_t ICACHE_FLASH_ATTR FMCommand::pop(stFMCommand *spCommand)
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

uint8_t ICACHE_FLASH_ATTR FMCommand::available()
{
  return gu8CommandPos;
}
// class end

FMCommand gsFMDataList;

//application data structure

//FM radio machine is pingpang key, our system: 1=on,0=off
#define CONST_APPLICATION_DEFAULT_STAT 1
//campus=on: 760-1080(MHz) campus =off: 870-1080(MHz)
#define CONST_APPLICATION_FREQUENCY_HIGH 1080
#define CONST_APPLICATION_FREQUENCY_CAMPUS_LOW 760
#define CONST_APPLICATION_FREQUENCY_LOW 870
#define CONST_APPLICATION_DEFAULT_FREQUENCY 900
//volume: 0-30
#define CONST_APPLICATION_MIN_VOL 0
#define CONST_APPLICATION_MAX_VOL 30
#define CONST_APPLICATION_DEFAULT_VOL 15
//campus=0,off, 1=on
#define CONST_APPLICATION_DEFAULT_SN_CAMPUS 0
//backgroud light on time: 00-99
#define CONST_APPLICATION_MIN_BLK 0
#define CONST_APPLICATION_MAX_BLK 99
#define CONST_APPLICATION_DEFAULT_BLK 10
//DSP SN 静噪 0=off, 1=on;
#define CONST_APPLICATION_DEFAULT_SN_DSP 0
//DSP SN 静噪 阈值  00-20
#define CONST_APPLICATION_SN_MIN_THR 0
#define CONST_APPLICATION_SN_MAX_THR 20
#define CONST_APPLICATION_DEFAULT_SN_THR 10

#define CONST_APPLICATION_FM_DEVICE_VER_BUFFLEN 60

class stApplicationData
{
  public:
    //data
    short nFrequency;
    uint8_t u8Vol;
    uint8_t u8Stat;
    uint8_t u8Campus;
    uint8_t u8Blk;
    uint8_t u8SN_THR;
    uint8_t u8DSP;

    //FM Radio Frequency range;
    short nFreqHigh = CONST_APPLICATION_FREQUENCY_HIGH;
    short nFreqLow = CONST_APPLICATION_FREQUENCY_LOW;
    //FM Radio Frequency saving array
    short anSaveFreq[CONST_APPLICATION_FREQ_MAX_SAVE_POS + 2];
    uint8_t u8freqPos;
    uint8_t u8freqNum;
    char achFMDeviceVer[CONST_APPLICATION_FM_DEVICE_VER_BUFFLEN];
    bool bSendSaveFreq;
    //last sys command
    char achSysCommand[CONST_APPLICATION_SYS_COMMAND_BUFFLEN + 1];
    //last to FM command
    stFMCommand currCommand;
    stFMCommand allCommand;
    //char achFMCommand[CONST_APPLICATION_SENDREV_BUFFLEN];
    //reply data
    char achFMReply[CONST_APPLICATION_SENDREV_BUFFLEN];
    //
    short errCount;

    //function
    stApplicationData();
    short freqCurr();
    short freqNext();
    short freqPrev();
    uint8_t freqAdd(short freq);
    uint8_t freqDel();
    uint8_t freqDel(short freq);
    uint8_t freqClear();
    bool freqSetSend();
    bool freqClearSend();
    uint8_t freqNum();
    uint8_t freqSort(int8_t softFlag);
};

short ICACHE_FLASH_ATTR stApplicationData::freqCurr()
{
  return anSaveFreq[u8freqPos];
}

stApplicationData::stApplicationData()
{
  u8Stat = CONST_APPLICATION_DEFAULT_STAT;
  nFrequency = CONST_APPLICATION_DEFAULT_FREQUENCY;
  u8Vol = CONST_APPLICATION_DEFAULT_VOL;
  u8Campus = CONST_APPLICATION_DEFAULT_SN_CAMPUS;
  u8Blk = CONST_APPLICATION_DEFAULT_BLK;
  u8SN_THR = CONST_APPLICATION_DEFAULT_SN_THR;
  u8DSP = CONST_APPLICATION_DEFAULT_SN_DSP;
}

short ICACHE_FLASH_ATTR stApplicationData::freqNext()
{
  short ret;
  short i;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    u8freqPos++;
    if (u8freqPos >= CONST_APPLICATION_FREQ_MAX_SAVE_POS) {
      u8freqPos = 0;
    }
    if (anSaveFreq[u8freqPos] > 0) {
      ret = anSaveFreq[u8freqPos];
      break;
    }
  };
  return ret;
}

short ICACHE_FLASH_ATTR stApplicationData::freqPrev()
{
  short ret;
  short i;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    u8freqPos--;
    if (u8freqPos < 0 ) {
      u8freqPos = CONST_APPLICATION_FREQ_MAX_SAVE_POS;
    }
    if (anSaveFreq[u8freqPos] > 0) {
      ret = anSaveFreq[u8freqPos];
      break;
    }
  };
  return ret;
}

uint8_t ICACHE_FLASH_ATTR stApplicationData::freqAdd(short freq)
{
  short i;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    u8freqPos++;
    if (u8freqPos >= CONST_APPLICATION_FREQ_MAX_SAVE_POS) {
      u8freqPos = 0;
    }
    if (anSaveFreq[u8freqPos] == 0) {
      break;//find a empty position
    }
  };
  anSaveFreq[u8freqPos] = freq;
  return u8freqPos;
}

uint8_t ICACHE_FLASH_ATTR stApplicationData::freqDel()
{
  short i;
  anSaveFreq[u8freqPos] = 0;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[i] > 0) {
      u8freqPos = i;
      break;
    }
  }
  freqNum();
  return u8freqPos;
}

uint8_t ICACHE_FLASH_ATTR stApplicationData::freqDel(short freq)
{
  short i;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[i] == freq) {
      anSaveFreq[i] = 0;
    }
  }
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[i] > 0) {
      u8freqPos = i;
      break;
    }
  }
  freqNum();
  return u8freqPos;
}

uint8_t ICACHE_FLASH_ATTR stApplicationData::freqClear()
{
  short i;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    anSaveFreq[i] = 0;
  }
  return freqNum();
}

uint8_t ICACHE_FLASH_ATTR stApplicationData::freqNum()
{
  short i;
  u8freqNum = 0;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    if (anSaveFreq[u8freqPos] > 0) {
      u8freqNum++;
    }
  };
  return u8freqNum;
  ;
}
//为了简化设计,考虑到数组较短,采用简单排序方式
uint8_t ICACHE_FLASH_ATTR stApplicationData::freqSort(int8_t softFlag)
{
  uint8_t i, j;
  short nT1;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    for (j = i; j < CONST_APPLICATION_FREQ_MAX_SAVE_POS; j++) {
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
  DBGPRINTLN("---sort---function---");
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    DBGPRINTF("%6d", anSaveFreq[i]);
  }
  DBGPRINTLN("\n---");
  if (softFlag <= 0) {
    j = 0;
    for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
      if (anSaveFreq[i] == 0) {
        j++;
      }
      else {
        break;
      }
    }
    DBGPRINTF("\n j:[%d]\n", j);
    //move data
    for (i = 0; i < (CONST_APPLICATION_FREQ_MAX_SAVE_POS - j); i++)
    {
      anSaveFreq[i] = anSaveFreq[i + j];
    }
    //clean restdata
    for (i = 0; i < j; i++)
    {
      anSaveFreq[i + CONST_APPLICATION_FREQ_MAX_SAVE_POS - j] = 0;
    }
  }
  DBGPRINTLN("---sort---function---");
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    DBGPRINTF("%6d", anSaveFreq[i]);
  }
  DBGPRINTLN("\n---");
  u8freqPos = 0;
  return freqNum();
}

bool ICACHE_FLASH_ATTR stApplicationData::freqSetSend()
{
  bSendSaveFreq = true;
  return bSendSaveFreq;
}


bool ICACHE_FLASH_ATTR stApplicationData::freqClearSend()
{
  bSendSaveFreq = false;
  return bSendSaveFreq;
}
//class end

stApplicationData gsCurrent;

// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SAVE_CYCLE 1000*60*60
unsigned long glApplicationSaveTick;

unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 1*1000
/*--------------------------------------------------------------------------------*/
//APPLICATION_DATA_PART_END
/*--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------*/
//APPLICATION_FUCTION_PART_BEGIN
/*--------------------------------------------------------------------------------*/

void check_sys_data()
{
  int i = 0;
  DBGPRINT("Free heap in setup: ");
  DBGPRINTLN( ESP.getFreeHeap());
  DBGPRINTLN("device info:");
  DBGPRINTLN(LVDATA.gsDeviceInfo.Version);
  DBGPRINTLN(LVDATA.gsDeviceInfo.HWVN);
  DBGPRINTLN(LVDATA.gsDeviceInfo.SWVN);
  DBGPRINTLN(LVDATA.gsDeviceInfo.MAID);
  DBGPRINTLN(LVDATA.gsDeviceInfo.PID);
  DBGPRINTLN(LVDATA.gsDeviceInfo.CAT);
  DBGPRINTLN("network info:");
  DBGPRINTLN(LVDATA.ghpTcpCommand->connected());
  //DBGPRINTLN("size information");
  //for (i = 0; i < 20; i++)
  //DBGPRINTLN(LVDATA.anFreeHeapInfo[i]);
  DBGPRINTLN(LVDATA.gBigBuff.chpData);

}


void vGenerate_IMEI()
{
  //sprintf(LVDATA.IMEI, "%s%010d", CONST_IMEI_PREFIX, ESP.getChipId());
  sprintf(LVDATA.IMEI, "%s%010d%s", CONST_IMEI_PREFIX_1, ESP.getChipId(), CONST_IMEI_PREFIX_2);

}


// application init data is used in setup
void  vApplication_setup_call()
{
  strncpy(LVDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);

  //  Serial.begin(CONST_APPLICATION_SIO_BAUD_DATA);
  APP_SERIAL.begin(CONST_APPLICATION_SIO_BAUD_DATA);
  while (!APP_SERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  bLoad_application_data();
};

void ICACHE_FLASH_ATTR vRestore_FM_setting()
{
  gsFMDataList.push(FM_PLAY_PAUSE, gsCurrent.u8Stat, CONST_APPLICATION_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_FRE, gsCurrent.nFrequency, CONST_APPLICATION_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_VOL, gsCurrent.u8Vol, CONST_APPLICATION_CMD_FLAG_SYSTEM);
  gsFMDataList.push(FM_SET_CAMPUS, gsCurrent.u8Campus, CONST_APPLICATION_CMD_FLAG_SYSTEM);
}


void ICACHE_FLASH_ATTR vApplication_wait_loop_call()
{
  vRestore_FM_setting();
}

void ICACHE_FLASH_ATTR vHandle_replay_data(char *param)
{
  int nLen;
  char *spTemp;
  strcpy(LVDATA.gBigBuff.chpData, param);
  spTemp = LVDATA.gBigBuff.chpData;
  nLen = strlen(spTemp);
  DBGPRINTF("\n**-- in Handle Replay Data: %d  freeheap: %d--**", nLen, ESP.getFreeHeap());
  if (nLen < 1) {
    return;
  }
  *(spTemp + nLen - 1) = '\0';

  if (memcmp(spTemp, "ERR", 3) == 0)
  {
    //error
    gsCurrent.errCount ++;
  }
  //stat on/off
  else if (memcmp(spTemp, "PAUS", 4) == 0)  {
    gsCurrent.u8Stat = 0;
    gsCurrent.errCount = 0;
  }
  else if (memcmp(spTemp, "PLAY", 4) == 0) {
    gsCurrent.u8Stat = 1;
    gsCurrent.errCount = 0;
  }
  //frequency
  else if ((memcmp(spTemp, "FRE=", 4) == 0) && (nLen > 4)) {
    short nT1;
    nT1 = atoi(spTemp + 4);
    if ((nT1 >= gsCurrent.nFreqLow) && (nT1 <= gsCurrent.nFreqHigh)) {
      gsCurrent.nFrequency = nT1;
    }
    gsCurrent.errCount = 0;
  }
  //volume
  else if ((memcmp(spTemp, "VOL=", 4) == 0) && (nLen > 4)) {
    short nT1;
    nT1 = atoi(spTemp + 4);
    if ((nT1 >= CONST_APPLICATION_MIN_VOL) && (nT1 <= CONST_APPLICATION_MAX_VOL)) {
      gsCurrent.u8Vol = nT1;
      //DBGPRINTF("\n--Convert VOL: %d  %d\n", nT1, gsCurrent.u8Vol);
    }
    gsCurrent.errCount = 0;
  }
  //backgroud light  BANK=04s\n
  else if ((memcmp(spTemp, "BANK=", 5) == 0) && (nLen > 5)) {
    short nT1;
    *(spTemp + 7) = '\0';
    nT1 = atoi(spTemp + 5);
    DBGPRINTLN(nT1);
    if ((nT1 >= CONST_APPLICATION_MIN_BLK) && (nT1 <= CONST_APPLICATION_MAX_BLK)) {
      gsCurrent.u8Blk = nT1;
    }
    gsCurrent.errCount = 0;
  }
  //Campus
  else if ((memcmp(spTemp, "CAMPUS_ON", 9) == 0) || (memcmp(spTemp, "CAMPOS_ON", 9) == 0)) {
    gsCurrent.u8Campus = 1;
    gsCurrent.errCount = 0;
  }
  else if ((memcmp(spTemp, "CAMPUS_OFF", 10) == 0) || (memcmp(spTemp, "CAMPOS_OFF", 9) == 0)) {
    gsCurrent.u8Campus = 0;
    gsCurrent.errCount = 0;
  }
  //DSP SN on/off
  else if (memcmp(spTemp, "SN_OFF", 6) == 0) {
    gsCurrent.u8DSP = 0;
    gsCurrent.errCount = 0;
  }
  else if (memcmp(spTemp, "SN_ON", 5) == 0) {
    gsCurrent.u8DSP = 1;
    gsCurrent.errCount = 0;
  }
  //DSP threold
  else if ((memcmp(spTemp, "SN_THR=", 7) == 0) && (nLen > 7)) {
    short nT1;
    nT1 = atoi(spTemp + 7);
    if ((nT1 >= CONST_APPLICATION_SN_MIN_THR) && (nT1 <= CONST_APPLICATION_SN_MAX_THR)) {
      gsCurrent.u8SN_THR = nT1;
    }
    gsCurrent.errCount = 0;
  }
  //reset response
  else if (memcmp(spTemp, "OK", 2) == 0) {
    gsCurrent.errCount = 0;
  }
  //RM radio device ver:PCB_NUMBE:LCD_FM_RX_ENC_V1.9
  else if (memcmp(spTemp, "PCB_NUM", 7) == 0) {
    strncpy(gsCurrent.achFMDeviceVer, spTemp, CONST_APPLICATION_FM_DEVICE_VER_BUFFLEN);
    gsCurrent.errCount = 0;
  }
  //status query header/tailer
  else if (memcmp(spTemp, "/***", 4) == 0) {
    gsCurrent.errCount = 0;
  }
  //status query tailer
  else if (memcmp(spTemp, "Thank", 5) == 0) {
    gsCurrent.errCount = 0;
  }
  else {
    gsCurrent.errCount++;
  }
  DBGPRINTF("\n**-- in Handle Replay Data END: %d--**", nLen);
}

void ICACHE_FLASH_ATTR vGet_reply_data()
{
  if (APP_SERIAL.available()) {
    uint8_t chR;
    chR = APP_SERIAL.read();
    gau8recvBuff[gu8recvLen++] = chR;
    if (gu8recvLen > CONST_APPLICATION_SENDREV_BUFFLEN) {
      gu8recvLen = 0;
    }
    if (chR == '\n') {
      //got respose/reply data;
      gau8recvBuff[gu8recvLen++] = '\0';
      bSendBusyFlag = false;
      DBGPRINTLN("**--Receive SIO data--**");
      DBGPRINTF("[%s]", gau8recvBuff);
      strcpy(gsCurrent.achFMReply, (char *) gau8recvBuff);
      gu8recvLen = 0;
      vHandle_replay_data(gsCurrent.achFMReply);
    }
  }
}
void ICACHE_FLASH_ATTR vSend_FM_query_data() {
  // regular check FM radio machine status
  gsFMDataList.push(FM_STATUS, 0, CONST_APPLICATION_CMD_FLAG_SYSTEM);

}


void ICACHE_FLASH_ATTR vGet_request_data(short nType, short nParam, char *pBuff) {
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


void ICACHE_FLASH_ATTR vSend_request_data(short nType, short nParam)
{
  int i;
  int nLen;

  vGet_request_data(nType, nParam, (char *) gau8sendBuff);
  nLen = strlen((char *) gau8sendBuff);
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(gau8sendBuff[i]);
  }
  bSendBusyFlag = true;
#ifdef DEBUG_SIO
  DBGPRINT("\nSend:");
  DBGPRINT((char *)gau8sendBuff);
  DBGPRINTLN("------------------");
#endif
}


void ICACHE_FLASH_ATTR vApplication_connected_loop_call()
{
  vGet_reply_data();
  if ((LVDATA.ulGet_interval(glRecvTimeOutTick) > CONST_RECV_TIMEOUT_LEN)) {
    if (bSendBusyFlag) {
      bSendBusyFlag = false;
      gsCurrent.errCount++;
    }
  }

  if (gsFMDataList.available()) {
    if (!bSendBusyFlag) {
      stFMCommand *spTemp;
      spTemp = &gsCurrent.allCommand;
      gsFMDataList.pop(spTemp);
      if (spTemp->flag != CONST_APPLICATION_CMD_FLAG_SYSTEM) {
        memcpy(&gsCurrent.currCommand, spTemp, sizeof(gsCurrent.currCommand));
      }
      DBGPRINTLN("----Prepare to send data---------");
      DBGPRINTF("\n %d   %d\n", spTemp->command, spTemp->param);
      //send data;
      vSend_request_data(spTemp->command, spTemp->param);
      bSave_application_data();
      //receive buff clean
      gu8recvLen = 0;
      //reset timeout tick
      LVDATA.vReset_interval(glRecvTimeOutTick);
    }
  }

  if ((LVDATA.ulGet_interval(glSIOQueryTick) > CONST_APPLICATION_SIO_QUERY_INTERVAL))
  {
    vSend_FM_query_data();
    LVDATA.vReset_interval(glSIOQueryTick);
  }

  // save application data
  if ((LVDATA.ulGet_interval(glApplicationSaveTick) > CONST_APPLICATION_SAVE_CYCLE))
  {
    bSave_application_data();
    LVDATA.vReset_interval(glApplicationSaveTick);

  }

  //other application read action
  if ((LVDATA.ulGet_interval(gulApplicationTicks) > CONST_READ_INTERVAL))
  {
    vApplication_read_data();
    LVDATA.vReset_interval(gulApplicationTicks);
  }

}

void ICACHE_FLASH_ATTR vApplication_local_timer_func()
{

}

void ICACHE_FLASH_ATTR vApplication_read_data()
{
  /*
    //test
    unsigned long lT1, lT2;
    lT1 = millis() - LVDATA.glLastCommandTick;
    lT2 = millis() - LVDATA.glConnIntervalTick;
    DBGPRINTF("\nglLastCommandTick:%d millis:%d = %d %d glConnIntervalTick:%d", LVDATA.glLastCommandTick, millis(), lT1, lT2, LVDATA.glConnIntervalTick);
  */
}


void ICACHE_FLASH_ATTR vHandle_FM_command(char achParam[])
{
  short nCMDType;
  short nCMDVal;
  char *spTemp;
  spTemp = strchr(achParam, ':');
  if (spTemp == NULL) {
    return;
  }
  //DBGPRINTF("\n-- %s %s---\n", achParam, spTemp);
  *spTemp = '\0';
  spTemp++;
  nCMDType = atoi(achParam);
  nCMDVal = atoi(spTemp);
  short nFMCommand = -1;
  short nFMParam = 0;
  //bool bIsFMRadioCommand = true;
  switch (nCMDType) {
    case 0:
      //on off handle
      if (gsCurrent.u8Stat != nCMDVal) {
        nFMCommand = FM_PLAY_PAUSE;
      }
      break;
    case 1:
      break;
    case 2:
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
      if (nCMDVal >= CONST_APPLICATION_MIN_VOL && nCMDVal <= CONST_APPLICATION_MAX_VOL) {
        nFMCommand = FM_SET_VOL;
        nFMParam = nCMDVal;
      }
      break;
    case 5:
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
      switch (nCMDVal)
      {
        case 0:
          gsCurrent.freqClear();
          break;
        case 1:
          gsCurrent.freqAdd(gsCurrent.nFrequency);
          break;
        case 2:
          gsCurrent.freqDel(gsCurrent.nFrequency);
          break;
        case 3:
          gsCurrent.freqSetSend();
          break;
        case 4:
          gsFMDataList.push(FM_SET_FRE, gsCurrent.freqPrev(), CONST_APPLICATION_CMD_FLAG_SYSTEM);
          break;
        case 5:
          gsFMDataList.push(FM_SET_FRE, gsCurrent.freqNext(), CONST_APPLICATION_CMD_FLAG_SYSTEM);
          break;
        case 6:
          gsCurrent.freqSort(-1);
          break;
        case 7:
          gsCurrent.freqSort(1);
          break;
        default:
          break;
      }
      break;
    case 7:
      if (nCMDVal > 0 && nCMDVal <= CONST_APPLICATION_FREQ_MAX_SAVE_POS) {
        nFMCommand = FM_SET_FRE;
        nFMParam = gsCurrent.anSaveFreq[nCMDVal - 1];
      }
      break;
    case 8:
      nFMCommand = FM_SET_FRE;
      nFMParam = nCMDVal;
      break;
    case 9:
      DBGPRINTF("nFMParam:%d", nFMParam);
      if (nFMParam >= CONST_APPLICATION_MIN_BLK && nFMParam <= CONST_APPLICATION_MAX_BLK) {
        nFMCommand = FM_SET_BANK;
        nFMParam = nCMDVal;
      }
      break;
    case 10:
      DBGPRINTF("nFMParam:%d", nFMParam);
      if (nFMParam >= CONST_APPLICATION_SN_MIN_THR && nFMParam <= CONST_APPLICATION_SN_MAX_THR) {
        nFMCommand = FM_SET_SN_THR;
        nFMParam = nCMDVal;
      }
      break;
    default:
      break;

  }
  if (nFMCommand >= 0) {
    gsFMDataList.push(nFMCommand, nFMParam, CONST_APPLICATION_CMD_FLAG_WEBAPP);
  }
  DBGPRINTLN("--------------------");
  DBGPRINTF("nCMDType: %d  nCMDVal: %d || nFMCommand: %d  nFMParam:%d\n", nCMDType, nCMDVal, nFMCommand, nFMParam);
}

int ICACHE_FLASH_ATTR nExec_application_Command(int nActionID, char  achParam[], char achInst[])
{
  int nCommand = CONST_CMD_9999;
  DBGPRINT("exe_serverCommand=[");
  DBGPRINT(nActionID);
  DBGPRINTLN("]");

  switch (nActionID)
  {
    case 0:
      //digitalWrite(RELAY_ID, LOW);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LVDATA.gnServerCommand = CONST_CMD_9999;
      break;

    case 2:
      //digitalWrite(RELAY_ID, HIGH);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LVDATA.gnServerCommand = CONST_CMD_9999;
      break;

    case 760:
      //digitalWrite(RELAY_ID, HIGH);
      DBGPRINT("param=[");
      DBGPRINT(achParam);
      DBGPRINTLN("]");
      vHandle_FM_command(achParam);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LVDATA.gnServerCommand = CONST_CMD_9999;
      //LVDATA.vRefresh_data_3001(3);
      break;

  }

  return nCommand;
}

void application_POST_call(int nCMD)
{
  ;
}


bool ICACHE_FLASH_ATTR bSend_3001()
{
  int ret;
  DBGPRINTLN("-- send 3001 --");
  int i;
  strcpy(LVDATA.gCommandBuff.chpData, "");
  if (gsCurrent.bSendSaveFreq) {
    char *strP = LVDATA.gCommandBuff.chpData;
    for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
      sprintf(strP, "%d:", gsCurrent.anSaveFreq[i]);
      strP += strlen(strP);
    }
    *strP = '\0';
  }

  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"FM\":\"%d:%d:%d:%d:%d:%d:%d\",\"SAVE\":\"%s\",\"sioErr\":\"%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0\",\"socketOutY_W\":\"0\",\"rlySub\":[0],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, gsCurrent.u8Stat, gsCurrent.nFrequency, gsCurrent.u8Vol, gsCurrent.u8Campus, gsCurrent.u8Blk, gsCurrent.u8DSP, gsCurrent.u8SN_THR, LVDATA.gCommandBuff.chpData, gsCurrent.errCount);
  //DBGPRINTLN(LVDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LVDATA.nCommonPOST( LVDATA.gBigBuff.chpData);
  //  ghTcpCommand.print();
  //  ghTcpCommand.print(thisData);
  gsCurrent.freqClearSend();
  DBGPRINTLN("--end--");

  return true;
}

bool ICACHE_FLASH_ATTR bSend_9999()
{
  int ret;
  DBGPRINTLN("--send_9999--");
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"actionID\":\"%d\",\"delay\":\"0\",\"res\":\"%d\"}",
          CONST_CMD_9999, LVDATA.IMEI, LVDATA.gnExecActionID, 1);

  ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData, false);

  DBGPRINTLN("--end--");

  return true;
}



void ICACHE_FLASH_ATTR vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen) {
  int i, k = 0;
  char chT;
  for (i = 0; i < nLen; i++) {
    chT = pBuff[i] >> 4;
    dispP[k] = LVDATA.ascii(chT);
    k++;
    chT = pBuff[i] & 0xF;
    dispP[k] = LVDATA.ascii(chT);
    k++;
  }
  dispP[k] = 0;
}



/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR bLoad_application_data()
{
  bool bRet = false;
  int nLen;
  char *spTemp;

  File configFile = SPIFFS.open(CONST_APPLICATION_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open application data file");
    return bRet;
  }
  //determine if the right config file
  nLen = configFile.readBytesUntil('\n', LVDATA.gBigBuff.chpData, CONST_BIG_BUFF_SIZE);
  if (nLen < CONST_BIG_BUFF_SIZE && nLen > 1)
  {
    LVDATA.gBigBuff.chpData[nLen - 1] = '\0'; //trim
    spTemp = strchr(LVDATA.gBigBuff.chpData, ':');
    spTemp++;
    DBGPRINTLN(LVDATA.gBigBuff.chpData);
    if (memcmp(LVDATA.gBigBuff.chpData, "Version", 7) || memcmp(spTemp, LVDATA.gsDeviceInfo.Version, strlen(LVDATA.gsDeviceInfo.Version)))
    {
      DBGPRINTLN("Application data file is not belongs to this device");
      configFile.close();
      //bSave_config();
      return bRet;
    }

    while (true)
    {
      nLen = configFile.readBytesUntil('\n', LVDATA.gBigBuff.chpData, CONST_BIG_BUFF_SIZE);
      if (nLen <= 0)
        break;
      LVDATA.gBigBuff.chpData[nLen - 1] = '\0'; //trim
      spTemp = strchr(LVDATA.gBigBuff.chpData, ':');
      if (spTemp == NULL)
        break;//not found;
      spTemp++;

      if (memcmp(LVDATA.gBigBuff.chpData, "sta", 3) == 0) {
        gsCurrent.u8Stat = atoi(spTemp);
      }
      if (memcmp(LVDATA.gBigBuff.chpData, "fre", 3) == 0) {
        gsCurrent.nFrequency = atoi(spTemp);
      }
      if (memcmp(LVDATA.gBigBuff.chpData, "vol", 3) == 0) {
        gsCurrent.u8Vol = atoi(spTemp);
      }
      if (memcmp(LVDATA.gBigBuff.chpData, "cam", 3) == 0) {
        gsCurrent.u8Campus = atoi(spTemp);
      }
      if (memcmp(LVDATA.gBigBuff.chpData, "blk", 3) == 0) {
        gsCurrent.u8Blk = atoi(spTemp);
      }
      if (memcmp(LVDATA.gBigBuff.chpData, "snt", 3) == 0) {
        gsCurrent.u8SN_THR = atoi(spTemp);
      }
      if (memcmp(LVDATA.gBigBuff.chpData, "dsp", 3) == 0) {
        gsCurrent.u8DSP = atoi(spTemp);
      }

      if (memcmp(LVDATA.gBigBuff.chpData, "sav", 3) == 0) {
        int nPos;
        *(spTemp + 2) = '\0';
        nPos = atoi(spTemp);
        if ((nPos >= 0) && (nPos < CONST_APPLICATION_FREQ_MAX_SAVE_POS)) {
          gsCurrent.anSaveFreq[nPos] = atoi(spTemp + 3);
        }
      }
    }
    // Real world application would store these values in some variables for later use
    DBGPRINT("Loaded stat: ");
    DBGPRINT("[" );
    DBGPRINT( gsCurrent.u8Stat );
    DBGPRINTLN("]");
    DBGPRINT("Loaded frequency: ");
    DBGPRINT("[" );
    DBGPRINT( gsCurrent.nFrequency );
    DBGPRINTLN("]");

    configFile.close();
    DBGPRINTLN("Application Config ok");
  }
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR bSave_application_data()
{
  DBGPRINTLN("--save application data--");
  File configFile = SPIFFS.open(CONST_APPLICATION_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APPLICATION_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }

  configFile.print("Version:");
  configFile.println(LVDATA.gsDeviceInfo.Version);
  configFile.print("sta:");
  configFile.println(gsCurrent.u8Stat);
  configFile.print("fre:");
  configFile.println(gsCurrent.nFrequency);
  configFile.print("vol:");
  configFile.println(gsCurrent.u8Vol);
  configFile.print("cam:");
  configFile.println(gsCurrent.u8Campus);
  configFile.print("blk:");
  configFile.println(gsCurrent.u8Blk);
  configFile.print("snt:");
  configFile.println(gsCurrent.u8SN_THR);
  configFile.print("dsp:");
  configFile.println(gsCurrent.u8DSP);

  int i;
  for (i = 0; i < CONST_APPLICATION_FREQ_MAX_SAVE_POS; i++) {
    sprintf(LVDATA.gBigBuff.chpData, "sav:%02d:%d", i, gsCurrent.anSaveFreq[i]);
    configFile.println(LVDATA.gBigBuff.chpData);
  }

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}

/*--------------------------------------------------------------------------------*/
//APPLICATION_PART_END
/*--------------------------------------------------------------------------------*/

