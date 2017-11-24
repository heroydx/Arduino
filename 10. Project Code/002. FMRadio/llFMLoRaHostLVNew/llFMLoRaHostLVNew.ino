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
  vGenerate_IMEI();
  vApplication_setup_call();
  LLDATA.mainsetup();
  check_sys_data();
}

void loop()
{
  LLDATA.mainloop();
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
#define CONST_DEVICE_VERSION "9_121_20170508"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/

#define CONST_DEVICE_SWVN "LVSW_FM_V1.0.0_20170310"
#define CONST_DEVICE_HWVN "LVHW_FM_4M1M_V1.0.0_20170310"
#define CONST_DEVICE_ADDTION_HWVN CONST_LORA_HARDWARE_DEVICE
#define CONST_DEVICE_MAID "9"
#define CONST_DEVICE_PID "121"
#define CONST_DEVICE_CAT "fmRa"


//#define CONST_IMEI_PREFIX "8019980908"
//#define CONST_IMEI_PREFIX "8116111401"
#define CONST_IMEI_PREFIX_1 "81170"
#define CONST_IMEI_PREFIX_2 "50802"

//FM LoRa information
short gFMState;
short loRaWorkChannel = 433;
LoRaMAC LoRaMacData;

FM_ZCJ FMDevice;
stDataStreamUnion appSendData;

#define CONST_SERVER_SEND_BUFF_MAX_LENGTH 16
#define CONST_SERVER_RECV_BUFF_MAX_LENGTH 16
#define CONST_SERVER_SEND_INTERVAL_TIME 60000
#define CONST_SERVER_SEND_THRESHOLD_NUM 8

static ListArray sendSeverBuffList(CONST_SERVER_SEND_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));
static ListArray recvSeverBuffList(CONST_SERVER_RECV_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));

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
  //sprintf(LLDATA.IMEI, "%s%010d", CONST_IMEI_PREFIX, ESP.getChipId());
  sprintf(LLDATA.IMEI, "%s%010d%s", CONST_IMEI_PREFIX_1, ESP.getChipId(), CONST_IMEI_PREFIX_2);

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
  short nLen;
  strncpy(LLDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  nLen = strlen(LLDATA.gsDeviceInfo.HWVN);
  strncpy(LLDATA.gsDeviceInfo.HWVN + nLen, CONST_DEVICE_ADDTION_HWVN, CONST_SYS_VERSION_LENGTH - nLen - 1);
  strncpy(LLDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);
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

      DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
      simpleHash((char *) LoRaMacData.key, (char *)  LoRaMacData.sendData.regFeedback.val, LoRaMacData.devID, LORA_MAC_KEY_LEN);

      //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
      //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);

      //send to server buff list
      sendSeverBuffList.push(LoRaMacData.sendData.u8Data);

      LoRaMacData.debug_print_union(LoRaMacData.sendData.u8Data);

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

      //send to server buff list
      sendSeverBuffList.push(LoRaMacData.recvData.u8Data);

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
    Serial.println("\n--LoRa received Data");
    Serial.println(nLen);
    handle_recv_LoRa_data();
    LoRaMacData.debug_print_self();
    delay(2);
  }
  FMDevice.FM_loop();

  //check if need to send data to server
  if (sendSeverBuffList.len() >= CONST_SERVER_SEND_THRESHOLD_NUM) {
    LLDATA.vRefresh_data_3001();
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
    DBGPRINTF("\nglLastCommandTick:%d millis:%d = %d %d glConnIntervalTick:%d", LLDATA.glLastCommandTick, millis(), lT1, lT2, LLDATA.glConnIntervalTick);
  */
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
      LLDATA.gnServerCommand = CONST_CMD_9999;
      break;

    case 2:
      //digitalWrite(RELAY_ID, HIGH);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LLDATA.gnServerCommand = CONST_CMD_9999;
      break;

    case 760:
      //digitalWrite(RELAY_ID, HIGH);
      DBGPRINT("param=[");
      DBGPRINT(achParam);
      DBGPRINTLN("]");
      strncpy(FMDevice.achSysCommand, achParam, CONST_APP_FM_RECV_SYS_COMMAND_BUFFLEN);

      handle_Server_to_LoRa_command(achParam);//这个命令会破坏原来的achParam,

      FMDevice.handle_Server_to_FM_command(FMDevice.achSysCommand);
      //gnStatusRelay = digitalRead(RELAY_ID);
      LLDATA.gnServerCommand = CONST_CMD_9999;
      //LLDATA.vRefresh_data_3001(3);
      break;

  }

  return nCommand;
}

void application_POST_call(int nCMD)
{
  ;
}

uint8_t simpleEncodeBuf[sizeof(stDataStreamUnion) + 8];


bool ICACHE_FLASH_ATTR bSend_3001()
{
  int ret;
  DBGPRINTLN("-- send 3001 --");
  int i;
  short nBuffLen;
  nBuffLen = sendSeverBuffList.len();
  strcpy(LLDATA.gCommandBuff.chpData, "[");
  for (i = 0; i < nBuffLen; i++)
  {
    char *ptr;
    short nLen;
    //using simpleEncode method to encode data
    ptr = LLDATA.gBigBuff.chpData;
    *ptr = '\"';
    ptr++;
    sendSeverBuffList.pop(appSendData.u8Data);
#ifdef DEBUG_SIO
    vHexString_2ascii(appSendData.u8Data, dispBuff, sizeof(stDataStreamUnion));
    DBGPRINTF("\nin 3001 u8Data:[%s]", dispBuff);
#endif
    nLen = simple_encode_data(simpleEncodeBuf, (unsigned char *) appSendData.u8Data, sizeof(stDataStreamUnion));
#ifdef DEBUG_SIO
    vHexString_2ascii(simpleEncodeBuf, dispBuff, nLen);
    DBGPRINTF("\nin 3001 simpleEncodeBuf:[%s]", dispBuff);
#endif
    Base64encode(ptr, (char *)simpleEncodeBuf, nLen);
    DBGPRINTF("\nin 3001 base64:[%s]", ptr);
    if (i == nBuffLen - 1) {
      strcat(ptr, "\"");
    }
    else {
      strcat(ptr, "\",");
    }
    DBGPRINTF("\nin 3001 base64 after :[%s]", ptr);
    strcat(LLDATA.gCommandBuff.chpData, LLDATA.gBigBuff.chpData);
  }
  strcat(LLDATA.gCommandBuff.chpData, "]");
  DBGPRINTF("\nLLDATA.gCommandBuff.chpData:[%s]", LLDATA.gCommandBuff.chpData);


  //old 3001
  /*
    sprintf(LLDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"FM\":\"%d:%d:%d:%d:%d:%d:%d\",\"LEN\":\"%d\",\"SAVE\":%s,\"sioErr\":\"%d\",\"LoRaChan\":\"%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0\",\"socketOutY_W\":\"0\",\"rlySub\":[0],\"rlystatus\":[0]}",
          CONST_CMD_3001, LLDATA.IMEI, LLDATA.gsDeviceInfo.CAT, FMDevice.currStat.u8Stat, FMDevice.currStat.nFrequency, FMDevice.currStat.u8Vol, FMDevice.currStat.u8Campus, FMDevice.currStat.u8Blk, \
          FMDevice.currStat.u8DSP, FMDevice.currStat.u8SN_THR, nBuffLen, LLDATA.gCommandBuff.chpData, FMDevice.currStat.errCount, loRaWorkChannel);
  */
  sprintf(LLDATA.gBigBuff.chpData, "{\"data\":%s,\"pushID\":\"192.168.0.1\"}",
          LLDATA.gCommandBuff.chpData);

  //DBGPRINTLN(LLDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LLDATA.nCommonPOST( LLDATA.gBigBuff.chpData);
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
    dispP[k] = LLDATA.ascii(chT);
    k++;
    chT = pBuff[i] & 0xF;
    dispP[k] = LLDATA.ascii(chT);
    k++;
  }
  dispP[k] = 0;
}


/*--------------------------------------------------------------------------------*/
//APPLICATION_PART_END
/*--------------------------------------------------------------------------------*/

