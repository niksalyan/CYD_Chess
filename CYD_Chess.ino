#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#include "GameScreen.h"
#include "Navigation.h"
#include "UI.h"

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST -1

static const int TOUCH_X_MIN = 200;
static const int TOUCH_X_MAX = 3900;
static const int TOUCH_Y_MIN = 200;
static const int TOUCH_Y_MAX = 3900;
static const bool TOUCH_SWAP_XY = false;
static const bool TOUCH_INVERT_X = false;
static const bool TOUCH_INVERT_Y = false;

SPIClass touchSPI = SPIClass(VSPI);
SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft(&tftSPI, TFT_DC, TFT_CS, TFT_RST);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);
GameScreen gameScreen;

static bool previousTouch = false;
static uint32_t lastTouchMillis = 0;

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

  gameScreen.begin(tft);
  Navigation::setRoot(gameScreen);
  UI::draw();
}

void loop() {
  int x, y;
  bool isTouched = readTouchPoint(x, y);
  uint32_t now = millis();

  if (isTouched && !previousTouch && now - lastTouchMillis > 180) {
    lastTouchMillis = now;
    if (UI::handleTouch(x, y)) UI::draw();
  }
  previousTouch = isTouched;

  if (UI::update()) UI::draw();
  delay(15);
}
