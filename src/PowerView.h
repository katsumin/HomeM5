#ifndef _POWERVIEW_H_
#define _POWERVIEW_H_
#include <M5Stack.h>
#include "Free_Fonts.h"
#include "DataStore.h"
#include "Rtc.h"
#include "View.h"

#define WATT_HOUR_FS (1.5f)
#define FS_PIXEL (40)
class PowerView : public View
{
private:
  // 1文字高
  int16_t _fontHeight;
  // font1高
  int16_t _font1Height;
  // boolean _enable;
  DataStore *_store;
  Rtc *_rtc;
  int _currentIndex;

public:
  PowerView(DataStore *store, Rtc *rtc)
  {
    setDataStore(store);
    setRtc(rtc);
  }
  ~PowerView(){};
  inline void setDataStore(DataStore *store) { _store = store; };
  inline void setRtc(Rtc *rtc) { _rtc = rtc; };
  // inline boolean isEnable() { return _enable; };
  // inline void setEnable(boolean enable) { _enable = enable; };
  virtual void init()
  {
    M5.Lcd.setFreeFont(FF9); // Select the font
    _fontHeight = M5.Lcd.fontHeight(GFXFF);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    _font1Height = M5.Lcd.fontHeight();
    int32_t y = 16;
    M5.Lcd.fillRect(0, y, M5.Lcd.width(), M5.Lcd.height() - y * 2, BLACK);             // ヘッダ以外を消去
    M5.Lcd.drawRoundRect(0, y, M5.Lcd.width(), M5.Lcd.height() - y * 2 - 4, 5, WHITE); // 外枠
    M5.Lcd.drawFastHLine(0, 90, M5.Lcd.width(), WHITE);                                // 区切り線
    _currentIndex = -1;

    // 瞬時値
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setFreeFont(FF5); // Select the font
    M5.Lcd.setTextColor(WHITE);
    int x = 160 - 18 / 2 * 11;
    M5.Lcd.drawString("SmartMeter:      W", x, 25);
    M5.Lcd.drawString("Eco-cute  :      W", x, 45);
    M5.Lcd.drawString("Air-con   :      W", x, 65);

    // setEnable(true);
  };
  virtual void update()
  {
    if (isEnable())
    {
      char buf[32];
      // 瞬時値
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setFreeFont(FF5); // Select the font
      M5.Lcd.setTextColor(YELLOW);
      int x = 160 + (-18 / 2 + 12) * 11;
      snprintf(buf, sizeof(buf), "%5ld", _store->getPower());
      M5.Lcd.fillRect(x, 25, 5 * 11, _fontHeight, BLACK);
      M5.Lcd.drawString(buf, x, 25);
      snprintf(buf, sizeof(buf), "%5ld", _store->getEcoPower());
      M5.Lcd.fillRect(x, 45, 5 * 11, _fontHeight, BLACK);
      M5.Lcd.drawString(buf, x, 45);
      snprintf(buf, sizeof(buf), "%5ld", _store->getAirPower());
      M5.Lcd.fillRect(x, 65, 5 * 11, _fontHeight, BLACK);
      M5.Lcd.drawString(buf, x, 65);
      // 積算値
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setFreeFont(FF9); // Select the font
      M5.Lcd.setTextFont(1);
      M5.Lcd.setTextSize(1);
      time_t epoch = _store->getWattHourPlusTime();
      int index = WattHour::time2Index(epoch);
      if (_currentIndex != index)
      {
        _currentIndex = index;

        // グラフ表示
        int x = 17;
        int y = 155;
        int w = 5;
        int step_x = w + 1;
        M5.Lcd.fillRect(x, y - FS_PIXEL, step_x * WATT_HOUR_POINTS + 1, FS_PIXEL * 2 + 2, BLACK);
        M5.Lcd.drawFastHLine(x, y, step_x * WATT_HOUR_POINTS, WHITE);
        M5.Lcd.drawRect(x - 1, y - FS_PIXEL - 1, step_x * WATT_HOUR_POINTS + 2, FS_PIXEL * 2 + 3, WHITE);
        M5.Lcd.drawFastVLine(x - 1 + step_x * 12, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, BLUE);
        M5.Lcd.drawFastVLine(x - 1 + step_x * 24, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, BLUE);
        M5.Lcd.drawFastVLine(x - 1 + step_x * 36, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, BLUE);
        for (int i = 0; i < WATT_HOUR_POINTS; i++)
        {
          int in = (index + i) % WATT_HOUR_POINTS;
          if (in == WATT_HOUR_LAST_POINT)
            M5.Lcd.drawFastVLine(x + i * step_x + w, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, YELLOW);
          float f = _store->getWattHourPlusAtIndex(in);
          int dh = f / WATT_HOUR_FS * FS_PIXEL;
          dh = (dh > FS_PIXEL - 1) ? FS_PIXEL - 1 : dh;
          if (f >= 0)
            M5.Lcd.fillRect(x + i * step_x, y - dh, w, dh, GREEN);
          else
            M5.Lcd.drawRect(x + i * step_x, y - FS_PIXEL, w, FS_PIXEL, LIGHTGREY);
          f = _store->getWattHourMinusAtIndex(in);
          dh = f / WATT_HOUR_FS * FS_PIXEL;
          dh = (dh > FS_PIXEL - 1) ? FS_PIXEL - 1 : dh;
          if (f >= 0)
            M5.Lcd.fillRect(x + i * step_x, y + 1, w, dh, PINK);
          else
            M5.Lcd.drawRect(x + i * step_x, y + 1, w, FS_PIXEL, LIGHTGREY);
        }
        // X軸
        M5.Lcd.setTextColor(GREENYELLOW);
        M5.Lcd.fillRect(1, y + FS_PIXEL + _font1Height * 2 - _font1Height / 2, M5.Lcd.width() - 2, _font1Height, BLACK);
        for (int i = 0; i < 5; i++)
        {
          int step = i * 12;
          int in = (index + step) % WATT_HOUR_POINTS;
          snprintf(buf, sizeof(buf), "%02d:%02d", in / 2, (in % 2) * 30);
          M5.Lcd.drawString(buf, x - 1 + step_x * step, y + FS_PIXEL + _font1Height * 2 + 1);
        }
        // Y軸
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.drawString(" 1.5kWh", x - 1, y - FS_PIXEL - _font1Height);
        M5.Lcd.drawString("0", x - w, y);
        M5.Lcd.drawString("-1.5kWh", x - 1, y + FS_PIXEL + _font1Height);

        // 最新値表示
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.setTextDatum(TL_DATUM);
        int prevIndex = WattHour::prevIndex(index);
        float f = _store->getWattHourPlusAtIndex(prevIndex);
        if (f >= 0)
        {
          snprintf(buf, sizeof(buf), "plus : %7.1fkWh", f);
          w = M5.Lcd.textWidth(buf);
          M5.Lcd.fillRect(319 - w, y - FS_PIXEL - _font1Height * 2 - 3, w, _font1Height, BLACK);
          M5.Lcd.drawString(buf, 319 - w, y - FS_PIXEL - _font1Height * 2 - 3);
        }
        f = _store->getWattHourMinusAtIndex(prevIndex);
        if (f >= 0)
        {
          snprintf(buf, sizeof(buf), "minus: %7.1fkWh", f * (-1.0f));
          w = M5.Lcd.textWidth(buf);
          M5.Lcd.fillRect(319 - w, y - FS_PIXEL - _font1Height - 3, w, _font1Height, BLACK);
          M5.Lcd.drawString(buf, 319 - w, y - FS_PIXEL - _font1Height - 3);
        }
      }
    }
  };
};

#endif