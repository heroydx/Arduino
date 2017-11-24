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

// definition
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
bool bSend_3003();
bool bSend_3701();
bool bSend_9999();
void vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen);
bool bLoad_application_data();
bool bSave_application_data();
void special_main_loop();

void setup()
{

  SERIAL_DEBUG_BEGIN
  //Serial.begin(115200);
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  //randomSeed(analogRead(2));

  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  vGenerate_IMEI();
  vApplication_setup_call();
  LVDATA.mainsetup();
  check_sys_data();
  randomSeed(micros());
}

void loop()
{
  LVDATA.mainloop();
  special_main_loop();
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
#define CONST_DEVICE_VERSION "0_3_20170725"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/
#define CONST_DEVICE_SWVN "LVSW_COMM_NON_V1.0.0_20170725"
#define CONST_DEVICE_HWVN "LVHW_GATE_4M1M_V1.0.0_20161116"
#define CONST_DEVICE_MAID "0"
#define CONST_DEVICE_PID "3"
#define CONST_DEVICE_CAT "coMm"


//#define CONST_IMEI_PREFIX "8019980908"
//#define CONST_IMEI_PREFIX   "8117050401"
#define CONST_IMEI_PREFIX_1 "81170"
//#define CONST_IMEI_PREFIX_2 "50401"
#define CONST_IMEI_PREFIX_2 "725"

// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SIO_QUERY_INTERVAL  2000
#define CONST_APPLICATION_SAVE_CYCLE 1000*60*60
unsigned long glApplicationSIOQueryTick;
unsigned long glApplicationSaveTick;

unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 10*1000

unsigned long gul3001TimeOutTick;

unsigned long gulSeqNo;

#define CONST_CMD_3003 3003
#define CONST_CMD_3701 3701
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
  uint8_t nT1;
  nT1 = random(99);
  //sprintf(LVDATA.IMEI, "%s%010d", CONST_IMEI_PREFIX, ESP.getChipId());
  sprintf(LVDATA.IMEI, "%s%010d%s%02d", CONST_IMEI_PREFIX_1, ESP.getChipId(), CONST_IMEI_PREFIX_2, nT1);
}

// application init data is used in setup
void vApplication_setup_call()
{
  DBGPRINTLN("----Application_setup_call()----");
  strncpy(LVDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);

  bLoad_application_data();
};


void vApplication_wait_loop_call()
{
}

void vApplication_connected_loop_call()
{

  //send request
  if ((ulGet_interval(glApplicationSIOQueryTick) > CONST_APPLICATION_SIO_QUERY_INTERVAL))
  {
    glApplicationSIOQueryTick = ulReset_interval();
  }

  // save application data
  if ((ulGet_interval(glApplicationSaveTick) > CONST_APPLICATION_SAVE_CYCLE))
  {
    bSave_application_data();
    glApplicationSaveTick = ulReset_interval();
  }

  //other application read action
  if ((ulGet_interval(gulApplicationTicks) > CONST_READ_INTERVAL))
  {
    //LVDATA.push_cmd(CONST_CMD_3001, 1, 200);
    vApplication_read_data();
    gulApplicationTicks = ulReset_interval();
  }

}

void vApplication_read_data()
{
}


int nExec_application_Command(int nActionID, char  achParam[], char achInst[])
{
  int nCommand = CONST_CMD_9999;
  DBGPRINTLN("nExec_application_Command 9999");
  LVDATA.push_cmd(CONST_CMD_9999);
  return nCommand;
}

void application_POST_call(int nCMD)
{
}

void special_main_loop()
{
  //DBGPRINTLN("\nSpecial main loop");
  //如果长时间没有上报数据,清除发送状态标志
  if (ulGet_interval(gul3001TimeOutTick) > (LVDATA.glConnIntervalTick * 2))
  {
    //DBGPRINTLN("\n3001 clean gnSendDataStatus!");
    //LVDATA.gnSendDatatStaus = 0;
  }
  //如果长时间没有上报数据,系统重启
  if (ulGet_interval(gul3001TimeOutTick) > (LVDATA.glConnIntervalTick * 3))
  {
    DBGPRINTLN("\n3001 timeout restart!");
    LVDATA.vSystem_restart(CONST_SYS_RESTART_REASON_NOPOST_TIME_OUT);
  }
}

bool bSend_3001()
{
  int ret;
  short i;
  char *spCurr;
  DBGPRINTLN("\n-- send 3001 --");
  spCurr = LVDATA.gCommandBuff.chpData;
  for (i = 0; i < 245; i++)
  {
    sprintf(spCurr, "|%08d|", i);
    spCurr += 10;
  }
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"SN\":\"%d\",\"testMsg\":\"%s\",\"socketOut_P\":\"0\",\"socketOut_W\":\"0.0\",\"socketOutY_W\":\"0.0\",\"rlySub\":[1],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, gulSeqNo, LVDATA.gCommandBuff.chpData);
  gulSeqNo++;
  DBGPRINTLN(LVDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LVDATA.nCommonPOST( LVDATA.gBigBuff.chpData);
  gul3001TimeOutTick = ulReset_interval();
  DBGPRINTLN("--end--");

  return true;
}


bool bSend_3003()
{
  int ret;
  DBGPRINTLN("-- send 3003 --");
  DBGPRINTLN("--end--");
  return true;
}


bool bSend_9999()
{
  int ret;
  DBGPRINTLN("--send_9999--");
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"actionID\":\"%d\",\"delay\":\"0\",\"res\":\"%d\"}",
          CONST_CMD_9999, LVDATA.IMEI, LVDATA.gnExecActionID, 1);

  ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData, false);

  DBGPRINTLN("--end--");

  return true;
}


/* load application data, return True==Success*/
bool bLoad_application_data()
{
  bool ret = false;
  ret = true;
  return ret;
}

/* save application data, return True = success */
bool bSave_application_data()
{
  bool ret = false;
  ret = true;
  return ret;

}


/*--------------------------------------------------------------------------------*/
//APPLICATION_PART_END
/*--------------------------------------------------------------------------------*/

