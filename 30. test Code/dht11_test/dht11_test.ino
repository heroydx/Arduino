#include "dht.h"
#define dht_dpin 16 //定義訊號要從Pin A0 進來  

dht DHT;

#define CONST_LED_COLOR_RED_PIN 13


#define DHTLIB_OK 0
#define DHTLIB_ERROR_CHECKSUM -1
#define DHTLIB_ERROR_TIMEOUT -2
uint8_t deviceReady;
bool dht11Ready;
bool dht22Ready;

char dispBuff[100];

#define DEBUG_SIO Serial


#if defined(DEBUG_SIO)
#define SERIAL_DEBUG_BEGIN DEBUG_SIO.begin(74880);
#define DBGPRINT(__VA_ARGS__) DEBUG_SIO.print(__VA_ARGS__)
#define DBGPRINTLN(__VA_ARGS__) DEBUG_SIO.println(__VA_ARGS__)
#define DBGPRINTF(fmt,...) DEBUG_SIO.printf(fmt,__VA_ARGS__)

#else
#define SERIAL_DEBUG_BEGIN
#define DBGPRINT(__VA_ARGS__)
#define DBGPRINTLN(__VA_ARGS__)
#define DBGPRINTF(fmt,...)
#endif

#define CONST_DHT_DEVICE_TYPE_DHT11 11
#define CONST_DHT_DEVICE_TYPE_DHT22 22
#define CONST_DHT_DELAY_TIME 250
#define CONST_DHT_TRY_TIMES 8

bool _testDHT11or12(uint8_t type)
{
  bool ret = false;
  int chk;
  DBGPRINTF("Try dht%d\n", type);
  switch (type)
  {
    case CONST_DHT_DEVICE_TYPE_DHT11:
      chk = DHT.read11(dht_dpin); //去library裡面找DHT.read11
      break;
    case CONST_DHT_DEVICE_TYPE_DHT22:
      chk = DHT.read22(dht_dpin); //去library裡面找DHT.read11
      break;
    default:
      chk = DHTLIB_ERROR_TIMEOUT;
      break;
  }
  switch (chk)
  {
    case DHTLIB_OK:
      DBGPRINTLN("OK");
      ret = true;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      DBGPRINTLN("Checksum error");
      ret = false;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      DBGPRINTLN("Time out error");
      ret = false;
      break;
    default:
      DBGPRINTLN("Unknown error");
      ret = false;
      break;
  }
  return ret;
}

uint8_t isDHT11or22()
{
  uint8_t ret = 0;
  bool dht11Ready, dht22Ready;
  short tryTimes = CONST_DHT_TRY_TIMES;
  while (tryTimes > 0)
  {
    delay(CONST_DHT_DELAY_TIME);
    dht22Ready = _testDHT11or12(CONST_DHT_DEVICE_TYPE_DHT22);
    delay(CONST_DHT_DELAY_TIME);
    dht11Ready = _testDHT11or12(CONST_DHT_DEVICE_TYPE_DHT11);
    if ((dht22Ready == true) && (dht11Ready == false))
    {
      ret = CONST_DHT_DEVICE_TYPE_DHT22;
    }
    else if (dht11Ready == true) {
      ret = CONST_DHT_DEVICE_TYPE_DHT11;
    }
    else {
      ret = 0;
    }
    tryTimes--;
    DBGPRINTF("\nret [%d]\n", ret);

    if (ret > 0) {
      break;
    }
  }
  return ret;
}


uint8_t isDHT11or22_2()
{
  int chk;
  short tryTimes = 5;
  deviceReady = -1;
  while (tryTimes > 0)
  {
    Serial.println("Try dht11");

    chk = DHT.read11(dht_dpin); //去library裡面找DHT.read11
    switch (chk)
    {
      case DHTLIB_OK:
        Serial.println("OK");
        dht11Ready = true;
        break;
      case DHTLIB_ERROR_CHECKSUM:
        Serial.println("Checksum error");
        dht11Ready = false;
        break;
      case DHTLIB_ERROR_TIMEOUT:
        Serial.println("Time out error");
        dht11Ready = false;
        break;
      default:
        Serial.println("Unknown error");
        dht11Ready = false;
        break;
    }
    delay(200);

    Serial.println("Try dht22");

    chk = DHT.read22(dht_dpin); //去library裡面找DHT.read11
    switch (chk)
    {
      case DHTLIB_OK:
        Serial.println("OK");
        dht22Ready = true;
        break;
      case DHTLIB_ERROR_CHECKSUM:
        Serial.println("Checksum error");
        dht22Ready = false;
        break;
      case DHTLIB_ERROR_TIMEOUT:
        Serial.println("Time out error");
        dht22Ready = false;
        break;
      default:
        Serial.println("Unknown error");
        dht22Ready = false;
        break;
    }
    delay(200);
    if ((dht22Ready == true) && (dht11Ready == false))
    {
      deviceReady = 2;
    }
    else if ((dht22Ready == false) && (dht11Ready == true)) {
      deviceReady = 1;
    }
    else {
      deviceReady = 0;
    }
    tryTimes--;
    sprintf(dispBuff, "device ready [ % d]", deviceReady);
    Serial.println(dispBuff);
    if (deviceReady > 0) {
      break;
    }
  }
  return deviceReady;
}



void setup() {
  Serial.begin(74880);
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  delay(300);             //Let system settle
  Serial.println("Humidity and temperature\n\n");
  delay(700);             //Wait rest of 1000ms recommended delay before
  //accessing sensor
  deviceReady = isDHT11or22();
}

void loop() {
  int chk;
  DBGPRINTF("deviceReady [%d]\n", deviceReady);
  if (deviceReady) {
    switch (deviceReady) {
      case 1:
      case CONST_DHT_DEVICE_TYPE_DHT11:
        chk = DHT.read11(dht_dpin); //去library裡面找DHT.read11
        break;
      case 2:
      case CONST_DHT_DEVICE_TYPE_DHT22:
        chk = DHT.read22(dht_dpin);
        break;
      default:
        break;
    }

    // put your main code here, to run repeatedly:
    Serial.println(chk);
    Serial.print("Humidity = ");
    Serial.print(DHT.humidity);
    Serial.print(" % ");
    Serial.print("temperature = ");
    Serial.print(DHT.temperature);
    Serial.println("C ");
  }
  digitalWrite(CONST_LED_COLOR_RED_PIN, 1);
  delay(1000);            //每1000ms更新一次
  digitalWrite(CONST_LED_COLOR_RED_PIN, 0);
  delay(1000);
}
