
//#define DEBUG_SIO Serial

#include "LVCommon.h"
//#include "ListArray.h"
#include "ESPSoftwareSerial.h"
#include "LoRaMAC.h"
//#include "FM_ZCJ.h"
#include "FM_ZCJ_SM.h"


#define ENABLE_LORA

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
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
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

#ifdef ENABLE_LORA
LoRaMAC LoRaMacData;
#endif
FM_ZCJ gsCurrent;


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


void lora_setup()
{
  // set the data rate for the SoftwareSerial port
#ifdef ENABLE_LORA
  LoRaMacData.begin(true);//true=host mode
  LoRaMacData.debug_print_self();
#endif
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
  gsCurrent.begin();
  lora_setup();
}

void ICACHE_FLASH_ATTR vApplication_wait_loop_call()
{
  //gsCurrent.restore_setting();
}


#define TEST_MODE_LEN 11
stDataStreamUnion toTest[TEST_MODE_LEN];
stDataStreamUnion toRecv;

unsigned long ulDevID = 12345678L;
uint8_t u8SeedKey[LORA_MAC_KEY_LEN];
uint8_t u8SeedVal[LORA_MAC_KEY_MD5];


void prepare_test_data()
{
  uint8_t i;
  //ulDevID = generate_devID();
  ulDevID = ESP.getChipId();
  random_string((char *) u8SeedKey, LORA_MAC_KEY_LEN);
  simpleHash((char *) u8SeedKey, (char *)  u8SeedVal, ulDevID, LORA_MAC_KEY_LEN);
  for (i = 0; i < TEST_MODE_LEN; i++) {
    //DBGPRINTLN(i);
    switch (i)
    {
      case LORA_CMD_EXTEND_CMD:
        toTest[i].exCMD.CMD = LORA_CMD_EXTEND_CMD;
        memcpy(toTest[i].exCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].exCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].exCMD.seq = (uint8_t) i;
        toTest[i].exCMD.exCMD = 1;
        break;
      case LORA_CMD_REGISTRATION_REQUEST:
        toTest[i].regRequest.CMD = LORA_CMD_REGISTRATION_REQUEST;
        memcpy(toTest[i].regRequest.sourceAddr, "\x00\x00\x00\x00", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regRequest.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].regRequest.seq = (uint8_t) i;
        toTest[i].regRequest.devID = ulDevID;
        toTest[i].regRequest.MAID = 8;
        toTest[i].regRequest.PID = 1;
        memcpy(toTest[i].regRequest.key, u8SeedKey, LORA_MAC_KEY_LEN);

        break;
      case LORA_CMD_REGISTRATION_FEEDBACK:
        toTest[i].regFeedback.CMD = LORA_CMD_REGISTRATION_FEEDBACK;
        memcpy(toTest[i].regFeedback.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regFeedback.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].regFeedback.seq = (uint8_t) i;
        toTest[i].regFeedback.devID = ulDevID;
        memcpy(toTest[i].regFeedback.addr, "\xC0\xA8\x00\x02", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regFeedback.val, u8SeedVal, LORA_MAC_KEY_MD5);
        break;
      case LORA_CMD_ADDR_REQUEST:
        break;
      case LORA_CMD_ADDR_FEEDBACK:
        break;
      case LORA_CMD_SYNC_CMD:
        toTest[i].syncCMD.CMD = LORA_CMD_SYNC_CMD;
        memcpy(toTest[i].syncCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].syncCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].syncCMD.seq = (uint8_t) i;
        memcpy(toTest[i].syncCMD.groupAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN - 1);
        toTest[i].syncCMD.groupLen = 8;
        for (short k = 1; k < toTest[i].syncCMD.groupLen + 1; k++) {
          toTest[i].syncCMD.list[k] = k;
        }
        break;
      case LORA_CMD_SYNC_TIME:
        break;
      case LORA_CMD_SYNC_RANDOM:
        toTest[i].syncCMD.CMD = LORA_CMD_SYNC_RANDOM;
        memcpy(toTest[i].syncCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].syncCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].syncCMD.seq = (uint8_t) i;
        toTest[i].syncCMD.groupLen = 16;
        memcpy(toTest[i].syncCMD.groupAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN - 1);
        break;
      case LORA_CMD_VERIFY_REQUEST:
        break;
      case LORA_CMD_VERIFY_FEEDBACK:
        break;
      default:
        break;
    }
  }
}




void test_handle_recv_data()
{
  short CMD;
  short nLen;
  nLen = sizeof (stDataStreamUnion);

  CMD = toRecv.exCMD.CMD;
  //public Part
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      break;
      DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

      DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
      toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.devID = toRecv.regRequest.devID;
      simpleHash((char *) toRecv.regRequest.key, (char *)  toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.val, toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.devID, LORA_MAC_KEY_LEN);

      //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
      //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);
#ifdef ENABLE_LORA
      LoRaMacData.debug_print_union(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
#endif
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
      gsCurrent.handle_Server_to_FM_command((char *)toRecv.app.data);
      break;
    default:
      DBGPRINTLN("\n== application call ==");
      //application call to call external application API
      DBGPRINTF("\napp CMD: [%d]\n", CMD);
      break;
  }
}

void ICACHE_FLASH_ATTR vApplication_connected_loop_call()
{
  short nLen=0;
  //nLen = LoRaDev.available();
#ifdef ENABLE_LORA
  nLen = LoRaMacData.available();
  if (nLen) {
    memcpy(toRecv.u8Data, LoRaMacData.recvData.u8Data, sizeof(stDataStreamUnion));
    LoRaMacData.debug_print_union(toRecv.u8Data);
    Serial.println("\n--LoRa received Data");
    Serial.println(nLen);
    test_handle_recv_data();
    LoRaMacData.debug_print_self();

  }
#endif
  gsCurrent.FM_loop();
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
      gsCurrent.handle_Server_to_FM_command(achParam);
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
    for (i = 0; i < CONST_APP_FM_RECV_FREQ_MAX_SAVE_POS; i++) {
      sprintf(strP, "%d:", gsCurrent.freq_next());
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
  gsCurrent.freq_clear_send();
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


/*--------------------------------------------------------------------------------*/
//APPLICATION_PART_END
/*--------------------------------------------------------------------------------*/

