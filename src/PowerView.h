#ifndef _POWERVIEW_H_
#define _POWERVIEW_H_
#include "DataStore.h"
#include "DeviceView.h"
#include "Device.h"

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

public:
    PowerView(SmartMeter *device, TFT_eSPI *lcd) : DeviceView(device, lcd)
    {
        setName("POWER");
    }
    ~PowerView(){};
    virtual void init()
    {
        getLcd()->setFont(&fonts::lgfxJapanGothic_16);
        _fontHeight = getLcd()->fontHeight();
        getLcd()->setTextFont(1);
        getLcd()->setTextSize(1);
        _font1Height = getLcd()->fontHeight();
        int32_t y = 16;
        getLcd()->fillRect(0, y, getLcd()->width(), getLcd()->height() - y * 2, TFT_BLACK);             // ヘッダ以外を消去
        getLcd()->drawRoundRect(0, y, getLcd()->width(), getLcd()->height() - y * 2 - 4, 5, TFT_WHITE); // 外枠
        _currentIndex = -1;

        // 瞬時値
        getLcd()->setTextDatum(TL_DATUM);
        // getLcd()->setFreeFont(FF5); // Select the font
        getLcd()->setFont(&fonts::lgfxJapanGothic_16);
        // getLcd()->setTextFont(&fonts::Font0);
        getLcd()->setTextColor(TFT_WHITE);
        int x = 120 - 12 / 2 * 11;
        getLcd()->drawString("消費電力:       W", x, 25);
    };
    virtual void update()
    {
        SmartMeter *_sm = (SmartMeter *)getDevice();
        if (isEnable())
        {
            char buf[32];
            // 瞬時値
            getLcd()->setTextDatum(TL_DATUM);
            getLcd()->setFont(&fonts::lgfxJapanGothic_16);
            getLcd()->setTextColor(TFT_YELLOW);
            int x = 120 + (-12 / 2 + 8) * 11;
            int32_t power = _sm->getPower();
            if (power <= 2147483645)
                snprintf(buf, sizeof(buf), "%5ld", power);
            int w = getLcd()->textWidth(buf);
            getLcd()->fillRect(x, 25, w, _fontHeight, TFT_BLACK);
            getLcd()->drawString(buf, x, 25);
            // 積算値
            getLcd()->setTextDatum(MC_DATUM);
            getLcd()->setFont(&fonts::Font0);
            getLcd()->setTextSize(1);
            time_t epoch = _sm->getWattHourObjPlus().getTime();
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
                    float f = _sm->getWattHourObjPlus().getValueAtIndex(in);
                    int dh = f / WATT_HOUR_FS * FS_PIXEL;
                    dh = (dh > FS_PIXEL - 1) ? FS_PIXEL - 1 : dh;
                    if (f >= 0)
                        getLcd()->fillRect(x + i * step_x, y - dh, w, dh, TFT_GREEN);
                    else
                        getLcd()->drawRect(x + i * step_x, y - FS_PIXEL, w, FS_PIXEL, TFT_LIGHTGREY);
                    f = _sm->getWattHourObjMinus().getValueAtIndex(in);
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
                float f = _sm->getWattHourObjPlus().getValueAtIndex(prevIndex);
                if (0.0 <= f && f <= 100000.0)
                {
                    snprintf(buf, sizeof(buf), "plus : %8.1fkWh", f);
                    w = getLcd()->textWidth(buf);
                    getLcd()->fillRect(319 - w, y - FS_PIXEL - _font1Height * 2 - 3, w, _font1Height, TFT_BLACK);
                    getLcd()->drawString(buf, 319 - w, y - FS_PIXEL - _font1Height * 2 - 3);
                }
                f = _sm->getWattHourObjMinus().getValueAtIndex(prevIndex);
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