#include <FS.h>

#include "LVCommon.h"

/*================================================================================*/
//MAIN_PART_BEGIN
/*================================================================================*/
//DON'T CHANGE THE MAIN PART CODE

LVCommon LVDATA;
//是否保持wifi链接,默认是true,否则可以在没有wifi的状态下工作。
#define CONST_SYS_KEEP_WIFI_CONNECTION false

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
void vApplication_main_loop_call();
void vApplication_read_data();
int nExec_application_Command(int nActionID, char achParam[], char achInst[]);
void application_POST_call(int nCMD);
bool bSend_3001();
bool bSend_9999();
void vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen);
bool bLoad_application_data();
bool bSave_application_data();

void EC11_setup();
void EC11_loop();
void EC11_CH1_interrupt();

void setup()
{
  SERIAL_DEBUG_BEGIN
  if (!SPIFFS.begin())
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
  }
  randomSeed(micros());
  vGenerate_IMEI();
  vApplication_setup_call();
  LVDATA.mainsetup();
  check_sys_data();
  LVDATA.vLED_poweron();
  LVDATA.bKeepConnection = CONST_SYS_KEEP_WIFI_CONNECTION;

}

void loop()
{
  //LVDATA.mainloop();
  vApplication_main_loop_call();
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
#define CONST_DEVICE_VERSION "8_7_20171115"
/* 软件版本解释：
   LVSW_GATE_A_S01_V1.0.1_20170301
   软件_设备类型_子类型_子参数_版本_日期
   例如： GATE==网关,探针类别; 子类型=A; 子参数=S01(网关扫描参数 CONST_HOMEAP_SPLIT =1),
   硬件版本解释：
   LVHW_GATE_4M1M_V1.0.0_20161116
   软件_设备类型_内存选择参数_版本_日期
*/

#define CONST_DEVICE_SWVN "LVSW_SIGNAL_GENERATOR_V0.0.1_20171115"
#define CONST_DEVICE_HWVN "LVHW_NORMAL_4M1M_V1.0.0_20171101"
#define CONST_DEVICE_MAID "8"
#define CONST_DEVICE_PID "7"
#define CONST_DEVICE_CAT "siGt"


//#define CONST_IMEI_PREFIX "8019980908"
//#define CONST_IMEI_PREFIX   "8117050401"
#define CONST_IMEI_PREFIX_1 "81171"
//#define CONST_IMEI_PREFIX_2 "50401"
#define CONST_IMEI_PREFIX_2 "115"

#include "SG_JDS6600.h"
SG_JDS gsSG;

//NEEDTOMODIFY BEGIN
#include "Button.h"
//Button gsButton;
//#define CONST_APP_SG_SAVE_PARAM_PIN    D0 //GPIO 16

#define CONST_APP_SG_SAVE_PARAM_PIN    02 //GPIO 02

#include "RotaryEncoder.h"
RotaryEncoder gsEC11_CH1_AMP;
//#define CONST_APP_SG_CH1_AMP_ENCODER_A_PIN D4 //GPIO 2
//#define CONST_APP_SG_CH1_AMP_ENCODER_B_PIN D8 //GPIO 15



#define CONST_APP_SG_CH1_AMP_ENCODER_A_PIN 14 //GPIO 14
#define CONST_APP_SG_CH1_AMP_ENCODER_B_PIN 16 //GPIO 16
long glCH1_amp_currPosition;
long glCH1_amp_prePosition;

//借用调占空比的引脚控制CH2频率
RotaryEncoder gsEC11_CH2_FREQ;
#define CONST_APP_SG_CH2_FREQ_ENCODER_A_PIN D2 //GPIO 04
#define CONST_APP_SG_CH2_FREQ_ENCODER_B_PIN D1 //GPIO 05
long glCH2_freq_currPosition;
long glCH2_freq_prePostion;


RotaryEncoder gsEC11_CH2_DUTY;
#define CONST_APP_SG_CH2_DUTY_ENCODER_A_PIN D6 //GPIO 12
#define CONST_APP_SG_CH2_DUTY_ENCODER_B_PIN 13 //GPIO 13
long glCH2_duty_currPosition;
long glCH2_duty_prePostion;

long isChangeFlag;

#define CONST_APP_SG_SAVING_TIME_TICK 1000
unsigned long glSavingTick;



//NEEDTOMODIFY END

// application data save cycle by minutes,default = 60 mins
#define CONST_APPLICATION_SIO_QUERY_INTERVAL  2000
#define CONST_APPLICATION_SAVE_CYCLE (1000*60*60)
unsigned long glApplicationSIOQueryTick;
unsigned long glApplicationSaveTick;

unsigned long gulApplicationTicks;
#define CONST_READ_INTERVAL 5*1000

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
  gsSG.begin();
  EC11_setup();
  //Serial.begin(115200);
  //Serial.println("\n TEST Serial.println");
  bLoad_application_data();
};


void vApplication_wait_loop_call()
{
}

void vApplication_connected_loop_call()
{
  //gsSG.loop();
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
    vApplication_read_data();
    gulApplicationTicks = ulReset_interval();
  }

}

void vApplication_main_loop_call()
{
  gsSG.loop();
  if ((ulGet_interval(glSavingTick) > CONST_APP_SG_SAVING_TIME_TICK))
  {
    short nTflag = 0;
    DBGPRINTLN("\n CONST_APP_SG_SAVING_TIME_TICK out");
    if (1) {
      long p1;
      DBGPRINTLN("\n CONST_APP_SG_SAVING_TIME_TICK in");

      //判断幅度是否超出范围
      //nTflag = 0;
      p1 = gsSG.currStat[CONST_APP_SG_CH1_NUM - 1].amp;
      if (p1 > CONST_APP_SG_CONFIG_AMP_MAX) {
        p1 = CONST_APP_SG_CONFIG_AMP_MAX;
        nTflag = 1;
      }
      if (p1 < CONST_APP_SG_CONFIG_AMP_MIN) {
        p1 = CONST_APP_SG_CONFIG_AMP_MIN;
        nTflag = 1;
      }
      if (nTflag) {
        gsSG.prepare_cmd_param((char *)gsSG.au8sendBuff, CONST_APP_SG_CMD_TYPE_STAND_WRITE, CONST_APP_SG_FMT_AMP_CH1, p1);
        DBGPRINTF("\n%s", gsSG.au8sendBuff);
        gsSG.send_CMD_to_device((char *)gsSG.au8sendBuff);
        //gsSG.bSave_config();
        delay(1);
      }
      //判断频率是否超出范围
      //nTflag = 0;
      p1 = gsSG.currStat[CONST_APP_SG_CH1_NUM - 1].freq;
      if (p1 > CONST_APP_SG_CONFIG_FREQ_MAX) {
        p1 = CONST_APP_SG_CONFIG_FREQ_MAX;
        nTflag = 1;
      }
      if (p1 < CONST_APP_SG_CONFIG_FREQ_MIN) {
        p1 = CONST_APP_SG_CONFIG_FREQ_MIN;
        nTflag = 1;
      }
      if (nTflag) {
        gsSG.prepare_cmd_param((char *)gsSG.au8sendBuff, CONST_APP_SG_CMD_TYPE_STAND_WRITE, CONST_APP_SG_FMT_FREQ_CH1, p1);
        DBGPRINTF("\n%s", gsSG.au8sendBuff);
        gsSG.send_CMD_to_device((char *)gsSG.au8sendBuff);
        //gsSG.bSave_config();
        delay(1);
      }

      //判断占空比是否超出范围
      //nTflag = 0;
      p1 = gsSG.currStat[CONST_APP_SG_CH2_NUM - 1].duty;
      if (p1 > CONST_APP_SG_CONFIG_DUTY_MAX) {
        p1 = CONST_APP_SG_CONFIG_DUTY_MAX;
        nTflag = 1;
      }
      if (p1 < CONST_APP_SG_CONFIG_DUTY_MIN) {
        p1 = CONST_APP_SG_CONFIG_DUTY_MIN;
        nTflag = 1;
      }
      if (nTflag) {
        gsSG.prepare_cmd_param((char *)gsSG.au8sendBuff, CONST_APP_SG_CMD_TYPE_STAND_WRITE, CONST_APP_SG_FMT_DUTY_CH2, p1);
        DBGPRINTF("\n%s", gsSG.au8sendBuff);
        gsSG.send_CMD_to_device((char *)gsSG.au8sendBuff);
        //gsSG.bSave_config();
        delay(1);
      }
      if (isChangeFlag > 0 || gsSG.bStatChangeFlag) {
        isChangeFlag = 0;
        gsSG.bSave_config();
      }
      //      if (isChangeFlag > 0) {
      //        isChangeFlag = 0;
      //        gsSG.bSave_config();
      //      }
    }
    glSavingTick = ulReset_interval();

  }
  EC11_loop();
}

//NEEDTOMODIFY BEGIN
void EC11_setup()
{
  DBGPRINTLN("\n EC11_setup()");
  //gsButton.begin(CONST_APP_SG_SAVE_PARAM_PIN);

  gsEC11_CH1_AMP.begin(CONST_APP_SG_CH1_AMP_ENCODER_A_PIN, CONST_APP_SG_CH1_AMP_ENCODER_B_PIN);
  gsEC11_CH2_FREQ.begin(CONST_APP_SG_CH2_FREQ_ENCODER_A_PIN, CONST_APP_SG_CH2_FREQ_ENCODER_B_PIN);
  gsEC11_CH2_DUTY.begin(CONST_APP_SG_CH2_DUTY_ENCODER_A_PIN, CONST_APP_SG_CH2_DUTY_ENCODER_B_PIN);

  //
  //  pinMode(CONST_APP_SG_CH1_AMP_ENCODER_A_PIN, INPUT);
  //  pinMode(CONST_APP_SG_CH1_AMP_ENCODER_B_PIN, INPUT);
  //
  //  attachInterrupt(CONST_APP_SG_CH1_AMP_ENCODER_A_PIN, EC11_CH1_interrupt, CHANGE);
}

void EC11_loop()
{
  int theStat;
  //DBGPRINTLN("\n EC11_loop()");
  /*
    theStat = gsButton.read();
    if (theStat) {
    DBGPRINTF("\nSwitch Pressed:[%d] and will save data", theStat);
    gsSG.bSave_config();
    }
  */
  //DBGPRINTF("\n glCH1_amp_currPosition += gsEC11_CH1_AMP.read() [%d]",glCH1_amp_currPosition);

  // AMP Part
  glCH1_amp_currPosition += gsEC11_CH1_AMP.read();
  if (glCH1_amp_currPosition != 0) {
    unsigned long val;
    val = glCH1_amp_currPosition * CONST_APP_SG_CONFIG_AMP_STEP;
    DBGPRINTF("\n Position:[%d] amp[%d] val[%d] ", glCH1_amp_currPosition, gsSG.currStat[0].amp, val);
    gsSG.trans_server_CMD(1, "AMP", val);
    glCH1_amp_currPosition = 0;
    isChangeFlag++;
  }



  // FREQ Part
  glCH2_freq_currPosition += gsEC11_CH2_FREQ.read();
  if (glCH2_freq_currPosition != 0) {
    unsigned long val;
    val = glCH2_freq_currPosition * CONST_APP_SG_CONFIG_FREQ_STEP;
    DBGPRINTF("\n Position:[%d] freq[%d] val[%d] ", glCH2_freq_currPosition, gsSG.currStat[1].freq, val);
    gsSG.trans_server_CMD(1, "FREQ", val);
    glCH2_freq_currPosition = 0;
    isChangeFlag++;
  }



  // DUTY Part
  glCH2_duty_currPosition += gsEC11_CH2_DUTY.read(); //先读取，再加，再扔回！
  if (glCH2_duty_currPosition != 0) {
    unsigned long val;
    val = glCH2_duty_currPosition * CONST_APP_SG_CONFIG_DUTY_STEP;
    DBGPRINTF("\n Position:[%d] duty[%d] val[%d] ", glCH2_duty_currPosition, gsSG.currStat[1].duty, val);
    gsSG.trans_server_CMD(2, "DUTY", val);
    glCH2_duty_currPosition = 0;
    isChangeFlag++;
  }

}

void EC11_CH1_interrupt() {
  // found a low-to-high on channel A ENA脚下降沿中断触发
  if (digitalRead(CONST_APP_SG_CH1_AMP_ENCODER_A_PIN) == HIGH) {
    // check channel B to see which way 查询ENB的电平以确认是顺时针还是逆时针旋转
    if (digitalRead(CONST_APP_SG_CH1_AMP_ENCODER_B_PIN) == HIGH)
      glCH1_amp_currPosition++;
  }
  // found a high-to-low on channel A ENA脚上升沿中断触发
  else {
    // check channel B to see which way 查询ENB的电平以确认是顺时针还是逆时针旋转
    if (digitalRead(CONST_APP_SG_CH1_AMP_ENCODER_B_PIN) == HIGH)
      glCH1_amp_currPosition--;
  }
}
//NEEDTOMODIFY END

void vApplication_read_data()
{
}


int nExec_application_Command(int nActionID, char  achParam[], char achInst[])
{
  int nCommand = CONST_CMD_9999;
  DBGPRINT("exe_serverCommand=[");
  DBGPRINT(nActionID);
  DBGPRINTLN("]");

  switch (nActionID)
  {
    case 762:
      LVDATA.push_cmd(CONST_CMD_9999);
      gsSG.decode_762_command(achParam);
      LVDATA.push_cmd(CONST_CMD_3001, 3, 20);
      break;

    default:
      //LVDATA.push_cmd(CONST_CMD_9999);
      break;
  }
  return nCommand;
}

void application_POST_call(int nCMD)
{
}


bool bSend_3001()
{
  int ret;
  unsigned short socketOut_P;
  DBGPRINTLN("-- send 3001 --");
  sprintf(LVDATA.gBigBuff.chpData, "{\"CMD\":\"%d\",\"devID\":\"%s\",\"CAT\":\"%s\",\"CDATA\":{\"Ready\":\"%d\",\"freq0\":\"%d\",\"phase0\":\"%d\",\"amp0\":\"%d\",\"duty0\":\"%d\"},\"socketOut_P\":\"0\",\"socketOut_W\":\"0.0\",\"socketOutY_W\":\"0.0\",\"rlySub\":[1],\"rlystatus\":[0]}",
          CONST_CMD_3001, LVDATA.IMEI, LVDATA.gsDeviceInfo.CAT, gsSG.devReady(), gsSG.currStat[0].freq, gsSG.currStat[0].phase, gsSG.currStat[0].amp, gsSG.currStat[0].duty);
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

