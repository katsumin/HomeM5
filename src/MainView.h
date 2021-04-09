#ifndef _MAIN_VIEW_H_
#define _MAIN_VIEW_H_
// #include <M5Stack.h>
// #include "Free_Fonts.h"
#include "DataStore.h"
#include "View.h"
#include <map>

class MainView : public View
{
private:
    // 1文字幅
    int16_t _fontWidth;
    // 1文字高
    int16_t _fontHeight;
    // font1高
    int16_t _font1Height;
    // DataStore
    DataStore *_store;

public:
    MainView(DataStore *store, TFT_eSPI *lcd) : View(lcd)
    {
        setDataStore(store);
    };
    ~MainView(){};
    inline void setDataStore(DataStore *store) { _store = store; };
    virtual void init()
    {
        // getLcd()->setTextColor(TFT_WHITE);
        // getLcd()->setTextDatum(TL_DATUM);
        // // getLcd()->setFreeFont(FF9); // Select the font
        // _fontHeight = getLcd()->fontHeight();
        // getLcd()->setTextFont(1);
        // getLcd()->setTextSize(1);
        // _font1Height = getLcd()->fontHeight();
        // int32_t y = _font1Height * 2;
        // getLcd()->fillRect(0, 16, getLcd()->width(), getLcd()->height() - 16 * 2, TFT_BLACK);
        // getLcd()->drawString("瞬時電力", 0, y);
        // y += _fontHeight * 1;
        // getLcd()->drawString("太陽光", 0, y);
        // y += _fontHeight * 1;
        // getLcd()->drawString("蓄電池", 0, y);
        // y += _fontHeight * 2;
        // getLcd()->drawString("Air-con", 0, y);
        // y += _fontHeight * 2;
        // getLcd()->drawString("Air-con", 0, y);
        // _fontWidth = 11;
        // int32_t x = _fontWidth * 4;
        // y = _font1Height * 2;
        // // getLcd()->setFreeFont(FF9); // Select the font
        // getLcd()->drawString("スマートメータ:          W", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("発電量    :          W", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("エアコン    :          %", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("エアコン    :          ", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("エコキュート:          kWh", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("積算電力量逆:          kWh", x, y);
        // y += _fontHeight;
        // y += _fontHeight;
        // getLcd()->drawString("power      :          W", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("tank       :          L", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("power      :          W", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("temp. room :          C", x, y);
        // y += _fontHeight;
        // getLcd()->drawString("temp. out  :          C", x, y);
        int32_t y = 16;
        getLcd()->fillRect(0, y, getLcd()->width(), getLcd()->height() - y * 2, TFT_BLACK);             // ヘッダ以外を消去
        getLcd()->drawRoundRect(0, y, getLcd()->width(), getLcd()->height() - y * 2 - 4, 5, TFT_WHITE); // 外枠
        getLcd()->setTextColor(TFT_YELLOW);
        getLcd()->setTextDatum(ML_DATUM);
        getLcd()->setFont(&fonts::lgfxJapanGothic_32);
        const char *s = "自宅状況モニタ";
        int len = getLcd()->textWidth(s);
        int32_t x = (getLcd()->width() - len) / 2;
        y = getLcd()->height() / 3;
        getLcd()->drawString(s, x, y);
    }
    virtual void update()
    {
        if (isEnable())
        {
            // char buf[32];
            // getLcd()->setTextDatum(TL_DATUM);
            // // getLcd()->setFreeFont(FF5); // Select the font
            // getLcd()->setTextColor(TFT_YELLOW);
            // int x = 17 * _fontWidth;
            // int32_t y = _font1Height * 2;
            // getLcd()->fillRect(x, y, _fontWidth * 9, _fontHeight * 11, TFT_BLACK);
            // snprintf(buf, sizeof(buf), "%9.1f", _store->getTemperature());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9.1f", _store->getHumidity());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9.1f", _store->getPressure());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9ld", _store->getPower());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // // snprintf(buf, sizeof(buf), "%9.1f", _store->getWattHourPlus());
            // snprintf(buf, sizeof(buf), "%9.1f", 0.0);
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // // snprintf(buf, sizeof(buf), "%9.1f", _store->getWattHourMinus());
            // snprintf(buf, sizeof(buf), "%9.1f", 0.0);
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9ld", _store->getEcoPower());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9ld", _store->getEcoTank());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9ld", _store->getAirPower());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9.1f", _store->getAirTempRoom());
            // getLcd()->drawString(buf, x, y);
            // y += _fontHeight;
            // snprintf(buf, sizeof(buf), "%9.1f", _store->getAirTempOut());
            // getLcd()->drawString(buf, x, y);
            char buf[64];
            int count = 0;
            std::map<String, Node *> *nodes = _store->getNodes();
            for (auto itr = nodes->begin(); itr != nodes->end(); ++itr)
            {
                std::map<uint32_t, Device *> *devices = itr->second->getDevices();
                count += devices->size();
            }
            getLcd()->setTextColor(TFT_WHITE);
            getLcd()->setFont(&fonts::lgfxJapanGothic_24);
            getLcd()->setTextDatum(MC_DATUM);
            int16_t fh = getLcd()->fontHeight();
            snprintf(buf, sizeof(buf), "デバイス数: %2d", count);
            int w = getLcd()->textWidth(buf);
            int x = getLcd()->width() / 2;
            int y = getLcd()->height() * 2 / 3;
            getLcd()->fillRect(x - w / 2, y - fh / 2, w, fh, TFT_BLACK);
            getLcd()->drawString(buf, x, y);
        }
    };
};

#endif