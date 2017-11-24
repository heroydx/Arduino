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

#define CONST_LED_COLOR_RED_PIN 13
#define CONST_IRDA_SEND_PORT 14
#define CONST_IRDA_38K_LOW_DELAY 19
//#define CONST_IRDA_38K_EMPTY_DELAY CONST_IRDA_38K_LOW_DELAY-CONST_IRDA_38K_HIGH_DELAY
#define CONST_IRDA_38K_EMPTY_DELAY 10
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

void cir_test()
{
  // put your main code here, to run repeatedly:
  digitalWrite(CONST_IRDA_SEND_PORT, 1);
  digitalWrite(CONST_LED_COLOR_RED_PIN, 1);
  Serial.println("ON");
  delay(2000);
  digitalWrite(CONST_IRDA_SEND_PORT, 0);
  digitalWrite(CONST_LED_COLOR_RED_PIN, 0);
  Serial.println("OFF");
  delay(2000);
 

}
void setup() {
  // put your setup code here, to run once:
  pinMode(CONST_IRDA_SEND_PORT, OUTPUT);
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  Serial.begin(115000);
  system_timer_reinit();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(CONST_IRDA_SEND_PORT, 1);
  digitalWrite(CONST_LED_COLOR_RED_PIN, 1);
  //delay(1);
  delayMicroseconds(CONST_IRDA_38K_HIGH_DELAY);
  digitalWrite(CONST_IRDA_SEND_PORT, 0);
  digitalWrite(CONST_LED_COLOR_RED_PIN, 0);
  //delay(1);
  delayMicroseconds(CONST_IRDA_38K_EMPTY_DELAY);
}
