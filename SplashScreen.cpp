#include "SplashScreen.h"

#include "ChessGraphics.h"
#include "Navigation.h"

namespace {
  const uint16_t BACKGROUND = 0x0861;
  const uint16_t PANEL = 0x10A2;
  const uint16_t BOARD_LIGHT = 0xBDF7;
  const uint16_t BOARD_DARK = 0x39C7;

  void drawMiniBoard(Adafruit_ILI9341 &display, int centerX, int top) {
    const int squareSize = 32;
    const int boardSize = squareSize * 4;
    int left = centerX - boardSize / 2;

    display.fillRoundRect(left - 5, top - 5, boardSize + 10, boardSize + 10,
                          5, 0x0000);

    const char pieces[4][4] = {
      {'r', 'n', 'b', 'q'},
      {'p', 'p', 'p', 'p'},
      {'P', 'P', 'P', 'P'},
      {'R', 'N', 'B', 'Q'}
    };

    for (int row = 0; row < 4; ++row) {
      for (int column = 0; column < 4; ++column) {
        int x = left + column * squareSize;
        int y = top + row * squareSize;
        uint16_t squareColor = ((row + column) & 1)
                                 ? BOARD_LIGHT : BOARD_DARK;
        display.fillRect(x, y, squareSize, squareSize, squareColor);
        ChessGraphics::drawPiece(display, pieces[row][column],
                                 x + squareSize / 2,
                                 y + squareSize / 2,
                                 squareSize / 2);
      }
    }

    display.drawRoundRect(left - 5, top - 5, boardSize + 10, boardSize + 10,
                          5, ILI9341_CYAN);
  }
}

SplashScreen::SplashScreen()
  : display(0), game(0) {
}

SplashScreen &SplashScreen::instance() {
  static SplashScreen screen;
  return screen;
}

void SplashScreen::begin(Adafruit_ILI9341 &tft, GameScreen &gameScreen) {
  display = &tft;
  game = &gameScreen;
}

void SplashScreen::draw() {
  if (!display) return;

  const int centerX = display->width() / 2;
  display->fillScreen(BACKGROUND);
  display->drawRoundRect(8, 8, display->width() - 16,
                         display->height() - 16, 12, 0x2945);
  display->fillRoundRect(24, 22, display->width() - 48, 66, 9, PANEL);

  display->setTextWrap(false);
  display->setTextSize(1);
  display->setTextColor(ILI9341_CYAN);
  display->setCursor(centerX - 37, 34);
  display->print("ESP32  |  CYD");

  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(centerX - 54, 52);
  display->print("CYD CHESS");

  drawMiniBoard(*display, centerX, 112);

  display->setTextSize(1);
  display->setTextColor(0xBDF7);
  display->setCursor(centerX - 42, 231);
  display->print("PLAYER  VS  AI");

  display->drawRoundRect(44, 262, display->width() - 88, 34, 7,
                         ILI9341_CYAN);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(centerX - 39, 275);
  display->print("TAP TO START");

  display->fillCircle(centerX - 5, 308, 2, ILI9341_CYAN);
  display->fillCircle(centerX + 5, 308, 2, 0x7BEF);
}

void SplashScreen::continueToGame() {
  if (game) Navigation::setRoot(*game);
}

bool SplashScreen::handleTouch(const TouchPoint &) {
  continueToGame();
  return game != 0;
}
