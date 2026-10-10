#include "ChessGraphics.h"

#if CHESS_SPRITE_ATLAS_AVAILABLE
#include "ChessPieceAtlas.h"
#endif

namespace ChessGraphics {
  bool isWhite(char piece) { return piece >= 'A' && piece <= 'Z'; }

  char lowerPiece(char piece) {
    return isWhite(piece) ? piece + ('a' - 'A') : piece;
  }

  int atlasIndexForPiece(char piece) {
    switch (lowerPiece(piece)) {
      case 'k': return 0;
      case 'q': return 1;
      case 'b': return 2;
      case 'r': return 3;
      case 'n': return 4;
      case 'p': return 5;
      default: return -1;
    }
  }

  void drawSpriteAtlas(Adafruit_ILI9341 &tft, int spriteIndex, int x, int y) {
#if CHESS_SPRITE_ATLAS_AVAILABLE
    const uint16_t tileSize = CHESS_PIECE_ATLAS_TILE_SIZE;
    const uint16_t transparent = 0xF81F;
    const uint16_t *sprite = chessPieceAtlas[spriteIndex];

    for (uint16_t py = 0; py < tileSize; ++py) {
      for (uint16_t px = 0; px < tileSize; ++px) {
        uint16_t color = sprite[py * tileSize + px];
        if (color == transparent) continue;
        tft.drawPixel(x + (int)px - tileSize / 2, y + (int)py - tileSize / 2, color);
      }
    }
#endif
  }

  void drawKing(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    tft.drawFastVLine(x, y - size - 2, 3, outline);
    tft.drawFastHLine(x - 1, y - size - 1, 3, outline);
    tft.fillRect(x - size + 2, y - 4, 2 * size - 4, size - 2, fill);
    tft.drawRect(x - size + 2, y - 4, 2 * size - 4, size - 2, outline);
    tft.drawFastHLine(x - size, y + size - 2, 2 * size, outline);
  }

  void drawQueen(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    for (int i = -2; i <= 2; i++) {
      tft.drawLine(x + i * 2, y - size - 1, x + i * 2 - 1, y - size + 2, outline);
      tft.drawLine(x + i * 2, y - size - 1, x + i * 2 + 1, y - size + 2, outline);
    }
    tft.fillRect(x - size + 2, y - 3, 2 * size - 4, size - 1, fill);
    tft.drawRect(x - size + 2, y - 3, 2 * size - 4, size - 1, outline);
    tft.drawFastHLine(x - size, y + size - 2, 2 * size, outline);
  }

  void drawRook(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    for (int i = -1; i <= 1; i++) {
      tft.drawFastVLine(x + i * size / 2, y - size - 1, 2, outline);
    }
    tft.fillRect(x - size + 2, y - 3, 2 * size - 4, size - 1, fill);
    tft.drawRect(x - size + 2, y - 3, 2 * size - 4, size - 1, outline);
    tft.drawFastHLine(x - size, y + size - 2, 2 * size, outline);
  }

  void drawBishop(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    tft.drawLine(x - 2, y - size, x, y - size - 2, outline);
    tft.drawLine(x + 2, y - size, x, y - size - 2, outline);
    tft.fillRect(x - size + 2, y - 2, 2 * size - 4, size - 2, fill);
    tft.drawRect(x - size + 2, y - 2, 2 * size - 4, size - 2, outline);
    tft.drawFastHLine(x - size / 2, y + 1, size, outline);
    tft.drawFastHLine(x - size, y + size - 1, 2 * size, outline);
  }

  void drawKnight(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    tft.drawLine(x - size + 2, y - size, x - size / 2, y - size - 2, outline);
    tft.drawLine(x - size / 2, y - size - 2, x, y - size, outline);
    tft.drawLine(x, y - size, x + 1, y + 1, outline);
    tft.fillRect(x - size + 2, y - 1, 2 * size - 4, size - 1, fill);
    tft.drawRect(x - size + 2, y - 1, 2 * size - 4, size - 1, outline);
    tft.drawFastHLine(x - size, y + size - 1, 2 * size, outline);
  }

  void drawPawn(Adafruit_ILI9341 &tft, int x, int y, int size, uint16_t fill, uint16_t outline) {
    tft.fillCircle(x, y - size, size / 3, fill);
    tft.drawCircle(x, y - size, size / 3, outline);
    tft.drawFastVLine(x, y - size + size / 3, size / 3, outline);
    tft.fillRect(x - size / 2, y - size / 3, size, size / 2, fill);
    tft.drawRect(x - size / 2, y - size / 3, size, size / 2, outline);
    tft.drawFastHLine(x - size, y + size / 3, 2 * size, outline);
  }

  void drawPiece(Adafruit_ILI9341 &tft, char piece, int x, int y, int size) {
    if (!piece) return;

#if CHESS_SPRITE_ATLAS_AVAILABLE
    int spriteIndex = atlasIndexForPiece(piece);
    if (spriteIndex >= 0) {
      drawSpriteAtlas(tft, spriteIndex + (isWhite(piece) ? 0 : 6), x, y);
      return;
    }
#endif

    bool white = isWhite(piece);
    uint16_t fillColor = white ? ILI9341_WHITE : 0x1082;
    uint16_t outlineColor = white ? 0x7BEF : ILI9341_BLACK;

    char kind = lowerPiece(piece);
    tft.fillCircle(x, y, size + 1, fillColor);
    tft.drawCircle(x, y, size + 1, outlineColor);

    switch (kind) {
      case 'k': drawKing(tft, x, y, size - 2, fillColor, outlineColor); break;
      case 'q': drawQueen(tft, x, y, size - 2, fillColor, outlineColor); break;
      case 'r': drawRook(tft, x, y, size - 2, fillColor, outlineColor); break;
      case 'b': drawBishop(tft, x, y, size - 2, fillColor, outlineColor); break;
      case 'n': drawKnight(tft, x, y, size - 2, fillColor, outlineColor); break;
      case 'p': drawPawn(tft, x, y, size - 2, fillColor, outlineColor); break;
    }
  }

  void drawAllPieces(Adafruit_ILI9341 &tft, const ChessPosition &position, int squareSize) {
    int pieceSize = (squareSize * 2) / 5;
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
