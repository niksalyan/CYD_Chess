#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include <Adafruit_ST7789.h>

#include "GameScreen.h"

class MenuScreen : public Screen {
public:
  static MenuScreen &instance();
  void begin(Adafruit_ST7789 &display, GameScreen &game);
  void draw();
  bool handleTouch(const TouchPoint &point);

private:
  MenuScreen();

  Adafruit_ST7789 *display;
  GameScreen *game;
};

#endif
