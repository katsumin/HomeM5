#ifndef _DATASTORE_H_
#define _DATASTORE_H_
#include <M5Stack.h>
#include <map>

#define WATT_HOUR_POINTS (48)
#define WATT_HOUR_LAST_POINT (WATT_HOUR_POINTS - 1)

class WattHour
{
private:
  time_t _time;
  float _value;
  float _values[WATT_HOUR_POINTS]; // 48コマ分

public:
  static int time2Index(time_t epoch)
  {
    long t = epoch % (60 * 60 * 24);
    t /= (30 * 60);
    return (int)t;
  };
  inline static int nextIndex(int index) { return (index + 1) % WATT_HOUR_POINTS; };
  inline static int prevIndex(int index) { return (index + WATT_HOUR_LAST_POINT) % WATT_HOUR_POINTS; };

  inline void setTime(time_t t) { _time = t; };
  inline time_t getTime() { return _time; };
  inline void setValue(float v) { _value = v; };
  inline float getValue() { return _value; };
  void init()
  {
    for (int i = 0; i < WATT_HOUR_POINTS; i++)
      _values[i] = -1;
  };
  void updateValues(float v, time_t t)
  {
    float preValue = getValue();
    time_t preTime = getTime();
    if (preValue > 0)
    {
      // 測定間隔が空いたコマを無効値で埋める
      int index = time2Index(preTime);
      int diff = time2Index(t - preTime) - 1;
      diff = (diff > WATT_HOUR_LAST_POINT) ? WATT_HOUR_LAST_POINT : diff;
      for (int i = 0; i < diff; i++)
      {
        index %= WATT_HOUR_POINTS;
        _values[index++] = -1;
      }
    }
    // 最新値で更新
    int curIndex = prevIndex(time2Index(t));
    setTime(t);
    _values[curIndex] = v - preValue;
    setValue(v);
  };
  inline float getValueAtIndex(int index) { return _values[index]; };
};

class DataStore
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
  // time_t _wattHourPlusTime = 0;
  // float _wattHourPlus = 0;
  // float _wattHourPlusMap[WATT_HOUR_POINTS]; // 48コマ分
  WattHour _plus;
  // 負方向積算電力量
  // time_t _wattHourMinusTime = 0;
  // float _wattHourMinus = 0;
  // float _wattHourMinusMap[WATT_HOUR_POINTS]; // 48コマ分
  WattHour _minus;
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

public:
  inline void setTemperature(float temp) { _temperature = temp; };
  inline float getTemperature() { return _temperature; };
  inline void setHumidity(float hum) { _humidity = hum; };
  inline float getHumidity() { return _humidity; };
  inline void setPressure(float press) { _pressure = press; };
  inline float getPressure() { return _pressure; };
  inline void setPower(long power) { _power = power; };
  inline long getPower() { return _power; };
  // static int time2Index(time_t epoch)
  // {
  //   long t = epoch % (60 * 60 * 24);
  //   t /= (30 * 60);
  //   return (int)t;
  // }
  // inline int nextIndex(int index) { return (index + 1) % WATT_HOUR_POINTS; };
  // inline int prevIndex(int index) { return (index + WATT_HOUR_LAST_POINT) % WATT_HOUR_POINTS; };
  void init()
  {
    // for (int i = 0; i < WATT_HOUR_POINTS; i++)
    // {
    //   _wattHourPlusMap[i] = -1;
    //   _wattHourMinusMap[i] = -1;
    // }
    _plus.init();
    _minus.init();
  }
  void setWattHourPlus(float plus, time_t t)
  {
    _plus.updateValues(plus, t);
    // if (_wattHourPlusTime > 0)
    // {
    //   // 測定間隔が空いたコマを無効値で埋める
    //   int index = time2Index(_wattHourPlusTime);
    //   int diff = time2Index(t - _wattHourPlusTime) - 1;
    //   diff = (diff > WATT_HOUR_LAST_POINT) ? WATT_HOUR_LAST_POINT : diff;
    //   for (int i = 0; i < diff; i++)
    //   {
    //     index %= WATT_HOUR_POINTS;
    //     _wattHourPlusMap[index++] = -1;
    //   }
    // }
    // // 最新値で更新
    // // int curIndex = (time2Index(t) + WATT_HOUR_LAST_POINT) % WATT_HOUR_POINTS;
    // int curIndex = prevIndex(time2Index(t));
    // _wattHourPlusTime = t;
    // _wattHourPlusMap[curIndex] = plus - _wattHourPlus;
    // _wattHourPlus = plus;
    // // Serial.println(curIndex);
    // // for (int i = 0; i < WATT_HOUR_POINTS; i++)
    // // {
    // //   Serial.printf(" %02d, %4.1f", i, _wattHourPlusMap[i]);
    // //   Serial.println();
    // // }
  };
  inline time_t getWattHourPlusTime() { return _plus.getTime(); };
  inline float getWattHourPlus() { return _plus.getValue(); };
  inline float getWattHourPlusAtIndex(int index) { return _plus.getValueAtIndex(index); };
  void setWattHourMinus(float minus, time_t t)
  {
    _minus.updateValues(minus, t);
    // if (_wattHourMinus > 0)
    // {
    //   int index = time2Index(_wattHourMinusTime) + 1;
    //   int diff = time2Index(t - _wattHourMinusTime) - 1;
    //   diff = (diff > 47) ? 47 : diff;
    //   for (int i = 0; i < diff; i++)
    //   {
    //     index %= WATT_HOUR_POINTS;
    //     _wattHourMinusMap[index++] = -1;
    //   }
    // }
    // int curIndex = time2Index(t);
    // _wattHourMinusTime = t;
    // _wattHourMinusMap[curIndex] = minus - _wattHourMinus;
    // _wattHourMinus = minus;
  };
  inline time_t getWattHourMinusTime() { return _minus.getTime(); };
  inline float getWattHourMinus() { return _minus.getValue(); };
  inline float getWattHourMinusAtIndex(int index) { return _minus.getValueAtIndex(index); };
  inline void setEcoPower(long power) { _ecoPower = power; };
  inline long getEcoPower() { return _ecoPower; };
  inline void setEcoTank(long tank) { _ecoTank = tank; };
  inline long getEcoTank() { return _ecoTank; };
  inline void setAirPower(long power) { _airPower = power; };
  inline long getAirPower() { return _airPower; };
  inline void setAirTempRoom(float temp) { _tempRoom = temp; };
  inline float getAirTempRoom() { return _tempRoom; };
  inline void setAirTempOut(float temp) { _tempOut = temp; };
  inline float getAirTempOut() { return _tempOut; };
};

#endif