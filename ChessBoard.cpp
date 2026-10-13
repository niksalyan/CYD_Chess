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
  pixels.setBrightness(144); // Brightness level out of 255
  pixels.show();
}

int ChessBoard::squareAt(int x, int y, bool humanPlaysWhite) const {
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return -1;
  int file = x / SQUARE_SIZE;
  int rank = humanPlaysWhite ? 7 - y / SQUARE_SIZE : y / SQUARE_SIZE;
  if (!humanPlaysWhite) file = 7 - file;
  return rank * 8 + file;
}

bool ChessBoard::isMenuButton(int x, int y) const {
  const int BUTTON_X = 155;
  const int PANEL_Y_LOC = 240;
  return x >= BUTTON_X && x < BUTTON_X + 78 && y >= PANEL_Y_LOC + 6 && y < PANEL_Y_LOC + 36;
}

bool ChessBoard::isDepthButton(int x, int y) const {
  const int BUTTON_X = 155;
  const int PANEL_Y_LOC = 240;
  return x >= BUTTON_X && x < BUTTON_X + 78 && y >= PANEL_Y_LOC + 42 && y < PANEL_Y_LOC + 72;
}

bool ChessBoard::isPlayerFirstButton(int x, int y) const {
  return x >= 28 && x < 116 && y >= 124 && y < 196;
}

bool ChessBoard::isAIFirstButton(int x, int y) const {
  return x >= 124 && x < 212 && y >= 124 && y < 196;
}

bool ChessBoard::isCancelNewGameButton(int x, int y) const {
  return x >= 78 && x < 162 && y >= 210 && y < 240;
}

void ChessBoard::draw(const ChessPosition &position, int selectedSquare,
                      const ChessMove *legalMoves, uint8_t legalMoveCount,
                      const ChessMove &lastMove, uint8_t aiDepth,
                      bool gameOver, bool thinking, bool humanPlaysWhite) {
  drawBoard(position, selectedSquare, legalMoves, legalMoveCount, lastMove,
            humanPlaysWhite);
  drawPanel(position, aiDepth, gameOver, thinking, humanPlaysWhite);
  updateMatrix(position, selectedSquare, legalMoves, legalMoveCount, lastMove,
               humanPlaysWhite);
}


// 2. Cleaned up drawBoard (Removed all 'pixels' logic entirely from here)
void ChessBoard::drawBoard(const ChessPosition &position, int selectedSquare,
                           const ChessMove *legalMoves, uint8_t legalMoveCount,
                           const ChessMove &lastMove, bool humanPlaysWhite) {

  int centerOffset = SQUARE_SIZE / 2;
  int pieceSize = (SQUARE_SIZE * 2) / 5;

  for (int rank = 0; rank < 8; ++rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;
      int displayFile = humanPlaysWhite ? file : 7 - file;
      int displayRank = humanPlaysWhite ? 7 - rank : rank;
      int x = displayFile * SQUARE_SIZE;
      int y = displayRank * SQUARE_SIZE;

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
                              const ChessMove &lastMove, bool humanPlaysWhite) {
  
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
        if (position.whiteToMove == humanPlaysWhite) {
          matrixColor = pixels.Color(255, 0, 0); 
        } else {
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
      drawMatrix(humanPlaysWhite ? file : 7 - file,
                 humanPlaysWhite ? 7 - rank : rank, matrixColor);
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
                           bool gameOver, bool thinking, bool humanPlaysWhite) {
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
    tft->print(position.whiteToMove == humanPlaysWhite ? "YOUR TURN" : "AI TURN");
    if (ChessEngine::isInCheck(position, position.whiteToMove)) {
      tft->setCursor(PANEL_X + 8, PANEL_Y + 42);
      tft->setTextColor(ILI9341_RED);
      tft->print("CHECK!");
    }
  }

  int middleX = PANEL_X + 95;
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(middleX, PANEL_Y + 8);
  tft->print(humanPlaysWhite ? "W: YOU" : "W: AI");
  tft->setCursor(middleX, PANEL_Y + 23);
  tft->print(humanPlaysWhite ? "B: AI" : "B: YOU");

  int buttonX = PANEL_X + 155;
  
  tft->drawRoundRect(buttonX, PANEL_Y + 6, 78, 30, 4, ILI9341_CYAN);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(buttonX + 22, PANEL_Y + 17);
  tft->print("MENU");

  tft->drawRoundRect(buttonX, PANEL_Y + 42, 78, 30, 4, ILI9341_YELLOW);
  tft->setCursor(buttonX + 16, PANEL_Y + 53);
  tft->print("LVL ");
  tft->print(aiDepth);
}

void ChessBoard::drawFirstMoveSelection() {
  const int x = 20;
  const int y = 70;
  const int w = 200;
  const int h = 180;

  tft->fillRoundRect(x, y, w, h, 10, 0x0861);
  tft->drawRoundRect(x, y, w, h, 10, ILI9341_CYAN);
  tft->fillRoundRect(x + 2, y + 2, w - 4, 36, 8, 0x10A2);

  tft->setTextWrap(false);
  tft->setTextSize(1);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(x + 76, y + 15);
  tft->print("NEW GAME");

  tft->setTextColor(0xBDF7);
  tft->setCursor(x + 52, y + 44);
  tft->print("Who moves first?");

  const int cardY = y + 54;
  tft->fillRoundRect(x + 8, cardY, 88, 72, 6, 0x2124);
  tft->drawRoundRect(x + 8, cardY, 88, 72, 6, ILI9341_CYAN);
  ChessGraphics::drawPiece(*tft, 'K', x + 52, cardY + 19, 24);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(x + 34, cardY + 43);
  tft->print("PLAYER");
  tft->setTextColor(ILI9341_CYAN);
  tft->setCursor(x + 37, cardY + 56);
  tft->print("WHITE");

  tft->fillRoundRect(x + 104, cardY, 88, 72, 6, 0x2124);
  tft->drawRoundRect(x + 104, cardY, 88, 72, 6, ILI9341_YELLOW);
  ChessGraphics::drawPiece(*tft, 'k', x + 148, cardY + 19, 24);
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(x + 138, cardY + 43);
  tft->print("AI");
  tft->setTextColor(ILI9341_YELLOW);
  tft->setCursor(x + 133, cardY + 56);
  tft->print("BLACK");

  tft->drawRoundRect(x + 58, y + 140, 84, 30, 5, 0x7BEF);
  tft->setTextColor(0xBDF7);
  tft->setCursor(x + 82, y + 151);
  tft->print("CANCEL");
}
