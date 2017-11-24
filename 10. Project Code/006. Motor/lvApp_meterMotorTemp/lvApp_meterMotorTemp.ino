#include <FS.h>

//#define DEBUG_SIO Serial

#include "LVCommon.h"

/*================================================================================*/
//MAIN_PART_BEGIN
/*============================================================================f====*/
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
bool bSend_9999();
void vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen);
bool bLoad_application_data();
bool bSave_application_data();

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
#define CONST_DEVICE_VERSION "9_909_20170512"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/
#define CONST_DEVICE_SWVN "LVSW_METER_MOTORTEMP_V1.0.4_20170526"
#define CONST_DEVICE_HWVN "LVHW_METER_4M1M_V1.0.3_20170416"
#define CONST_DEVICE_ADDTION_HWVN ""
#define CONST_DEVICE_MAID "9"
#define CONST_DEVICE_PID "909"
#define CONST_DEVICE_CAT "meMo"


//#define CONST_IMEI_PREFIX "8019980908"
//#define CONST_IMEI_PREFIX   "8117050401"
#define CONST_IMEI_PREFIX_1 "81170"
//#define CONST_IMEI_PREFIX_2 "51201"
#define CONST_IMEI_PREFIX_2 "526"

#include "Meter_PZ.h"
Meter_DQ gsMeter;

// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SIO_QUERY_INTERVAL  6000
#define CONST_APPLICATION_SAVE_CYCLE 1000*60*60
unsigned long glApplicationSIOQueryTick;
unsigned long glApplicationSaveTick;

unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 5*1000

////irda part
//#include "LVIrda.h"
//LVIrda LVIRDA;

//dht temperautre 温度部分

//#include "LVwenshiduIrda.h"
#include "wenshidu.h"
wenshidu gsWenShiDu;

//Motor control
#include "Motor_DQ.h"

#define CONST_APP_MOTOR_SPEED_DIFF_REFRESH_VAL 30
short gnPreMotorSpeed;


#define CONST_APP_MOTOR_
motor_DQ gsMotorData;

//Motor wendu control
#include "MotorWenshidu.h"

MotorWenshidu gsMotorWenshidu;
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
  short nLen;
  DBGPRINTLN("----Application_setup_call()----");
  strncpy(LVDATA.gsDeviceInfo.Version, CONST_DEVICE_VERSION, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.SWVN, CONST_DEVICE_SWVN, CONST_SYS_VERSION_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.HWVN, CONST_DEVICE_HWVN, CONST_SYS_VERSION_LENGTH);
  nLen=strlen(LVDATA.gsDeviceInfo.HWVN);
  strncpy(LVDATA.gsDeviceInfo.HWVN+nLen,CONST_DEVICE_ADDTION_HWVN,CONST_SYS_VERSION_LENGTH-nLen-1);
  strncpy(LVDATA.gsDeviceInfo.MAID, CONST_DEVICE_MAID, CONST_SYS_MAID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.PID, CONST_DEVICE_PID, CONST_SYS_PID_LENGTH);
  strncpy(LVDATA.gsDeviceInfo.CAT, CONST_DEVICE_CAT, CONST_SYS_CAT_LENGTH);

  gsMeter.begin(LVDATA.gBigBuff.chpData);


  bLoad_application_data();
  //pinMode(dht_dpin, INPUT);
  //gsWenShiDu.begin();
  DBGPRINTLN("--will call gsWenShiDu.begin(&LVIRDA, LVDATA.gBigBuff.chpData)-- ");
  //gsWenShiDu.begin(&LVIRDA, LVDATA.gBigBuff.chpData);
  gsWenShiDu.begin();
  if (gsWenShiDu.devReady() > 0) {
    DBGPRINTF("\n Wenshidu ready! [%d]", gsWenShiDu.devReady());
  }
  else {
    DBGPRINTLN("\n Wenshidu Not ready!");

  }

  gsMotorData.begin();

  if (gsMotorData.devReady()) {
    DBGPRINTLN("\n Motor Control ready!");
  }
  else {
    DBGPRINTLN("\n Motor Control Not ready!");
  }

  gsMotorWenshidu.begin(&gsMotorData, &gsWenShiDu, LVDATA.gBigBuff.chpData);
};


void vApplication_wait_loop_call()
{
}

void vApplication_connected_loop_call()
{
  if (gsMeter.read()) {
    if ((abs(gsMeter.nWDianyaInt - gsMeter.nDianyaInt) > CONST_WARNING_DIANYA)
        || (abs(gsMeter.nWGonglv - gsMeter.nGonglv) > CONST_WARNING_GONGLV)
        //|| (gsMeter.nWGonglv < CONST_WARNING_GONGLV_LOW && gsMeter.nGonglv > CONST_WARNING_GONGLV_LOW)
        || (gsMeter.nWGonglv == 0 && gsMeter.nGonglv > CONST_WARNING_GONGLV_ZERO)
        || (gsMeter.nWGonglv > CONST_WARNING_GONGLV_ZERO && gsMeter.nGonglv == 0)
       ) {
      DBGPRINTLN("==================REFRESH 3001=================");
      LVDATA.vRefresh_data_3001();
    }

  }

  if (LVDATA.gTime.hour != gsMeter.preHour) {
    gsMeter.nHDianliangInt = gsMeter.nDianliangInt;
    gsMeter.nHDianliangDec = gsMeter.nDianliangDec;
    gsMeter.lHMeterDianliang = gsMeter.lMeterDianliang;
    gsMeter.preHour = LVDATA.gTime.hour;
  }

  if (LVDATA.gTime.day != gsMeter.preDay)
  {
    gsMeter.nYDianliangInt = gsMeter.nDianliangInt;
    gsMeter.nYDianliangDec = gsMeter.nDianliangDec;
    gsMeter.lYMeterDianliang = gsMeter.lMeterDianliang;
    gsMeter.preDay = LVDATA.gTime.day;
  }


  //send request
  if ((ulGet_interval(glApplicationSIOQueryTick) > CONST_APPLICATION_SIO_QUERY_INTERVAL))
  {
    //gsWenShiDu.debug_print_info();

    // speed change;

    if ( gnPreMotorSpeed != gsMotorData.currStat.speed) {
      short nT1;
      nT1 = abs(gnPreMotorSpeed - gsMotorData.currStat.speed);
      if (nT1 > CONST_APP_MOTOR_SPEED_DIFF_REFRESH_VAL) {
        LVDATA.vRefresh_data_3001();
      }
      gnPreMotorSpeed = gsMotorData.currStat.speed;

    }

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
    vApplication_read_data();
    gulApplicationTicks = ulReset_interval();
  }

  gsMotorData.motor_loop();

}

void vApplication_read_data()
{
  short nT1;
  nT1 = gsWenShiDu.read();
  gsMotorWenshidu.loop();
  //gsMotorWenshidu.debug_print_rule();
  //gsWenShiDu.set_curr_power(gsMeter.nGonglv);
  if (nT1 > 0) {
    DBGPRINTF("\n Wendu %d.%d, shidu %d.%d", gsWenShiDu.nWenduInt, gsWenShiDu.nWenduDec, gsWenShiDu.nShiduInt, gsWenShiDu.nShiduDec);
  }
  if (nT1 == 2)
  {
    DBGPRINTLN("refersh 3001 data");
    LVDATA.vRefresh_data_3001();
  }
}


int nExec_application_Command(int nActionID, char  achParam[], char achInst[])
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
      LVDATA.push_cmd(CONST_CMD_9999);
      break;

    case 2:
      //digitalWrite(RELAY_ID, HIGH);
      //gnStatusRelay = digitalRead(RELAY_ID);
      //LVDATA.gnServerCommand = CONST_CMD_9999;
      //LVDATA.push_cmd(CONST_CMD_9999);
      break;

    case 761:
      LVDATA.push_cmd(CONST_CMD_9999);
      //LVDATA.push_cmd(CONST_CMD_9999,3,20);
      DBGPRINTLN("--gsMotorWenshidu.decode_761_command(achParam)--");
      gsMotorWenshidu.decode_761_command(achParam);
      LVDATA.push_cmd(CONST_CMD_3001,3,20);
      //LVDATA.push_cmd(CONST_CMD_9999);
      break;


    case 802:
      LVDATA.push_cmd(CONST_CMD_9999);
      DBGPRINTLN("--gsWenShiDu.decode_802_command(achParam)--");
      gsWenShiDu.decode_802_command(achParam);
      break;

    default:
      break;

  }

  return nCommand;
}

void application_POST_call(int nCMD)
{
  //DBGPRINTLN("APPLICATION_POST_CALL");
  switch (nCMD)
  {
    default:
      break;
  }
}


bool bSend_3001()
{
  int ret;
  unsigned short socketOut_P;
  DBGPRINTLN("-- send 3001 --");
  short i, pos;
  char *strPtr;
  strPtr = LVDATA.gCommandBuff.chpData;
  strcpy(strPtr, "[");
  strPtr += strlen(strPtr);
  pos = gsMeter.nLastGonglvPos;
  for (i = 0; i < CONST_APP_METER_GONGLV_LAST_MAX_LEN; i++) {
    sprintf(strPtr, "%d,", gsMeter.anLastGonglv[pos]);
    pos++;
    if (pos > CONST_APP_METER_GONGLV_LAST_MAX_LEN) {
      pos = 0;
    }
    strPtr += strlen(strPtr);
  }
  strPtr--;
  if (*strPtr == ',') {
    *strPtr = ' ';
  }
  strcpy(strPtr, "]");
  /*
    sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"dianliang\":\"%d.%03d\",\"dianya\":\"%d.%d\",\"dianliu\":\"%d.%02d\",\"gonglv\":\"%d\"},\"socketOut_P\":\"%d\",\"socketOut_W\":\"%d.%03d\",\"socketOutY_W\":\"%d.%03d\",\"rlySub\":[1],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, gsMeter.nDianliangInt, gsMeter.nDianliangDec, gsMeter.nDianyaInt, gsMeter.nDianyaDec, gsMeter.nDianliuInt, gsMeter.nDianliuDec,gsMeter.nGonglv, \
          gsMeter.nGonglv, gsMeter.nDianliangInt, gsMeter.nDianliangDec, gsMeter.nYDianliangInt, gsMeter.nYDianliangDec);
  */
  if (gsMeter.bFirstReadDianliangFlag == true)
    ret = 1;
  else
    ret = 0;
  if (gsMeter.nGonglv > CONST_AIR_CONDITION_GONGLV_THRESHOLD) {
    socketOut_P = gsMeter.nGonglv;
  }
  else {
    socketOut_P = 0;
  }
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"dianliang\":\"%d.%03d\",\"dianya\":\"%d.%d\",\"dianliu\":\"%d.%02d\",\"gonglv\":\"%d\",\"sioErr\":\"%d\",\"meter\":\"%d\",\"lmeter\":\"%d\",\"Ymeter\":\"%d\",\"first\":\"%d\",\"last\":%s,\"stdev\":\"%d\",\"DHT\":\"%d\", \"wendu\":\"%d.%d\",\"shidu\":\"%d.%d\",\"zhuansu\":\"%d\",\"maxzhuansu\":\"%d\", \"autoControl\":\"%d\"},\"socketOut_P\":\"%d\",\"socketOut_W\":\"%d.%03d\",\"socketOutY_W\":\"%d.%03d\",\"rlySub\":[1],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, gsMeter.nDianliangInt, gsMeter.nDianliangDec, gsMeter.nDianyaInt, gsMeter.nDianyaDec, gsMeter.nDianliuInt, gsMeter.nDianliuDec, gsMeter.nGonglv, \
          gsMeter.failTotal, gsMeter.lCurrMeterDianliang, gsMeter.lMeterDianliang, gsMeter.lYMeterDianliang, ret, LVDATA.gCommandBuff.chpData, gsMeter.stdev, \
          gsWenShiDu.devReady(), gsWenShiDu.nWenduInt, gsWenShiDu.nWenduDec, gsWenShiDu.nShiduInt, gsWenShiDu.nShiduDec, \
          gsMotorData.currStat.speed, gsMotorData.currStat.maxSpeed, gsMotorWenshidu.autoControlEnable, \
          socketOut_P, gsMeter.nDianliangInt, gsMeter.nDianliangDec, gsMeter.nYDianliangInt, gsMeter.nYDianliangDec);
  //DBGPRINTLN(LVDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LVDATA.nCommonPOST( LVDATA.gBigBuff.chpData);
  //  ghTcpCommand.print();
  //  ghTcpCommand.print(thisData);

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


void vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen) {
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

