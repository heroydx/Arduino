//Modified by Steven Lian on 2017/06/11
#ifndef LVIRDA_H
#define LVIRDA_H

#define CONST_IRDA_VERSION 170614

#define CONST_IRDA_MAXSYNCCOUNT 8
#define CONST_IRDA_MAXPARAMLEN  4*16
#define CONST_IRDA_MAXDATACOUNT 2
#define CONST_IRDA_MAXSTOPLEN 4*8

//如果是集电极放大电路，正好高低电平反向
#define CONST_IRDA_HIGH_LEVEL 1
#define CONST_IRDA_LOW_LEVEL 0
//#define CONST_IRDA_HIGH_LEVEL 0
//#define CONST_IRDA_LOW_LEVEL 1

typedef struct
{
  short nLen;
  char achParam[CONST_IRDA_MAXPARAMLEN + 1];
  char achSign[2];
} ST_SUBFIXED;

typedef struct
{
  ST_SUBFIXED sHead;
  short nSyncCount;
  ST_SUBFIXED asSync[CONST_IRDA_MAXSYNCCOUNT];
  ST_SUBFIXED sStop;
  short nDataCount;
  ST_SUBFIXED asData[CONST_IRDA_MAXDATACOUNT];
} ST_FIXED;


#define CONST_IRDA_FILE_NAME "/irda.data"
#define CONST_IRDA_FILE_SIZE 1024
//lv IRDA port 14/5
#define CONST_IRDA_SEND_PORT 14
#define CONST_IRDA_RECV_PORT 5

//#define CONST_IRDA_SEND_PORT 12
//#define CONST_IRDA_RECV_PORT 13

#define CONST_IRDA_MAX_FIXCODELEN 120
#define CONST_IRDA_DATA_LEN 600
#define CONST_IRDA_MAX_RECV_COUNT 5
#define CONST_IRDA_MIN_FIXCODELEN 6
#define CONST_IRDA_MAX_SPLIT_MS 1

// check data valid
#define CONST_IRDA_MIN_RECE_DATA_LEN 8
#define CONST_IRDA_MAX_RECE_DATA_VAL 45000
#define CONST_IRDA_MIN_RECE_DATA_VAL 20
#define CONST_IRDA_AVG_RECE_DATA_LEN 200
#define CONST_IRDA_MIN_RECE_DATA_DIFF 2
#define CONST_IRDA_MAX_RECE_DATA_DIFF 16
#define CONST_IRDA_LEN_DIFF_RATE 4
#define CONST_IRDA_MAX_RECE_DATA_SYNC_LEN 18500
#define CONST_IRDA_MAX_RECE_DATA_SYNC_COUNT 8


#define CONST_IRDA_ERR_MIN_DATA_LEN -1
#define CONST_IRDA_ERR_MIN_MAX_DATA_VAL -2
#define CONST_IRDA_ERR_MAX_RECE_DATA_DIFF -3
#define CONST_IRDA_ERR_RANDOM_DATA -4

//irda frequency
#define CONST_IRDA_38K 38
#define CONST_IRDA_56K 56
#define CONST_IRDA_38K_LOOP_TIME 26
#define CONST_IRDA_38K_LOW_DELAY 25
#define CONST_IRDA_38K_HIGH_DELAY 12
//#define CONST_IRDA_38K_HIGH_DELAY 8
#define CONST_IRDA_38K_DIFF 2

#define CONST_IRDA_56K_LOOP_TIME 17
#define CONST_IRDA_56K_LOW_DELAY 16
#define CONST_IRDA_56K_HIGH_DELAY 7
//#define CONST_IRDA_56K_HIGH_DELAY 5
#define CONST_IRDA_56K_DIFF 5

typedef struct  {
  short fixCodeLen;
  char fixCode[CONST_IRDA_MAX_FIXCODELEN + 1];
  short sCount;
  short sLen;
  unsigned short sData[CONST_IRDA_DATA_LEN + 128];
  short rLen;
  short rPreLen;
  short rErrLen;
  short rCheckCount;
  unsigned short rData[CONST_IRDA_DATA_LEN];
  unsigned long ulCurrIrdaTick;
  unsigned long ulPreIrdaTick;

} stIrDaData;


//head
#ifndef LVENCODE_H
//int int2hex(long nValue,int nZeroFill, int nLen,char *pBuff);
#endif

class LVIrda
{
  public:
    void begin();
    short recvPort;
    short sendPort;
    ST_FIXED gsFixed;

    LVIrda();
    void fixcode(char *fixCode);
    int getFixcode(char *buff);
    int recvAvailable();
    short getRecvDataLen();
    int irsend(short *nP, short nLen);
    int irsend6(char *strP, char*buffP);
    int irsend100(char *strP);
    int prepare3003(char *buffP);
    void clean3003();

    unsigned short get_version();

  private:
    unsigned short version;
    int nLoopCount;
    short anDiffDataArray[CONST_IRDA_MAX_RECE_DATA_DIFF + 2];
    short nDiffDataCount;
    short nSyncDataCount;

    int decodeElevel(char *strParam, short *pn, int maxElevel);
    int param2eLevel(char *param, char *strOut);

    int lv_int2hex(long nValue, int nFillLen, int nBigEnding, char *pBuff);
    void enableRecv();
    void enableRecv(short port);
    void disableRecv();

    int recvValid();

    int nCall_send_irda_data(int frequency);
    void vIR_SendKHZ(int x, int y, short highDelay, short lowDelay); //产生38KHZ红外脉冲

    void vReset_interval(unsigned long &resetTick);
    unsigned long ulGet_interval(unsigned long checkTick);

    int hex2int(char *hex);
    char ch_int2hex(int n);
    int l4_hex2int(char *lhex);
    char chHighLowSwitch(char chSwitch);

    int decodeFixedCode(char *fixedCode);
    int decodeFixedSub(char *strIn, ST_SUBFIXED *psSub);
    int decodeParam(char *strIn, char *strOut);
    int decodeParamSub(char *strOut, int nPos, char *pchLastSign, ST_SUBFIXED *pS);
    bool bLoad_Fixcode();
    bool bSave_Fixcode();

};

void vIrda_recv_interrupt();

#endif // LVIRDA_H