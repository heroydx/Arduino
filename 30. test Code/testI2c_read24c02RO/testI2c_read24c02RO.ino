#include <arduino.h>

extern "C" {
#include "miscCommon.h"
}


/////////////////////////////////////////////////////////////////////////////////////////
// General include
#include <Wire.h>
//#include <EEPROM.h>
/////////////////////////////////////////////////////////////////////////////////////////
// EEPROM definitions
unsigned int EEPROMBank = 0x50;                 // Base address of 1st EEPROM chip
unsigned int EEPROMAddress = 0;                 // Sets the start EEPROM address
unsigned int EEPROMValue = 0;                   // EEPROM value
byte Dummy;

// EEPROM list taken from:
// http://vascoferraz.com/projects/24c-eeprom-check/
// Sets the Maximum EEPROM address, uncomment accordingly:
//#define MaxBankAddress  128 - 1             //24C01    -> 1024 bit    -> 128 byte
#define MaxBankAddress  256 - 1             //24C02    -> 2048 bit    -> 256 byte
//#define MaxBankAddress  512 - 1             //24C04    -> 4096 bit    -> 512 byte
//#define MaxBankAddress  1024 - 1         //24C08    -> 8192 bit    -> 1024 byte
//#define MaxBankAddress  2048 - 1            //24C16    -> 16384 bit   -> 2048 byte
//#define MaxBankAddress  4096 - 1            //24C32    -> 32768 bit   -> 4096 byte
//#define MaxBankAddress  8192 - 1            //24C64    -> 65536 bit   -> 8192 byte
//#define MaxBankAddress  16384 - 1           //24C128   -> 131072 bit  -> 16384 byte
//#define MaxBankAddress  32768 - 1           //24C256   -> 262144 bit  -> 32768 byte
//#define unsigned int maxaddress  65536 - 1;  //24C512   -> 524288 bit  -> 65536 byte
/////////////////////////////////////////////////////////////////////////////////////////
// I2C definitions
//#define CONST_I2C_SCL_PIN 5 //D1
//#define CONST_I2C_SCL_PIN 4 //D2
//#define CONST_I2C_SDA_PIN 2 //D4
#define CONST_I2C_SCL_PIN D1
#define CONST_I2C_SDA_PIN D2
//#define SCLPin D1                               // I2C SCL Pin
//#define SDAPin D2                               // I2C SDA Pin
/////////////////////////////////////////////////////////////////////////////////////////
// RTC DS1307 definitions
/////////////////////////////////////////////////////////////////////////////////////////

// data
//  254, 155, 11, 105, 21, 134, 215, 200, 4, 117, 7, 129, 156, 1, 6, 192, 168, 1, 1, 85, 240

byte au8WriteData[MaxBankAddress + 2] = {
  123, 170, 11, 40, 22, 2, 45, 56, 4, 92, 0, 0, 20, 1, 41, 192, 170, 1, 1, 85, 240
};

void setup() {
  Serial.begin(74880);                          // Initialize serial port
  while (!Serial);                              // wait for serial
  delay(1000);                                  // Waits 3 seconds
  Serial.println("\n=============================");          // Sends preamble and Cr+Lf
  // Wire.begin();                              // Initialize 2-Wire bus
  // http://www.esp8266.com/viewtopic.php?f=13&t=10374#p49060
  // Wire.begin(int sda, int scl);              // Initialize 2-Wire bus
  Wire.begin(CONST_I2C_SDA_PIN, CONST_I2C_SCL_PIN);                   // Initialize 2-Wire bus
  //pinMode(D1, INPUT_PULLUP);                    // I Need to validate this line
  //pinMode(D2, INPUT_PULLUP);                    // I Need to validate this line
  EEPROMAddress = 0;                            // EEPROM word variable
  //EEPROMValue = 123;                            // EEPROM value
  Serial.print(", EEPROMBank: 0x");
  Serial.println(EEPROMBank, HEX);
  /*
    for (EEPROMAddress = 0; EEPROMAddress < MaxBankAddress; EEPROMAddress++) {
      writeEEPROM(EEPROMBank, EEPROMAddress, byte(au8WriteData[EEPROMAddress]));
    }
  */
  for (EEPROMAddress = 0; EEPROMAddress < MaxBankAddress; EEPROMAddress++) {
    Serial.print("EEPROMAddress: ");
    Serial.print(EEPROMAddress);
    Serial.print(", Value: ");
    Serial.println(readEEPROM(EEPROMBank, EEPROMAddress), DEC);
  }
}
/////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Do something useful here.
}

///////////////////////////////////////////////////////////////////////////////////////////
// http://www.hobbytronics.co.uk/arduino-external-eeprom
void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ) {
  Wire.beginTransmission(deviceaddress);
  if (MaxBankAddress >= 256) {
    Wire.write((int)(eeaddress >> 8));   // MSB
  }
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}



/////////////////////////////////////////////////////////////////////////////////////////
byte readEEPROM(int deviceaddress, unsigned int eeaddress ) {
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  if (MaxBankAddress >= 256) {
    Wire.write((int)(eeaddress >> 8));   // MSB
  }
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress, 1);
  /*
    while (1)
    {
    if (Wire.available()) {
      rdata = Wire.read();
      break;
    }
    }
  */

  if (Wire.available()) {
    rdata = Wire.read();
  }
  Wire.endTransmission();
  return rdata;
}
///////////////////////////////////////////////////////////////////////////////////////// - See more at: http://www.esp8266.com/viewtopic.php?f=32&t=12623#sthash.6OLRa606.dpuf
