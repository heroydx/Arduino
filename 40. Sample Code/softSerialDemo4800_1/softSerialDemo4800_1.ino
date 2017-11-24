/*
  Software serial multple serial test

 Receives from the two software serial ports,
 sends to the hardware serial port.

 In order to listen on a software port, you call port.listen().
 When using two software serial ports, you have to switch ports
 by listen()ing on each one in turn. Pick a logical time to switch
 ports, like the end of an expected transmission, or when the
 buffer is empty. This example switches ports when there is nothing
 more to read from a port

 The circuit:
 Two devices which communicate serially are needed.
 * First serial device's TX attached to digital pin 10(RX), RX to pin 11(TX)
 * Second serial device's TX attached to digital pin 8(RX), RX to pin 9(TX)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created 18 Apr. 2011
 modified 19 March 2016
 by Tom Igoe
 based on Mikal Hart's twoPortRXExample

 This example code is in the public domain.

 */
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX
#define SIO_BAUD 9600
void setup()  
{
 // Open serial communications and wait for port to open:
 Serial.begin(SIO_BAUD);
 while (!Serial) {
   ; // wait for serial port to connect. Needed for Leonardo only
 }


 Serial.println("Goodnight moon!");

 // set the data rate for the SoftwareSerial port
 mySerial.begin(SIO_BAUD);
 mySerial.println("Hello, world?");
}

void loop() // run over and over
{
 if (mySerial.available())
   mySerial.write(mySerial.read());
   
   /*
 if (Serial.available())
   mySerial.write(Serial.read());
*/
}

