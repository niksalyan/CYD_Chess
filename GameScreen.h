#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include <Adafruit_ILI9341.h>

#include "AIEngine.h"
#include "ChessBoard.h"
#include "Screen.h"

class GameScreen : public Screen {
public:
  GameScreen();
  void begin(Adafruit_ILI9341 &display);
  void draw();
  bool handleTouch(int x, int y);
  bool update();
  void requestNewGame();

private:
  void draw(bool thinking);
  void refreshLegalMoves();
  bool makeHumanMove(int from, int to);
  void handleBoardTouch(int x, int y);
  void startNewGame(bool playerFirst);
  void makeAIMove();

  ChessPosition position;
  ChessBoard chessBoard;
  int selectedSquare;
  ChessMove legalMoves[ChessEngine::MAX_MOVES];
  uint8_t legalMoveCount;
  ChessMove lastMove;
  uint8_t aiDepth;
  bool gameOver;
  bool waitingForAI;
  bool humanPlaysWhite;
  bool showFirstMoveSelection;
};

#endif
