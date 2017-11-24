#include <FS.h>

#define DEBUG_SIO 1

#include "LVCommon.h"


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

/*================================================================================*/
//APPLICATION_PART_BEGIN you can change this part to meet your requirements
/*================================================================================*/
#include "dht.h"

/*--------------------------------------------------------------------------------*/
//APPLICATION_DATA_PART_BEGIN
/*--------------------------------------------------------------------------------*/
#define CONST_DEVICE_VERSION "9_906_20161116"
#define CONST_DEVICE_SWVN "LVSW_TEMP_V1.0.9_20161202"
#define CONST_DEVICE_HWVN "LVHW_TEMP_V1.0.2_20161116"
#define CONST_DEVICE_MAID "9"
#define CONST_DEVICE_PID "906"
#define CONST_DEVICE_CAT "wdSd"

#define CONST_IMEI_PREFIX "8016121201"
#define CONST_APPLICATION_FILE_NAME "/application.data"
#define CONST_APPLICATION_FILE_SIZE 1024


#define dht_dpin 16 //定义8266 D0为数据接收口  

typedef struct  {
  int nWenduInt;
  int nWenduDec;
  int nShiduInt;
  int nShiduDec;
  int nPreWenduInt;
  int nPreShiduInt;
} stApplicationData;


// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SAVE_CYCLE 1000*60*60
#define CONST_READ_INTERVAL 5*1000

dht DHT;

stApplicationData gsCurrent;

unsigned long glApplicationSaveTick;

unsigned long gulApplicationTicks;

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
  sprintf(LVDATA.IMEI, "%s%010d", CONST_IMEI_PREFIX, ESP.getChipId());
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
  pinMode(dht_dpin, INPUT);
};

void vApplication_wait_loop_call()
{

}


void vApplication_connected_loop_call()
{
  // save application data
  if ((LVDATA.ulGet_interval(glApplicationSaveTick) > CONST_APPLICATION_SAVE_CYCLE))
  {
    bSave_application_data();
    LVDATA.vReset_interval(glApplicationSaveTick);

  }

  //other application read action
  if ((LVDATA.ulGet_interval(gulApplicationTicks) > CONST_READ_INTERVAL))
  {
    //char dispBuff[100];

    DHT.read11(dht_dpin);   //去library裡面找DHT.read11
    if (DHT.temperature < 100.0 && DHT.temperature > -30.0 && DHT.humidity >= 0.0 && DHT.humidity <= 100.0)
    {
      int nT1, nT2;
      gsCurrent.nWenduInt = DHT.temperature;
      gsCurrent.nWenduDec = (DHT.temperature - gsCurrent.nWenduInt) * 100 + 0.5;
      gsCurrent.nShiduInt = DHT.humidity;
      gsCurrent.nShiduDec = (DHT.humidity - gsCurrent.nShiduInt) * 100 + 0.5;
      nT1 = abs(gsCurrent.nWenduInt - gsCurrent.nPreWenduInt);
      nT2 = abs(gsCurrent.nShiduInt - gsCurrent.nPreShiduInt);
      DBGPRINTLN("DEBUG:");
      DBGPRINT(nT1);
      DBGPRINT(" ");
      DBGPRINTLN(nT2);
      if (( nT1 >= 2) || ( nT2 >= 5))
      {
        DBGPRINTLN("refersh 3001 data");
        LVDATA.vRefresh_data_3001();
      }
    }

    DBGPRINT("DEBUG:Regular Query");
    DBGPRINTLN(gulApplicationTicks);
    DBGPRINTLN(DHT.temperature);
    DBGPRINTLN(DHT.humidity);
#ifdef DEBUG_SIO
    sprintf(dispBuff, "wendu:%d.%d,shidu:%d.%d Pre:wendu:%d.0,shidu:%d.0", gsCurrent.nWenduInt, gsCurrent.nWenduDec, gsCurrent.nShiduInt, gsCurrent.nShiduDec, gsCurrent.nPreWenduInt, gsCurrent.nPreShiduInt);
    DBGPRINTLN(dispBuff);
#endif

    //vApplication_read_data();
    LVDATA.vReset_interval(gulApplicationTicks);
  }

}

void vApplication_read_data()
{

}


int nExec_application_Command(int nActionID, char  achParam[], char achInst[])
{
  int nCommand = CONST_CMD_9999;
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

void application_POST_call()
{
  ;
}

bool bSend_3001()
{
  int ret;

  DBGPRINTLN("-- send 3001 --");
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"wendu\":\"%d.%d\",\"shidu\":\"%d.%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0.0\",\"socketOutY_W\":\"0.0\",\"rlySub\":[1],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, gsCurrent.nWenduInt, gsCurrent.nWenduDec, gsCurrent.nShiduInt, gsCurrent.nShiduDec);

  DBGPRINTLN("======================================================================");
  DBGPRINTLN(LVDATA.gBigBuff.chpData);

  // This will send the POST request to the server
  ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData);

  DBGPRINTLN("send_3001:");

  gsCurrent.nPreWenduInt = gsCurrent.nWenduInt;
  gsCurrent.nPreShiduInt = gsCurrent.nShiduInt;

  DBGPRINTLN("--end--");
  return true;
}


bool bSend_9999()
{
  int ret;
  DBGPRINTLN("--send_9999--");
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"actionID\":\"%d\",\"delay\":\"0\",\"res\":\"1\"}",
          CONST_CMD_9999, LVDATA.IMEI, LVDATA.gnExecActionID);

  ret = LVDATA.nCommonPOST(LVDATA.gBigBuff.chpData, false);

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


/*--------------------------------------------------------------------------------*/
//APPLICATION_FUCTION_PART_END
/*--------------------------------------------------------------------------------*/

/*================================================================================*/
//APPLICATION_PART_END
/*================================================================================*/

