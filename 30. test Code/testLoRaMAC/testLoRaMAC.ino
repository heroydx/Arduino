#include <arduino.h>

#include <LoRa_AS62.h>
#include <ListArray.h>

extern "C" {
#include "simpleHash.h"
}

void print_hex_data(char *title, uint8_t *ptr, short nLen)
{
  short i;
  DBGPRINTF("\n%s", title);
  for (i = 0; i < nLen; i++)
  {
    DBGPRINTF("%02X ", *(ptr + i));
  }
}


#define SIO_BAUD 115200
/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20170406
/*--------------------------------------------------------------------------------*/

LoRa_AS62 LoRaDev;

/*
   LoRaMac.h

   The definition of LoRa air protocol CMD list

   2017  Steven Lian (steven.lian@gmail.com)

*/

#define LORA_MAC_HOST_FLAG false

//common cmd list
#define LORA_CMD_NULL_CMD 0 //空命令,无需解释和执行
#define LORA_CMD_EXTEND_CMD 1    //扩展命令,用于避免一个字节的命令类型不够的情况
#define LORA_CMD_REGISTRATION_REQUEST  2   //服务登记请求，一个Terminal向router申请加入LoRa网络,同时会上传自己的唯一号 devID(IMEI),MAID(制造商代号)和PID(项目代号)
#define LORA_CMD_REGISTRATION_FEEDBACK  3   //服务登记回馈，一个Terminal向router申请加入LoRa网络,同时会下发内部地址 
#define LORA_CMD_ADDR_REQUEST 4   //内部地址分配请求,一般是一个Terminal加入一个LoRa后,需要新分配或者获取已经分配的内部地址。
#define LORA_CMD_ADDR_FEEDBACK 5   //内部地址分配回馈,返回新分配或者获取已经分配的内部地址。
#define LORA_CMD_SYNC_CMD 6  //router 同步,Terminal发送的开始时间,以及,分组和顺序
#define LORA_CMD_SYNC_TIME 7   // router 同步时间
#define LORA_CMD_SYNC_RANDOM 8  //router 给出自由time slot,等需要发送设备请求，注册等信息的设备。 
#define LORA_CMD_VERIFY_REQUEST 9 //要求验证对方设备的真伪，需要发送一组随机数,至少是8个字节的数据,
#define LORA_CMD_VERIFY_FEEDBACK 10 //验证反馈,回送对方8个字节的数据,和对应的MD5或者其他方式的hash结果,结果不少于8个字节。
#define LORA_CMD_DHCP_POOL_REQUEST 10 //host 主设备向上级服务器申请一个内部地址池
#define LORA_CMD_DHCP_POOL_FEEDBACK 11 //上级服务器对 host 主设备向申请一个内部地址池请求的回应,给一个段落(开始和结束)
#define LORA_CMD_ADDR_CHANGE_ACK 12 //对特定设备内部地址变更的通知,是对LORA_CMD_ADDR_REQUEST/LORA_CMD_ADDR_FEEDBACK的增强,格式和LORA_CMD_ADDR_FEEDBACK相同

//application cmd list
#define LORA_CMD_APP_EXTEND_CMD 64 //应用扩展命令集合
#define LORA_CMD_APP_FMRADIO 65 //FM Radion APPLICATION CMD 

#define LORA_MAC_ADDRESS_LEN 4
#define LORA_MAC_KEY_LEN 8
#define LORA_MAC_KEY_MD5 8
#define LORA_MAC_SYNC_LIST_MAX_LEN 8

#define LORA_MAC_APPLICATION_MAX_DATA_LEN 16
#define LORA_MAC_UNION_MAX_DATA_LEN 28

#define LORA_MAC_TIME_SLOT_IN_MS 200 // 200 ms 根据 LORA_MAC_UNION_MAX_DATA_LEN 计算
#define LORA_MAC_TIME_SOLT_OFFSET_IN_MS 5 //第一个发送槽在多少时间后开始

//#define LORA_MAC_BROADCAST_0000 0x00000000

#define LORA_MAC_DEFAULT_HOST_ADDR "\xC0\xA8\x00\x01"
uint8_t LORA_MAC_BROADCAST_FFFF[LORA_MAC_ADDRESS_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};

#define LORA_MAC_SEND_BUFF_MAX_LENGTH 16
#define LORA_MAC_RECV_BUFF_MAX_LENGTH 5

#define LORA_MAC_HOST_DATA_LENGTH 248
#define LORA_MAC_HOST_DATA_ADDR_BEGIN 5

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  short exCMD;
} stDataCMD_00;

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  unsigned long devID;
  unsigned short MAID;
  unsigned short PID;
  uint8_t key[LORA_MAC_KEY_LEN];
  //uint8_t val[LORA_MAC_KEY_MD5];
} stDataCMD_RegistrationRequest;

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  unsigned long devID;
  uint8_t addr[LORA_MAC_ADDRESS_LEN];
  uint8_t val[LORA_MAC_KEY_MD5];
} stDataCMD_RegistrationFeedback;


typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  uint8_t groupAddr[LORA_MAC_ADDRESS_LEN - 1];
  uint8_t groupLen;
  uint8_t list[LORA_MAC_SYNC_LIST_MAX_LEN];
} stDataCMD_synCMD;


typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  short year;
  short month;
  short day;
  short hour;
  short minute;
  short second;
  short wday;
  short timezone2; //2 x timezone , consider india GMT +7.5= 15
} stDataCMD_synTime;

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  unsigned long devID;
  uint8_t key[LORA_MAC_KEY_LEN];
} stDataCMD_verifyRequestCMD;

typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  unsigned long devID;
  uint8_t val[LORA_MAC_KEY_MD5];
} stDataCMD_verifyFeedbackCMD;


typedef struct {
  uint8_t sourceAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
  uint8_t seq;
  uint8_t fill;
  short CMD;
  uint8_t data[LORA_MAC_APPLICATION_MAX_DATA_LEN];
} stDataCMD_APP;


typedef union {
  stDataCMD_00 exCMD;
  stDataCMD_RegistrationRequest regRequest;
  stDataCMD_RegistrationFeedback regFeedback;
  stDataCMD_synCMD syncCMD;
  stDataCMD_synTime syncTime;
  stDataCMD_verifyRequestCMD verifyRegCMD;
  stDataCMD_verifyFeedbackCMD verifyFeedCMD;
  stDataCMD_APP app;
  uint8_t u8Data[LORA_MAC_UNION_MAX_DATA_LEN];
} stDataStreamUnion;


#define CONST_LORA_HOST_DEFAULT_BATCH_LEN 8

class LoRaHost {
  public:
    //data
    uint8_t hostAddr[LORA_MAC_ADDRESS_LEN];
    uint8_t resultAddr[LORA_MAC_ADDRESS_LEN];
    uint8_t addrBegin;
    short maxCount;
    short addrCount;
    unsigned long *addrListPtr;
    uint8_t *seqListPtr;
    short currPos;
    short batchLen;

    //function
    LoRaHost();
    virtual ~LoRaHost();

    short begin(uint8_t *Addr, uint8_t beginAddr, short subNum);
    bool isEmpty();
    bool isFull();
    short find(unsigned long devID);
    short find(unsigned long devID, uint8_t *addr );
    short add(unsigned long devID, uint8_t *addr);
    short del(unsigned long devID);
    void debug_print_info();
  private:
    short stat();
};


LoRaHost::LoRaHost()
{
  //清空hostData数据
  memset(hostAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));
  memset(resultAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));

  addrBegin = LORA_MAC_HOST_DATA_ADDR_BEGIN;
  maxCount = 0;
  addrCount = 0;
  currPos = 0;
  batchLen = CONST_LORA_HOST_DEFAULT_BATCH_LEN;
  addrListPtr = NULL;
  seqListPtr = NULL;
}

LoRaHost::~LoRaHost()
{
  //清空hostData数据
  memset(hostAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));
  memset(resultAddr, 0, sizeof(LORA_MAC_ADDRESS_LEN));

  addrBegin = LORA_MAC_HOST_DATA_ADDR_BEGIN;
  maxCount = 0;
  addrCount = 0;
  currPos = 0;
  batchLen = CONST_LORA_HOST_DEFAULT_BATCH_LEN;
  //free memory
  if (addrListPtr != NULL) {
    free (addrListPtr);
  }
  if (seqListPtr != NULL) {
    free (seqListPtr);
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
  if (seqListPtr != NULL) {
    free (seqListPtr);
  }
  int sizOfMemory;
  sizOfMemory = sizeof (unsigned long) * maxCount;
  addrListPtr = (unsigned long *) malloc (sizOfMemory);
  memset(addrListPtr, 0, sizOfMemory);
  sizOfMemory = sizeof (uint8_t) * maxCount;
  seqListPtr = (uint8_t *) malloc (sizOfMemory);
  memset(addrListPtr, 0, sizOfMemory);
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


short LoRaHost::add(unsigned long devID, uint8_t *addr)
{
  short ret = -1;
  short i;
  if (find(devID) < 0) {
    for (i = 0; i < maxCount; i++)  {
      if (*(addrListPtr + i) == 0L) {
        uint8_t nT1;
        *(addrListPtr + i) = devID;
        memcpy(addr, hostAddr, LORA_MAC_ADDRESS_LEN - 1);
        nT1 = (uint8_t)  (addrBegin + i);
        *(addr + LORA_MAC_ADDRESS_LEN - 1) = nT1;
        addrCount++;
        ret = i;
        break;
      }
    }
  }
  return ret;
}

short LoRaHost::del(unsigned long devID)
{
  short ret = -1;
  if (find(devID) >= 0) {
    *(addrListPtr + ret) = 0L;
    addrCount--;
  }
  return ret;
}

short LoRaHost::stat()
{
  short ret = 0;
  short i;
  for (i = 0; i < maxCount; i++)  {
    if (*(addrListPtr + ret) != 0L) {
      ret++;
    }
  }
  return ret;
}


void LoRaHost::debug_print_info()
{
  DBGPRINTLN("\n==** Host Data info ==**");
  DBGPRINTF("\n maxCount: [%d]", maxCount);
  DBGPRINTF("\n addrCount: [%d]", addrCount);
  DBGPRINTF("\n addrBegin: [%d]", addrBegin);
}


LoRaHost LoRaHostData;

extern short cmd_application_call(short CMD, uint8_t *ptr, short nLen);

ListArray sendList(LORA_MAC_SEND_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));

class LoRaMAC
{
  public:
    //data
    unsigned long devID;
    uint8_t hostAddr[LORA_MAC_ADDRESS_LEN];//主机地址
    uint8_t hostMASK[LORA_MAC_ADDRESS_LEN]; //
    uint8_t meAddr[LORA_MAC_ADDRESS_LEN]; //本机地址
    uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
    uint8_t key[LORA_MAC_KEY_LEN];
    bool hostFlag;
    uint8_t seq;
    uint8_t synced;
    short channel;
    bool sendWindowReady;
    unsigned long nextWindowTick;
    unsigned long nextWindowTimeSlotBegin;
    unsigned long normalWindowPeriod;
    unsigned long normalWindowTick;
    unsigned long emergencyWindowPeriod;
    unsigned long emergencyWindowTick;
    short  CMD;
    stDataStreamUnion recvData;
    stDataStreamUnion sendData;
    stDataStreamUnion emergencyData;

    //function
    LoRaMAC();
    //LoRaMAC(bool isHost, int subNum);
    short begin(bool isHost);
    short begin(bool isHost, uint8_t *addr);
    short begin(bool isHost, uint8_t *addr, short subNum);
    short begin(bool isHost, uint8_t *addr, uint8_t beginAddr, short subNum );
    virtual ~LoRaMAC();
    short available();
    short check_time_window_send();
    short handle_received_data_automatic();
    short send_data(uint8_t *u8Ptr);
    short send_data(uint8_t *u8Ptr, short nLen);

    //debug print function
    void debug_print_union(uint8_t *ptr);
    void debug_print_self();

  private:
    //data
    //ListArray sendList(LORA_MAC_SEND_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));
    //ListArray sendList();

    //function
    unsigned long generate_devID();
    bool hash_verification_process(char *ptr, char *result);  //to verify host and client
    short address_me(uint8_t *addrPtr); // return 0 to me address, 4,3,2,1 broadcast address,;
    short func_cmd_extend_cmd();
    short func_cmd_registration_request();
    short func_cmd_registration_feedback();
    short func_cmd_addr_request();
    short func_cmd_addr_feedback();
    short func_cmd_sync_cmd();
    short func_cmd_sync_time();
    short func_cmd_sync_random();
    short func_cmd_verify_request();
    bool func_cmd_verify_feedback();
    unsigned long cal_host_time_slot();//计算主控机器(hostFlag==true)的发射时间窗口,默认利用发送数据(sendData)里面的参数计算。
};

LoRaMAC::LoRaMAC()
{
  memcpy(hostAddr, LORA_MAC_DEFAULT_HOST_ADDR, LORA_MAC_ADDRESS_LEN);
  begin(false, hostAddr, 0);
}

short LoRaMAC::begin(bool isHost, uint8_t *addr, uint8_t beginAddr, short subNum )
{
  short ret = 0;
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(micros());
  devID = generate_devID();
  hostFlag = isHost;

  memcpy(hostAddr, addr, LORA_MAC_ADDRESS_LEN);

  normalWindowTick = ulReset_interval();
  emergencyWindowTick = ulReset_interval();
  if (hostFlag && (subNum > 0)) {
    LoRaHostData.begin(hostAddr, beginAddr, subNum);
  }
}

short LoRaMAC::begin(bool isHost, uint8_t *addr, short subNum)
{
  return begin(isHost, addr, LORA_MAC_HOST_DATA_ADDR_BEGIN, subNum );
}


short LoRaMAC::begin(bool isHost, uint8_t *addr)
{
  return begin(isHost, addr, LORA_MAC_HOST_DATA_ADDR_BEGIN, LORA_MAC_HOST_DATA_LENGTH );
}


short LoRaMAC::begin(bool isHost)
{
  return begin(isHost, (uint8_t *)LORA_MAC_DEFAULT_HOST_ADDR, LORA_MAC_HOST_DATA_ADDR_BEGIN, LORA_MAC_HOST_DATA_LENGTH );
}


LoRaMAC::~LoRaMAC()
{

}

unsigned long LoRaMAC::generate_devID()
{
  unsigned long devID;
  devID = ESP.getChipId();
  return devID;
}

bool LoRaMAC::hash_verification_process(char *ptr, char *result)
{
  return true;
}

//计算主控机器(hostFlag==true)的发射时间窗口,默认利用发送数据(sendData)里面的参数计算。
unsigned long LoRaMAC::cal_host_time_slot() {
  unsigned long ret;
  short CMD;
  DBGPRINTLN("===*** cal_host_time_slot ***===");
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

short LoRaMAC::check_time_window_send()
{
  short ret = 0;
  unsigned long tick;
  tick = ulGet_interval(nextWindowTick);
  if (sendWindowReady) {
    bool timeSlotOpen = true;
    //send window is ready
    //DBGPRINTF("\n--** sendWindowReady:[%d] **--\n", sendWindowReady);
    if (!hostFlag) {
      //主控设备和非主控设备对时间窗口的判断不同
      //非主控设备,只能按照主控设备规定的发送时间窗口
      if (tick < nextWindowTimeSlotBegin ) {
        //窗口时间没有到
        timeSlotOpen = false;
      }
      //非主控设备,超出给定窗口时间,此次发送窗口就关闭了
      if (tick > (nextWindowTimeSlotBegin + LORA_MAC_TIME_SLOT_IN_MS)) {
        //窗口时间过了
        timeSlotOpen = false;
        sendWindowReady = false;
      }
    }
    //二者公用代码
    if (timeSlotOpen) {
      //send windows is avaiable
      if  (sendList.count() > 0) {
        if (!LoRaDev.busy()) {
          //loRaDevice is not busy,so pop data in sendList
          sendList.rpop(sendData.u8Data);
          //填充顺序号
          sendData.syncCMD.seq = seq++;
          //send data via lora
          DBGPRINTLN("===**==== data sent begin ===**===");
          debug_print_union(sendData.u8Data);
          ret = LoRaDev.send(sendData.u8Data, sizeof (stDataStreamUnion));
          DBGPRINTLN("===**==== data sent end ===**===");
          //sent,disable windows
          sendWindowReady = false;
          if (hostFlag) {
            //如果是主控设备,需要计算自己的下一个时间窗口.
            nextWindowTimeSlotBegin = cal_host_time_slot();
          }
        }
        //close the send window,waiting for next time
      }
    }
  }

  if (hostFlag) {
    //如果是主控设备,只要是等待发送时间超过,并清除等待时间,就可以打开发送窗口
    if (tick > (nextWindowTimeSlotBegin )) {
      sendWindowReady = true;
      nextWindowTimeSlotBegin = 5;//给个5ms过度时间
    }
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


short LoRaMAC::func_cmd_extend_cmd()
{
  ;
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
      memcpy(sendData.regFeedback.val, recvData.regRequest.key, LORA_MAC_KEY_MD5);
      debug_print_union(sendData.u8Data);
      send_data(sendData.u8Data);
    }
  }
  return ret;
}

short LoRaMAC::func_cmd_registration_feedback()
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

  }
  return ret;
}

short LoRaMAC::func_cmd_addr_request()
{
  ;
}

// got feedback from host
short LoRaMAC::func_cmd_addr_feedback()
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

//自动处理有关注册登记,时间窗口,校验等信息
short LoRaMAC::handle_received_data_automatic()
{
  short ret = 0;
  short CMD;
  DBGPRINTLN("===*** handle_received_data_automatic ***===");
  CMD = recvData.exCMD.CMD;
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      func_cmd_extend_cmd();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      if (hostFlag) {
        func_cmd_registration_request();
        ret = sizeof (stDataStreamUnion);
      }
      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      func_cmd_registration_feedback();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_ADDR_REQUEST:
      func_cmd_addr_request();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      func_cmd_addr_feedback();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_SYNC_CMD:
      if (!hostFlag) {
        //非主控设备需要获得时间窗口
        func_cmd_sync_cmd();
      }
      break;
    case LORA_CMD_SYNC_TIME:
      func_cmd_sync_time();
      break;
    case LORA_CMD_SYNC_RANDOM:
      if (!hostFlag) {
        //非主控设备需要获得时间窗口
        func_cmd_sync_random();
      }
      break;
    case LORA_CMD_VERIFY_REQUEST:
      func_cmd_verify_request();
      ret = sizeof (stDataStreamUnion);
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      func_cmd_verify_feedback();
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

//检查是否收到数据，并自动执行部分无需上层处理的操作,例如时间窗口处理,收到的数据存储在recvData结构里面
short LoRaMAC::available()
{
  short ret = 0;
  short nLen;
  nLen = LoRaDev.available();
  nLen = abs(nLen); //nLen <0, crc8 error,
  if (nLen > 0) {
    LoRaDev.get(recvData.u8Data); // 必须使用recvData.u8Data,后面会默认按这个数据地址处理
    debug_print_union(recvData.u8Data);
    ret = handle_received_data_automatic();
  }
  check_time_window_send();
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


void LoRaMAC::debug_print_union(uint8_t *ptr)
{
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
}

void LoRaMAC::debug_print_self()
{
  DBGPRINTLN("\n************** self information *************");
  DBGPRINTF(" devID: [%d]\n", devID);
  DBGPRINTF(" channel: [%d]\n", channel);
  DBGPRINTF(" hostFlag: [%d]\n", hostFlag);
  DBGPRINTF(" seq: [%d]", seq);
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
}


#define TEST_MODE_LEN 11
stDataStreamUnion toTest[TEST_MODE_LEN];
stDataStreamUnion toRecv;

unsigned long ulDevID = 12345678L;
uint8_t u8SeedKey[LORA_MAC_KEY_LEN];
uint8_t u8SeedVal[LORA_MAC_KEY_MD5];


void prepare_test_data()
{
  uint8_t i;
  //ulDevID = generate_devID();
  ulDevID = ESP.getChipId();
  random_string((char *) u8SeedKey, LORA_MAC_KEY_LEN);
  simpleHash((char *) u8SeedKey, (char *)  u8SeedVal, ulDevID, LORA_MAC_KEY_LEN);
  for (i = 0; i < TEST_MODE_LEN; i++) {
    //DBGPRINTLN(i);
    switch (i)
    {
      case LORA_CMD_EXTEND_CMD:
        toTest[i].exCMD.CMD = LORA_CMD_EXTEND_CMD;
        memcpy(toTest[i].exCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].exCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].exCMD.seq = (uint8_t) i;
        toTest[i].exCMD.exCMD = 1;
        break;
      case LORA_CMD_REGISTRATION_REQUEST:
        toTest[i].regRequest.CMD = LORA_CMD_REGISTRATION_REQUEST;
        memcpy(toTest[i].regRequest.sourceAddr, "\x00\x00\x00\x00", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regRequest.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].regRequest.seq = (uint8_t) i;
        toTest[i].regRequest.devID = ulDevID;
        toTest[i].regRequest.MAID = 8;
        toTest[i].regRequest.PID = 1;
        memcpy(toTest[i].regRequest.key, u8SeedKey, LORA_MAC_KEY_LEN);

        break;
      case LORA_CMD_REGISTRATION_FEEDBACK:
        toTest[i].regFeedback.CMD = LORA_CMD_REGISTRATION_FEEDBACK;
        memcpy(toTest[i].regFeedback.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regFeedback.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].regFeedback.seq = (uint8_t) i;
        toTest[i].regFeedback.devID = ulDevID;
        memcpy(toTest[i].regFeedback.addr, "\xC0\xA8\x00\x02", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regFeedback.val, u8SeedVal, LORA_MAC_KEY_MD5);
        break;
      case LORA_CMD_ADDR_REQUEST:
        break;
      case LORA_CMD_ADDR_FEEDBACK:
        break;
      case LORA_CMD_SYNC_CMD:
        toTest[i].syncCMD.CMD = LORA_CMD_SYNC_CMD;
        memcpy(toTest[i].syncCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].syncCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].syncCMD.seq = (uint8_t) i;
        memcpy(toTest[i].syncCMD.groupAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN - 1);
        toTest[i].syncCMD.groupLen = 8;
        for (short k = 1; k < toTest[i].syncCMD.groupLen + 1; k++) {
          toTest[i].syncCMD.list[k] = k;
        }
        break;
      case LORA_CMD_SYNC_TIME:
        break;
      case LORA_CMD_SYNC_RANDOM:
        toTest[i].syncCMD.CMD = LORA_CMD_SYNC_RANDOM;
        memcpy(toTest[i].syncCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].syncCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].syncCMD.seq = (uint8_t) i;
        toTest[i].syncCMD.groupLen = 16;
        memcpy(toTest[i].syncCMD.groupAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN - 1);
        break;
      case LORA_CMD_VERIFY_REQUEST:
        break;
      case LORA_CMD_VERIFY_FEEDBACK:
        break;
      default:
        break;
    }
  }
}


#define TEST_BUFF_LEN 100
uint8_t au8Buff[TEST_BUFF_LEN];
short au8BuffLen = 0;
uint8_t gDataBuff[CONST_LORA_COMMUNICATION_BUFFSIZE * 2 + 2];
uint8_t gDataBuffLen = 0;

char testSendData[5][40] = {
  "HELLO WORLD",
  "01234567890",
  "ABCDEFGHIJKLMN\n",
  "abcdefghijklmnopqrstuvwxyz\n",
  "AAABBBCCCDDD\n",
};


LoRaMAC LoRaMacTest;

void setup()
{
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  // Open serial communications and wait for port to open:
#ifdef DEBUG_SIO
  DEBUG_SIO.begin(SIO_BAUD);
  while (!DEBUG_SIO) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  DBGPRINTLN();
  DBGPRINTLN();
  DBGPRINTLN("Serial Port for debug is ready on : ");
  DBGPRINTLN(SIO_BAUD);
#endif

  DBGPRINTLN("---- the size of dataStream ----");
  DBGPRINTF(" % d\n", sizeof(stDataStreamUnion));

  DBGPRINTLN("---- the size of stDataCMD_APP ----");
  DBGPRINTF(" % d\n", sizeof(stDataCMD_APP));

  DBGPRINTLN("\nPrepare test data");
  prepare_test_data();
  DBGPRINTLN("\nprint test data");
  for (short k = 0; k < TEST_MODE_LEN; k++) {
    DBGPRINTF("\nNo. %d", k);
    LoRaMacTest.debug_print_union(toTest[k].u8Data);
  }

  // set the data rate for the SoftwareSerial port
  LoRaDev.begin();
  if (LoRaDev.isReady()) {
    Serial.println();
    LoRaMacTest.channel = LoRaDev.sendChannel;
    Serial.println("AS62 is ready for communication");
  }
  LoRaMacTest.begin(true);
  LoRaMacTest.debug_print_self();
}

char serialRecvBuf[10];
short serialRecvBufLen = 0;

void test_handle_recv_data()
{
  short CMD;
  short nLen;
  nLen = sizeof (stDataStreamUnion);

  CMD = toRecv.exCMD.CMD;
  //public Part
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      break;
      DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

      DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
      toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.devID = toRecv.regRequest.devID;
      simpleHash((char *) toRecv.regRequest.key, (char *)  toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.val, toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.devID, LORA_MAC_KEY_LEN);

      //LoRaMacTest.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
      LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);
      LoRaMacTest.debug_print_union(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);

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
}

void loop() // run over and over
{
  if (Serial.available()) {
    uint8_t chR;
    short nLen;
    short nNum = 0;
    chR = Serial.read();
    switch (chR)
    {
      case '1':
        nNum = 0;
        break;
      case '2':
        nNum = 1;
        break;
      case '3':
        nNum = 2;
        break;
      case '4':
        nNum = 3;
        break;
      case '5':
        nNum = 4;
        break;
      case '6':
        nNum = 5;
        break;
      case '7':
        nNum = 6;
        break;
      case '8':
        nNum = 7;
        break;
      case '9':
        nNum = 8;
        break;
      case 'a':
        nNum = 9;
      case 'b':
        nNum = 10;
        break;
      case 'h':
        //send_data
        LoRaMacTest.hostFlag = true;
        if (LoRaMacTest.hostFlag) {
          DBGPRINTF("\n simulate host [%d]", LoRaMacTest.hostFlag);
        }
        else {
          DBGPRINTF("\n simulate non host [%d]", LoRaMacTest.hostFlag);
        }
        nNum = -2;
        break;
      case 'n':
        //send_data
        LoRaMacTest.hostFlag = false;
        if (LoRaMacTest.hostFlag) {
          DBGPRINTF("\n simulate host [%d]", LoRaMacTest.hostFlag);
        }
        else {
          DBGPRINTF("\n simulate non host [%d]", LoRaMacTest.hostFlag);
        }
        nNum = -3;
        break;
      case 's':
        //send_data
        DBGPRINTF("\n LoRaMacTest.send_data [%d]", LoRaMacTest.send_data(toTest[LORA_CMD_EXTEND_CMD].u8Data));
        nNum = -101;
        break;
      case 't':
        //send_data
        memcpy(LoRaMacTest.emergencyData.u8Data, toTest[LORA_CMD_EXTEND_CMD].u8Data, sizeof (stDataStreamUnion));
        DBGPRINTLN("\n emergencyData:");
        LoRaMacTest.debug_print_union(LoRaMacTest.emergencyData.u8Data);
        nNum = -102;
        break;
      default:
        nNum = -1;
        break;
    }
    if (nNum >= 0) {
      while (LoRaDev.busy())
      { ;
      }
      nLen = sizeof (stDataStreamUnion);
      DBGPRINTLN("\n--------- will send begin ---------");
      LoRaMacTest.debug_print_union(toTest[nNum].u8Data);
      LoRaDev.send(toTest[nNum].u8Data, nLen);
      //LoRaMacTest.send(toTest[nNum].u8Data, nLen);
      DBGPRINTLN("--------- will send end ---------\n");
    }
  }
  short nLen;
  //nLen = LoRaDev.available();
  nLen = LoRaMacTest.available();
  if (nLen) {
    if (nLen < 0) {
      Serial.printf("\nCRC ERROR : % d\n", nLen);
    }
    nLen = abs(nLen);
    LoRaDev.get(toRecv.u8Data);
    LoRaMacTest.debug_print_union(toRecv.u8Data);
    Serial.println("\n--LoRa received Data");
    Serial.println(nLen);
    test_handle_recv_data();
    LoRaMacTest.debug_print_self();
  }
}

