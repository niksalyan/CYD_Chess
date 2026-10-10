#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include "ChessEngine.h"

namespace AIEngine {
  static const int MATE_SCORE = 30000;

  // Find the best move for the current position using minimax with alpha-beta pruning
  bool findBestMove(const ChessPosition &position, uint8_t depth, ChessMove &bestMove);
}

#endif
