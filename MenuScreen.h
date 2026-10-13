#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include <Adafruit_ILI9341.h>

#include "GameScreen.h"

class MenuScreen : public Screen {
public:
  static MenuScreen &instance();
  void begin(Adafruit_ILI9341 &display, GameScreen &game);
  void draw();
  bool handleTouch(int x, int y);

private:
  MenuScreen();

  Adafruit_ILI9341 *display;
  GameScreen *game;
};

#endif
