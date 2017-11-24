#include <time.h>
// 8266 time

time_t gNowTime, gPreTime;

typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} LVTime;


LVTime gTime;

int str2month(char *strMonth)
{
  int month = 0;
  if (strcmp(strMonth, "Jan") == 0)
  {
    month = 1;
  }
  else if (strcmp(strMonth, "Feb") == 0)
  {
    month = 2;
  }
  else if (strcmp(strMonth, "Mar") == 0)
  {
    month = 3;
  }
  else if (strcmp(strMonth, "Apr") == 0)
  {
    month = 4;
  }
  else if (strcmp(strMonth, "May") == 0)
  {
    month = 5;
  }
  else if (strcmp(strMonth, "Jun") == 0)
  {
    month = 6;
  }
  else if (strcmp(strMonth, "Jul") == 0)
  {
    month = 7;
  }
  else if (strcmp(strMonth, "Aug") == 0)
  {
    month = 8;
  }
  else if (strcmp(strMonth, "Sep") == 0)
  {
    month = 9;
  }
  else if (strcmp(strMonth, "Oct") == 0)
  {
    month = 10;
  }
  else if (strcmp(strMonth, "Nov") == 0)
  {
    month = 11;
  }
  else if (strcmp(strMonth, "Dec") == 0)
  {
    month = 12;
  }
  return month;
}

void vSyncTimeFromServer(char achYMD[], LVTime *sp)
{
  int hh, mm, ss, dd, month, year;
  char strMonth[6];
  char buff[10];

  memcpy(buff, &achYMD[10], 3);
  buff[3] = 0;
  dd=((String)(buff)).toInt();

  memcpy(strMonth, &achYMD[14], 3);
  strMonth[3] = 0;
  month=((String)(buff)).toInt();

  memcpy(buff, &achYMD[18], 4);
  buff[4] = 0;
  year=((String)(buff)).toInt();
  
  memcpy(buff, &achYMD[23], 2);
  buff[2] = 0;
  hh=((String)(buff)).toInt();
  
  memcpy(buff, &achYMD[26], 2);
  buff[2] = 0;
  mm=((String)(buff)).toInt();

  memcpy(buff, &achYMD[29], 2);
  buff[2] = 0;
  ss=((String)(buff)).toInt();

  sp->year = year;
  sp->month = month;
  sp->day = dd;
  sp->hour = hh;
  sp->minute = mm;
  sp->second = ss;

}

char serverHeader[] = "Date: Wed, 02 Nov 2016 05:38:59 GMT";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial.println("waiting for sync");
  ensureBootTimeIsSet()
  vSyncTimeFromServer(serverHeader,&gTime);
  Serial.println("===Test LVTime===");
  Serial.println("vSyncTimeFromServer");
  Serial.print(gTime.year);
  Serial.print(gTime.month);
  Serial.print(gTime.day);
  Serial.print(gTime.hour);
  Serial.print(gTime.minute);
  Serial.print(gTime.second);
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
}


void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
