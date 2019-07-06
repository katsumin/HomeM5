#ifndef _FUNC_BTN_H_
#define _FUNC_BTN_H_
#include <M5Stack.h>

#define TEXT_HEIGHT (15)
#define WIDTH (60)
#define POS_A_X (36)
#define POS_B_X (129)
#define POS_C_X (224)

class FunctionButton
{
private:
  Button *_button;
  char *_label;
  void _set(const char *label, uint32_t color);
  boolean _enable = false;

public:
  FunctionButton(Button *button);
  inline char *getLabel() { return _label; };
  inline Button *getButton()
  {
    return _button;
  };
  inline void enable(const char *label)
  {
    _set(label, WHITE);
    _enable = true;
  };
  inline void disable(const char *label)
  {
    _set(label, PINK);
    _enable = false;
  };
  inline boolean isEnable() { return _enable; };
};

#endif
