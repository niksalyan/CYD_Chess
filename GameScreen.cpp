#include "GameScreen.h"
#include "MenuScreen.h"
#include "Navigation.h"

GameScreen::GameScreen()
  : historyCountBeforeHumanMove(0),
    hasUndoPosition(false),
    hasHumanTurnSnapshot(false),
    selectedSquare(-1),
    legalMoveCount(0),
    lastMove{255, 255, 0, 0},
    recordedMoveCount(0),
    historyTruncated(false),
    aiDepth(3),
    gameOver(false),
    waitingForAI(false),
    humanPlaysWhite(true),
    showFirstMoveSelection(false) {
}

void GameScreen::begin(Adafruit_ILI9341 &display) {
  chessBoard.begin(display);
  startNewGame(true);
}

void GameScreen::refreshLegalMoves() {
  legalMoveCount = ChessEngine::getLegalMoves(
    position, legalMoves, ChessEngine::MAX_MOVES);
}

bool GameScreen::makeHumanMove(int from, int to) {
  for (uint8_t i = 0; i < legalMoveCount; ++i) {
    if (legalMoves[i].from == from && legalMoves[i].to == to) {
      if (recordedMoveCount >= MAX_RECORDED_MOVES) {
        hasUndoPosition = false;
        hasHumanTurnSnapshot = false;
      } else {
        positionBeforeLastMove = position;
        positionBeforeHumanMove = position;
        historyCountBeforeHumanMove = recordedMoveCount;
        hasUndoPosition = true;
        hasHumanTurnSnapshot = true;
      }
      recordMove(legalMoves[i]);
      ChessEngine::makeMove(position, legalMoves[i]);
      lastMove = legalMoves[i];
      selectedSquare = -1;
      refreshLegalMoves();
      gameOver = legalMoveCount == 0 || position.halfmoveClock >= 100;
      waitingForAI = !gameOver;
      return true;
    }
  }
  return false;
}

void GameScreen::handleBoardTouch(int x, int y) {
  int square = chessBoard.squareAt(x, y, humanPlaysWhite);
  if (square < 0 || gameOver || waitingForAI ||
      position.whiteToMove != humanPlaysWhite) return;

  if (selectedSquare < 0) {
    if (position.board[square] &&
        ChessEngine::isWhitePiece(position.board[square]) == humanPlaysWhite)
      selectedSquare = square;
  } else if (square == selectedSquare) {
    selectedSquare = -1;
  } else if (!makeHumanMove(selectedSquare, square)) {
    if (position.board[square] &&
        ChessEngine::isWhitePiece(position.board[square]) == humanPlaysWhite)
      selectedSquare = square;
  }
}

void GameScreen::startNewGame(bool playerFirst) {
  ChessEngine::reset(position);
  humanPlaysWhite = playerFirst;
  selectedSquare = -1;
  lastMove.from = 255;
  lastMove.to = 255;
  recordedMoveCount = 0;
  hasUndoPosition = false;
  hasHumanTurnSnapshot = false;
  historyTruncated = false;
  gameOver = false;
  waitingForAI = !playerFirst;
  refreshLegalMoves();
}

void GameScreen::makeAIMove() {
  ChessMove move;
  if (AIEngine::findBestMove(position, aiDepth, move)) {
    if (recordedMoveCount < MAX_RECORDED_MOVES) {
      positionBeforeLastMove = position;
      hasUndoPosition = true;
    } else {
      hasUndoPosition = false;
      hasHumanTurnSnapshot = false;
    }
    recordMove(move);
    ChessEngine::makeMove(position, move);
    lastMove = move;
    refreshLegalMoves();
  }
  waitingForAI = false;
}

void GameScreen::draw() {
  draw(false);
}

void GameScreen::draw(bool thinking) {
  chessBoard.draw(position, selectedSquare, legalMoves, legalMoveCount,
                  lastMove, aiDepth, gameOver, thinking, humanPlaysWhite);
  if (showFirstMoveSelection) chessBoard.drawFirstMoveSelection();
}

bool GameScreen::handleTouch(const TouchPoint &point) {
  int x = point.x;
  int y = point.y;
  if (showFirstMoveSelection) {
    if (chessBoard.isPlayerFirstButton(x, y)) {
      showFirstMoveSelection = false;
      startNewGame(true);
      return true;
    }
    if (chessBoard.isAIFirstButton(x, y)) {
      showFirstMoveSelection = false;
      startNewGame(false);
      return true;
    }
    if (chessBoard.isCancelNewGameButton(x, y)) {
      showFirstMoveSelection = false;
      return true;
    }
    return false;
  }

  if (chessBoard.isMenuButton(x, y)) {
    return Navigation::navigateTo(MenuScreen::instance());
  }
  if (chessBoard.isDepthButton(x, y)) {
    aiDepth = aiDepth >= 4 ? 2 : aiDepth + 1;
    return true;
  }

  handleBoardTouch(x, y);
  return true;
}

void GameScreen::requestNewGame() {
  showFirstMoveSelection = true;
}

uint16_t GameScreen::moveHistoryCount() const {
  return recordedMoveCount;
}

bool GameScreen::getRecordedMove(uint16_t index, RecordedMove &move) const {
  if (index >= recordedMoveCount) return false;
  move = moveHistory[index];
  return true;
}

bool GameScreen::moveHistoryTruncated() const {
  return historyTruncated;
}

bool GameScreen::canUndoLastMove() const {
  return hasUndoPosition && recordedMoveCount > 0;
}

bool GameScreen::undoLastMove() {
  if (!canUndoLastMove()) return false;

  if (hasHumanTurnSnapshot && historyCountBeforeHumanMove < recordedMoveCount) {
    position = positionBeforeHumanMove;
    recordedMoveCount = historyCountBeforeHumanMove;
  } else {
    position = positionBeforeLastMove;
    --recordedMoveCount;
  }
  hasUndoPosition = false;
  hasHumanTurnSnapshot = false;
  selectedSquare = -1;
  refreshLegalMoves();
  lastMove = recordedMoveCount
    ? moveHistory[recordedMoveCount - 1].move
    : ChessMove{255, 255, 0, 0};
  gameOver = legalMoveCount == 0 || position.halfmoveClock >= 100;
  waitingForAI = !gameOver && position.whiteToMove != humanPlaysWhite;
  return true;
}

void GameScreen::recordMove(const ChessMove &move) {
  if (recordedMoveCount >= MAX_RECORDED_MOVES) {
    historyTruncated = true;
    return;
  }
  moveHistory[recordedMoveCount++] = {move, position.whiteToMove};
}

bool GameScreen::update() {
  if (!waitingForAI) return false;

  draw(true);
  makeAIMove();
  gameOver = legalMoveCount == 0 || position.halfmoveClock >= 100;
  return true;
}
