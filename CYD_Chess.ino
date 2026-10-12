#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include <XPT2046_Touchscreen.h>

#include "ChessEngine.h"
#include "AIEngine.h"
#include "ChessBoard.h"


#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST -1

// Adjust these values if your touch panel's raw readings differ.
static const int TOUCH_X_MIN = 200;
static const int TOUCH_X_MAX = 3900;
static const int TOUCH_Y_MIN = 200;
static const int TOUCH_Y_MAX = 3900;
static const bool TOUCH_SWAP_XY = false;
static const bool TOUCH_INVERT_X = false;
static const bool TOUCH_INVERT_Y = false;

bool showNewGameDialog = false;

SPIClass touchSPI = SPIClass(VSPI);
SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft(&tftSPI, TFT_DC, TFT_CS, TFT_RST);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

ChessPosition position;
ChessBoard chessBoard;
int selectedSquare = -1;
ChessMove legalMoves[ChessEngine::MAX_MOVES];
uint8_t legalMoveCount = 0;
ChessMove lastMove = { 255, 255, 0, 0 };
uint8_t aiDepth = 3;
bool gameOver = false;
bool waitingForAI = false;
bool previousTouch = false;
uint32_t lastTouchMillis = 0;



static int mapTouchAxis(int value, int low, int high, int extent, bool invert) {
  if (high <= low) return 0;
  long mapped = (long)(value - low) * (extent - 1) / (high - low);
  mapped = constrain(mapped, 0, extent - 1);
  return invert ? extent - 1 - (int)mapped : (int)mapped;
}

static bool readTouchPoint(int &x, int &y) {
  if (!touch.touched()) return false;
  TS_Point point = touch.getPoint();
  int rawX = TOUCH_SWAP_XY ? point.y : point.x;
  int rawY = TOUCH_SWAP_XY ? point.x : point.y;
  x = mapTouchAxis(rawX, TOUCH_X_MIN, TOUCH_X_MAX, tft.width(), TOUCH_INVERT_X);
  y = mapTouchAxis(rawY, TOUCH_Y_MIN, TOUCH_Y_MAX, tft.height(), TOUCH_INVERT_Y);
  return true;
}

static void refreshLegalMoves() {
  legalMoveCount = ChessEngine::getLegalMoves(position, legalMoves, ChessEngine::MAX_MOVES);
}

static bool makeHumanMove(int from, int to) {
  for (uint8_t i = 0; i < legalMoveCount; ++i) {
    if (legalMoves[i].from == from && legalMoves[i].to == to) {
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

static void handleBoardTouch(int x, int y) {
  int square = chessBoard.squareAt(x, y);
  if (square < 0 || gameOver || waitingForAI || !position.whiteToMove) return;

  if (selectedSquare < 0) {
    if (ChessEngine::isWhitePiece(position.board[square])) selectedSquare = square;
  } else if (square == selectedSquare) {
    selectedSquare = -1;
  } else if (!makeHumanMove(selectedSquare, square)) {
    if (ChessEngine::isWhitePiece(position.board[square])) selectedSquare = square;
  }
}

static void startNewGame() {
  ChessEngine::reset(position);
  selectedSquare = -1;
  lastMove.from = 255;
  lastMove.to = 255;
  gameOver = false;
  waitingForAI = false;
  refreshLegalMoves();
}

static void makeAIMove() {
  ChessMove move;
  if (AIEngine::findBestMove(position, aiDepth, move)) {
    ChessEngine::makeMove(position, move);
    lastMove = move;
    refreshLegalMoves();
  }
  waitingForAI = false;
}

void setup() {
  Serial.begin(115200);
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  touch.setRotation(0);

  tftSPI.begin();
  tft.begin();
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  tft.setRotation(0);


  chessBoard.begin(tft);
  startNewGame();
  chessBoard.draw(position, selectedSquare, legalMoves, legalMoveCount, lastMove,
                  aiDepth, gameOver, false);
}

void loop() {
  int x, y;
  bool isTouched = readTouchPoint(x, y);
  uint32_t now = millis();
  bool redraw = false;

  if (isTouched && !previousTouch && now - lastTouchMillis > 180) {
    lastTouchMillis = now;

    if (showNewGameDialog) {
      // While the dialog is open, ignore all other controls.
      if (chessBoard.isConfirmNewGameButton(x, y)) {
        showNewGameDialog = false;
        startNewGame();
        redraw = true;
      } else if (chessBoard.isCancelNewGameButton(x, y)) {
        showNewGameDialog = false;
        redraw = true;
      }
    } else if (chessBoard.isNewGameButton(x, y)) {
      showNewGameDialog = true;
      redraw = true;
    } else if (chessBoard.isDepthButton(x, y)) {
      aiDepth = aiDepth >= 4 ? 2 : aiDepth + 1;
      redraw = true;
    } else {
      handleBoardTouch(x, y);
      redraw = true;
    }
  }
  previousTouch = isTouched;

  if (waitingForAI) {
    chessBoard.draw(position, selectedSquare, legalMoves, legalMoveCount, lastMove,
                    aiDepth, false, true);
    makeAIMove();
    gameOver = legalMoveCount == 0 || position.halfmoveClock >= 100;
    redraw = true;
  }

  if (redraw) {
    chessBoard.draw(position, selectedSquare, legalMoves, legalMoveCount,
                    lastMove, aiDepth, gameOver, false);

    if (showNewGameDialog) {
      chessBoard.drawNewGameConfirmation();
    }
  }
  delay(15);
}
