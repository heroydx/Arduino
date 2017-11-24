#include <arduino.h>

#include "LoRaMAC.h"
#include "FM_ZCJ_SM.h"

#define SIO_BAUD 115200


#define TEST_MODE_LEN 11
stDataStreamUnion toTest[TEST_MODE_LEN];
stDataStreamUnion toRecv;

unsigned long ulDevID = 12345678L;
uint8_t u8SeedKey[LORA_MAC_KEY_LEN];
uint8_t u8SeedVal[LORA_MAC_KEY_MD5];


void prepare_test_data()
{
  uint8_t i;
  //ulDevID = generate_devID();
  ulDevID = ESP.getChipId();
  random_string((char *) u8SeedKey, LORA_MAC_KEY_LEN);
  simpleHash((char *) u8SeedKey, (char *)  u8SeedVal, ulDevID, LORA_MAC_KEY_LEN);
  for (i = 0; i < TEST_MODE_LEN; i++) {
    //DBGPRINTLN(i);
    switch (i)
    {
      case LORA_CMD_EXTEND_CMD:
        toTest[i].exCMD.CMD = LORA_CMD_EXTEND_CMD;
        memcpy(toTest[i].exCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].exCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].exCMD.seq = (uint8_t) i;
        toTest[i].exCMD.exCMD = 1;
        break;
      case LORA_CMD_REGISTRATION_REQUEST:
        toTest[i].regRequest.CMD = LORA_CMD_REGISTRATION_REQUEST;
        memcpy(toTest[i].regRequest.sourceAddr, "\x00\x00\x00\x00", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regRequest.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].regRequest.seq = (uint8_t) i;
        toTest[i].regRequest.devID = ulDevID;
        toTest[i].regRequest.MAID = 8;
        toTest[i].regRequest.PID = 1;
        memcpy(toTest[i].regRequest.key, u8SeedKey, LORA_MAC_KEY_LEN);

        break;
      case LORA_CMD_REGISTRATION_FEEDBACK:
        toTest[i].regFeedback.CMD = LORA_CMD_REGISTRATION_FEEDBACK;
        memcpy(toTest[i].regFeedback.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regFeedback.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].regFeedback.seq = (uint8_t) i;
        toTest[i].regFeedback.devID = ulDevID;
        memcpy(toTest[i].regFeedback.addr, "\xC0\xA8\x00\x02", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].regFeedback.val, u8SeedVal, LORA_MAC_KEY_MD5);
        break;
      case LORA_CMD_ADDR_REQUEST:
        break;
      case LORA_CMD_ADDR_FEEDBACK:
        break;
      case LORA_CMD_SYNC_CMD:
        toTest[i].syncCMD.CMD = LORA_CMD_SYNC_CMD;
        memcpy(toTest[i].syncCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].syncCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].syncCMD.seq = (uint8_t) i;
        memcpy(toTest[i].syncCMD.groupAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN - 1);
        toTest[i].syncCMD.groupLen = 8;
        for (short k = 1; k < toTest[i].syncCMD.groupLen + 1; k++) {
          toTest[i].syncCMD.list[k] = k;
        }
        break;
      case LORA_CMD_SYNC_TIME:
        break;
      case LORA_CMD_SYNC_RANDOM:
        toTest[i].syncCMD.CMD = LORA_CMD_SYNC_RANDOM;
        memcpy(toTest[i].syncCMD.sourceAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN);
        memcpy(toTest[i].syncCMD.destAddr, "\xFF\xFF\xFF\xFF", LORA_MAC_ADDRESS_LEN);
        toTest[i].syncCMD.seq = (uint8_t) i;
        toTest[i].syncCMD.groupLen = 16;
        memcpy(toTest[i].syncCMD.groupAddr, "\xC0\xA8\x00\x01", LORA_MAC_ADDRESS_LEN - 1);
        break;
      case LORA_CMD_VERIFY_REQUEST:
        break;
      case LORA_CMD_VERIFY_FEEDBACK:
        break;
      default:
        break;
    }
  }
}



LoRaMAC LoRaMacData;
FM_ZCJ FMDevice;
bool isHost = true;



void setup()
{
  if (!SPIFFS.begin())
  {
    //DBGPRINTLN("Failed to mount file system");
    return;
  }
  // Open serial communications and wait for port to open:
#ifdef DEBUG_SIO
  DEBUG_SIO.begin(SIO_BAUD);
  while (!DEBUG_SIO) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  DBGPRINTLN();
  DBGPRINTLN();
  DBGPRINTLN("Serial Port for debug is ready on : ");
  DBGPRINTLN(SIO_BAUD);
#endif

  DBGPRINTLN("---- the size of dataStream ----");
  DBGPRINTF(" % d\n", sizeof(stDataStreamUnion));

  DBGPRINTLN("---- the size of stDataCMD_APP ----");
  DBGPRINTF(" % d\n", sizeof(stDataCMD_APP));

  DBGPRINTLN("\nPrepare test data");
  prepare_test_data();
  DBGPRINTLN("\nprint test data");
  for (short k = 0; k < TEST_MODE_LEN; k++) {
    DBGPRINTF("\nNo. %d", k);
    LoRaMacData.debug_print_union(toTest[k].u8Data);
  }

  // set the data rate for the SoftwareSerial port
  if (isHost) {
    uint8_t hostAddr[LORA_MAC_ADDRESS_LEN + 2];
    hostAddr[0] = 0xc0;
    hostAddr[1] = 0xa8;
    hostAddr[2] = 0x0;
    hostAddr[3] = 0x1;
    DBGPRINTLN("== LORA HOST SERVER===");
    LoRaMacData.begin(isHost, 433, hostAddr, 5, 248);

  }
  else {
    LoRaMacData.begin(isHost, 433);

  }
  LoRaMacData.debug_print_self();

  //FMDevice.begin();

}

void test_handle_recv_data()
{
  short CMD;
  short nLen;
  nLen = sizeof (stDataStreamUnion);

  CMD = toRecv.exCMD.CMD;
  //public Part
  switch (CMD)
  {
    case LORA_CMD_EXTEND_CMD:
      break;
    case LORA_CMD_REGISTRATION_REQUEST:
      break;
      DBGPRINTLN("\n-- GOT LORA_CMD_REGISTRATION_REQUEST --");

      DBGPRINTLN("\n-- will send back LORA_CMD_REGISTRATION_FEEDBACK information --");
      toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.devID = toRecv.regRequest.devID;
      simpleHash((char *) toRecv.regRequest.key, (char *)  toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.val, toTest[LORA_CMD_REGISTRATION_FEEDBACK].regFeedback.devID, LORA_MAC_KEY_LEN);

      //LoRaMacData.send_data(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);
      //LoRaDev.send(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data, nLen);
      LoRaMacData.debug_print_union(toTest[LORA_CMD_REGISTRATION_FEEDBACK].u8Data);

      break;
    case LORA_CMD_REGISTRATION_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_REGISTRATION_FEEDBACK ==");
      break;
    case LORA_CMD_ADDR_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_ADDR_REQUEST ==");
      break;
    case LORA_CMD_ADDR_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_ADDR_FEEDBACK ==");
      break;
    case LORA_CMD_SYNC_CMD:
      DBGPRINTLN("\n== LORA_CMD_SYNC_CMD ==");
      break;
    case LORA_CMD_SYNC_TIME:
      DBGPRINTLN("\n== LORA_CMD_SYNC_TIME ==");
      break;
    case LORA_CMD_SYNC_RANDOM:
      DBGPRINTLN("\n== LORA_CMD_SYNC_RANDOM ==");
      break;
    case LORA_CMD_VERIFY_REQUEST:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_REQUEST ==");
      break;
    case LORA_CMD_VERIFY_FEEDBACK:
      DBGPRINTLN("\n== LORA_CMD_VERIFY_FEEDBACK ==");
      break;
    case LORA_CMD_APP_FMRADIO:
      DBGPRINTLN("\n== LORA_CMD_APP_FMRADIO ==");
      FMDevice.handle_Server_to_FM_command((char *)toRecv.app.data);
      break;
    default:
      DBGPRINTLN("\n== application call ==");
      //application call to call external application API
      DBGPRINTF("\napp CMD: [%d]\n", CMD);
      break;
  }
}

void loop() // run over and over
{
  if (Serial.available()) {
    uint8_t chR;
    short nLen;
    short nNum = 0;
    chR = Serial.read();
    switch (chR)
    {
      case '1':
        nNum = 0;
        break;
      case '2':
        nNum = 1;
        break;
      case '3':
        nNum = 2;
        break;
      case '4':
        nNum = 3;
        break;
      case '5':
        nNum = 4;
        break;
      case '6':
        nNum = 5;
        break;
      case '7':
        nNum = 6;
        break;
      case '8':
        nNum = 7;
        break;
      case '9':
        nNum = 8;
        break;
      case 'a':
        nNum = 9;
      case 'b':
        nNum = 10;
        break;
      case 's':
        //send_data
        DBGPRINTF("\n LoRaMacData.send_data [%d]", LoRaMacData.send_data(toTest[LORA_CMD_EXTEND_CMD].u8Data));
        nNum = -101;
        break;
      case 't':
        //send_data
        memcpy(LoRaMacData.emergencyData.u8Data, toTest[LORA_CMD_EXTEND_CMD].u8Data, sizeof (stDataStreamUnion));
        DBGPRINTLN("\n emergencyData:");
        LoRaMacData.debug_print_union(LoRaMacData.emergencyData.u8Data);
        nNum = -102;
        break;
      default:
        nNum = -1;
        break;
    }
    if (nNum >= 0) {
      nLen = sizeof (stDataStreamUnion);
      DBGPRINTLN("\n--------- will send begin ---------");
      LoRaMacData.debug_print_union(toTest[nNum].u8Data);
      LoRaMacData.send_data(toTest[nNum].u8Data);
      //LoRaMacData.send(toTest[nNum].u8Data, nLen);
      DBGPRINTLN("--------- will send end ---------\n");
    }
  }
  short nLen;
  //nLen = LoRaDev.available();
  nLen = LoRaMacData.available();
  if (nLen) {
    memcpy(toRecv.u8Data, LoRaMacData.recvData.u8Data, sizeof(stDataStreamUnion));
    LoRaMacData.debug_print_union(toRecv.u8Data);
    Serial.println("\n--LoRa received Data");
    Serial.println(nLen);
    test_handle_recv_data();
    LoRaMacData.debug_print_self();
  }
  if (LoRaMacData.hostFlag)
  {
    //host


  }
  //FMDevice.FM_loop();
}


