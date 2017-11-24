#include <arduino.h>

//#include "miscCommon.h"
#include "LoRaMAC.h"

#define DEBUG_SIO Serial

/***********************************************************************
   DEBUG MODIFY FILES
   1. miscCommon.h
     //#define DEBUG_SIO Serial--> remove quote
   2. FM_ZCJ_SM.h
     //#define APP_SERIAL Serial-->  #define APP_SERIAL SerialFM
   3. FM_ZCJ_SM.cpp
     static SoftwareSerial SerialFM(CONST_FM_ZCJ_RX_PORT, CONST_FM_ZCJ_TX_PORT); // RX, TX 8266 GPIO 4,2
 **********************************************************************/

#define SIO_BAUD 74880

#define CONST_LORA_MONITOR_CHANNEL 425

#define NEED_REGULAR_CONNECTION_FUNTION 1
#ifdef NEED_REGULAR_CONNECTION_FUNTION
#define CONST_CONNINTERVAL_TICK 125*1000
#else
#define CONST_CONNINTERVAL_TICK 20000*1000
#endif
#define CONST_CONNINTERVAL_SHORT_TICK 5000

#define CONST_MIN_POST_INTERVAL_TICK 1000

short nLoRaChannel = CONST_LORA_MONITOR_CHANNEL;

//LoRaMAC LoRaMacData;
LoRa_AS62 gLoRaDev;

stDataStreamUnion appSendData;
stDataStreamUnion appRecvData;

unsigned long glsecondTick = 0;
unsigned long glStateTransferTicks = ulReset_interval();
int gnStateTransferTicksCount = 0;

short gnTimeSlotTotalNum;
short gnTimeSlotCount;
unsigned long gulTimeSlotTicks;

unsigned long gulPreTicks;
unsigned long gulCurrTicks;

void lora_setup()
{
  // set the data rate for the SoftwareSerial port

  //LoRaMacData.begin(false, nLoRaChannel); //true=host mode
  //LoRaMacData.debug_print_self();
  //loRaDevAddress = 0x1234;
  //loRaDevChannel = CONST_LORA_DEFAULT_CHANNEL; //send channel
  //loRaDevBandRate = CONST_LORA_AIR_BAUD_RATE;

  gLoRaDev.begin(0x1234, nLoRaChannel, CONST_LORA_AIR_BAUD_RATE);
  if (gLoRaDev.isReady()) {
    DBGPRINTLN("AS62 is ready for communication");
  }
}


void setup()
{
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  // Open serial communications and wait for port to open:
#ifdef DEBUG_SIO
  SERIAL_DEBUG_BEGIN
  while (!DEBUG_SIO) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  DBGPRINTLN();
  DBGPRINTLN();
  DBGPRINTLN("Serial Port for debug is ready on : ");
  DBGPRINTLN(SIO_BAUD);
#endif
  lora_setup();
  DBGPRINTF("\nLoRa Channel [%d]\n", nLoRaChannel);

}

long diff_ticks()
{
  long ret;
  gulPreTicks = gulCurrTicks;
  gulCurrTicks = millis();
  ret = gulCurrTicks - gulPreTicks;
  if (ret < 0) {
    ret += 0xffffffff;
  }
  return ret;
}

void monitor_data_print(uint8_t *ptr, short nLen)
{
  short CMD;
  char dispBuff[100];
  long ticks;
  stDataStreamUnion forPrint;
  memcpy(forPrint.u8Data, ptr, sizeof(stDataStreamUnion));
  ticks = diff_ticks();
  //DBGPRINTF("\n%d %d %d", gulCurrTicks, gulPreTicks, ticks);
  //DBGPRINTF("\n{\"tick\":\"%d\",\"len\":\"%d\",", gulCurrTicks, nLen);
  DBGPRINTF("\n{\"tick\":\"%d\",\"len\":\"%d\",", ticks, nLen);
  CMD = forPrint.exCMD.CMD;
  //source addr
  convert_to_ip_format(dispBuff, forPrint.exCMD.sourceAddr, 4);
  DBGPRINTF("\"from\":\"%s\",", dispBuff);
  //dest addr
  convert_to_ip_format(dispBuff, forPrint.exCMD.destAddr, 4);
  DBGPRINTF("\"to\":\"%s\",", dispBuff);
  //chksum and seq
  DBGPRINTF("\"chksum\":\"%d\",\"seq\":\"%d\",", forPrint.exCMD.chksum, forPrint.exCMD.seq);

  //public Part
  switch (CMD)
  {
    case LORA_CMD_CONFIG_CHANNEL_BROARDCAST:
      DBGPRINT("\"type\":\"LORA_CMD_CONFIG_CHANNEL_BROARDCAST\",");
      DBGPRINTF("\"channel01\":\"%d\", \"channel02\":\"%d\", \"channel11\":\"%d\", \"channel12\":\"%d\",", forPrint.configChannel.channel01, forPrint.configChannel.channel02, forPrint.configChannel.channel11, forPrint.configChannel.channel12);
      break;
    case LORA_CMD_HOST_INFO_BROARDCAST:
      DBGPRINT("\"type\":\"LORA_CMD_HOST_INFO_BROARDCAST\",");
      DBGPRINTF("\"devID\":\"%d\", \"maxCount\":\"%d\", \"addrCount\":\"%d\", \"addrBegin\":\"%d\",", forPrint.hostInfo.devID, forPrint.hostInfo.maxCount, forPrint.hostInfo.addrCount, forPrint.hostInfo.addrBegin);
      break;
    case LORA_CMD_REGISTRATION_CLEANUP:
      DBGPRINT("\"type\":\"LORA_CMD_REGISTRATION_CLEANUP\",");
      break;
    case LORA_CMD_EXTEND_CMD:
      DBGPRINT("\"type\":\"LORA_CMD_EXTEND_CMD\",");
      DBGPRINTF("exCMD:\"%d\"", forPrint.exCMD.exCMD);
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      DBGPRINT("\"type\":\"LORA_CMD_REGISTRATION_REQUEST\",");
      DBGPRINTF("\"devID\":\"%d\", \"MAID\":\"%d\", \"PID\":\"%d\",", forPrint.regRequest.devID, forPrint.regRequest.MAID, forPrint.regRequest.PID);
      convert_to_hex_format(dispBuff, forPrint.regRequest.key, LORA_MAC_KEY_LEN);
      DBGPRINTF("\"key\":\"%s\",", dispBuff);
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINT("\"type\":\"LORA_CMD_REGISTRATION_FEEDBACK\",");
      DBGPRINTF("\"devID\":\"%d\",",  forPrint.regFeedback.devID);
      convert_to_ip_format(dispBuff, forPrint.regFeedback.addr, 4);
      DBGPRINTF("\"addr\":\"%s\",", dispBuff);
      convert_to_hex_format(dispBuff, forPrint.regFeedback.val, LORA_MAC_KEY_MD5);
      DBGPRINTF("\"val\":\"%s\",", dispBuff);
      break;
    case LORA_CMD_ADDR_REQUEST:
      DBGPRINT("\"type\":\"LORA_CMD_ADDR_REQUEST\",");
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      DBGPRINT("\"type\":\"LORA_CMD_ADDR_FEEDBACK\",");
      break;
    case LORA_CMD_SYNC_CMD:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_CMD\",");
      convert_to_ip_format(dispBuff, forPrint.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1);
      DBGPRINTF("\"groupAddr\":\"%s\",", dispBuff);
      DBGPRINTF("\"groupLen\":\"%d\",", forPrint.syncCMD.groupLen);
      convert_to_dec_format(dispBuff, forPrint.syncCMD.list, forPrint.syncCMD.groupLen);
      DBGPRINTF("\"groupList\":\"%s\",", dispBuff);

      gnTimeSlotTotalNum = forPrint.syncCMD.groupLen;
      gnTimeSlotCount = 0;
      gulTimeSlotTicks = ulReset_interval();

      break;
    case LORA_CMD_SYNC_LOOP_BEGIN:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_LOOP_BEGIN\",");
      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_TIME\",");
      break;
    case LORA_CMD_SYNC_RANDOM:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_RANDOM\",");
      convert_to_ip_format(dispBuff, forPrint.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1);
      DBGPRINTF("\"groupAddr\":\"%s\",", dispBuff);
      DBGPRINTF("\"groupLen\":\"%d\",", forPrint.syncCMD.groupLen);

      gnTimeSlotTotalNum = forPrint.syncCMD.groupLen;
      gnTimeSlotCount = 0;
      gulTimeSlotTicks = ulReset_interval();

      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINT("\"type\":\"LORA_CMD_VERIFY_REQUEST\",");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINT("\"type\":\"LORA_CMD_VERIFY_FEEDBACK\",");
      break;
    default:
      DBGPRINT("\"type\":\"APPLICATION\",");
      //application call to call external application API
      DBGPRINTF("\"appCMD\":\"%d\"", CMD);
      break;
  }

  DBGPRINT("\"end\":\"\"}");

}

void print_time_slot()
{
  long ticks;
  if (ulGet_interval(gulTimeSlotTicks) > LORA_MAC_TIME_SLOT_IN_MS) {
    if (gnTimeSlotTotalNum > 0) {
      ticks = diff_ticks();
      //DBGPRINTF("\n%d %d %d", gulCurrTicks, gulPreTicks, ticks);
      DBGPRINTF("\n{\"tick\":\"%d\",\"TimeSlot\":\"%d\"}", ticks, gnTimeSlotCount);
      gnTimeSlotCount++;
      gnTimeSlotTotalNum--;
      gulTimeSlotTicks = ulReset_interval();
    }
  }
}

void loop() // run over and over
{
  short nLen = 0;
  //nLen = LoRaMacData.available(1);
  nLen = gLoRaDev.available();
  if (nLen) {
    gLoRaDev.get(appRecvData.u8Data); // 必须使用recvData.u8Data,后面会默认按这个数据地址处理
  }

  if (nLen) {
    monitor_data_print(appRecvData.u8Data, nLen);
  }
  delay(2);//必须保留
  print_time_slot();
  if (ulGet_interval(glsecondTick) > 1000) {
    //DBGPRINTF("\nnLen:[%d]", nLen);
    glsecondTick = ulReset_interval();
  }
}


