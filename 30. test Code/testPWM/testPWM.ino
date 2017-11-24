#include <arduino.h>
#define CONST_LED_COLOR_RED_PIN 12
#define CONST_LED_COLOR_BLUE_PIN 14
#define CONST_LED_COLOR_GREEN_PIN 5

void vLED_ON(short val)
{
  analogWrite(CONST_LED_COLOR_RED_PIN, val);
  analogWrite(CONST_LED_COLOR_BLUE_PIN, val);
  analogWrite(CONST_LED_COLOR_GREEN_PIN, val);
}


void vLED_OFF()
{
  analogWrite(CONST_LED_COLOR_RED_PIN, 0);
  analogWrite(CONST_LED_COLOR_BLUE_PIN, 0);
  analogWrite(CONST_LED_COLOR_GREEN_PIN, 0);
}

void vLED_Blink(short val)
{
  vLED_ON(val);
  delay(50);
  vLED_OFF();
}


void vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}

unsigned long ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}

unsigned long gl100msLoopTick = 0;/* a common 100 ms loop */


void setup() {
  // put your setup code here, to run once:
  Serial.begin(74880);
  //LED PIN MODE
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  Serial.println();
  Serial.println("----Setup----");
  vLED_ON(1023);
}

short count;
short PWMVal;

#define PWM_LIST_LEN 6

short ganPWMList[6] = {1023, 512, 256, 128, 64, 0};

void loop() {
  // put your main code here, to run repeatedly:
  // a common 100 us loop function, for led, sync time, ...
  if (ulGet_interval(gl100msLoopTick) >= 1000)
  {
    if (count % 2 == 0)
    {
      vLED_OFF();
    }
    PWMVal = analogRead(CONST_LED_COLOR_RED_PIN);
    Serial.printf("\n PWM Val:[%d],count:[%d],ganPWMList:[%d]", PWMVal, count,ganPWMList[count]);
    vLED_ON(ganPWMList[count]);
    vReset_interval(gl100msLoopTick);
    count++;
    if (count >= PWM_LIST_LEN) {
      count = 0;
    }
  }
}

