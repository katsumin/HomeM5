#ifndef _LOG_H_
#define _LOG_H_
#include <M5Stack.h>

class Log
{
private:
  // 時刻
  time_t _epoch_time;
  time_t _diff_time;

public:
  inline void setTime(time_t epoch)
  {
    _epoch_time = epoch;
    _diff_time = millis() / 1000;
  };
  void out(const char *msg)
  {
    bool flgSD = SD.begin(TFCARD_CS_PIN, SPI, 40000000);
    if (flgSD)
    {
      setTime(_epoch_time + (millis() / 1000 - _diff_time));
      time_t epoch = _epoch_time + 3600 * 9;
      tm *t = localtime(&epoch);
      File f = SD.open("/log.txt", FILE_APPEND);
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