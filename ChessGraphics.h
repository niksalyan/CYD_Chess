#ifndef CHESS_GRAPHICS_H
#define CHESS_GRAPHICS_H

#include <Adafruit_ILI9341.h>
#include "ChessEngine.h"

namespace ChessGraphics {
  // Draw a single chess piece at the given position
  void drawPiece(Adafruit_ILI9341 &tft, char piece, int x, int y, int size = 12);
  
  // Draw all pieces on the board
  void drawAllPieces(Adafruit_ILI9341 &tft, const ChessPosition &position, int squareSize);
}

#endif
