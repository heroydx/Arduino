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
#define CONST_I2C_SCL_PIN 4 //D2
#define CONST_I2C_SDA_PIN 2 //D4
//#define SCLPin D1                               // I2C SCL Pin
//#define SDAPin D2                               // I2C SDA Pin
/////////////////////////////////////////////////////////////////////////////////////////
// RTC DS1307 definitions
/////////////////////////////////////////////////////////////////////////////////////////

// data
//电压不对110，其余的数对
// 254, 155, 11, 105, 21, 134, 215, 200, 4, 117, 7, 129, 156, 1, 6, 192, 168, 1, 1, 85, 240
//  123, 170, 11, 40, 22, 2, 45, 56, 4, 92, 0, 0, 20, 1, 41, 192, 170, 1, 1, 85, 240
// 124,183,11,77,21,149,2,88,4,114,0,0,13,1,54,192,168,1,1,85,240
#define CONST_WRITE_DATE_LEN 24

byte au8WriteData[CONST_WRITE_DATE_LEN] = {
  //254, 155, 11, 105, 21, 134, 215, 200, 4, 117, 7, 129, 156, 1, 6, 192, 168, 1, 1, 85, 240
  //123, 170, 11, 40, 22, 2, 45, 56, 4, 92, 0, 0, 20, 1, 41, 192, 170, 1, 1, 85, 240
  //124,183,11,77,21,149,2,88,4,114,0,0,13,1,54,192,168,1,1,85,240
  124, 241, 11, 60, 21, 162, 51, 176, 4, 112, 0, 0, 10, 0, 160, 192, 168, 1, 1, 85, 240, 255, 255, 255
};

byte au8ReadData[MaxBankAddress + 2];

byte defaulatVal = 0xff;
#define CONST_COMMAND_BUFF_LENGTH 80
char gachCommand[CONST_COMMAND_BUFF_LENGTH];
short gnCommandCount = 0;

void setup() {
  Serial.begin(74880);                          // Initialize serial port
  while (!Serial);                              // wait for serial
  delay(100);                                  // Waits 3 seconds
  Serial.println("\n=============================");          // Sends preamble and Cr+Lf
  // Wire.begin();                              // Initialize 2-Wire bus
  // http://www.esp8266.com/viewtopic.php?f=13&t=10374#p49060
  // Wire.begin(int sda, int scl);              // Initialize 2-Wire bus
  Wire.begin(CONST_I2C_SDA_PIN, CONST_I2C_SCL_PIN);                   // Initialize 2-Wire bus
  //pinMode(D1, INPUT_PULLUP);                    // I Need to validate this line
  //pinMode(D2, INPUT_PULLUP);                    // I Need to validate this line
  EEPROMAddress = 0;                            // EEPROM word variable
  EEPROMValue = 123;                            // EEPROM value
  Serial.print(", EEPROMBank: 0x");
  Serial.println(EEPROMBank, HEX);
  read_data();

  Serial.println("\n r--> read, w--> write, p--> print read data, d--> duplicate read data");
  short i;
  for (i = 0; i < CONST_WRITE_DATE_LEN; i++) {
    Serial.printf(" %02X", au8WriteData[i]);
  }
  Serial.println("");
}
/////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // Do something useful here.
  char chT;
  while (Serial.available() ) {
    chT = Serial.read();
    gachCommand[gnCommandCount] = chT;
    gnCommandCount++;
    if (gnCommandCount >= CONST_COMMAND_BUFF_LENGTH) {
      gnCommandCount = 0;
    }
    Serial.print(chT);
    switch (chT)
    {
      case '\n':
      case '\a':
        //Serial.print(" got");
        gachCommand[gnCommandCount] = 0;
        exec_command(gachCommand);
        gnCommandCount = 0;
        break;
      default:
        break;
    }
  }
  delay(10);
}

void exec_command(char achCommand[])
{
  short nLen;
  nLen = strlen(achCommand);
  if (nLen == 2) {
    switch (achCommand[0])
    {
      case 'w':
      case 'W':
        Serial.println("write to chip");
        write_data();
        read_data();
        break;
      case 'r':
      case 'R':
        Serial.println("read chip");
        read_data();
      case 'p':
      case 'P':
        Serial.println("print read data");
        print_read_data();
        break;
      case 'd':
      case 'D':
        Serial.println("duplicate read data");
        duplicate_data();
        read_data();
        break;
      default:
        break;
    }
  }
}

void read_data()
{
  short nLen = 16;
  short nLine = 0;
  Serial.println("\n======READ DATA=======================");          // Sends preamble and Cr+
  Serial.printf("\n%5s%5d%5d%5d%5d%5d%5d%5d%5d-%5d%5d%5d%5d%5d%5d%5d%5d", ":", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  for (EEPROMAddress = 0; EEPROMAddress <= MaxBankAddress; EEPROMAddress++) {
    if (EEPROMAddress % nLen == 0) {
      Serial.printf("\n%4d:", nLine);
      nLine++;
    }
    //Serial.printf("[%02X]:", EEPROMAddress);
    au8ReadData[EEPROMAddress] = readEEPROM(EEPROMBank, EEPROMAddress);
    Serial.printf("  %02X,", au8ReadData[EEPROMAddress]);
    delay(1);
  }
  Serial.println();

}


void print_read_data()
{
  short nLen = 16;
  short nLine = 0;
  Serial.println("\n======PRITN READ DATA=======================");          // Sends preamble and Cr+
  Serial.printf("\n%5s%5d%5d%5d%5d%5d%5d%5d%5d-%5d%5d%5d%5d%5d%5d%5d%5d", ":", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
  for (EEPROMAddress = 0; EEPROMAddress <= MaxBankAddress; EEPROMAddress++) {
    if (EEPROMAddress % nLen == 0) {
      Serial.printf("\n%4d:", nLine);
      nLine++;
    }
    //Serial.printf("[%02X]:", EEPROMAddress);
    //au8ReadData[EEPROMAddress] = readEEPROM(EEPROMBank, EEPROMAddress);
    Serial.printf("  %02X,", au8ReadData[EEPROMAddress]);
    delay(1);
  }
  Serial.println();

}

void write_data()
{
  byte val;
  short nLen;
  nLen = sizeof (au8WriteData);
  Serial.println("\n======WRITE DATA=======================");          // Sends preamble and Cr+Lf
  for (EEPROMAddress = 0; EEPROMAddress <= MaxBankAddress; EEPROMAddress++) {
    if (EEPROMAddress >= nLen) {
      val = defaulatVal;
    }
    else {
      val = byte(au8WriteData[EEPROMAddress]);
    }
    writeEEPROM(EEPROMBank, EEPROMAddress, byte(val));
    delay(2);
  }

}


void duplicate_data()
{
  byte val;
  Serial.println("\n====== Duplicate READ DATA=======================");          // Sends preamble and Cr+Lf
  for (EEPROMAddress = 0; EEPROMAddress <= MaxBankAddress; EEPROMAddress++) {
    val = byte(au8ReadData[EEPROMAddress]);
    writeEEPROM(EEPROMBank, EEPROMAddress, byte(val));
    delay(2);
  }

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
