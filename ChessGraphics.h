#ifndef CHESS_GRAPHICS_H
#define CHESS_GRAPHICS_H

#include <Adafruit_ILI9341.h>
#include "ChessEngine.h"
#include "ChessPieceAtlas.h"

namespace ChessGraphics {
  // Draw a single chess piece using sprite atlas or vector fallback
  // piece: character representing the piece (K, Q, R, B, N, P, k, q, r, b, n, p)
  // x, y: pixel coordinates (center of the piece)
  // size: radius in pixels
  // tft: display object to draw on
  void drawPiece(Adafruit_ILI9341 &tft, char piece, int x, int y, int size);
}

#endif
