#include <arduino.h>
#include <string.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX D10, TX D11 Nano

#define DEBUG_SIO

//SoftwareSerial mySerial(19, 20); // RX, TX 8266 GPIO 4,5
#define SIO_BAUD 115200
//#define APP_SERIAL Serial
#define APP_SERIAL mySerial
#define CONST_APPLICATION_SIO_BAUD_DATA 38400
#define CONST_APPLICATION_SIO_DATA_LEN 2

enum ATCOMMAND {
  FM_SET_FRE,
  FM_FRE_DOWN,
  FM_FRE_UP,
  FM_PLAY_PAUSE,
  FM_SET_VOL,
  FM_VOL_DOWN,
  FM_VOL_UP,
  FM_SET_BANK,
  FM_SET_CAMPUS,
  FM_SET_DSP,
  FM_SET_SN_THR,
  FM_RESET,
  FM_STATUS,
};

uint8_t chpData[100];
int gnSIOCommandNum = 0;
unsigned long glSIOQueryTick;

#define CONST_APPLICATION_SIO_QUERY_INTERVAL  1000*5
#define CONST_APPLICATION_SENDREV_BUFFLEN 50

uint8_t gu8sendBuff[CONST_APPLICATION_SENDREV_BUFFLEN + 2];
uint8_t gu8revBuff[CONST_APPLICATION_SENDREV_BUFFLEN + 2];

#define CONST_APPLICATION_SIO_COMMAND_COUNT 12
#define CONST_APPLICATION_PARAM_LEN 6
unsigned long glTick;
struct testDataSt {
  int command;
  char param[CONST_APPLICATION_PARAM_LEN];
};

int gnSendCommand;
bool bSendBusyFlag = false;

#define CONST_RECV_TIMEOUT_LEN 1000*2
unsigned long glRecvTimeOutTick;

#define CONST_TEST_DATA_LEN 50

testDataSt gaTestData[CONST_TEST_DATA_LEN] = {
  {FM_SET_FRE, "900" },
  {FM_PLAY_PAUSE, },
  {FM_SET_FRE, "1061" },
  {FM_FRE_DOWN, },
  {FM_SET_FRE, "1039" },
  {FM_SET_FRE, "876" },
  {FM_VOL_DOWN, },
  {FM_SET_VOL, "20"},
  {FM_SET_BANK,"10"},
  {FM_SET_DSP,"1"},
  {FM_SET_SN_THR,"10"},
  {FM_STATUS, },
};
/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20170309
/*--------------------------------------------------------------------------------*/


void vReset_interval(unsigned long &resetTick)
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


void setup()
{
  // Open serial communications and wait for port to open:
#ifdef DEBUG_SIO
  Serial.begin(SIO_BAUD);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print("Serial Port for debug is ready on :");
  Serial.println(SIO_BAUD);
#endif

  // set the data rate for the SoftwareSerial port
  APP_SERIAL.begin(CONST_APPLICATION_SIO_BAUD_DATA);
}

void loop() // run over and over
{
  vApplication_connected_loop_call();
}


void vApplication_connected_loop_call()
{
  vGet_reply_data(gu8revBuff);
  if ((ulGet_interval(glRecvTimeOutTick) > CONST_RECV_TIMEOUT_LEN)) {
    bSendBusyFlag = false;
  }

  if ((ulGet_interval(glSIOQueryTick) > CONST_APPLICATION_SIO_QUERY_INTERVAL))
  {
    char *strParam;
    gnSendCommand = gaTestData[gnSIOCommandNum].command;
    strParam = (char *) gaTestData[gnSIOCommandNum].param;
    if (!bSendBusyFlag) {
      //set recv time out tick
      vReset_interval(glRecvTimeOutTick);

      vSend_request_data(gnSendCommand, strParam);
    }
    gnSIOCommandNum++;
    if (gnSIOCommandNum >= CONST_APPLICATION_SIO_COMMAND_COUNT)
    {
      gnSIOCommandNum = 0;
    }
    vReset_interval(glSIOQueryTick);
  }

}

void vGet_request_data(int nType, char *strParam, char *pBuff) {
  char chSum = 0;
  //int i;char bigBuff[100];
  if (nType < 0 || nType >= CONST_APPLICATION_SIO_COMMAND_COUNT)
    nType = 0;
  switch (nType)
  {
    case  FM_SET_FRE:
      sprintf(pBuff, "AT+FRE=%s\n", strParam);
      break;
    case  FM_FRE_DOWN:
      strcpy(pBuff, "AT+FRED\n");
      break;
    case  FM_FRE_UP:
      strcpy(pBuff, "AT+FREU\n");
      break;
    case  FM_PLAY_PAUSE:
      strcpy(pBuff, "AT+PAUS\n");
      break;
    case  FM_SET_VOL:
      sprintf(pBuff, "AT+VOL=%s\n", strParam);
      break;
    case  FM_VOL_DOWN:
      strcpy(pBuff, "AT+VOLD\n");
      break;
    case  FM_VOL_UP:
      strcpy(pBuff, "AT+VOLU\n");
      break;
    case  FM_SET_BANK:
      sprintf(pBuff, "AT+BANK=%s\n", strParam);
      break;
    case  FM_SET_CAMPUS:
      sprintf(pBuff, "AT+CAMPUS=%s\n", strParam);
      break;
    case  FM_SET_DSP:
      sprintf(pBuff, "AT+SN=%s\n", strParam);
      break;
    case  FM_SET_SN_THR:
      sprintf(pBuff, "AT+SN_THR=%s\n", strParam);
      break;
    case  FM_RESET:
      strcpy(pBuff, "AT+CR\n");
      break;
    case  FM_STATUS:
      strcpy(pBuff, "AT+RET\n");
      break;
    default:
      break;
  }

};


void vSend_request_data(int nType, char *strParam)
{
  int i;
  int nLen;

  vGet_request_data(nType, strParam, (char *) gu8sendBuff);
  nLen = strlen((char *)gu8sendBuff);
  for (i = 0; i < nLen; i++) {
    APP_SERIAL.write(gu8sendBuff[i]);
  }
  bSendBusyFlag = true;
#ifdef DEBUG_SIO
  Serial.print("\nSend:");
  Serial.print((char *)gu8sendBuff);
  Serial.println("------------------");
#endif
}

void vGet_reply_data(uint8_t *rBuff)
{
  int nSIOava;
  nSIOava = APP_SERIAL.available();
  if (nSIOava > (CONST_APPLICATION_SIO_DATA_LEN - 1))
  {
    uint8_t chR;
    int j = 0;

#ifdef DEBUG_SIO
    Serial.print("***SIO READ DATA:");
    Serial.println(nSIOava);
#endif

    while (APP_SERIAL.available()) {
       chR = APP_SERIAL.read();
       chpData[j] = chR;
       if (chR == '\n') {
          bSendBusyFlag = false;
       }
       j++;
       delay(1);
   }
       sprintf((char *) rBuff, "%s", chpData);
       Serial.println((char *)chpData);
       if (strcmp( (char *) chpData, "ERR\n")) {
         Serial.println("failed");
       }
 }
}

