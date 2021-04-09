#ifndef _POWERVIEW_H_
#define _POWERVIEW_H_
#include "DataStore.h"
#include "DeviceView.h"
#include "Device.h"

#define WATT_HOUR_POINTS (48)
#define WATT_HOUR_LAST_POINT (WATT_HOUR_POINTS - 1)
class WattHour
{
private:
    time_t _time = 0;
    float _value = 0.0;
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
        // Serial.printf("preValue: %f", preValue);
        // Serial.println();
        time_t preTime = getTime();
        if (preTime == t)
            return;
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

#define WATT_HOUR_FS (3.0f)
#define FS_PIXEL (76)
class PowerView : public DeviceView
{
private:
    // 1文字高
    int16_t _fontHeight;
    // font1高
    int16_t _font1Height;
    int _currentIndex;
    WattHour _plus;
    WattHour _minus;

public:
    PowerView(SmartMeter *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
    {
        _plus.init();
        _minus.init();
        setName("POWER");
    }
    ~PowerView(){};
    // inline void setDataStore(DataStore *store) { _store = store; };
    //   inline void setRtc(Rtc *rtc) { _rtc = rtc; };
    // inline void setNtp(NTPClient *ntp) { _ntp = ntp; };
    virtual void init()
    {
        // getLcd()->setFreeFont(FF9); // Select the font
        getLcd()->setFont(&fonts::lgfxJapanGothic_16);
        _fontHeight = getLcd()->fontHeight();
        getLcd()->setTextFont(1);
        getLcd()->setTextSize(1);
        _font1Height = getLcd()->fontHeight();
        int32_t y = 16;
        getLcd()->fillRect(0, y, getLcd()->width(), getLcd()->height() - y * 2, TFT_BLACK);             // ヘッダ以外を消去
        getLcd()->drawRoundRect(0, y, getLcd()->width(), getLcd()->height() - y * 2 - 4, 5, TFT_WHITE); // 外枠
        // getLcd()->drawFastHLine(0, 90, getLcd()->width(), WHITE);                                // 区切り線
        _currentIndex = -1;

        // 瞬時値
        getLcd()->setTextDatum(TL_DATUM);
        // getLcd()->setFreeFont(FF5); // Select the font
        getLcd()->setFont(&fonts::lgfxJapanGothic_16);
        // getLcd()->setTextFont(&fonts::Font0);
        getLcd()->setTextColor(TFT_WHITE);
        int x = 120 - 12 / 2 * 11;
        getLcd()->drawString("消費電力:       W", x, 25);
        // getLcd()->drawString("Eco-cute  :      W", x, 45);
        // getLcd()->drawString("Air-con   :      W", x, 65);

        // setEnable(true);
    };
    virtual void update()
    {
        // if (_sm == nullptr)
        // {
        //     std::map<String, Node *> *_nodes = _store->getNodes();
        //     for (auto itr = _nodes->begin(); itr != _nodes->end(); itr++)
        //     {
        //         std::map<uint32_t, Device *> *_devices = itr->second->getDevices();
        //         for (auto itrSub = _devices->begin(); itrSub != _devices->end(); itrSub++)
        //         {
        //             Device *d = itrSub->second;
        //             if (d->getClassType() == 0x0288)
        //             {
        //                 _sm = (SmartMeter *)d;
        //             }
        //         }
        //     }
        //     if (_sm == nullptr)
        //         return;
        // }
        SmartMeter *_sm = (SmartMeter *)getDevice();
        float wattHourP = _sm->getWattHourPlus();
        float wattHourM = _sm->getWattHourMinus();
        if (wattHourP < 0 || wattHourM < 0)
            return;
        _plus.updateValues(wattHourP, _sm->getTimePlus());
        _minus.updateValues(wattHourM, _sm->getTimeMinus());
        if (isEnable())
        {
            char buf[32];
            // 瞬時値
            getLcd()->setTextDatum(TL_DATUM);
            // getLcd()->setFreeFont(FF5); // Select the font
            getLcd()->setFont(&fonts::lgfxJapanGothic_16);
            getLcd()->setTextColor(TFT_YELLOW);
            int x = 120 + (-12 / 2 + 8) * 11;
            int32_t power = _sm->getPower();
            if (power <= 2147483645)
                snprintf(buf, sizeof(buf), "%5ld", power);
            // snprintf(buf, sizeof(buf), "%5ld", -1000);
            int w = getLcd()->textWidth(buf);
            getLcd()->fillRect(x, 25, w, _fontHeight, TFT_BLACK);
            getLcd()->drawString(buf, x, 25);
            //   snprintf(buf, sizeof(buf), "%5ld", _store->getEcoPower());
            //   getLcd()->fillRect(x, 45, 5 * 11, _fontHeight, BLACK);
            //   getLcd()->drawString(buf, x, 45);
            //   snprintf(buf, sizeof(buf), "%5ld", _store->getAirPower());
            //   getLcd()->fillRect(x, 65, 5 * 11, _fontHeight, BLACK);
            //   getLcd()->drawString(buf, x, 65);
            // 積算値
            getLcd()->setTextDatum(MC_DATUM);
            // getLcd()->setFreeFont(FF9); // Select the font
            getLcd()->setFont(&fonts::Font0);
            // getLcd()->setTextFont(1);
            getLcd()->setTextSize(1);
            // time_t epoch = _store->getWattHourPlusTime();
            time_t epoch = _plus.getTime();
            int index = WattHour::time2Index(epoch);
            if (_currentIndex != index)
            {
                _currentIndex = index;

                // グラフ表示
                int x = 17;
                int y = 25 + FS_PIXEL + _font1Height * 2 + 3;
                int w = 5;
                int step_x = w + 1;
                getLcd()->fillRect(x, y - FS_PIXEL, step_x * WATT_HOUR_POINTS + 1, FS_PIXEL * 2 + 2, TFT_BLACK);
                getLcd()->drawFastHLine(x, y, step_x * WATT_HOUR_POINTS, TFT_WHITE);
                getLcd()->drawRect(x - 1, y - FS_PIXEL - 1, step_x * WATT_HOUR_POINTS + 2, FS_PIXEL * 2 + 3, TFT_WHITE);
                getLcd()->drawFastVLine(x - 1 + step_x * 12, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, TFT_BLUE);
                getLcd()->drawFastVLine(x - 1 + step_x * 24, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, TFT_BLUE);
                getLcd()->drawFastVLine(x - 1 + step_x * 36, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, TFT_BLUE);
                for (int i = 0; i < WATT_HOUR_POINTS; i++)
                {
                    int in = (index + i) % WATT_HOUR_POINTS;
                    if (in == WATT_HOUR_LAST_POINT)
                        getLcd()->drawFastVLine(x + i * step_x + w, y - FS_PIXEL + 1, FS_PIXEL * 2 - 1, TFT_YELLOW);
                    // float f = _store->getWattHourPlusAtIndex(in);
                    float f = _plus.getValueAtIndex(in);
                    // Serial.printf("plus %f at %d", f, i);
                    // Serial.println();
                    int dh = f / WATT_HOUR_FS * FS_PIXEL;
                    dh = (dh > FS_PIXEL - 1) ? FS_PIXEL - 1 : dh;
                    if (f >= 0)
                        getLcd()->fillRect(x + i * step_x, y - dh, w, dh, TFT_GREEN);
                    else
                        getLcd()->drawRect(x + i * step_x, y - FS_PIXEL, w, FS_PIXEL, TFT_LIGHTGREY);
                    // f = _store->getWattHourMinusAtIndex(in);
                    f = _minus.getValueAtIndex(in);
                    // Serial.printf("minus %f at %d", f, i);
                    // Serial.println();
                    dh = f / WATT_HOUR_FS * FS_PIXEL;
                    dh = (dh > FS_PIXEL - 1) ? FS_PIXEL - 1 : dh;
                    if (f >= 0)
                        getLcd()->fillRect(x + i * step_x, y + 1, w, dh, TFT_MAGENTA);
                    else
                        getLcd()->drawRect(x + i * step_x, y + 1, w, FS_PIXEL, TFT_LIGHTGREY);
                }
                // X軸
                getLcd()->setTextColor(TFT_GREENYELLOW);
                getLcd()->fillRect(1, y + FS_PIXEL + _font1Height * 2 - _font1Height / 2, getLcd()->width() - 2, _font1Height, TFT_BLACK);
                for (int i = 0; i < 5; i++)
                {
                    int step = i * 12;
                    int in = (index + step) % WATT_HOUR_POINTS;
                    snprintf(buf, sizeof(buf), "%02d:%02d", in / 2, (in % 2) * 30);
                    getLcd()->drawString(buf, x - 1 + step_x * step, y + FS_PIXEL + _font1Height * 2 + 1);
                }
                // Y軸
                getLcd()->setTextColor(TFT_WHITE);
                getLcd()->drawString(" 3.0kWh", x - 1, y - FS_PIXEL - _font1Height);
                getLcd()->drawString("0", x - w, y);
                getLcd()->drawString("-3.0kWh", x - 1, y + FS_PIXEL + _font1Height);

                // 最新値表示
                getLcd()->setTextColor(TFT_YELLOW);
                getLcd()->setTextDatum(TL_DATUM);
                int prevIndex = WattHour::prevIndex(index);
                // float f = _store->getWattHourPlusAtIndex(prevIndex);
                float f = _plus.getValueAtIndex(prevIndex);
                if (0.0 <= f && f <= 100000.0)
                {
                    snprintf(buf, sizeof(buf), "plus : %8.1fkWh", f);
                    w = getLcd()->textWidth(buf);
                    getLcd()->fillRect(319 - w, y - FS_PIXEL - _font1Height * 2 - 3, w, _font1Height, TFT_BLACK);
                    getLcd()->drawString(buf, 319 - w, y - FS_PIXEL - _font1Height * 2 - 3);
                }
                // f = _store->getWattHourMinusAtIndex(prevIndex);
                f = _minus.getValueAtIndex(prevIndex);
                if (0.0 <= f && f <= 100000.0)
                {
                    snprintf(buf, sizeof(buf), "minus: %8.1fkWh", f * (-1.0f));
                    w = getLcd()->textWidth(buf);
                    getLcd()->fillRect(319 - w, y - FS_PIXEL - _font1Height - 3, w, _font1Height, TFT_BLACK);
                    getLcd()->drawString(buf, 319 - w, y - FS_PIXEL - _font1Height - 3);
                }
            }
        }
    };
};

#endif