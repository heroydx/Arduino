#include <arduino.h>
#include <Wire.h>
#include <FS.h>

extern "C" {
#include "miscCommon.h"
}

#define CONST_I2C_SCL_PIN 4 //D2
#define CONST_I2C_SDA_PIN 2 //D4


// I2C address automatic distribution (I2C addr dns)
#define CONST_I2C_DEFAULT_BROARDCAST_ADDR 0x7F
#define CONST_I2C_DEFAULT_TIME_SLOT_INMS 20

#define CONST_I2C_RESET_DATA_LEN 6

#define CONST_I2C_CMD_ADDR_RESET_TO_DEFAULT_ADDR 0xFF01
#define CONST_I2C_CMD_ADDR_RANDOM_TIMESLOT 0xFF02
#define CONST_I2C_CMD_ADDR_REQUEST_ADDR 0xFF03
#define CONST_I2C_CMD_ADDR_FEEDBACK_ADDR 0xFF04
#define CONST_I2C_CMD_ADDR_COMM_TEST_SYN 0xFF05
#define CONST_I2C_CMD_ADDR_COMM_TEST_ACK 0xFF06

//RESET msg. 目的是避免随便发出reset消息
#define CONST_I2C_CMD_ADDR_RESET_STRING "\x52\x45\x53\x45\x54\x83"

typedef struct {
  short CMD;
  char data[CONST_I2C_RESET_DATA_LEN];
} stI2CCMD_reset;

typedef struct {
  short CMD;
  short len;
  short delayInMS;
  short tag;
} stI2CCMD_random;

typedef struct {
  short CMD;
  short seq;
  short tag;
  unsigned long devID;
} stI2CCMD_request;

typedef struct {
  short CMD;
  unsigned long devID;
  uint8_t addr1;
  uint8_t addr2;
} stI2CCMD_feedback;

typedef struct {
  short CMD;
  short data;
} stI2CCMD_test;

typedef union {
  stI2CCMD_reset reset;
  stI2CCMD_random random;
  stI2CCMD_request request;
  stI2CCMD_feedback feedback;
  stI2CCMD_test test;
  uint8_t u8Data[sizeof(stI2CCMD_request)];
} stI2CMDAddrUnion;


#define CONST_I2C_MAX_ADDR_TABLE_LEN 32
#define CONST_I2C_CONFIG_FILE_NAME "/i2c_config.ini"
#define CONST_I2C_FILE_BUFFLEN 100

class I2CAutoAddr
{
  public:
    //data
    uint8_t total; //需要分配的地址总数
    uint8_t curr; //当前分配的地址数
    uint8_t beginAddr; //开始地址

    //func
    bool begin();
    bool begin(uint8_t isMaster, uint8_t addrCount);

    bool bLoad_config();
    bool bSave_config();

  private:
    //data
    uint8_t autoDoneFlag;
    uint8_t masterFlag;
    uint8_t meAddr;
    uint8_t addrTable[32];
    //func
};

bool I2CAutoAddr::begin()
{
  begin(0, 0);
}

bool I2CAutoAddr::begin(uint8_t isMaster, uint8_t addrCount)
{

}

bool I2CAutoAddr::bLoad_config()
{
  bool bRet = false;
  int nLen;
  char *spTemp;
  uint8_t bigBuff[CONST_I2C_FILE_BUFFLEN + 2];
  uint8_t *bufPtr;
  bufPtr = bigBuff;
  //DBGPRINTLN("\n************** bLoad_config begin ****************");

  File configFile = SPIFFS.open(CONST_I2C_CONFIG_FILE_NAME, "r");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file :%s", CONST_I2C_CONFIG_FILE_NAME);
    return bRet;
  }

  while (true)
  {
    short nT1;
    nLen = configFile.readBytesUntil('\n', bufPtr, CONST_I2C_FILE_BUFFLEN);
    if (nLen <= 0)
      break;
    bufPtr[nLen - 1] = '\0'; //trim
    spTemp = strchr((char *)bufPtr, ':');
    if (spTemp == NULL)
      break;//not found;
    spTemp++;

    if (memcmp(bufPtr, "madd", 4) == 0) {
      meAddr = atoi(spTemp);
      if (meAddr < 0 || meAddr > CONST_I2C_DEFAULT_BROARDCAST_ADDR) {
        meAddr = CONST_I2C_DEFAULT_BROARDCAST_ADDR;
      }
    }
    else if (memcmp(bufPtr, "tota", 4) == 0) {
      total = atoi(spTemp);
      if (total < 0 || total > CONST_I2C_DEFAULT_BROARDCAST_ADDR) {
        total = 0;
      }
    }
    else if (memcmp(bufPtr, "badd", 4) == 0) {
      beginAddr = atoi(spTemp);
      if (beginAddr < 0 || beginAddr > CONST_I2C_DEFAULT_BROARDCAST_ADDR) {
        beginAddr = CONST_I2C_DEFAULT_BROARDCAST_ADDR;
      }
    }
  }
  configFile.close();
  //DBGPRINTLN("\n************** bLoad_config end ****************");
  return bRet;

}

bool I2CAutoAddr::bSave_config()
{
  //DBGPRINTLN("--save data--");
  File configFile = SPIFFS.open(CONST_I2C_CONFIG_FILE_NAME, "w");
  if (!configFile)
  {
    DBGPRINTF("Failed to open config file [%s] for writing, then format ... ...", CONST_I2C_CONFIG_FILE_NAME);
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_I2C_CONFIG_FILE_NAME, "w");
    if (!configFile)
    {
      DBGPRINTF("Again! Failed to open config file [%s] for writing", CONST_I2C_CONFIG_FILE_NAME);
      return false;
    }
  }
  //DBGPRINT("before print to file");
  configFile.print("madd:"); //me address
  configFile.println(meAddr);
  configFile.print("tota:"); // total
  configFile.println(total);
  configFile.print("badd:"); // begin address
  configFile.println(beginAddr);

  configFile.close();
  DBGPRINTLN("-- end --");
  return true;

}

I2CAutoAddr gI2CAddr;

void setup() {
  if (!SPIFFS.begin()) {
    return;
  }
  Serial.begin(74880);                          // Initialize serial port
  while (!Serial);                              // wait for serial
  delay(100);                                  // Waits 3 seconds
  Serial.println("\n=============================");          // Sends preamble and Cr+Lf
  Wire.begin(CONST_I2C_SDA_PIN, CONST_I2C_SCL_PIN);                   // Initialize 2-Wire bus
  Serial.println("");
  randomSeed(micros());
}

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
