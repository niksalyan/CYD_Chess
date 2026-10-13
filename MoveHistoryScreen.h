#ifndef MOVE_HISTORY_SCREEN_H
#define MOVE_HISTORY_SCREEN_H

#include <Adafruit_ILI9341.h>

#include "GameScreen.h"
#include "Screen.h"

class MoveHistoryScreen : public Screen {
public:
  static MoveHistoryScreen &instance();
  void begin(Adafruit_ILI9341 &display, GameScreen &game);
  void showLatest();
  void draw();
  bool handleTouch(const TouchPoint &point);

private:
  MoveHistoryScreen();
  void drawMove(uint16_t moveNumber, uint16_t firstPly);

  Adafruit_ILI9341 *display;
  GameScreen *game;
  uint16_t firstMoveNumber;
  bool confirmUndo;
};

#endif
