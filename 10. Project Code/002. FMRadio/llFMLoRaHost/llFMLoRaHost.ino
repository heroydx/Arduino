//#define DEBUG_SIO Serial

#include "LLCommon.h"
#include "ListArray.h"
#include "ESPSoftwareSerial.h"
#include "LoRaMAC.h"
//#include "FM_ZCJ.h"
#include "FM_ZCJ_SM.h"
extern "C" {
  //#include "AppleBase64.h"
#include "simpleEncode.h"
}

void ICACHE_FLASH_ATTR vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen);

/*================================================================================*/
//MAIN_PART_BEGIN
/*================================================================================*/
//DON'T CHANGE THE MAIN PART CODE

LLCommon LLDATA;

#ifdef DEBUG_SIO
extern char dispBuff[];
#endif


void setup()
{
#ifdef DEBUG_SIO
  SERIAL_DEBUG_BEGIN
#endif
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  //LEDControl.begin();
  vGenerate_IMEI();
  vApplication_setup_call();
  LLDATA.mainsetup();
  check_sys_data();
}

void loop()
{
  LLDATA.mainloop();
  second_loop();
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
#define CONST_DEVICE_VERSION "8_1_20170501"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/

#define CONST_DEVICE_SWVN "LLSW_FM_V0.0.2_20171008"
#define CONST_DEVICE_HWVN "LLHW_FM_4M1M_V1.0.0_20170310"
#define CONST_DEVICE_ADDTION_HWVN CONST_LORA_HARDWARE_DEVICE
#define CONST_DEVICE_MAID "8"
#define CONST_DEVICE_PID "1"
//sub pid 必须是数字
#define CONST_DEVICE_SUB_PID 2
#define CONST_DEVICE_CAT "fmRa"

#define CONST_DEVICE_CODING_TYPE  _DEF_COMM_CODING_TYPE_JSON


//#define CONST_IMEI_PREFIX "6019980908"
//#define CONST_IMEI_PREFIX "6116111401"
#define CONST_IMEI_PREFIX_1 "61170"
//#define CONST_IMEI_PREFIX_2 "50802"
#define CONST_IMEI_PREFIX_2 "919"

//FM LoRa information
short gFMState;
short loRaWorkChannel = 425;
LoRaMAC LoRaMacData;

FM_ZCJ FMDevice;
stDataStreamUnion appSendData;
stLoRaDataServer appSendBuf;

#define CONST_MAX_MESSGE_COUNT 30
#define CONST_MAX_BATCH_SEND_TIME_IN_SECONDS 180

short gnMsgBatchTotal, gnBatchCount, gnSendBufLen, gnStatBufLen;
bool gbFistBatchFlag = true;
bool gbInIdeleStatus = true;


miscLED LEDControl;

#define CONST_SERVER_SEND_BUFF_MAX_LENGTH 16
#define CONST_SERVER_RECV_BUFF_MAX_LENGTH 16
#define CONST_SERVER_SEND_INTERVAL_TIME 60000
#define CONST_SERVER_SEND_THRESHOLD_NUM 8

//static ListArray sendSeverBuffList(CONST_SERVER_SEND_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));
//static ListArray recvSeverBuffList(CONST_SERVER_RECV_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));
static ListArray sendSeverBuffList(CONST_SERVER_SEND_BUFF_MAX_LENGTH, sizeof(stLoRaDataServer));
static ListArray recvSeverBuffList(CONST_SERVER_RECV_BUFF_MAX_LENGTH, sizeof(stLoRaDataServer));


unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 10*1000


// 信息存储class BEGIN
#define CONST_MAX_DEVICE_INFO_COUNT 256
//default no update data time=1200 seconds=20minutes
#define CONST_NO_UPDATE_DATA_TIME_IN_SECONDS 1200
#define CONST_NO_VALID_DATA_IN_LIST -1
#define CONST_MEET_END_DATA_IN_LIST -2

class fmInfo {
  public:
    //data
    short rptCount;
    short checkingSecond;
    short currPos;
    stFMDeviceRptStat rptStat[CONST_MAX_DEVICE_INFO_COUNT];
    // timeStamp表明数据的更新情况. ==0,表明数据无效
    short begin();
    short update(short pos, stFMDeviceRptStat *data);
    short clean(short pos);
    short get(short pos, stFMDeviceRptStat *data);
    short getNext(stFMDeviceRptStat *data);
    short checking();
    short getCurrPos();
    short resetCurrPos();

    //function
  private:
    //data
    //function

};

short ICACHE_FLASH_ATTR fmInfo::begin()
{
  memset(rptStat, 0, sizeof(rptStat));
}

short ICACHE_FLASH_ATTR fmInfo::update(short pos, stFMDeviceRptStat *data)
{
  short ret = CONST_NO_VALID_DATA_IN_LIST;
  if (pos < CONST_MAX_DEVICE_INFO_COUNT && pos >= 0) {
    memcpy(&rptStat[pos], data, sizeof(stFMDeviceRptStat));
    // timeStamp 表明数据的更新情况, nReset_seconds 永远返回>0的数。
    rptStat[pos].timeStamp = nReset_seconds();
    ret = pos;
  }
  return ret;
}


short ICACHE_FLASH_ATTR fmInfo::clean(short pos)
{
  short ret = CONST_NO_VALID_DATA_IN_LIST;
  if (pos < CONST_MAX_DEVICE_INFO_COUNT && pos >= 0) {
    // timeStamp ==0 表明本数据无效
    rptStat[pos].timeStamp = 0;
    ret = pos;
  }
  return ret;
}

short ICACHE_FLASH_ATTR fmInfo::get(short pos, stFMDeviceRptStat *data)
{
  short ret = CONST_NO_VALID_DATA_IN_LIST;
  if (pos < CONST_MAX_DEVICE_INFO_COUNT && pos >= 0) {
    memcpy(data, &rptStat[pos], sizeof(stFMDeviceRptStat));
    ret = pos;
  }
  return ret;
}

//ret>=0: valid data
//ret==-1: no data
//ret==-2: to end
short ICACHE_FLASH_ATTR fmInfo::getNext(stFMDeviceRptStat *data)
{
  short ret = CONST_NO_VALID_DATA_IN_LIST;
  while (currPos < CONST_MAX_DEVICE_INFO_COUNT) {
    if (nGet_seconds(rptStat[currPos].timeStamp) > CONST_NO_UPDATE_DATA_TIME_IN_SECONDS) {
      // timeStamp ==0 表明本数据无效
      rptStat[currPos].timeStamp = 0;
    }
    if (rptStat[currPos].timeStamp > 0) {
      //found valid data
      memcpy(data, &rptStat[currPos], sizeof(stFMDeviceRptStat));
      ret = currPos;
      currPos++;
      break;
    }
    currPos++;
  }
  if (currPos == CONST_MAX_DEVICE_INFO_COUNT) {
    ret = CONST_MEET_END_DATA_IN_LIST;
  }
  return ret;
}


short ICACHE_FLASH_ATTR fmInfo::checking()
{
  short ret = -1;
  short pos;
  checkingSecond = nReset_seconds();
  rptCount = 0;
  for (pos = 0; pos < CONST_MAX_DEVICE_INFO_COUNT; pos++) {
    if (nGet_seconds(rptStat[pos].timeStamp) > CONST_NO_UPDATE_DATA_TIME_IN_SECONDS) {
      // timeStamp ==0 表明本数据无效
      rptStat[pos].timeStamp = 0;
    }
    if (rptStat[pos].timeStamp != 0) {
      //有效数据计数
      rptCount++;
    }
  }
  return rptCount;
}


short ICACHE_FLASH_ATTR fmInfo::getCurrPos()
{
  return currPos;
}

short ICACHE_FLASH_ATTR fmInfo::resetCurrPos()
{
  currPos = 0;
  return currPos;
}

fmInfo fmStatData;

// 信息存储class END

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
  DBGPRINTLN(LLDATA.gsDeviceInfo.Version);
  DBGPRINTLN(LLDATA.gsDeviceInfo.HWVN);
  DBGPRINTLN(LLDATA.gsDeviceInfo.SWVN);
  DBGPRINTLN(LLDATA.gsDeviceInfo.MAID);
  DBGPRINTLN(LLDATA.gsDeviceInfo.PID);
  DBGPRINTLN(LLDATA.gsDeviceInfo.CAT);
  DBGPRINTLN("network info:");
  DBGPRINTLN(LLDATA.ghpTcpCommand->connected());
  //DBGPRINTLN("size information");
  //for (i = 0; i < 20; i++)
  //DBGPRINTLN(LLDATA.anFreeHeapInfo[i]);
  DBGPRINTLN(LLDATA.gBigBuff.chpData);

}


void vGenerate_IMEI()
{
  vGenerate_IMEI(LLDATA.IMEI, ESP.getChipId());
}

void vGenerate_IMEI(char *out, unsigned long id)
{
  uint8_t nT1;
  nT1 = random(99);
  sprintf(out, "%s%010d%s%02d", CONST_IMEI_PREFIX_1, id, CONST_IMEI_PREFIX_2, nT1);
}


void lora_setup()
{
  // set the data rate for the SoftwareSerial port
  DBGPRINTLN("\nlora_setup");
  if (LLDATA.gsDeviceInfo.server.loraChannel == 0) {
    DBGPRINTF("\nLora_setup: [%d]", LLDATA.gsDeviceInfo.server.loraChannel);
    LoRaMacData.begin(true); //true=host mode
  }
  else {
    loRaWorkChannel = LLDATA.gsDeviceInfo.server.loraChannel;
    DBGPRINTF("\nLora_setup: [%d]", loRaWorkChannel);
    LoRaMacData.begin(true, loRaWorkChannel); //true=host mode
  }
  LoRaMacData.debug_print_self();
}

void vThird_party_setup_call()
{
  lora_setup();
}

// application init data is used in setup
void  vApplication_setup_call()
{
  short nLen;
  strncpy(LLDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  nLen = strlen(LLDATA.gsDeviceInfo.HWVN);
  strncpy(LLDATA.gsDeviceInfo.HWVN + nLen, CONST_DEVICE_ADDTION_HWVN, CONST_SYS_VERSION_LENGTH - nLen - 1);
  strncpy(LLDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);
  LLDATA.gsDeviceInfo.SUBPID = CONST_DEVICE_SUB_PID;
  FMDevice.begin();
  //lora_setup();
  //strncpy(LLDATA.codingType, CONST_DEVICE_CODING_TYPE, _DEF_COMM_CODING_TYPE_LENGTH);
  strncpy(LLDATA.codingType, _DEF_COMM_CODING_TYPE_B6401, _DEF_COMM_CODING_TYPE_LENGTH);
}

void ICACHE_FLASH_ATTR vApplication_wait_loop_call()
{
  //FMDevice.restore_setting();
}

void func_LORA_CMD_REGISTRATION_REQUEST()
{
  DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

  DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
  simpleHash((char *) LoRaMacData.key, (char *)  LoRaMacData.sendData.regFeedback.val, LoRaMacData.devID, LORA_MAC_KEY_LEN);

  //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
  //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);

  //send to server buff list
  appSendBuf.reg.type = CONST_LORA_REPORT_TYPE_REGISTRATION;
  appSendBuf.reg.devID = LoRaMacData.get_devID(LoRaMacData.sendData.regFeedback.addr);
  memcpy(appSendBuf.reg.addr, LoRaMacData.sendData.regFeedback.addr, LORA_MAC_ADDRESS_LEN);
  appSendBuf.reg.MAID = atoi(LLDATA.gsDeviceInfo.MAID);
  appSendBuf.reg.PID = LLDATA.gsDeviceInfo.SUBPID;

  //send data to client by lora
  sendSeverBuffList.push(&appSendBuf);
  DBGPRINTF("\n -- sendSeverBuffList [ % d]", sendSeverBuffList.len());
  DBGPRINTF("\n devID[ % d], MAID[ % d], PID[ % d], ", appSendBuf.reg.devID, appSendBuf.reg.MAID, appSendBuf.reg.PID);

  LoRaMacData.debug_print_union(LoRaMacData.sendData.u8Data);
}

void func_LORA_CMD_APP_FMRADIO()
{
  FMCommandUnion fmData;
  stFMDeviceRptStat stRpt;
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  short pos;
  //DBGPRINTLN("\n == LORA_CMD_APP_FMRADIO == ");
  memcpy(sourceAddr, LoRaMacData.recvData.app.sourceAddr, LORA_MAC_ADDRESS_LEN);
  pos = sourceAddr[LORA_MAC_ADDRESS_LEN - 1] & 0xFF;

  memcpy(fmData.u8Data, LoRaMacData.recvData.app.data, sizeof(FMCommandUnion));

  DBGPRINTF("\n -- fmData.stat.command: [ % d]", fmData.stat.command);
  //vHexString_2ascii(sourceAddr, dispBuff, sizeof(sourceAddr));
  //DBGPRINTF("\n sourceAddr[ % s]", dispBuff);
#ifdef DEBUG_SIO
  vHexString_2ascii(fmData.stat.u8statString, dispBuff, CONST_APP_FM_STAT_STRING_LENGTH);
#endif
  DBGPRINTF("\n fmData u8Data[ % s]", dispBuff);
  //update FM radio status
  switch (fmData.stat.command)
  {
    case FM_STAT_REPORT:
      //FMDevice.copy2_rpt_data(&stRpt, (uint8_t *)LoRaMacData.recvData.app.data);
      FMDevice.copy2_rpt_data(&stRpt, fmData.stat.u8statString);
      DBGPRINTF("\n -- FMDevice: [ % d] [ % d] [ % d]", FMDevice.statBuf.nFrequency, FMDevice.statBuf.u8Vol, FMDevice.statBuf.u8Stat);
      fmStatData.update(pos, &stRpt);
      DBGPRINTF("\n -- FM_STAT_REPORT: pos[ % d], [ % d] [ % d] [ % d]", pos, stRpt.nFrequency, stRpt.u8Vol, stRpt.u8Stat);
      break;
    default:
      break;
  }

}

//处理收到的LoRa数据
void handle_recv_LoRa_data()
{
  short CMD;
  short nLen;
  //stLoRaDataServer sendBuf;
  nLen = sizeof (stDataStreamUnion);

  CMD = LoRaMacData.recvData.exCMD.CMD;
  //public Part
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      func_LORA_CMD_REGISTRATION_REQUEST();
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINTLN("\n == LORA_CMD_REGISTRATION_FEEDBACK == ");
      break;
    case LORA_CMD_ADDR_REQUEST:
      DBGPRINTLN("\n == LORA_CMD_ADDR_REQUEST == ");
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      DBGPRINTLN("\n == LORA_CMD_ADDR_FEEDBACK == ");
      break;
    case LORA_CMD_SYNC_CMD:
      DBGPRINTLN("\n == LORA_CMD_SYNC_CMD == ");
      //发送FM Radio 状态信息

      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINTLN("\n == LORA_CMD_SYNC_TIME == ");
      break;
    case LORA_CMD_SYNC_RANDOM:
      DBGPRINTLN("\n == LORA_CMD_SYNC_RANDOM == ");
      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINTLN("\n == LORA_CMD_VERIFY_REQUEST == ");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINTLN("\n == LORA_CMD_VERIFY_FEEDBACK == ");
      break;
    case LORA_CMD_APP_FMRADIO:
      DBGPRINTLN("\n == LORA_CMD_APP_FMRADIO == ");
      func_LORA_CMD_APP_FMRADIO();
      break;
    default:
      DBGPRINTLN("\n == application call == ");
      //application call to call external application API
      DBGPRINTF("\napp CMD: [ % d]\n", CMD);
      break;
  }
}

void handle_Server_to_LoRa_command(char achParam[])
{
  /*
    short nLen;
    nLen = strlen(achParam);
    if (nLen < LORA_MAC_APPLICATION_MAX_DATA_LEN - 1) {
    appSendData.app.CMD = LORA_CMD_APP_FMRADIO;
    memcpy(appSendData.app.sourceAddr, LoRaMacData.hostAddr, LORA_MAC_ADDRESS_LEN);
    memcpy(appSendData.app.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);

    strncpy((char *)appSendData.app.data, achParam, LORA_MAC_APPLICATION_MAX_DATA_LEN - 1);

    LoRaMacData.send_data(appSendData.u8Data);
    DBGPRINTLN("\n---------- handle_Server_to_LoRa_command ------------");
    }
  */

  short nFMCommand = -1;
  short nFMParam = 0;

  if (FMDevice.trans_Server_to_Inter_FM_command(achParam, &nFMCommand, &nFMParam)) {
    appSendData.app.CMD = LORA_CMD_APP_FMRADIO;
    memcpy(appSendData.app.sourceAddr, LoRaMacData.hostAddr, LORA_MAC_ADDRESS_LEN);
    memcpy(appSendData.app.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
    FMDevice.allCommand.CMD.command = nFMCommand;
    FMDevice.allCommand.CMD.param = nFMParam;
    FMDevice.allCommand.CMD.flag = CONST_APP_FM_RECV_CMD_FLAG_WEBAPP;
    memcpy(appSendData.app.data, FMDevice.allCommand.u8Data, sizeof(FMDevice.allCommand.u8Data));
    LoRaMacData.send_data(appSendData.u8Data);
    DBGPRINTLN("\n---------- handle_Server_to_LoRa_command ------------");
  };
}


void ICACHE_FLASH_ATTR vApplication_connected_loop_call()
{
  short nLen = 0;

  nLen = LoRaMacData.available();
  if (nLen) {
    //memcpy(appSendData.u8Data, LoRaMacData.recvData.u8Data, sizeof(stDataStreamUnion));
    LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);
    DBGPRINTLN("\n--LoRa received Data");
    DBGPRINTLN(nLen);
    handle_recv_LoRa_data();
    LoRaMacData.debug_print_self();
    delay(2);
  }

  FMDevice.FM_loop();

  //if ((ulGet_interval(gulApplicationTicks) > LLDATA.glConnIntervalTick) || (sendSeverBuffList.len() >= CONST_SERVER_SEND_THRESHOLD_NUM))
  if ((ulGet_interval(gulApplicationTicks) > LLDATA.glConnIntervalTick))
  {
    //test data,should be removed in future.
    //vApplication_read_data();
    DBGPRINTF("\n glConnIntervalTick: [ % d] ", LLDATA.glConnIntervalTick);
    gnMsgBatchTotal = 0;

    gnSendBufLen = sendSeverBuffList.len() ;
    if (gnSendBufLen > 0) {
      gnMsgBatchTotal = (gnSendBufLen + CONST_MAX_MESSGE_COUNT - 1) / CONST_MAX_MESSGE_COUNT;
    }
    DBGPRINTF("\n gnSendBufLen: [ % d], gnMsgBatchTotal: [ % d] ", gnSendBufLen, gnMsgBatchTotal);

    fmStatData.checking();
    if (fmStatData.getCurrPos() == CONST_MAX_DEVICE_INFO_COUNT)
    {
      //meet the end of data list
      fmStatData.resetCurrPos();
    }

    gnMsgBatchTotal += (fmStatData.rptCount + CONST_MAX_MESSGE_COUNT - 1) / CONST_MAX_MESSGE_COUNT;
    DBGPRINTF("\n fmStatData.rptCount: [ % d], gnMsgBatchTotal: [ % d]", fmStatData.rptCount, gnMsgBatchTotal);
    gnStatBufLen = fmStatData.rptCount;

    if (gnMsgBatchTotal > 0) {
      short u;
      LLDATA.push_cmd(CONST_CMD_2001, 0, CONST_MAX_BATCH_SEND_TIME_IN_SECONDS);
      for (u = 0; u < gnMsgBatchTotal; u++) {
        //防止2001合并命令
        LLDATA.push_cmd(CONST_CMD_2001, 1, CONST_MAX_BATCH_SEND_TIME_IN_SECONDS);
      }
    }
    else {
      LLDATA.push_cmd(CONST_CMD_2001, 0, 60);
    }
    gnBatchCount = 0;
    gulApplicationTicks = ulReset_interval();
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
    lT1 = millis() - LLDATA.glLastCommandTick;
    lT2 = millis() - LLDATA.glConnIntervalTick;
    DBGPRINTF("\nglLastCommandTick: % d millis: % d = % d % d glConnIntervalTick: % d", LLDATA.glLastCommandTick, millis(), lT1, lT2, LLDATA.glConnIntervalTick);
  */
  appSendBuf.reg.type = CONST_LORA_REPORT_TYPE_REGISTRATION;
  appSendBuf.reg.devID = 12345678;
  appSendBuf.reg.addr[0] = 0xC0;
  appSendBuf.reg.addr[1] = 0xA8;
  appSendBuf.reg.addr[2] = 0x0;
  appSendBuf.reg.addr[3] = 0x10;

  appSendBuf.reg.MAID = atoi(LLDATA.gsDeviceInfo.MAID);
  appSendBuf.reg.PID = LLDATA.gsDeviceInfo.SUBPID;

  sendSeverBuffList.push(appSendBuf.u8Data);

  //DBGPRINTLN("\n active loop");
  short pos;
  pos = appSendBuf.reg.addr[3];
  stFMDeviceRptStat stRpt;
  stRpt.nFrequency = 900;
  stRpt.u8Vol = 15;
  stRpt.u8Stat = 1;
  stRpt.u8Campus = 1;
  stRpt.u8Blk = 10;
  stRpt.u8SN_THR = 1;
  stRpt.u8DSP = 20;

  fmStatData.update(pos, &stRpt);
}

int ICACHE_FLASH_ATTR nExec_transfer_Command(char achData[])
{
  ;
}

int ICACHE_FLASH_ATTR nExec_application_Command(int nActionID, char  achParam[], char achInst[])
{
  int nCommand = CONST_CMD_9999;
  DBGPRINT("exe_serverCommand = [");
  DBGPRINT(nActionID);
  DBGPRINTLN("]");

  switch (nActionID)
  {
    case 0:
      //digitalWrite(RELAY_ID, LOW);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LLDATA.push_cmd(CONST_CMD_9999);
      break;

    case 2:
      //digitalWrite(RELAY_ID, HIGH);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LLDATA.push_cmd(CONST_CMD_9999);
      break;

    case 760:
      //digitalWrite(RELAY_ID, HIGH);
      DBGPRINT("param = [");
      DBGPRINT(achParam);
      DBGPRINTLN("]");
      strncpy(FMDevice.achSysCommand, achParam, CONST_APP_FM_RECV_SYS_COMMAND_BUFFLEN);

      handle_Server_to_LoRa_command(achParam);//这个命令会破坏原来的achParam,

      FMDevice.handle_Server_to_FM_command(FMDevice.achSysCommand);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LLDATA.push_cmd(CONST_CMD_9999);
      //LLDATA.vRefresh_data_status(3);
      break;

  }

  return nCommand;
}

void application_POST_call(int nCMD)
{
  ;
}

//uint8_t simpleEncodeBuf[sizeof(stDataStreamUnion) + 8];


bool ICACHE_FLASH_ATTR bSend_status()
{
  int ret;
  int i;
  short nEachBatchMax;

  LLDATA.SN++;
  DBGPRINTF("\n-- send 2001 --  SN[ % d]", LLDATA.SN);

  if (gnMsgBatchTotal > 0)
  {
    //do have data to send

    if ( gnSendBufLen > 0) {
      //stLoRaDataServer sendBuf;
      // send sendBufData
      gnSendBufLen -= CONST_APP_LORA_MAX_REPORT_PKG_LEN;
      if (gnSendBufLen > 0) {
        nEachBatchMax = CONST_APP_LORA_MAX_REPORT_PKG_LEN;
      }
      else {
        nEachBatchMax = gnSendBufLen + CONST_APP_LORA_MAX_REPORT_PKG_LEN;
      }

      strcpy(LLDATA.gCommandBuff.chpData, "[");

      for (i = 0; i < nEachBatchMax; i++)
      {
        char *ptr;
        short nLen;
        //using simpleEncode method to encode data
        ptr = LLDATA.gBigBuff.chpData;
        *ptr = '\"';
        ptr++;
        //sendSeverBuffList.pop(appSendData.u8Data);
        sendSeverBuffList.pop(appSendBuf.u8Data);

#ifdef DEBUG_SIO
        //vHexString_2ascii(appSendData.u8Data, dispBuff, sizeof(stDataStreamUnion));
        vHexString_2ascii(appSendBuf.u8Data, dispBuff, sizeof(appSendBuf));
        DBGPRINTF("\nin 2001 u8Data:devID [ % d], [ % s]", appSendBuf.reg.devID, dispBuff);
#endif

        //nLen = simple_encode_data(simpleEncodeBuf, (unsigned char *) appSendData.u8Data, sizeof(stDataStreamUnion));
        //nLen = simple_encode_data(simpleEncodeBuf, (unsigned char *) &sendBuf, sizeof(stDataStreamUnion));
#ifdef DEBUG_SIO
        //vHexString_2ascii(simpleEncodeBuf, dispBuff, nLen);
        //DBGPRINTF("\nin 2001 simpleEncodeBuf:[ % s]", dispBuff);
#endif
        //Base64encode(ptr, (char *)simpleEncodeBuf, nLen);
        nLen = LoRaMacData.cal_server_data_len(appSendBuf.reg.type);
        Base64encode(ptr, (char *)appSendBuf.u8Data, nLen);

        DBGPRINTF("\nin 2001 base64:[ % s]", ptr);
        if (i == nEachBatchMax - 1) {
          strcat(ptr, "\"");
        }
        else {
          strcat(ptr, "\",");
        }
        DBGPRINTF("\nin 2001 base64 after :[%s]", ptr);
        strcat(LLDATA.gCommandBuff.chpData, LLDATA.gBigBuff.chpData);
      }

      strcat(LLDATA.gCommandBuff.chpData, "]");

      DBGPRINTF("\n reg:LLDATA.gCommandBuff.chpData:[%s]", LLDATA.gCommandBuff.chpData);


    }
    else {
      //send stat data
      if ( gnStatBufLen > 0) {
        char *ptr;
        ptr = LLDATA.gCommandBuff.chpData;
        // send stat Data
        nEachBatchMax = 0; //nEachBatchMax use as a count
        strcpy(ptr, "[");
        ptr++;
        while (true)
        {
          short pos;
          stFMDeviceRptStat stRpt;
          //get next valid stat data
          pos = fmStatData.getNext(&stRpt);
          if (pos >= 0) {
            short nLen;
            //status report message
            appSendBuf.stat.type = CONST_LORA_REPORT_TYPE_STATUS;
            appSendBuf.stat.devID = LoRaMacData.get_devID(pos);
            memcpy(appSendBuf.stat.data, &stRpt, sizeof(stRpt));

            //valid data;
            *ptr = '\"';
            ptr++;
            nLen = LoRaMacData.cal_server_data_len(appSendBuf.stat.type);
            Base64encode(ptr, (char *)appSendBuf.u8Data, nLen);
            strcat(ptr, "\",");
            ptr += strlen(ptr);
            nEachBatchMax++;

          }
          else {
            // no data to send
            /*
              if (pos == CONST_MEET_END_DATA_IN_LIST) {
              //meet the end of data list
              fmStatData.resetCurrPos();
              }
            */
            break;
          }
          if (nEachBatchMax >= CONST_APP_LORA_MAX_REPORT_PKG_LEN) {
            //exceed the max send buff
            break;
          }
        }
        if (*(ptr - 1) == ',') {
          // 至少有一组数据,否则这里='['
          *(ptr - 1) = '\0'; //remove last ','
        }
        strcat(LLDATA.gCommandBuff.chpData, "]");
        DBGPRINTF("\n stat:LLDATA.gCommandBuff.chpData:[%s]", LLDATA.gCommandBuff.chpData);
      }
    }
  }
  else {
    //no data to send,just send a regular msg
    //sprintf(LLDATA.gCommandBuff.chpData."%s","[]");
    strcpy(LLDATA.gCommandBuff.chpData, "[]");
  }

  //old 2001
  /*
    sprintf(LLDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"FM\":\"%d:%d:%d:%d:%d:%d:%d\",\"LEN\":\"%d\",\"SAVE\":%s,\"sioErr\":\"%d\",\"LoRaChan\":\"%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0\",\"socketOutY_W\":\"0\",\"rlySub\":[0],\"rlystatus\":[0]}",
          CONST_CMD_2001, LLDATA.IMEI, LLDATA.gsDeviceInfo.CAT, FMDevice.currStat.u8Stat, FMDevice.currStat.nFrequency, FMDevice.currStat.u8Vol, FMDevice.currStat.u8Campus, FMDevice.currStat.u8Blk, \
          FMDevice.currStat.u8DSP, FMDevice.currStat.u8SN_THR, nBuffLen, LLDATA.gCommandBuff.chpData, FMDevice.currStat.errCount, loRaWorkChannel);
  */

  sprintf (LLDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"reqID\":\"%s\",\"rtnID\":\"%s\",\"SN\":\"%d\",\"rptType\":\"%s\",\"LoRaChan\":%d,\"rptData\":%s}",
           CONST_CMD_2001, LLDATA.IMEI, LLDATA.IMEI, LLDATA.SN, LLDATA.gsDeviceInfo.CAT, LoRaMacData.workChannel, LLDATA.gCommandBuff.chpData);
  DBGPRINTF("\n LLDATA.gBigBuff.chpData:[%s]", LLDATA.gBigBuff.chpData);
  // encode and decode
  LLDATA.encode_data(LLDATA.codingType, strlen(LLDATA.gBigBuff.chpData), LLDATA.gBigBuff.chpData, LLDATA.gCommandBuff.chpData);

  //sprintf(LLDATA.gBigBuff.chpData, "{\"codingType\":\"%s\",\"data\":%s}", LLDATA.codingType, LLDATA.gCommandBuff.chpData);

  //DBGPRINTLN(LLDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LLDATA.nCommonPOST( LLDATA.gBigBuff.chpData);
  //  ghTcpCommand.print();
  //  ghTcpCommand.print(thisData);
  FMDevice.freq_clear_send();
  DBGPRINTLN("--end--");

  return true;

}

bool ICACHE_FLASH_ATTR bSend_feedback_OK()
{
  int ret;
  DBGPRINTLN("--send_9999--");
  sprintf(LLDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"actionID\":\"%d\",\"delay\":\"0\",\"res\":\"%d\"}",
          CONST_CMD_9999, LLDATA.IMEI, LLDATA.gnExecActionID, 1);

  ret = LLDATA.nCommonPOST(LLDATA.gBigBuff.chpData, false);

  DBGPRINTLN("--end--");

  return true;
}


void ICACHE_FLASH_ATTR vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen)
{
  int i, k = 0;
  char chT;
  for (i = 0; i < nLen; i++) {
    chT = pBuff[i] >> 4;
    dispP[k] = ascii(chT);
    k++;
    chT = pBuff[i] & 0xF;
    dispP[k] = ascii(chT);
    k++;
  }
  dispP[k] = 0;
}


/*--------------------------------------------------------------------------------*/
//APPLICATION_PART_END
/*--------------------------------------------------------------------------------*/

