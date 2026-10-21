#ifndef MOVE_HISTORY_SCREEN_H
#define MOVE_HISTORY_SCREEN_H

#include <Adafruit_ST7789.h>

#include "GameScreen.h"
#include "Screen.h"

class MoveHistoryScreen : public Screen {
public:
  static MoveHistoryScreen &instance();
  void begin(Adafruit_ST7789 &display, GameScreen &game);
  void showLatest();
  void draw();
  bool handleTouch(const TouchPoint &point);

private:
  MoveHistoryScreen();
  void drawMove(uint16_t moveNumber, uint16_t firstPly);

  Adafruit_ST7789 *display;
  GameScreen *game;
  uint16_t firstMoveNumber;
  bool confirmUndo;
};

#endif
