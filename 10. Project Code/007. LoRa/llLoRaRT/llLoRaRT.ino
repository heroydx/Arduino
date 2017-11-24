/*
   LoRa Router by steven.lian@gmail.com 2017/11/09
   have the following function
   1. control up to 248 device.
   2. automatic handle device registration and address assigement (DHCP)
   3. two priority list for registration and data transfer
*/

#include "LLCommon.h"
#include "ListArray.h"
#include "ESPSoftwareSerial.h"
#include "LoRaMAC.h"

extern "C" {
  //#include "AppleBase64.h"
#include "simpleEncode.h"
}

void ICACHE_FLASH_ATTR vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen);

/*================================================================================*/
//MAIN_PART_BEGIN
/*================================================================================*/
//DON'T CHANGE THE MAIN PART CODE

//communication with server part
LLCommon LLDATA;

#ifdef DEBUG_SIO
extern char dispBuff[];
#endif

//arduino main setup
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
  randomSeed(micros());
  vGenerate_IMEI();
  vApplication_setup_call();
  LLDATA.mainsetup();
  check_sys_data();
  //power on LED blink,major for flash code success indicator
  LEDControl.LED_poweron();
}

void loop()
{
  LLDATA.mainloop();
  // second count loop, from miscCommon
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
   SW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   HW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/

#define CONST_DEVICE_SWVN "LLSW_LORA_RT_V0.0.1_20171109"
#define CONST_DEVICE_HWVN "LLHW_LORA_4M1M_V1.0.0_20170310"
#define CONST_DEVICE_ADDTION_HWVN CONST_LORA_HARDWARE_DEVICE
#define CONST_DEVICE_MAID "8"
#define CONST_DEVICE_PID "1"

//#define CONST_DEVICE_CAT "fmRa"
#define CONST_DEVICE_CAT "loRa"

#define CONST_DEVICE_CODING_TYPE  _DEF_COMM_CODING_TYPE_JSON

//#define CONST_IMEI_PREFIX "6019980908"
//#define CONST_IMEI_PREFIX "6116111401"
#define CONST_IMEI_PREFIX_1 "61171"
//#define CONST_IMEI_PREFIX_2 "50802"
#define CONST_IMEI_PREFIX_2 "109"

//LoRa information
short loRaWorkChannel = 425;
LoRaMAC LoRaMacData;

//发送接收处理
stDataStreamUnion appSendData;
stLoRaDataServer appSendBuf;

#define CONST_MAX_MESSGE_COUNT 30
#define CONST_MAX_BATCH_SEND_TIME_IN_SECONDS 180

short gnMsgBatchTotal;
short gnBatchCount;
short gnSendBufLen;
short gnStatBufLen;

bool gbFistBatchFlag = true;
bool gbInIdeleStatus = true;


#define CONST_SERVER_SEND_BUFF_MAX_LENGTH 32
#define CONST_SERVER_RECV_BUFF_MAX_LENGTH 32
#define CONST_SERVER_SEND_INTERVAL_TIME 60000
#define CONST_SERVER_SEND_CLASSONE_THRESHOLD_NUM 2
#define CONST_SERVER_SEND_CLASSTWO_THRESHOLD_NUM 8

//服务器方向缓冲区队列,主要分成两个方向
// ClassOne高优先级,主要是存储LoRa注册,告警类别信息,发送到服务器
static ListArray sendSeverBufClassOneList(CONST_SERVER_SEND_BUFF_MAX_LENGTH, sizeof(stLoRaDataServer));
// ClassTwo普通优先级，主要是状态报告
static ListArray sendSeverBufClassTwoList(CONST_SERVER_SEND_BUFF_MAX_LENGTH, sizeof(stLoRaDataServer));
// 接收缓冲区,从服务器接收数据,向LoRa网络发送
static ListArray recvSeverBuffList(CONST_SERVER_RECV_BUFF_MAX_LENGTH, sizeof(stLoRaDataServer));

//LED 控制组件
miscLED LEDControl;

unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 10*1000

/*--------------------------------------------------------------------------------*/
//APPLICATION_DATA_PART_END
/*--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------*/
//APPLICATION_FUCTION_PART_BEGIN
/*--------------------------------------------------------------------------------*/

/* 开机打印系统的一些基本信息,方便调试,可以不需要*/
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

/* 生成本机的IMEI */
void vGenerate_IMEI()
{
  vGenerate_IMEI(LLDATA.IMEI, ESP.getChipId());
}

void vGenerate_IMEI(char *out, unsigned long id)
{
  //利用随机数,防止由于id重复造成的IMEI重复
  uint8_t nT1;
  nT1 = random(99);
  sprintf(out, "%s%010d%s%02d", CONST_IMEI_PREFIX_1, id, CONST_IMEI_PREFIX_2, nT1);
}

/* 初始化LoRa部分,lora参数通过smart config得到 */
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

/*void LLCommon::func_WAIT_FOR_THIRD_PARTY_DEVICE() 会调用*/
void vThird_party_setup_call()
{
  lora_setup();
}

// application init data is used in setup
void  vApplication_setup_call()
{
  //初始化硬件版本,软件版本等信息
  short nLen;
  strncpy(LLDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  nLen = strlen(LLDATA.gsDeviceInfo.HWVN);
  strncpy(LLDATA.gsDeviceInfo.HWVN + nLen, CONST_DEVICE_ADDTION_HWVN, CONST_SYS_VERSION_LENGTH - nLen - 1);
  strncpy(LLDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LLDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);
  //lora_setup();
  //strncpy(LLDATA.codingType, CONST_DEVICE_CODING_TYPE, _DEF_COMM_CODING_TYPE_LENGTH);

  //确定和服务器通信所用的加密方式
  strncpy(LLDATA.codingType, _DEF_COMM_CODING_TYPE_B6401, _DEF_COMM_CODING_TYPE_LENGTH);
}

/* 会被 LLCommon里面的代码调用*/
void ICACHE_FLASH_ATTR vApplication_wait_loop_call()
{
}


// 处理LoRa部分的数据开始

/* 处理注册消息,把 loRaMacData里面处理后的注册消息,转发到服务器,告知服务器目前地址分配情况*/
void func_LORA_CMD_REGISTRATION_REQUEST()
{
  DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

  DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
  simpleHash((char *) LoRaMacData.key, (char *)  LoRaMacData.sendData.regFeedback.val, LoRaMacData.devID, LORA_MAC_KEY_LEN);

  //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
  //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);

  //准备给服务器的数据
  appSendBuf.reg.type = CONST_LORA_REPORT_TYPE_REGISTRATION;
  appSendBuf.reg.devID = LoRaMacData.get_devID(LoRaMacData.sendData.regFeedback.addr);
  memcpy(appSendBuf.reg.addr, LoRaMacData.sendData.regFeedback.addr, LORA_MAC_ADDRESS_LEN);
  appSendBuf.reg.MAID = LoRaMacData.recvData.regRequest.MAID;
  appSendBuf.reg.PID = LoRaMacData.recvData.regRequest.PID;

  //send to server buff list
  sendSeverBufClassOneList.push(&appSendBuf);
  DBGPRINTF("\n -- sendSeverBufClassOneList [ % d]", sendSeverBufClassOneList.len());
  DBGPRINTF("\n devID[ % d], MAID[ % d], PID[ % d], ", appSendBuf.reg.devID, appSendBuf.reg.MAID, appSendBuf.reg.PID);

  LoRaMacData.debug_print_union(LoRaMacData.sendData.u8Data);
}

/*
   处理应用消息,包括各种应用的状态上报等数据.
   考虑到简化路由器的功能,路由器不再考虑数据的类型,只负责转发和上报,具体的数据内容由服务器和客户端协商
*/
void func_LORA_CMD_APP_Data()
{
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  short pos;
  DBGPRINTLN("\n == func_LORA_CMD_APP_Data == ");
  memcpy(sourceAddr, LoRaMacData.recvData.app.sourceAddr, LORA_MAC_ADDRESS_LEN);

  //准备给服务器的数据
  appSendBuf.stat.type = CONST_LORA_REPORT_TYPE_STATUS;
  appSendBuf.stat.devID = LoRaMacData.get_devID(LoRaMacData.recvData.app.sourceAddr);
  appSendBuf.stat.CMD = LoRaMacData.recvData.app.CMD;
  memcpy(appSendBuf.stat.data, LoRaMacData.recvData.app.data, sizeof(LORA_MAC_APPLICATION_MAX_DATA_LEN));

  //send to server buff list
  sendSeverBufClassTwoList.push(&appSendBuf);
  DBGPRINTF("\n -- sendSeverBufClassOneList [ % d]", sendSeverBufClassOneList.len());
  DBGPRINTF("\n devID[ % d], MAID[ % d], PID[ % d], ", appSendBuf.reg.devID, appSendBuf.reg.MAID, appSendBuf.reg.PID);
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
    case LORA_CMD_NULL_CMD:
      //空消息,可能用来判断设备是否长时间没有和服务器通信,不过建议这个判断由服务器完成.
      DBGPRINTLN("\n == LORA_CMD_NULL_CMD == ");
      break;
    case LORA_CMD_EXTEND_CMD:
      DBGPRINTLN("\n == LORA_CMD_EXTEND_CMD == ");
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
    case LORA_CMD_APP_MOTOR:
    case LORA_CMD_APP_LIGHT:
    default:
      DBGPRINTF("\n == application call cmd[%d] == \n", CMD);
      func_LORA_CMD_APP_Data();
      //application call to call external application API
      break;
  }
}


//处理服务器到LoRa的命令
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
  /*
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
  */
}


/*
   会被 LLCommon里面的代码调用,这个是应用的主循环部分
*/
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

  //判断是否需要和服务器通信,触发条件是通信间隔到,或者是两个队列的待发送数据达到一定数量
  if ((ulGet_interval(gulApplicationTicks) > LLDATA.glConnIntervalTick)
      || (sendSeverBufClassOneList.len() >= CONST_SERVER_SEND_CLASSONE_THRESHOLD_NUM)
      || (sendSeverBufClassTwoList.len() >= CONST_SERVER_SEND_CLASSTWO_THRESHOLD_NUM))
    //if ((ulGet_interval(gulApplicationTicks) > LLDATA.glConnIntervalTick))
  {
    DBGPRINTF("\n glConnIntervalTick: [%d] ", LLDATA.glConnIntervalTick);
    //发送批次清零
    gnMsgBatchTotal = 0;

    //根据待发送数据个数,计算需要发送的批次数
    gnSendBufLen = sendSeverBufClassOneList.len() + sendSeverBufClassTwoList.len();
    if (gnSendBufLen > 0) {
      gnMsgBatchTotal = (gnSendBufLen + CONST_MAX_MESSGE_COUNT - 1) / CONST_MAX_MESSGE_COUNT;
    }
    DBGPRINTF("\n gnSendBufLen: [%d], gnMsgBatchTotal: [%d] ", gnSendBufLen, gnMsgBatchTotal);

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
    //发送批次计数
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
     测试数据可以删除
  */
  appSendBuf.reg.type = CONST_LORA_REPORT_TYPE_REGISTRATION;
  appSendBuf.reg.devID = 12345678;
  appSendBuf.reg.addr[0] = 0xC0;
  appSendBuf.reg.addr[1] = 0xA8;
  appSendBuf.reg.addr[2] = 0x0;
  appSendBuf.reg.addr[3] = 0x10;

  appSendBuf.reg.MAID = atoi(LLDATA.gsDeviceInfo.MAID);
  appSendBuf.reg.PID = LLDATA.gsDeviceInfo.SUBPID;
  sendSeverBufClassOneList.push(appSendBuf.u8Data);
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

  }

  return nCommand;
}

void application_POST_call(int nCMD)
{
  ;
}


/*
   会被 LLCommon里面的代码调用,这个是向服务器发送数据的主要接口
*/
bool ICACHE_FLASH_ATTR bSend_status()
{
  int ret;
  short int i;
  short nEachBatchMax;

  LLDATA.SN++;
  DBGPRINTF("\n-- send 2001 --  SN:[%d] gnMsgBatchTotal:[%d] gnSendBufLen:[%d]", LLDATA.SN, gnMsgBatchTotal, gnSendBufLen);

  if ((gnMsgBatchTotal > 0) && ( gnSendBufLen > 0))
  {
    //do have data to send
    //计算单次发送的个数,最多不超过CONST_APP_LORA_MAX_REPORT_PKG_LEN
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
      if (sendSeverBufClassOneList.pop(appSendBuf.u8Data) < 0)
      {
        //ClassOneBuf 空
        if (sendSeverBufClassTwoList.pop(appSendBuf.u8Data) < 0)
        {
          //没有数据了
          DBGPRINTF("\n for nEachBatchMax,noData [%s]", LLDATA.gBigBuff.chpData);
          strcat(ptr, "\"");
          break;
        }
      }

#ifdef DEBUG_SIO
      //vHexString_2ascii(appSendData.u8Data, dispBuff, sizeof(stDataStreamUnion));
      vHexString_2ascii(appSendBuf.u8Data, dispBuff, sizeof(appSendBuf));
      DBGPRINTF("\nin 2001 u8Data:devID [%d], [%s]", appSendBuf.reg.devID, dispBuff);
#endif

      nLen = LoRaMacData.cal_server_data_len(appSendBuf.reg.type);
      Base64encode(ptr, (char *)appSendBuf.u8Data, nLen);

      DBGPRINTF("\nin 2001 base64:[%s]", ptr);
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
    //no data to send,just send a regular msg
    //sprintf(LLDATA.gCommandBuff.chpData."%s","[]");
    strcpy(LLDATA.gCommandBuff.chpData, "[]");
    DBGPRINTF("no data to send,just send a regular msg", LLDATA.gCommandBuff.chpData);
  }

  //old 2001
  /*
    sprintf(LLDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"FM\":\"%d:%d:%d:%d:%d:%d:%d\",\"LEN\":\"%d\",\"SAVE\":%s,\"sioErr\":\"%d\",\"LoRaChan\":\"%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0\",\"socketOutY_W\":\"0\",\"rlySub\":[0],\"rlystatus\":[0]}",
          CONST_CMD_2001, LLDATA.IMEI, LLDATA.gsDeviceInfo.CAT, FMDevice.currStat.u8Stat, FMDevice.currStat.nFrequency, FMDevice.currStat.u8Vol, FMDevice.currStat.u8Campus, FMDevice.currStat.u8Blk, \
          FMDevice.currStat.u8DSP, FMDevice.currStat.u8SN_THR, nBuffLen, LLDATA.gCommandBuff.chpData, FMDevice.currStat.errCount, loRaWorkChannel);
  */
  sprintf (LLDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"reqID\":\"%s\",\"rtnID\":\"%s\",\"SN\":\"%d\",\"rptType\":\"%s\",\"LoRaChan\":%d,\"rptData\":%s}",
           CONST_CMD_2001, LLDATA.IMEI, LLDATA.IMEI, LLDATA.SN, LLDATA.gsDeviceInfo.CAT, LoRaMacData.workChannel, LLDATA.gCommandBuff.chpData);
  DBGPRINTF("\nLLDATA.gBigBuff.chpData:[%s]", LLDATA.gBigBuff.chpData);
  // encode and decode
  LLDATA.encode_data(LLDATA.codingType, strlen(LLDATA.gBigBuff.chpData), LLDATA.gBigBuff.chpData, LLDATA.gCommandBuff.chpData);

  //sprintf(LLDATA.gBigBuff.chpData, "{\"codingType\":\"%s\",\"data\":%s}", LLDATA.codingType, LLDATA.gCommandBuff.chpData);

  //DBGPRINTLN(LLDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LLDATA.nCommonPOST( LLDATA.gBigBuff.chpData);
  //  ghTcpCommand.print();
  //  ghTcpCommand.print(thisData);
  //FMDevice.freq_clear_send();
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

