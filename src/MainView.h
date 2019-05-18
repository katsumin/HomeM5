#ifndef _MAIN_VIEW_H_
#define _MAIN_VIEW_H_
#include <M5Stack.h>

class MainView
{
private:
  // 気温
  float _temperature = 0;
  // 湿度
  float _humidity = 0;
  // 気圧
  float _pressure = 0;
  // 瞬時電力
  long _power = 0;
  // 正方向積算電力量
  float _wattHourPlus = 0;
  // 負方向積算電力量
  float _wattHourMinus = 0;
  // 1文字幅
  int16_t _fontWidth;
  // 1文字高
  int16_t _fontHeight;

public:
  inline void setTemperature(float temp) { _temperature = temp; };
  inline void setHumidity(float hum) { _humidity = hum; };
  inline void setPressure(float press) { _pressure = press; };
  inline void setPower(long power) { _power = power; };
  inline void setWattHourPlus(float plus) { _wattHourPlus = plus; };
  inline void setWattHoueMinus(float minus) { _wattHourMinus = minus; };
  void init()
  {
    M5.Lcd.setTextSize(2);
    _fontHeight = M5.Lcd.fontHeight(1);
    _fontWidth = M5.Lcd.textWidth(" ");
    M5.Lcd.drawString("temperature:           C", 0, 0);
    M5.Lcd.drawString("humidity   :           %H", 0, _fontHeight);
    M5.Lcd.drawString("pressure   :           hPa", 0, _fontHeight * 2);
    M5.Lcd.drawString("power      :           W", 0, _fontHeight * 3);
    M5.Lcd.drawString("watt-hour +:           kWh", 0, _fontHeight * 4);
    M5.Lcd.drawString("watt-hour -:           kWh", 0, _fontHeight * 5);
  }
  void update()
  {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(YELLOW);
    int x = 13 * _fontWidth;
    M5.Lcd.fillRect(x, 0, _fontWidth * 9, _fontHeight * 6, BLACK);
    char buf[16];
    x = 16 * _fontWidth;
    snprintf(buf, sizeof(buf), "%6.1f", _temperature);
    M5.Lcd.drawString(buf, x, _fontHeight * 0);
    snprintf(buf, sizeof(buf), "%6.1f", _humidity);
    M5.Lcd.drawString(buf, x, _fontHeight * 1);
    snprintf(buf, sizeof(buf), "%6.1f", _pressure);
    M5.Lcd.drawString(buf, x, _fontHeight * 2);
    x = 13 * _fontWidth;
    snprintf(buf, sizeof(buf), "%9ld", _power);
    M5.Lcd.drawString(buf, x, _fontHeight * 3);
    snprintf(buf, sizeof(buf), "%9.1f", _wattHourPlus);
    M5.Lcd.drawString(buf, x, _fontHeight * 4);
    snprintf(buf, sizeof(buf), "%9.1f", _wattHourMinus);
    M5.Lcd.drawString(buf, x, _fontHeight * 5);
  };
};

#endif