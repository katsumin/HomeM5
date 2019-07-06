#ifndef _VIEW_H_
#define _VIEW_H_
#include <M5Stack.h>
#include <map>

class View
{
private:
  boolean _enable;

public:
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

public:
  ViewController() { setCurrentKey(""); };
  ~ViewController(){};
  inline void setCurrentKey(const char *key) { _curKey = String(key); };
  inline const char *getCurrentKey() { return _curKey.c_str(); };
  inline void setView(const char *key, View *view) { _views[String(key)] = view; };
  void change(const char *key)
  {
    String oldKey = getCurrentKey();
    if (_views.count(oldKey) > 0)
    {
      View *pre = _views[key];
      pre->leave();
    }
    View *cur = _views[String(key)];
    cur->enter();
    setCurrentKey(key);
  };
};

#endif