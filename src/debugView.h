#ifndef _DEBUG_VIEW_H_
#define _DEBUG_VIEW_H_
#include <M5Stack.h>

#define SCROLL_SIZE (12)
class DebugView
{
private:
  int _x;
  int _y;
  int _w;
  int _h;
  TFT_eSprite _debugView = TFT_eSprite(&M5.Lcd);
  char _buf[320 / 5];
  void _out(char *buf)
  {
    _debugView.setTextSize(1);
    _debugView.drawString(_buf, 0, _h - SCROLL_SIZE);
    _debugView.scroll(0, -SCROLL_SIZE);
    _debugView.pushSprite(_x, _y);
  }

public:
  DebugView(int x, int y, int w, int h)
  {
    _x = x;
    _y = y;
    _w = w;
    _h = h;
    _debugView.setColorDepth(8);
    _debugView.createSprite(_w, _h);
    _debugView.setTextColor(BLUE);
  }
  void output(const char *str)
  {
    snprintf(_buf, sizeof(_buf) - 1, "%s", str);
    _out(_buf);
    Serial.println(str);
  }
  void output(int v)
  {
    snprintf(_buf, sizeof(_buf) - 1, "%d", v);
    _out(_buf);
    Serial.println(v);
  }
};

#endif