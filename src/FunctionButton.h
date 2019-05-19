#ifndef _FUNC_BTN_H_
#define _FUNC_BTN_H_
#include <M5Stack.h>

#define TEXT_HEIGHT (15)
#define WIDTH (60)
#define POS_A_X (36)
#define POS_B_X (130)
#define POS_C_X (224)

class FunctionButton
{
private:
  Button *_button;
  void _set(const char *label, uint32_t color);

public:
  FunctionButton(Button *button);
  inline Button *getButton()
  {
    return _button;
  };
  inline void enable(const char *label) { _set(label, WHITE); };
  inline void disable(const char *label) { _set(label, DARKGREY); };
};

#endif
