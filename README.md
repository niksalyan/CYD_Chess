# CYD Chess

CYD Chess is a touchscreen chess game for an ESP32-based Cheap Yellow Display
(CYD). It renders a chessboard on the TFT, plays against an onboard chess AI,
and mirrors the board on a 16x16 addressable LED matrix.

## Features

- Play chess against a minimax AI with alpha-beta pruning.
- Choose whether the player or AI moves first. The board rotates so the
  player's pieces are at the bottom.
- Select the AI search depth from the game screen.
- View the current game's move history in coordinate notation, with paging.
- Recalibrate touchscreen coordinates and save calibration across reboots.
- Enter calibration at startup by holding the touchscreen if normal touch
  input is too inaccurate to use the menu.
- View a branded startup splash screen; tap to skip it.
- Navigate between screens through the shared UI and navigation system.

## Hardware

The pin assignments are defined near the top of `CYD_Chess.ino` and in
`ChessBoard.h`.

| Component | Signal | ESP32 GPIO |
| --- | --- | ---: |
| ILI9341 TFT | Chip select | 15 |
| ILI9341 TFT | Data/command | 2 |
| ILI9341 TFT | Reset | Not connected (`-1`) |
| TFT backlight | Enable | 21 |
| XPT2046 touch | Chip select | 33 |
| XPT2046 touch | IRQ | 36 |
| XPT2046 touch | MOSI | 32 |
| XPT2046 touch | MISO | 39 |
| XPT2046 touch | Clock | 25 |
| 16x16 NeoPixel matrix | Data | 22 |

The TFT and touchscreen use separate SPI buses. The LED matrix is configured
as a GRB, 800 kHz NeoPixel chain. The chessboard maps each of its 64 squares
onto a 2x2 group of LEDs.

Check your display and matrix wiring before powering the hardware. Addressable
LED matrices can draw substantial current; use a suitable power supply and
common ground, and do not power a large matrix from the ESP32's 3.3 V pin.

## Software requirements

- Arduino IDE or another Arduino-compatible build workflow.
- ESP32 Arduino board support package.
- Libraries:
  - Adafruit GFX Library
  - Adafruit ILI9341
  - Adafruit NeoPixel
  - XPT2046_Touchscreen
- The ESP32 Arduino core's `Preferences` library is used to save calibration
  data in non-volatile storage.

Select the ESP32 board profile matching your CYD and the serial port for your
device. Open `CYD_Chess.ino`, compile, and upload. The project's `.cpp` and
`.h` files should remain alongside the sketch so the Arduino build can include
them.

## Controls

The splash screen appears briefly at startup. Tap anywhere to continue
immediately to the game.

### Game screen

- Tap a piece, then tap one of its highlighted legal destinations to move it.
- Tap **LVL** to cycle the AI search depth from 2 through 4.
- Tap **MENU** for game actions and settings.
- The status panel shows whose turn it is, check/game status, and which color
  is controlled by the player and AI.

### Menu

- **NEW GAME** starts a fresh game and asks whether the player or AI should
  move first.
- **MOVE HISTORY** opens the current game's move list.
- **TOUCH CALIBRATION** opens the four-target touchscreen calibration.
- **BACK** returns to the previous screen.

### Move history

Moves are displayed in coordinate notation, such as `e2-e4`. Each row pairs
White's and Black's move for one turn. Use the arrow buttons to page through
longer games; **BACK** returns to the previous screen. Starting a new game
clears the history. Up to 512 plies are stored in memory for the current game.

### Touch calibration

Open **MENU > TOUCH CALIBRATION** and tap the center of each target as it
appears. The calibration is saved across reboots. If the touchscreen is too
far off to reach the menu:

1. Restart the device.
2. Touch and hold the screen during startup.
3. Keep holding through the on-screen prompt. Calibration opens after the
   hold is detected.
4. Release, then tap the four targets.

Touch calibration corrects the raw X/Y range. If your panel's axes are swapped
or inverted, adjust `TOUCH_SWAP_XY`, `TOUCH_INVERT_X`, or `TOUCH_INVERT_Y` in
`CYD_Chess.ino` and upload again.

## Project structure

| File | Responsibility |
| --- | --- |
| `CYD_Chess.ino` | Hardware setup, touch sampling, and the main UI loop |
| `Screen.h` | Base screen interface and normalized/raw touch event |
| `UI.h`, `UI.cpp` | Static dispatch to the active screen |
| `Navigation.h`, `Navigation.cpp` | Screen navigation and back stack |
| `GameScreen.h`, `GameScreen.cpp` | Chess state, player input, AI turns, and move history |
| `ChessEngine.h`, `ChessEngine.cpp` | Chess position, legal moves, and move application |
| `AIEngine.h`, `AIEngine.cpp` | AI search and position evaluation |
| `ChessBoard.h`, `ChessBoard.cpp` | TFT and LED board rendering and game-panel controls |
| `ChessGraphics.h`, `ChessGraphics.cpp` | Chess piece rendering |
| `ChessPieceAtlas.h` | Generated RGB565 piece sprites stored in program memory |
| `MenuScreen.h`, `MenuScreen.cpp` | Main menu |
| `MoveHistoryScreen.h`, `MoveHistoryScreen.cpp` | Move-history screen |
| `TouchCalibrationScreen.h`, `TouchCalibrationScreen.cpp` | Touch calibration and persistent calibration data |
| `SplashScreen.h`, `SplashScreen.cpp` | Startup splash screen |
| `tools/generate_chess_sprites.py` | Regenerates the piece atlas from the sprite sheet |
| `mat/` | 3D models and printable enclosure/board parts |

New screens implement `Screen::draw()` and `Screen::handleTouch()`. The active
screen receives drawing, touch, and optional update calls through `UI`; use
`Navigation` to change screens.

## Regenerating the piece atlas

The generated atlas is already included. To regenerate it after changing
`Chess_Pieces_Sprite.png`, install Python and Pillow, then run from the project
root:

```sh
python -m pip install Pillow
python tools/generate_chess_sprites.py
```

The generator expects a 180x60 PNG containing 12 30x30 piece sprites.

## Current limitations

- Move history is kept in RAM and is cleared when a new game starts or the
  device restarts. It is not a full PGN/SAN recorder.
- The AI plays one move at a time on the main loop; deeper search levels take
  longer to respond.
- There is no persistent game-save or resume feature.
