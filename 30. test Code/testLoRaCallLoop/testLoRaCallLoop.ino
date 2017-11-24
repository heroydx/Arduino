#include <arduino.h>


#include <LoRa_AS62.h>


#define SIO_BAUD 115200
/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20170309
/*--------------------------------------------------------------------------------*/

LoRa_AS62 LoRaDev;

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


void setup()
{
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
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
  if (LoRaDev.isReady()) {
    Serial.println();
    Serial.println("AS62 is ready for communication");
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
      default:
        nNum = -1;
        break;
    }
    if (nNum >= 0) {
      Serial.printf("\n selected : % d : ", nNum);
      strcpy((char *) au8Buff, testSendData[nNum]);
      nLen = strlen((char *)au8Buff);
      Serial.printf("\nwill send  % d : [", nLen);
      Serial.printf(" % s]\n", (char *)au8Buff);
      while (LoRaDev.busy())
      { ;
      }
      LoRaDev.send(au8Buff, nLen);
    }
  }
  short nLen;
  nLen = LoRaDev.available();
  if (nLen) {
    if (nLen < 0) {
      Serial.printf("\nCRC ERROR:%d\n", nLen);
    }
    nLen = abs(nLen);
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
    //LoRaDev.send(gDataBuff, nLen);
  }
}


