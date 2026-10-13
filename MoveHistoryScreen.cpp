#include "MoveHistoryScreen.h"

#include "Navigation.h"

namespace {
  const uint16_t BACKGROUND = 0x18E3;
  const uint16_t ROW_BACKGROUND = 0x2124;
  const int ROW_X = 12;
  const int ROW_Y = 65;
  const int ROW_WIDTH = 216;
  const int ROW_HEIGHT = 22;
  const uint16_t ROWS_PER_PAGE = 8;
  const int BUTTON_Y = 258;
  const int BUTTON_HEIGHT = 38;

  void squareName(uint8_t square, char *name) {
    name[0] = 'a' + (square & 7);
    name[1] = '1' + (square >> 3);
    name[2] = '\0';
  }

  void formatMove(const ChessMove &move, char *text) {
    squareName(move.from, text);
    text[2] = '-';
    squareName(move.to, text + 3);
    text[5] = '\0';
    if (move.promotion) {
      text[5] = '=';
      text[6] = move.promotion >= 'a' && move.promotion <= 'z'
                  ? move.promotion - ('a' - 'A') : move.promotion;
      text[7] = '\0';
    }
  }
}

MoveHistoryScreen::MoveHistoryScreen()
  : display(0), game(0), firstMoveNumber(0) {
}

MoveHistoryScreen &MoveHistoryScreen::instance() {
  static MoveHistoryScreen screen;
  return screen;
}

void MoveHistoryScreen::begin(Adafruit_ILI9341 &tft, GameScreen &gameScreen) {
  display = &tft;
  game = &gameScreen;
}

void MoveHistoryScreen::showLatest() {
  uint16_t totalMoveNumbers = (game->moveHistoryCount() + 1) / 2;
  firstMoveNumber = totalMoveNumbers > ROWS_PER_PAGE
                      ? totalMoveNumbers - ROWS_PER_PAGE : 0;
}

void MoveHistoryScreen::drawMove(uint16_t moveNumber, uint16_t firstPly) {
  display->fillRoundRect(ROW_X, ROW_Y + moveNumber * ROW_HEIGHT,
                         ROW_WIDTH, ROW_HEIGHT - 2, 3, ROW_BACKGROUND);
  display->setTextSize(1);
  display->setTextColor(ILI9341_YELLOW);
  display->setCursor(ROW_X + 7, ROW_Y + moveNumber * ROW_HEIGHT + 7);
  display->print(firstMoveNumber + moveNumber + 1);
  display->print('.');

  RecordedMove recorded;
  char moveText[8];
  if (game->getRecordedMove(firstPly, recorded)) {
    formatMove(recorded.move, moveText);
    display->setTextColor(recorded.white ? ILI9341_WHITE : ILI9341_CYAN);
    display->setCursor(ROW_X + 31, ROW_Y + moveNumber * ROW_HEIGHT + 7);
    display->print(moveText);
  }

  if (game->getRecordedMove(firstPly + 1, recorded)) {
    formatMove(recorded.move, moveText);
    display->setTextColor(recorded.white ? ILI9341_WHITE : ILI9341_CYAN);
    display->setCursor(ROW_X + 112, ROW_Y + moveNumber * ROW_HEIGHT + 7);
    display->print(moveText);
  }
}

void MoveHistoryScreen::draw() {
  if (!display || !game) return;

  display->fillScreen(BACKGROUND);
  display->setTextWrap(false);
  display->setTextSize(2);
  display->setTextColor(ILI9341_WHITE);
  display->setCursor(35, 16);
  display->print("MOVE HISTORY");

  uint16_t totalMoves = game->moveHistoryCount();
  uint16_t totalMoveNumbers = (totalMoves + 1) / 2;
  uint16_t lastPageStart = totalMoveNumbers > ROWS_PER_PAGE
                             ? totalMoveNumbers - ROWS_PER_PAGE : 0;
  if (firstMoveNumber > lastPageStart) firstMoveNumber = lastPageStart;

  display->setTextSize(1);
  display->setTextColor(game->moveHistoryTruncated() ? ILI9341_RED : 0xA514);
  display->setCursor(18, 49);
  if (game->moveHistoryTruncated()) {
    display->print("History limit reached");
  } else {
    display->print(totalMoveNumbers);
    display->print(totalMoveNumbers == 1 ? " move" : " moves");
  }
  if (!game->moveHistoryTruncated() && totalMoveNumbers > ROWS_PER_PAGE) {
    display->setCursor(150, 49);
    display->print(firstMoveNumber + 1);
    display->print('-');
    uint16_t lastShown = firstMoveNumber + ROWS_PER_PAGE;
    if (lastShown > totalMoveNumbers) lastShown = totalMoveNumbers;
    display->print(lastShown);
  }

  uint16_t visibleRows = totalMoveNumbers - firstMoveNumber;
  if (visibleRows > ROWS_PER_PAGE) visibleRows = ROWS_PER_PAGE;
  if (visibleRows == 0) {
    display->setTextColor(0xA514);
    display->setCursor(72, 128);
    display->print("No moves yet");
  } else {
    for (uint16_t row = 0; row < visibleRows; ++row)
      drawMove(row, (firstMoveNumber + row) * 2);
  }

  display->drawRoundRect(10, BUTTON_Y, 64, BUTTON_HEIGHT, 5, ILI9341_CYAN);
  display->setCursor(32, BUTTON_Y + 15);
  display->setTextColor(ILI9341_WHITE);
  display->print("<");

  display->drawRoundRect(82, BUTTON_Y, 76, BUTTON_HEIGHT, 5, ILI9341_YELLOW);
  display->setCursor(105, BUTTON_Y + 15);
  display->print("BACK");

  display->drawRoundRect(166, BUTTON_Y, 64, BUTTON_HEIGHT, 5, ILI9341_CYAN);
  display->setCursor(190, BUTTON_Y + 15);
  display->print(">");
}

bool MoveHistoryScreen::handleTouch(const TouchPoint &point) {
  uint16_t totalMoveNumbers = (game->moveHistoryCount() + 1) / 2;
  uint16_t lastPageStart = totalMoveNumbers > ROWS_PER_PAGE
                             ? totalMoveNumbers - ROWS_PER_PAGE : 0;

  if (point.y >= BUTTON_Y && point.y < BUTTON_Y + BUTTON_HEIGHT) {
    if (point.x >= 10 && point.x < 74) {
      if (firstMoveNumber >= ROWS_PER_PAGE)
        firstMoveNumber -= ROWS_PER_PAGE;
      else
        firstMoveNumber = 0;
      return true;
    }
    if (point.x >= 82 && point.x < 158)
      return Navigation::goBack();
    if (point.x >= 166 && point.x < 230) {
      firstMoveNumber += ROWS_PER_PAGE;
      if (firstMoveNumber > lastPageStart) firstMoveNumber = lastPageStart;
      return true;
    }
  }
  return false;
}
