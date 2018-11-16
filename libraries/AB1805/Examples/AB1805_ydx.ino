/* 如何驱动晶振开启 */
//#include <I2C.h>
#include <Wire.h>
#include "AB1805.h"

AB1805 timer;

#define CONST_I2C_SCL_PIN 5
#define CONST_I2C_SDA_PIN 4

void setup() {

  Serial.begin(74880);
  delay(1000);
  Serial.printf("\n\n\n");
  Serial.println("\nI2C A1805 begin");

  //初始化A1805
  timer.begin(CONST_I2C_SDA_PIN, CONST_I2C_SCL_PIN);
  //设置当前时间
  timer.set_datetime(0x18, 0x11, 0x07, 0x02, 0x16, 0x56, 0x00); //year, month, date, week_day, hour, minute, second (2018, Nov 07, wen, 16:56:00)
  //设置告警时间,每分钟一次告警
  timer.set_datetime_alarm(0x00); //year, month, date, week_day, hour, minute, second (2018, Nov 07, wen, 16:56:00)
  Serial.printf("\nAlarm:%02X-%02X %02X:%02X:%02X ", timer.get_date_alarm(), timer.get_month_alarm(), timer.get_hour_alarm(), timer.get_minute_alarm(), timer.get_second_alarm());
  read_interrupt_reg();
  timer.set_AIE_interrupt(1);
  read_interrupt_reg();


  uint8_t val;
  read_all_reg();
  read_all_reg();

  // print information
  Serial.printf("\n\n %s  %02X", timer.get_date_time(), timer.get_weekday());
  Serial.printf("\n Alarm:%02X-%02X %02X:%02X:%02X  %02X", timer.get_date_alarm(), timer.get_month_alarm(), timer.get_hour_alarm(), timer.get_minute_alarm(), timer.get_second_alarm(), timer.get_weekday_alarm());
  read_status_reg();
  read_interrupt_reg();

  Serial.println("\n==== enter sleep mode begin ===");
  int delayValue = 10000;
  Serial.printf("\n before enter sleep mode, delay [%d]", delayValue);
  delay(delayValue);

  // enter sleep mode
  // control2
  read_control2_reg();
  read_outcontrol_reg();
  read_osc_control_reg();

  //set control2 sleep mode
  timer.set_OUT2S_control2(CONTROL2_SLEEP_MODE);
  Serial.println("\n After set controls sleep out2s reg");
  read_control2_reg();

  //sleep mode
  read_sleep_reg();
  //set SLTO the time of SWAIT.
  timer.set_SLTO_sleep(7);

  timer.set_outcontrol(0, CCTRL_SLEEP_MODE_MASK);
  timer.set_osc_control(0, OSC_CONTROL_PWGT_MASK);

  //set sleep mode
  timer.set_SLP_sleep(1);

  Serial.println("\n After set sleep reg");
  read_sleep_reg();

  Serial.println("\n==== enter sleep mode end ===");
  /*
    uint8_t ret;
    ret = timer.enter_sleep_mode(7, 2);
    Serial.printf("\n enter_sleep_mode ret:[%d]", ret);
  */
  Serial.println("\nI2C A1805 setup end.");
  
}

#define REG_ENG_ADDR 0x3F

uint8_t all_reg[REG_ENG_ADDR + 4];
uint8_t pre_all_reg[REG_ENG_ADDR + 4];

void read_all_reg()
{
  // read all registers
  uint8_t i;
  uint8_t val;
  for (i = 0; i < 0x40; i++)
  {
    if (((i % 8) == 0))
      Serial.printf("\n");
    val = timer.read_rtc_register(i);
    if (pre_all_reg[i] != all_reg[i]) {
      Serial.printf("\ndiff; R:%02X %02X %02X \n ", i, pre_all_reg[i], all_reg[i]);
    }
    pre_all_reg[i] = all_reg[i];
    all_reg[i] = val;
    Serial.printf("R:%02X %02X  ", i, val);
  }
  Serial.printf("\n");
}

void read_status_reg()
{
  uint8_t status_flag;
  uint8_t status_ALM;
  status_flag = timer.get_status();
  Serial.printf("\n=== Status:[%02X]", status_flag);
  status_ALM = (status_flag & 0x4);
  Serial.printf(" ALM:[%02X]", status_ALM);
  if (status_ALM)
    timer.clean_ALM_status();
}

void read_sleep_reg()
{
  uint8_t sleep_mode;
  uint8_t SLP, SLRES, EX2P, EX1P, SLST, SLTO;
  sleep_mode = timer.get_sleep();
  SLP = sleep_mode & 0x80;
  SLRES = sleep_mode & 0x40;
  EX2P = sleep_mode & 0x20;
  EX1P = sleep_mode & 0x10;
  SLST = sleep_mode & 0x08;
  SLTO = sleep_mode & 0x07;

  Serial.printf("\n\n sleep register:[%02X]", sleep_mode);
  if (SLST) {
    Serial.printf("\n enters sleep mode [%02X]", SLST);
  }
  Serial.printf("\n Sleep:SLP:[%02X],SLRES:[%02X],EX2P:[%02X],EX1P:[%02X],SLST:[%02X],SLTO:[%02X],", SLP, SLRES, EX2P, EX1P, SLST, SLTO );
}

void read_control2_reg()
{
  uint8_t control2;
  uint8_t RS1E, OUT2S, OUT1S;
  control2 = timer.get_control2();
  RS1E = control2 & 0x20;
  OUT2S = control2 & 0x1C;
  OUT1S = control2 & 0x3;

  Serial.printf("\n\n control2 register:[%02X]", control2);
  Serial.printf("\n control2:RS1E:[%02X],OUT2S:[%02X],OUT1S:[%02X]", RS1E, OUT2S, OUT1S );
}

void read_outcontrol_reg()
{
  uint8_t outcontrol;
  uint8_t WDBM, EXBM, WDDS, EXDS, RSEN, O4EN, O3EN, O1EN;
  outcontrol = timer.get_outcontrol();
  WDBM = outcontrol & OCTRL_WDBM_MASK;
  EXBM = outcontrol & OCTRL_EXBM_MASK;
  WDDS = outcontrol & OCTRL_WDDS_MASK;
  EXDS = outcontrol & OCTRL_EXDS_MASK;
  RSEN = outcontrol & OCTRL_RSEN_MASK;
  O4EN = outcontrol & OCTRL_O4EN_MASK;
  O3EN = outcontrol & OCTRL_O3EN_MASK;
  O1EN = outcontrol & OCTRL_O1EN_MASK;

  Serial.printf("\n\n outcontrol register:[%02X]", outcontrol);
  Serial.printf("\n control2:WDBM:[%02X],EXBM:[%02X],WDDS:[%02X],EXDS:[%02X],RSEN:[%02X],O4EN:[%02X],O3EN:[%02X],O1EN:[%02X]", WDBM, EXBM, WDDS, EXDS, RSEN, O4EN, O3EN, O1EN );
}

void read_osc_control_reg()
{
  uint8_t osc_control;
  uint8_t OSEL, ACAL, AOS, FOS, PWGT, OFIE, ACIE;
  osc_control = timer.get_osc_control();
  OSEL = osc_control & OSC_CONTROL_OSEL_MASK;
  ACAL = osc_control & OSC_CONTROL_ACAL_MASK;
  AOS = osc_control & OSC_CONTROL_AOS_MASK;
  FOS = osc_control & OSC_CONTROL_FOS_MASK;
  PWGT = osc_control & OSC_CONTROL_PWGT_MASK;
  OFIE = osc_control & OSC_CONTROL_OFIE_MASK;
  ACIE = osc_control & OSC_CONTROL_ACIE_MASK;

  Serial.printf("\n\n osc_control register:[%02X]", osc_control);
  Serial.printf("\n osc_control:OSEL:[%02X],ACAL:[%02X],AOS:[%02X],FOS:[%02X],PWGT:[%02X],OFIE:[%02X],ACIE:[%02X]", OSEL, ACAL, AOS, FOS, PWGT, OFIE, ACIE);
}


void read_interrupt_reg()
{
  uint8_t interrupt_flag;
  uint8_t CEB, IM, BLIE, TIE, AIE, EX2E, EX1E;
  interrupt_flag = timer.get_interrupt();
  Serial.printf("\n=== interrupt_flag:[%02X]", interrupt_flag);
  CEB = interrupt_flag & INTERRUPT_CEB_MASK;
  IM = interrupt_flag & INTERRUPT_IM_MASK;
  BLIE = interrupt_flag & INTERRUPT_BLIE_MASK;
  TIE = interrupt_flag & INTERRUPT_TIE_MASK;
  AIE = interrupt_flag & INTERRUPT_AIE_MASK;
  EX2E = interrupt_flag & INTERRUPT_EX2E_MASK;
  EX1E = interrupt_flag & INTERRUPT_EX1E_MASK;
  Serial.printf(" CEB:[%02X],IM:[%02X],BLIE:[%02X],TIE:[%02X],AIE:[%02X],EX2E:[%02X],EX1E:[%02X]", CEB, IM, BLIE, TIE, AIE, EX2E, EX1E);
}


void set_alarm_reg()
{

}

void old_loop()
{

  delay(1000);

  //Serial.printf("get_seconds_alarm:%d \n",timer.get_seconds_alarm());

  Serial.println(timer.get_date_time());
  Serial.println(timer.get_rtc_data(OSC_CONTROL_REGISTER, OSEL_MASK));
  uint8_t val;
  val = timer.read_rtc_register(OSC_CONTROL_REGISTER);

  Serial.println(val, HEX);
  val = 0x80 | 0x18 | val;
  Serial.println(val, HEX);
  timer.write_rtc_register(OSC_CONTROL_REGISTER, val);
  val = timer.read_rtc_register(OSC_CONTROL_REGISTER);
  Serial.println(val, HEX);

}

void loop()
{
  delay(1000);

  //Serial.printf("get_seconds_alarm:%d \n",timer.get_seconds_alarm());

  Serial.printf("\n\n %s  %02X", timer.get_date_time(), timer.get_weekday());
  Serial.printf("\nAlarm:%02X-%02X %02X:%02X:%02X  %02X", timer.get_date_alarm(), timer.get_month_alarm(), timer.get_hour_alarm(), timer.get_minute_alarm(), timer.get_second_alarm(), timer.get_weekday_alarm());
  read_status_reg();
  read_interrupt_reg();
  read_sleep_reg();

}

