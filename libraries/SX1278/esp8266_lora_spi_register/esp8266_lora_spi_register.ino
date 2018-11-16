#include <SPI.h>

// GPIO15: SPI_NSS
// GPIO13: SPI_MOSI  miso
// GPIO12: SPI_MISO  mosi
// GPIO14: SPI_CLK
// GPIO16: connects to ESP8266 RST, too
// GPIO0: D0
// Extra Pins
// GPIO2: TX
// GPIO4/5: SDA/SCL

#define REG_VERSION              0x42
//#define RST 16
#define SS 15

uint32_t i = 0;
SPISettings _spiSettings(1E6, MSBFIRST, SPI_MODE0);

uint8_t singleTransfer(uint8_t address, uint8_t value)
{
  uint8_t response;

  digitalWrite(SS, LOW);
  //_spiSettings = SPISettings(frequency, MSBFIRST, SPI_MODE0);
  SPI.beginTransaction(_spiSettings);
  SPI.transfer(address);
  response = SPI.transfer(value);
  SPI.endTransaction();
  digitalWrite(SS, HIGH);

  return response;
}

uint8_t readRegister(uint8_t address)
{
  return singleTransfer(address & 0x7f, 0x00);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("SPI Test");
  //SPI.setHwCs(false); // false as default

  SPI.begin();
  pinMode(SS,OUTPUT);
  //pinMode(RST,OUTPUT);
  
}

void loop() {
  uint8_t res = 0;

  // put your main code here, to run repeatedly:
  //Serial.printf("loop(%d)\r\n",i);
  delay(10);
  //i++;

  res = readRegister(0x42);
  Serial.printf("Register[0x42] = 0x%02X\n\r",res);  //应该返回0x12

}
