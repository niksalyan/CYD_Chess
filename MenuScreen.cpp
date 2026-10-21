#include "MenuScreen.h"

#include "MoveHistoryScreen.h"
#include "Navigation.h"
#include "TouchCalibrationScreen.h"

namespace {
  const uint16_t MENU_BACKGROUND = 0x18E3;
  const uint16_t CARD_BACKGROUND = 0x2124;
  const int CARD_X = 20;
  const int CARD_W = 200;
  const int CARD_H = 48;
  const int RESET_BUTTON_Y = 84;
  const int HISTORY_BUTTON_Y = 141;
  const int CALIBRATION_BUTTON_Y = 198;
  const int BACK_BUTTON_Y = 263;

  void drawMenuCard(Adafruit_ST7789 &display, int y, uint16_t accent,
                    const char *title, const char *subtitle) {
    display.fillRoundRect(CARD_X, y, CARD_W, CARD_H, 6, CARD_BACKGROUND);
    display.drawRoundRect(CARD_X, y, CARD_W, CARD_H, 6, accent);
    display.fillRoundRect(CARD_X + 7, y + 10, 4, 28, 2, accent);
    display.setTextSize(1);
    display.setTextColor(ST77XX_WHITE);
    display.setCursor(CARD_X + 20, y + 10);
    display.print(title);
    display.setTextColor(0xA514);
    display.setCursor(CARD_X + 20, y + 27);
    display.print(subtitle);
  }
}

MenuScreen::MenuScreen()
  : display(0), game(0) {
}

MenuScreen &MenuScreen::instance() {
  static MenuScreen menu;
  return menu;
}

void MenuScreen::begin(Adafruit_ST7789 &tft, GameScreen &gameScreen) {
  display = &tft;
  game = &gameScreen;
  MoveHistoryScreen::instance().begin(tft, gameScreen);
}

void MenuScreen::draw() {
  if (!display) return;

  display->fillScreen(MENU_BACKGROUND);
  display->setTextWrap(false);
  display->fillRect(0, 0, display->width(), 68, 0x1082);
  display->setTextSize(1);
  display->setTextColor(ST77XX_CYAN);
  display->setCursor(20, 15);
  display->print("CYD CHESS");
  display->setTextSize(2);
  display->setTextColor(ST77XX_WHITE);
  display->setCursor(20, 34);
  display->print("MENU");

  drawMenuCard(*display, RESET_BUTTON_Y, ST77XX_RED,
               "NEW GAME", "Start a fresh match");
  drawMenuCard(*display, HISTORY_BUTTON_Y, ST77XX_YELLOW,
               "MOVE HISTORY", "Review moves this game");
  drawMenuCard(*display, CALIBRATION_BUTTON_Y, ST77XX_CYAN,
               "TOUCH CALIBRATION", "Adjust screen touch");

  display->fillRoundRect(CARD_X, BACK_BUTTON_Y, CARD_W, 40, 6, CARD_BACKGROUND);
  display->drawRoundRect(CARD_X, BACK_BUTTON_Y, CARD_W, 40, 6, 0x7BEF);
  display->setTextColor(ST77XX_WHITE);
  display->setCursor(108, BACK_BUTTON_Y + 16);
  display->print("BACK");
}

bool MenuScreen::handleTouch(const TouchPoint &point) {
  int x = point.x;
  int y = point.y;
  if (x >= CARD_X && x < CARD_X + CARD_W &&
      y >= RESET_BUTTON_Y && y < RESET_BUTTON_Y + CARD_H) {
    if (!game || !Navigation::goBack()) return false;
    game->requestNewGame();
    return true;
  }

  if (x >= CARD_X && x < CARD_X + CARD_W &&
      y >= HISTORY_BUTTON_Y && y < HISTORY_BUTTON_Y + CARD_H) {
    MoveHistoryScreen::instance().showLatest();
    return Navigation::navigateTo(MoveHistoryScreen::instance());
  }

  if (x >= CARD_X && x < CARD_X + CARD_W &&
      y >= CALIBRATION_BUTTON_Y &&
      y < CALIBRATION_BUTTON_Y + CARD_H) {
    TouchCalibrationScreen::instance().start();
    return Navigation::navigateTo(TouchCalibrationScreen::instance());
  }

  if (x >= CARD_X && x < CARD_X + CARD_W &&
      y >= BACK_BUTTON_Y && y < BACK_BUTTON_Y + 40)
    return Navigation::goBack();

  return false;
}
