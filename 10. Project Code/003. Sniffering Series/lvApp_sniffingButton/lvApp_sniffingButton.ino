#include <FS.h>

//#define DEBUG_SIO 1

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
#define CONST_DEVICE_VERSION "1_25_20170515"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/
#define CONST_DEVICE_SWVN "LVSW_GATE_B_S01_V1.0.5_20170629"
#define CONST_DEVICE_HWVN "LVHW_GATE_BUTTON_4M1M_V1.0.0_20170526"
#define CONST_DEVICE_MAID "1"
#define CONST_DEVICE_PID "25"
#define CONST_DEVICE_CAT "stSn"
//#define CONST_IMEI_PREFIX "8017050301"
#define CONST_IMEI_PREFIX_1 "80170"
//#define CONST_IMEI_PREFIX_2 "50301"
#define CONST_IMEI_PREFIX_2 "526"

#define CONST_APPLICATION_FILE_NAME "/application.data"
#define CONST_APPLICATION_FILE_SIZE 1024

#define CONST_MAX_NOTHING_NEW 200

// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SAVE_CYCLE 1000*60*60
#define CONST_SEND_INTERVAL_POWERUP 30*1000
#define CONST_SEND_INTERVAL_REGULAR 1000*60*2
#define CONST_CHANNEL_SWITCH_DELAY 5
#define CONST_POWERUP_SHORT_SCAN_CYCLE 1000*60*5 //开机短扫描最长时间

unsigned long glApplicationSaveTick;
unsigned long gulApplicationTicks;
unsigned long gulApplicationPowerupNoScanTicks;
unsigned long gulSendInterval = CONST_SEND_INTERVAL_POWERUP; 

unsigned short gnSeqNo;
unsigned long gulOneSecondTick;
unsigned short gnSecondSeq;
//char achTimeStampString[20];

unsigned long gul3001TimeOutTick;

#define CONST_APP_SNIFF_GET_3001_DELAY 1000
unsigned long gulGet3001DealyTick;

uint8_t gu8PowerupNoScanFlag = 0;

#define disable 0
#define enable  1
// WiFi 2.4G Hz 1-14 channel
uint8_t beginScanChannel = 1;
uint8_t endScanChannel = 14;
unsigned int channel = beginScanChannel;
#define CONST_HOMEAP_SPLIT 1 // scan home SSID frequency, =1, scan 1-14; =10, scan 1-14 one time + scan home ssid 9 time.

uint8_t gu8SniffStatusFlag = 0;

#define CONST_MAX_MESSGE_COUNT 24
//#define CONST_MAX_MESSGE_COUNT 1
#define CONST_MAX_BATCH_SEND_TIME_IN_SECONDS 180

short gnMsgBatchTotal, gnBatchCount, gnKnowCount;
bool gbFistBatchFlag = true;
bool gbInIdeleStatus = true;
//short gnSpecialCount;
//short gnPreHour = -1;

//power
#define CONST_APP_VCC_REFERNCE 460/1023
short gnVCC ;

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
  DBGPRINTLN( ESP.getResetReason());  DBGPRINT("Free heap in setup: ");
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

  pinMode(A0, INPUT);

  bLoad_application_data();
};

void vApplication_wait_loop_call()
{
  strncpy((char *) homeSSID, LVDATA.gsDeviceInfo.wifi.ssid, CONST_SSID_LEN);
}

void vApp_read_vcc()
{
  short nT1;
  nT1 = analogRead(A0);
  gnVCC = nT1 * CONST_APP_VCC_REFERNCE;
  DBGPRINTF("\n A0:[%d] vcc: [%d] ", nT1, gnVCC);

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
      gulSendInterval = CONST_SEND_INTERVAL_REGULAR;
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
    //gsWenShiDu.read();

    //read battery life
    vApp_read_vcc();

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
            //print_client(clients_known[u]);
            print_client_steven(clients_known[u]);
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
  gnSecondSeq = 0;
  //LVDATA.debug_print_time();
  LVDATA.vLED_OFF();
  DBGPRINTF("\nSDK version:%s \n\r", system_get_sdk_version());
  //DBGPRINTF("AP Count: %d, Client Count: %d \n\r", aps_known_count, clients_known_count);
  //DBGPRINTLN(F("ESP8266 mini-sniff by Ray Burnette http://www.hackster.io/rayburne/projects"));
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
    //LVDATA.vGet_time();
    //DBGPRINTF("\nLVDATA.gTime.timeString: [%s]", LVDATA.gTime.timeString);
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
      if (loopCount)
        break;
    }
    if (ulGet_interval(gulOneSecondTick) > 1000) {
      gnSecondSeq++;
      gulOneSecondTick = ulReset_interval();
    }

    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()
    // Press keyboard ENTER in console with NL active to repaint the screen
  }
  find_home_channel();
  DBGPRINTLN("======================");
  //clean_data();
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
    default:
      break;

  }
  return nCommand;
}

void application_POST_call(int nCMD)
{
  ;
}


void determine_station_type()
{
  short i, k, u;
  uint8_t u8ApsFlag;
  //determine station type
  for (i = 0; i < clients_known_count; i++) {
    //默认类型是N-> 和本sniffer不在同一个homeSSID下
    u8ApsFlag = CONST_APP_SNIFF_TIMESTAMP_TYPE_NOT_HOME;

    for (u = 0; u < aps_known_count; u++)
    {
      if (! memcmp(aps_known[u].bssid, clients_known[i].bssid, CONST_ETH_MAC_LEN)) {
        //先找到client所在的AP
        if (!memcmp(aps_known[u].ssid, homeSSID, CONST_SSID_LEN)) {
          //如果此AP和homeSSID一直,类型是 A或者Y
          if (!memcmp(clients_known[i].bssid, homeMAC, CONST_ETH_MAC_LEN))
            u8ApsFlag = CONST_APP_SNIFF_TIMESTAMP_TYPE_AP;
          else
            u8ApsFlag = CONST_APP_SNIFF_TIMESTAMP_TYPE_STATION;
          break;
        }
      }
    }
    // is special
    k = (clients_known[i].timeStamp & CONST_APP_SNIFF_TIMESTAMP_TYPE_MASK) >> CONST_APP_SNIFF_TIMESTAMP_TYPE_SHIFT;
    if (k == CONST_APP_SNIFF_TIMESTAMP_TYPE_SPECIAL) {
      u8ApsFlag = CONST_APP_SNIFF_TIMESTAMP_TYPE_SPECIAL;
    }
    clients_known[i].timeStamp = clients_known[i].timeStamp & CONST_APP_SNIFF_TIMESTAMP_DATA_MASK;
    clients_known[i].timeStamp |= (u8ApsFlag & CONST_APP_SNIFF_TIMESTAMP_TYPE_SMALL_MASK) << CONST_APP_SNIFF_TIMESTAMP_TYPE_SHIFT;
  }
}

bool bSend_3001()
{
  int ret;
  short i, k, u;
  uint8_t u8ApsFlag;
  char *spStart, *spCurr;
  bool bSendFlag = false;

  DBGPRINTLN("-- send 3001 Begin --");
  DBGPRINTF("Free heap in 3001: %d\n", ESP.getFreeHeap());

  DBGPRINTF("\ngulSendInterval: %d gu8PowerupNoScanFlag: %d", gulSendInterval, gu8PowerupNoScanFlag);


  /*
    memcpy(homeMAC, LVDATA.gsDeviceInfo.bssid, CONST_ETH_MAC_LEN);

    //find home channel
    for (u = 0; u < aps_known_count; u++) {
      if (! memcmp(aps_known[u].bssid, homeMAC, CONST_ETH_MAC_LEN)) {
        homeChannel = aps_known[u].channel;
        break;
      }
    }
  */

  DBGPRINTF("\nLVDATA.gTime.timeString: [%s]", LVDATA.gTime.timeString);

  determine_station_type();

  DBGPRINTF("\nPosition:%d /  %d, rest: %d/%d \n", gnBatchCount, gnMsgBatchTotal, gnKnowCount, clients_known_count);

  //while (gnKnowCount > 0)
  if (gnKnowCount > 0) {
    int nEachBatchMax;
    gnBatchCount++;
    //gnKnowCount -= gnBatchCount * CONST_MAX_MESSGE_COUNT;
    gnKnowCount -=  CONST_MAX_MESSGE_COUNT;
    if (gnKnowCount > 0)
      nEachBatchMax = CONST_MAX_MESSGE_COUNT;
    else
      nEachBatchMax = gnKnowCount + CONST_MAX_MESSGE_COUNT;

    //each batch;
    spStart = LVDATA.gBigBuff.chpData;
    *spStart = '\0';
    spCurr = spStart;
    LVDATA.vGet_time();
    //homeSSID
    //gTime.timeString
    //sprintf(spCurr, "{\"homeSSID\":\"%s\",", homeSSID);
    //sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"SN\":\"%s\",\"lastStamp\":\"%d\",\"homeSSID\":\"%s\",", gnBatchCount, gnMsgBatchTotal, achTimeStampString, gnSecondSeq, homeSSID);
    //sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"VCC\":\"%d\",\"SN\":\"%s\",\"heap\":\"%d\",\"lastStamp\":\"%d\",\"homeSSID\":\"%s\",", gnBatchCount, gnMsgBatchTotal, gnVCC, achTimeStampString, ESP.getFreeHeap(), gnSecondSeq, homeSSID);
    sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"VCC\":\"%d\",\"SN\":\"%s\",\"heap\":\"%d\",\"lastStamp\":\"%d\",\"homeSSID\":\"%s\",", gnBatchCount, gnMsgBatchTotal, gnVCC, LVDATA.gTime.timeString, ESP.getFreeHeap(), gnSecondSeq, homeSSID);
    //sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"VCC\":\"%d\",\"SN\":\"%s\",\"lastStamp\":\"%d\",\"homeSSID\":\"%s\",\"sStamp\":\"%d\",\"sMac\":\"%02X:%02X:%02X:%02X:%02X:%02X\","\
    //sprintf(spCurr, "{\"Batch\":\"%d:%d\",\"VCC\":\"%d\",\"SN\":\"%s\",\"lastStamp\":\"%d\",\"homeSSID\":\"%s\"," , gnBatchCount, gnMsgBatchTotal, gnVCC, achTimeStampString, gnSecondSeq , homeSSID );
    //memset(&specialBeacon, 0, sizeof(specialBeacon));
    DBGPRINTF("\nspCurr: %s\n", spCurr);
    spCurr += strlen(spCurr);

    //data
    sprintf(spCurr, "\"data\":[");
    spCurr += strlen(spCurr);

    //    for (i = 0; i < clients_known_count; i++)
    for (i = (gnBatchCount - 1) * CONST_MAX_MESSGE_COUNT; i < ((gnBatchCount - 1)*CONST_MAX_MESSGE_COUNT + nEachBatchMax); i++)
    {
      sprintf(spCurr, "{\"mac\":\"");
      spCurr += strlen(spCurr);

      for (k = 0; k < 6; k++)
      {
        sprintf((spCurr ), "%02X:", clients_known[i].station[k]);
        spCurr += 3;
      }
      *(spCurr - 1) = '\"';
      *spCurr++ = ',';

      //data type
      u8ApsFlag = (clients_known[i].timeStamp & CONST_APP_SNIFF_TIMESTAMP_TYPE_MASK) >> CONST_APP_SNIFF_TIMESTAMP_TYPE_SHIFT;
      switch (u8ApsFlag)
      {
        case CONST_APP_SNIFF_TIMESTAMP_TYPE_STATION:
          sprintf(spCurr, "\"home\":\"Y\",");
          break;
        case CONST_APP_SNIFF_TIMESTAMP_TYPE_AP:
          sprintf(spCurr, "\"home\":\"A\",");
          break;
        case CONST_APP_SNIFF_TIMESTAMP_TYPE_SPECIAL:
          sprintf(spCurr, "\"home\":\"T\",");
          break;
        default:
          sprintf(spCurr, "\"home\":\"N\",");
          break;

      }
      spCurr += strlen(spCurr);

      delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()

      sprintf(spCurr, "\"dbm\":\"%d\",", clients_known[i].rssi);
      spCurr += strlen(spCurr);
      //sprintf(spCurr, "\"name\":\"\"");
      //spCurr += strlen(spCurr);
      sprintf(spCurr, "\"stamp\":\"%d\"", clients_known[i].timeStamp & CONST_APP_SNIFF_TIMESTAMP_DATA_MASK);
      spCurr += strlen(spCurr);
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

    DBGPRINTF("spStart:\n%s\n", spStart);

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
    sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":\"%s\",\"socketOut_P\":\"0\",\"socketOut_W\":\"0.0\",\"socketOutY_W\":\"0.0\",\"rlySub\":[1],\"rlystatus\":[0]}",
            CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, LVDATA.gCommandBuff.chpData);
    //DBGPRINTLN(LVDATA.gBigBuff.chpData);
    DBGPRINTLN("======================================================================");

    //DBGPRINTLN("--CALL nCommonPOST--");
    ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData);
  }


  //判断是否数据是否发送完毕
  if (gnKnowCount <= 0) {
    clean_data();
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

