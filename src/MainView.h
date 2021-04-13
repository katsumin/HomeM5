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