#include "ChessBoard.h"

namespace {
  const int SQUARE_SIZE = 30;
  const int BOARD_SIZE = 240;
  const int PANEL_X = 240;
  const uint16_t LIGHT_SQUARE = 0xD69A;
  const uint16_t DARK_SQUARE = 0x5AEB;
  const uint16_t HIGHLIGHT = 0xFFE0;
  const uint16_t MOVE_HINT = 0x07E0;
  const uint16_t PANEL_BG = 0x18E3;
}

void ChessBoard::begin(Adafruit_ILI9341 &display) {
  tft = &display;
}

int ChessBoard::squareAt(int x, int y) const {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return -1;
  int file = x / SQUARE_SIZE;
  int rank = 7 - y / SQUARE_SIZE;
  return rank * 8 + file;
}

bool ChessBoard::isNewGameButton(int x, int y) const {
  return x >= PANEL_X + 5 && x < PANEL_X + 75 && y >= 174 && y < 205;
}

bool ChessBoard::isDepthButton(int x, int y) const {
  return x >= PANEL_X + 5 && x < PANEL_X + 75 && y >= 210 && y < 238;
}

void ChessBoard::draw(const ChessPosition &position, int selectedSquare,
                      const ChessMove *legalMoves, uint8_t legalMoveCount,
                      const ChessMove &lastMove, uint8_t aiDepth,
                      bool gameOver, bool thinking) {
  drawBoard(position, selectedSquare, legalMoves, legalMoveCount, lastMove);
  drawPanel(position, aiDepth, gameOver, thinking);
}

void ChessBoard::drawBoard(const ChessPosition &position, int selectedSquare,
                           const ChessMove *legalMoves, uint8_t legalMoveCount,
                           const ChessMove &lastMove) {
  // Draw square backgrounds
  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;
      int x = file * SQUARE_SIZE;
      int y = (7 - rank) * SQUARE_SIZE;
      bool isLight = ((file + rank) & 1) != 0;
      uint16_t color = isLight ? LIGHT_SQUARE : DARK_SQUARE;
      if (square == lastMove.from || square == lastMove.to) color = 0xB5F3;
      if (square == selectedSquare) color = HIGHLIGHT;
      tft->fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, color);
    }
  }

  // Draw move hints
  for (uint8_t i = 0; i < legalMoveCount; ++i) {
    if (selectedSquare < 0 || legalMoves[i].from != selectedSquare) continue;
    int x = (legalMoves[i].to & 7) * SQUARE_SIZE + SQUARE_SIZE / 2;
    int y = (7 - (legalMoves[i].to >> 3)) * SQUARE_SIZE + SQUARE_SIZE / 2;
    if (position.board[legalMoves[i].to])
      tft->drawCircle(x, y, 13, MOVE_HINT);
    else
      tft->fillCircle(x, y, 4, MOVE_HINT);
  }

  // Draw all pieces using graphics module
  ChessGraphics::drawAllPieces(*tft, position, SQUARE_SIZE);
}

void ChessBoard::drawPanel(const ChessPosition &position, uint8_t aiDepth,
                           bool gameOver, bool thinking) {
  tft->fillRect(PANEL_X, 0, 80, 240, PANEL_BG);
  tft->setTextWrap(false);
  tft->setTextSize(1);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(PANEL_X + 6, 8);
  tft->print("CYD CHESS");
  tft->drawFastHLine(PANEL_X + 5, 23, 70, 0x7BEF);

  tft->setCursor(PANEL_X + 6, 34);
  if (gameOver) {
    ChessMove moves[ChessEngine::MAX_MOVES];
    uint8_t count = ChessEngine::getLegalMoves(position, moves, ChessEngine::MAX_MOVES);
    if (count == 0) {
      bool checked = ChessEngine::isInCheck(position, position.whiteToMove);
      tft->setTextColor(checked ? ILI9341_RED : ILI9341_YELLOW);
      tft->print(checked ? "CHECK" : "DRAW");
      tft->setCursor(PANEL_X + 6, 48);
      tft->print(checked ? "MATE" : "STALE");
    } else {
      tft->setTextColor(ILI9341_YELLOW);
      tft->print("DRAW");
      tft->setCursor(PANEL_X + 6, 48);
      tft->print("50 MOVE");
    }
  } else if (thinking) {
    tft->setTextColor(ILI9341_YELLOW);
    tft->print("AI THINK");
  } else {
    tft->setTextColor(ILI9341_CYAN);
    tft->print(position.whiteToMove ? "YOUR TURN" : "AI TURN");
    if (ChessEngine::isInCheck(position, position.whiteToMove)) {
      tft->setCursor(PANEL_X + 6, 49);
      tft->setTextColor(ILI9341_RED);
      tft->print("CHECK!");
    }
  }

  tft->setCursor(PANEL_X + 6, 76);
  tft->setTextColor(ILI9341_WHITE);
  tft->print("WHITE");
  tft->setCursor(PANEL_X + 6, 89);
  tft->print("YOU");
  tft->setCursor(PANEL_X + 6, 111);
  tft->print("BLACK");
  tft->setCursor(PANEL_X + 6, 124);
  tft->print("AI");

  tft->drawRoundRect(PANEL_X + 5, 174, 70, 31, 4, ILI9341_CYAN);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(PANEL_X + 11, 186);
  tft->print("NEW GAME");
  tft->drawRoundRect(PANEL_X + 5, 210, 70, 28, 4, ILI9341_YELLOW);
  tft->setCursor(PANEL_X + 11, 220);
  tft->print("AI D");
  tft->print(aiDepth);
  tft->print(">");
}
