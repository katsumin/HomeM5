#ifndef _MAIN_VIEW_H_
#define _MAIN_VIEW_H_
#include <M5Stack.h>
#include "Free_Fonts.h"

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
  // IPアドレス
  IPAddress _ipaddr;
  // 時刻
  time_t _epoch_time;
  time_t _diff_time;
  // NW type
  String _nwType;
  // ecocute-power
  long _ecoPower = 0;
  // ecocute-tank
  long _ecoTank = 0;
  // air-con power
  long _airPower = 0;
  // air-con temp. room
  float _tempRoom = 0;
  // air-con temp. out
  float _tempOut = 0;
  // 1文字幅
  int16_t _fontWidth;
  // 1文字高
  int16_t _fontHeight;
  // font1高
  int16_t _font1Height;

public:
  inline void setTemperature(float temp) { _temperature = temp; };
  inline void setHumidity(float hum) { _humidity = hum; };
  inline void setPressure(float press) { _pressure = press; };
  inline void setPower(long power) { _power = power; };
  inline void setWattHourPlus(float plus) { _wattHourPlus = plus; };
  inline void setWattHoueMinus(float minus) { _wattHourMinus = minus; };
  inline void setIpAddress(IPAddress addr) { _ipaddr = addr; };
  inline void setTime(time_t epoch)
  {
    _epoch_time = epoch;
    _diff_time = millis() / 1000;
  };
  inline void setNwType(const char *type) { _nwType = String(type); };
  inline void setEcoPower(long power) { _ecoPower = power; };
  inline void setEcoTank(long tank) { _ecoTank = tank; };
  inline void setAirPower(long power) { _airPower = power; };
  inline void setAirTempRoom(float temp) { _tempRoom = temp; };
  inline void setAirTempOut(float temp) { _tempOut = temp; };
  void init()
  {
    setTime(0);
    M5.Lcd.setFreeFont(FF9); // Select the font
    _fontHeight = M5.Lcd.fontHeight(GFXFF);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    _font1Height = M5.Lcd.fontHeight();
    M5.Lcd.fillRect(0, 0, 320, _font1Height + 2, BLACK);
    int32_t y = _font1Height * 2;
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
  }
  void update()
  {
    char buf[32];
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.fillRect(0, 0, 320, _font1Height + 2, BLACK);
    snprintf(buf, sizeof(buf), "%s(%s)", _ipaddr.toString().c_str(), _nwType.c_str());
    M5.Lcd.drawString(buf, 1, 1);
    setTime(_epoch_time + (millis() / 1000 - _diff_time));
    time_t epoch = _epoch_time + 3600 * 9;
    tm *t = localtime(&epoch);
    snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    int w = M5.Lcd.textWidth(buf);
    // Serial.print(w);
    // Serial.print(": ");
    // Serial.println(buf);
    M5.Lcd.drawString(buf, 319 - w, 1);

    M5.Lcd.setFreeFont(FF5); // Select the font
    M5.Lcd.setTextColor(YELLOW);
    int x = 17 * _fontWidth;
    int32_t y = _font1Height * 2;
    M5.Lcd.fillRect(x, y, _fontWidth * 9, _fontHeight * 11, BLACK);
    snprintf(buf, sizeof(buf), "%9.1f", _temperature);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9.1f", _humidity);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9.1f", _pressure);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    // x = 14 * _fontWidth;
    snprintf(buf, sizeof(buf), "%9ld", _power);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9.1f", _wattHourPlus);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9.1f", _wattHourMinus);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9ld", _ecoPower);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9ld", _ecoTank);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9ld", _airPower);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9.1f", _tempRoom);
    M5.Lcd.drawString(buf, x, y, GFXFF);
    y += _fontHeight;
    snprintf(buf, sizeof(buf), "%9.1f", _tempOut);
    M5.Lcd.drawString(buf, x, y, GFXFF);
  };
};

#endif