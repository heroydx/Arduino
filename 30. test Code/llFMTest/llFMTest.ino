
//#define DEBUG_SIO Serial

#include "ESPSoftwareSerial.h"
#include "FM_ZCJ_SM.h"


/*================================================================================*/
//MAIN_PART_BEGIN
/*================================================================================*/
//DON'T CHANGE THE MAIN PART CODE

#ifdef DEBUG_SIO
extern char dispBuff[];
#endif

short gFMState;

FM_ZCJ FMDevice;

void setup()
{
#ifdef DEBUG_SIO
  SERIAL_DEBUG_BEGIN
#endif
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  FMDevice.begin();
}



short nFMCommand;
short nFMParam;
char achFM[100];

#define DELAY_LEN 2000

void loop()
{
  char *strP;
  strP = achFM;
  FMDevice.FM_loop();
  delay(DELAY_LEN);
  strcpy(achFM, "8:900");
  DBGPRINTF("\n change to FM [%s]", achFM);
  FMDevice.trans_Server_to_Inter_FM_command( strP, &nFMCommand, &nFMParam);
  FMDevice.send_CMD_to_FM(nFMCommand,nFMParam);
  delay(DELAY_LEN);
  strcpy(achFM, "8:1000");
  DBGPRINTF("\n change to FM [%s]", achFM);
  FMDevice.trans_Server_to_Inter_FM_command( strP, &nFMCommand, &nFMParam);
  FMDevice.send_CMD_to_FM(nFMCommand,nFMParam);
  delay(DELAY_LEN);
  strcpy(achFM, "8:1039");
  DBGPRINTF("\n change to FM [%s]", achFM);
  FMDevice.trans_Server_to_Inter_FM_command( strP, &nFMCommand, &nFMParam);
  FMDevice.send_CMD_to_FM(nFMCommand,nFMParam);

}

/*================================================================================*/
//MAIN_PART_END
/*================================================================================*/

