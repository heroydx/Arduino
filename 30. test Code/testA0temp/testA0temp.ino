extern "C" {
#include "miscCommon.h"
}

void setup() {
  // put your setup code here, to run once:
  pinMode(A0, INPUT);

}

short nApp_read_A0()
{
  short nT1;
  nT1 = analogRead(A0);
  DBGPRINTF("\n A0:[%d] ", nT1);
  return nT1;
}

unsigned long gulApplicationTicks;

void loop() {
  // put your main code here, to run repeatedly:
  if ((ulGet_interval(gulApplicationTicks) > 1000))
  {
    gulApplicationTicks = ulReset_interval();
    nApp_read_A0();
  }

}
