#ifndef _RTC_H_
#define _RTC_H_
#include <M5Stack.h>
#include "NtpClient.h"

class Rtc
{
private:
  // 時刻
  time_t _epoch_time;
  time_t _diff_time;
  // NTP
  NtpClient *_ntp;

private:
  static void _callback(void *arg)
  {
    Serial.println("callbacked on Rtc.");
    Rtc *r = (Rtc *)arg;
    r->setTime(r->getNtp()->getEpocTime());
    tm *t = r->getTm();
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    Serial.println(buf);
  };

public:
  inline NtpClient *getNtp() { return _ntp; };
  inline void setTime(time_t epoch)
  {
    _epoch_time = epoch;
    _diff_time = millis() / 1000;
  };
  inline time_t getTime() { return _epoch_time + 3600 * 9; };
  inline void adjust()
  {
    _ntp->update();
  };
  void init(NtpClient *ntp)
  {
    _ntp = ntp;
    _ntp->setCallback(_callback, this);
    _ntp->update();
  };
  tm *getTm()
  {
    setTime(_epoch_time + (millis() / 1000 - _diff_time));
    time_t epoch = _epoch_time + 3600 * 9;
    tm *t = localtime(&epoch);
    return t;
  };
};

#endif