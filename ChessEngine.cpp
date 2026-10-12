#include "ChessEngine.h"


namespace {
  const uint8_t WHITE_KING_SIDE = 1;
  const uint8_t WHITE_QUEEN_SIDE = 2;
  const uint8_t BLACK_KING_SIDE = 4;
  const uint8_t BLACK_QUEEN_SIDE = 8;
  const uint8_t EN_PASSANT = 1;
  const uint8_t CASTLING = 2;
  const char START_POSITION[64] = {
    'R','N','B','Q','K','B','N','R',
    'P','P','P','P','P','P','P','P',
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    'p','p','p','p','p','p','p','p',
    'r','n','b','q','k','b','n','r'
  };

  bool isPiece(char piece) { return piece != 0; }
  bool isWhite(char piece) { return piece >= 'A' && piece <= 'Z'; }
  bool sameColor(char a, char b) { return isPiece(a) && isPiece(b) && isWhite(a) == isWhite(b); }
  char lowerPiece(char piece) { return isWhite(piece) ? piece + ('a' - 'A') : piece; }

  int findKing(const ChessPosition &position, bool white) {
    const char king = white ? 'K' : 'k';
    for (int i = 0; i < 64; ++i) if (position.board[i] == king) return i;
    return -1;
  }

  bool attacked(const ChessPosition &position, int square, bool byWhite) {
    int file = square & 7;
    int rank = square >> 3;
    int pawnRank = rank + (byWhite ? -1 : 1);
    if (pawnRank >= 0 && pawnRank < 8) {
      for (int df = -1; df <= 1; df += 2) {
        int pawnFile = file + df;
        if (pawnFile >= 0 && pawnFile < 8) {
          char piece = position.board[pawnRank * 8 + pawnFile];
          if (piece == (byWhite ? 'P' : 'p')) return true;
        }
      }
    }

    const int knightOffsets[8][2] = {
      {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}
    };
    for (uint8_t i = 0; i < 8; ++i) {
      int f = file + knightOffsets[i][0], r = rank + knightOffsets[i][1];
      if (f >= 0 && f < 8 && r >= 0 && r < 8 &&
          position.board[r * 8 + f] == (byWhite ? 'N' : 'n')) return true;
    }

    for (int df = -1; df <= 1; ++df) {
      for (int dr = -1; dr <= 1; ++dr) {
        if (df == 0 && dr == 0) continue;
        int f = file + df, r = rank + dr;
        if (f >= 0 && f < 8 && r >= 0 && r < 8 &&
            position.board[r * 8 + f] == (byWhite ? 'K' : 'k')) return true;
      }
    }

    const int directions[8][2] = {
      {1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}
    };
    for (uint8_t d = 0; d < 8; ++d) {
      int f = file + directions[d][0], r = rank + directions[d][1];
      while (f >= 0 && f < 8 && r >= 0 && r < 8) {
        char piece = position.board[r * 8 + f];
        if (piece) {
          if (isWhite(piece) == byWhite) {
            char kind = lowerPiece(piece);
            if (kind == 'q' || (d < 4 && kind == 'r') || (d >= 4 && kind == 'b'))
              return true;
          }
          break;
        }
        f += directions[d][0];
        r += directions[d][1];
      }
    }
    return false;
  }

  void addMove(ChessMove *moves, uint8_t capacity, uint8_t &count,
               int from, int to, char promotion = 0, uint8_t flags = 0) {
    if (count >= capacity) return;
    moves[count++] = {(uint8_t)from, (uint8_t)to, promotion, flags};
  }

  void addPawnMove(ChessMove *moves, uint8_t capacity, uint8_t &count,
                   int from, int to, bool white, uint8_t flags = 0) {
    int rank = to >> 3;
    if ((white && rank == 7) || (!white && rank == 0))
      addMove(moves, capacity, count, from, to, white ? 'Q' : 'q', flags);
    else
      addMove(moves, capacity, count, from, to, 0, flags);
  }

  void addPseudoMoves(const ChessPosition &p, ChessMove *moves, uint8_t capacity, uint8_t &count) {
    count = 0;
    for (int from = 0; from < 64; ++from) {
      char piece = p.board[from];
      if (!piece || isWhite(piece) != p.whiteToMove) continue;
      int file = from & 7, rank = from >> 3;
      char kind = lowerPiece(piece);

      if (kind == 'p') {
        int step = p.whiteToMove ? 1 : -1;
        int nextRank = rank + step;
        if (nextRank >= 0 && nextRank < 8 && !p.board[nextRank * 8 + file]) {
          addPawnMove(moves, capacity, count, from, nextRank * 8 + file, p.whiteToMove);
          int startRank = p.whiteToMove ? 1 : 6;
          int twiceRank = rank + 2 * step;
          if (rank == startRank && !p.board[twiceRank * 8 + file])
            addMove(moves, capacity, count, from, twiceRank * 8 + file);
        }
        for (int df = -1; df <= 1; df += 2) {
          int f = file + df, r = rank + step;
          if (f < 0 || f >= 8 || r < 0 || r >= 8) continue;
          int to = r * 8 + f;
          if (p.board[to] && !sameColor(piece, p.board[to]))
            addPawnMove(moves, capacity, count, from, to, p.whiteToMove);
          else if (to == p.enPassantSquare)
            addPawnMove(moves, capacity, count, from, to, p.whiteToMove, EN_PASSANT);
        }
      } else if (kind == 'n') {
        const int offsets[8][2] = {
          {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}
        };
        for (uint8_t i = 0; i < 8; ++i) {
          int f = file + offsets[i][0], r = rank + offsets[i][1];
          if (f < 0 || f >= 8 || r < 0 || r >= 8) continue;
          int to = r * 8 + f;
          if (!sameColor(piece, p.board[to])) addMove(moves, capacity, count, from, to);
        }
      } else if (kind == 'k') {
        for (int df = -1; df <= 1; ++df) {
          for (int dr = -1; dr <= 1; ++dr) {
            if (df == 0 && dr == 0) continue;
            int f = file + df, r = rank + dr;
            if (f < 0 || f >= 8 || r < 0 || r >= 8) continue;
            int to = r * 8 + f;
            if (!sameColor(piece, p.board[to])) addMove(moves, capacity, count, from, to);
          }
        }
        if (!attacked(p, from, !p.whiteToMove)) {
          int homeRank = p.whiteToMove ? 0 : 7;
          uint8_t kingSide = p.whiteToMove ? WHITE_KING_SIDE : BLACK_KING_SIDE;
          uint8_t queenSide = p.whiteToMove ? WHITE_QUEEN_SIDE : BLACK_QUEEN_SIDE;
          char rook = p.whiteToMove ? 'R' : 'r';
          if (rank == homeRank && file == 4 && (p.castlingRights & kingSide) &&
              p.board[homeRank * 8 + 5] == 0 && p.board[homeRank * 8 + 6] == 0 &&
              p.board[homeRank * 8 + 7] == rook &&
              !attacked(p, homeRank * 8 + 5, !p.whiteToMove) &&
              !attacked(p, homeRank * 8 + 6, !p.whiteToMove))
            addMove(moves, capacity, count, from, homeRank * 8 + 6, 0, CASTLING);
          if (rank == homeRank && file == 4 && (p.castlingRights & queenSide) &&
              p.board[homeRank * 8 + 1] == 0 && p.board[homeRank * 8 + 2] == 0 &&
              p.board[homeRank * 8 + 3] == 0 && p.board[homeRank * 8] == rook &&
              !attacked(p, homeRank * 8 + 3, !p.whiteToMove) &&
              !attacked(p, homeRank * 8 + 2, !p.whiteToMove))
            addMove(moves, capacity, count, from, homeRank * 8 + 2, 0, CASTLING);
        }
      } else {
        const int directions[8][2] = {
          {1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}
        };
        for (uint8_t d = 0; d < 8; ++d) {
          if (kind == 'b' && d < 4) continue;
          if (kind == 'r' && d >= 4) continue;
          int f = file + directions[d][0], r = rank + directions[d][1];
          while (f >= 0 && f < 8 && r >= 0 && r < 8) {
            int to = r * 8 + f;
            if (sameColor(piece, p.board[to])) break;
            addMove(moves, capacity, count, from, to);
            if (p.board[to]) break;
            f += directions[d][0];
            r += directions[d][1];
          }
        }
      }
    }
  }

  void applyMove(ChessPosition &p, const ChessMove &move) {
    char piece = p.board[move.from];
    char captured = p.board[move.to];
    int fromRank = move.from >> 3, toRank = move.to >> 3;
    p.board[move.from] = 0;
    if (move.flags & EN_PASSANT) {
      int capturedSquare = move.to + (p.whiteToMove ? -8 : 8);
      captured = p.board[capturedSquare];
      p.board[capturedSquare] = 0;
    }
    p.board[move.to] = move.promotion ? move.promotion : piece;
    if (move.flags & CASTLING) {
      if (move.to == 6) { p.board[5] = p.board[7]; p.board[7] = 0; }
      else if (move.to == 2) { p.board[3] = p.board[0]; p.board[0] = 0; }
      else if (move.to == 62) { p.board[61] = p.board[63]; p.board[63] = 0; }
      else if (move.to == 58) { p.board[59] = p.board[56]; p.board[56] = 0; }
    }

    if (piece == 'K') p.castlingRights &= ~(WHITE_KING_SIDE | WHITE_QUEEN_SIDE);
    if (piece == 'k') p.castlingRights &= ~(BLACK_KING_SIDE | BLACK_QUEEN_SIDE);
    if (move.from == 0 || move.to == 0) p.castlingRights &= ~WHITE_QUEEN_SIDE;
    if (move.from == 7 || move.to == 7) p.castlingRights &= ~WHITE_KING_SIDE;
    if (move.from == 56 || move.to == 56) p.castlingRights &= ~BLACK_QUEEN_SIDE;
    if (move.from == 63 || move.to == 63) p.castlingRights &= ~BLACK_KING_SIDE;

    p.enPassantSquare = -1;
    if (lowerPiece(piece) == 'p' && abs(toRank - fromRank) == 2)
      p.enPassantSquare = (move.from + move.to) / 2;
    if (lowerPiece(piece) == 'p' || captured) p.halfmoveClock = 0;
    else ++p.halfmoveClock;
    if (!p.whiteToMove) ++p.fullmoveNumber;
    p.whiteToMove = !p.whiteToMove;
  }

  uint8_t legalMoves(const ChessPosition &position, ChessMove *moves, uint8_t capacity) {
    ChessMove pseudo[ChessEngine::MAX_MOVES];
    uint8_t pseudoCount = 0, resultCount = 0;
    addPseudoMoves(position, pseudo, ChessEngine::MAX_MOVES, pseudoCount);
    for (uint8_t i = 0; i < pseudoCount && resultCount < capacity; ++i) {
      ChessPosition next = position;
      applyMove(next, pseudo[i]);
      if (!ChessEngine::isInCheck(next, position.whiteToMove))
        moves[resultCount++] = pseudo[i];
    }
    return resultCount;
  }
}

namespace ChessEngine {
  void reset(ChessPosition &position) {
    for (uint8_t i = 0; i < 64; ++i) position.board[i] = START_POSITION[i];
    position.whiteToMove = true;
    position.castlingRights = WHITE_KING_SIDE | WHITE_QUEEN_SIDE |
                              BLACK_KING_SIDE | BLACK_QUEEN_SIDE;
    position.enPassantSquare = -1;
    position.halfmoveClock = 0;
    position.fullmoveNumber = 1;
  }

  bool isWhitePiece(char piece) { return piece && isWhite(piece); }

  uint8_t getLegalMoves(const ChessPosition &position, ChessMove *moves, uint8_t capacity) {
    return legalMoves(position, moves, capacity);
  }

  bool makeMove(ChessPosition &position, const ChessMove &move) {
    if (move.from >= 64 || move.to >= 64 || !position.board[move.from]) return false;
    applyMove(position, move);
    return true;
  }

  bool isInCheck(const ChessPosition &position, bool whiteKing) {
    int king = findKing(position, whiteKing);
    return king < 0 || attacked(position, king, !whiteKing);
  }
}
