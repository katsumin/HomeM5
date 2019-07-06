#ifndef _LOG_H_
#define _LOG_H_
#include <M5Stack.h>
#include "Rtc.h"

class Log
{
private:
  // RTC
  Rtc *_rtc;
  char _buf[32];

public:
  inline void setRtc(Rtc *rtc) { _rtc = rtc; };
  void out(const char *msg)
  {
    bool flgSD = SD.begin(TFCARD_CS_PIN, SPI, 40000000);
    if (flgSD)
    {
      tm *t = _rtc->getTm();
      snprintf(_buf, sizeof(_buf), "/log_%04d%02d%02d.txt", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
      File f = SD.open(_buf, FILE_APPEND);
      f.printf("%04d/%02d/%02d %02d:%02d:%02d %s", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, msg);
      f.println();
      f.close();
      SD.end();
    }
    else
    {
      Serial.println("SD.begin error");
    }
  };
};

Log sd_log;
#endif