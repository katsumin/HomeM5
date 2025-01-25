#ifndef _FUNC_BTN_H_
#define _FUNC_BTN_H_
#include <utility/Button_Class.hpp>
#include <M5Unified.h>

#define TEXT_HEIGHT (15)
#define WIDTH (60)
#define POS_A_X (36)
#define POS_B_X (129)
#define POS_C_X (224)

class FunctionButton
{
private:
    M5GFX *_lcd;
    m5::Button_Class *_button;
    char *_label;
    void _set(const char *label, int color);
    boolean _enable = false;
    uint16_t _xpos;

public:
    FunctionButton(m5::Button_Class *button, M5GFX *lcd, uint16_t xpos);
    inline M5GFX *getLcd() { return _lcd; }
    inline char *getLabel() { return _label; };
    inline m5::Button_Class *getButton()
    {
        return _button;
    };
    inline void enable(const char *label)
    {
        _set(label, TFT_WHITE);
        _enable = true;
    };
    inline void disable(const char *label)
    {
        _set(label, TFT_MAGENTA);
        _enable = false;
    };
    inline boolean isEnable() { return _enable; };
};

#endif
