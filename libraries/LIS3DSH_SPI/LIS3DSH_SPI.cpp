#include "LIS3DSH_SPI.h"

//modified on 2018/06/15 by yang dongxiao yangdongxiao666@gmail.com

#define CONST_APP_LIS3DSH_SPI_QUERY_LIST_LEN 2

static ListArray recvList(RECEIVE_BUFFER_COUNT,sizeof(stDeviceDataInfo)+2);
//static ListArray sendList(SEND_BUFFER_COUNT,CONST_APP_LIS3DSH_SPI_BASE64_BUFF_LEN);


LIS3DSH_SPI::LIS3DSH_SPI()
{
}

bool LIS3DSH_SPI::begin(char *ptr)   //参数
{
  begin(ptr,LIS3DSH_DATARATE_100);
}

bool LIS3DSH_SPI::begin(char *ptr,uint16_t rate)
{
  bool ret = false;
  
  bigBuff = ptr;
  
  bStatChangeFlag = false;
  u8ParamChangeFlag = 0;
  

  DBGPRINTLN("\n\n");
  DBGPRINTLN("--LIS3DSH INIT BEGIN--");
 
  pinMode (CS_Pin, OUTPUT);  //Chip Select pin to control SPI
  pinMode (sclk_Pin, OUTPUT);  //Chip Select pin to control SPI
  pinMode (mosi_Pin, OUTPUT);  //Chip Select pin to control SPI
  pinMode (miso_Pin, INPUT);  //Chip Select pin to control SPI
  
  digitalWrite(CS_Pin, HIGH);//Disable SPI
  digitalWrite(CS_Pin, LOW);//Enable SPI
  digitalWrite(CS_Pin, HIGH);//Disable SPI
  
  ctrl = 0x00;
  Output_DataRate = rate;
  Axes_Enable = LIS3DSH_XYZ_ENABLE;
  SPI_Wire = LIS3DSH_SERIALINTERFACE_4WIRE;
  Self_Test = LIS3DSH_SELFTEST_NORMAL;
  Full_Scale = LIS3DSH_FULLSCALE_2;
  Filter_BW = LIS3DSH_FILTER_BW_200;
  
  ctrl = Output_DataRate | Axes_Enable;
  ctrl |= ((SPI_Wire | Self_Test | Full_Scale  | Filter_BW) << 8);
  
  ACCELERO_IO_Init();
  
  //print(ctrl)
  uint8_t ctrl_low = ctrl & 0xFF;
  ACCELERO_IO_WriteByte(ctrl_low, LIS3DSH_CTRL_REG4_ADDR);

  uint8_t ctrl_high = ctrl >> 8;
  ACCELERO_IO_WriteByte(ctrl_high, LIS3DSH_CTRL_REG5_ADDR);
  
  DBGPRINT("\n start read sensor data...");
  _ID = ReadID();
  DBGPRINTF("\n device ID:%d",_ID);
  
/*
  thresholdVal.AX = CONST_APP_LIS3DSH_SPI_THRESHOLD_AX;
  thresholdVal.AY = CONST_APP_LIS3DSH_SPI_THRESHOLD_AY;
  thresholdVal.AZ = CONST_APP_LIS3DSH_SPI_THRESHOLD_AZ;
*/  

  ulDataCheckTickInterval = CONST_APP_LIS3DSH_SPI_DEFAULT_DATA_CHECK_TICKS;
  ulTimeStampTickInterval = ulReset_interval();
  
  //if (!bLoad_config()) {
  //  bSave_config();
  //}
  
  
  return ret;

}

void  LIS3DSH_SPI::loop()
{
  available();
  //dataCount = sendList.len();
  delay(1);
}

bool  LIS3DSH_SPI::setFrameNum(uint32_t id)
{
  selfID = id;
  return true;
}


void  LIS3DSH_SPI::setTimeStamp(char *YMDHMS)
{
  strcpy(timeStamp,YMDHMS);
}

short LIS3DSH_SPI::getData(char *ptr)
{
  //sendList.pop(ptr);
  memcpy(ptr,bigBuff,sizeof(bigBuff));
  dataCount = 0 ;
  //strcpy(timeStamp,YMDHMS);
  //return sendList.len();
  return 0;
}

short LIS3DSH_SPI::available()
{
  ACCELERO_IO_ReadData();
  dataCount = recvList.len();
  //if (recvList.len() >= SEND_DATA_BATCH_COUNT)
  //{
    //package_data();
  //}
  //
  //return sendList.len();
  return dataCount;
}

uint16_t LIS3DSH_SPI::package_data()
{
  short i;
  short nLen;
  char *ptr;
  //char b64Buff[CONST_APP_LIS3DSH_SPI_BIG_BUFF_LEN];
  //b64Buff[0]=0;
  bigBuff[0]=0;
  ptr = bigBuff;
  *ptr = '[';
  ptr++;
  for (i=0; i<SEND_DATA_BATCH_COUNT;i++ )
  {
    //ptr = b64Buff;
    *ptr = '\"';
    ptr++;
    //strcat(ptr, "\"");
    recvList.pop(&buffStat);
    nLen = Base64encode(ptr,(char *) &buffStat,sizeof(buffStat));
    DBGPRINTF("\n nLen[%d]",nLen);
    ptr += nLen-1;
    *ptr = '\"';
    ptr++;
    DBGPRINTF("base64[%s]",ptr);
    if (i == SEND_DATA_BATCH_COUNT - 1) {
      //strcat(ptr, "\"");
    }
    else {
      //strcat(ptr, "\",");
      *ptr = ',';
      ptr++;      
    }
    //strcat(bigBuff,ptr);
  }
  *ptr =']';
  ptr++;
  *ptr = 0;
  //sendList.push(bigBuff);
  dataCount = 1;
  //debug_print_info();
  DBGPRINTF("\n after b64 [%d],[%d] [%s]",i,strlen(bigBuff),bigBuff);
  ulTimeStampTickInterval = ulReset_interval();
}

uint16_t LIS3DSH_SPI::calc_CRC(uint8_t *in, short nLen)
{
  uint16_t crc = 0xFFFF;
  short i,j;
  DBGPRINTF("\n== calc_CRC [%d] ==",nLen);
  //for (i = 0; i < nLen - CONST_APP_WIT_CHKSUM_TAIL_LEN; i++) {
  for (i = 0; i < nLen; i++) {
    DBGPRINTF("[%02X %04X]",in[i],crc);
	  crc ^= (uint16_t) in[i];
	  for (j = 0; j<8;++j){
      if (crc & 1)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc = (crc >> 1);
	  }
  }

  DBGPRINTF("\n --- crc ---[%04X]",crc);
  return crc;

}


uint16_t LIS3DSH_SPI::package_data_0A0D()
{
  short ret = 0;
  short i;
  short nLen;
  char *ptr;
  char *prePtr;
  //char b64Buff[CONST_APP_LIS3DSH_SPI_BIG_BUFF_LEN];
  //b64Buff[0]=0;
  /*
  //帧头
  sendbuffer[0] = 0x0A;
  sendbuffer[1] = 0x0D;
  
  //帧长度
  //sendbuffer[2] = sizeof(reportBuffer.frameData) >> 8;
  //sendbuffer[3] = sizeof(reportBuffer.frameData) & 0xFF;
  sendbuffer[2] = 137 >> 8;
  sendbuffer[3] = 137 & 0xFF;
  
  //帧类型
  sendbuffer[4] = 0xD5;
  
  //设备号
  sendbuffer[5] = 0x00;
  sendbuffer[6] = 0x00;
  sendbuffer[7] = 0x00;
  sendbuffer[8] = 0x00;
  sendbuffer[9] = 0x00;
  sendbuffer[10] = 0x00;
  sendbuffer[11] = 0xA3;
  sendbuffer[12] = 0x00;
  
  //Data
  
  for (i=0; i<SEND_DATA_BATCH_COUNT;i++ )
  {
    prePtr = ptr;
    //ptr = (char *) &reportBuffer.frameData[i];
    ptr = (char *) &sendbuffer[i+13];
    recvList.pop(ptr);
    //reportBuffer.frameData[i].relMicroSeconds = i;
    //DBGPRINTF("\nptr-prePtr [%04X]",ptr-prePtr);
  }
  */
  
  
  reportBuffer.frameBegin[0] = 0x0A;
  reportBuffer.frameBegin[1] = 0x0D;
  
  reportBuffer.frameType = 0xD5;
  
  reportBuffer.frameNum[6] = 0xA3;
  reportBuffer.frameNum[7] = 0x00;
  
  //reportBuff.frameNum[0] = selfID & 0xFF;
  //reportBuff.frameNum[1] = (selfID>>8) & 0xFF;
  //reportBuff.frameNum[2] = (selfID>>16) & 0xFF;
  //reportBuff.frameNum[3] = (selfID>>24) & 0xFF;
  
  //reportBuffer.frameLen[0] = sizeof(reportBuffer.frameData)>>8;
  for (i=0; i<SEND_DATA_BATCH_COUNT;i++ )
  {
    prePtr = ptr;
    //ptr = (char *) &reportBuffer.frameData[i];
    ptr = (char *) &reportBuffer.frameData[i];
    recvList.pop(ptr);
    //reportBuffer.frameData[i].relMicroSeconds = i;
    //DBGPRINTF("\nptr-prePtr [%04X]",ptr-prePtr);
  }  
  
  
  reportBuffer.frameLen[0] = sizeof(reportBuffer.frameData) >> 8;
  reportBuffer.frameLen[1] = sizeof(reportBuffer.frameData) & 0xFF;

  reportBuffer.frameCRC = calc_CRC(&reportBuffer.frameLen[0],sizeof(reportBuffer)-6);
 
  reportBuffer.frameEnd[0] = 0xD;
  reportBuffer.frameEnd[1] = 0xA;
  
  
  //CRC
  //sendbuffer[12+SEND_DATA_BATCH_COUNT] = calc_CRC(&sendbuffer[2],sizeof(sendbuffer)-6) >> 8;
  //sendbuffer[13+SEND_DATA_BATCH_COUNT] = calc_CRC(&sendbuffer[2],sizeof(sendbuffer)-6) & 0xFF;
  //sendbuffer[13+SEND_DATA_BATCH_COUNT] = reportBuffer.frameCRC >> 8;
  //sendbuffer[14+SEND_DATA_BATCH_COUNT] = reportBuffer.frameCRC & 0xFF;
  
  //帧尾
  //sendbuffer[15+SEND_DATA_BATCH_COUNT] = 0x0D;
  //sendbuffer[16+SEND_DATA_BATCH_COUNT] = 0x0A;
  
  //sendbuffer[5] = 
  
  
  ulTimeStampTickInterval = ulReset_interval();
  
}



uint16_t LIS3DSH_SPI::ACCELERO_IO_ReadData()
{
  while(1)
  {
    if (GetDataState())
      break;
    delay(0);
  }
  ReadACC();
}

uint16_t LIS3DSH_SPI::ReadACC()
{
  uint16_t crtl;
  uint16_t buffL,buffH,val;
  
  crtl = ACCELERO_IO_ReadByte(LIS3DSH_CTRL_REG5_ADDR);
  
  //preStat.AX_H = currStat.AX_H;
  //preStat.AX_L = currStat.AX_L; 
  
  currStat.AX_H = ACCELERO_IO_ReadByte(LIS3DSH_OUT_X_H_ADDR);
  currStat.AX_L = ACCELERO_IO_ReadByte(LIS3DSH_OUT_X_L_ADDR);
  //val = (buffH << 8) | buffL;

  //preStat.AY_H = currStat.AY_H;
  //preStat.AY_L = currStat.AY_L; 
  
  currStat.AY_H = ACCELERO_IO_ReadByte(LIS3DSH_OUT_Y_H_ADDR);
  currStat.AY_L = ACCELERO_IO_ReadByte(LIS3DSH_OUT_Y_L_ADDR);
  //val = (buffH << 8) | buffL;
  
  //preStat.AZ_H = currStat.AZ_H;
  //preStat.AZ_L = currStat.AZ_L;

  currStat.AZ_H = ACCELERO_IO_ReadByte(LIS3DSH_OUT_Z_H_ADDR); 
  currStat.AZ_L = ACCELERO_IO_ReadByte(LIS3DSH_OUT_Z_L_ADDR); 
  //val = (buffH << 8) | buffL;
  
  
  //preStat.relMicroSeconds = currStat.relMicroSeconds;
  //currStat.relMicroSeconds = ulGet_interval(ulTimeStampTickInterval);
  recvList.push(&currStat);
  debug_print_info();
}

void LIS3DSH_SPI::ACCELERO_IO_Init()
{
  spi_disable();
  spi_init();
}

void LIS3DSH_SPI::spi_init()
{
  digitalWrite(sclk_Pin, HIGH);
  digitalWrite(CS_Pin, HIGH);
  digitalWrite(mosi_Pin, HIGH);
}

void LIS3DSH_SPI::spi_disable()
{
  digitalWrite(CS_Pin, HIGH);//Disable SPI
}

void LIS3DSH_SPI::spi_enable()
{
  digitalWrite(CS_Pin, LOW);//enable SPI
}

uint16_t LIS3DSH_SPI::getHigh(uint16_t data)
{
  return data >> 8;
}

uint16_t LIS3DSH_SPI::getLow(uint16_t data) {
  return data & 0xFF;
}

uint16_t LIS3DSH_SPI::ACCELERO_IO_WriteByte(uint16_t val1, uint16_t WriteAddr) 
{
  uint16_t tmp ;
  spi_enable();
  SPIx_WriteRead(WriteAddr);
  tmp = SPIx_WriteRead(val1 & 0xFF);
  spi_disable();
  return tmp;
}

uint16_t LIS3DSH_SPI::SPIx_WriteRead(uint16_t rwAddr)
{
  uint16_t val1 = 0;
  uint16_t tmp;
  for (int i = 7; i > -1; i--)
  {
    digitalWrite(sclk_Pin, LOW);
    digitalWrite(mosi_Pin, getBit(rwAddr, i));
    digitalWrite(sclk_Pin, HIGH);
    tmp = digitalRead(miso_Pin);
    //Serial.printf("\n tmp:[%d] i[%d],",tmp,i);
    val1 = val1 + (tmp << i);
  }
  return val1;
}

uint16_t LIS3DSH_SPI::GetDataState()
{
  uint16_t tmp = ACCELERO_IO_ReadByte(0x18);
  return getBit(tmp, 0);
}


uint16_t LIS3DSH_SPI::ACCELERO_IO_ReadByte(uint16_t ReadAddr) 
{
  uint16_t READWRITE_CMD = 0x80;
  uint16_t DUMMY_BYTE = 0x0;

  ReadAddr = ReadAddr | READWRITE_CMD;
  spi_enable();
  SPIx_WriteRead(ReadAddr);
  uint16_t tmp = SPIx_WriteRead(DUMMY_BYTE);
  spi_disable();
  return tmp;
}

uint16_t LIS3DSH_SPI::getBit(uint16_t data, uint16_t bitnum)
{
  return (data & (1 << bitnum)) >> bitnum;
}

bool LIS3DSH_SPI::is_device_exist()
{
  bool ret = false;
  ret = true;
  return ret;
}


bool LIS3DSH_SPI::devReady()
{
  return deviceReady;
}


bool LIS3DSH_SPI::is_state_change(uint32_t currVal,uint32_t preVal,int16_t threshold)
{
  int32_t diff;
  diff = currVal - preVal;
  if (diff < 0){
    diff = -diff;
  }
  if (diff >= threshold )  {
    bStatChangeFlag = true;
    DBGPRINTF("\n Stat changed: currVal [%d],preVal [%d],diff [%d],threshold [%d]",currVal,preVal,diff,threshold);
  }
  else{
    bStatChangeFlag = false;        
  }
  return bStatChangeFlag;
}


uint16_t LIS3DSH_SPI::ReadID()
{
  uint16_t tmp = 0;
  ACCELERO_IO_Init();
  uint16_t LIS3DSH_WHO_AM_I_ADDR = 0x0F;
  tmp = ACCELERO_IO_ReadByte(LIS3DSH_WHO_AM_I_ADDR);
  return tmp;
}


short ICACHE_FLASH_ATTR LIS3DSH_SPI::decode_811_command(char *comPtr)
{
  short ret = 0;
  char *spCurr,*spEnd;
  char *spCMD,*spVal;
  short nLen;
  unsigned short nT1;
  nLen = strlen(comPtr);
  if (nLen >= 4){
    spCurr = comPtr;
    while (true){
      spEnd = strchr(spCurr,';');
      if (spEnd == NULL)
        break;
      *spEnd = 0;
      spVal = strchr(spCurr,'=');
      if (spVal == NULL){
        break;
      }
      *spVal = 0;
      spVal++;
      spCMD = spCurr;
      nT1 = atol(spVal);
/*
      if(!strcmp(spCMD,"AX")){
        thresholdVal.AX = nT1;
        ret++;
      }
      else if (!strcmp(spCMD,"AY")){
        thresholdVal.AY = nT1;
        ret++;
      }
      else if (!strcmp(spCMD,"AZ")){
        thresholdVal.AZ = nT1;
        ret++;
      }
      else if (!strcmp(spCMD,"INT")){
        ulDataCheckTickInterval = nT1;
        ret++;
      }
*/
      spCurr = spEnd+1;
      
      if (spCurr >= (comPtr+nLen))
      {
        //超出范围
        break;
      }
      
    }
  }
  if (ret>0){
    bSave_config();
  }
  return ret;
}


/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR LIS3DSH_SPI::bLoad_config()
{
  /*
  bool bRet = false;
  int nLen;
  char *spTemp;
  char *bigBuffPtr=bigBuff;

  File configFile = SPIFFS.open(CONST_APP_LIS3DSH_SPI_RECV_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s", CONST_APP_LIS3DSH_SPI_RECV_FILE_NAME);
    return bRet;
  }

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuffPtr, CONST_APP_LIS3DSH_SPI_BIG_BUFF_LEN);
    if (nLen <= 0)
      break;
    bigBuffPtr[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bigBuffPtr, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bigBuffPtr, "TAX", 3) == 0) {
      thresholdVal.AX =  atol(spTemp);
      if (thresholdVal.AX < 1){
        thresholdVal.AX = CONST_APP_LIS3DSH_SPI_THRESHOLD_AX;
      }
    }
    if (memcmp(bigBuffPtr, "TAY", 3) == 0) {
      thresholdVal.AY =  atol(spTemp);
      if (thresholdVal.AY < 1){
        thresholdVal.AY = CONST_APP_LIS3DSH_SPI_THRESHOLD_AY;
      }
    }
    if (memcmp(bigBuffPtr, "TAZ", 3) == 0) {
      thresholdVal.AZ =  atol(spTemp);
      if (thresholdVal.AZ < 1){
        thresholdVal.AZ = CONST_APP_LIS3DSH_SPI_THRESHOLD_AZ;
      }
    }
    if (memcmp(bigBuffPtr, "INT", 3) == 0) {
      ulDataCheckTickInterval =  atol(spTemp);
      if (thresholdVal.AZ < 50){
        ulDataCheckTickInterval = CONST_APP_LIS3DSH_SPI_DEFAULT_DATA_CHECK_TICKS;
      }
    }
  }

  // Real world application would store these values in some variables for later use
  DBGPRINTF("Loaded data: AX[%d],AY[%d],AZ[%d]",thresholdVal.AX,thresholdVal.AY,thresholdVal.AZ);
  bRet = true;

  configFile.close();
  DBGPRINTLN("Application Config ok");
  return bRet;
  */
  return true;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR LIS3DSH_SPI::bSave_config()
{
/*
  char *bigBuffPtr=bigBuff;

  DBGPRINTLN("--save DTU config data--");
  File configFile = SPIFFS.open(CONST_APP_LIS3DSH_SPI_RECV_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_APP_LIS3DSH_SPI_RECV_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTF("Failed to open config file :%s", CONST_APP_LIS3DSH_SPI_RECV_FILE_NAME);
      return false;
    }
  }

  configFile.print("AAX:");
  configFile.println(thresholdVal.AX);

  configFile.print("AAY:");
  configFile.println(thresholdVal.AY);

  configFile.print("AAZ:");
  configFile.println(thresholdVal.AZ);

  configFile.print("INT:");
  configFile.println(ulDataCheckTickInterval);

  configFile.close();
  DBGPRINTLN(" -- end");
  */
  return true;
}


void LIS3DSH_SPI::debug_print_info()
{
  DBGPRINT("\n--- debug_print_info begin ----");
  
  DBGPRINTF("\n AX[%6d] [%6d]  AY[%6d] [%6d] AZ[%6d] [%6d]] ",currStat.AX_L, currStat.AX_H,currStat.AY_L, currStat.AY_H,currStat.AZ_L, currStat.AZ_H);
  
  DBGPRINT("\n--- debug_print_info end ----");

}
