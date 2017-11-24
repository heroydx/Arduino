#include <FS.h>

//#define DEBUG_SIO 1
/*
   this application is to read the configuration data of
   COMMON config/APPLICATION config/IRDA config
*/

#define CONST_FILE_NAME_SIZE 100
#define CONST_FILE_LIST_LENGTH 60

char gasFileNameList[CONST_FILE_LIST_LENGTH][CONST_FILE_NAME_SIZE];
char gachFileNameShortcut[CONST_FILE_LIST_LENGTH] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
  'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
  'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'
};
short gnFileNameCount;


#define CONST_ACTION_LIST_SIZE 2
char gasActionList[CONST_ACTION_LIST_SIZE][40] = {
  " (0) to print file ",
  " (1) to delete file "
};

char gachSelectedList[CONST_FILE_LIST_LENGTH];
short gnSelectedCount;


char gnSelected = 0;

#define CONST_STATUS_LIST_FILE 0
#define CONST_STATUS_SELECTED_FILE 1
#define CONST_STATUS_FILE_ACTION 2

int gStatus = CONST_STATUS_LIST_FILE;

#define BUFF_SIZE_LEN 2000

int read_file(char * fileName)
{
  int ret = 0;
  int count;
  int nLen;
  char dispBuff[BUFF_SIZE_LEN];
  File configFile = SPIFFS.open(fileName, "r");
  if (!configFile) {
    Serial.printf("\nFailed to open %s file\n", fileName);
    ret = -1;
    return ret;
  }
  Serial.printf("\nopen file: [%s]", fileName);
  Serial.printf("\nfile size: [%d]", configFile.size());
  Serial.println("\n------");
  count = 0;
  while (true)
  {
    nLen = configFile.readBytesUntil('\n', dispBuff, BUFF_SIZE_LEN);
    if (nLen <= 0)
      break;
    dispBuff[nLen] = 0;
    count++;
    //Serial.printf("\nline:%d [%s]",count,dispBuff);
    Serial.println(dispBuff);
  }
  return ret;
}

int delete_file(char * fileName)
{
  int ret = 0;
  SPIFFS.remove(fileName);
  return ret;
}


void  list_file_name()
{
  short i;
  Serial.println("");
  Serial.println("");
  Serial.println("     [No]: File Name");
  Serial.println("----------------------------------------------------------------------------");
  for (i = 0; i < gnFileNameCount; i++) {
    Serial.printf("     [%c]: [%s]\n", gachFileNameShortcut[i], gasFileNameList[i]);
  }
  Serial.println("");
  Serial.println("---->:");
}

void list_action_opition()
{
  short i;
  Serial.println("");
  Serial.println("");
  Serial.printf("     [%c]: [%s]\n", gachFileNameShortcut[gnSelected], gasFileNameList[gnSelected]);
  Serial.println("----------------------------------------------------------------------------");
  for (i = 0; i < CONST_ACTION_LIST_SIZE; i++) {
    Serial.printf("     %s\n", gasActionList[i]);
  }
  Serial.println("");
  Serial.println("---->:");

}

char serial_read()
{
  char ret = 0;
  if (Serial.available()) {
    ret = Serial.read();
  }
  return ret;
}

void read_dir()
{
  gnFileNameCount = 0;
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.println("----------------------------------------------------------------------------");
    strcpy(gasFileNameList[gnFileNameCount], dir.fileName().c_str());
    gachSelectedList[gnFileNameCount] = gachFileNameShortcut[gnFileNameCount];
    gnFileNameCount++;
  }
  gnSelectedCount = gnFileNameCount;
}

short is_in_selected_list(char chT)
{
  short ret = -1;
  short i;
  for (i = 0; i < gnSelectedCount; i++) {
    if (chT == gachSelectedList[i]) {
      ret = i;
      break;
    }
  }
  return ret;
}

void status_list_file()
{
  short nSelect;
  char chR;
  chR = serial_read();
  if (chR > 0)
  {
    nSelect = is_in_selected_list(chR);
    if (nSelect >= 0) {
      //Serial.print(chR);
      gnSelected = nSelect;
      gachSelectedList[0] = '0';
      gachSelectedList[1] = '1';
      gnSelectedCount = 2;
      list_action_opition();
      gStatus = CONST_STATUS_SELECTED_FILE;
    }
  }
}

void status_selected_file()
{
  short nSelect;
  char chR;
  chR = serial_read();
  if (chR > 0)
  {
    nSelect = is_in_selected_list(chR);
    if (nSelect >= 0) {
      switch (nSelect)
      {
        case 0:
          read_file(gasFileNameList[gnSelected]);
          break;
        case 1:
          delete_file(gasFileNameList[gnSelected]);
          break;
      }
      gStatus = CONST_STATUS_FILE_ACTION;
    }
  }
}



void setup()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount file system");
    return;
  }
  Serial.begin(74880);
  Serial.println("****************************************************************************");
  read_dir();
  for (short i = 0; i < gnFileNameCount; i++) {
    Serial.println("----------------------------------------------------------------------------");
    read_file(gasFileNameList[i]);
  }
  Serial.println();
  Serial.println();
  Serial.println();
  list_file_name();
}

void loop()
{
  switch (gStatus)
  {
    case CONST_STATUS_LIST_FILE:
      status_list_file();
      break;
    case CONST_STATUS_SELECTED_FILE:
      status_selected_file();
      break;
    case CONST_STATUS_FILE_ACTION:
      read_dir();
      list_file_name();
      gStatus = CONST_STATUS_LIST_FILE;
      break;
    default:
      break;
  }
}

