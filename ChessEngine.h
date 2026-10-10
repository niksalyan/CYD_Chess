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

  void reset(ChessPosition &position);
  bool isWhitePiece(char piece);
  uint8_t getLegalMoves(const ChessPosition &position, ChessMove *moves, uint8_t capacity);
  bool makeMove(ChessPosition &position, const ChessMove &move);
  bool isInCheck(const ChessPosition &position, bool whiteKing);
  bool findBestMove(const ChessPosition &position, uint8_t depth, ChessMove &bestMove);
}

#endif
