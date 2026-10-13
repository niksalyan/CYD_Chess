#ifndef SCREEN_H
#define SCREEN_H

class Screen {
public:
  virtual ~Screen() {}
  virtual void draw() = 0;
  virtual bool handleTouch(int x, int y) = 0;
  virtual bool update() { return false; }
};

#endif
