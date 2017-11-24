#include "Timenew.h"

typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int wday;
  int timezone2;//consider indea situation 7.5
  int splithour;
} LVTime;


LVTime gTime;


void vGet_time() {
  time_t t;

  t = now();
  Serial.print("time_t:");
  Serial.println(t);
  t += 1800 * gTime.timezone2;
  Serial.print("time_t:");
  Serial.println(t);

  gTime.year = year(t);
  gTime.month = month(t);
  gTime.day = day(t);
  gTime.hour = hour(t);
  gTime.minute = minute(t);
  gTime.second = second(t);
  gTime.wday = weekday(t);
}

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
  int hh, mm, ss, dd, mon, yy;
  char strMonth[6];
  char buff[10];

  memcpy(buff, &achYMD[10], 3);
  buff[3] = 0;
  dd = ((String)(buff)).toInt();

  memcpy(strMonth, &achYMD[14], 3);
  strMonth[3] = 0;
  mon = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[18], 4);
  buff[4] = 0;
  yy = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[23], 2);
  buff[2] = 0;
  hh = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[26], 2);
  buff[2] = 0;
  mm = ((String)(buff)).toInt();

  memcpy(buff, &achYMD[29], 2);
  buff[2] = 0;
  ss = ((String)(buff)).toInt();

  sp->year = yy;
  sp->month = mon;
  sp->day = dd;
  sp->hour = hh;
  sp->minute = mm;
  sp->second = ss;
  sp->timezone2 = 8 * 2; //default;
  setTime(hh, mm, ss, dd, mon, yy);

}

void vSyncTime_998(String sYMDHMS, LVTime *sp)
{
  time_t t;
  int hh, mm, ss, dd, mon, yy, timezone;
  yy = sYMDHMS.substring(0, 4).toInt();
  mon = sYMDHMS.substring(4, 6).toInt();
  dd = sYMDHMS.substring(6, 8).toInt();
  hh = sYMDHMS.substring(8, 10).toInt();
  mm = sYMDHMS.substring(10, 12).toInt();
  ss = sYMDHMS.substring(12, 14).toInt();
  timezone = sYMDHMS.substring(14, 16).toInt();
  Serial.println("---------------------");
  Serial.println(sYMDHMS.substring(0, 4));
  Serial.println("---------------------");


  setTime(hh, mm, ss, dd, mon, yy);

  t = now() -  1800 * gTime.timezone2;

  sp->timezone2 = timezone; 
  
  yy = year(t);
  mon = month(t);
  dd = day(t);
  hh = hour(t);
  mm = minute(t);
  ss = second(t);
  setTime(hh, mm, ss, dd, mon, yy);
  
  t = now();
}

char serverHeader[12][100] = {
  "Date: Wed, 01 Jan 2016 05:38:59 GMT",
  "Date: Wed, 02 Feb 2016 05:38:59 GMT",
  "Date: Wed, 03 Mar 2016 05:38:59 GMT",
  "Date: Wed, 04 Apr 2016 05:38:59 GMT",
  "Date: Wed, 05 May 2016 05:38:59 GMT",
  "Date: Wed, 06 Jun 2016 05:38:59 GMT",
  "Date: Wed, 07 Jul 2016 05:38:59 GMT",
  "Date: Wed, 08 Aug 2016 05:38:59 GMT",
  "Date: Wed, 09 Sep 2016 05:38:59 GMT",
  "Date: Wed, 10 Oct 2016 05:38:59 GMT",
  "Date: Wed, 11 Nov 2016 05:38:59 GMT",
  "Date: Wed, 12 Dec 2016 05:38:59 GMT",
};

char testYMDHMS[] = "2016010203040516";

unsigned long glTimeTick;

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

int gLoop, gMonth = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  Serial.println("waiting for sync");
  vSyncTimeFromServer(serverHeader[0], &gTime);
  Serial.println("===Test LVTime===");
  Serial.println("vSyncTimeFromServer");
  Serial.print(gTime.year);
  Serial.print(gTime.month);
  Serial.print(gTime.day);
  Serial.print(gTime.hour);
  Serial.print(gTime.minute);
  Serial.print(gTime.second);
  Serial.println();
  Serial.println("setup end");
}

void loop() {
  // put your main code here, to run repeatedly:
  int i;

  // sync time
  if (ulGet_interval(glTimeTick) >= 1000)
  {
    char dispBuff[100];
    vGet_time();
    sprintf(dispBuff, "%04d-%02d-%02d %02d %02d:%02d:%02d", gTime.year, gTime.month, gTime.day, gTime.wday, gTime.hour, gTime.minute, gTime.second);
    Serial.println(dispBuff);
    vReset_interval(glTimeTick);
    digitalClockDisplay();
    //delay(1000);
    gLoop++;
    {
      String testLong = "1607071211";
      unsigned lTemp;
      lTemp = atol(testLong.c_str());
      Serial.println(lTemp);
    }
  }
  if (gLoop == 10) {
    gLoop = 0;
    //vSyncTimeFromServer(serverHeader[gMonth], &gTime);
    vSyncTime_998(testYMDHMS, &gTime);
    gMonth++;
    Serial.println();

  }
  if (gMonth >= 12) {
    gMonth = 0;
  }
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

