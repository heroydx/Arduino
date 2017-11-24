#include "Meter_PZ.h"
Meter_DQ gsMeter;

char dispBuff[1024];
char gBigBuff[1024];

void setup()
{
  SERIAL_DEBUG_BEGIN
  randomSeed(micros());
  gsMeter.begin(gBigBuff);

  DBGPRINTLN("SIO METER QUERY BEGIN:\n");
}

void loop() // run over and over
{
  if (gsMeter.read()) {
    DBGPRINT("\n gsMeter read data ");
    DBGPRINTF("\n %10s,%10s,%10s,%10s", "dianya", "dianliu","gonglv","dianliang");
    DBGPRINTF("\n [%8d],[%8d],[%8d],[%8d]", gsMeter.nDianyaInt, gsMeter.nDianliuInt, gsMeter.nGonglv, gsMeter.lMeterDianliang);
  }
}

