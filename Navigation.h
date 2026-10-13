#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>

#include "Screen.h"

class Navigation {
public:
  static void setRoot(Screen &screen);
  static bool navigateTo(Screen &screen);
  static bool goBack();

private:
  static const uint8_t MAX_HISTORY = 8;
  static Screen *history[MAX_HISTORY];
  static uint8_t historySize;
};

#endif
