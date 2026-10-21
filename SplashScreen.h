#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <Adafruit_ST7789.h>

#include "GameScreen.h"
#include "Screen.h"

class SplashScreen : public Screen {
public:
  static SplashScreen &instance();
  void begin(Adafruit_ST7789 &display, GameScreen &game);
  void draw();
  bool handleTouch(const TouchPoint &point);

private:
  SplashScreen();
  void continueToGame();

  Adafruit_ST7789 *display;
  GameScreen *game;
};

#endif
