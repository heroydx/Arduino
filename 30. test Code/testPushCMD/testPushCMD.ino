#include <arduino.h>
#include "ListArray.h"

extern "C" {
#include "miscCommon.h"
}

#define CONST_SERVER_COMMAND_LIST_LEN 30
#define CONST_CMD_3001 3001
#define CONST_CMD_3002 3002
#define CONST_CMD_3003 3003
#define CONST_CMD_9999 9999

typedef struct {
  short command;
  unsigned long tick;
  unsigned long beginTick;
  unsigned long endTick;
} stLVCommmandList;

static ListArray serverCommand(CONST_SERVER_COMMAND_LIST_LEN, sizeof(stLVCommmandList));
static stLVCommmandList filterData;
static stLVCommmandList sServerCommand;

void print_cmd_list_info(stLVCommmandList *stPtr)
{
  Serial.print("\n-----------BEGIN-----------------");
  Serial.printf("\n cmd [%d]", stPtr->command);
  Serial.printf(" tick [%d]", stPtr->tick);
  Serial.printf("\n begin [%d]", stPtr->beginTick);
  Serial.printf("\n end [%d]", stPtr->endTick);
  Serial.print("\n-----------END-----------------");
}

static void private_filter_server_command()
{
  short i;
  short nLen;
  short cmd = 0;
  nLen = serverCommand.len();
  if (nLen < 2)
    return;
  for (i = 0; i < nLen; i++)
  {
    short pos;
    // to check repeat data ; only check 3001 command
    pos = serverCommand.lpop(&filterData);
    //DBGPRINTF("\n=== filter [%d] pos [%d],len[%d] head[%d],tail[%d]",filterData.command,pos,serverCommand.len(),serverCommand.head,serverCommand.tail);

    if (cmd == CONST_CMD_3001) {
      if ((filterData.command != CONST_CMD_3001) || (filterData.beginTick != 0) || (filterData.endTick != 0)) {
        //if ((filterData.command != CONST_CMD_3001)) {
        //not 3001 or special 3001
        serverCommand.rpush(&filterData);
      }
      //=3001, trash
    }
    else {
      //if (filterData.command == CONST_CMD_3001 && filterData.beginTick==0) {
      if (filterData.command == CONST_CMD_3001 ) {
        //first 3001
        cmd = CONST_CMD_3001;
      }
      serverCommand.rpush(&filterData);
    }
  }
}

class LVCommon
{
  public:

    void push_cmd(short cmd, short beginTime, short endTime);
    void push_cmd(short cmd);
    short pop_cmd();
  private:


};

void ICACHE_FLASH_ATTR LVCommon::push_cmd(short cmd, short beginTime, short endTime)
{
  short pos;
  sServerCommand.command = cmd;
  sServerCommand.tick = ulReset_interval();
  sServerCommand.beginTick = (unsigned long) beginTime * 1000L;
  sServerCommand.endTick = (unsigned long) endTime * 1000L;
  pos = serverCommand.rpush(&sServerCommand);
  //DBGPRINTF("\n=== push_cmd [%d] pos [%d],len[%d] head[%d],tail[%d]",cmd,pos,serverCommand.len(),serverCommand.head,serverCommand.tail);
}

void ICACHE_FLASH_ATTR LVCommon::push_cmd(short cmd)
{
  push_cmd(cmd, 0, 0);
}


short ICACHE_FLASH_ATTR LVCommon::pop_cmd()
{
  short ret = 0;
  short pos;
  DBGPRINTF("\n=== serverCommand len =[%d] ", serverCommand.len());
  private_filter_server_command();
  pos = serverCommand.lpop(&sServerCommand);
  //DBGPRINTF("\n=== pop [%d] pos [%d],len[%d] head[%d],tail[%d]",filterData.command,pos,serverCommand.len(),serverCommand.head,serverCommand.tail);
  //DBGPRINTF("serverCommand=[%d]\n", sServerCommand.command);
  if (sServerCommand.beginTick != 0 || sServerCommand.endTick != 0)
  {
    //时间判断;
    if (ulGet_interval(sServerCommand.tick) < sServerCommand.beginTick) {
      //时间没有到，重新放回队列
      serverCommand.rpush(&sServerCommand);
      ret = 0;
    }
    else {
      if (ulGet_interval(sServerCommand.tick) < sServerCommand.endTick) {
        //合适，去执行这个命令
        ret = sServerCommand.command;
        Serial.printf("\n === meet === cmd [%d] ret[%d]", sServerCommand.command, ret);
      }
      else {
        //过期，抛弃这个命令
        ret = 0;
      }
    }
  }
  else {
    ret = sServerCommand.command;
  }
  DBGPRINTF(" [%d]", ret);
  Serial.printf("\n === RET === [%d]", ret);
  return ret;
}

LVCommon LVDATA;

void debug_print()
{
  short i;
  for (i = 0; i < serverCommand.len(); i++) {
    Serial.printf("\n No:[%d]", i);
    serverCommand.index(&sServerCommand, i);
    print_cmd_list_info(&sServerCommand);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(78440);
  Serial.println("begin");
  LVDATA.push_cmd(CONST_CMD_3001);
  LVDATA.push_cmd(CONST_CMD_3002);
  LVDATA.push_cmd(CONST_CMD_3001, 5, 30);
  LVDATA.push_cmd(CONST_CMD_3003);
  LVDATA.push_cmd(CONST_CMD_9999);
  debug_print();
}

void loop() {
  // put your main code here, to run repeatedly:
  short cmd;
  short count = 0;
  unsigned long tick;
  short nLen;
  nLen = serverCommand.len();
  if (nLen > 0) {
    Serial.println("\n---------- BEGIN --------------------------------------------------");
    cmd = LVDATA.pop_cmd();
    tick = millis();
    Serial.printf("\n count:[%d],tick:[%d], cmd:[%d]", count, tick, cmd);
    count++;
    debug_print();
    delay(500);
    Serial.println("\n---------- END --------------------------------------------------");

  }

}
