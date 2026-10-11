#include "ChessGraphics.h"

#if CHESS_SPRITE_ATLAS_AVAILABLE
#include "ChessPieceAtlas.h"
#endif

namespace ChessGraphics {
  
  int atlasIndexForPiece(char piece) {
    switch (piece) {
      case 'K': return 0;
      case 'Q': return 1;
      case 'B': return 2;
      case 'N': return 3;
      case 'R': return 4;
      case 'P': return 5;
      case 'k': return 6;
      case 'q': return 7;
      case 'b': return 8;
      case 'n': return 9;
      case 'r': return 10;
      case 'p': return 11;
      default: return -1;
    }
  }

  void drawSpriteAtlas(
    Adafruit_ILI9341 &tft,
    int spriteIndex,
    int x,
    int y)
{
    if (spriteIndex < 0 ||
        spriteIndex >= CHESS_PIECE_ATLAS_COUNT)
        return;

    const uint16_t transparent = 0xF81F;

    const uint16_t* sprite =
        (const uint16_t*)pgm_read_ptr(&chessPieceAtlas[spriteIndex]);

    for (int py = 0; py < 30; ++py) {
        for (int px = 0; px < 30; ++px) {
            uint16_t color =
                pgm_read_word(&sprite[py * 30 + px]);

            if (color != transparent) {
                tft.drawPixel(
                    x + px - 15,
                    y + py - 15,
                    color);
            }
        }
    }
}


  void drawPiece(Adafruit_ILI9341 &tft, char piece, int x, int y, int size) {
    int spriteIndex = atlasIndexForPiece(piece);
    if (spriteIndex >= 0) {
      drawSpriteAtlas(tft, spriteIndex, x, y);
      return;
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
