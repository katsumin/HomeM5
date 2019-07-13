#ifndef _LOG_H_
#define _LOG_H_
#include <M5Stack.h>
#include "Rtc.h"

#define LOG_DIR ("/logs")
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
      if (!SD.exists(LOG_DIR))
        SD.mkdir(LOG_DIR);
      tm *t = _rtc->getTm();
      snprintf(_buf, sizeof(_buf), "%s/log_%04d%02d%02d.txt", LOG_DIR, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
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