#include <SoftwareSerial.h>


SoftwareSerial mySerial(10, 11); // RX, TX

#define CONST_SIO_BAUD_TEST 115200

#define CONST_APPLICATION_SIO_BAUD_DATA 9600
#define CONST_APPLICATION_SENDREV_BUFFLEN 40
#define CONST_APPLICATION_SIO_DATA_LEN 7

uint8_t gu8sendBuff[CONST_APPLICATION_SENDREV_BUFFLEN + 1];

#define CONST_APPLICATION_DIANYA_SEND 0xB0
#define CONST_APPLICATION_DIANYA_RECE 0xA0
#define CONST_APPLICATION_DIANLIU_SEND 0xB1
#define CONST_APPLICATION_DIANLIU_RECE 0xA1
#define CONST_APPLICATION_GONGLV_SEND 0xB2
#define CONST_APPLICATION_GONGLV_RECE 0xA2
#define CONST_APPLICATION_DIANLIANG_SEND 0xB3
#define CONST_APPLICATION_DIANLIANG_RECE 0xA3

#define CONST_APPLICATION_COLLECTION_INTERVAL 5000
#define CONST_APPLICATION_SIO_COMMAND_COUNT 4
#define CONST_APPLICATION_SIO_QUERY_INTERVAL  1000

uint8_t gu8SIOCommandHeader[4] = {
  CONST_APPLICATION_DIANYA_RECE, CONST_APPLICATION_DIANLIU_RECE,
  CONST_APPLICATION_GONGLV_RECE, CONST_APPLICATION_DIANLIANG_RECE
};


char dispBuff[256];
uint8_t dataBuff[256];

int gnSIOCommandNum = -1;
unsigned long glSIOQueryTick;


void vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}

uint8_t ascii(int hex)
{
  uint8_t cc;
  cc = 0x30;
  switch (hex)
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      cc = 0x30 + hex;
      break;
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      cc = 'A' + hex - 0x0A;
  }
  //Serial.printf("hex=[%x]\n",hex);
  //Serial.printf("cc=[%x]\n",cc);
  return cc;
}

uint8_t ascii2hex(uint8_t cc)
{
  uint8_t hex;
  hex = 0x00;
  switch (cc)
  {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      hex = cc - 0x30;
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
      hex = cc - 'a' + 0x0A;
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
      hex = cc - 'A' + 0x0A;
  }
  return hex;
}


uint8_t u8CheckSum(uint8_t *pBuff, int nLen) {
  uint8_t i, val = 0;
  for (i = 0; i < nLen; i++) {
    val += pBuff[i];
  }
  return (val & 0xFF);
}

void vGet_request_data(int nType, uint8_t *pBuff) {
  char chSum = 0;
  //int i;char bigBuff[100];
  if (nType < 0 || nType >= CONST_APPLICATION_SIO_COMMAND_COUNT)
    nType = 0;

  pBuff[0] = gu8SIOCommandHeader[nType];
  switch (pBuff[0])
  {

    case CONST_APPLICATION_DIANYA_RECE:
      pBuff[1] = 0x00;
      pBuff[2] = 0xE6;
      pBuff[3] = 0x02;
      pBuff[4] = 0x00;
      pBuff[5] = 0x00;
      break;
    case CONST_APPLICATION_DIANLIU_RECE:
      pBuff[1] = 0x00;
      pBuff[2] = 0x11;
      pBuff[3] = 0x20;
      pBuff[4] = 0x00;
      pBuff[5] = 0x00;
      break;
    case CONST_APPLICATION_GONGLV_RECE:
      pBuff[1] = 0x08;
      pBuff[2] = 0x98;
      pBuff[3] = 0x00;
      pBuff[4] = 0x00;
      pBuff[5] = 0x00;
      break;
    case CONST_APPLICATION_DIANLIANG_RECE:
      pBuff[1] = 0x01;
      pBuff[2] = 0x86;
      pBuff[3] = 0x9f;
      pBuff[4] = 0x00;
      pBuff[5] = 0x00;
      break;
  }
  pBuff[6] = u8CheckSum(pBuff, CONST_APPLICATION_SIO_DATA_LEN - 1);
  /*
    Serial.println("getData");
    for (i=0;i<CONST_APPLICATION_SIO_DATA_LEN;i++){
    sprintf(bigBuff,"%d:%02X ",i,pBuff[i]);
    Serial.print(bigBuff);
    }
    Serial.println();
  */
};

void vHexString_2ascii(uint8_t *pBuff, char *dispP, int nLen) {
  int i, k = 0;
  char chT;
  for (i = 0; i < nLen; i++) {
    chT = pBuff[i] >> 4;
    dispP[k] = ascii(chT);
    k++;
    chT = pBuff[i] & 0xF;
    dispP[k] = ascii(chT);
    k++;
  }
  dispP[k] = 0;
}

int nAna_Data(uint8_t data[])
{
  int result = -1;
  unsigned int i, nD0, nD1, nD2, nD3;
  uint8_t checkSum = u8CheckSum(data, CONST_APPLICATION_SIO_DATA_LEN);
  //if (checkSum == data[CONST_APPLICATION_SIO_DATA_LEN - 1])
  {
    nD0 = (int) data[0];
    nD1 = (int) data[1];
    nD2 = (int) data[2];
    nD3 = (int) data[3];

    Serial.println("anaData");
    Serial.print(nD0);
    Serial.print(" ");
    Serial.print(nD1);
    Serial.print(" ");
    Serial.print(nD2);
    Serial.print(" ");
    Serial.print(nD3);
    Serial.println();

    switch (nD0)
    {
      case CONST_APPLICATION_DIANYA_SEND:
        Serial.println("anaData-Dianya");
        result = 0;
        break;
      case CONST_APPLICATION_DIANLIU_SEND:
        Serial.println("anaData-Dianli");
        result = 1;
        break;
      case CONST_APPLICATION_GONGLV_SEND:
        Serial.println("anaData-Gonglv");
        result = 2;
        break;
      case CONST_APPLICATION_DIANLIANG_SEND:
        Serial.println("anaData-Dianliang");
        result = 3;
        break;
    }
  }
  return result;
}

void sendRequestData(int nType)
{
  int i;
  //char dispBuff[50];
  vGet_request_data(nType, gu8sendBuff);
  for (i = 0; i < CONST_APPLICATION_SIO_DATA_LEN; i++) {
    //mySerial.write(gu8sendBuff[i]);
    //Serial.write(gu8sendBuff[i]);
    mySerial.write(gu8sendBuff[i]);
    sprintf(&dispBuff[i * 3], "%02X ", gu8sendBuff[i]);
  }
  dispBuff[3 * CONST_APPLICATION_SIO_DATA_LEN] = 0;
  //mySerial.println(dispBuff);
  Serial.print("sendRequestData:");
  Serial.println(dispBuff);
}

void setup()
{
  int i, k;
  //char bigBuff[100];
  //init data
  //asDataMark[DATAMARK_VOLTAGE][]={0xB0,0xC0,0xA8,0x01,0x01,0x00};
  // Open serial communications and wait for port to open:
  Serial.begin(CONST_SIO_BAUD_TEST);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //Serial.println("SIO TEST BEGIN:");
  // set the data rate for the SoftwareSerial port
  mySerial.begin(CONST_APPLICATION_SIO_BAUD_DATA);
  //mySerial.println("SIO TEST BEGIN:\n");
  Serial.println("SIO METER SIMULATION BEGIN:\n");
  //mySerial.println("Hello, world?");

  /*
    for (i=0;i<DATAMARK_LEN;i++){
    //sprintf(bigBuff,"%d",i);
    //Serial.println(bigBuff);

    vGet_request_data(i,gu8sendBuff);
    vHexString_2ascii(gu8sendBuff,dispBuff,CONST_APPLICATION_SIO_DATA_LEN);
    //sprintf(bigBuff,"%s",dispBuff);
    Serial.println(dispBuff);
    mySerial.println(dispBuff);
    delay(500);

    }
  */
  /*
    for (k = 0; k < 4; k++) {
    //sprintf(dispBuff, "%2d:", k);
    //Serial.println(dispBuff);
    //mySerial.println(dispBuff);
    sendRequestData(k);

    delay(500);

    }
  */

  Serial.println(CONST_APPLICATION_SIO_QUERY_INTERVAL);
  Serial.println("SIO SETUP END:\n");
}


void loop() // run over and over
{

  /*   if( nCount<6){
      vGet_request_data(nCount,gu8sendBuff);
      vHexString_2ascii(gu8sendBuff,dispBuff,CONST_APPLICATION_SIO_DATA_LEN);
      Serial.println("in LOOP:");
      mySerial.println(dispBuff);
      nCount++;
      delay(10);
    }
    sendRequestData(nCount);
    nCount++;
    if (nCount>3)
      nCount=0;
  */
  int nSIOava;
  nSIOava = mySerial.available();

  if (nSIOava == (CONST_APPLICATION_SIO_DATA_LEN))
  {
    uint8_t chT;
    int i = 0;
    Serial.println();
    Serial.print("Get Data:");
    Serial.println(nSIOava);
    while (mySerial.available()) {
      chT = mySerial.read();
      dataBuff[i] = chT;
      i++;
      delay(50);
    }
    Serial.println("data len");
    Serial.println(i);
    Serial.println("before call vAna_Data");
    gnSIOCommandNum = nAna_Data(dataBuff);

    vHexString_2ascii(dataBuff, dispBuff, i);
    Serial.println(dispBuff);

  }
  if (Serial.available())
  {

    char chT;

    chT = Serial.read();
    switch (chT) {
      case '0':
        gnSIOCommandNum = 0;
        break;
      case '1':
        gnSIOCommandNum = 1;
        break;
      case '2':
        gnSIOCommandNum = 2;
        break;
      case '3':
        gnSIOCommandNum = 3;
        break;

    }
    Serial.println();
    Serial.print("system command:");
    Serial.print(chT);
    Serial.print(":");
    //sendRequestData(nType);
  }
  /*
    if (Serial.available())
     mySerial.write(Serial.read());
  */


  //nType=2;
  vReset_interval(glSIOQueryTick);
  if (gnSIOCommandNum >= 0)
  {
    Serial.println("**CLEAN SIODATA BEGIN:");
    while (mySerial.available()) {
      mySerial.read();
      //delay(1);
    }
    Serial.println("**CLEAN SIODATA END:");
    Serial.println();
    Serial.println("Respose to:");
    Serial.println(glSIOQueryTick);
    sendRequestData(gnSIOCommandNum);
    gnSIOCommandNum = -1;
  }
}

