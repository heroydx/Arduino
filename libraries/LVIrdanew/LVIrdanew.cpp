#include <string.h>
#include <stdio.h>
#include <arduino.h>
#include <FS.h>

extern "C" {
#include "gpio.h"
#include "eagle_soc.h"
#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
  //#include "driver/i2s.h"
#include "i2s.h"
  //#include "driver/sigma_delta.h"
#include "sigma_delta.h"
}

#include "lVIrdanew.h"
/*
  irData receive and send
  steven.lian@gmail.com 2016/12/21
*/

volatile stIrDaData gsIrda;
unsigned long gl100msIrDaTick;


LVIrda::LVIrda()
{
  nLoopCount = 1;
}

void ICACHE_FLASH_ATTR LVIrda::begin()
{
  void *strPtr;
  pinMode(CONST_IRDA_RECV_PORT, INPUT);
  pinMode(CONST_IRDA_SEND_PORT, OUTPUT);
  ir_tx_init();
  enableRecv();
  strPtr = (void *) &gsIrda;
  memset(strPtr, 0, sizeof(gsIrda));
  bLoad_Fixcode();
  vReset_interval(gl100msIrDaTick);

}

void LVIrda::enableRecv()
{
  enableRecv(CONST_IRDA_RECV_PORT);
}


void LVIrda::enableRecv(short port)
{
  attachInterrupt(port, vIrda_recv_interrupt, CHANGE);
}

void LVIrda::disableRecv()
{
  detachInterrupt(CONST_IRDA_RECV_PORT);
}


void vIrda_recv_interrupt()
{
  //nRecvBusy = 1;
  gsIrda.ulCurrIrdaTick = micros();
  if (gsIrda.rLen == 0)
  {
    gsIrda.rData[0] = (unsigned short) gsIrda.ulCurrIrdaTick;
  }
  else
  {
    gsIrda.rData[gsIrda.rLen] = (unsigned short) ( gsIrda.ulCurrIrdaTick - gsIrda.ulPreIrdaTick);
  }
  gsIrda.ulPreIrdaTick = gsIrda.ulCurrIrdaTick;
  /*
    if (gsIrda.rData[gsIrda.rLen] < 0) {
    gsIrda.rData[gsIrda.rLen] += 0xffffffff;
    }
  */
  gsIrda.rLen++;
  if (gsIrda.rLen >= CONST_IRDA_DATA_LEN)
    gsIrda.rLen = 0;
}

void LVIrda::fixcode(char *fixedCode)
{
  char *strPtr;
  strPtr = (char *) gsIrda.fixCode;
  gsIrda.fixCodeLen = strlen(fixedCode);
  strncpy(strPtr, fixedCode, CONST_IRDA_MAX_FIXCODELEN);
  decodeFixedCode(fixedCode);
  bSave_Fixcode();
}

int ICACHE_FLASH_ATTR LVIrda::irsend(short *nP, short nLen)
{
  int nRet = 0;
  short *nPtr;
  nPtr = (short *) gsIrda.sData;
  if (nLen < CONST_IRDA_DATA_LEN) {
    memcpy(nPtr, nP, (sizeof(short)*nLen));
    gsIrda.sLen = nLen;
    nRet = nCall_send_irda_data(CONST_IRDA_38K);
  }
  return nRet;
}


int ICACHE_FLASH_ATTR LVIrda::irsend6(char *strP, char *buffP)
{
  int nRet = 0;

  if (gsIrda.fixCodeLen > CONST_IRDA_MIN_FIXCODELEN)
  { //fixcode is available
    // should use LVDATA.bigBuff as buff
    short *nPtr;
    nPtr = (short *) gsIrda.sData;
    param2eLevel(strP, buffP);
    gsIrda.sLen = (short) decodeElevel(buffP, nPtr, CONST_IRDA_DATA_LEN);
    if (gsIrda.fixCode[0] == '3') {
      nRet = nCall_send_irda_data(CONST_IRDA_56K);
    }
    else {
      nRet = nCall_send_irda_data(CONST_IRDA_38K);
    }
  }
  else
  {
    nRet = -1;
  }
  return nRet;
}

int ICACHE_FLASH_ATTR LVIrda::irsend100(char *strP)
{
  int nRet = 0;
  short *nPtr;
  nPtr = (short *) gsIrda.sData;
  gsIrda.sLen = (short) decodeElevel(strP, nPtr, CONST_IRDA_DATA_LEN);
  nRet = nCall_send_irda_data(CONST_IRDA_38K);
  return nRet;
}

int ICACHE_FLASH_ATTR LVIrda::prepare3003(char *buffP)
{
  int i;
  char *strPtr;
  strPtr = buffP;
  lv_int2hex(gsIrda.rLen, 4, 1, strPtr);
  strPtr += 4;
  //first data remove
  for (i = 1; i < gsIrda.rLen; i++)
  {
    if (gsIrda.rData[i] < 0) {
      gsIrda.rData[i] += 0xffffffff;
    }
    lv_int2hex(gsIrda.rData[i], 4, 1, strPtr);
    strPtr += 4;
  }
  // add last irda data
  lv_int2hex(65000, 4, 1, strPtr);
  return (strPtr - buffP);

}

void LVIrda::vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}

unsigned long LVIrda::ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}


short ICACHE_FLASH_ATTR LVIrda::getRecvDataLen()
{
  return gsIrda.rErrLen;
};


int ICACHE_FLASH_ATTR LVIrda::recvValid()
{
  int nRet = 0;//valid default
  short i, k, nT1;
  bool isInDiff;
  nDiffDataCount = 0;
  nSyncDataCount = 0;
  memset(anDiffDataArray, 0, sizeof(anDiffDataArray));
  if (gsIrda.rLen <= CONST_IRDA_MIN_RECE_DATA_LEN) {
    //invalid data
    nRet = -1;

  }
  else {

    //check if have too big data or have too many different data length;
    for (i = 1; i < gsIrda.rLen; i++) {
      //have too big or small data?
      if ((gsIrda.rData[i] < CONST_IRDA_MIN_RECE_DATA_VAL) || (gsIrda.rData[i] > CONST_IRDA_MAX_RECE_DATA_VAL)) {
        //Serial.printf("\n\r---- -2:%d\n\r",gsIrda.rData[i]);
        nRet = CONST_IRDA_ERR_MIN_MAX_DATA_VAL;
        break;
      }
      // too many sync data?
      if (gsIrda.rData[i] > CONST_IRDA_MAX_RECE_DATA_SYNC_LEN) {
        nSyncDataCount++;
      }
      // random data?
      if (gsIrda.rData[i] > CONST_IRDA_AVG_RECE_DATA_LEN) {
        nT1 = (short)gsIrda.rData[i] / CONST_IRDA_AVG_RECE_DATA_LEN;
      } else {
        nT1 = - (short)gsIrda.rData[i] / (CONST_IRDA_AVG_RECE_DATA_LEN / 4);
      }
      isInDiff = false;
      for (k = 0; k < nDiffDataCount; k++) {
        if (nT1 == anDiffDataArray[k]) {
          isInDiff = true;
          break;
        }
      }
      if (isInDiff == false) {
        anDiffDataArray[nDiffDataCount] = nT1;
        nDiffDataCount++;
        // check sync data count valid
        if (nDiffDataCount >= CONST_IRDA_MAX_RECE_DATA_DIFF) {
          nRet = CONST_IRDA_ERR_MAX_RECE_DATA_DIFF;
          break;
        }
      }
    }
    // check if random data
    if ((nDiffDataCount < CONST_IRDA_MIN_RECE_DATA_DIFF) || ((gsIrda.rLen / nDiffDataCount) < CONST_IRDA_LEN_DIFF_RATE)) {
      nRet = CONST_IRDA_ERR_RANDOM_DATA;
    }
    if (nDiffDataCount >= CONST_IRDA_MAX_RECE_DATA_DIFF) {
      nRet = CONST_IRDA_ERR_MAX_RECE_DATA_DIFF;
    }

  }
  return nRet;
}


int ICACHE_FLASH_ATTR LVIrda::recvAvailable()
{
  int nRet = 1;//invalid data
  if ((ulGet_interval(gl100msIrDaTick) > 100)) {
    if (gsIrda.rLen > 0) {
      if (gsIrda.rLen == gsIrda.rPreLen) {
        gsIrda.rCheckCount++;
      }

      if (gsIrda.rCheckCount >= CONST_IRDA_MAX_SPLIT_MS) {
        //data received done
        nRet = recvValid();
        if (nRet) {
          // invalid Data, clean up
          //DBGPRINTLN("====invalid Data,clean up=====");
          gsIrda.rErrLen = gsIrda.rLen;
          clean3003();
        }
        else {
          nRet = 0; //valid data
        }
      }
      gsIrda.rPreLen = gsIrda.rLen;
    }
    vReset_interval(gl100msIrDaTick);
  }
  return nRet;
}

void LVIrda::clean3003()
{
  gsIrda.rLen = 0;
  gsIrda.rPreLen = 0;
  gsIrda.rCheckCount = 0;
}



void ICACHE_FLASH_ATTR LVIrda::vIR_SendKHZ(int x, int y, short highDelay, short lowDelay) //产生38KHZ红外脉冲
{
  short i;

  for (i = 0; i < x; i++) //15=386US
  {
    if (y == 1)
    {
      digitalWrite(CONST_IRDA_SEND_PORT, 1);
      delayMicroseconds(highDelay);
      digitalWrite(CONST_IRDA_SEND_PORT, 0);
      //delayMicroseconds(lowDelay-highDelay);
      delayMicroseconds(highDelay);
    }
    else
    {
      digitalWrite(CONST_IRDA_SEND_PORT, 0);
      delayMicroseconds(lowDelay);
    }
  }
}



int ICACHE_FLASH_ATTR LVIrda::nCall_send_irda_data(int frequency)
{
  int i;
  int nfLoop, nDiff, highDelay, lowDelay;
  disableRecv();
  switch (frequency)
  {
    case CONST_IRDA_56K:
      nfLoop = CONST_IRDA_56K_LOW_DELAY;
      nDiff = CONST_IRDA_56K_DIFF;
      highDelay = CONST_IRDA_56K_HIGH_DELAY;
      lowDelay = CONST_IRDA_56K_LOW_DELAY;
      for (i = 0; i < gsIrda.sLen; i++)
      {
        int nT1, nHigLow;
        nT1 =  gsIrda.sData[i];
        nT1 = (nT1 / nfLoop) - nDiff;
        if (nT1 < 0)
          nT1 = 0;
        if (i % 2 == 0)
          nHigLow = 1;
        else
          nHigLow = 0;
      }
      vIR_SendKHZ(nT1, nHigLow, highDelay, lowDelay);
      break;
    case CONST_IRDA_38K:
    default:
      nfLoop = CONST_IRDA_38K_LOW_DELAY;
      nDiff = CONST_IRDA_38K_DIFF;
      highDelay = CONST_IRDA_38K_HIGH_DELAY;
      lowDelay = CONST_IRDA_38K_LOW_DELAY;
	  for (i=0;i<gsIrda.sLen;i++)
	  {
        int nT1, nHigLow;
        nT1 =  gsIrda.sData[i];
        nT1 = (nT1 / nfLoop) - nDiff;
        if (nT1 < 0)
          nT1 = 0;
        if (i % 2 == 0)
          nHigLow = 1;
        else
          nHigLow = 0;
        if (nHighLow>0){
		  gen_carrier_clk();
		}
		else{
		  ir_tx_carrier_clr();	
		}
		delayMicroseconds(nT1);
	  }
      break;
  }

  enableRecv();
  return gsIrda.sLen;
}


void ir_tx_carrier_clr()
{
  PIN_FUNC_SELECT(IR_GPIO_OUT_MUX, IR_GPIO_OUT_FUNC);
  GPIO_OUTPUT_SET(IR_GPIO_OUT_NUM, 1);
  WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x0 << 8) ) ; //i2s clk stop
}



u8 BF_i2c_readReg(u8 block, u8 host_id, u8 reg_add)
{
  u32 mst_ctrl_addr = (0x60000d00 + host_id * 4);
  WRITE_PERI_REG(mst_ctrl_addr, (0 << 24) | (0 << 16) | (reg_add << 8) | block ) ; //
  while (GET_PERI_REG_BITS(mst_ctrl_addr, 25, 25) != 0) {
  };
  return (READ_PERI_REG(mst_ctrl_addr) >> 16) & 0xff;
}


u8 BF_i2c_writeReg(u8 block, u8 host_id, u8 reg_add, u8 pData)
{
  u32 mst_ctrl_addr = (0x60000d00 + host_id * 4);
  WRITE_PERI_REG(mst_ctrl_addr, (1 << 24) | (pData << 16) | (reg_add << 8) | block ) ; //
  while (GET_PERI_REG_BITS(mst_ctrl_addr, 25, 25) != 0) {
  };
  return 0;
}


void BF_i2c_writeReg_Mask(u8 block, u8 host_id, u8 reg_add, u8 Msb, u8 Lsb, u8 indata)
{
  BF_i2c_writeReg
  (
    block, host_id, reg_add,
    BF_i2c_readReg(block, host_id, reg_add) &
    (~(((1 << (Msb - Lsb + 1)) - 1) << Lsb)) |
    (indata << Lsb)
  );
}


void gen_carrier_clk()
{

  WRITE_PERI_REG(I2SCONF,  READ_PERI_REG(I2SCONF) & 0xf0000fff |
                 ( (( 62 & I2S_BCK_DIV_NUM ) << I2S_BCK_DIV_NUM_S) |
                   ((2 & I2S_CLKM_DIV_NUM) << I2S_CLKM_DIV_NUM_S) |
                   ((1 & I2S_BITS_MOD  )   <<  I2S_BITS_MOD_S )  )  );
  WRITE_PERI_REG(IR_GPIO_OUT_MUX, (READ_PERI_REG(IR_GPIO_OUT_MUX) & 0xfffffe0f) | (0x1 << 4) );
  WRITE_PERI_REG(0x60000e08, READ_PERI_REG(0x60000e08) & 0xfffffdff | (0x2 << 8) ) ; //i2s rx  start
}


void ICACHE_FLASH_ATTR ir_tx_init()
{
  /*NOTE: NEC IR code tx function need a us-accurate timer to generate the tx logic waveform*/
  /* So we call system_timer_reinit() to reset the os_timer API's clock*/
  /* Also , Remember to #define USE_US_TIMER to enable  os_timer_arm_us function*/


  //rom_i2c_writeReg_Mask(i2c_bbpll, i2c_bbpll_hostid, i2c_bbpll_en_audio_clock_out, i2c_bbpll_en_audio_clock_out_msb, i2c_bbpll_en_audio_clock_out_lsb, 1);
  BF_i2c_writeReg_Mask(i2c_bbpll, i2c_bbpll_hostid, i2c_bbpll_en_audio_clock_out, i2c_bbpll_en_audio_clock_out_msb, i2c_bbpll_en_audio_clock_out_lsb, 1);
#if IR_TX_STATUS_MACHINE_HW_TIMER
  //hw_timer_init(0, 0); //0:FRC1 INTR LEVEL 0:NOT AUTOLOAD
  //hw_timer_set_func(ir_tx_handler);
  //hw_timer_stop();
  //os_printf("Ir tx status machine Clk is hw_timer\n");
#else
  system_timer_reinit();
  //os_timer_disarm(&ir_tx_timer);
  //os_timer_setfn(&ir_tx_timer, (os_timer_func_t *)ir_tx_handler, NULL);
  //os_printf("Ir tx status machine Clk is soft_timer\n");
#endif
  //init code for mode 2;
#if GEN_IR_CLK_FROM_DMA_IIS
  i2s_carrier_duty_set(0xfff);//0xfff:50% : THE DATA WIDTH IS SET 24BIT, SO IF WE SET 0XFFF,THAT IS A HALF-DUTY.
  i2s_dma_mode_init();
#endif
}







int ICACHE_FLASH_ATTR LVIrda::param2eLevel(char *param, char *strOut)
{
  int rtn = 0;
  int i, j = 0, k = 0, m, nLen;
  int nT1;
  char achTBuff[16];
  //printf("\n test Begin:\n");
  //decodeFixedCode(fixedCode);
  //printParam();
  //decodeParam(param,gachSendBuff);
  decodeParam(param, strOut);
  /*
      k = decodeElevel(strOut,ganSendBuff);
      printf("\n test int array result:\n %d \n",k);
      for (i=0;i<k;i++)
      {
          printf(" %d",ganSendBuff[i]);
      }
      printf("\n test end:\n");
  */
  return rtn;
}


int ICACHE_FLASH_ATTR LVIrda::decodeFixedSub(char *strIn, ST_SUBFIXED *psSub)
{
  int rtn = 0;
  int i, m, k;
  char eLevelSign = 'H';
  char strTBuff[10];
  memcpy(strTBuff, strIn, 2);
  strTBuff[2] = '\0';
  m = hex2int(strTBuff);
  if (m > 128)
  {
    eLevelSign = 'L';
    m -= 128;
  }
  if (m > 0)
  {
    psSub->nLen = m;
    k = 0;
    psSub->achSign[0] = eLevelSign;
    for (i = 0; i < m; i++)
    {
      psSub->achParam[k] = eLevelSign;
      //k++;
      eLevelSign = chHighLowSwitch(eLevelSign);
      memcpy(&psSub->achParam[k], &strIn[2 + i * 4], 4);
      k += 4;
    }
    eLevelSign = chHighLowSwitch(eLevelSign);
    psSub->achSign[1] = eLevelSign;
    psSub->achParam[k + 1] = '\0';
    rtn = m * 4 + 2;
  }
  else
  {
    psSub->nLen = 0;
    psSub->achSign[0] = eLevelSign;
    psSub->achSign[1] = eLevelSign;
    psSub->achParam[0] = '\0';
    rtn = 2;

  }
  //printf("\n val:--\n %d %s %c %c \n",m,strIn, eLevelSign,eLevelSign);
  //printf("\n Sub:--\n %d %s %c %c \n",psSub->nLen, psSub->achParam, psSub->achSign[0],psSub->achSign[1]);
  return rtn;
}

int ICACHE_FLASH_ATTR LVIrda::decodeFixedCode(char *fixedCode)
{
  int nStart = 4;
  int i, m;
  char strTBuff[10];
  //printf("\nsize of %d",sizeof(ST_FIXED));
  memset(&gsFixed, 0, sizeof(ST_FIXED));
  //lead code
  //bit0
  //nStart+=decodeFixedSub(&fixedCode[nStart],&strHEAD[0],&nHeadLen,achHEADSign);
  nStart += decodeFixedSub(&fixedCode[nStart], &gsFixed.sHead);
  //bit0
  nStart += decodeFixedSub(&fixedCode[nStart], &gsFixed.asData[0]);
  //bit1
  nStart += decodeFixedSub(&fixedCode[nStart], &gsFixed.asData[1]);
  gsFixed.nDataCount = 2;
  //stop
  //printf("\nbefore STOP: %d %d %s",nStart,gsFixed.sStop.nLen,gsFixed.sStop.achParam);
  nStart += decodeFixedSub(&fixedCode[nStart], &gsFixed.sStop);
  //printf("\nafter STOP: %d %d %s",nStart,gsFixed.sStop.nLen,gsFixed.sStop.achParam);
  //printParam();
  //SYNC
  i = 0;
  while (1)
  {
    memcpy(strTBuff, &fixedCode[nStart], 2);
    strTBuff[2] = '\0';
    m = hex2int(strTBuff);
    if (m == 0)
      break;
    //printf("\nbefore SYNC:%d %d %d %s",i,nStart,gsFixed.sStop.nLen,gsFixed.sStop.achParam);
    //printParam();
    nStart += decodeFixedSub(&fixedCode[nStart], &gsFixed.asSync[i]);
    //printf("\nafter SYNC:%d %d %d %s",i,nStart,gsFixed.sStop.nLen,gsFixed.sStop.achParam);
    i++;
  }
  gsFixed.nSyncCount = i;
  //printf("\nafter sync");
  //printParam();
}

int ICACHE_FLASH_ATTR LVIrda::decodeParamSub(char *strOut, int nPos, char *pchLastSign, ST_SUBFIXED *pS)
{
  int nLast, nCurr;
  char achTBuff[10];
  char *currSign;
  //printf("\nparamSUB:%c %c %c \n",*pchLastSign,pS->achSign[0],pS->achSign[1]);
  if (pS->nLen > 0)
  {
    if (*pchLastSign == pS->achSign[0])
    {
      //merge two elevel due to the sign is equal
      memcpy(achTBuff, &strOut[nPos - 4], 4);
      achTBuff[4] = '\0';
      nLast = l4_hex2int(achTBuff);
      memcpy(achTBuff, pS->achParam, 4);
      achTBuff[4] = '\0';
      nCurr = l4_hex2int(achTBuff);
      lv_int2hex(nCurr + nLast, 4, 1, achTBuff);
      //printf ("\n %d %d %s",nLast,nCurr,achTBuff);
      nPos -= 4;
      memcpy(&strOut[nPos], achTBuff, 4);
      nPos += 4;
      nCurr = pS->nLen * 4 - 4;
      memcpy(&strOut[nPos], &pS->achParam[4], nCurr);
      nPos += nCurr;
      strOut[nPos] = '\0';
    }
    else
    {
      nCurr = pS->nLen * 4;
      //printf("\n-----%d nPos:%d, %s",nCurr,nPos,&strOut[nPos]);
      memcpy(&strOut[nPos], pS->achParam, nCurr);
      nPos += nCurr;
      strOut[nPos] = '\0';
    }
    *pchLastSign = pS->achSign[1];
  }
  return nPos;
}

int ICACHE_FLASH_ATTR LVIrda::decodeParam(char *strParam, char *strOut)
{
  int nStart = 0;
  int nPos = 0;
  int i, k, m;
  int nSync = 0;
  int nBitCount, nByteCount;
  char chLastSign = 'L';
  char strTBuff[10];

  //send count
  memcpy(strOut, "010000", 6);
  nPos = 6;
  strOut[nPos] = '\0';
  //printf("\nSEND BUF: %s",strOut);

  memcpy(strTBuff, &strParam[nStart], 2);
  strTBuff[2] = '\0';
  m = hex2int(strTBuff);
  nLoopCount = m;
  nStart += 2;

  //head
  if (gsFixed.sHead.nLen > 0)
  {
    k = gsFixed.sHead.nLen * 4;
    memcpy(&strOut[nPos], gsFixed.sHead.achParam, k);
    nPos += k;
    strOut[nPos] = '\0';
    chLastSign = gsFixed.sHead.achSign[1];
    //printf("\nSEND BUF: %s",strOut);
  }

  //data & sync
  while (1)
  {
    memcpy(strTBuff, &strParam[nStart], 4);
    nStart += 4;
    strTBuff[4] = '\0';
    m = l4_hex2int(strTBuff);
    if (m == 0)
      break;
    nBitCount = m & 0x7;
    nByteCount = m >> 3;
    //printf ("\n while: %d, %d, %d",m,nBitCount,nByteCount);
    //printf ("\n lastSign: %c\n",chLastSign);
    for (i = 0; i < nByteCount; i++)
    {
      //memcpy(strTBuff,&strParam[nStart],2);
      strTBuff[0] = strParam[nStart];
      nStart++;
      strTBuff[1] = strParam[nStart];
      nStart++;
      strTBuff[2] = '\0';
      m = hex2int(strTBuff);
      //printf("%02x ",m);
      for (k = 0; k < 8; k++)
      {
        int nT1;
        nT1 = (m >> k) & 1;
        switch (nT1)
        {
          case 0:
            nPos = decodeParamSub(strOut, nPos, &chLastSign, &gsFixed.asData[0]);
            break;
          case 1:
            nPos = decodeParamSub(strOut, nPos, &chLastSign, &gsFixed.asData[1]);
            break;
        }
        //printf("\nSend BUF: %s\n", strOut);
      }
    }
    if (nBitCount > 0)
    {
      strTBuff[0] = strParam[nStart];
      nStart++;
      strTBuff[1] = strParam[nStart];
      nStart++;
      strTBuff[2] = '\0';
      m = hex2int(strTBuff);
      for (k = 0; k < nBitCount; k++)
      {
        int nT1;
        nT1 = (m >> k) & 1;
        //printf("%d",nT1);
        switch (nT1)
        {
          case 0:
            nPos = decodeParamSub(strOut, nPos, &chLastSign, &gsFixed.asData[0]);
            break;
          case 1:
            nPos = decodeParamSub(strOut, nPos, &chLastSign, &gsFixed.asData[1]);
            break;
        }
        //printf("\nSend BUF: %s\n", strOut);
      }
    }
    // double check if the sync code is wrong
    memcpy(strTBuff, &strParam[nStart], 4);
    //nStart+=4;
    strTBuff[4] = '\0';
    m = l4_hex2int(strTBuff);
    if (m == 0)
      break;
    //sync data
    nPos = decodeParamSub(strOut, nPos, &chLastSign, &gsFixed.asSync[nSync]);
    nSync++;

  }
  //stop
  if (gsFixed.sStop.nLen > 0)
    nPos = decodeParamSub(strOut, nPos, &chLastSign, &gsFixed.sStop);
  //sendCount,or loop
  k = nPos - 6;
  for (i = 0; i < nLoopCount - 1; i++)
  {
    memcpy(&strOut[nPos], &strOut[6], k);
    nPos += k;

  }
  //count eLevel
  k = (nPos - 6) >> 2;
  lv_int2hex(k, 4, 1, strTBuff);
  memcpy(&strOut[2], strTBuff, 4);

  //printf("\nnPos: %d\n", nPos);
  //printf("\nSend BUF: %s\n", strOut);
};

int ICACHE_FLASH_ATTR LVIrda::decodeElevel(char *strParam, short an[], int maxElevel)
{
  int nStart = 2;
  int i, k, m;
  char strTBuff[10];
  memcpy(strTBuff, &strParam[nStart], 4);
  nStart += 4;
  strTBuff[4] = '\0';
  k = l4_hex2int(strTBuff);
  //printf("\n elevel len: %d\n",k);
  if (k < maxElevel)
  {
    int nPos = 0;
    for (i = 0; i < k; i++)
    {
      memcpy(strTBuff, &strParam[nStart], 4);
      nStart += 4;
      strTBuff[4] = '\0';
      m = l4_hex2int(strTBuff);
      an[nPos] = m;
      //printf(" %d",an[nPos]);
      nPos++;

    }
  }
  else
    k = -1;
  return k;
};


char ICACHE_FLASH_ATTR LVIrda::chHighLowSwitch(char chSwitch)
{
  if (chSwitch == 'H')
  {
    chSwitch = 'L';
  }
  else
  {
    chSwitch = 'H';
  }
  return chSwitch;
}

char ICACHE_FLASH_ATTR LVIrda::ch_int2hex(int n)
{
  char rtn = ' ';
  char hex[] = "0123456789ABCDEF";
  if (n < 16 && n >= 0)
    rtn = hex[n];
  return rtn;
}

int ICACHE_FLASH_ATTR LVIrda::lv_int2hex(long nValue, int nFillLen, int nBigEnding, char *pBuff)
{
  char strMode[16] = "%";
  int nModePos;
  int i, nT1;
  // special optimize for 04X format
  if (nValue < 0x10000 && nFillLen == 4)
  {
    for (i = 0; i < 4; i++)
    {
      nT1 = nValue & 0xF;
      nValue >>= 4;
      pBuff[3 - i] = ch_int2hex(nT1);
    }
    pBuff[4] = '\0';
    //printf("\n test int2hex %s\n",pBuff);
  }
  else
  {
    //printf ("\nstrMode:%d %s\n",nModePos,strMode);
    nModePos = 1;
    if (nFillLen > 0)
    {
      strMode[1] = 0x30;
      nModePos++;
      sprintf(&strMode[nModePos], "%d", nFillLen);
    }
    strcat(strMode, "X");
    //printf ("\nstrMode:%d %s\n",nModePos,strMode);
    sprintf(pBuff, strMode, nValue);
    //printf ("\nbuff:%d %s\n",nValue,pBuff);
  }
  if (nBigEnding > 0 && nFillLen >= 4)
  {
    for (i = 0; i < nFillLen; i += 4)
    {
      nT1 = pBuff[i];
      pBuff[i] = pBuff[i + 2];
      pBuff[i + 2] = nT1;
      nT1 = pBuff[i + 1];
      pBuff[i + 1] = pBuff[i + 3];
      pBuff[i + 3] = nT1;

    }
  }
  //printf ("\nswitch buff:%d %s\n",nValue,pBuff);
}


int ICACHE_FLASH_ATTR LVIrda::hex2int(char *hex)
{
  int i, t;
  int sum = 0;
  for (i = 0; hex[i]; i++)
  {
    if (hex[i] == 'x' || hex[i] == 'X')
      continue;
    if (hex[i] <= '9' && hex[i] >= '0')
      t = hex[i] - '0';
    else if (hex[i] <= 'f' && hex[i] >= 'a')
      t = hex[i] - 'a' + 10;
    else if (hex[i] <= 'F' && hex[i] >= 'A')
      t = hex[i] - 'A' + 10;
    else
      break;
    sum = (sum << 4) + t;

  }
  return sum;
}

int ICACHE_FLASH_ATTR LVIrda::l4_hex2int(char *lhex)
{
  char chT;
  chT = lhex[0];
  lhex[0] = lhex[2];
  lhex[2] = chT;
  chT = lhex[1];
  lhex[1] = lhex[3];
  lhex[3] = chT;
  lhex[4] = '\0';
  return hex2int(lhex);
}


/* load application data, return True==Success*/
bool ICACHE_FLASH_ATTR LVIrda::bLoad_Fixcode()
{
  bool bRet = false;
  int nLen;
  char *buffP;
  buffP = (char *) gsIrda.fixCode;

  File configFile = SPIFFS.open(CONST_IRDA_FILE_NAME, "r");
  if (!configFile)
  {
    //DBGPRINTLN("Failed to open IMEI file");
    return bRet;
  }
  //determine if the right config file
  nLen = configFile.readBytesUntil('\n', buffP, CONST_IRDA_MAX_FIXCODELEN);
  if (nLen <= CONST_IRDA_MAX_FIXCODELEN && nLen > CONST_IRDA_MIN_FIXCODELEN)
  {
    buffP[nLen - 1] = '\0'; //trim

    // Real world application would store these values in some variables for later use
    //DBGPRINTF("Loaded FixedCode:[%s]\n", buffP);
    bRet = true;
  }
  else {
    buffP[0] = '\0';
  }

  configFile.close();
  //DBGPRINTLN("Config ok");
  return bRet;
}


/* save application data, return True = success */
bool ICACHE_FLASH_ATTR LVIrda::bSave_Fixcode()
{
  //DBGPRINTLN("--save irda data--");
  File configFile = SPIFFS.open(CONST_IRDA_FILE_NAME, "w");
  if (!configFile)
  {
    //DBGPRINTLN("Failed to open config file for writing, then format ... ...");
    SPIFFS.format();
    configFile = SPIFFS.open(CONST_IRDA_FILE_NAME, "w");
    if (!configFile)
    {
      //DBGPRINTLN("Again! Failed to open config file for writing");
      return false;
    }
  }

  char *strPtr;
  strPtr = (char *) gsIrda.fixCode;
  configFile.println(strPtr);

  configFile.close();
  //DBGPRINTLN(" -- end");
  return true;
}



