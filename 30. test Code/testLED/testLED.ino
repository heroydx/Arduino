//#define LEDPIN 4

//LED global data

#define CONST_LED_CHANGE_COUNT 8

#define CONST_LED_STATUS_ALWAYSOFF 254
#define CONST_LED_STATUS_ALWAYSON 255

#define CONST_LED_COLOR_RED_PIN 13
#define CONST_LED_COLOR_GREEN_PIN 13
#define CONST_LED_COLOR_BLUE_PIN 13

#define CONST_LED_COLOR_RED_MASK 4
#define CONST_LED_COLOR_GREEN_MASK 2
#define CONST_LED_COLOR_BLUE_MASK 1

#define CONST_LED_LOOP_ALWAYS 255

#define CONST_LED_STRUCT_LEN 5
#define CONST_LED_STATUS_SELFDEF 0
#define CONST_LED_STATUS_SMARTCONFIG 1
#define CONST_LED_STATUS_CONNECTING 2
#define CONST_LED_STATUS_SERVER 3
#define CONST_LED_STATUS_ONE 4

uint8_t gu8LEDStatus = 0;
uint8_t gu8LEDLoopCount = 0;
uint8_t gu8LEDDispCount = 0;

typedef struct {
  uint8_t loopFlag;
  uint8_t onColor;
  uint8_t blinkCount;
  uint8_t blinkLoop[CONST_LED_CHANGE_COUNT];
} stLedDispStruct;

stLedDispStruct asLedDisplay[CONST_LED_STRUCT_LEN] = {
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {3, 6, 9, 19}}, // 300ms on, 300ms off, 300ms on, 1000ms off
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {2, 4, 6, 18}}, // 200ms on, 200ms off, 200ms on, 1200ms off
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 4, {3, 6, 9, 12 }}, //300ms on, 300ms off,300ms on, 300ms off
  {CONST_LED_LOOP_ALWAYS, CONST_LED_COLOR_RED_MASK, 2, {10, 20}}, //1000ms on, 1000ms off
  {1, CONST_LED_COLOR_RED_MASK, 2, {2, 8}}, //500ms on, 500ms off
};


// to setup or change the LED display status, the status include:CONST_LED_STATUS_ALWAYSON,CONST_LED_STATUS_SMARTCONFIG,etc.
void vChange_LED_status(uint8_t u8Status)
{
  gu8LEDStatus = u8Status;
  gu8LEDLoopCount = 0;
  gu8LEDDispCount = 0;
}

void vLED_ON(uint8_t onColor)
{
  if (onColor & CONST_LED_COLOR_RED_MASK)
    digitalWrite(CONST_LED_COLOR_RED_PIN, HIGH);
  if (onColor & CONST_LED_COLOR_GREEN_MASK)
    digitalWrite(CONST_LED_COLOR_GREEN_PIN, HIGH);
  if (onColor & CONST_LED_COLOR_BLUE_MASK)
    digitalWrite(CONST_LED_COLOR_BLUE_PIN, HIGH);
}


void vLED_OFF()
{
  digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
  digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
}

void vLED_Blink()
{
  vLED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
  delay(50);
  vLED_OFF();
}

#define CONST_LED_COLOR_BLINK_DELAY 50

void ICACHE_FLASH_ATTR LED_poweron()
{
  Serial.println("LED_poweron begin");
  for(int i=0;i<10;i++)
  {
    Serial.println("vLED_ON");
    vLED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
    delay(CONST_LED_COLOR_BLINK_DELAY);
    Serial.println("vLED_OFF");
    vLED_OFF();
    delay(CONST_LED_COLOR_BLINK_DELAY);
    delay(CONST_LED_COLOR_BLINK_DELAY);
  }
  Serial.println("LED_poweron end");
}

void vDisplay_LED_color(void)
{
  if (gu8LEDStatus == CONST_LED_STATUS_ALWAYSOFF)
  {
    //DBGPRINTLN("DISPLAY ALWAYS OFF");
    vLED_OFF();
    //DBGPRINT("_");

  }
  else if  (gu8LEDStatus == CONST_LED_STATUS_ALWAYSON)
  {
    //DBGPRINTLN("DISPLAY ALWAYS ON");
    vLED_ON(CONST_LED_COLOR_RED_MASK + CONST_LED_COLOR_GREEN_MASK + CONST_LED_COLOR_BLUE_MASK);
    //DBGPRINT("T");
  }
  else
  {
    if (gu8LEDLoopCount < asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
      //if (gu8LEDLoopCount == asLedDisplay[gu8LEDStatus].blinkLoop[gu8LEDDispCount])
    {
      if ((gu8LEDDispCount % 2) == 0)
      {
        vLED_ON(asLedDisplay[gu8LEDStatus].onColor);
        //digitalWrite(CONST_LED_COLOR_RED_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_RED_MASK );
        //digitalWrite(CONST_LED_COLOR_GREEN_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_GREEN_MASK);
        //digitalWrite(CONST_LED_COLOR_BLUE_PIN, asLedDisplay[gu8LEDStatus].onColor & CONST_LED_COLOR_BLUE_MASK);
        //DBGPRINT("T");
      }
      else
      {
        //DBGPRINTLN("DISPLAY OFF");
        digitalWrite(CONST_LED_COLOR_RED_PIN, LOW);
        digitalWrite(CONST_LED_COLOR_GREEN_PIN, LOW);
        digitalWrite(CONST_LED_COLOR_BLUE_PIN, LOW);
        //DBGPRINT("_");
      }

    }
    else {
      gu8LEDDispCount++;
      if (gu8LEDDispCount >= asLedDisplay[gu8LEDStatus].blinkCount)
      {
        gu8LEDDispCount = 0;
        if (asLedDisplay[gu8LEDStatus].loopFlag != CONST_LED_LOOP_ALWAYS)
        {
          gu8LEDStatus = CONST_LED_STATUS_ALWAYSOFF;
          //DBGPRINTLN("!!!!!");
        }
      }
    }

    gu8LEDLoopCount++;
    if (gu8LEDLoopCount >= asLedDisplay[gu8LEDStatus].blinkLoop[asLedDisplay[gu8LEDStatus].blinkCount - 1])
    {
      gu8LEDLoopCount = 0;
      gu8LEDDispCount = 0;
      if (asLedDisplay[gu8LEDStatus].loopFlag != CONST_LED_LOOP_ALWAYS)
      {
        gu8LEDStatus = CONST_LED_STATUS_ALWAYSOFF;
        //DBGPRINTLN("!!!!!");
      }

    }
  }
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
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("----Setup----");
  //LED PIN MODE
  pinMode(CONST_LED_COLOR_RED_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_GREEN_PIN, OUTPUT);
  pinMode(CONST_LED_COLOR_BLUE_PIN, OUTPUT);
  LED_poweron();

}

int count = 0;
uint8_t gu8LED_status[] = {
  CONST_LED_STATUS_CONNECTING,
  CONST_LED_STATUS_SMARTCONFIG
};

void loop() {
  // put your main code here, to run repeatedly:
  // a common 100 us loop function, for led, sync time, ...
  if (ulGet_interval(gl100msLoopTick) >= 100)
  {
    // LED Control
    vDisplay_LED_color();
    vReset_interval(gl100msLoopTick);
    count++;
    if (count % 100 == 0)
    {
      int nStat;
      nStat = random(0, 2);
      Serial.println();
      Serial.print("stat:");
      Serial.println(nStat);
      vChange_LED_status(gu8LED_status[nStat]);
    }
    else if (count % 10 == 0)
    {
      Serial.print(".");
    }
  }

}
