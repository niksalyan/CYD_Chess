#include "ChessGraphics.h"

namespace ChessGraphics {
  bool isWhite(char piece) { return piece >= 'A' && piece <= 'Z'; }

  char lowerPiece(char piece) {
    return isWhite(piece) ? piece + ('a' - 'A') : piece;
  }

  void drawKing(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    // Cross on top
    tft.drawFastVLine(x, y - size - 2, 3, outline);
    tft.drawFastHLine(x - 1, y - size - 1, 3, outline);
    // Crown base
    tft.fillRect(x - size + 2, y - 4, 2 * size - 4, size - 2, fill);
    tft.drawRect(x - size + 2, y - 4, 2 * size - 4, size - 2, outline);
    // Base
    tft.drawFastHLine(x - size, y + size - 2, 2 * size, outline);
  }

  void drawQueen(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    // Crown with multiple points
    for (int i = -2; i <= 2; i++) {
      tft.drawLine(x + i * 2, y - size - 1, x + i * 2 - 1, y - size + 2, outline);
      tft.drawLine(x + i * 2, y - size - 1, x + i * 2 + 1, y - size + 2, outline);
    }
    // Crown base
    tft.fillRect(x - size + 2, y - 3, 2 * size - 4, size - 1, fill);
    tft.drawRect(x - size + 2, y - 3, 2 * size - 4, size - 1, outline);
    // Base
    tft.drawFastHLine(x - size, y + size - 2, 2 * size, outline);
  }

  void drawRook(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    // Crenellations on top
    for (int i = -1; i <= 1; i++) {
      tft.drawFastVLine(x + i * size / 2, y - size - 1, 2, outline);
    }
    // Main body
    tft.fillRect(x - size + 2, y - 3, 2 * size - 4, size - 1, fill);
    tft.drawRect(x - size + 2, y - 3, 2 * size - 4, size - 1, outline);
    // Base
    tft.drawFastHLine(x - size, y + size - 2, 2 * size, outline);
  }

  void drawBishop(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    // Pointed top
    tft.drawLine(x - 2, y - size, x, y - size - 2, outline);
    tft.drawLine(x + 2, y - size, x, y - size - 2, outline);
    // Body
    tft.fillRect(x - size + 2, y - 2, 2 * size - 4, size - 2, fill);
    tft.drawRect(x - size + 2, y - 2, 2 * size - 4, size - 2, outline);
    // Slit in middle
    tft.drawFastHLine(x - size / 2, y + 1, size, outline);
    // Base
    tft.drawFastHLine(x - size, y + size - 1, 2 * size, outline);
  }

  void drawKnight(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    // Horse head shape - simplified
    tft.drawLine(x - size + 2, y - size, x - size / 2, y - size - 2, outline);
    tft.drawLine(x - size / 2, y - size - 2, x, y - size, outline);
    tft.drawLine(x, y - size, x + 1, y + 1, outline);
    // Body
    tft.fillRect(x - size + 2, y - 1, 2 * size - 4, size - 1, fill);
    tft.drawRect(x - size + 2, y - 1, 2 * size - 4, size - 1, outline);
    // Base
    tft.drawFastHLine(x - size, y + size - 1, 2 * size, outline);
  }

  void drawPawn(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    // Small head on top
    tft.fillCircle(x, y - size, size / 3, fill);
    tft.drawCircle(x, y - size, size / 3, outline);
    // Neck
    tft.drawFastVLine(x, y - size + size / 3, size / 3, outline);
    // Body
    tft.fillRect(x - size / 2, y - size / 3, size, size / 2, fill);
    tft.drawRect(x - size / 2, y - size / 3, size, size / 2, outline);
    // Base
    tft.drawFastHLine(x - size, y + size / 3, 2 * size, outline);
  }

  void drawPiece(Adafruit_ILI9341 &tft, char piece, int x, int y, int size) {
    if (!piece) return;

    bool white = isWhite(piece);
    uint16_t fillColor = white ? ILI9341_WHITE : 0x1082;        // Light or dark fill
    uint16_t outlineColor = white ? 0x7BEF : ILI9341_BLACK;      // Dark or light outline

    char kind = lowerPiece(piece);

    // Draw background circle
    tft.fillCircle(x, y, size + 1, fillColor);
    tft.drawCircle(x, y, size + 1, outlineColor);

    // Draw the specific piece
    switch (kind) {
      case 'k':
        drawKing(tft, x, y, size - 2, fillColor, outlineColor);
        break;
      case 'q':
        drawQueen(tft, x, y, size - 2, fillColor, outlineColor);
        break;
      case 'r':
        drawRook(tft, x, y, size - 2, fillColor, outlineColor);
        break;
      case 'b':
        drawBishop(tft, x, y, size - 2, fillColor, outlineColor);
        break;
      case 'n':
        drawKnight(tft, x, y, size - 2, fillColor, outlineColor);
        break;
      case 'p':
        drawPawn(tft, x, y, size - 2, fillColor, outlineColor);
        break;
    }
  }

  void drawAllPieces(Adafruit_ILI9341 &tft, const ChessPosition &position, int squareSize) {
    int pieceSize = (squareSize * 2) / 5;  // Piece radius is about 40% of square size
    int centerOffset = squareSize / 2;

    for (int square = 0; square < 64; ++square) {
      char piece = position.board[square];
      if (!piece) continue;

      int file = square & 7;
      int rank = square >> 3;
      int x = file * squareSize + centerOffset;
      int y = (7 - rank) * squareSize + centerOffset;

      drawPiece(tft, piece, x, y, pieceSize);
    }
  }
}
