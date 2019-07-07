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
  std::map<String, String> _keys;

public:
  ViewController() { setCurrentKey(""); };
  ~ViewController(){};
  inline const char *getNextKey() { return _keys[_curKey].c_str(); };
  inline void setCurrentKey(const char *key) { _curKey = String(key); };
  inline const char *getCurrentKey() { return _curKey.c_str(); };
  inline void setView(const char *key, View *view, const char *nextKey)
  {
    String newKey = String(key);
    _views[newKey] = view;
    _keys[newKey] = String(nextKey);
    _curKey = newKey;
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
      _curKey = nextKey;
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