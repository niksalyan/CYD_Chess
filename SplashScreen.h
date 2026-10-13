#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <Adafruit_ILI9341.h>

#include "GameScreen.h"
#include "Screen.h"

class SplashScreen : public Screen {
public:
  static SplashScreen &instance();
  void begin(Adafruit_ILI9341 &display, GameScreen &game);
  void draw();
  bool handleTouch(const TouchPoint &point);
  bool update();

private:
  SplashScreen();
  void continueToGame();

  Adafruit_ILI9341 *display;
  GameScreen *game;
  uint32_t startedAt;
};

#endif
