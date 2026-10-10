#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <Adafruit_ILI9341.h>
#include "ChessEngine.h"

class ChessBoard {
public:
  void begin(Adafruit_ILI9341 &display);
  void draw(const ChessPosition &position, int selectedSquare,
            const ChessMove *legalMoves, uint8_t legalMoveCount,
            const ChessMove &lastMove, uint8_t aiDepth,
            bool gameOver, bool thinking);
  int squareAt(int x, int y) const;
  bool isNewGameButton(int x, int y) const;
  bool isDepthButton(int x, int y) const;

private:
  Adafruit_ILI9341 *tft;
  void drawBoard(const ChessPosition &position, int selectedSquare,
                 const ChessMove *legalMoves, uint8_t legalMoveCount,
                 const ChessMove &lastMove);
  void drawPanel(const ChessPosition &position, uint8_t aiDepth,
                 bool gameOver, bool thinking);
};

#endif
