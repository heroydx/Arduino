
//#define DEBUG_SIO Serial

//#include "LVCommonOLD.h"
#include "LVCommon.h"
//#include "ListArray.h"
#include "ESPSoftwareSerial.h"
#include "LoRaMAC.h"
//#include "FM_ZCJ.h"
#include "FM_ZCJ_SM.h"


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
  SERIAL_DEBUG_BEGIN
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
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/

#define CONST_DEVICE_SWVN "LVSW_FM_V1.0.1_20170608"
#define CONST_DEVICE_HWVN "LVHW_FM_4M1M_V1.0.0_20170310"
#define CONST_DEVICE_MAID "9"
#define CONST_DEVICE_PID "121"
#define CONST_DEVICE_CAT "fmRa"


//#define CONST_IMEI_PREFIX "8019980908"
//#define CONST_IMEI_PREFIX "8116111401"
#define CONST_IMEI_PREFIX_1 "81170"
#define CONST_IMEI_PREFIX_2 "31001"

//FM LoRa information
short gFMState;
short loRaWorkChannel = LORA_MAC_DEFAULT_CONFIG_CHANNEL;
LoRaMAC LoRaMacData;

FM_ZCJ FMDevice;
stDataStreamUnion appSendData;


#define CONST_SERVER_DATA_STREAM_ONE_DATA 1
#define CONST_SERVER_DATA_STREAM_BATCH_DATA 2

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  short seqNo;
  short CMD;
  stDataStreamUnion data;
} stServerDataStrcut;

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  short seqNo;
  short CMD;
  short batchTotal; //总共批次
  short batchNo; //本批次序号
  short len; //有效数据个数
  stDataStreamUnion list[LORA_MAC_SYNC_LIST_MAX_LEN];
} stServerBatchStrcut;


typedef union {
  stServerDataStrcut aData;
  stServerBatchStrcut batchData;
  uint8_t u8data[sizeof(stServerBatchStrcut)];
} stServerSteamUnion;

#define CONST_SERVER_SEND_BUFF_MAX_LENGTH 5
#define CONST_SERVER_RECV_BUFF_MAX_LENGTH 5

static ListArray sendSeverBuffList(CONST_SERVER_SEND_BUFF_MAX_LENGTH, sizeof(stServerSteamUnion));
static stServerSteamUnion recvSeverBuff;


unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 5*1000
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


void lora_setup()
{
  // set the data rate for the SoftwareSerial port

  LoRaMacData.begin(true, loRaWorkChannel); //true=host mode
  LoRaMacData.debug_print_self();

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
  FMDevice.begin();
  lora_setup();
}

void ICACHE_FLASH_ATTR vApplication_wait_loop_call()
{
  //FMDevice.restore_setting();
}


void handle_recv_LoRa_data()
{
  short CMD;
  short nLen;
  nLen = sizeof (stDataStreamUnion);

  CMD = LoRaMacData.recvData.exCMD.CMD;
  //public Part
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

      //DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
      //simpleHash((char *) LoRaMacData.key, (char *)  LoRaMacData.recvData.regFeedback.val, LoRaMacData.devID, LORA_MAC_KEY_LEN);

      //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
      //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);
      LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);

      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_REGISTRATION_FEEDBACK ==");
      break;
    case LORA_CMD_ADDR_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_ADDR_REQUEST ==");
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_ADDR_FEEDBACK ==");
      break;
    case LORA_CMD_SYNC_CMD:
      DBGPRINTLN("\n== LORA_CMD_SYNC_CMD ==");
      //发送FM Radio 状态信息

      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINTLN("\n== LORA_CMD_SYNC_TIME ==");
      break;
    case LORA_CMD_SYNC_RANDOM:
      DBGPRINTLN("\n== LORA_CMD_SYNC_RANDOM ==");
      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_REQUEST ==");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_FEEDBACK ==");
      break;
    case LORA_CMD_APP_FMRADIO:
      DBGPRINTLN("\n== LORA_CMD_APP_FMRADIO ==");
      FMDevice.handle_Server_to_FM_command((char *)LoRaMacData.recvData.app.data);
      break;
    default:
      DBGPRINTLN("\n== application call ==");
      //application call to call external application API
      DBGPRINTF("\napp CMD: [%d]\n", CMD);
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
  //nLen = LoRaDev.available();

  nLen = LoRaMacData.available();
  if (nLen) {
    //memcpy(appSendData.u8Data, LoRaMacData.recvData.u8Data, sizeof(stDataStreamUnion));
    LoRaMacData.debug_print_union(LoRaMacData.recvData.u8Data);
    DBGPRINTF("\n--LoRa received Data:[%d]", nLen);
    handle_recv_LoRa_data();
    //LoRaMacData.debug_print_self();
    delay(2);
  }
  FMDevice.FM_loop();
  //other application read action
  if ((ulGet_interval(gulApplicationTicks) > CONST_READ_INTERVAL))
  {
    vApplication_read_data();
    gulApplicationTicks = ulReset_interval();
  }

}



void ICACHE_FLASH_ATTR vApplication_local_timer_func()
{

}

void ICACHE_FLASH_ATTR vApplication_read_data()
{

    LoRaMacData.debug_print_self();
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
      LVDATA.push_cmd(CONST_CMD_9999);
      //digitalWrite(RELAY_ID, LOW);
      //gnStatusRelay = digitalRead(RELAY_ID);
      //LVDATA.gnServerCommand = CONST_CMD_9999;
      break;

    case 2:
      LVDATA.push_cmd(CONST_CMD_9999);
      //digitalWrite(RELAY_ID, HIGH);
      //gnStatusRelay = digitalRead(RELAY_ID);
      //LVDATA.gnServerCommand = CONST_CMD_9999;
      break;

    case 760:
      //digitalWrite(RELAY_ID, HIGH);
      LVDATA.push_cmd(CONST_CMD_9999);
      DBGPRINT("param=[");
      DBGPRINT(achParam);
      DBGPRINTLN("]");
      strncpy(FMDevice.achSysCommand, achParam, CONST_APP_FM_RECV_SYS_COMMAND_BUFFLEN);
      handle_Server_to_LoRa_command(achParam);//这个命令会破坏原来的achParam,
      FMDevice.handle_Server_to_FM_command(FMDevice.achSysCommand);
      //gnStatusRelay = digitalRead(RELAY_ID);
      //LVDATA.gnServerCommand = CONST_CMD_9999;
      LVDATA.push_cmd(CONST_CMD_3001, 3, 20);
      //LVDATA.vRefresh_data_3001();
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
  if (FMDevice.bSendSaveFreq) {
    char *strP = LVDATA.gCommandBuff.chpData;
    for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
      sprintf(strP, "%d:", FMDevice.freq_next());
      strP += strlen(strP);
    }
    *strP = '\0';
  }

  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"FM\":\"%d:%d:%d:%d:%d:%d:%d\",\"SAVE\":\"%s\",\"sioErr\":\"%d\",\"LoRaChan\":\"%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0\",\"socketOutY_W\":\"0\",\"rlySub\":[0],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, FMDevice.currStat.u8Stat, FMDevice.currStat.nFrequency, FMDevice.currStat.u8Vol, FMDevice.currStat.u8Campus, FMDevice.currStat.u8Blk, \
          FMDevice.currStat.u8DSP, FMDevice.currStat.u8SN_THR, LVDATA.gCommandBuff.chpData, FMDevice.currStat.errCount, loRaWorkChannel);
  //DBGPRINTLN(LVDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LVDATA.nCommonPOST( LVDATA.gBigBuff.chpData);
  //  ghTcpCommand.print();
  //  ghTcpCommand.print(thisData);
  FMDevice.freq_clear_send();
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


void  disp_LoRa_channel()
{
  short nLoRaChannel;
  short dispChannel;
  short nCurrFMFrequency;
  uint8_t currStat;
  nLoRaChannel = LoRaMacData.workChannel;
  nCurrFMFrequency = FMDevice.currStat.nFrequency;
  currStat = FMDevice.currStat.u8Stat;
  DBGPRINTLN("");
  DBGPRINTF("\n LoRa Channel:[%d]", nLoRaChannel);
  DBGPRINTF("\n FM: stat:[%d],freq:[%d]", currStat, nCurrFMFrequency);

  //display current LoRa Channel and turn on
  dispChannel = nLoRaChannel + 500;
  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = dispChannel;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(100);
  //FM radio: turn off
  FMDevice.allCommand.CMD.command = FM_PLAY_PAUSE;
  FMDevice.allCommand.CMD.param = 0;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);

  delay(3000);

  //restore setting
  // set freq and turn on
  FMDevice.allCommand.CMD.command = FM_SET_FRE;
  FMDevice.allCommand.CMD.param = nCurrFMFrequency;
  FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  delay(100);
  if (currStat == 0) {
    //turn off
    FMDevice.allCommand.CMD.command = FM_PLAY_PAUSE;
    FMDevice.allCommand.CMD.param = 0;
    FMDevice.send_CMD_to_FM(FMDevice.allCommand.CMD.command, FMDevice.allCommand.CMD.param);
  }
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


/*--------------------------------------------------------------------------------*/
//APPLICATION_PART_END
/*--------------------------------------------------------------------------------*/

