#ifndef _MAIN_VIEW_H_
#define _MAIN_VIEW_H_
#include <M5Stack.h>
#include "Free_Fonts.h"
#include "DataStore.h"
#include "View.h"

class MainView : public View
{
private:
  // 1文字幅
  int16_t _fontWidth;
  // 1文字高
  int16_t _fontHeight;
  // font1高
  int16_t _font1Height;
  // //
  // boolean _enable;
  // DataStore
  DataStore *_store;

public:
  MainView(DataStore *store)
  {
    setDataStore(store);
  };
  ~MainView(){};
  inline void setDataStore(DataStore *store) { _store = store; };
  // inline boolean isEnable() { return _enable; };
  // inline void setEnable(boolean enable) { _enable = enable; };
  virtual void init()
  {
    // setTime(0);
    M5.Lcd.setTextDatum(TL_DATUM);
    M5.Lcd.setFreeFont(FF9); // Select the font
    _fontHeight = M5.Lcd.fontHeight(GFXFF);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    _font1Height = M5.Lcd.fontHeight();
    // M5.Lcd.fillRect(0, 0, 320, _font1Height + 2, BLACK);
    int32_t y = _font1Height * 2;
    M5.Lcd.fillRect(0, 16, M5.Lcd.width(), M5.Lcd.height() - 16 * 2, BLACK);
    // M5.Lcd.fillRect(0, y, 320, y + _fontHeight * 10, BLACK);
    M5.Lcd.drawString("BME280", 0, y, GFXFF);
    y += _fontHeight * 3;
    M5.Lcd.drawString("Smart", 0, y, GFXFF);
    M5.Lcd.drawString("Meter", 0, y + _font1Height, GFXFF);
    y += _fontHeight * 3;
    M5.Lcd.drawString("Ecocute", 0, y, GFXFF);
    y += _fontHeight * 2;
    M5.Lcd.drawString("Air-con", 0, y, GFXFF);
    // _fontWidth = M5.Lcd.textWidth(" ", GFXFF);
    _fontWidth = 11;
    int32_t x = _fontWidth * 4;
    y = _font1Height * 2;
    M5.Lcd.setFreeFont(FF9); // Select the font
    M5.Lcd.drawString("temperature:          C", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("humidity   :          %H", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("pressure   :          hPa", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("power      :          W", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("watt-hour +:          kWh", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("watt-hour -:          kWh", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("power      :          W", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("tank       :          L", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("power      :          W", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("temp. room :          C", x, y, GFXFF);
    y += _fontHeight;
    M5.Lcd.drawString("temp. out  :          C", x, y, GFXFF);
    // setEnable(true);
  }
  virtual void update()
  {
    if (isEnable())
    {
      char buf[32];
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setFreeFont(FF5); // Select the font
      M5.Lcd.setTextColor(YELLOW);
      int x = 17 * _fontWidth;
      int32_t y = _font1Height * 2;
      M5.Lcd.fillRect(x, y, _fontWidth * 9, _fontHeight * 11, BLACK);
      snprintf(buf, sizeof(buf), "%9.1f", _store->getTemperature());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9.1f", _store->getHumidity());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9.1f", _store->getPressure());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      // x = 14 * _fontWidth;
      snprintf(buf, sizeof(buf), "%9ld", _store->getPower());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9.1f", _store->getWattHourPlus());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9.1f", _store->getWattHourMinus());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9ld", _store->getEcoPower());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9ld", _store->getEcoTank());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9ld", _store->getAirPower());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9.1f", _store->getAirTempRoom());
      M5.Lcd.drawString(buf, x, y, GFXFF);
      y += _fontHeight;
      snprintf(buf, sizeof(buf), "%9.1f", _store->getAirTempOut());
      M5.Lcd.drawString(buf, x, y, GFXFF);
    }
  };
};

#endif