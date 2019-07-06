#ifndef _HEADVIEW_H_
#define _HEADVIEW_H_
#include <M5Stack.h>
#include "Free_Fonts.h"
#include "Rtc.h"

class HeadView
{
private:
  // IPアドレス
  IPAddress _ipaddr;
  // NW type
  String _nwType;
  // RTC
  Rtc *_rtc;
  // font1高
  int16_t _font1Height;
  char _buf[32];
  void _printNwInfo()
  {
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    snprintf(_buf, sizeof(_buf), "%s(%s)", _ipaddr.toString().c_str(), _nwType.c_str());
    int w = M5.Lcd.textWidth(_buf);
    M5.Lcd.fillRect(0, 0, w, _font1Height + 2, BLACK);
    M5.Lcd.drawString(_buf, 1, 1);
  }
  void _printDate()
  {
    M5.Lcd.setTextDatum(TR_DATUM);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    tm *t = _rtc->getTm();
    snprintf(_buf, sizeof(_buf), "%04d/%02d/%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    int w = M5.Lcd.textWidth(_buf);
    M5.Lcd.fillRect(319 - w, 0, w, _font1Height + 2, BLACK);
    M5.Lcd.drawString(_buf, 319, 1);
  }

public:
  inline void setIpAddress(IPAddress addr)
  {
    _ipaddr = addr;
    _printNwInfo();
  };
  inline void setNwType(const char *type)
  {
    _nwType = String(type);
    _printNwInfo();
  };
  inline void setRtc(Rtc *rtc) { _rtc = rtc; };
  void init()
  {
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    _font1Height = M5.Lcd.fontHeight();
  };
  void update()
  {
    _printDate();
  };
};

HeadView headView;
#endif