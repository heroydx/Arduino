/*
   LoRaMac.h

   The definition of LoRa air protocol CMD list

   2017  Steven Lian (steven.lian@gmail.com)


*/
#include <LoRaMAC.h>

//global var

//#define LORA_MAC_DEFAULT_HOST_ADDR "\xC0\xA8\x00\x01"
//uint8_t LORA_MAC_BROADCAST_FFFF[LORA_MAC_ADDRESS_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};


static LoRaHost LoRaHostData;
static ListArray sendList(LORA_MAC_SEND_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));
static LoRa_AS62 LoRaDev;
//static LoRa_LCZ LoRaDev;


//print function for debug purpose

static void print_hex_data(char *title, uint8_t *ptr, short nLen)
{
  short i;
  DBGPRINTF("%s", title);
  for (i = 0; i < nLen; i++)
  {
    DBGPRINTF("%02X ", *(ptr + i));
  }
}


static uint8_t check_sum(unsigned long devID)
{
  uint8_t ret;
  ret = devID & 0xFF;
  ret ^= (devID >> 8) & 0xFF;
  ret ^= (devID >> 16) & 0xFF;
  ret ^= (devID >> 24) & 0xFF;
  return ret;
}


//class functions LoRaHost
LoRaHost::LoRaHost()
{
  //清空hostData数据
  memset(hostAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));
  memset(resultAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));

  addrBegin = LORA_MAC_HOST_DATA_ADDR_BEGIN;
  maxCount = 0;
  addrCount = 0;
  lastPos = 0;
  currPos = 0;
  batchLen = CONST_LORA_HOST_DEFAULT_BATCH_LEN;

  addrListPtr = NULL;
  seqListPtr = NULL;
  feedbackListPtr = NULL;

  strncpy(hwDevice, CONST_LORA_HARDWARE_DEVICE, CONST_HARDWARE_DEVICE_LENGTH - 1);
}

LoRaHost::~LoRaHost()
{
  //清空hostData数据
  memset(hostAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));
  memset(resultAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));

  addrBegin = LORA_MAC_HOST_DATA_ADDR_BEGIN;
  maxCount = 0;
  addrCount = 0;
  lastPos = 0;
  currPos = 0;
  batchLen = CONST_LORA_HOST_DEFAULT_BATCH_LEN;
  //free memory
  if (addrListPtr != NULL) {
    free (addrListPtr);
  }
  if (feedbackListPtr != NULL) {
    free (feedbackListPtr);
  }
}


short LoRaHost::begin(uint8_t *addr, uint8_t beginAddr, short subNum)
{
  memcpy(hostAddr, addr, LORA_MAC_ADDRESS_LEN);
  addrBegin = beginAddr;
  if ((addrBegin + subNum) > 255) {
    maxCount = ((255 - addrBegin) / 8) * 8;
  }
  else {
    maxCount = subNum;
  }

  //主控模式,分配相应的内存
  if (addrListPtr != NULL) {
    free (addrListPtr);
  }
  if (feedbackListPtr != NULL) {
    free (feedbackListPtr);
  }
  int sizOfMemory;
  sizOfMemory = sizeof (unsigned long) * maxCount;
  addrListPtr = (unsigned long *) malloc (sizOfMemory);
  memset(addrListPtr, 0, sizOfMemory);
  sizOfMemory = sizeof (uint8_t) * maxCount;
  feedbackListPtr = (uint8_t *) malloc (sizOfMemory);
  memset(addrListPtr, 0, sizOfMemory);
  bLoad_config();

}



bool LoRaHost::isEmpty()
{
  return addrCount == 0;
}

bool LoRaHost::isFull()
{
  return addrCount = maxCount;
}

//return >=0, 位置,<0没有找到
short LoRaHost::find(unsigned long devID)
{
  short ret = -1;
  short i;
  for (i = 0; i < maxCount; i++)  {
    if (devID == *(addrListPtr + i)) {
      ret = i;
      break;
    }
  }
  return ret;
}

//return >=0, 位置,<0没有找到
short LoRaHost::find(unsigned long devID, uint8_t *addr)
{
  short ret = -1;
  ret = find(devID);
  if (ret >= 0) {
    memcpy(addr, hostAddr, LORA_MAC_ADDRESS_LEN - 1);
    * (addr + LORA_MAC_ADDRESS_LEN - 1) = (uint8_t) (addrBegin + ret);

  }
  return ret;
}

//增加一个设备，如果存在，返回其位置，如果不存在，增加， 如果没有空间，返回-1；
short LoRaHost::add(unsigned long devID, uint8_t *addr)
{
  short ret = -1;
  short i;
  short pos;
  pos = find(devID);
  if (pos < 0) {
    //不存在，从新分配一个位置
    for (i = 0; i < maxCount; i++)  {
      if (*(addrListPtr + i) == 0L) {
        uint8_t nT1;
        *(addrListPtr + i) = devID;
        memcpy(addr, hostAddr, LORA_MAC_ADDRESS_LEN - 1);
        nT1 = (uint8_t)  (addrBegin + i);
        *(addr + LORA_MAC_ADDRESS_LEN - 1) = nT1;
        addrCount++;
        lastPos=i;
        ret = i;
        break;
      }
    }
    bSave_config();
  }
  else {
    //已经存在，把已经分配的地址给addr
    uint8_t nT1;
    memcpy(addr, hostAddr, LORA_MAC_ADDRESS_LEN - 1);
    nT1 = (uint8_t)  (addrBegin + pos);
    *(addr + LORA_MAC_ADDRESS_LEN - 1) = nT1;
    ret = pos;
  }
  return ret;
}

//把给定地址的位置的数据设置成devID
short LoRaHost::set(unsigned long devID, uint8_t *addr)
{
  short ret = -1;
  uint8_t pos;
  pos = *(addr + LORA_MAC_ADDRESS_LEN - 1) - addrBegin;
  if (pos >= 0 && pos < maxCount) {
    *(addrListPtr + pos) = devID;
    ret = 0;
  }
  return ret;
}


short LoRaHost::del(unsigned long devID)
{
  short ret = -1;
  if (find(devID) >= 0) {
    *(addrListPtr + ret) = 0L;
    addrCount--;
    ret = 0;
  }
  bSave_config();
  return ret;
}

//通过某个地址的获取相应设备的devID
unsigned long LoRaHost::get_devID(uint8_t *addr)
{
  unsigned long ret = 0;
  uint8_t pos;
  pos = *(addr + LORA_MAC_ADDRESS_LEN - 1) - addrBegin;
  DBGPRINTF("\n LoRaHost::get_devID [%d] [%d]",*(addr + LORA_MAC_ADDRESS_LEN - 1),pos);
  if (pos >= 0 && pos < maxCount) {
    ret = *(addrListPtr + pos);
  }
  return ret;
}

//通过某个地址的位置获取相应设备的devID
unsigned long LoRaHost::get_devID(uint8_t pos)
{
  unsigned long ret = 0;
  pos -= addrBegin;
  if (pos >= 0 && pos < maxCount) {
    ret = *(addrListPtr + pos);
  }
  return ret;
}


bool LoRaHost::if_addr_exist(uint8_t *addr, uint8_t checkSum)
{
  bool ret = false;
  DBGPRINTF("\n addr [%02X-%02X-%02X-%02X] [%02X]", *(addr), *(addr + 1), *(addr + 2), *(addr + 3), checkSum);
  if (memcmp(addr, LORA_MAC_BROADCAST_0000, LORA_MAC_ADDRESS_LEN) == 0) {
    return ret;
  }
  if (addrListPtr != NULL) {
    if (memcmp(addr, hostAddr, LORA_MAC_ADDRESS_LEN - 1) == 0) {
      uint8_t nPos;
      nPos = *(addr + 3) - addrBegin;
      if (nPos >= 0 && nPos <= maxCount) {
        if (check_sum(*(addrListPtr + nPos)) == checkSum) {
          ret = true;
          //根据发送地址，确定是否清除发送标志
          clean_send_flag(nPos);
        }
      }
    }
  }
  return ret;
}


short LoRaHost::stat()
{
  short ret = 0;
  if (addrListPtr != NULL) {
    short i;
    for (i = 0; i < maxCount; i++)  {
      if (*(addrListPtr + i) != 0L) {
        DBGPRINTF("\n STAT: i [%d] [%d]", i, *(addrListPtr + i));
        ret++;
        lastPos=i+1;
      }
    }
  }
  return ret;
}

void LoRaHost::clean()
{
  if (addrListPtr != NULL) {
    short i;
    for (i = 0; i < maxCount; i++)  {
      *(addrListPtr + i) = 0L;
    }
  }
}


short LoRaHost::position(short index)
{
  short ret;
  ret = index + addrBegin;
  if (ret >= 255) {
    ret = -1;
  }
  return ret;
}

bool LoRaHost::set_send_flag(short index)
{
  bool ret = true;
  *(feedbackListPtr + index) += 1;
  if (*(feedbackListPtr + index) > CONST_LoRa_NO_FEEDBACK_MAX_VAL)
  {
    //太多次数没有反馈，设备可能失联了，清除数据，释放空间。
    *(feedbackListPtr + index) = 0;
    *(addrListPtr + index) = 0L;
    bSave_config();
  }
  return ret;
}

bool LoRaHost::clean_send_flag(short index)
{
  bool ret = true;
  *(feedbackListPtr + index) = 0;
  return ret;
}


/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR LoRaHost::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  short nCount;
  uint8_t bigBuff[CONST_LORA_MAC_FILE_BUFFLEN + 2];
  //DBGPRINTLN("\n************** bLoad_config begin ****************");

  File configFile = SPIFFS.open(CONST_LORA_MAC_ADDRESS_FILENAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s", CONST_LORA_MAC_ADDRESS_FILENAME);
    return bRet;
  }

  nCount = 0;

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuff, CONST_LORA_MAC_FILE_BUFFLEN);
    if (nLen <= 0)
      break;
    bigBuff[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bigBuff, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bigBuff, "add", 3) == 0) {
      short nPos;
      unsigned long lVal;
      *(spTemp + 3) = '\0';
      nPos = atoi(spTemp);
      lVal = atol(spTemp + 4);
      //DBGPRINTF("\n load Config: pos[%d] [%d]", nPos, lVal);
      if ((nPos >= 0) && (nPos < maxCount)) {
        *(addrListPtr + nPos) = atol(spTemp + 4);
      }
    }
  }
  addrCount = stat();
  configFile.close();
  DBGPRINTF("\nLoRa Host addrCount [%d]\n", addrCount);
  //DBGPRINTLN("\n************** bLoad_config end ****************");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR LoRaHost::bSave_config()
{

  uint8_t bigBuff[CONST_LORA_MAC_FILE_BUFFLEN + 2];
  
  addrCount = stat();
  
  DBGPRINTLN("--save lora host data--");
  File configFile = SPIFFS.open(CONST_LORA_MAC_ADDRESS_FILENAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_LORA_MAC_ADDRESS_FILENAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }

  int i;
  for (i = 0; i < maxCount; i++) {
    if (*(addrListPtr + i) > 0) {
      sprintf((char *)bigBuff, "add:%03d:%d", i, *(addrListPtr + i));
      configFile.println((char *)bigBuff);
    }
  }

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}


void LoRaHost::debug_print_info()
{
  short i;
  DBGPRINT("\n ==Host Data==>");
  DBGPRINTF(" addrCount:[%d] / [%d] [%d] ", addrCount, maxCount,lastPos);
  DBGPRINTF(" addrBegin:[%d] currPos:[%d]  ", addrBegin, currPos);
  for (i = 0; i < maxCount; i++)  {
    if (*(addrListPtr + i) != 0L) {
      DBGPRINTF("{addr:[%d],devID:[%d]},", i + addrBegin, *(addrListPtr + i));
    }
  }
  DBGPRINTLN("<== END ==<\n");
}



//class functions LoRaMAC
LoRaMAC::LoRaMAC()
{
  memcpy(hostAddr, LORA_MAC_DEFAULT_HOST_ADDR, LORA_MAC_ADDRESS_LEN);
  //LoRa device Parameter
  loRaDevAddress = 0x1234;
  loRaDevChannel = CONST_LORA_DEFAULT_CHANNEL; //send channel
  loRaDevBandRate = CONST_LORA_AIR_BAUD_RATE;
  deviceReady = false;
  linkReady = false;

  tokenAutoFlag = true;
  tokenStrategyKey = LORA_MAC_TOKEN_STRATEGRY_DEFAULT_KEY; //emergency 和 normal 令牌间隔发送策略。每位代表一次发送机会，=0， normal, =1 emergency
  emergencyTokenLen = LORA_MAC_TOKEN_EMERGENCY_DEFAULT_LEN;
  normalTokenLen = LORA_MAC_TOKEN_NORMAL_DEFAULT_LEN;
  normalTokenPos = 0;
  ulTokenMinTimeTicks = LORA_MAC_AUTO_TOKEN_TIME_SLOT_TICK_LEN;
  nextWindowTimeSlotBegin = 0;
  //begin(false, hostAddr, 0);
}

short LoRaMAC::begin(bool isHost, short channel, uint8_t *addr, uint8_t beginAddr, short subNum )
{
  short ret = 0;
  version = CONST_LORA_MAC_VERSION;

  randomSeed(micros());
  devID = generate_devID();
  if (isHost) {
    hostFlag = LORA_MAC_HOST_FLAG_SLAVE;
    DBGPRINTLN("\n hostFlag = LORA_MAC_HOST_FLAG_SLAVE");
  }
  else {
    hostFlag = LORA_MAC_HOST_FLAG_CLIENT;
    DBGPRINTLN("\n hostFlag = LORA_MAC_HOST_FLAG_CLIENT");
  }

  nRestartReason = CONST_SYS_RESTART_REASON_NORMAL_BOOT;

  memcpy(hostAddr, addr, LORA_MAC_ADDRESS_LEN);

  normalWindowTick = ulReset_interval();
  emergencyWindowTick = ulReset_interval();
  if (hostFlag && (subNum > 0)) {
    LoRaHostData.begin(hostAddr, beginAddr, subNum);
    memcpy(meAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
  }

  //判断是否需要改变LoRa频道
  //channel = LORA_MAC_DEFAULT_CONFIG_CHANNEL; //初始化数据
  configChannel = LORA_MAC_DEFAULT_CONFIG_CHANNEL; //初始化数据

  if (bLoad_config()) {
    //提取保存的configChannel 成功
    //DBGPRINTF("\n [%d] [%d] [%d]",is_valid_channel(configChannel),configChannel,nRestartReason);
    
    if ((is_valid_channel(configChannel))
        && (configChannel != LORA_MAC_DEFAULT_CONFIG_CHANNEL)
        && (nRestartReason != CONST_SYS_RESTART_REASON_LORA_NO_SYNC_DATA)) {
          channel=configChannel;
          DBGPRINTF("\n channel=configChannel [%d] [%d]",channel,configChannel);
    }
  }

  DBGPRINTF("\n current LoRa Channel [%d] loRaDevBandRate[%d] ", channel,loRaDevBandRate);

  LoRaDev.begin(loRaDevAddress, channel, loRaDevBandRate);

  if (LoRaDev.isReady()) {
    deviceReady = true;
    DBGPRINTLN();
    workChannel = LoRaDev.sendChannel;
    loRaDevChannel = LoRaDev.sendChannel;
    DBGPRINTF("\n LoRa device is ready for communication @channel[%d]", workChannel);
  }
  else{
    DBGPRINTF("\n LoRa device is NOT ready @channel[%d], will restart!!!", workChannel);
    vSystem_restart(CONST_SYS_RESTART_REASON_LORA_DEVICE_IS_NOT_EXIST);
  }

  //ulClientNoTimeSlotPeiod=CONST_LORA_DEFAULT_CLIENT_NO_TIME_SLOT_inMS;

  set_received_data_automatic(true);

  hostMasterExist = false;
  slaveCheckHostMasterExistTick = ulReset_interval();
  clientCheckHostMasterExistTick = ulReset_interval();
  configCheckTick = ulReset_interval();

  //reset boot reason
  nRestartReason = CONST_SYS_RESTART_REASON_NORMAL_BOOT;
  bSave_config();

}

short LoRaMAC::begin(bool isHost, short channel, uint8_t *addr, short subNum)
{
  return begin(isHost, channel, addr, LORA_MAC_HOST_DATA_ADDR_BEGIN, subNum );
}


short LoRaMAC::begin(bool isHost, short channel, uint8_t *addr)
{
  return begin(isHost, channel, addr, LORA_MAC_HOST_DATA_ADDR_BEGIN, LORA_MAC_HOST_DATA_LENGTH );
}


short LoRaMAC::begin(bool isHost, short channel)
{
  return begin(isHost, channel, (uint8_t *)LORA_MAC_DEFAULT_HOST_ADDR, LORA_MAC_HOST_DATA_ADDR_BEGIN, LORA_MAC_HOST_DATA_LENGTH );
}


short LoRaMAC::begin(bool isHost)
{
  return begin(isHost, loRaDevChannel, (uint8_t *)LORA_MAC_DEFAULT_HOST_ADDR, LORA_MAC_HOST_DATA_ADDR_BEGIN, LORA_MAC_HOST_DATA_LENGTH );
}

uint8_t LoRaMAC::set_host_mode(uint8_t hostMode)
{
  switch (hostMode)
  {
    case LORA_MAC_HOST_FLAG_MASTER:
    case LORA_MAC_HOST_FLAG_SLAVE:
    case LORA_MAC_HOST_FLAG_CLIENT:
      hostFlag = hostMode;
      break;
    default:
      break;
  }
  return hostFlag;
};


LoRaMAC::~LoRaMAC()
{

}

unsigned short LoRaMAC::get_version()
{
  return version;
}

unsigned long LoRaMAC::generate_devID()
{
  unsigned long devID;
  devID = ESP.getChipId();
  chksum = check_sum(devID);
  return devID;
}

bool LoRaMAC::devReady()
{
  return deviceReady;
}

bool LoRaMAC::is_valid_channel(short channel)
{
  return LoRaDev.is_valid_channel(channel);
}

bool LoRaMAC::hash_verification_process(char *ptr, char *result)
{
  //暂时没有进行校验
  return true;
}

//计算主控机器(hostFlag==true)的发射时间窗口,默认利用发送数据(sendData)里面的参数计算。
unsigned long LoRaMAC::cal_host_time_slot() {
  unsigned long ret;
  short CMD;
  //DBGPRINTLN("===*** cal_host_time_slot ***===");
  CMD = sendData.exCMD.CMD;
  switch (CMD)
  {
    case LORA_CMD_SYNC_CMD:
    case LORA_CMD_SYNC_RANDOM:
      //给非主控设备的time solt个数+1
      ret = LORA_MAC_TIME_SLOT_IN_MS * (sendData.syncCMD.groupLen + 1);
      break;
    default:
      //默认命令下发以后,等待两个time slt;
      ret = LORA_MAC_TIME_SLOT_IN_MS + LORA_MAC_TIME_SLOT_IN_MS;
      break;
  }
  return ret;
}

short LoRaMAC::check_time_window_host()
{
  short ret = 0;
  unsigned long tick;
  tick = ulGet_interval(nextWindowTick);

  //如果是主控设备,只要是等待发送时间超过,并清除等待时间,就可以打开发送窗口
  if (tick > (nextWindowTimeSlotBegin )) {
    sendWindowReady = true;
    nextWindowTimeSlotBegin = 5;//给个5ms过度时间
  }

  if (sendWindowReady) {
    //send windows is avaiable
    if  (sendList.count() > 0) {
      if (!LoRaDev.busy()) {
        //loRaDevice is not busy,so pop data in sendList
        sendList.rpop(sendData.u8Data);
        //填充顺序号
        sendData.syncCMD.seq = seq++;
        //填充checksum
        sendData.syncCMD.chksum = chksum;
        if (hostFlag == LORA_MAC_HOST_FLAG_MASTER) {
          //只有主控master设备才可以发送数据
          //send data via lora
          DBGPRINTLN("\n---------->>>>>>> data sent begin=");
          //debug_print_union(sendData.u8Data);
          ret = LoRaDev.send(sendData.u8Data, sizeof (stDataStreamUnion));
          DBGPRINTLN("\n---------->>>>>>> data sent end=\n");
        }
        //sent,disable windows
        sendWindowReady = false;
        //如果是主控设备,需要计算自己的下一个时间窗口.
        nextWindowTimeSlotBegin = cal_host_time_slot();
        nextWindowTick = ulReset_interval();
      }
    }
  }
}


short LoRaMAC::check_time_window_client()
{
  short ret = 0;
  unsigned long tick;
  tick = ulGet_interval(nextWindowTick);
  if (sendWindowReady) {
    bool timeSlotOpen = true;
    //send window is ready
    //if (!hostFlag) DBGPRINTF("\n--** sendWindowReady:[%d]  send List [%d] **--\n", sendWindowReady,sendList.count());
    //主控设备和非主控设备对时间窗口的判断不同
    //非主控设备,只能按照主控设备规定的发送时间窗口
    if (tick < nextWindowTimeSlotBegin ) {
      //窗口时间没有到
      timeSlotOpen = false;
    }
    //非主控设备,超出给定窗口时间,此次发送窗口就关闭了
    if (tick > (nextWindowTimeSlotBegin + LORA_MAC_TIME_SLOT_IN_MS)) {
      //窗口时间过了
      DBGPRINTLN ("============================================missed window ==============================================");
      timeSlotOpen = false;
      sendWindowReady = false;
    }
    if (timeSlotOpen) {
      //send windows is avaiable
      if  (sendList.count() > 0) { 
        if (!LoRaDev.busy()) {
          //loRaDevice is not busy,so pop data in sendList
          sendList.rpop(sendData.u8Data);
          //填充顺序号
          sendData.syncCMD.seq = seq++;
          //填充checksum
          sendData.syncCMD.chksum = chksum;

          //send data via lora
          //DBGPRINTLN("\n---------->>>>>>> data sent begin=");
          //debug_print_union(sendData.u8Data);
          ret = LoRaDev.send(sendData.u8Data, sizeof (stDataStreamUnion));
          //DBGPRINTLN("\n---------->>>>>>> data sent end=\n");
          //sent,disable windows
          sendWindowReady = false;
        }
        //close the send window,waiting for next time
      }
      // send window is avaiable, so, don't need to restart;
      //ulClientNoTimeSlotTick=ulReset_interval();
      clientNoTimeSlotCount = 0;
    }
  }

  //check if need to restart
  //if (ulGet_interval(ulClientNoTimeSlotTick)>ulClientNoTimeSlotPeiod){
  if (clientNoTimeSlotCount > CONST_LORA_DEFAULT_MAX_CLIENT_NO_TIME_SLOT_LOOP) {
    DBGPRINTF("\n(clientNoTimeSlotCount > CONST_LORA_DEFAULT_MAX_CLIENT_NO_TIME_SLOT_LOOP):[%d]",clientNoTimeSlotCount);
    vSystem_restart(CONST_SYS_RESTART_REASON_LORA_NO_SYNC_DATA);
  }

}

short LoRaMAC::check_time_window_send()
{
  short ret = 0;
  if (hostFlag) {
    ret = check_time_window_host();
  }
  else {
    ret = check_time_window_client();
  }
  return ret;
}

// return >0, 1,2,3,4,100 if the message is to me;
short LoRaMAC::address_me(uint8_t *addrPtr)
{
  short ret = 0;
  short i;
  short k = 0;
  uint8_t tempAddr[LORA_MAC_ADDRESS_LEN];
  for (i = LORA_MAC_ADDRESS_LEN - 1; i >= 0 ; i--)
  {
    if (addrPtr[i] == 0xFF) {
      k++;
    }
  }

  if (k == LORA_MAC_ADDRESS_LEN) {
    ret = 4;
  }
  else if (k == 3) {
    if (meAddr[0] == addrPtr[0])
      ret = 3;
  }
  else if (k == 2) {
    if ((meAddr[0] == addrPtr[0]) && (meAddr[1] == addrPtr[1]))
      ret = 2;
  }
  else if (k == 1) {
    if ((meAddr[0] == addrPtr[0]) && (meAddr[1] == addrPtr[1]) && (meAddr[2] == addrPtr[2]))
      ret = 1;
  }
  else {
    if (memcmp(addrPtr, meAddr, LORA_MAC_ADDRESS_LEN == 0)) {
      ret = 100;
    }
  }
  return ret;
}

short LoRaMAC::func_cmd_config_channel_broardcast()
{
  short ret;
  DBGPRINTLN("**== test func_cmd_config_channel_broardcast ==**");
  DBGPRINTF("to channel: [%d] [%d]\n", recvData.configChannel.channel01, recvData.configChannel.channel02);
  if (address_me(recvData.configChannel.destAddr)) //广播命令或者是给me自己的命令
  {
    if ((recvData.configChannel.channel01 = recvData.configChannel.channel02)
        && (is_valid_channel(recvData.configChannel.channel01)))
    {
      configChannel = recvData.configChannel.channel01;
      vSystem_restart(CONST_SYS_RESTART_REASON_LORA_CONFIG_DONE_REBOOT);
    }
  }
  return ret;
}


short LoRaMAC::func_cmd_host_info_broardcast()
{
  short ret;
  return ret;
}


short LoRaMAC::func_cmd_extend_cmd()
{
  short ret;
  return ret;
}



short LoRaMAC::send_registration_cleanup(uint8_t *addr)
{
  short ret = -1;
  DBGPRINTLN("\n==send_registration_cleanup==");
  sendData.exCMD.CMD = LORA_CMD_REGISTRATION_CLEANUP;
  memcpy(sendData.exCMD.destAddr, addr, LORA_MAC_ADDRESS_LEN);
  memcpy(sendData.exCMD.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
  debug_print_union(sendData.u8Data);
  send_data(sendData.u8Data);
  return ret;
}


//原来分配的地址有问题，冲突或者没有登记，需要重新申请
bool LoRaMAC::func_cmd_registration_cleanup()
{
  bool ret = false;

  DBGPRINTLN("**== test func_cmd_registration_cleanup ==**");
  DBGPRINTF("to devID: [%d], me devID:[%d]\n", recvData.regFeedback.devID, devID);
  if (address_me(recvData.regFeedback.destAddr)) //广播命令或者是给me自己的命令
  {
    //message is from host to me.
    //memcpy(hostAddr,recvData.regFeedback.sourceAddr, LORA_MAC_ADDRESS_LEN);
    //got host DHCP result
    DBGPRINTLN("-- func_cmd_registration_cleanup match--\n");
    memset(meAddr, 0, LORA_MAC_ADDRESS_LEN);
    memcpy(hostAddr, recvData.regFeedback.sourceAddr, LORA_MAC_ADDRESS_LEN);
    linkReady = false;
    ret = true;
    vSystem_restart(CONST_SYS_RESTART_REASON_LORA_ADDR_CLEAN_UP);
  }
  return ret;
}



short LoRaMAC::func_cmd_registration_request()
{
  short ret = -1;
  DBGPRINTLN("**== test func_cmd_registration_request ==**");
  if (address_me(recvData.regRequest.destAddr)) { //广播命令或者是给me自己的命令
    unsigned long ulDevID;
    DBGPRINTLN("-- func_cmd_registration_request match--\n");
    ulDevID = recvData.regRequest.devID;
    ret = LoRaHostData.add(ulDevID, sendData.regFeedback.addr);
    DBGPRINTF("--LoRaHostData.add ret [%d] \n", ret);
    if (ret >= 0) {
      sendData.regFeedback.CMD = LORA_CMD_REGISTRATION_FEEDBACK;
      sendData.regFeedback.devID = recvData.regRequest.devID;
      memcpy(sendData.regFeedback.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
      memcpy(sendData.regFeedback.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
      //memcpy(sendData.regFeedback.addr, sendData.regFeedback.sourceAddr, LORA_MAC_ADDRESS_LEN);
      //sendData.regFeedback.addr[LORA_MAC_ADDRESS_LEN-1]=ret;
      memcpy(sendData.regFeedback.val, recvData.regRequest.key, LORA_MAC_KEY_MD5);
      DBGPRINTLN("----SPECIAL CHECK-----");
      debug_print_union(sendData.u8Data);
      send_data(sendData.u8Data);
    }
  }
  return ret;
}


short LoRaMAC::func_cmd_registration_feedback_slave()
{
  short ret = -1;
  DBGPRINTLN("**== test func_cmd_registration_feedback_slave ==**");
  if (!memcpy(recvData.regRequest.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN)) { //主机master发送的命令，自己是从机
    unsigned long ulDevID;
    DBGPRINTLN("-- func_cmd_registration_feedback_slave match--\n");
    ulDevID = recvData.regFeedback.devID;
    ret = LoRaHostData.set(ulDevID, recvData.regFeedback.destAddr);
  }
  return ret;
}


short LoRaMAC::func_cmd_addr_feedback_slave()
{
  short ret = -1;
  DBGPRINTLN("**== test func_cmd_registration_feedback_slave ==**");
  if (!memcpy(recvData.regRequest.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN)) { //主机master发送的命令，自己是从机
    unsigned long ulDevID;
    DBGPRINTLN("-- func_cmd_registration_feedback_slave match--\n");
    ulDevID = recvData.regFeedback.devID;
    ret = LoRaHostData.set(ulDevID, recvData.regFeedback.destAddr);
  }
  return ret;
}



short LoRaMAC::func_cmd_registration_feedback_client()
{
  short ret = 0;

  DBGPRINTLN("**== test cmd_registration_feedback ==**");
  DBGPRINTF("to devID: [%d], me devID:[%d]\n", recvData.regFeedback.devID, devID);
  if ((address_me(recvData.regFeedback.destAddr)) && //广播命令或者是给me自己的命令
      (recvData.regFeedback.devID == devID) && //发给me的命令,
      hash_verification_process((char *) key, (char *) recvData.regFeedback.val)) //hash校验成功
  {
    //message is from host to me.
    //memcpy(hostAddr,recvData.regFeedback.sourceAddr, LORA_MAC_ADDRESS_LEN);
    //got host DHCP result
    DBGPRINTLN("-- cmd_registration_feedback match--\n");
    memcpy(meAddr, recvData.regFeedback.addr, LORA_MAC_ADDRESS_LEN);
    memcpy(hostAddr, recvData.regFeedback.sourceAddr, LORA_MAC_ADDRESS_LEN);
    linkReady = true;

  }
  return ret;
}

short LoRaMAC::func_cmd_addr_request()
{
  ;
}

// got feedback from host
short LoRaMAC::func_cmd_addr_feedback_client()
{
  ;
}

//正常通信发射窗口,非主控设备
short LoRaMAC::func_cmd_sync_cmd()
{
  short ret = -1;
  DBGPRINTLN("**== test cmd_sync_cmd ==**");
  if (address_me(recvData.syncCMD.destAddr))  //广播命令或者是给me自己的命令
  {
    if (!memcmp(meAddr, recvData.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1)) {
      // 本机属于这个组group
      uint8_t i;
      DBGPRINTLN("-- cmd_sync_cmd match--\n");
      for (i = 0; i < recvData.syncCMD.groupLen; i++) {
        if (meAddr[LORA_MAC_ADDRESS_LEN - 1] == recvData.syncCMD.list[i]) {
          //本机在本次的发送位置,并计算发送时间,并打开发射时间窗口
          ret = i;
          nextWindowTimeSlotBegin = LORA_MAC_TIME_SOLT_OFFSET_IN_MS + LORA_MAC_TIME_SLOT_IN_MS * ret;
          DBGPRINTF("\nTime Slot %d, next Windows Time %d\n", ret, nextWindowTimeSlotBegin);
          nextWindowTick = ulReset_interval();
          sendWindowReady = true;
          //计算两个窗口距离时间
          normalWindowPeriod = ulGet_interval(normalWindowTick);
          normalWindowTick = ulReset_interval();
          break;
        }
      }
    }
  }
  //
  clientCheckHostMasterExistTick=ulReset_interval();
  return ret;
}

short LoRaMAC::func_cmd_sync_time()
{

  ;
}

//紧急命令发射窗口,非主控设备
short LoRaMAC::func_cmd_sync_random()
{
  short ret = -1;
  DBGPRINTLN("**== test cmd_sync_random ==**");
  if (address_me(recvData.syncCMD.destAddr))  //广播命令或者是给me自己的命令
  {
    if (emergencyData.syncCMD.CMD != LORA_CMD_NULL_CMD) {
      DBGPRINTLN("+++ there is a emergency data avaiable +++");
      //有紧急命令,清空当前队列
      sendList.clean();
      //塞入 emergencyData
      //sendList.lpush(emergencyData.u8Data);
      send_data(emergencyData.u8Data);
      //随机产生本机在本次的发送位置,并计算发送时间,并打开发射时间窗口
      ret = random(recvData.syncCMD.groupLen);
      nextWindowTimeSlotBegin = LORA_MAC_TIME_SOLT_OFFSET_IN_MS + LORA_MAC_TIME_SLOT_IN_MS * ret;
      DBGPRINTF("\nTime Slot %d, next Window Time %d\n", ret, nextWindowTimeSlotBegin);
      nextWindowTick = ulReset_interval();
      sendWindowReady = true;
      //计算两个窗口距离时间
      emergencyWindowPeriod = ulGet_interval(emergencyWindowTick);
      emergencyWindowTick = ulReset_interval();
      //清空紧急命令
      emergencyData.syncCMD.CMD = LORA_CMD_NULL_CMD;
    }
  }
  return ret;
}

//对验证请求的反馈,回送对方8个字节的数据,和对应的MD5或者其他方式的hash结果,结果不少于8个字节。
short LoRaMAC::func_cmd_verify_request()
{
  short ret = 0;
  DBGPRINTLN("**== test func_cmd_verify_request ==**");
  DBGPRINTF("to devID: [%d], me devID:[%d]\n", recvData.regFeedback.devID, devID);
  if ((address_me(recvData.verifyRegCMD.destAddr)) && //广播命令或者是给me自己的命令
      (recvData.verifyRegCMD.devID == devID)) //发给me的命令,
  {
    //message is to me.
    DBGPRINTLN("-- func_cmd_verify_request match--\n");
    sendData.verifyFeedCMD.devID = devID;
    memcpy(sendData.verifyFeedCMD.sourceAddr, meAddr, LORA_MAC_ADDRESS_LEN);
    memcpy(sendData.verifyFeedCMD.destAddr, recvData.verifyRegCMD.sourceAddr, LORA_MAC_ADDRESS_LEN);
    hash_verification_process((char *) recvData.verifyRegCMD.key, (char *) sendData.verifyFeedCMD.val);
    send_data(sendData.u8Data);
  }
  return ret;
}


bool LoRaMAC::func_cmd_verify_feedback()
{
  bool ret = false;
  DBGPRINTLN("**== test func_cmd_verify_feedback ==**");
  if ((address_me(recvData.verifyFeedCMD.destAddr)) && //广播命令或者是给me自己的命令
      (recvData.verifyFeedCMD.devID == devID) && //发给me的命令,
      hash_verification_process((char *) key, (char *) recvData.verifyFeedCMD.val)) //hash校验成功
  {
    //message is to me.
    DBGPRINTLN("-- func_cmd_verify_feedback match--\n");
    ret = true;
  };
  return ret;
}

short LoRaMAC::func_cmd_frequency_change()
{
  short ret;
  DBGPRINTLN("**== test func_cmd_config_channel_broardcast ==**");
  DBGPRINTF("to channel: [%d] [%d]\n", recvData.configChannel.channel01, recvData.configChannel.channel02);
  if (address_me(recvData.configChannel.destAddr)) //广播命令或者是给me自己的命令
  {
    if ((recvData.configChannel.channel01 = recvData.configChannel.channel02)
        && (is_valid_channel(recvData.configChannel.channel01)))
    {
      configChannel = recvData.configChannel.channel01;
      vSystem_restart(CONST_SYS_RESTART_REASON_LORA_FREQ_CHANGE_REBOOT);
    }
  }
  return ret;
}


//准备并且发送注册信息给服务器
short LoRaMAC::registration(short maid, short pid)
{
  short ret = 0;
  if (!hostFlag && emergencyData.regRequest.CMD == LORA_CMD_NULL_CMD)
  {
    //非主控设备才需要注册
    emergencyData.regRequest.CMD = LORA_CMD_REGISTRATION_REQUEST;
    memcpy(emergencyData.regRequest.sourceAddr, LORA_MAC_BROADCAST_0000, LORA_MAC_ADDRESS_LEN);
    memcpy(emergencyData.regRequest.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
    //emergencyData.regRequest.seq = seq;
    emergencyData.regRequest.devID = devID;
    emergencyData.regRequest.MAID = maid;
    emergencyData.regRequest.PID = pid;
    //生成验证的key 和 val
    random_string((char *) key, LORA_MAC_KEY_LEN);
    memcpy(emergencyData.regRequest.key, key, LORA_MAC_KEY_LEN);
    simpleHash((char *) key, (char *)  val, devID, LORA_MAC_KEY_LEN);

    ret = pid;
  }
  return ret;
}

unsigned long LoRaMAC::get_devID(uint8_t *addr)
{
  return LoRaHostData.get_devID(addr);
}

unsigned long LoRaMAC::get_devID(uint8_t pos)
{
  return LoRaHostData.get_devID(pos);
}


short LoRaMAC::cal_server_data_len(unsigned short type)
{
  short ret;
  switch (type)
  {
    case CONST_LORA_REPORT_TYPE_REGISTRATION:
      ret = sizeof(stLoRaRptRegistration);
      break;
    case CONST_LORA_REPORT_TYPE_STATUS:
      ret = sizeof(stLoRaRptStatus);
      break;
    case CONST_LORA_REPORT_TYPE_TODO:
      ret = sizeof(stLoRaRptTODO);
      break;
    default:
      ret = sizeof(stLoRaDataServer);
      break;

  }
  return ret;
}

//自动处理有关注册登记,时间窗口,校验等信息,主机侧master
short LoRaMAC::handle_received_data_auto_host_master()
{
  short ret = 0;
  short CMD;
  bool bHostIfAddrExist;
  DBGPRINTLN("\n===*** handle_received_data_auto_host_master ***===");
  //check if the address exist
  if (LoRaHostData.if_addr_exist(recvData.exCMD.sourceAddr, recvData.exCMD.chksum)) {
    //device 已经登记 registerred
    CMD = recvData.exCMD.CMD;
    switch (CMD)
    {
      case LORA_CMD_EXTEND_CMD:
        func_cmd_extend_cmd();
        ret = sizeof (stDataStreamUnion);
        break;
      case LORA_CMD_REGISTRATION_REQUEST:
        func_cmd_registration_request();
        ret = sizeof (stDataStreamUnion);
        break;
      case LORA_CMD_ADDR_REQUEST:
        func_cmd_addr_request();
        ret = sizeof (stDataStreamUnion);
        break;
      //      case LORA_CMD_SYNC_TIME:
      //        func_cmd_sync_time();
      //        break;
      case LORA_CMD_VERIFY_REQUEST:
        func_cmd_verify_request();
        ret = sizeof (stDataStreamUnion);
        break;
      case LORA_CMD_VERIFY_FEEDBACK:
        func_cmd_verify_feedback();
        ret = sizeof (stDataStreamUnion);
        break;
      default:
        ret = sizeof (stDataStreamUnion);
        break;
    }

  }
  else {
    //no registerred device,need to register
    CMD = recvData.exCMD.CMD;
    DBGPRINT("\n ---- no registerred device,need to register----");

    switch (CMD)
    {
      case LORA_CMD_REGISTRATION_REQUEST:
        DBGPRINTF("\n ---- CMD:[%d], devID:[%d], ----", CMD, recvData.regRequest.devID);
        func_cmd_registration_request();
        ret = sizeof (stDataStreamUnion);
        break;
      default:
        DBGPRINTF("\n ---- CMD:[%d] ----", CMD);
        debug_print_union(recvData.u8Data);
        send_registration_cleanup(recvData.exCMD.sourceAddr);
        ret = sizeof (stDataStreamUnion);
        break;
    }
  }
  return ret;
}


bool LoRaMAC::is_host_master_msg()
{
  //默认使用recvData
  bool ret = false;
  if (!memcmp(recvData.exCMD.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN)) {
    hostMasterExist = true;
    slaveCheckHostMasterExistTick = ulReset_interval();
    ret = true;
  }
  return ret;
}

void LoRaMAC::addr_conflict_detection()
{
  //默认使用recvData
  if (!memcmp(recvData.exCMD.sourceAddr, meAddr, LORA_MAC_ADDRESS_LEN)) {        
    DBGPRINTLN("\n CONST_SYS_RESTART_REASON_LORA_ADDR_CONFLICT_REBOOT");
    vSystem_restart(CONST_SYS_RESTART_REASON_LORA_ADDR_CONFLICT_REBOOT);
  }
}


bool LoRaMAC::is_to_me_msg()
{
  //默认使用recvData
  bool ret = false;
  if (!memcmp(recvData.exCMD.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN)) {
    if(address_me(recvData.exCMD.destAddr))
      ret = true;
  }
  return ret;
}

//自动处理有关注册登记,时间窗口,校验等信息,主机侧slave
short LoRaMAC::handle_received_data_auto_host_slave()
{
  short ret = 0;
  short CMD;
  DBGPRINTLN("\n===*** handle_received_data_auto_host_slave ***===");
  CMD = recvData.exCMD.CMD;
  switch (CMD)
  {
    /*    case LORA_CMD_CONFIG_CHANNEL_BROARDCAST:
          func_cmd_config_channel_broardcast();
          ret = sizeof (stDataStreamUnion);
          break;
    */
    case LORA_CMD_EXTEND_CMD:
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      func_cmd_registration_feedback_slave();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_ADDR_REQUEST:
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      func_cmd_addr_feedback_slave();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_SYNC_TIME:
      func_cmd_sync_time();
      break;
    case LORA_CMD_VERIFY_REQUEST:
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      ret = sizeof (stDataStreamUnion);
      break;
    default:
      ret = sizeof (stDataStreamUnion);
      break;
  }
  return ret;
}



//自动处理有关注册登记,时间窗口,校验等信息
short LoRaMAC::handle_received_data_auto_client()
{
  short ret = 0;
  short CMD;
  DBGPRINTLN("\n===*** handle_received_data_automatic client ***===");
  CMD = recvData.exCMD.CMD;
  switch (CMD)
  {
    case LORA_CMD_CONFIG_CHANNEL_BROARDCAST:
      func_cmd_config_channel_broardcast();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_HOST_INFO_BROARDCAST:
      func_cmd_host_info_broardcast();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_REGISTRATION_CLEANUP:
      func_cmd_registration_cleanup();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_EXTEND_CMD:
      func_cmd_extend_cmd();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      func_cmd_registration_feedback_client();
      ret = sizeof (stDataStreamUnion);
      clientNoTimeSlotCount=0;//清除time slot over time 计数器
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      func_cmd_addr_feedback_client();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_SYNC_CMD:
      //非主控设备需要获得时间窗口
      func_cmd_sync_cmd();
      break;
    case LORA_CMD_SYNC_LOOP_BEGIN:
      //注册成功以后，新一轮time slot 分配,用于判断本机是否在一轮中有至少一次发送机会。
      //if (linkReady)
        clientNoTimeSlotCount++;
      break;
    case LORA_CMD_SYNC_TIME:
      func_cmd_sync_time();
      break;
    case LORA_CMD_SYNC_RANDOM:
      //非主控设备需要获得时间窗口
      func_cmd_sync_random();
      break;
    case LORA_CMD_VERIFY_REQUEST:
      func_cmd_verify_request();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      func_cmd_verify_feedback();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_FREQ_CHANGE:
      func_cmd_frequency_change();
      ret = sizeof (stDataStreamUnion);
      break;
    default:
      //application call to call external application API
      //cmd_application_call(CMD, recvData.u8Data, sizeof(stDataStreamUnion));
      ret = sizeof (stDataStreamUnion);
      break;
  }
  
  return ret;
}


//自动处理有关注册登记,时间窗口,校验等信息
short LoRaMAC::handle_received_data_automatic()
{
  short ret = 0;
  if (automaticFlag) {
    switch (hostFlag)
    {
      case LORA_MAC_HOST_FLAG_CLIENT:
        ret = handle_received_data_auto_client();
        break;
      case LORA_MAC_HOST_FLAG_MASTER:
        ret = handle_received_data_auto_host_master();
        break;
      case LORA_MAC_HOST_FLAG_SLAVE:
        ret = handle_received_data_auto_host_slave();
        break;
      default:
        break;
    }
  }
  return ret;
}

//如果configChannel和默认LORA_MAC_DEFAULT_CONFIG_CHANNEL不一致，说明配置完成。
void LoRaMAC::LoRa_channel_config_check()
{
  if (workChannel == LORA_MAC_DEFAULT_CONFIG_CHANNEL) {
    if (ulGet_interval(configCheckTick) > LORA_MAC_CHECK_CONFIG_INFORMATION_TIME_inMS) {
      if (configChannel != LORA_MAC_DEFAULT_CONFIG_CHANNEL) {
        DBGPRINTF("\n LoRa_channel_config_check:[%d] [%d] then reboot", configChannel, LORA_MAC_DEFAULT_CONFIG_CHANNEL);
        vSystem_restart(CONST_SYS_RESTART_REASON_LORA_CONFIG_TIME_OUT_REBOOT);
      }
    }
  }
}

bool LoRaMAC::set_received_data_automatic(bool onOff)
{
  automaticFlag = onOff;
  return automaticFlag;
}

//检查是否收到数据，并自动执行部分无需上层处理的操作,例如时间窗口处理,收到的数据存储在recvData结构里面
short LoRaMAC::available()
{
  short ret = 0;
  short nLen;
  //DBGPRINTLN("LoRaMAC::available()");
  nLen = LoRaDev.available();
  //nLen = abs(nLen); //nLen <0, crc8 error,
  if (nLen > 0) {
    LoRaDev.get(recvData.u8Data); // 必须使用recvData.u8Data,后面会默认按这个数据地址处理
    DBGPRINTLN("\n<<-- data received==:");
    debug_print_union(recvData.u8Data);
    //ret = handle_received_data_automatic();
    is_host_master_msg(); //默认使用recvData
    addr_conflict_detection(); //地址冲突检测
    handle_received_data_automatic();
    ret = nLen;
  }

  switch (hostFlag)
  {
    case LORA_MAC_HOST_FLAG_CLIENT:
      check_time_window_send();
      //只有client需要配置频道，主机可以从wifi配置里面得到。这个里面可以有争议
      LoRa_channel_config_check();
      //check if exist host,otherwise restart;
      if (ulGet_interval(clientCheckHostMasterExistTick)>LORA_MAC_CHECK_HOST_EXIST_TIME_inMS){
        DBGPRINTF("\n(clientCheckHostMasterExistTick > LORA_MAC_CHECK_HOST_EXIST_TIME_inMS):[%d]",LORA_MAC_CHECK_HOST_EXIST_TIME_inMS);
        vSystem_restart(CONST_SYS_RESTART_REASON_LORA_CLIENT_NO_HOST_TIME_OUT_REBOOT);
      }
      break;
    case LORA_MAC_HOST_FLAG_MASTER:
      //只有主控master设备才可以自动发放token
      check_insert_token_window();
      check_time_window_send();
      break;
    case LORA_MAC_HOST_FLAG_SLAVE:
      //主控Slave模式不需要发送信息
      if (ulGet_interval(slaveCheckHostMasterExistTick) > LORA_MAC_CHECK_HOST_MASTER_SLAVE_TIME_inMS) {
        if (hostMasterExist == false) {
          //主机master不存在，需要切换到master
          DBGPRINTLN("== Host Master is not existing, switch to master mode");
          hostFlag = LORA_MAC_HOST_FLAG_MASTER;
        }
      }
      break;
    default:
      check_time_window_send();
      break;
  }

  delay(1);

  return ret;
}

//检查是否收到数据，收到的数据存储在recvData结构里面
short LoRaMAC::available(short debug)
{
  short ret = 0;
  ret = LoRaDev.available();
  if (ret) {
    LoRaDev.get(recvData.u8Data); // 必须使用recvData.u8Data,后面会默认按这个数据地址处理
  }
  delay(1);
  return ret;
}


//发送数据,实际操作是发到一个队列里面, <0表示失败,>=0表示在队列的位置,
short LoRaMAC::send_data(uint8_t *u8Ptr)
{
  return send_data(u8Ptr, sizeof(stDataStreamUnion));
}

short LoRaMAC::send_data(uint8_t *u8Ptr, short nLen)
{
  short ret = -1;
  if (nLen <= sizeof(stDataStreamUnion)) {
    //发送长度符合要求;
    if (!sendList.isFull()) {
      ret = sendList.lpush(u8Ptr);
    }
  }
  return ret;
}

//directly send data, only for config purpose;
short LoRaMAC::config_send_data(uint8_t *u8Ptr)
{
  short ret = -1;
  ret = LoRaDev.send(u8Ptr, sizeof (stDataStreamUnion));
  return ret;
}


short LoRaMAC::send_normal_new_loop_flag()
{
  short ret = -1;
  DBGPRINTLN("\n==send_normal_new_loop_flag==");
  sendData.exCMD.CMD = LORA_CMD_SYNC_LOOP_BEGIN;
  memcpy(sendData.exCMD.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
  memcpy(sendData.exCMD.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
  debug_print_union(sendData.u8Data);
  send_data(sendData.u8Data);
  return ret;
}

short LoRaMAC::token_type(short loopCount)
{
  short ret;
  ret = tokenStrategyKey >> (tokenStrategyloopCount % LORA_MAC_TOKEN_STRATEGRY_BITLEN);
  ret &= 1;
  return ret;
}

void LoRaMAC::check_insert_token_window()
{
  //DBGPRINTLN("LoRaMAC::check_insert_token_window()");
  if (ulGet_interval(ulTokenSlotTick) > ulTokenMinTimeTicks)
  {
    //DBGPRINTF("\nulGet_interval(ulTokenSlotTick) > ulTokenMinTimeTicks %d,   %d",ulGet_interval(ulTokenSlotTick),ulTokenMinTimeTicks);
    //DBGPRINTF("\nsendList.isEmpty() && sendWindowReady %d %d",sendList.isEmpty(),sendWindowReady);
    if (sendList.isEmpty() && sendWindowReady)
    {
      //发送队列空闲，插入一个token
      uint8_t nListLen = 0;
      if ((token_type(tokenStrategyloopCount)) == LORA_MAC_AIR_TIME_SLOT_TYPE_NORMAL) {
        short i;
        uint8_t nT1, nT2 = 2;
        DBGPRINTLN("\nsendList is empty, Normal");
        if (normalTokenPos == 0) {
          DBGPRINTLN("\nNew Loop");
          send_normal_new_loop_flag();
        }

        tokenData.syncCMD.CMD = LORA_CMD_SYNC_CMD;
        memcpy(tokenData.syncCMD.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
        memcpy(tokenData.syncCMD.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
        tokenData.syncCMD.groupLen = normalTokenLen;
        memcpy(tokenData.syncCMD.groupAddr, hostAddr, LORA_MAC_ADDRESS_LEN - 1);
        //DBGPRINTF("\nNomal: addrCount[%d],normalTokenPos[%d]",LoRaHostData.addrCount, normalTokenPos);
        DBGPRINTF("\nsend list, normalTokenLen [%d]", normalTokenLen);
        //clean list
        for (i = 0; i < normalTokenLen; i++)
        {
          tokenData.syncCMD.list[i] = 0;
        }

        //fill list
        for (i = 0; i < normalTokenLen; i++)
        {
          nT1 = (uint8_t) LoRaHostData.position(normalTokenPos);
          if ((nT1 <= nT2) && (nT2 != (LoRaHostData.addrBegin + LoRaHostData.maxCount)))
          {
            //DBGPRINTF("break：nT1[%d],nT2[%d] i[%d]",nT1,nT2, i);
            DBGPRINTF("break：nListLen[%d]",nT1,nT2, i);
            //nListLen = i;
            break;
          }

          //设置运行相应地址设备回馈消息标志
          LoRaHostData.set_send_flag(normalTokenPos);

          if (nT1 < 0) {
            break;
          }
          DBGPRINTF(" [%d]", nT1);
          nListLen++;
          tokenData.syncCMD.list[i] = nT1;
          nT2 = nT1;

          normalTokenPos++;
          //if (normalTokenPos >= LoRaHostData.addrCount) {
          if (normalTokenPos >= LoRaHostData.lastPos) {
            normalTokenPos = 0;
            //if (i == 0 && LoRaHostData.addrCount > 0) {
            //  nListLen = 1;
            //}
            //nListLen = i;
            break;//确保每一轮Normal Token的发放都是从0开始。
          }
        }
        tokenData.syncCMD.groupLen = nListLen;
        DBGPRINTF("\n Nomal Token Data: nListLen [%d]",nListLen);
        debug_print_union(tokenData.u8Data);
      }
      else {
        //LORA_MAX_AIR_TIME_SLOT_TYPE_EMERGENCY
        DBGPRINTLN("\nsendList is empty, Emergency");
        tokenData.syncCMD.CMD = LORA_CMD_SYNC_RANDOM;
        memcpy(tokenData.syncCMD.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
        memcpy(tokenData.syncCMD.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
        tokenData.syncCMD.groupLen = emergencyTokenLen;
        memcpy(tokenData.syncCMD.groupAddr, hostAddr, LORA_MAC_ADDRESS_LEN - 1);
      }
      send_data(tokenData.u8Data);
      tokenStrategyloopCount++;
      //每10次或者 LORA_MAC_HOST_BROADCAST_TIME_LEN ，插入一个主机信息
      if ((tokenStrategyloopCount % LORA_MAC_HOST_BROADCAST_TIME_LEN) == 0) {
        tokenData.hostInfo.CMD = LORA_CMD_HOST_INFO_BROARDCAST;
        memcpy(tokenData.hostInfo.sourceAddr, hostAddr, LORA_MAC_ADDRESS_LEN);
        memcpy(tokenData.hostInfo.destAddr, LORA_MAC_BROADCAST_FFFF, LORA_MAC_ADDRESS_LEN);
        tokenData.hostInfo.devID = devID;
        //tokenData.hostInfo.MAID = devID;
        //tokenData.hostInfo.PID = devID;
        tokenData.hostInfo.maxCount = LoRaHostData.maxCount;
        tokenData.hostInfo.addrCount = LoRaHostData.addrCount;
        //tokenData.hostInfo.lastPos = LoRaHostData.addrCount;
        tokenData.hostInfo.addrBegin = LoRaHostData.addrBegin;
        send_data(tokenData.u8Data);

      }

    }
    ulTokenSlotTick = ulReset_interval();
  }
}



/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR LoRaMAC::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  short nCount;
  uint8_t bigBuff[CONST_LORA_MAC_FILE_BUFFLEN + 2];
  DBGPRINTLN("\n************** bLoad_config begin ****************");

  File configFile = SPIFFS.open(CONST_LORA_CONFIG_DATA_FILENAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s", CONST_LORA_CONFIG_DATA_FILENAME);
    return bRet;
  }

  nCount = 0;

  while (true)
  {
    nLen = configFile.readBytesUntil('\n', bigBuff, CONST_LORA_MAC_FILE_BUFFLEN);
    if (nLen <= 0)
      break;
    bigBuff[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bigBuff, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bigBuff, "wcha", 4) == 0) {
      preWorkchannel = atoi(spTemp);
      DBGPRINTF(" preWorkChannel:[%d]",preWorkchannel);
    }
    else if (memcmp(bigBuff, "ccha", 4) == 0) {
      configChannel = atoi(spTemp);
      DBGPRINTF("\n configChannel:[%d]",configChannel);
      bRet=true;
    }
    else if (memcmp(bigBuff, "BOOT", 4) == 0)
    {
      nRestartReason = atoi(spTemp);
      DBGPRINTF("\n BOOT:[%d]",nRestartReason);
    }
    bRet=true;

  }
  configFile.close();
  DBGPRINTLN("\n************** bLoad_config end ****************");
  return bRet;
}

/* save application data, return True = success */
bool ICACHE_FLASH_ATTR LoRaMAC::bSave_config()
{

  uint8_t bigBuff[CONST_LORA_MAC_FILE_BUFFLEN + 2];

  DBGPRINTLN("--save lora host data--");
  File configFile = SPIFFS.open(CONST_LORA_CONFIG_DATA_FILENAME, "w");
  if (!configFile)
  {
    DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_LORA_CONFIG_DATA_FILENAME, "w");
    if (!configFile)
    {
      DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }

  configFile.print("wcha:");
  configFile.println(workChannel);

  configFile.print("ccha:");
  configFile.println(configChannel);

  configFile.print("BOOT:");
  configFile.println(nRestartReason);

  configFile.close();
  DBGPRINTLN(" -- end");
  return true;
}


void LoRaMAC::debug_print_union(uint8_t *ptr)
{
  /*
    short CMD;
    stDataStreamUnion forPrint;
    memcpy(forPrint.u8Data, ptr, sizeof(stDataStreamUnion));

    CMD = forPrint.exCMD.CMD;
    //public Part
    switch (CMD)
    {
    case LORA_CMD_EXTEND_CMD:
      DBGPRINTLN("\n== LORA_CMD_EXTEND_CMD ==");
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_REGISTRATION_REQUEST ==");
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
    default:
      DBGPRINTLN("\n== application call ==");
      //application call to call external application API
      DBGPRINTF("\napp CMD: [%d]\n", CMD);
      break;
    }
    DBGPRINTF("Millis : [%d] ", millis());
    print_hex_data("SourceAddr:", forPrint.exCMD.sourceAddr, LORA_MAC_ADDRESS_LEN);
    print_hex_data("destAddr  :", forPrint.exCMD.destAddr, LORA_MAC_ADDRESS_LEN);
    DBGPRINTF("\nseq %d", forPrint.exCMD.seq);

    //private Part
    switch (CMD)
    {
    case LORA_CMD_EXTEND_CMD:
      DBGPRINTF("\nexCMD: %d", forPrint.exCMD.exCMD);
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      DBGPRINTF("\ndevID: %d, MAID: %d, PID: %d", forPrint.regRequest.devID, forPrint.regRequest.MAID, forPrint.regRequest.PID);
      print_hex_data("key:", forPrint.regRequest.key, LORA_MAC_KEY_LEN);
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINTF("\ndevID: %d", forPrint.regFeedback.devID);
      print_hex_data("addr:", forPrint.regFeedback.addr, LORA_MAC_ADDRESS_LEN);
      print_hex_data("val :", forPrint.regFeedback.val, LORA_MAC_KEY_MD5);
      break;
    case LORA_CMD_ADDR_REQUEST:
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      break;
    case LORA_CMD_SYNC_CMD:
      print_hex_data("groupAddr:", forPrint.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1);
      DBGPRINTF("groupLen:%d", forPrint.syncCMD.groupLen);
      print_hex_data("groupList:", forPrint.syncCMD.list, forPrint.syncCMD.groupLen);
      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINTLN("\n== LORA_CMD_SYNC_TIME ==");
      break;
    case LORA_CMD_SYNC_RANDOM:
      print_hex_data("groupAddr:", forPrint.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1);
      DBGPRINTF("groupLen:%d", forPrint.syncCMD.groupLen);
      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_REQUEST ==");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_FEEDBACK ==");
      break;
    default:
      DBGPRINTLN("\n== application call ==");
      //application call to call external application API
      break;
    }
    DBGPRINTLN();
  */
#ifdef DEBUG_SIO
  short CMD;
  char dispBuff[50];
  stDataStreamUnion forPrint;
  memcpy(forPrint.u8Data, ptr, sizeof(stDataStreamUnion));
  DBGPRINTF("\n{\"len\":\"%d\",", sizeof(stDataStreamUnion));
  CMD = forPrint.exCMD.CMD;
  //source addr
  convert_to_ip_format(dispBuff, forPrint.exCMD.sourceAddr, 4);
  DBGPRINTF("\"from\":\"%s\",", dispBuff);
  //dest addr
  convert_to_ip_format(dispBuff, forPrint.exCMD.destAddr, 4);
  DBGPRINTF("\"to\":\"%s\",", dispBuff);
  //chksum and seq
  DBGPRINTF("\"chksum\":\"%d\",\"seq\":\"%d\",", forPrint.exCMD.chksum, forPrint.exCMD.seq);

  //public Part
  switch (CMD)
  {
    case LORA_CMD_CONFIG_CHANNEL_BROARDCAST:
      DBGPRINT("\"type\":\"LORA_CMD_CONFIG_CHANNEL_BROARDCAST\",");
      break;
    case LORA_CMD_HOST_INFO_BROARDCAST:
      DBGPRINT("\"type\":\"LORA_CMD_HOST_INFO_BROARDCAST\",");
      break;
    case LORA_CMD_REGISTRATION_CLEANUP:
      DBGPRINT("\"type\":\"LORA_CMD_REGISTRATION_CLEANUP\",");
      break;
    case LORA_CMD_EXTEND_CMD:
      DBGPRINT("\"type\":\"LORA_CMD_EXTEND_CMD\",");
      DBGPRINTF("exCMD:\"%d\"", forPrint.exCMD.exCMD);
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      DBGPRINT("\"type\":\"LORA_CMD_REGISTRATION_REQUEST\",");
      DBGPRINTF("\"devID\":\"%d\", \"MAID\":\"%d\", \"PID\":\"%d\",", forPrint.regRequest.devID, forPrint.regRequest.MAID, forPrint.regRequest.PID);
      convert_to_hex_format(dispBuff, forPrint.regRequest.key, LORA_MAC_KEY_LEN);
      DBGPRINTF("\"key\":\"%s\",", dispBuff);
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINT("\"type\":\"LORA_CMD_REGISTRATION_FEEDBACK\",");
      DBGPRINTF("\"devID\":\"%d\",",  forPrint.regFeedback.devID);
      convert_to_ip_format(dispBuff, forPrint.regFeedback.addr, 4);
      DBGPRINTF("\"addr\":\"%s\",", dispBuff);
      convert_to_hex_format(dispBuff, forPrint.regFeedback.val, LORA_MAC_KEY_MD5);
      DBGPRINTF("\"val\":\"%s\",", dispBuff);
      break;
    case LORA_CMD_ADDR_REQUEST:
      DBGPRINT("\"type\":\"LORA_CMD_ADDR_REQUEST\",");
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      DBGPRINT("\"type\":\"LORA_CMD_ADDR_FEEDBACK\",");
      break;
    case LORA_CMD_SYNC_CMD:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_CMD\",");
      convert_to_ip_format(dispBuff, forPrint.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1);
      DBGPRINTF("\"groupAddr\":\"%s\",", dispBuff);
      DBGPRINTF("\"groupLen\":\"%d\",", forPrint.syncCMD.groupLen);
      convert_to_dec_format(dispBuff, forPrint.syncCMD.list, forPrint.syncCMD.groupLen);
      DBGPRINTF("\"groupList\":\"%s\",", dispBuff);
      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_TIME\",");
      break;
    case LORA_CMD_SYNC_RANDOM:
      DBGPRINT("\"type\":\"LORA_CMD_SYNC_RANDOM\",");
      convert_to_ip_format(dispBuff, forPrint.syncCMD.groupAddr, LORA_MAC_ADDRESS_LEN - 1);
      DBGPRINTF("\"groupAddr\":\"%s\",", dispBuff);
      DBGPRINTF("\"groupLen\":\"%d\",", forPrint.syncCMD.groupLen);
      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINT("\"type\":\"LORA_CMD_VERIFY_REQUEST\",");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINT("\"type\":\"LORA_CMD_VERIFY_FEEDBACK\",");
      break;
    default:
      DBGPRINT("\"type\":\"APPLICATION\",");
      //application call to call external application API
      DBGPRINTF("\"appCMD\":\"%d\"", CMD);
      break;
  }

  DBGPRINT("\"end\":\"\"}");
#endif

}



void LoRaMAC::debug_print_self()
{
  /*
    DBGPRINTLN("\n************** self information *************");
    DBGPRINTF(" devID: [%d]\n", devID);
    DBGPRINTF(" channel: [%d]\n", workChannel);
    DBGPRINTF(" hostFlag: [%d]\n", hostFlag);
    if (hostFlag) {
    LoRaHostData.debug_print_info();
    }
    DBGPRINTF(" seq: [%d]  chksum: [%d]", seq,chksum);
    print_hex_data(" hostAddr:", hostAddr, LORA_MAC_ADDRESS_LEN);
    print_hex_data(" hostMASK:", hostMASK, LORA_MAC_ADDRESS_LEN);
    print_hex_data(" meAddr  :", meAddr, LORA_MAC_ADDRESS_LEN);
    DBGPRINTF("\n sendWindowReady: [%d]\n", sendWindowReady);
    DBGPRINTF(" nextWindowTick: [%d]\n", nextWindowTick);
    DBGPRINTF(" nextWindowTimeSlotBegin: [%d]\n", nextWindowTimeSlotBegin);
    DBGPRINTF(" normalWindowPeriod: [%d]\n", normalWindowPeriod);
    DBGPRINTF(" emergencyWindowPeriod: [%d]\n", emergencyWindowPeriod);
    DBGPRINTF(" CMD: [%d]\n", CMD);
    DBGPRINTF(" freeheap:[%d]\n", ESP.getFreeHeap());
    DBGPRINTLN("*********************************************");
  */
  DBGPRINTF("\n SELF_INFO: devID:[%d] ", devID);
  DBGPRINTF("channel:[%d] ", workChannel);
  DBGPRINTF("hostFlag: [%d] ", hostFlag);
  DBGPRINTF("seq:[%d] chksum:[%d] ", seq, chksum);
  print_hex_data("hostAddr:", hostAddr, LORA_MAC_ADDRESS_LEN);
  print_hex_data("hostMASK:", hostMASK, LORA_MAC_ADDRESS_LEN);
  print_hex_data("meAddr  :", meAddr, LORA_MAC_ADDRESS_LEN);
  DBGPRINTF("sendWindowReady:[%d] ", sendWindowReady);
  DBGPRINTF("nextWindowTick:[%d] ", nextWindowTick);
  DBGPRINTF("nextWindowTimeSlotBegin:[%d] ", nextWindowTimeSlotBegin);
  DBGPRINTF("normalWindowPeriod:[%d] ", normalWindowPeriod);
  DBGPRINTF("emergencyWindowPeriod:[%d] ", emergencyWindowPeriod);
  DBGPRINTF("CMD:[%d]", CMD);
  DBGPRINTF("freeheap:[%d]", ESP.getFreeHeap());
  if (hostFlag) {
    LoRaHostData.debug_print_info();
  }
}




void ICACHE_FLASH_ATTR LoRaMAC::vSystem_restart(short reason)
{
  DBGPRINTF("\n system restart:[%d]", reason);
  nRestartReason = reason;
  bSave_config();
  delay(500);

  ESP.restart();
}

