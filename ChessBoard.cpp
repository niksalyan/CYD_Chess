#include "ChessBoard.h"

// Instantiate using the configuration flags defined in ChessBoard.h
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

namespace {
  const int SQUARE_SIZE = 30;
  const int BOARD_SIZE = 240;
  
  const int PANEL_X = 0;
  const int PANEL_Y = 240;
  const int PANEL_W = 240;
  const int PANEL_H = 80;

  const uint16_t LIGHT_SQUARE = 0xD69A;
  const uint16_t DARK_SQUARE = 0x5AEB;
  const uint16_t HIGHLIGHT = 0xFFE0;
  const uint16_t MOVE_HINT = 0x07E0;
  const uint16_t PANEL_BG = 0x18E3;
}

void ChessBoard::begin(Adafruit_ILI9341 &display) {
  tft = &display;
  pixels.begin();
  pixels.setBrightness(128); // Brightness level out of 255
  pixels.show();
}

int ChessBoard::squareAt(int x, int y) const {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return -1;
  int file = x / SQUARE_SIZE;
  int rank = 7 - y / SQUARE_SIZE;
  return rank * 8 + file;
}

bool ChessBoard::isNewGameButton(int x, int y) const {
  const int BUTTON_X = 155;
  const int PANEL_Y_LOC = 240;
  return x >= BUTTON_X && x < BUTTON_X + 78 && y >= PANEL_Y_LOC + 6 && y < PANEL_Y_LOC + 36;
}

bool ChessBoard::isDepthButton(int x, int y) const {
  const int BUTTON_X = 155;
  const int PANEL_Y_LOC = 240;
  return x >= BUTTON_X && x < BUTTON_X + 78 && y >= PANEL_Y_LOC + 42 && y < PANEL_Y_LOC + 72;
}

void ChessBoard::draw(const ChessPosition &position, int selectedSquare,
                      const ChessMove *legalMoves, uint8_t legalMoveCount,
                      const ChessMove &lastMove, uint8_t aiDepth,
                      bool gameOver, bool thinking) {
  drawBoard(position, selectedSquare, legalMoves, legalMoveCount, lastMove);
  drawPanel(position, aiDepth, gameOver, thinking);
  updateMatrix(position, selectedSquare, legalMoves, legalMoveCount, lastMove);
}


// 2. Cleaned up drawBoard (Removed all 'pixels' logic entirely from here)
void ChessBoard::drawBoard(const ChessPosition &position, int selectedSquare,
                           const ChessMove *legalMoves, uint8_t legalMoveCount,
                           const ChessMove &lastMove) {

  int centerOffset = SQUARE_SIZE / 2;
  int pieceSize = (SQUARE_SIZE * 2) / 5;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;
      int x = file * SQUARE_SIZE;
      int y = (7 - rank) * SQUARE_SIZE;

      // Determine TFT Colors
      bool isLight = ((file + rank) & 1) != 0;
      uint16_t tftColor = isLight ? LIGHT_SQUARE : DARK_SQUARE;

      if (square == lastMove.from || square == lastMove.to)
        tftColor = 0xB5F3;

      if (square == selectedSquare)
        tftColor = HIGHLIGHT;

      // Render Background to TFT
      tft->fillRect(x, y, SQUARE_SIZE, SQUARE_SIZE, tftColor);

      // Render Move Hints to TFT
      if (selectedSquare >= 0) {
        for (uint8_t i = 0; i < legalMoveCount; ++i) {
          if (legalMoves[i].from != selectedSquare || legalMoves[i].to != square)
            continue;

          int cx = x + centerOffset;
          int cy = y + centerOffset;

          if (position.board[square])
            tft->drawCircle(cx, cy, 13, MOVE_HINT);
          else
            tft->fillCircle(cx, cy, 4, MOVE_HINT);

          break;
        }
      }

      // Render Chess Pieces to TFT
      char piece = position.board[square];
      if (piece) {
        int cx = x + centerOffset;
        int cy = y + centerOffset;
        ChessGraphics::drawPiece(*tft, piece, cx, cy, pieceSize);
      }
    }
  }
}

// 3. New Dedicated Matrix Update Function (With independent timing hooks)
void ChessBoard::updateMatrix(const ChessPosition &position, int selectedSquare,
                              const ChessMove *legalMoves, uint8_t legalMoveCount,
                              const ChessMove &lastMove) {
  
  // Optional: Delay before processing matrix (e.g., let TFT finish rendering)
  delay(5); 

  pixels.clear();

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      // Calculate base matrix background pattern colors
      bool isLight = ((file + rank) & 1) != 0;
      uint32_t matrixColor = isLight ? pixels.Color(70, 70, 70) : pixels.Color(20, 20, 20);

      // --- Last Move Highlight Logic ---
      if (square == lastMove.from || square == lastMove.to) {
        if (position.whiteToMove) {
          // It's your turn, meaning the AI just made this move -> RED
          matrixColor = pixels.Color(255, 0, 0); 
        } else {
          // It's the AI's turn, meaning YOU just made this move -> GREEN (or choose Orange/Cyan)
          matrixColor = pixels.Color(0, 255, 0); 
        }
      }

      // Selected Piece Assignment (Yellow)
      if (square == selectedSquare) {
        matrixColor = pixels.Color(255, 255, 0); 
      }

      // Legal Move Destination Assignment (Green)
      if (selectedSquare >= 0) {
        for (uint8_t i = 0; i < legalMoveCount; ++i) {
          if (legalMoves[i].from == selectedSquare && legalMoves[i].to == square) {
            matrixColor = pixels.Color(0, 255, 0); 
            break;
          }
        }
      }

      // Write to matrix coordinate buffer
      drawMatrix(file, 7 - rank, matrixColor);
    }
  }

  pixels.show(); // Push color stream down physical data line

  // Optional: Delay after processing matrix (allows power rail voltage to settle)
  delay(10); 
}



// Restored to full 2x2 cluster drawing functionality
void ChessBoard::drawMatrix(uint8_t x, uint8_t y, uint32_t color) {
  setPixel(x * 2, y * 2, color);
  setPixel(x * 2 + 1, y * 2, color);
  setPixel(x * 2, y * 2 + 1, color);
  setPixel(x * 2 + 1, y * 2 + 1, color);
}

void ChessBoard::setPixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= MATRIX_SIZE || y < 0 || y >= MATRIX_SIZE) return;

  int rx = x;
  int ry = y;

  // 1. Run baseline screen rotation logic from your macro settings
  switch (SCREEN_DIR) {
    case SCREEN_DIR_0:
      rx = x;
      ry = y;
      break;
    case SCREEN_DIR_90:
      rx = MATRIX_SIZE - 1 - y;
      ry = x;
      break;
    case SCREEN_DIR_180:
      rx = MATRIX_SIZE - 1 - x;
      ry = MATRIX_SIZE - 1 - y;
      break;
    case SCREEN_DIR_270:
      rx = y;
      ry = MATRIX_SIZE - 1 - x;
      break;
  }

  // 2. Map cleanly onto a Top-Left Mirrored Serpentine data track.
  // Because the physical panel layout is mirrored, even rows now run right-to-left
  // and odd rows run left-to-right. This completely fixes the 1-pixel zigzag shift.
  int i = (ry % 2 == 0)
            ? (ry * MATRIX_SIZE + (MATRIX_SIZE - 1 - rx))
            : (ry * MATRIX_SIZE + rx);

  pixels.setPixelColor(i, color);
}


void ChessBoard::drawPanel(const ChessPosition &position, uint8_t aiDepth,
                           bool gameOver, bool thinking) {
  tft->fillRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, PANEL_BG);
  tft->setTextWrap(false);
  tft->setTextSize(1);
  
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(PANEL_X + 8, PANEL_Y + 8);
  tft->print("CYD CHESS");
  tft->drawFastHLine(PANEL_X + 6, PANEL_Y + 20, 75, 0x7BEF);

  tft->setCursor(PANEL_X + 8, PANEL_Y + 30);
  if (gameOver) {
    ChessMove moves[ChessEngine::MAX_MOVES];
    uint8_t count = ChessEngine::getLegalMoves(position, moves, ChessEngine::MAX_MOVES);
    if (count == 0) {
      bool checked = ChessEngine::isInCheck(position, position.whiteToMove);
      tft->setTextColor(checked ? ILI9341_RED : ILI9341_YELLOW);
      tft->print(checked ? "CHECKMATE" : "STALEMATE");
    } else {
      tft->setTextColor(ILI9341_YELLOW);
      tft->print("DRAW (50-M)");
    }
  } else if (thinking) {
    tft->setTextColor(ILI9341_YELLOW);
    tft->print("AI THINKING...");
  } else {
    tft->setTextColor(ILI9341_CYAN);
    tft->print(position.whiteToMove ? "YOUR TURN" : "AI TURN");
    if (ChessEngine::isInCheck(position, position.whiteToMove)) {
      tft->setCursor(PANEL_X + 8, PANEL_Y + 42);
      tft->setTextColor(ILI9341_RED);
      tft->print("CHECK!");
    }
  }

  int middleX = PANEL_X + 95;
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(middleX, PANEL_Y + 8);
  tft->print("W: YOU");
  tft->setCursor(middleX, PANEL_Y + 23);
  tft->print("B: AI");

  int buttonX = PANEL_X + 155;
  
  tft->drawRoundRect(buttonX, PANEL_Y + 6, 78, 30, 4, ILI9341_CYAN);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(buttonX + 16, PANEL_Y + 17);
  tft->print("RESET");

  tft->drawRoundRect(buttonX, PANEL_Y + 42, 78, 30, 4, ILI9341_YELLOW);
  tft->setCursor(buttonX + 16, PANEL_Y + 53);
  tft->print("LVL ");
  tft->print(aiDepth);
}

void ChessBoard::drawNewGameConfirmation() {
  const int x = 20;
  const int y = 105;
  const int w = 200;
  const int h = 100;

  // Dim the board behind the dialog.
  tft->fillRect(x, y, w, h, ILI9341_BLACK);
  tft->drawRoundRect(x, y, w, h, 8, ILI9341_CYAN);

  tft->setTextWrap(false);
  tft->setTextSize(1);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(x + 32, y + 17);
  tft->print("Start a new game?");

  // Cancel button
  tft->fillRoundRect(
    x + 12, y + 48, 84, 32, 5, 0x4208
  );
  tft->drawRoundRect(
    x + 12, y + 48, 84, 32, 5, ILI9341_WHITE
  );
  tft->setCursor(x + 31, y + 60);
  tft->print("CANCEL");

  // Confirm button
  tft->fillRoundRect(
    x + 104, y + 48, 84, 32, 5, 0x7800
  );
  tft->drawRoundRect(
    x + 104, y + 48, 84, 32, 5, ILI9341_RED
  );
  tft->setCursor(x + 117, y + 60);
  tft->print("NEW GAME");
}

bool ChessBoard::isConfirmNewGameButton(int x, int y) const {
  return x >= 124 && x < 208 &&
         y >= 153 && y < 185;
}

bool ChessBoard::isCancelNewGameButton(int x, int y) const {
  return x >= 32 && x < 116 &&
         y >= 153 && y < 185;
}
