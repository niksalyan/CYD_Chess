#ifndef SCREEN_H
#define SCREEN_H

struct TouchPoint {
  int rawX;
  int rawY;
  int x;
  int y;
};

class Screen {
public:
  virtual ~Screen() {}
  virtual void draw() = 0;
  virtual bool handleTouch(const TouchPoint &point) = 0;
  virtual bool update() { return false; }
};

#endif
