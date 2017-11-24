short nIntPart, nDecPart;
char dispBuff[100];

#define CONST_IRDA_DATA_LEN 600
int nLen = 292;
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

volatile short nDataLen;
volatile short nRecvBusy;
#define CONST_IRDA_RECV_PORT 5
volatile unsigned short recv_buff[CONST_IRDA_DATA_LEN + 2];
volatile long ulCurrIrdaTick;
volatile unsigned long ulPreIrdaTick;

short irda_len[CONST_IRDA_DATA_LEN];


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


void vIrda_recv_interrupt()
{
  //nRecvBusy = 1;
  ulCurrIrdaTick = micros();
  if (nDataLen == 0)
  {
    recv_buff[0] = (unsigned short) ulCurrIrdaTick;
  }
  else
  {
    recv_buff[nDataLen] = (unsigned short) ( ulCurrIrdaTick - ulPreIrdaTick);
  }
  ulPreIrdaTick = ulCurrIrdaTick;
  if (recv_buff[nDataLen] < 0)
    recv_buff[nDataLen] += 0xffffffff;
  nDataLen++;
  if (nDataLen >= CONST_IRDA_DATA_LEN)
    nDataLen = 0;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(CONST_IRDA_RECV_PORT, INPUT);
  //init a  ms timer interrupt
  //os_timer_setfn(&ghTimer100ms, vTimer_interrupt100ms, NULL);
  //os_timer_arm(&ghTimer100ms, 100, true);
  vReset_microseconds(ul25usLoopTick);
  vReset_microseconds(ul1msLoopTick);
  attachInterrupt(CONST_IRDA_RECV_PORT, vIrda_recv_interrupt, CHANGE);
}


void loop() {
  // put your main code here, to run repeatedly:
  int i;

  // a common 100 us loop function, for led, sync time, ...
  if (ulGet_microseconds(ul25usLoopTick) >= 25)
  {
    ul25Count++;
    vReset_microseconds(ul25usLoopTick);

  }
  if (ulGet_microseconds(ul1msLoopTick) >= 1000000)
  {
    //Serial.printf("\n micro count %d %d", ul25Count, ul25Count * 25);
    //Serial.printf("\n\r Recv Data  %d", nDataLen);
    ul25Count = 0;
    vReset_microseconds(ul1msLoopTick);
    if (nRecvBusy > 0)
    {
      //Serial.printf("\n\rinterrypt %d %d", nRecvBusy, nDataLen);
      //nRecvBusy = 0;
    }
    if (nDataLen > 0)
    {
      for (i = 0; i < nDataLen; i++)
      {
        //if (i % 10 == 0)
        //Serial.println();
        Serial.printf("\n%10d ", recv_buff[i]);
        Serial.printf("%4d ", i);

      }
      //Serial.println();
      //delay(3000);
    }
    nDataLen = 0;
    Serial.println();
    Serial.println();
  }
}
