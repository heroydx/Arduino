#include <SoftwareSerial.h>
//SoftwareSerial mySerial(10, 11); // RX, TX Nano
//SoftwareSerial mySerial(19, 20); // RX, TX 8266 GPIO 4,5
#define SIO_BAUD 9600
#define SIO_MONITOR 38400


/*--------------------------------------------------------------------------------*/
//DEBUG_PART_BEGIN Ver=20161104
/*--------------------------------------------------------------------------------*/
#define DEBUG_SIO 1
//#define DEBUG_HTTP 1

#ifdef DEBUG_SIO
#define DBGPRINT(__VA_ARGS__) \
  Serial1.print(__VA_ARGS__)
#define DBGPRINTLN(__VA_ARGS__) \
  Serial1.println(__VA_ARGS__)
#define DBGPRINTF(fmt,...) \
  Serial1.printf(fmt,__VA_ARGS__)
#else
#define DBGPRINT(__VA_ARGS__)
#define DBGPRINTLN(__VA_ARGS__)
#define DBGPRINTF(fmt,...)
#endif




unsigned long glTick;

void vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}

unsigned long getInterval(unsigned long checkTick)
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
 Serial.begin(SIO_BAUD);
 Serial1.begin(SIO_MONITOR);
 while (!Serial) {
   ; // wait for serial port to connect. Needed for Leonardo only
 }


 //Serial.println("Goodnight moon!");

 // set the data rate for the SoftwareSerial port
 //mySerial.begin(SIO_BAUD);
 //mySerial.println("Hello, world?");
}

void loop() // run over and over
{
  uint8_t nT1;

 if (Serial.available()){
   nT1=Serial.read();
   //mySerial.write(nT1);
   Serial1.write(nT1);
 }
 if (getInterval(glTick)>=1000){
  DBGPRINTLN("Serial TX");
  DBGPRINTLN(glTick);
  vReset_interval(glTick);
 }
 
}

