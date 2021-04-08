#ifndef _VIEW_H_
#define _VIEW_H_
// #include "Free_Fonts.h"
// #include <M5Stack.h>
#include <map>
#define LGFX_M5STACK
#include <LGFX_TFT_eSPI.hpp>
#include "FunctionButton.h"

class View
{
private:
    boolean _enable;
    TFT_eSPI *_lcd;

public:
    View(TFT_eSPI *lcd)
    {
        _lcd = lcd;
    }
    inline TFT_eSPI *getLcd() { return _lcd; }
    inline boolean isEnable() { return _enable; };
    inline void setEnable(boolean enable) { _enable = enable; };
    virtual void init();
    virtual void update();
    void enter()
    {
        setEnable(true);
        init();
        update();
    };
    void leave()
    {
        setEnable(false);
    };
};

class ViewController
{
private:
    String _curKey;
    std::map<String, View *> _views;
    std::map<String, String> _keys;
    FunctionButton *_button;
    TFT_eSPI *_lcd;

public:
    ViewController(FunctionButton *btn, TFT_eSPI *lcd)
    {
        _curKey = "";
        _button = btn;
        _lcd = lcd;
    };
    ~ViewController(){};
    inline TFT_eSPI *getLcd() { return _lcd; }
    inline const char *getNextKey() { return _keys[_curKey].c_str(); };
    inline void setCurrentKey(const char *key) { setCurrentKey(String(key)); };
    void setCurrentKey(const String key)
    {
        _curKey = key;
        _button->enable(getNextKey());
    };
    inline const char *getCurrentKey() { return _curKey.c_str(); };
    inline void setView(const char *key, View *view)
    {
        String newKey = String(key);
        _views[newKey] = view;
        if (_keys.count(_curKey) == 0)
        {
            _keys[newKey] = newKey;
        }
        else
        {
            String oldKey = _keys[_curKey];
            _keys[_curKey] = newKey;
            _keys[newKey] = oldKey;
        }
        setCurrentKey(newKey);
    };
    void changeNext()
    {
        if (_views.count(_curKey) > 0)
        {
            View *pre = _views[_curKey];
            pre->leave();
        }
        String nextKey = _keys[_curKey];
        if (_views.count(nextKey) > 0)
        {
            View *cur = _views[nextKey];
            cur->enter();
            setCurrentKey(nextKey);
        }
    }
    void update()
    {
        String oldKey = getCurrentKey();
        if (_views.count(oldKey) > 0)
        {
            View *pre = _views[oldKey];
            pre->update();
        }
    }
};

#endif