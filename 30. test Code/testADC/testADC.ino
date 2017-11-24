#include <arduino.h>

#define SIO_BAUD_RATE 74880

/*
Analog input

ESP8266 has a single ADC channel available to users. It may be used either to read voltage at ADC pin, or to read module supply voltage (VCC).

To read external voltage applied to ADC pin, use analogRead(A0). Input voltage range is 0 — 1.0V.

To read VCC voltage, ADC pin must be kept unconnected. Additionally, the following line has to be added to the sketch:

ADC_MODE(ADC_VCC);
*/

void setup()
{
  Serial.begin(SIO_BAUD_RATE);
  pinMode(A0,INPUT);

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.

}

#define CONST_ADC_MAX_VAL 1024
int referenceVCC=3306;
int i;

void loop()
{
  int nT1;
  long vcc;
  nT1 = analogRead(A0);
  i++;
  vcc=referenceVCC* (nT1*1000/CONST_ADC_MAX_VAL)/1000;
  Serial.printf("\nNo: %8d, A0:[%d] vcc: [%d] mv",i,nT1,vcc);
  delay(1000);
}

