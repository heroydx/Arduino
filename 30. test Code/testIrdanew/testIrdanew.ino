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

#define IR_GPIO_OUT_MUX   PERIPHS_IO_MUX_MTMS_U //MTCK PIN ACT AS I2S CLK OUT

#define I2C_BASE                            0x60000D00
#define I2S_BCK_DIV_NUM       0x0000003F
#define I2S_BCK_DIV_NUM_S       22
#define I2S_CLKM_DIV_NUM       0x0000003F
#define I2S_CLKM_DIV_NUM_S    16
#define I2S_BITS_MOD          0x0000000F
#define I2S_BITS_MOD_S          12
#define I2SCONF               (DR_REG_I2S_BASE + 0x0008)
#define DR_REG_I2S_BASE       (0x60000e00)

#define U32 uint32
#define i2c_bbpll                           0x67
#define i2c_bbpll_en_audio_clock_out           4
#define i2c_bbpll_en_audio_clock_out_msb       7
#define i2c_bbpll_en_audio_clock_out_lsb       7
#define i2c_bbpll_hostid                      4

void ir_tx_carrier_clr()
{
  //PIN_FUNC_SELECT(IR_GPIO_OUT_MUX, IR_GPIO_OUT_FUNC);
  //GPIO_OUTPUT_SET(IR_GPIO_OUT_NUM, 1);
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
  hw_timer_init(0, 0); //0:FRC1 INTR LEVEL 0:NOT AUTOLOAD
  hw_timer_set_func(ir_tx_handler);
  hw_timer_stop();
  os_printf("Ir tx status machine Clk is hw_timer\n");
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

short nIntPart, nDecPart;
char dispBuff[100];

#define CONST_IRDA_DATA_LEN 600
int nLen = 292;

/*
  short send_buff[CONST_IRDA_DATA_LEN] =
  {
  9950, 1191, 2974, 994, 2972, 993, 2970, 975, 2978, 981, 1000, 2949, 3001, 990, 2971, 993, 967, 2978, 995, 2984,
  3002, 989, 970, 2976, 1023, 2979, 974, 3004, 2972, 989, 972, 3006, 974, 2975, 3001, 996, 2969, 985, 975, 2974,
  3001, 989, 973, 3000, 995, 2981, 2973, 988, 2971, 987, 1000, 2980, 2973, 987, 1003, 2943, 3005, 988, 972, 2979,
  3000, 977, 3001, 965, 993, 2957, 1000, 2981, 3004, 989, 997, 2942, 3001, 988, 1000, 2978, 974, 3006, 2977, 985,
  1028, 2928, 1019, 2961, 2977, 1006, 999, 2956, 1007, 2994, 3001, 964, 2996, 960, 3002, 957, 1027, 2932, 3029, 964,
  3001, 957, 1003, 2963, 3004, 972, 3001, 954, 1006, 2942, 1027, 2964, 1007, 2970, 996, 2984, 1024, 2959, 1021, 2946,
  1059, 2920, 1024, 2960, 1079, 2926, 1024, 2956, 3033, 928, 3032, 932, 3026, 940, 3005, 954, 3032, 927, 3029, 934,
  3026, 931, 3030, 940, 3003, 4944, 10007, 1143, 3030, 940, 3003, 958, 3033, 925, 3033, 926, 1016, 2961, 3041, 931,
  3005, 958, 1048, 2907, 1062, 2917, 3054, 927, 1025, 2951, 1063, 2907, 1061, 2909, 3061, 933, 1027, 2928, 1062, 2918,
  3056, 930, 3027, 940, 1049, 2896, 3064, 943, 1048, 2895, 1060, 2921, 3052, 937, 3033, 930, 990, 2965, 3054, 937,
  1034, 2915, 3053, 938, 992, 2964, 3058, 925, 3039, 934, 988, 2988, 1037, 2910, 3066, 926, 1053, 2906, 3059, 933,
  1024, 2951, 1028, 2935, 3051, 928, 1060, 2918, 1050, 2898, 3064, 938, 1052, 2907, 1061, 2919, 3054, 936, 3032, 932,
  3033, 933, 1031, 2915, 3055, 940, 3034, 933, 1037, 2911, 3064, 927, 3037, 922, 1060, 2899, 1060, 2954, 1035, 2931,
  1059, 2918, 1056, 2934, 993, 2965, 1089, 2893, 1089, 2921, 996, 2962, 1066, 2918, 3047, 941, 3033, 930, 3033, 932,
  3037, 923, 3032, 932, 3031, 934, 3034, 930, 3033, 932, 3037, 65535
  };
*/

short send_buff[CONST_IRDA_DATA_LEN] =
{
  //haier 7 (on)
  //3000, 3000, 3000, 4500, 550, 1660, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 1660, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 1660, 550, 1660, 550, 1660, 550, 20000
  //geli9 on 120800
  9000, 4500, 550, 550, 550, 550, 550, 1660, 550, 1660, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 20000, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 1660, 550, 1660, 550, 40000
  //geli 9 off
  //9000, 4500, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 20000, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 550, 1660, 550, 1660, 550, 550, 550, 1660, 550, 40000
};

short irda_len[CONST_IRDA_DATA_LEN];

#define CONST_LED_COLOR_RED_PIN 13
#define CONST_IRDA_SEND_PORT 14
#define CONST_IRDA_38K_LOW_DELAY 26
#define CONST_IRDA_38K_EMPTY_DELAY CONST_IRDA_38K_LOW_DELAY-CONST_IRDA_38K_HIGH_DELAY
#define CONST_IRDA_38K_HIGH_DELAY 8
#define CONST_IRDA_38K_DIFF 5

void vIR_Send38KHZ(int x, int y) //产生38KHZ红外脉冲
{
  int i;
  for (i = 0; i < x; i++) //15=386US
  {
    if (y == 1)
    {
      digitalWrite(CONST_IRDA_SEND_PORT, 1);
      delayMicroseconds(CONST_IRDA_38K_HIGH_DELAY);
      digitalWrite(CONST_IRDA_SEND_PORT, 0);
      delayMicroseconds(CONST_IRDA_38K_EMPTY_DELAY);
    }
    else
    {
      digitalWrite(CONST_IRDA_SEND_PORT, 0);
      delayMicroseconds(CONST_IRDA_38K_LOW_DELAY);
    }
  }
}


void vReset_microseconds(unsigned long &resetTick)
{
  resetTick = micros();
}

unsigned long ulGet_microseconds(unsigned long checkTick)
{
  unsigned long cur = micros();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}

int count = 0;
unsigned long ul25usLoopTick;
unsigned long ul1msLoopTick;
int ul25Count;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(CONST_IRDA_SEND_PORT, OUTPUT);
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  //init a  ms timer interrupt
  //os_timer_setfn(&ghTimer100ms, vTimer_interrupt100ms, NULL);
  //os_timer_arm(&ghTimer100ms, 100, true);
  vReset_microseconds(ul25usLoopTick);
  vReset_microseconds(ul1msLoopTick);
  ir_tx_init();
  cli();
  ir_tx_carrier_clr();

}


void loop() {
  // put your main code here, to run repeatedly:
  int i;

  for (i = 0; i < CONST_IRDA_DATA_LEN; i++)
  {
    int nT1, nHigLow;
    digitalWrite(CONST_LED_COLOR_RED_PIN, 1);
    nT1 =  send_buff[i];
    if (nT1 <= 0)
      break;
    /*
        nT1 = (nT1 / CONST_IRDA_38K_LOW_DELAY) - CONST_IRDA_38K_DIFF;
        if (nT1 < 0)
          nT1 = 0;
    */
    if (i % 2 == 0)
      nHigLow = 1;
    else
      nHigLow = 0;

    irda_len[i] = nT1;
    /*    vIR_Send38KHZ(nT1, nHigLow);
    */
    if (nHigLow > 0) {
      gen_carrier_clk();
    }
    else {
      ir_tx_carrier_clr();
      //gen_carrier_clk();
    }
    delayMicroseconds(nT1);
  }
  digitalWrite(CONST_LED_COLOR_RED_PIN, 0);
  Serial.println(count);
  Serial.println(nLen);
  count++;
  nLen = i;
  for (i = 0; i < nLen; i++)
  {
    if (i % 10 == 0)
      Serial.println();
    Serial.printf("%5d ", irda_len[i]);
    Serial.printf("%d ", i);

  }
  Serial.println();
  delay(3000);

}
