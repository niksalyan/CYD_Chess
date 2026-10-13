#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H
#define LED_PIN 22

#define SCREEN_DIR_0    0  // no rotation
#define SCREEN_DIR_90   1  // 90° clockwise
#define SCREEN_DIR_180  2
#define SCREEN_DIR_270  3


#define MATRIX_SIZE 16
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#define SCREEN_DIR SCREEN_DIR_0 



#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>
#include "ChessEngine.h"
#include "ChessGraphics.h"


class ChessBoard {
public:
  void begin(Adafruit_ILI9341 &display);
  void draw(const ChessPosition &position, int selectedSquare,
            const ChessMove *legalMoves, uint8_t legalMoveCount,
            const ChessMove &lastMove, uint8_t aiDepth,
            bool gameOver, bool thinking, bool humanPlaysWhite);
  int squareAt(int x, int y, bool humanPlaysWhite) const;
  bool isMenuButton(int x, int y) const;
  bool isDepthButton(int x, int y) const;

  
  bool isPlayerFirstButton(int x, int y) const;
  bool isAIFirstButton(int x, int y) const;
  bool isCancelNewGameButton(int x, int y) const;

  
  void drawPanel(const ChessPosition &position, uint8_t aiDepth,
                 bool gameOver, bool thinking, bool humanPlaysWhite);

  void drawFirstMoveSelection();

private:
  Adafruit_ILI9341 *tft;
  void drawBoard(const ChessPosition &position, int selectedSquare,
                 const ChessMove *legalMoves, uint8_t legalMoveCount,
                 const ChessMove &lastMove, bool humanPlaysWhite);

  void updateMatrix(const ChessPosition &position, int selectedSquare,
                  const ChessMove *legalMoves, uint8_t legalMoveCount,
                  const ChessMove &lastMove, bool humanPlaysWhite);


  void drawMatrix(uint8_t x, uint8_t y, uint32_t color);
  void setPixel(int x, int y, uint32_t color);


};

#endif
