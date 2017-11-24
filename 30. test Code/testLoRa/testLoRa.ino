#include <arduino.h>
#include <string.h>



void vReset_interval(unsigned long & resetTick)
{
  resetTick = millis();
}

unsigned long ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}



extern "C" {
  //#include "AppleBase64.h"
#include "slipcrc8.h"
}
#define DEBUG_SIO



//Lora Class and releated data
//#include <SoftwareSerial.h>
#include <ESPSoftwareSerial.h>
//SoftwareSerial SerialPort(10, 11); // RX D10, TX D11 Nano
SoftwareSerial SerialPort(4, 2); // RX, TX 8266 GPIO 4,2
//SoftwareSerial LORASerial(19, 20); // RX, TX 8266 GPIO 4,5

//Nano
//#define CONST_LORA_AS62_M0_PORT 14
//#define CONST_LORA_AS62_M1_PORT 15
//ESP
#define CONST_LORA_AS62_M0_PORT 16
#define CONST_LORA_AS62_M1_PORT 5
#define CONST_LORA_AS62_AUX_PORT 12

//AS62 communication mode
#define CONST_LORA_AS62_COMM_MODE_NORMAL 0
#define CONST_LORA_AS62_COMM_MODE_WAKEUP 1
#define CONST_LORA_AS62_COMM_MODE_SAVING 2
#define CONST_LORA_AS62_COMM_MODE_SLEEP 3


#define LORA_Serial LORASerial
#define CONST_LORA_DEVICE_TYPE_AS62 1
#define CONST_LORA_COMMUICATION_MAX_DATALEN 28
#define CONST_LORA_COMMUNICATION_BUFFSIZE CONST_LORA_COMMUICATION_MAX_DATALEN*2+4
#define CONST_LORA_COMMUNICATION_RECV_BUFF_COUNT 5
//#define CONST_LORA_COMMUNICATION_BEGIN_CHAR '\r'
//#define CONST_LORA_COMMUNICATION_END_CHAR '\n'
#define CONST_LORA_COMMUNICATION_BEGIN_CHAR '$'
#define CONST_LORA_COMMUNICATION_END_CHAR '#'



#define CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN 6
#define CONST_LORA_AS62_SIO_BAUD_RATE_NORMAL 9600
#define CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT 9600
#define CONST_LORA_AS62_AIR_BAUD_RATE 2400

#define CONST_LORA_AS62_AIR_DELAY_TIME (1000/(CONST_LORA_AS62_AIR_BAUD_RATE/10))

typedef struct {
  short num;
  short head;
  short tail;
  uint8_t data[CONST_LORA_COMMUNICATION_RECV_BUFF_COUNT][CONST_LORA_COMMUNICATION_BUFFSIZE + 2];
} hashRecvSt;


uint8_t gDataBuff[CONST_LORA_COMMUNICATION_BUFFSIZE * 2 + 2];
uint8_t gDataBuffLen = 0;

class LoRa_AS62
{
  public:
    //data
    bool isReady;
    uint8_t deviceType;// Lora device type; default is AS62
    short sendChannel;//send channel
    short recvChannel;//received channel
    uint8_t sendBuf[CONST_LORA_COMMUNICATION_BUFFSIZE + 2];
    short sendLen;
    uint8_t recvBuf[CONST_LORA_COMMUNICATION_BUFFSIZE + 2];
    short recvLen;
    short recvReady;

    hashRecvSt readyData;

    short loraBandRate;
    unsigned short address;
    //function
    LoRa_AS62();
    virtual ~LoRa_AS62();

    short begin();
    short set(short channel, short rate);
    short available();
    uint8_t busy();
    short send(uint8_t *u8Ptr, short nLen);
    short get(uint8_t *u8Ptr);
    short setConfig();

    short readConfig(uint8_t *u8Ptr, short delayTime);
    void reset(short delayTime);
    short setConfig(short delayTime);

  private:
    uint8_t u8Busy;
    unsigned long airSendTime;
    unsigned long gulBusyTick;
    uint8_t recvCHAN;
    uint8_t sendCHAN;
    uint8_t M0Port;
    uint8_t M1Port;
    // HEAD,ADDH,ADDL,SPEED,CHAN,OPTION
    uint8_t configData[CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN];
    void setCommMode(uint8_t mode);
    uint8_t getSEED(short SIORate, short AirRate);
    short readConfig(uint8_t *u8Ptr);
    void reset();



};

#define CONST_LORA_AS62_DEFAULT_CHANNEL 425
#define CONST_LORA_AS62_BAND_433_LOW 410
#define CONST_LORA_AS62_BAND_433_HIGH 441

uint8_t LoRa_AS62::getSEED(short SIORate, short AirRate)
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


LoRa_AS62::LoRa_AS62()
{
  isReady = false;
  deviceType = CONST_LORA_DEVICE_TYPE_AS62;
  sendChannel = CONST_LORA_AS62_DEFAULT_CHANNEL;
  recvChannel = CONST_LORA_AS62_DEFAULT_CHANNEL;
  if ((sendChannel >= CONST_LORA_AS62_BAND_433_LOW) && (sendChannel <= CONST_LORA_AS62_BAND_433_HIGH)) {
    sendCHAN = (uint8_t) (sendChannel - CONST_LORA_AS62_BAND_433_LOW);
    recvCHAN = (uint8_t) (recvChannel - CONST_LORA_AS62_BAND_433_LOW);
  }
  //clear data
  memset(sendBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE + 2);
  memset(recvBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE + 2);
  memset((void *) & readyData, 0, sizeof(readyData));

  u8Busy = 0;

  //set communication rate
  loraBandRate = CONST_LORA_AS62_AIR_BAUD_RATE;
  SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  //config data
  address = 0xFFFF;
  configData[0] = 0xc0; //HEAD, C0：所设置的参数会掉电保存。C2：所设置的参数不会掉电保存。
  configData[1] = 0xFF; // 模块地址高字节
  configData[2] = 0xFF; //模块地址低字节
  configData[1] = 0x12; // 模块地址高字节
  configData[2] = 0x34; //模块地址低字节
  configData[3] = getSEED(CONST_LORA_AS62_SIO_BAUD_RATE_NORMAL, CONST_LORA_AS62_AIR_BAUD_RATE);
  configData[4] = sendCHAN;//通信频率（ 通信频率（ 410 M + CHAN * 1M）（默认 17H ：433M ）
  configData[5] = 0x44;//定点传输+上拉输入+250ms唤醒时间+FEC开+27dbm发射功率(7/6/543/2/10)

  //set config port working mode
  M0Port = CONST_LORA_AS62_M0_PORT;
  M1Port = CONST_LORA_AS62_M1_PORT;
  pinMode(M0Port, OUTPUT);
  pinMode(M1Port, OUTPUT);
}


LoRa_AS62::~LoRa_AS62()
{
  isReady=false;
}


uint8_t LoRa_AS62::busy()
{
  if (ulGet_interval(gulBusyTick) > airSendTime) {
    u8Busy = 0;
    airSendTime = 0;
    vReset_interval(gulBusyTick);
  }
  return u8Busy;
}


void LoRa_AS62::reset()
{
  short ret = 0;
  SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  setCommMode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  //
  //send query config
#ifdef DEBUG_SIO
  Serial.println("send 0xc4 0xc4 0xc4");
#endif
  SerialPort.write(0xc4);
  SerialPort.write(0xc4);
  SerialPort.write(0xc4);
  delay(200);
  setCommMode(CONST_LORA_AS62_COMM_MODE_NORMAL);
}

short LoRa_AS62::readConfig(uint8_t *u8Ptr)
{
  short ret = 0;
  //SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  setCommMode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  //
  //send query config
#ifdef DEBUG_SIO
  Serial.println("send 0xc1 0xc1 0xc1");
#endif
  SerialPort.write(0xc1);
  SerialPort.write(0xc1);
  SerialPort.write(0xc1);
  delay(150);
  //wait for 1 seconds to get data;
  unsigned long ulTick;
  vReset_interval(ulTick);
  short nLen = 0;
#ifdef DEBUG_SIO
  Serial.println("wait for receive:");
#endif

  while (ulGet_interval(ulTick) < 1000) {
    if (SerialPort.available()) {
      uint8_t chR;
      chR = SerialPort.read();
      if (chR == 0xC0 || chR == 0xC2) {
        nLen = 0;
      }
      u8Ptr[nLen++] = chR;
#ifdef DEBUG_SIO
      //debug
      Serial.printf("%02X ", chR);
#endif
    }
    if (nLen == 6)
      break;
    if (nLen > CONST_LORA_COMMUNICATION_BUFFSIZE) {
      nLen = 0;
    }
    delay(5);
  }
#ifdef DEBUG_SIO
  Serial.println("");
  Serial.println("finished");
#endif
  //
  setCommMode(CONST_LORA_AS62_COMM_MODE_NORMAL);
  ret = nLen;
  return ret;
}

short LoRa_AS62::setConfig()
{
  short ret = 0;
  SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  delay(50);
  setCommMode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  //
  short i;
  for (i = 0; i < CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN; i++) {
    SerialPort.write(configData[i]);
  }
  delay(100);
  short tryTimes = 20;
  while (tryTimes)
  {
    if (readConfig(recvBuf) > 0)
      break;
    tryTimes--;
  }
  if (!memcmp(recvBuf, configData, CONST_LORA_AS62_DEFAULT_CONFIG_DATA_LEN)) {
    ret = -1;
  }
  //
  setCommMode(CONST_LORA_AS62_COMM_MODE_NORMAL);
  return ret;
}



short LoRa_AS62::begin()
{
  short ret = -1;
  int tryTimes = 5;
  while (tryTimes) {
    if (setConfig()) {
#ifdef DEBUG_SIO
      Serial.println();
      Serial.println("AS62 is ready for communication");
#endif
      ret = 0;
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
  return ret;
}

short LoRa_AS62::set(short channel, short rate)
{ ;
}

short nLFCount = 0;

/*
  // based on base64 recv function
  short LoRa_AS62::available()
  {
  char dispBuff[30];
  short ret = 0;
  while (SerialPort.available()) {
    uint8_t chR;
    chR = SerialPort.read();
    //sprintf(dispBuff, "|%3d %c", chR, chR );
    //sprintf(dispBuff, "%c", chR);
    switch (chR)
    {
      case CONST_LORA_COMMUNICATION_BEGIN_CHAR:
        recvLen = 0;
        break;
      case CONST_LORA_COMMUNICATION_END_CHAR:
        Serial.println("==ReceiveLF==");
        if (recvLen > 0) {
          recvBuf[recvLen ] = chR;
          recvBuf[recvLen + 1] = '\0';
          Serial.printf("\nrecvBuf[%s]", (char *)recvBuf);
          recvBuf[recvLen] = '\0';
          Serial.printf("\nrecvBufDecode %d, [%s]", strlen((char *) recvBuf), (char *) recvBuf);
          gDataBuffLen = Base64decode((char *)gDataBuff, (char *) recvBuf);
          //gDataBuffLen = slip_decode((unsigned char *)gDataBuff, (unsigned char *) recvBuf,recvLen);
          Serial.printf("\ngDataBuffLen[%s]", gDataBuff);
          Serial.println(gDataBuffLen);
          ret = gDataBuffLen;
        }
        break;
      default:
        recvBuf[recvLen++] = chR;
        break;
    }
    if (recvLen >= CONST_LORA_COMMUNICATION_BUFFSIZE) {
      recvLen = 0;
    }
  }
  return ret;
  }

  //based on base64 send function.
  short LoRa_AS62::send(uint8_t *u8Ptr, short nLen)
  {
  short ret;
  char dispBuff[100];
  short batchCount, restCount;
  short k;
  if (!u8Busy) {
    //可以发送,设置busy=1,重置发送时间定时器
    u8Busy = 1;
    vReset_interval(gulBusyTick);

    Serial.println("--send--");
    batchCount = (nLen + CONST_LORA_COMMUICATION_MAX_DATALEN - 1) / CONST_LORA_COMMUICATION_MAX_DATALEN;
    restCount = nLen % CONST_LORA_COMMUICATION_MAX_DATALEN;
    sprintf(dispBuff, "batch %d  rest %d", batchCount, restCount);
    Serial.println(dispBuff);
    for (k = 0; k < batchCount; k++) {
      short nLen;
      //memset(sendBuf, '\n', CONST_LORA_COMMUNICATION_BUFFSIZE);
      memset(sendBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE);
      //memset(sendBuf, ' ', CONST_LORA_COMMUNICATION_BUFFSIZE);
      //nLen = Base64encode(&sendBuf[2], (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), CONST_LORA_COMMUICATION_MAX_DATALEN);
      if (k < (batchCount - 1)) {
        nLen = Base64encode( (char *) &sendBuf[1], (char *) (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), CONST_LORA_COMMUICATION_MAX_DATALEN);
        //nLen = slip_encode( (unsigned char *) &sendBuf[1], (unsigned char *) (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), CONST_LORA_COMMUICATION_MAX_DATALEN);
      }
      else {
        nLen = Base64encode((char *) &sendBuf[1], (char *)(u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), restCount);
        //nLen = slip_encode((unsigned char  *) &sendBuf[1], (unsigned char *)(u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), restCount);
      }
      sendBuf[0] = CONST_LORA_COMMUNICATION_BEGIN_CHAR;
      nLen = strlen((char *)sendBuf);
      sendBuf[nLen] = CONST_LORA_COMMUNICATION_END_CHAR;
      nLen++;
      sendBuf[nLen] = '\0';
      sprintf(dispBuff, "%d Base64:%d %d [%s]", k, nLen, strlen((char *)sendBuf), (char *)sendBuf);
      Serial.println(dispBuff);
      short i;
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

*/


//based on slip protocol
short LoRa_AS62::available()
{
  char dispBuff[100];
  short ret = 0;
  recvReady = 0;
  while (SerialPort.available()) {
    uint8_t chT;
    chT = SerialPort.read();
    Serial.printf("%2X ", chT);
    //sprintf(dispBuff, "%c", chR);
    switch (chT)
    {
      case CONST_SLIP_END:
        if (recvLen == 0) {
          continue;
        }
        else {
          short i;
          Serial.println();
          for (i = 0; i < recvLen; i++) {
            chT = recvBuf[i];
            Serial.printf("%2X ", chT);
          }
          recvLen--;
          chT = crc8(recvBuf, recvLen);
          if (chT != recvBuf[recvLen]) {
            recvLen = -recvLen;
          }
          ret = recvLen;
          recvReady = recvLen;
          recvLen = 0;
        }
        break;
      case CONST_SLIP_ESC:
        chT = SerialPort.read();
        switch (chT)
        {
          case CONST_SLIP_ESC_END:
            recvBuf[recvLen++] = CONST_SLIP_END;
            break;
          case CONST_SLIP_ESC_ESC:
            recvBuf[recvLen++] = CONST_SLIP_ESC;
            break;
          default:
            break;
        }
        break;
      default:
        recvBuf[recvLen++] = chT;
        break;
    }
    if (recvLen >= CONST_LORA_COMMUNICATION_BUFFSIZE) {
      recvLen = 0;
    }
  }
  return ret;
}


//based on slip protocol
short LoRa_AS62::send(uint8_t *u8Ptr, short nLen)
{
  short ret;
  char dispBuff[100];
  short batchCount, restCount;
  short k;
  short i;
  if (!u8Busy) {
    //可以发送,设置busy=1,重置发送时间定时器
    u8Busy = 1;
    vReset_interval(gulBusyTick);

    Serial.println("--send--");
    batchCount = (nLen + CONST_LORA_COMMUICATION_MAX_DATALEN - 1) / CONST_LORA_COMMUICATION_MAX_DATALEN;
    restCount = nLen % CONST_LORA_COMMUICATION_MAX_DATALEN;
    sprintf(dispBuff, "batch %d  rest %d", batchCount, restCount);
    Serial.println(dispBuff);
    for (k = 0; k < batchCount; k++) {
      short nLen;
      //memset(sendBuf, '\0', CONST_LORA_COMMUNICATION_BUFFSIZE);
      Serial.println("send buff:");

      if (k < (batchCount - 1)) {
        nLen = slip_encode( (unsigned char *) &sendBuf[0], (unsigned char *) (u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), CONST_LORA_COMMUICATION_MAX_DATALEN);
      }
      else {
        //nLen = Base64encode((char *) &sendBuf[1], (char *)(u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), restCount);
        nLen = slip_encode((unsigned char  *) &sendBuf[0], (unsigned char *)(u8Ptr + k * CONST_LORA_COMMUICATION_MAX_DATALEN), restCount);
      }
      Serial.println("send buff:");
      for (i = 0; i < nLen; i++) {
        sprintf(&dispBuff[i * 3], "%02X ", sendBuf[i]);
      }
      Serial.println(dispBuff);
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

//wait for M0/M1 ready time, default =100ms;
#define CONST_LORA_AS62_WAIT_FOR_READY_TIME 200
void LoRa_AS62::setCommMode(uint8_t mode)
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

#define SIO_BAUD 115200
/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20170309
/*--------------------------------------------------------------------------------*/

LoRa_AS62 LoRaDev;

#define TEST_BUFF_LEN 100
uint8_t au8Buff[TEST_BUFF_LEN];
short au8BuffLen = 0;

char testSendData[5][40] = {
  "HELLO WORLD",
  "012345678901",
  "ABCDEFGHIJKLMN\n",
  "abcdefghijklmnopqrstuvwxyz\n",
  "AAABBBCCCDDD\n",
};

void test_send_receive()
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
      default:
        nNum = -1;
        break;
    }
    if (nNum >= 0) {
      Serial.printf("\n selected : %d : ", nNum);
      strcpy((char *) au8Buff, testSendData[nNum]);
      nLen = strlen((char *)au8Buff);
      Serial.printf("\nwill send  %d : [%s]\n", nLen, (char *)au8Buff);
      while (LoRaDev.busy())
      { ;
      }
      LoRaDev.send(au8Buff, nLen);
    }
  }
  short nLen;
  nLen = LoRaDev.available();
  if (nLen) {
    LoRaDev.get(gDataBuff);
    gDataBuff[nLen] = 0;
    Serial.println("\n--LoRa received Data");
    Serial.println(nLen);
    short i;
    char dispBuff[200];
    for (i = 0; i < nLen; i++)
    {
      sprintf(&dispBuff[i * 3], "%02X ", gDataBuff[i]);
    }
    Serial.println(dispBuff);
    for (i = 0; i < nLen; i++)
    {
      sprintf(&dispBuff[i], "%c", gDataBuff[i]);
    }
    Serial.println(dispBuff);

  }

}


short LoRa_AS62::readConfig(uint8_t *u8Ptr, short delayTime)
{
  short ret = 0;
  //SerialPort.begin(CONST_LORA_AS62_SIO_BAUD_RATE_DEFAULT);
  setCommMode(CONST_LORA_AS62_COMM_MODE_SLEEP);
  //
  //send query config
#ifdef DEBUG_SIO
  Serial.println("send 0xc1 0xc1 0xc1");
#endif
  SerialPort.write(0xc1);
  SerialPort.write(0xc1);
  SerialPort.write(0xc1);
  delay(delayTime);
  //wait for 2 seconds to get data;
  unsigned long ulTick;
  vReset_interval(ulTick);
  short nLen = 0;
#ifdef DEBUG_SIO
  Serial.println("wait for receive:");
#endif

  while (ulGet_interval(ulTick) < 2000) {
    if (SerialPort.available()) {
      uint8_t chR;
      chR = SerialPort.read();
      if (chR == 0xC0 || chR == 0xC2) {
        nLen = 0;
      }
      u8Ptr[nLen++] = chR;
#ifdef DEBUG_SIO
      //debug
      char dispBuff[10];
      sprintf(dispBuff, "%3X", chR);
      Serial.print(dispBuff);
#endif
    }
    if (nLen == 6)
      break;
    if (nLen > CONST_LORA_COMMUNICATION_BUFFSIZE) {
      nLen = 0;
    }
    delay(5);
  }
#ifdef DEBUG_SIO
  Serial.println("");
  Serial.println("finished");
#endif
  //
  setCommMode(CONST_LORA_AS62_COMM_MODE_NORMAL);
  ret = nLen;
  return ret;
}





void setup()
{
  // Open serial communications and wait for port to open:
#ifdef DEBUG_SIO
  Serial.begin(SIO_BAUD);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print("Serial Port for debug is ready on : ");
  Serial.println(SIO_BAUD);
#endif

  // set the data rate for the SoftwareSerial port
  LoRaDev.begin();
}

uint8_t congfigDataBuf[100];
short delayTimeMin = 50;
short delayTimeMax = 500;
short delayTime;

void loop() // run over and over
{
  Serial.println("test read config");
  while (delayTime < delayTimeMax) {
    delayTime += delayTimeMin;
    Serial.println(delayTime);
    LoRaDev.readConfig(congfigDataBuf, delayTime);
    delay(1000);
  }
  delayTime = 0;

}



