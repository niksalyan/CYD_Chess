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
  : display(0), game(0), firstMoveNumber(0), confirmUndo(false) {
}

MoveHistoryScreen &MoveHistoryScreen::instance() {
  static MoveHistoryScreen screen;
  return screen;
}

void MoveHistoryScreen::begin(Adafruit_ST7789 &tft, GameScreen &gameScreen) {
  display = &tft;
  game = &gameScreen;
}

void MoveHistoryScreen::showLatest() {
  confirmUndo = false;
  uint16_t totalMoveNumbers = (game->moveHistoryCount() + 1) / 2;
  firstMoveNumber = totalMoveNumbers > ROWS_PER_PAGE
                      ? totalMoveNumbers - ROWS_PER_PAGE : 0;
}

void MoveHistoryScreen::drawMove(uint16_t moveNumber, uint16_t firstPly) {
  display->fillRoundRect(ROW_X, ROW_Y + moveNumber * ROW_HEIGHT,
                         ROW_WIDTH, ROW_HEIGHT - 2, 3, ROW_BACKGROUND);
  display->setTextSize(1);
  display->setTextColor(ST77XX_YELLOW);
  display->setCursor(ROW_X + 7, ROW_Y + moveNumber * ROW_HEIGHT + 7);
  display->print(firstMoveNumber + moveNumber + 1);
  display->print('.');

  RecordedMove recorded;
  char moveText[8];
  if (game->getRecordedMove(firstPly, recorded)) {
    formatMove(recorded.move, moveText);
    display->setTextColor(recorded.white ? ST77XX_WHITE : ST77XX_CYAN);
    display->setCursor(ROW_X + 31, ROW_Y + moveNumber * ROW_HEIGHT + 7);
    display->print(moveText);
  }

  if (game->getRecordedMove(firstPly + 1, recorded)) {
    formatMove(recorded.move, moveText);
    display->setTextColor(recorded.white ? ST77XX_WHITE : ST77XX_CYAN);
    display->setCursor(ROW_X + 112, ROW_Y + moveNumber * ROW_HEIGHT + 7);
    display->print(moveText);
  }
}

void MoveHistoryScreen::draw() {
  if (!display || !game) return;

  display->fillScreen(BACKGROUND);
  display->setTextWrap(false);
  display->setTextSize(2);
  display->setTextColor(ST77XX_WHITE);
  display->setCursor(35, 16);
  display->print("MOVE HISTORY");

  uint16_t totalMoves = game->moveHistoryCount();
  uint16_t totalMoveNumbers = (totalMoves + 1) / 2;
  uint16_t lastPageStart = totalMoveNumbers > ROWS_PER_PAGE
                             ? totalMoveNumbers - ROWS_PER_PAGE : 0;
  if (firstMoveNumber > lastPageStart) firstMoveNumber = lastPageStart;

  display->setTextSize(1);
  display->setTextColor(game->moveHistoryTruncated() ? ST77XX_RED : 0xA514);
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

  display->drawRoundRect(10, BUTTON_Y, 64, BUTTON_HEIGHT, 5, ST77XX_CYAN);
  display->setCursor(30, BUTTON_Y + 15);
  display->setTextColor(ST77XX_WHITE);
  display->print("<");

  display->drawRoundRect(78, BUTTON_Y, 60, BUTTON_HEIGHT, 5, ST77XX_YELLOW);
  display->setCursor(95, BUTTON_Y + 15);
  display->print("BACK");

  display->drawRoundRect(142, BUTTON_Y, 64, BUTTON_HEIGHT, 5,
                         game->canUndoLastMove() ? ST77XX_RED : 0x7BEF);
  display->setTextColor(game->canUndoLastMove() ? ST77XX_WHITE : 0x7BEF);
  display->setCursor(155, BUTTON_Y + 15);
  display->print("UNDO");

  display->drawRoundRect(210, BUTTON_Y, 20, BUTTON_HEIGHT, 5, ST77XX_CYAN);
  display->setCursor(216, BUTTON_Y + 15);
  display->print(">");

  if (confirmUndo) {
    display->fillRoundRect(24, 103, 192, 94, 8, 0x0841);
    display->drawRoundRect(24, 103, 192, 94, 8, ST77XX_YELLOW);
    display->setTextSize(1);
    display->setTextColor(ST77XX_WHITE);
    display->setCursor(48, 120);
    display->print("Undo the last move?");
    display->drawRoundRect(43, 151, 68, 32, 5, ST77XX_RED);
    display->setCursor(65, 163);
    display->print("UNDO");
    display->drawRoundRect(129, 151, 68, 32, 5, ST77XX_CYAN);
    display->setCursor(149, 163);
    display->print("CANCEL");
  }
}

bool MoveHistoryScreen::handleTouch(const TouchPoint &point) {
  if (confirmUndo) {
    if (point.y >= 151 && point.y < 183) {
      if (point.x >= 43 && point.x < 111) {
        bool undone = game->undoLastMove();
        confirmUndo = false;
        if (undone) return Navigation::returnTo(*game);
      } else if (point.x >= 129 && point.x < 197) {
        confirmUndo = false;
      }
      return true;
    }
    return true;
  }

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
    if (point.x >= 78 && point.x < 138)
      return Navigation::goBack();
    if (point.x >= 142 && point.x < 206) {
      if (!game->canUndoLastMove()) return false;
      confirmUndo = true;
      return true;
    }
    if (point.x >= 210 && point.x < 230) {
      firstMoveNumber += ROWS_PER_PAGE;
      if (firstMoveNumber > lastPageStart) firstMoveNumber = lastPageStart;
      return true;
    }
  }
  return false;
}
