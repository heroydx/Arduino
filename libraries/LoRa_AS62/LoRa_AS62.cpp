#include <LoRa_AS62.h>

//modified on 2017/4/2 by Steven Lian steven.lian@gmail.com


//SoftwareSerial SerialPort(10, 11); // RX D10, TX D11 Nano
SoftwareSerial SerialPort(CONST_LORA_AS62_RX_PORT, CONST_LORA_AS62_TX_PORT); // RX, TX 8266 GPIO 4,2
//SoftwareSerial LORASerial(19, 20); // RX, TX 8266 GPIO 4,5

uint8_t LoRa_AS62::get_SPEED(short SIORate, short AirRate)
{
  uint8_t ret = 0;
  //7,6=00 :8N1
  //TTL speed 5,4,3 = 011
  switch (SIORate)
  {
    case 19200:
      ret |= (0x4 << 3);
      break;
    case 38400:
      ret |= (0x5 << 3);
      break;
    case 9600:
    default:
      ret |= (0x3 << 3);
      break;
  }
  //Air speed 2,1,0 = 010
  switch (AirRate)
  {
    case 300:
      ret |= 0x0;
      break;
    case 1200:
      ret |= 0x1;
      break;
    case 4800:
      ret |= 0x3;
      break;
    case 9600:
      ret |= 0x4;
      break;
    case 2400:
    default:
      ret |= 0x2;
      break;
  }
  return ret;
}


// return channel value
uint8_t LoRa_AS62::get_CHAN(short channel)
{
  uint8_t ret;
  ret=0x17; //default 433MHz
  if ((channel>=CONST_LORA_AS62_BAND_433_LOW) &&(channel<=CONST_LORA_AS62_BAND_433_HIGH))
  {
    ret=channel-CONST_LORA_AS62_BAND_433_LOW;
  }
  return ret;
}


bool LoRa_AS62::is_valid_channel(short channel)
{
  bool ret=false;
  if ((channel>=CONST_LORA_AS62_BAND_433_LOW) &&(channel<=CONST_LORA_AS62_BAND_433_HIGH))
  {
    ret=true;
  }
  return ret;
}

unsigned short LoRa_AS62::get_version()
{
  return version;
}


void LoRa_AS62::prepare_configData()
{
  //config data
  configData[0] = 0xc0; //HEAD, C0：所设置的参数会掉电保存。C2：所设置的参数不会掉电保存。
  configData[1] = (uint8_t) (address>>8) & 0xFF; // 模块地址高字节
  configData[2] = (uint8_t) address & 0xFF; //模块地址低字节
  configData[3] = get_SPEED(sioBandRate, loraBandRate);
  configData[4] = sendCHAN;//通信频率（ 通信频率（ 410 M + CHAN * 1M）（默认 17H ：433M ）
  configData[5] = 0x44;//定点传输+上拉输入+250ms唤醒时间+FEC开+27dbm发射功率(7/6/543/2/10)
}

LoRa_AS62::LoRa_AS62()
{
  bReady=false;
  
  deviceType = CONST_LORA_DEVICE_TYPE_AS62;
  sendChannel = CONST_LORA_AS62_DEFAULT_CHANNEL;
  recvChannel = CONST_LORA_AS62_DEFAULT_CHANNEL;
  sendCHAN=get_CHAN(sendChannel);
  recvCHAN=get_CHAN(recvChannel);

  //clear data
  memset(sendBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE + 2);
  memset(recvBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE + 2);

  u8Busy = 0;

  address = 0x1234;
  
  //set communication rate
  loraBandRate = CONST_LORA_AS62_AIR_BAUD_RATE;
  sioBandRate = CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT;
  
  prepare_configData();
  
  SerialPort.begin(sioBandRate);

  //set config port working mode
  M0Port = CONST_LORA_AS62_M0_PORT;
  M1Port = CONST_LORA_AS62_M1_PORT;
  pinMode(M0Port, OUTPUT);
  pinMode(M1Port, OUTPUT);
  
}


LoRa_AS62::~LoRa_AS62()
{
  bReady=false;
}


bool LoRa_AS62::isReady()
{
  return bReady;
}


uint8_t LoRa_AS62::busy()
{
  if (ulGet_interval(gulBusyTick) > airSendTime) {
    u8Busy = 0;
    airSendTime = 0;
    //vReset_interval(gulBusyTick);
    gulBusyTick=ulReset_interval();
  }
  return u8Busy;
}


void LoRa_AS62::reset()
{
  short ret = 0;
  SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  set_comm_mode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  //
  //send query config

  DBGPRINTLN("send 0xc4 0xc4 0xc4");

  SerialPort.write(0xc4);
  SerialPort.write(0xc4);
  SerialPort.write(0xc4);
  delay(200);
  set_comm_mode(CONST_LORA_AS62_COMM_MODE_NORMAL);
}

short LoRa_AS62::read_config(uint8_t *u8Ptr)
{
  short ret = 0;
  SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  set_comm_mode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  delay(100);
  //
  //send query config

  DBGPRINTLN("send 0xc1 0xc1 0xc1");
  SerialPort.write(0xc1);
  SerialPort.write(0xc1);
  SerialPort.write(0xc1);
  delay(100);
  //wait for 1 seconds to get data;
  unsigned long ulTick;
  //vReset_interval(ulTick);
  ulTick=ulReset_interval();
  short nLen = 0;

  //DBGPRINTLN("wait for receive:");

  while (ulGet_interval(ulTick) < 1000) {
    if (SerialPort.available()) {
      uint8_t chR;
      chR = SerialPort.read();
      if (chR == 0xC0 || chR == 0xC2) {
        nLen = 0;
      }
      u8Ptr[nLen] = chR;
      nLen++;

      //debug
      //DBGPRINTF("%02X ", chR);

    }
    if (nLen == 6)
    {

      //debug
      DBGPRINTLN("get config data");
      
      break;
    }
    if (nLen > CONST_LORA_COMMUNICATION_BUFFSIZE) {
      nLen = 0;
    }
    delay(1);
  }
  //DBGPRINTLN("");
  DBGPRINTLN("-read_config-");

  //
  set_comm_mode(CONST_LORA_AS62_COMM_MODE_NORMAL);
  ret = nLen;
  return ret;
}

short LoRa_AS62::set_config()
{
  short ret = 0;
  SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  delay(100);
  set_comm_mode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  //
  short i;
  for (i = 0; i < CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN; i++) {
    SerialPort.write(configData[i]);
  }
  delay(100);
  short tryTimes = 5;
  while (tryTimes){
    if (read_config(recvBuf)>0)
      break;
    tryTimes--;
  }
  
  if (memcmp(recvBuf, configData, CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN)) {
    ret = -1;
  }
  else
  {
    DBGPRINTLN("--before save config---");
    bSave_config();
  }
  //
  set_comm_mode(CONST_LORA_AS62_COMM_MODE_NORMAL);
  return ret;
}

bool LoRa_AS62::init_device()
{
  int tryTimes = 5;
  version=CONST_LORA_AS62_VERSION;
 
  bReady=false;

  recvCHAN=get_CHAN(recvChannel);
  sendCHAN=get_CHAN(sendChannel);
  
  prepare_configData();

  while (tryTimes > 0) {
    if (set_config()==0) {
      bReady=true;
      break;
    }
    else
    {
      reset();
      tryTimes--;
    }
  }
  recvLen = 0;
  recvReady = 0;
  return bReady;
  
}

bool LoRa_AS62::begin()
{
  bLoad_config();
  return init_device();
}

bool LoRa_AS62::begin(short addr, short channel, short airRate)
{ 
  address=addr;
  sendChannel=channel;
  recvChannel=channel;
  loraBandRate=airRate;
  return init_device();
  
}


//based on slip protocol
short LoRa_AS62::available()
{
  short ret = 0;
#ifdef DEBUG_SIO
  char dispBuff[100];
#endif

  recvReady = 0;
  while (SerialPort.available()) {
    uint8_t chCurr, chPre;
    chCurr = SerialPort.read();

    //DBGPRINTF("%2X|", chCurr);

    //sprintf(dispBuff, "%c", chR);
    if (recvLen > 0) {
      chPre = recvBuf[recvLen - 1];
    }
    else{
      chPre=0;
    }
    if (chPre == CONST_SLIP_ESC) {
      //recvLen>0
      //chPre=0;

      switch (chCurr)
      {
        case CONST_SLIP_ESC_END:
          recvLen--;
          recvBuf[recvLen] = CONST_SLIP_END;
          recvLen++;
          break;
        case CONST_SLIP_ESC_ESC:
          recvLen--;
          recvBuf[recvLen] = CONST_SLIP_ESC;
          recvLen++;
          break;
        default:
          recvBuf[recvLen] = chCurr;
          recvLen++;
          break;
      }
/*
      DBGPRINTF("\n[%2X]-[%2X] %d",chPre, chCurr,recvLen);
      DBGPRINTLN("\n==============\n");
      for (short i=0;i<recvLen;i++){
         DBGPRINTF("%02X.",recvBuf[i]);
      }
      DBGPRINTLN();
*/
    }
    else {
      switch (chCurr)
      {
        case CONST_SLIP_END:
          if (recvLen == 0) {
            continue;
          }
          else {
            uint8_t CRCFlag = 0;
#ifdef DEBUG_SIO
            short i;
            //DBGPRINTLN();
            //DBGPRINTLN("after slip,before encode");
            for (i = 0; i < recvLen; i++) {
              chCurr = recvBuf[i];
              //DBGPRINTF("%2X ", chCurr);
            }
#endif
            //verify crc
            recvLen--;
            chCurr = crc8(recvBuf, recvLen);
            if (chCurr != recvBuf[recvLen]) {
              CRCFlag = -1; //crc error
            }
            //decode
            recvLen = simple_decode_data(simpleEncodeBuf, recvBuf, recvLen);
            memcpy(recvBuf, simpleEncodeBuf, recvLen);
            if (CRCFlag < 0) {
              recvReady = -recvLen;
            }
            else{
              recvReady=recvLen;            
            }
            ret = recvReady;
            //recvReady = recvLen;
            recvLen = 0;
          }
          break;
        default:
          recvBuf[recvLen] = chCurr;
          recvLen++;
          break;
      }
    }
    if (recvLen >= CONST_LORA_COMMUNICATION_BUFFSIZE) {
      recvLen = 0;
    }
  }
  delay(1);
  return ret;
}

//based on slip protocol
short LoRa_AS62::send(uint8_t *u8Ptr, short nLen)
{
  short ret;
  short batchCount, restCount;
  short i;
  short k;
  if (!u8Busy) {
    //可以发送,设置busy=1,重置发送时间定时器
    u8Busy = 1;
    //vReset_interval(gulBusyTick);
    gulBusyTick=ulReset_interval();

    //DBGPRINTLN("--send--");

    batchCount = (nLen + CONST_LORA_COMMUICATION_MAX_DATALEN - 1) / CONST_LORA_COMMUICATION_MAX_DATALEN;
    restCount = nLen % CONST_LORA_COMMUICATION_MAX_DATALEN;
    //DBGPRINTF("batch %d  rest %d",batchCount, restCount);
    for (k = 0; k < batchCount; k++) {
      short nLen;
      //memset(sendBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE);
      //DBGPRINTLN("send buff:");
      if (k < (batchCount - 1)) {
        //encode
        nLen=simple_encode_data(simpleEncodeBuf,(unsigned char *) (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN),CONST_LORA_COMMUICATION_MAX_DATALEN);
        //nLen = slip_encode( (unsigned char *) &sendBuf[0], (unsigned char *) (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), CONST_LORA_COMMUICATION_MAX_DATALEN);
      }
      else {
        nLen=simple_encode_data(simpleEncodeBuf,(unsigned char *) (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN),restCount);
        //nLen = slip_encode((unsigned char  *) &sendBuf[0], (unsigned char *)(u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), restCount);
      }
      nLen = slip_encode( (unsigned char *) &sendBuf[0], simpleEncodeBuf, nLen);
#ifdef DEBUG_SIO
/*
      char dispBuff[200];
      //DBGPRINTLN("send buff:");
      for (i = 0; i < nLen; i++) {
        sprintf(&dispBuff[i * 3], "%02X_", *(u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN+i));
      }
      //DBGPRINTLN(dispBuff);
      for (i = 0; i < nLen; i++) {
        sprintf(&dispBuff[i * 3], "%02X ", sendBuf[i]);
      }
      //DBGPRINTLN(dispBuff);
*/
#endif
      for (i = 0; i < nLen; i++) {
        SerialPort.write(sendBuf[i]);
      }
      //计算本次发送需要时间,单位:ms ;公式=发送字节*1000/(空口波特率/10)
      airSendTime = nLen * 1000 / (CONST_LORA_AS62_AIR_BAUD_RATE / 10) + 10;
      ret = 0;
    }
  }
  else {
    busy();
    //send fail
    ret = -1;
  }
}


short LoRa_AS62::get(uint8_t *u8Ptr)
{
  memcpy(u8Ptr, recvBuf, recvReady);
  return recvReady;
}

//wait for M0/M1 ready time, default =200ms;
#define CONST_LORA_AS62_WAIT_FOR_READY_TIME 200
void LoRa_AS62::set_comm_mode(uint8_t mode)
{
  switch (mode)
  {
    case CONST_LORA_AS62_COMM_MODE_WAKEUP:
      digitalWrite(M0Port, HIGH);
      digitalWrite(M1Port, LOW);
      break;
    case CONST_LORA_AS62_COMM_MODE_SAVING:
      digitalWrite(M0Port, LOW);
      digitalWrite(M1Port, HIGH);
      break;
    case CONST_LORA_AS62_COMM_MODE_SLEEP:
      digitalWrite(M0Port, HIGH);
      digitalWrite(M1Port, HIGH);
      break;
    case CONST_LORA_AS62_COMM_MODE_NORMAL:
    default:
      digitalWrite(M0Port, LOW);
      digitalWrite(M1Port, LOW);
      break;
  }
  delay(CONST_LORA_AS62_WAIT_FOR_READY_TIME);
}

#define CONFIG_BUFFER_MAXLEN CONST_LORA_COMMUNICATION_BUFFSIZE
/* load config, return True==Success*/
bool ICACHE_FLASH_ATTR LoRa_AS62::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  char *bufPtr;

  bufPtr = (char *) sendBuf;

  File configFile = SPIFFS.open(CONST_LORA_AS62_CONFIG_FILENAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s",CONST_LORA_AS62_CONFIG_FILENAME);
    return bRet;
  }
  //determine if the right config file
  nLen = configFile.readBytesUntil('\n', bufPtr, CONFIG_BUFFER_MAXLEN);
  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bufPtr, CONFIG_BUFFER_MAXLEN);
    if (nLen <= 0)
      break;
    bufPtr[nLen - 1] = '\0'; //trim
    spTemp = strchr(bufPtr, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bufPtr, "SC", 2) == 0) {
      sendChannel = atoi(spTemp);
    }
    else if (memcmp(bufPtr, "RC", 2) == 0) {
      recvChannel = atoi(spTemp);
    }
    else if (memcmp(bufPtr, "AD", 2) == 0) {
      address = atoi(spTemp);
    }
    else if (memcmp(bufPtr, "LR", 2) == 0) {
      loraBandRate = atoi(spTemp);
    }
    else if (memcmp(bufPtr, "SR", 2) == 0) {
      sioBandRate = atoi(spTemp);
    }

  }
  // Real world application would store these values in some variables for later use
  DBGPRINTF("Loaded sendChannel:[%d]\n", sendChannel);
  DBGPRINTF("Loaded recvChannel:[%d]\n", recvChannel);
  DBGPRINTF("Loaded address:[%04X]\n", address);
  DBGPRINTF("Loaded Air Rate:[%d]\n", loraBandRate);
  DBGPRINTF("Loaded SIO Rate:[%d]\n", sioBandRate);

  bRet = true;
  configFile.close();

  //DBGPRINTLN("Config ok");
  return bRet;
}

/* save config file, return True = success */
bool ICACHE_FLASH_ATTR LoRa_AS62::bSave_config()
{

  //DBGPRINTLN("--save data--");

  File configFile = SPIFFS.open(CONST_LORA_AS62_CONFIG_FILENAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_LORA_AS62_CONFIG_FILENAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }
  //DBGPRINT("before print to file");
  configFile.print("SC:"); //send channel
  configFile.println(sendChannel);
  configFile.print("RC:"); // recv Channel
  configFile.println(recvChannel);
  configFile.print("AD:"); // address 
  configFile.println(address);
  configFile.print("LR:"); // LoRa air baud rate
  configFile.println(loraBandRate);
  configFile.print("SR:"); // sio baud rate
  configFile.println(sioBandRate);

  configFile.close();
  DBGPRINTLN("-- end --");
  return true;
}

