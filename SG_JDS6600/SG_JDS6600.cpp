#include "SG_JDS6600.h"

//modified on 2017/11/14 by Steven Lian steven.lian@gmail.com

static SoftwareSerial SerialSG(CONST_SG_RX_PORT, CONST_SG_TX_PORT); //
static ListArray gsDeviceCMDList(CONST_APP_SG_SEND_BUF_COUNT, CONST_APP_SG_RECV_SENDREV_BUFFLEN);


SG_JDS::SG_JDS()
{
  
}

bool SG_JDS::begin()
{
  bool ret=false;
  short i;
  devStatusReady=false;
  bStatChangeFlag=false;  
  //  Serial.begin(CONST_APP_SG_RECV_SIO_BAUD_DATA);
  APP_SERIAL.begin(CONST_APP_SG_RECV_SIO_BAUD_DATA);
  while (!APP_SERIAL) {
    ; // wait for serial port to connect. Needed for Leonardo only
  };
  DBGPRINTLN("\n\n");
  DBGPRINTLN("--Signal Generator INIT BEGIN--");
  //DBGPRINTF("\nAPP_SERIAL=[%s]",APP_SERIAL);

  short nTimes = 5;
  while (nTimes > 0) {
    delay(1);
    deviceReady = is_device_exist();
    if (deviceReady) {
      DBGPRINTLN("Signal Generator ready!!!");
      break;
    }
    nTimes--;
  }

  if (!bLoad_config()) {
    set_default_config();
	delay(10);
	bSave_config();
  }

  restore_setting();
  //device status check list
  anDevStatusCheckList[0]=CONST_APP_SG_FMT_MODEL;
  anDevStatusCheckList[1]=CONST_APP_SG_FMT_SN;
  anDevStatusCheckList[2]=CONST_APP_SG_FMT_WAVE_CH1;
  anDevStatusCheckList[3]=CONST_APP_SG_FMT_WAVE_CH2;
  anDevStatusCheckList[4]=CONST_APP_SG_FMT_FREQ_CH1;
  anDevStatusCheckList[5]=CONST_APP_SG_FMT_FREQ_CH2;
  anDevStatusCheckList[6]=CONST_APP_SG_FMT_AMP_CH1;
  anDevStatusCheckList[7]=CONST_APP_SG_FMT_AMP_CH2;
  anDevStatusCheckList[8]=CONST_APP_SG_FMT_BIAS_CH1;
  anDevStatusCheckList[9]=CONST_APP_SG_FMT_BIAS_CH2;
  anDevStatusCheckList[10]=CONST_APP_SG_FMT_DUTY_CH1;
  anDevStatusCheckList[11]=CONST_APP_SG_FMT_DUTY_CH2;
  anDevStatusCheckList[12]=CONST_APP_SG_FMT_PHASE_CH1;
  anDevStatusCheckList[13]=CONST_APP_SG_FMT_TRACE_CH2;
  anDevStatusCheckList[14]=CONST_APP_SG_FMT_EXTEND_CH2;
  anDevStatusCheckList[15]=CONST_APP_SG_FMT_COUPLING;
  anDevStatusCheckList[16]=CONST_APP_SG_FMT_GATE;
  anDevStatusCheckList[17]=CONST_APP_SG_FMT_MEASURE;
  anDevStatusCheckList[18]=CONST_APP_SG_FMT_SWEEP_BEGIN;
  anDevStatusCheckList[19]=CONST_APP_SG_FMT_SWEEP_END;
  anDevStatusCheckList[20]=CONST_APP_SG_FMT_SWEEP_TIME;
  anDevStatusCheckList[21]=CONST_APP_SG_FMT_SWEEP_DIRECTION;
  anDevStatusCheckList[22]=CONST_APP_SG_FMT_SWEEP_MODE;
  anDevStatusCheckList[23]=CONST_APP_SG_FMT_PULSE_WIDTH;
  anDevStatusCheckList[24]=CONST_APP_SG_FMT_PULSE_CYCLE;
  anDevStatusCheckList[25]=CONST_APP_SG_FMT_PULSE_BIAS;
  anDevStatusCheckList[26]=CONST_APP_SG_FMT_PULSE_AMP;
  anDevStatusCheckList[27]=CONST_APP_SG_FMT_BURST;
  anDevStatusCheckList[28]=-1;
  devStatusCount=0;
  
  ulSIOQueryTickThreshold=CONST_APP_SG_STAT_QUERY_FAST;
  bSendBusyFlag=false;
  return ret;

}

void SG_JDS::prepare_cmd_param(char *dataPtr,char *param)
{
  sprintf(dataPtr,":%s\n\r",param);
}

void SG_JDS::prepare_cmd_param(char *dataPtr,short cmd,short actionID, long param)
{
  sprintf(dataPtr,":%c%02d=%d.\n\r",cmd,actionID,param);
}

void SG_JDS::prepare_cmd_param(char *dataPtr,short cmd,short actionID, long p1, long p2)
{
  sprintf(dataPtr,":%c%02d=%d,%d.\n\r",cmd,actionID,p1,p2);
}

void SG_JDS::prepare_cmd_param(char *dataPtr,short cmd,short actionID, long p1, long p2, long p3, long p4, long p5)
{
  sprintf(dataPtr,":%c%02d=%d,%d,%d,%d,%d.\n\r",cmd,actionID,p1,p2,p3,p4,p5);
}

bool SG_JDS::set_default_config()
{
  bool ret = false;
  //准备并发送默认状态数据
  DBGPRINTLN("\nset_default_config");
  
  //ch1锯齿波（直角三角波）
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_WAVE_CH1,CONST_APP_SG_DEFAULT_WAVE_CH1); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //ch2矩形波（10%占空比）
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_WAVE_CH2,CONST_APP_SG_DEFAULT_WAVE_CH2); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_DUTY_CH2,CONST_APP_SG_DEFAULT_DUTY_CH2); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //ch1输出频率20khz
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_FREQ_CH1,CONST_APP_SG_DEFAULT_FREQ,CONST_FREQ_FMT_UNIT_HZ); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //ch2输出频率20khz
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_FREQ_CH2,CONST_APP_SG_DEFAULT_FREQ,CONST_FREQ_FMT_UNIT_HZ); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //双路频率同步
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_TRACE_CH2,1,0,0,0,0); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //相位差36度
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_PHASE_CH1,CONST_APP_SG_DEFAULT_PHASE_CH1); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_PHASE_CH2,CONST_APP_SG_DEFAULT_PHASE_CH2); 
  //DBGPRINTF("\n%s",au8sendBuff);
  //send_CMD_to_device((char *)au8sendBuff);
  //delay(5);
  
  //ch1满幅0---20v
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_AMP_CH1,CONST_APP_SG_DEFAULT_AMP_CH1); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //ch2 0---3.3v
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_AMP_CH2,CONST_APP_SG_DEFAULT_AMP_CH2); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //无直流偏置
  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_BIAS_CH1,CONST_APP_SG_DEFAULT_BIAS_CH1); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);

  prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,CONST_APP_SG_FMT_BIAS_CH2,CONST_APP_SG_DEFAULT_BIAS_CH2); 
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);
  
  //clean data
  while(APP_SERIAL.available())
  {
    APP_SERIAL.read();
  }
  return ret;
}

bool SG_JDS::is_device_exist()
{
  bool ret = false;
  //准备并发送查询状态数据
  strcpy((char *) au8sendBuff,":r00=1.\n\r");
  
  send_CMD_to_device((char *) au8sendBuff);

  //wait for 2 seconds to get data;
  unsigned long ulTick;
  //vReset_interval(ulTick);
  ulTick = ulReset_interval();
  DBGPRINTLN("wait for receive:");
  while (ulGet_interval(ulTick) < 2000) {
    //DBGPRINT("fc");
    if (APP_SERIAL.available()) {
      uint8_t chR;
      chR = APP_SERIAL.read();
      if (chR == '\r'){
        ret=true;
        break;
      }
    }
    delay(5);
  }
  while(APP_SERIAL.available())
  {
    APP_SERIAL.read();
  }
  DBGPRINTLN("\nis_device_exist done");
  return ret;
}


bool SG_JDS::devReady()
{
  return deviceReady;
}


bool SG_JDS::read_device_status()
{
  bool ret = true;
  //准备并发送查询状态数据
  sprintf((char *) au8sendBuff,":r%02d=1.\n\r",anDevStatusCheckList[devStatusCount]);
  devStatusCount++;
  if (anDevStatusCheckList[devStatusCount]<0){
    devStatusCount=0;
    ulSIOQueryTickThreshold=CONST_APP_SG_STAT_QUERY_INTERVAL;
    devStatusReady=true;
  }
  else{
    ulSIOQueryTickThreshold=CONST_APP_SG_STAT_QUERY_FAST;
    
  }
  DBGPRINTF("\nread_device_status:[%s]",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  //delay(5);
  
  return ret;
}

void ICACHE_FLASH_ATTR SG_JDS::restore_setting()
{
  
}

void ICACHE_FLASH_ATTR SG_JDS::handle_SIO_reply_data(char *param)
{
  int nLen;
  char *spTemp;
  spTemp = param;
  nLen = strlen(spTemp);
  DBGPRINTF("\n**-- in Handle Replay Data: % d  freeheap: % d--**", nLen, ESP.getFreeHeap());
  if (nLen < 1) {
    return;
  }
  
  if (!memcmp(spTemp, "OK", 2))
  {
    //ok
  }
  else{
    short cmd,channel;
    char *spP1,*spP2,*spP3,*spP4,*spP5;
    spTemp=strchr(param,'=');
    if (spTemp==NULL){
      return;
    }
    *spTemp=0;
    spP1=spTemp+1;
    spTemp=strchr(spP1,'.');
    if (spTemp==NULL){
      return;
    }
    *spTemp=0;
    cmd=atoi(param+1);
    switch (cmd)
    {
      case CONST_APP_SG_FMT_MODEL:
        devModel=atol(spP1);
        break;

      case CONST_APP_SG_FMT_SN:
        devSN=atol(spP1);
        break;

      case CONST_APP_SG_FMT_WAVE_CH1:
        preStat[0].waveMode=currStat[0].waveMode;
        currStat[0].waveMode=atoi(spP1);
        break;

      case CONST_APP_SG_FMT_WAVE_CH2:
        preStat[1].waveMode=currStat[1].waveMode;
        currStat[1].waveMode=atoi(spP1);
        break;

      case CONST_APP_SG_FMT_FREQ_CH1:
        spTemp=strchr(spP1,',');
        if (spTemp==NULL){
          break;
        }
        *spTemp=0;
        spP2=spTemp+1;
        preStat[0].freq=currStat[0].freq;
        currStat[0].freq=atoi(spP1);
        preStat[0].freqUnit=currStat[0].freqUnit;
        currStat[0].freqUnit=atoi(spP2);
        break;
        
      case CONST_APP_SG_FMT_FREQ_CH2:
        spTemp=strchr(spP1,',');
        if (spTemp==NULL){
          break;
        }
        *spTemp=0;
        spP2=spTemp+1;
        preStat[1].freq=currStat[1].freq;
        currStat[1].freq=atoi(spP1);
        preStat[1].freqUnit=currStat[1].freqUnit;
        currStat[1].freqUnit=atoi(spP2);
        break;
        
      case CONST_APP_SG_FMT_AMP_CH1:
        preStat[0].amp=currStat[0].amp;
        currStat[0].amp=atoi(spP1);
        break;
      
      case CONST_APP_SG_FMT_AMP_CH2:
        preStat[1].amp=currStat[1].amp;
        currStat[1].amp=atoi(spP1);
        break;
      
      case CONST_APP_SG_FMT_BIAS_CH1:
        preStat[0].bias=currStat[0].bias;
        currStat[0].bias=atoi(spP1);
        break;
      
      case CONST_APP_SG_FMT_BIAS_CH2:
        preStat[1].bias=currStat[1].bias;
        currStat[1].bias=atoi(spP1);
        break;
      
      case CONST_APP_SG_FMT_DUTY_CH1:
        preStat[0].duty=currStat[0].duty;
        currStat[0].duty=atoi(spP1);
        break;
      
      case CONST_APP_SG_FMT_DUTY_CH2:
        preStat[1].duty=currStat[1].duty;
        currStat[1].duty=atoi(spP1);
        break;
      
      case CONST_APP_SG_FMT_PHASE_CH1:
        preStat[0].phase=currStat[0].phase;
        currStat[0].phase=atoi(spP1);
        break;
      
      //case CONST_APP_SG_FMT_PHASE_CH2:
        //currStat[1].phase=atoi(spP1);
        //break;
        
      case CONST_APP_SG_FMT_TRACE_CH2:
        break;

      case CONST_APP_SG_FMT_EXTEND_CH2:
        break;
      
      case CONST_APP_SG_FMT_SWITCH:
        break;        
              
      case CONST_APP_SG_FMT_COUPLING:
        break;        
              
      case CONST_APP_SG_FMT_GATE:
        preStat[0].gate=currStat[0].gate;
        currStat[0].gate=atoi(spP1);
        preStat[1].gate=currStat[0].gate;
        currStat[1].gate=currStat[0].gate;
        break;        
              
      case CONST_APP_SG_FMT_MEASURE:
        break;        
              
      case CONST_APP_SG_FMT_SWEEP_BEGIN:
        break;        
              
      case CONST_APP_SG_FMT_SWEEP_END:
        break;        
              
      case CONST_APP_SG_FMT_SWEEP_TIME:
        break;        
              
      case CONST_APP_SG_FMT_SWEEP_DIRECTION:
        break;        
              
      case CONST_APP_SG_FMT_SWEEP_MODE:
        break;        
              
      case CONST_APP_SG_FMT_PULSE_WIDTH:
        break;        
              
      case CONST_APP_SG_FMT_PULSE_CYCLE:
        break;        
              
      case CONST_APP_SG_FMT_PULSE_BIAS:
        break;        
              
      case CONST_APP_SG_FMT_PULSE_AMP:
        break;        
              
      case CONST_APP_SG_FMT_BURST:
        break;        
                            
      default:
        break;
    }
    if(memcmp(currStat,preStat,sizeof(currStat)))
    {
      bStatChangeFlag=true;
      debug_print_info();
    }
    
  }
 
}

void ICACHE_FLASH_ATTR SG_JDS::check_device_reply()
{
  if (APP_SERIAL.available()) {
    uint8_t chR;
    chR = APP_SERIAL.read();
    //DBGPRINT(chR);
    au8recvBuff[u8recvLen++] = chR;
    if ((chR==CONST_APP_SG_RECV_SIO_DATA_BEGIN)
    ||(u8recvLen > CONST_APP_SG_RECV_SENDREV_BUFFLEN)) 
    {
      u8recvLen = 0;
    }
    if (chR == CONST_APP_SG_RECV_SIO_DATA_END) {
      //got respose/reply data;
      au8recvBuff[u8recvLen++] = '\0';
      bSendBusyFlag = false;
      DBGPRINTLN("**--Receive SIO data--**");
      DBGPRINTF("[%s]", au8recvBuff);
      handle_SIO_reply_data((char *) au8recvBuff);
      u8recvLen = 0;
    }
  }
  delay(1);
}

// 762 "CH:1;AMP:+100;" "CH:1,2;FREQ:+50;" "param":"CH:2;DUTY:-5;"
bool SG_JDS::decode_762_command(char *ptr)
{
  bool ret = false;
  short nT1;
  nT1 = strlen(ptr);
  if (nT1 > 0)
  {
    char *spChan,*spType,*spVal,*spStr;
    spChan=ptr;
    spStr=strchr(ptr,';');
    if (spStr!=NULL){
      *spStr=0;
      spType=spStr+1;
      ptr=spType;
      //split type & value
      spStr=strchr(ptr,';');
      if (spStr!=NULL)
      {
        *spStr=0;
        spStr=strchr(spChan,':');
        if (spStr!=NULL){
          spChan=spStr+1;
          spStr=strchr(spType,':');
          if (spStr!=NULL){
            *spStr=0;
            spVal=spStr+1;
            ret=true;
          }
        }
      }
      else{
        if(!memcmp(spChan,"SA",2)){
          //save command
          channelNum=1;
          strcpy(cmdType,"SAVE");
          cmdValue=5;
          DBGPRINTF("\n chan:[%d],type:[%s],val:[%d]",channelNum,cmdType,cmdValue);
          trans_server_CMD(channelNum,cmdType,cmdValue);
        }
          

      }
    }
    if (ret==true){
      short i;
      DBGPRINTF("\n chan:[%s],type:[%s],val:[%s]",spChan,spType,spVal);
      strncpy(cmdType,spType,CONST_APP_SG_SERVER_CMD_LENGTH);
      cmdValue=atol(spVal);

      spStr=strchr(spChan,',');
      if (spStr!=NULL){
        // two channel
        channelNum=1;
        DBGPRINTF("\n chan:[%d],type:[%s],val:[%d]",channelNum,cmdType,cmdValue);
        trans_server_CMD(channelNum,cmdType,cmdValue);
        channelNum=2;
        DBGPRINTF("\n chan:[%d],type:[%s],val:[%d]",channelNum,cmdType,cmdValue);
        trans_server_CMD(channelNum,cmdType,cmdValue);
      }
      else{
        // one channel
        channelNum=atoi(spChan);
        DBGPRINTF("\n chan:[%d],type:[%s],val:[%d]",channelNum,cmdType,cmdValue);
        trans_server_CMD(channelNum,cmdType,cmdValue);
      }
            
    }
  }
  return ret;
}



void ICACHE_FLASH_ATTR SG_JDS::trans_server_CMD(short channel, char *type, long val)
{
  short cmd,actionID;
  long p1,p2,p3,p4,p5;
  DBGPRINTF("\ntrans_server_CMD:chan:[%d],type:[%s],val:[%d]",channel,type,val);
  if (!memcmp(type,"AMP",3)){
    //ch1满幅0---20v
    switch(channel)
    {
      case 1:
        cmd=CONST_APP_SG_FMT_AMP_CH1;
        break;
      default:
        cmd=CONST_APP_SG_FMT_AMP_CH2;
        break;
    }
    p1=currStat[channel-1].amp+val;
    DBGPRINTF("AMP:[%d] P1[%d]",currStat[channel-1].amp,p1);
    if (p1>CONST_APP_SG_CONFIG_AMP_MAX){
      p1=CONST_APP_SG_CONFIG_AMP_MAX;
    }
    if (p1<CONST_APP_SG_CONFIG_AMP_MIN){
      p1=CONST_APP_SG_CONFIG_AMP_MIN;
    }
    currStat[channel-1].amp=p1;
    //write
    prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,cmd,p1); 
    DBGPRINTF("\n%s",au8sendBuff);
    gsDeviceCMDList.push(au8sendBuff);
    //check status
    prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_READ,cmd,1); 
    DBGPRINTF("\n%s",au8sendBuff);
    gsDeviceCMDList.push(au8sendBuff);
  }
  else if (!memcmp(type,"FREQ",4)){
    //频率
    switch(channel)
    {
      case 1:
        cmd=CONST_APP_SG_FMT_FREQ_CH1;
        break;
      default:
        cmd=CONST_APP_SG_FMT_FREQ_CH2;
        break;
    }
    p1=currStat[channel-1].freq+val;
    DBGPRINTF("FREQ:[%d] P1[%d]",currStat[channel-1].freq,p1);
    if (p1>CONST_APP_SG_CONFIG_FREQ_MAX){
      p1=CONST_APP_SG_CONFIG_FREQ_MAX;
    }
    if (p1<CONST_APP_SG_CONFIG_FREQ_MIN){
      p1=CONST_APP_SG_CONFIG_FREQ_MIN;
    }
    currStat[channel-1].freq=p1;
    //write
    prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,cmd,p1,0); 
    DBGPRINTF("\n%s",au8sendBuff);
    gsDeviceCMDList.push(au8sendBuff);    
    //check status
    prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_READ,cmd,1); 
    DBGPRINTF("\n%s",au8sendBuff);
    gsDeviceCMDList.push(au8sendBuff);
  }
  else if (!memcmp(type,"DUTY",4)){
    //ch2矩形波（10%占空比）
    switch(channel)
    {
      case 1:
        cmd=CONST_APP_SG_FMT_DUTY_CH1;
        break;
      default:
        cmd=CONST_APP_SG_FMT_DUTY_CH2;
        break;
    }
    p1=currStat[channel-1].duty+val;
    DBGPRINTF("DUTY:[%d] P1[%d]",currStat[channel-1].duty,p1);
    if (p1>CONST_APP_SG_CONFIG_DUTY_MAX){
      p1=CONST_APP_SG_CONFIG_DUTY_MAX;
    }
    if (p1<CONST_APP_SG_CONFIG_DUTY_MIN){
      p1=CONST_APP_SG_CONFIG_DUTY_MIN;
    }
    currStat[channel-1].duty=p1;
    //write
    prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_WRITE,cmd,p1); 
    DBGPRINTF("\n%s",au8sendBuff);
    gsDeviceCMDList.push(au8sendBuff);    
    //check status
    prepare_cmd_param((char *)au8sendBuff,CONST_APP_SG_CMD_TYPE_STAND_READ,cmd,1); 
    DBGPRINTF("\n%s",au8sendBuff);
    gsDeviceCMDList.push(au8sendBuff);

  }
  else if (!memcmp(type,"SAVE",4)){
    bSave_config();
  }
  DBGPRINTF("\ngsDeviceCMDList len:[%d] \n",gsDeviceCMDList.len());
}

void ICACHE_FLASH_ATTR SG_JDS::send_CMD_to_device(char *param)
{
  short i;
  DBGPRINTF("\nsend_CMD_to_device [%d][%s][",strlen(param),param);
  /*
  for (i = 0; i < strlen(param); i++) {
    Serial.write(param[i]);
  }
  DBGPRINT("]\n");
  */
  //reset timeout tick
  ulRecvTimeOutTick = ulReset_interval();

  DBGPRINT("\n[");
  for (i = 0; i < strlen(param); i++) {
    APP_SERIAL.write(param[i]);
  }
  DBGPRINT("]\n");
}

void ICACHE_FLASH_ATTR SG_JDS::send_CMD_to_device(short nLen, uint8_t *param)
{
  short i;
  DBGPRINTF("\n==<send_CMD_to_device>==\n len[%d][%s] \n[",nLen,param);
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(param[i]);
  }
  DBGPRINT("]");
  //表明有数据发送了,需要在收到回馈数据以后清除.
  bSendBusyFlag=true;
}


void ICACHE_FLASH_ATTR SG_JDS::loop()
{
  check_device_reply();
  
  if ((ulGet_interval(ulRecvTimeOutTick) > CONST_APP_SG_RECV_TIMEOUT_LEN)) {
    if (bSendBusyFlag) {
      bSendBusyFlag = false;
    }
  }
  
  if ((ulGet_interval(ulSIOSendTick) > CONST_APP_SG_SEND_SIO_CMD_INTERVAL)) {
    if ((gsDeviceCMDList.len()) 
      && (!bSendBusyFlag)) {
      gsDeviceCMDList.pop(au8sendBuff);
      DBGPRINTF("\n gsDeviceCMDList.pop:[%s]",au8sendBuff);
      send_CMD_to_device((char *)au8sendBuff);
      //receive buff clean
      u8recvLen = 0;
    }
    ulSIOSendTick=ulReset_interval();
  }
  
  //query current device Status
  if ((ulGet_interval(ulSIOQueryTick) > ulSIOQueryTickThreshold))
  {
    read_device_status();
    ulSIOQueryTick = ulReset_interval();
  }

  
  delay(1);
}

/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR SG_JDS::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  uint8_t bigBuff[CONST_APP_SG_RECV_BIG_BUFFLEN + 2];

  strcpy((char *) au8sendBuff,":w71=5.\n\r");
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);

  File configFile = SPIFFS.open(CONST_APP_SG_RECV_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s",CONST_APP_SG_RECV_FILE_NAME);
    return bRet;
  }
  
  bRet=true;

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuff, CONST_APP_SG_RECV_BIG_BUFFLEN);
    if (nLen <= 0)
      break;
    bigBuff[nLen - 1] = '\0'; //trim
    
    prepare_cmd_param((char *) au8sendBuff,(char *)bigBuff);
    DBGPRINTF("\n[config cmd] %s",au8sendBuff);
    send_CMD_to_device((char *)au8sendBuff);
    delay(5);
  }
    
  configFile.close();
  DBGPRINTLN("Application Config ok");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR SG_JDS::bSave_config()
{  
  uint8_t bigBuff[CONST_APP_SG_RECV_BIG_BUFFLEN + 2];
  
  strcpy((char *) au8sendBuff,":w70=5.\n\r");
  DBGPRINTF("\n%s",au8sendBuff);
  send_CMD_to_device((char *)au8sendBuff);
  delay(5);

  
  DBGPRINTLN("--save SG config data--");
  File configFile = SPIFFS.open(CONST_APP_SG_RECV_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_SG_RECV_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTF("Failed to open config file :%s",CONST_APP_SG_RECV_FILE_NAME);
      return false;
    }
  }
  
  //configFile.print("w71=5.");

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}


void SG_JDS::debug_print_info()
{
  short i;
  DBGPRINT("\n== debug_print_info ==");
  for (i=0;i<CONST_APP_SG_CHANNEL_COUNT;i++){
    DBGPRINTF("\nCH%d: freq:[%d],phase:[%d],amp:[%d],bias:[%d],duty:[%d],waveMode:[%d]",i,currStat[i].freq,currStat[i].phase,currStat[i].amp,currStat[i].bias,currStat[i].duty,currStat[i].waveMode);
  }
}
