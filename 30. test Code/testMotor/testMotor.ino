
#include "Motor_DQ.h"

extern "C" {
#include "miscCommon.h"
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
    dispP[k] = ' ';
    k++;
  }
  dispP[k] = 0;
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


#define SIO_BAUD 74880
char dispBuff[100];

motor_DQ LLMotorData;
unsigned gulSecondTicks;
unsigned gulRecvTimeOutTick = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(SIO_BAUD);
  LLMotorData.begin();
  if (LLMotorData.devReady()) {
    DBGPRINTLN("\n ready!");
  }
  else {
    DBGPRINTLN("\n Not ready!");
  }

}

void loop() {
  // put your main code here, to run repeatedly:
  short nSIOava;
  LLMotorData.motor_loop();

  nSIOava = Serial.available();
  if (nSIOava > 0) {
    char chT;
    uint8_t nLen;
    chT = Serial.read();
    if (chT != 0) {
      Serial.printf("\n == Recv Serail Command: [ % c] == \n", chT);
    }
    switch (chT)
    {
      case '1':
        Serial.printf("\n Query Max Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_QUERY_MAX_CYCLE;
        LLMotorData.send_CMD(&LLMotorData.bufCommand);
        break;
      case '2':
        Serial.printf("\n SET Max Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_MAX_CYCLE;
        LLMotorData.bufCommand.speed = 3000;
        LLMotorData.send_CMD(&LLMotorData.bufCommand);
        break;
      case '3':
        Serial.printf("\n Query Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_QUERY_CURR_CYCLE;
        LLMotorData.send_CMD(&LLMotorData.bufCommand);
        break;
      case '4':
        Serial.printf("\n Set Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        LLMotorData.bufCommand.speed = 300;
        LLMotorData.send_CMD(&LLMotorData.bufCommand);
        break;
      case '5':
        Serial.printf("\n Set Current Cycle\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        LLMotorData.bufCommand.speed = 600;
        LLMotorData.send_CMD(&LLMotorData.bufCommand);
        break;
      case '0':
        Serial.printf("\n Stop\n", chT);
        LLMotorData.bufCommand.cmd = CONST_APP_MOTOR_SET_CURR_CYCLE;
        LLMotorData.bufCommand.speed = 00;
        LLMotorData.send_CMD(&LLMotorData.bufCommand);
        break;
      case 'p':
      case 'P':
        LLMotorData.debug_print_info();
        chT = 0;
        break;
      case '\x0A':
        chT = 0;
      default:
        chT = 0;
        break;
    }

    if (chT != 0) {
      Serial.println();
      Serial.println(LLMotorData.sendBuff);
      vHexString_2ascii((uint8_t *)LLMotorData.sendBuff, dispBuff, CONST_APP_MOTOR_MSG_LENG);
      Serial.println(dispBuff);
    }
  }

  if (ulGet_interval(gulSecondTicks) > 5000) {
    //Serial.println(nSeqCount);
    /*
      Serial.printf("\n---- - recv len [ % d] ------ -\n", recvBuffLen);
      for (int i = 0; i < CONST_APP_MOTOR_MSG_LENG; i++) {
      Serial.printf("[ % 02X]", recvBuff[i]);
      }
      Serial.println("------------------------------ -");
    */
    LLMotorData.debug_print_info();
    gulSecondTicks = ulReset_interval();
  }

}
