#include "AIEngine.h"

namespace {
  char lowerPiece(char piece) {
    return (piece >= 'A' && piece <= 'Z') ? piece + ('a' - 'A') : piece;
  }

  bool isWhite(char piece) { return piece >= 'A' && piece <= 'Z'; }

  int pieceValue(char piece) {
    switch (lowerPiece(piece)) {
      case 'p': return 100;
      case 'n': return 320;
      case 'b': return 330;
      case 'r': return 500;
      case 'q': return 900;
      default: return 0;
    }
  }

  int evaluate(const ChessPosition &p) {
    int score = 0;
    for (int square = 0; square < 64; ++square) {
      char piece = p.board[square];
      if (!piece) continue;
      int value = pieceValue(piece);
      int file = square & 7, rank = square >> 3;
      int advance = isWhite(piece) ? rank : 7 - rank;
      if (lowerPiece(piece) == 'p') value += advance * 5;
      if (lowerPiece(piece) == 'n' || lowerPiece(piece) == 'b')
        value += 4 * (3 - abs(3 - file)) + 4 * (3 - abs(3 - rank));
      score += isWhite(piece) ? value : -value;
    }
    return p.whiteToMove ? score : -score;
  }

  int moveOrderScore(const ChessPosition &p, const ChessMove &move) {
    char victim = p.board[move.to];
    if (move.flags & 1) victim = p.whiteToMove ? 'p' : 'P';  // EN_PASSANT = 1
    return pieceValue(victim) * 10 - pieceValue(p.board[move.from]) +
           (move.promotion ? pieceValue(move.promotion) : 0);
  }

  int search(const ChessPosition &p, uint8_t depth, int alpha, int beta, int ply) {
    ChessMove moves[ChessEngine::MAX_MOVES];
    uint8_t count = ChessEngine::getLegalMoves(p, moves, ChessEngine::MAX_MOVES);
    if (count == 0) return ChessEngine::isInCheck(p, p.whiteToMove) ? -AIEngine::MATE_SCORE + ply : 0;
    if (p.halfmoveClock >= 100) return 0;
    if (depth == 0) return evaluate(p);

    for (uint8_t i = 0; i < count; ++i) {
      for (uint8_t j = i + 1; j < count; ++j) {
        if (moveOrderScore(p, moves[j]) > moveOrderScore(p, moves[i])) {
          ChessMove temp = moves[i]; moves[i] = moves[j]; moves[j] = temp;
        }
      }
      ChessPosition next = p;
      ChessEngine::makeMove(next, moves[i]);
      int score = -search(next, depth - 1, -beta, -alpha, ply + 1);
      if (score > alpha) alpha = score;
      if (alpha >= beta) break;
    }
    return alpha;
  }
}

namespace AIEngine {
  bool findBestMove(const ChessPosition &position, uint8_t depth, ChessMove &bestMove) {
    ChessMove moves[ChessEngine::MAX_MOVES];
    uint8_t count = ChessEngine::getLegalMoves(position, moves, ChessEngine::MAX_MOVES);
    if (!count) return false;
    int bestScore = -MATE_SCORE;
    bool found = false;
    int alpha = -MATE_SCORE;
    for (uint8_t i = 0; i < count; ++i) {
      ChessPosition next = position;
      ChessEngine::makeMove(next, moves[i]);
      int score = -search(next, depth > 0 ? depth - 1 : 0, -MATE_SCORE, -alpha, 1);
      if (!found || score > bestScore) {
        bestScore = score;
        bestMove = moves[i];
        found = true;
      }
      if (score > alpha) alpha = score;
    }
    return found;
  }
}
