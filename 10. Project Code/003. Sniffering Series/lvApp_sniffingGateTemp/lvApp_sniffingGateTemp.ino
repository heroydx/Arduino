#include <FS.h>

//这个版本凡是在同一个AP的不论信号强度都登记， 或者登记信号强度大于-70dbm的

#include "LVCommon.h"
#include "./functions.h"
/*================================================================================*/
//MAIN_PART_BEGIN
/*================================================================================*/
//DON'T CHANGE THE MAIN PART CODE

LVCommon LVDATA;

#ifdef DEBUG_SIO
extern char dispBuff[];
#endif

void setup();
void loop();
void check_sys_data();
void vGenerate_IMEI();
void vApplication_setup_call();
void vApplication_wait_loop_call();
void vApplication_connected_loop_call();
void vApplication_read_data();
int nExec_application_Command(int nActionID, char achParam[], char achInst[]);
void application_POST_call(int nCMD);
bool bSend_3001();
bool bSend_9999();
bool bLoad_application_data();
bool bSave_application_data();

void special_main_loop();

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
  special_main_loop();
}

/*================================================================================*/
//MAIN_PART_END
/*================================================================================*/

/*================================================================================*/
//APPLICATION_PART_BEGIN you can change this part to meet your requirements
/*================================================================================*/


/*--------------------------------------------------------------------------------*/
//APPLICATION_DATA_PART_BEGIN
/*--------------------------------------------------------------------------------*/
// 不要修改下面的CONST_DEVICE_VERSION的值,一类设备对应于一个值,修改这个设备的值,会造成软件升级失败
#define CONST_DEVICE_VERSION "1_24_20170423"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =10),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   硬件_设备类型_内存选择参数_版本_日期
*/
#define CONST_DEVICE_SWVN "LVSW_GATETEMP_A_S10_V1.0.6_20170814"
#define CONST_DEVICE_HWVN "LVHW_GATETEMP_4M1M_V1.0.0_20161116"
#define CONST_DEVICE_MAID "1"
#define CONST_DEVICE_PID "24"
#define CONST_DEVICE_CAT "stSt"

//#define CONST_IMEI_PREFIX "8017021501"
#define CONST_IMEI_PREFIX_1 "80170"
#define CONST_IMEI_PREFIX_2 "810"

#define CONST_APPLICATION_FILE_NAME "/application.data"
#define CONST_APPLICATION_FILE_SIZE 1024

#define CONST_MAX_NOTHING_NEW 200

// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SAVE_CYCLE 1000*60*60
#define CONST_SEND_INTERVAL_POWERUP 1000*30
#define CONST_SEND_INTERVAL_REGULAR 1000*60*6
#define CONST_CHANNEL_SWITCH_DELAY 10
#define CONST_POWERUP_SHORT_SCAN_CYCLE 1000*60*5

unsigned long glApplicationSaveTick;
unsigned long gulApplicationTicks;
unsigned long gulApplicationPowerupNoScanTicks;
unsigned long gulSendInterval = CONST_SEND_INTERVAL_POWERUP;

unsigned long gul3001TimeOutTick;

//#define CONST_APP_SNIFF_GET_3001_DELAY 1000
//unsigned long gulGet3001DealyTick;

uint8_t gu8PowerupNoScanFlag = 0;

#define disable 0
#define enable  1
// WiFi 2.4G Hz 1-14 channel
uint8_t beginScanChannel = 1;
uint8_t endScanChannel = 14;
unsigned int channel = beginScanChannel;
#define CONST_HOMEAP_SPLIT 10 // scan home SSID frequency, =1, scan 1-14; =10, scan 1-14 one time + scan home ssid 9 time.

uint8_t gu8SniffStatusFlag = 0;

//需要检查整个字节发送长度不能超过2700个字节问题,因此CONST_MAX_MESSGE_COUNT=16
#define CONST_MAX_MESSGE_COUNT 16
#define CONST_MAX_BATCH_SEND_TIME_IN_SECONDS 180

short gnMsgBatchTotal, gnBatchCount, gnKnowCount;
bool gbFistBatchFlag = true;
bool gbInIdeleStatus = true;
//short gnPreHour = -1;

//温度部分
#include "wenshidu.h"
wenshidu gsWenShiDu;

/*--------------------------------------------------------------------------------*/
//APPLICATION_DATA_PART_END
/*--------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------*/
//APPLICATION_FUCTION_PART_BEGIN
/*--------------------------------------------------------------------------------*/

// application init data is used in setup

void check_sys_data()
{
  int i = 0;
  DBGPRINT("Reset Reason: ");
  DBGPRINTLN( ESP.getResetReason());
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
  uint8_t nT1;
  nT1 = random(99);
  //sprintf(LVDATA.IMEI, "%s%010d", CONST_IMEI_PREFIX, ESP.getChipId());
  sprintf(LVDATA.IMEI, "%s%010d%s%02d", CONST_IMEI_PREFIX_1, ESP.getChipId(), CONST_IMEI_PREFIX_2, nT1);
}

void vApplication_setup_call()
{
  strncpy(LVDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);

  bLoad_application_data();

  gsWenShiDu.begin();
};

void vApplication_wait_loop_call()
{
  strncpy((char *) homeSSID, LVDATA.gsDeviceInfo.wifi.ssid, CONST_SSID_LEN);
}


void vApplication_connected_loop_call()
{
  // save application data
  int u;
  if ((ulGet_interval(glApplicationSaveTick) > CONST_APPLICATION_SAVE_CYCLE))
  {
    bSave_application_data();
    glApplicationSaveTick = ulReset_interval();
  }

  //DBGPRINTLN("\nApplication_connected_loop_call");
  //DBGPRINTF("\ngulSendInterval:%d", gulSendInterval);

  //开机10分钟,每30秒和主机通信一次
  if (gu8PowerupNoScanFlag == 0) {
    DBGPRINTF("\ngbSmartConfigFlag %d", LVDATA.gbSmartConfigFlag);
    if ((ulGet_interval(gulApplicationPowerupNoScanTicks) > CONST_POWERUP_SHORT_SCAN_CYCLE) || (LVDATA.gbSmartConfigFlag == 0)) {
      gu8PowerupNoScanFlag = 1;
      //恢复正常通信频度
      //gulSendInterval = CONST_SEND_INTERVAL_REGULAR;
      gulSendInterval = LVDATA.glConnIntervalTick;
      DBGPRINTLN("\ngu8PowerupNoScanFlag=1");
    }
  }

  //进入扫描
  //other application read action
  //DBGPRINTLN("\n---Begin scan ---");
  //if (gbInIdeleStatus && (ulGet_interval(gulGet3001DealyTick) > CONST_APP_SNIFF_GET_3001_DELAY)) {
  if (gbInIdeleStatus ) {
    DBGPRINTF("\ngbInIdeleStatus [%d]", gbInIdeleStatus);
    vApplication_read_data();
  }

  if ((ulGet_interval(gulApplicationTicks) > gulSendInterval))
  {
    //read wendu
    gsWenShiDu.read();

    if (gbFistBatchFlag ) {
      LVDATA.gchMainState = WAIT_CONNECTION_STATE;
      LVDATA.nChange_WiFi_Mode(WL_DISCONNECTED);
      gbInIdeleStatus = false;

      if (clients_known_count > 0) {
        // do have data to send
        //DBGPRINTF("\n gbFistBatchFlag [%d]", gbFistBatchFlag);
        if (gbFistBatchFlag ) {
          DBGPRINTLN("--Prepare to send data--");
          //change wifi mode to connection status
          DBGPRINTLN("=====================================================================================");
          gbFistBatchFlag = false;
          for (u = 0; u < clients_known_count; u++) {
            DBGPRINTF("C No:%3d ", u);
            print_client(clients_known[u]);
          }
          for (u = 0; u < aps_known_count; u++) {
            DBGPRINTF("A No:%3d ", u);
            print_beacon(aps_known[u]);
          }
          DBGPRINTLN("=====================================================================================");
          // count the batch of the data
          //LVDATA.gnSendDatatStaus = 1;

          gnMsgBatchTotal = (clients_known_count + CONST_MAX_MESSGE_COUNT - 1) / CONST_MAX_MESSGE_COUNT;
          DBGPRINTF("  msgBatchTotal:[%d]", gnMsgBatchTotal);

          if (gnMsgBatchTotal > 0) {
            LVDATA.push_cmd(CONST_CMD_3001, 0, CONST_MAX_BATCH_SEND_TIME_IN_SECONDS);
            for (u = 0; u < gnMsgBatchTotal; u++) {
              //防止3001合并命令
              LVDATA.push_cmd(CONST_CMD_3001, 1, CONST_MAX_BATCH_SEND_TIME_IN_SECONDS);
            }
          }
          else {
            LVDATA.push_cmd(CONST_CMD_3001, 0, 60);
          }

          gnBatchCount = 0;
          gnKnowCount = clients_known_count;
          DBGPRINTF(" gnKnowCount[%d]", gnKnowCount);
        }
      }
    }
  }

  // 数据发送完毕
  if (gnKnowCount <= 0) {
    //no data to send
    //DBGPRINTF("\n gnKnowCount[%d]", gnKnowCount);
    gbFistBatchFlag = true;
    gbInIdeleStatus = true;
    gulApplicationTicks = ulReset_interval();
    if (gu8SniffStatusFlag) {
      //gulGet3001DealyTick = ulReset_interval();
      gu8SniffStatusFlag = 0;
    }
  }
}

void find_home_channel()
{
  short u;
  //find homeChannel;
  memcpy(homeMAC, LVDATA.gsDeviceInfo.bssid, CONST_ETH_MAC_LEN);
  for (u = 0; u < aps_known_count; u++) {
    if (! memcmp(aps_known[u].bssid, homeMAC, CONST_ETH_MAC_LEN)) {
      homeChannel = aps_known[u].channel;
      break;
    }
  }
  DBGPRINTF("\nHOME Channel:%d\n", homeChannel);
}

void vApplication_read_data()
{
  int u, loopCount;
  //LVDATA.debug_print_time();
  LVDATA.vLED_OFF();

  DBGPRINTF("\nSDK version: % s \n\r", system_get_sdk_version());
  //DBGPRINTF("AP Count: % d, Client Count: % d \n\r", aps_known_count, clients_known_count);
  //DBGPRINTLN(F("ESP8266 mini - sniff by Ray Burnette http: //www.hackster.io/rayburne/projects"));
  //DBGPRINTLN(F("Type:   /-------MAC------/-----WiFi Access Point SSID-----/  /----MAC---/  Chnl  RSSI"));
  if (gu8SniffStatusFlag == 0)
  {
    LVDATA.nChange_WiFi_Mode(WL_DISCONNECTED);
    //wifi_set_opmode(STATION_MODE);            // Promiscuous works only with station mode
    wifi_set_channel(channel);
    wifi_promiscuous_enable(disable);
    wifi_set_promiscuous_rx_cb(promisc_cb);   // Set up promiscuous callback
    wifi_promiscuous_enable(enable);
    gu8SniffStatusFlag = 1;
  }

  /*
    //find homeChannel;
    for (u = 0; u < aps_known_count; u++)
    {
      if (!memcmp(aps_known[u].ssid, homeSSID, sizeof(homeSSID)))
      {
        homeChannel = aps_known[u].channel;
      }
    }
    DBGPRINTF("\nHOMESSID:%d\n", homeChannel);
  */

  u = 1;
  loopCount = 0;
  channel = beginScanChannel;
  wifi_set_channel(channel);

  clean_data();

  while (true)
  {
    nothing_new++;                          // Array is not finite, check bounds and adjust if required
    if (nothing_new > CONST_MAX_NOTHING_NEW)
    {
      nothing_new = 0;
      if (u % CONST_HOMEAP_SPLIT == 0)
      {
        //scan the regular 1-14 channel
        channel = (int) u / CONST_HOMEAP_SPLIT;
#ifdef DEBUG_SIO
        //LVDATA.vLED_ON();
#endif
        //DBGPRINTF("u %d c %d ", u, channel);
      }
      else
      {
        //scan home channel
        channel = homeChannel;
        //LVDATA.vLED_OFF();
        //DBGPRINTF("home %d ", channel);
      }
      u++;

      if (channel >= endScanChannel)
      {
        u = beginScanChannel;
        loopCount++;
        //DBGPRINTF("\nc %d loopCount %d\n", channel, loopCount);
        //LVDATA.vLED_Blink();
      }
      //break;             // Only scan channels 1 to 14
      //DBGPRINTF("\nc %d loopCount %d\n", channel, loopCount);
      wifi_set_channel(channel);
    }
    if (ulGet_interval(gulApplicationTicks) > gulSendInterval)
    {
      //DBGPRINTF("\ntime out loop %d\n", loopCount);
      if (loopCount) {
        break;
      }
    }
    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
    // Press keyboard ENTER in console with NL active to repaint the screen
  }
  find_home_channel();
  DBGPRINTLN("======================");
  wifi_promiscuous_enable(disable);
  delay(CONST_CHANNEL_SWITCH_DELAY);
}


int nExec_application_Command(int nActionID, char  achParam[], char achInst[])
{
  int nCommand = CONST_CMD_9999;
  //DBGPRINTF("nExec_application_Command:%d",nActionID);
  switch (nActionID)
  {

    case 0:
      //digitalWrite(RELAY_ID, LOW);
      //gnStatusRelay = digitalRead(RELAY_ID);
      nCommand = CONST_CMD_9999;
      break;

    case 2:
      //digitalWrite(RELAY_ID, HIGH);
      //gnStatusRelay = digitalRead(RELAY_ID);
      nCommand = CONST_CMD_9999;
      break;

    case 30:
      //digitalWrite(RELAY_ID, HIGH);
      DBGPRINT("param=[");
      DBGPRINT(achParam);
      DBGPRINTLN("]");
      //gnStatusRelay = digitalRead(RELAY_ID);
      nCommand = CONST_CMD_9999;
      break;

    case 802:
      DBGPRINTLN("--gsWenShiDu.decode_802_command(achParam)--");
      gsWenShiDu.decode_802_command(achParam);

    default:
      break;

  }
  return nCommand;
}

void application_POST_call(int nCMD)
{
  ;
}


bool is_home_mac(uint8_t *dataPtr, uint8_t *homePtr)
{
  bool ret = false;
  short i;
  DBGPRINTLN("\nis_home_mac");
  for (i = 0; i < CONST_ETH_MAC_LEN; i++) {
    DBGPRINTF("%02X %02X ", dataPtr[i], homePtr[i]);
  }
  DBGPRINTLN("");
  if (!memcmp(dataPtr, homePtr, CONST_ETH_MAC_LEN )) {
    DBGPRINTLN("\n HOME MAC YES");
    ret = true;
  }
  return ret;
}

bool bSend_3001()
{
  int ret;
  short i, k, u;
  char *spStart, *spCurr;
  bool bSendFlag = false;

  DBGPRINTLN("-- send 3001 Begin --");
  DBGPRINTF("\nPosition: %d / %d, rest: %d/%d \n", gnBatchCount, gnMsgBatchTotal, gnKnowCount, clients_known_count);

  //while (gnKnowCount > 0)
  if (gnKnowCount > 0)
  {
    int nEachBatchMax;
    gnBatchCount++;
    //gnKnowCount -= gnBatchCount * CONST_MAX_MESSGE_COUNT;
    gnKnowCount -=  CONST_MAX_MESSGE_COUNT;
    if (gnKnowCount > 0) {
      nEachBatchMax = CONST_MAX_MESSGE_COUNT;
    }
    else {
      nEachBatchMax = gnKnowCount + CONST_MAX_MESSGE_COUNT;
      gnKnowCount = 0;
    }
    //each batch;
    spStart = LVDATA.gBigBuff.chpData;
    spCurr = spStart;
    //homeSSID
    //sprintf(spCurr, "{\"homeSSID\":\"%s\",", homeSSID);
    //sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"homeSSID\":\"%s\",", gnBatchCount, gnMsgBatchTotal, homeSSID);
    sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"INT\":\"%d\",\"homeSSID\":\"%s\",", gnBatchCount, gnMsgBatchTotal, gulSendInterval, homeSSID);
    spCurr += strlen(spCurr);

    //wendu
    sprintf(spCurr, "\"wendu\":\"%s\",\"shidu\":\"%s\",", gsWenShiDu.decWendu, gsWenShiDu.decShidu);
    spCurr += strlen(spCurr);
    
    //data
    sprintf(spCurr, "\"data\":[");
    spCurr += strlen(spCurr);

    //    for (i = 0; i < clients_known_count; i++)
    for (i = (gnBatchCount - 1) * CONST_MAX_MESSGE_COUNT; i < ((gnBatchCount - 1)*CONST_MAX_MESSGE_COUNT + nEachBatchMax); i++)
    {
      int nApsFlag = 0, u;
      sprintf(spCurr, "{\"mac\":\"");
      spCurr += strlen(spCurr);

      for (k = 0; k < 6; k++)
      {
        sprintf((spCurr ), "%02X:", clients_known[i].station[k]);
        spCurr += 3;
      }
      *(spCurr - 1) = '\"';
      *spCurr++ = ',';
      for (u = 0; u < aps_known_count; u++)
      {
        if (! memcmp(aps_known[u].bssid, clients_known[i].bssid, CONST_ETH_MAC_LEN))
        {
          if (!memcmp(aps_known[u].ssid, homeSSID, CONST_SSID_LEN))
          {
            //if (is_home_mac(clients_known[i].station, homeMAC))
            //if (is_home_mac(clients_known[i].bssid, homeMAC))
            if (!memcmp(clients_known[i].bssid, homeMAC, CONST_ETH_MAC_LEN))
              sprintf(spCurr, "\"home\":\"A\",");
            else
              sprintf(spCurr, "\"home\":\"Y\",");

            nApsFlag = 1;
            break;
          }
        }
      }
      if (nApsFlag == 0)
      {
        sprintf(spCurr, "\"home\":\"N\",", aps_known[u].ssid);
      }
      spCurr += strlen(spCurr);

      delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()

      sprintf(spCurr, "\"dbm\":\"%d\"", clients_known[i].rssi);
      spCurr += strlen(spCurr);
      //sprintf(spCurr, "\"name\":\"\"");
      //spCurr += strlen(spCurr);
      *(spCurr++) = '}';
      *(spCurr++) = ',';
    }
    *(spCurr - 1) = ']';
    *(spCurr++) = ',';
    sprintf(spCurr, "\"homeMAC\":\"");
    spCurr += strlen(spCurr);
    for (k = 0; k < 6; k++)
    {
      sprintf((spCurr ), "%02X:", homeMAC[k]);
      spCurr += 3;
    }
    *(spCurr - 1) = '\"';
    *(spCurr++) = '}';
    *(spCurr++) = 0;
    DBGPRINTF("\n%s\n", LVDATA.gBigBuff.chpData);
    //DBGPRINTF("spStart:\n%s\n", spStart);
    bSendFlag = true;
  }
  else {
    //如果只有一组数据,多发一组数据,保证上级命令可以收到
    if (gnMsgBatchTotal == 1) {
      //sprintf(LVDATA.gBigBuff.chpData, "{\"Batch\":\"0:0\",\"SN\":\"\",\"lastStamp\":\"00\",\"homeSSID\":\"\",\"data\":[],\"homeMAC\":\"\"}");
      sprintf(LVDATA.gBigBuff.chpData, "{\"Batch\":\"0:0\",\"SN\":\"\",\"lastStamp\":\"0\",\"INT\":\"%d\",\"homeSSID\":\"%s\",\"data\":[],\"homeMAC\":\"\"}",  gulSendInterval, homeSSID);
      bSendFlag = true;
    }
  }

  if (bSendFlag) {
    k = encodeWOSeed(LVDATA.gBigBuff.chpData, LVDATA.gCommandBuff.chpData);
    DBGPRINTLN("-- send 3001 --");
    //sprintf(LVDATA.gBigBuff.chpData, " {\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":%s,\"socketOut_P\":\"0\",\"socketOut_W\":\"0.0\",\"socketOutY_W\":\"0.0\",\"rlySub\":[1],\"rlystatus\":[0]}",
    //        CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, LVDATA.gCommandBuff.chpData);
    sprintf(LVDATA.gBigBuff.chpData, " {\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":\"%s\",\"socketOut_P\":\"0\",\"socketOut_W\":\"0.0\",\"socketOutY_W\":\"0.0\",\"rlySub\":[1],\"rlystatus\":[0]}",
            CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, LVDATA.gCommandBuff.chpData);
    //DBGPRINTLN(LVDATA.gBigBuff.chpData);
    DBGPRINTLN("======================================================================");

    //DBGPRINTLN("--CALL nCommonPOST--");
    ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData);
  }
  
  //判断是否数据是否发送完毕
  if (gnKnowCount <= 0) {
    clean_data();
    //LVDATA.gnSendDatatStaus = 0;
  }
  gul3001TimeOutTick = ulReset_interval();
  DBGPRINTLN("--send 3001 end--");
  return true;

}


bool bSend_9999()
{
  int ret;
  DBGPRINTLN("--send_9999--");
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"actionID\":\"%d\",\"delay\":\"0\",\"res\":\"1\"}",
          CONST_CMD_9999, LVDATA.IMEI, LVDATA.gnExecActionID);

  ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData);

  DBGPRINTLN("--end--");

  return true;

}



/* load application data, return True==Success*/
bool bLoad_application_data()
{
  return true;
}

/* save application data, return True = success */
bool bSave_application_data()
{
  return true;
}

void special_main_loop()
{
  //DBGPRINTLN("\nSpecial main loop");
  //如果长时间没有上报数据,清除发送状态标志
  if (ulGet_interval(gul3001TimeOutTick) > (gulSendInterval * 2))
  {
    //DBGPRINTLN("\n3001 clean gnSendDataStatus!");
    //LVDATA.gnSendDatatStaus = 0;
  }
  //如果长时间没有上报数据,系统重启
  if (ulGet_interval(gul3001TimeOutTick) > (gulSendInterval * 3))
  {
    DBGPRINTLN("\n3001 timeout restart!");
    ESP.restart();
  }
}

/*--------------------------------------------------------------------------------*/
//APPLICATION_FUCTION_PART_END
/*--------------------------------------------------------------------------------*/

/*================================================================================*/
//APPLICATION_PART_END
/*================================================================================*/

