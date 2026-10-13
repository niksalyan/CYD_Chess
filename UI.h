#ifndef UI_H
#define UI_H

#include "Screen.h"

class Navigation;

class UI {
public:
  static Screen *currentScreen();
  static void draw();
  static bool handleTouch(int x, int y);
  static bool update();

private:
  friend class Navigation;
  static void setCurrentScreen(Screen *screen);
  static Screen *activeScreen;
};

#endif
