#include "ChessGraphics.h"

namespace ChessGraphics {
  bool isWhite(char piece) { return piece >= 'A' && piece <= 'Z'; }

  char lowerPiece(char piece) {
    return isWhite(piece) ? piece + ('a' - 'A') : piece;
  }

  void drawPiece(Adafruit_ILI9341 &tft, char piece, int x, int y, int size) {
    if (!piece) return;

    bool white = isWhite(piece);
    uint16_t fillColor = white ? ILI9341_WHITE : 0x1082;       // Light or dark fill
    uint16_t outlineColor = white ? 0x7BEF : ILI9341_BLACK;     // Dark or light outline
    uint16_t textColor = white ? 0x0841 : ILI9341_WHITE;        // Text color (contrasting)

    char kind = lowerPiece(piece);

    // Draw base circle for the piece
    tft.fillCircle(x, y, size, fillColor);
    tft.drawCircle(x, y, size, outlineColor);

    // Draw piece symbol using text
    // Adjust text size and position based on piece size
    int textSize = (size > 10) ? 2 : 1;
    tft.setTextSize(textSize);
    tft.setTextColor(textColor);

    // Map piece characters to single-character symbols
    const char *symbolMap = "KQRBNP";  // King, Queen, Rook, Bishop, Knight, Pawn
    int idx = -1;
    switch (kind) {
      case 'k': idx = 0; break;  // King
      case 'q': idx = 1; break;  // Queen
      case 'r': idx = 2; break;  // Rook
      case 'b': idx = 3; break;  // Bishop
      case 'n': idx = 4; break;  // Knight
      case 'p': idx = 5; break;  // Pawn
    }

    if (idx >= 0) {
      // Center the text on the circle
      int offsetX = (textSize == 2) ? 6 : 3;
      int offsetY = (textSize == 2) ? 8 : 4;
      tft.setCursor(x - offsetX, y - offsetY);
      tft.write((uint8_t)symbolMap[idx]);
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
