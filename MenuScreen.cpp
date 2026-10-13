#include "MenuScreen.h"

#include "Navigation.h"

namespace {
  const uint16_t MENU_BACKGROUND = 0x18E3;
  const int BUTTON_X = 40;
  const int BUTTON_W = 160;
  const int BUTTON_H = 44;
  const int RESET_BUTTON_Y = 105;
  const int BACK_BUTTON_Y = 165;
}

MenuScreen::MenuScreen()
  : display(0), game(0) {
}

MenuScreen &MenuScreen::instance() {
  static MenuScreen menu;
  return menu;
}

void MenuScreen::begin(Adafruit_ILI9341 &tft, GameScreen &gameScreen) {
  display = &tft;
  game = &gameScreen;
}

void MenuScreen::draw() {
  if (!display) return;

  display->fillScreen(MENU_BACKGROUND);
  display->setTextWrap(false);
  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(84, 45);
  display->print("MENU");

  display->fillRoundRect(BUTTON_X, RESET_BUTTON_Y, BUTTON_W, BUTTON_H,
                         6, 0x7800);
  display->drawRoundRect(BUTTON_X, RESET_BUTTON_Y, BUTTON_W, BUTTON_H,
                         6, ILI9341_RED);
  display->setTextSize(1);
  display->setCursor(103, RESET_BUTTON_Y + 18);
  display->print("RESET GAME");

  display->fillRoundRect(BUTTON_X, BACK_BUTTON_Y, BUTTON_W, BUTTON_H,
                         6, 0x4208);
  display->drawRoundRect(BUTTON_X, BACK_BUTTON_Y, BUTTON_W, BUTTON_H,
                         6, ILI9341_CYAN);
  display->setCursor(108, BACK_BUTTON_Y + 18);
  display->print("BACK");
}

bool MenuScreen::handleTouch(int x, int y) {
  if (x >= BUTTON_X && x < BUTTON_X + BUTTON_W &&
      y >= RESET_BUTTON_Y && y < RESET_BUTTON_Y + BUTTON_H) {
    if (!game || !Navigation::goBack()) return false;
    game->requestNewGame();
    return true;
  }

  if (x >= BUTTON_X && x < BUTTON_X + BUTTON_W &&
      y >= BACK_BUTTON_Y && y < BACK_BUTTON_Y + BUTTON_H)
    return Navigation::goBack();

  return false;
}
