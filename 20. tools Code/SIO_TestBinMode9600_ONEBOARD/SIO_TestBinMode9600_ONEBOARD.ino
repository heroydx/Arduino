#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX
#define SIO_BAUD 9600
#define SENDREV_BUFFLEN 40
#define SIODATA_LEN 7
#define DATAMARK_LEN 6
#define DATAMARK_VOLTAGE 0
#define DATAMARK_CURRENT 1
#define DATAMARK_ACTIVEPOWER 2
#define DATAMARK_READPOWER 3
#define DATAMARK_SETIP 4
#define DATAMARK_SETALARM 5

uint8_t sendBuff[SENDREV_BUFFLEN+1];

uint8_t ascii(int hex)
{
  uint8_t cc;
  cc = 0x30;
  switch (hex)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
    cc = 0x30 + hex;
    break;
  case 0x0A:
  case 0x0B:
  case 0x0C:
  case 0x0D:
  case 0x0E:
  case 0x0F:
    cc = 'A' + hex - 0x0A;
  }
//Serial.printf("hex=[%x]\n",hex);
//Serial.printf("cc=[%x]\n",cc);
  return cc;
}

uint8_t ascii2hex(uint8_t cc)
{
  uint8_t hex;
  hex = 0x00;
  switch (cc)
  {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    hex = cc - 0x30;
    break;
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
    hex = cc - 'a' + 0x0A;
    break;
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    hex = cc - 'A' + 0x0A;
  }
  return hex;
}


uint8_t chCheckSum(uint8_t *pBuff,int nLen){
  uint8_t i,val=0;
  for (i=0;i<nLen;i++){
    val+=pBuff[i];
  }
  return (val&0xFF);
}

void getRequestData(int type, uint8_t *pBuff){
  char chSum=0;
  //int i;char bigBuff[100];
  pBuff[0]= uint8_t (0xB0+type)&0xFF;
  pBuff[1]=0xC0;
  pBuff[2]=0xA8;
  pBuff[3]=0x01;
  pBuff[4]=0x01;
  pBuff[5]=0x00;
  if (type==DATAMARK_SETALARM){
    pBuff[5]=0x14;
  }
  pBuff[6]=chCheckSum(pBuff,SIODATA_LEN-1);
  /*
  Serial.println("getData");
  for (i=0;i<SIODATA_LEN;i++){
    sprintf(bigBuff,"%d:%02X ",i,pBuff[i]);
    Serial.print(bigBuff);
  }
  Serial.println();
*/
};

int hexString2ascii(uint8_t *pBuff, char *dispP,int nLen){
  int i,k=0;
  char chT;
  for (i=0;i<nLen;i++){
    chT=pBuff[i]>>4;
    dispP[k]=ascii(chT);
    k++;
    chT=pBuff[i]&0xF;
    dispP[k]=ascii(chT);
    k++;
  }
  dispP[k]=0;
  return k;
}

void setup()  
{
  int i,k;
  char dispBuff[50];
  //char bigBuff[100];
  //init data
  //asDataMark[DATAMARK_VOLTAGE][]={0xB0,0xC0,0xA8,0x01,0x01,0x00};
  // Open serial communications and wait for port to open:
  Serial.begin(SIO_BAUD);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
    }
  Serial.println("SIO TEST BEGIN:");
  // set the data rate for the SoftwareSerial port
  mySerial.begin(SIO_BAUD);
  //mySerial.println("Hello, world?");
  
  /*
  for (i=0;i<DATAMARK_LEN;i++){
    //sprintf(bigBuff,"%d",i);
    //Serial.println(bigBuff);
    
    getRequestData(i,sendBuff);
    hexString2ascii(sendBuff,dispBuff,SIODATA_LEN);
    //sprintf(bigBuff,"%s",dispBuff);
    Serial.println(dispBuff);
    mySerial.println(dispBuff);
    delay(500);
    
  }
  */
  for (k=0;k<4;k++){
       sprintf(dispBuff,"%2d:",k);
       Serial.println(dispBuff);
       sendRequestData(k);
    
      delay(100);
  }
}

void sendRequestData(int type)
{
   int i;
   char dispBuff[50];
   getRequestData(type,sendBuff);
   for (i=0;i<SIODATA_LEN;i++){
     mySerial.write(sendBuff[i]);
     sprintf(&dispBuff[i*3],"%02X ",sendBuff[i]);
   }
   dispBuff[3*SIODATA_LEN]=0;
   Serial.println(dispBuff);
}

int nCount=0;

void loop() // run over and over
{

  /*   if( nCount<6){
      getRequestData(nCount,sendBuff);
      hexString2ascii(sendBuff,dispBuff,SIODATA_LEN);
      Serial.println("in LOOP:");
      mySerial.println(dispBuff);
      nCount++;
      delay(10);
   }
   sendRequestData(nCount);
   nCount++;
   if (nCount>3) 
      nCount=0;
   */
      
   if (mySerial.available())
   {

      char chT;
      char achDisp[10],achData[10];
      int i,nLen;
      
      chT=mySerial.read();
      //Serial.write(mySerial.read());
      achData[0]=chT;
      achData[1]=0;
      nLen=hexString2ascii(achData,achDisp,1);
      for (i=0;i<nLen;i++){
          Serial.write(achDisp[i]);
      }
      Serial.write(' ');
      nCount++;
      if (nCount%35==0)
          Serial.println();
   }
   if (Serial.available())
   {
      char chT;
      
      chT=Serial.read();
      switch(chT){
        case '0':
           Serial.println();
           Serial.write(chT);
           Serial.write(":");
           sendRequestData(0);
           break;
        case '1':
           Serial.println();
           Serial.write(chT);
           Serial.write(":");
           sendRequestData(1);
           break;           
        case '2':
           Serial.println();
           Serial.write(chT);
           Serial.write(":");
           sendRequestData(2);
           break;      
        case '3':
           Serial.println();
           Serial.write(chT);
           Serial.write(":");
           sendRequestData(3);
           break;      
      }
      
   }
   /*
   if (Serial.available())
      mySerial.write(Serial.read());
  */    
  //sendRequestData(0);
  //delay(1000);
}

