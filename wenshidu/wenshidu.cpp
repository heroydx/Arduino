#include <wenshidu.h>


//class functions LoRaHost


short wenshidu::begin()
{
  nWenduInt=0;
  nWenduDec=0;
  nShiduInt=0;
  nShiduDec=0;
  nPreWenduInt=0;
  nPreShiduInt=0;
  decWendu[0]=0;
  decShidu[0]=0;

  pinMode(dht_dpin, INPUT);
  deviceReady = isDHT11or22();
}

bool wenshidu::_testDHT11or12(uint8_t type)
{
  bool ret = false;
  int chk;
  //DBGPRINTF("Try dht%d\n", type);
  switch (type)
  {
    case CONST_DHT_DEVICE_TYPE_DHT11:
      chk = DHT.read11(dht_dpin); //
      break;
    case CONST_DHT_DEVICE_TYPE_DHT22:
      chk = DHT.read22(dht_dpin); //
      break;
    default:
      chk = DHTLIB_ERROR_TIMEOUT;
      break;
  }
  switch (chk)
  {
    case DHTLIB_OK:
      //DBGPRINTLN("OK");
      ret = true;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      //DBGPRINTLN("Checksum error");
      ret = false;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      //DBGPRINTLN("Time out error");
      ret = false;
      break;
    default:
      //DBGPRINTLN("Unknown error");
      ret = false;
      break;
  }
  return ret;
}

uint8_t wenshidu::isDHT11or22()
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
    //DBGPRINTF("\nret [%d]\n", ret);

    if (ret > 0) {
      break;
    }
  }
  return ret;
}


uint8_t wenshidu::devReady()
{
  return deviceReady;
}

//检查是否收到数据
short wenshidu::read()
{
  short ret = 0;
  int nT1, nT2;
  if (deviceReady > 0)
  {

    switch (deviceReady) {
      case CONST_DHT_DEVICE_TYPE_DHT11:
        nT1 = DHT.read11(dht_dpin);
        break;
      case CONST_DHT_DEVICE_TYPE_DHT22:
        nT1 = DHT.read22(dht_dpin);
        break;
      default:
        break;

    }
    //DHT.read22(dht_dpin);
    if (DHT.temperature < 100.0 && DHT.temperature > -30.0 && DHT.humidity >= 0.0 && DHT.humidity <= 100.0)
    {
      //keep old data;
      nPreWenduInt = nWenduInt;
      nPreShiduInt = nShiduInt;

      nWenduInt = DHT.temperature;
      nWenduInt += nWenduOffset;
      nWenduDec = (DHT.temperature - nWenduInt) * 100 ;
      nShiduInt = DHT.humidity;
      nShiduDec = (DHT.humidity - nShiduInt) * 100 ;
      nT1 = abs(nWenduInt - nPreWenduInt);
      nT2 = abs(nShiduInt - nPreShiduInt);
      sprintf(decWendu, "%d.%d", nWenduInt, nWenduDec);
      sprintf(decShidu, "%d.%d", nShiduInt, nShiduDec);

      if (( nT1 >= CONST_WENDU_CHANGE_ALARM_VALUE) || ( nT2 >= CONST_SHIDU_CHANGE_ALARM_VALUE))
      {
        ret = 2;
      }
      else {
        ret = 1;
      }
      //check_status_action();
    }
  }

  return ret;
}



bool wenshidu::decode_802_command(char *ptr)
{
  bool ret = false;
  short nT1;
  nT1 = strlen(ptr);
  if (nT1 > 0)
  {
    nT1 = atoi(ptr);
    if (abs(nT1) <= CONST_WENDU_MAX_OFFSET_VAL)
    {
      nWenduOffset = nT1;
      ret = true;
    }
  }
  return ret;
}

void ICACHE_FLASH_ATTR wenshidu::debug_print_info()
{
  DBGPRINTLN("\n === wenshidu curr Stat begin ===");
  DBGPRINTF(" wendu [%d.%d], shidu [%d.%d]", nWenduInt, nWenduDec, nShiduInt,nShiduDec);
  DBGPRINTLN("\n === wenshidu Curr Stat end === \n");

  
}

