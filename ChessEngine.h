#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include <Arduino.h>

struct ChessMove {
  uint8_t from;
  uint8_t to;
  char promotion;
  uint8_t flags;
};

struct ChessPosition {
  char board[64];
  bool whiteToMove;
  uint8_t castlingRights;
  int8_t enPassantSquare;
  uint16_t halfmoveClock;
  uint16_t fullmoveNumber;
};

namespace ChessEngine {
  static const uint8_t MAX_MOVES = 224;

  // Board state initialization and management
  void reset(ChessPosition &position);

  // Piece and position queries
  bool isWhitePiece(char piece);

  // Move generation and validation
  uint8_t getLegalMoves(const ChessPosition &position, ChessMove *moves, uint8_t capacity);
  bool makeMove(ChessPosition &position, const ChessMove &move);

  // Position evaluation
  bool isInCheck(const ChessPosition &position, bool whiteKing);
}

#endif
