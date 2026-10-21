#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include <Adafruit_ST7789.h>

#include "AIEngine.h"
#include "ChessBoard.h"
#include "Screen.h"

struct RecordedMove {
  ChessMove move;
  bool white;
};

class GameScreen : public Screen {
public:
  GameScreen();
  void begin(Adafruit_ST7789 &display);
  void draw();
  bool handleTouch(const TouchPoint &point);
  bool update();
  void requestNewGame();
  uint16_t moveHistoryCount() const;
  bool getRecordedMove(uint16_t index, RecordedMove &move) const;
  bool moveHistoryTruncated() const;
  bool canUndoLastMove() const;
  bool undoLastMove();

private:
  static const uint16_t MAX_RECORDED_MOVES = 512;

  void draw(bool thinking);
  void refreshLegalMoves();
  bool makeHumanMove(int from, int to);
  void handleBoardTouch(int x, int y);
  void startNewGame(bool playerFirst);
  void makeAIMove();
  void recordMove(const ChessMove &move);

  ChessPosition position;
  ChessPosition positionBeforeLastMove;
  ChessPosition positionBeforeHumanMove;
  uint16_t historyCountBeforeHumanMove;
  bool hasUndoPosition;
  bool hasHumanTurnSnapshot;
  ChessBoard chessBoard;
  int selectedSquare;
  ChessMove legalMoves[ChessEngine::MAX_MOVES];
  uint8_t legalMoveCount;
  ChessMove lastMove;
  RecordedMove moveHistory[MAX_RECORDED_MOVES];
  uint16_t recordedMoveCount;
  bool historyTruncated;
  uint8_t aiDepth;
  bool gameOver;
  bool waitingForAI;
  bool humanPlaysWhite;
  bool showFirstMoveSelection;
};

#endif
