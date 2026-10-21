#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <XPT2046_Touchscreen.h>

#include "GameScreen.h"
#include "MenuScreen.h"
#include "Navigation.h"
#include "SplashScreen.h"
#include "TouchCalibrationScreen.h"
#include "UI.h"

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST -1
#define TFT_BL 21
#define TFT_SCLK 14
#define TFT_MISO 12
#define TFT_MOSI 13

static const int TOUCH_X_MIN = 200;
static const int TOUCH_X_MAX = 3900;
static const int TOUCH_Y_MIN = 200;
static const int TOUCH_Y_MAX = 3900;
static const bool TOUCH_SWAP_XY = false;
static const bool TOUCH_INVERT_X = false;
static const bool TOUCH_INVERT_Y = false;

SPIClass touchSPI(VSPI);
SPIClass tftSPI(HSPI);
Adafruit_ST7789 tft(&tftSPI, TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);
GameScreen gameScreen;

static bool previousTouch = false;
static uint32_t lastTouchMillis = 0;

static void setRotationWithHorizontalFlip(uint8_t rotation) {
  tft.setRotation(rotation);

  uint8_t madctl;
  switch (rotation & 3) {
    case 0:  madctl = 0xC0; break;
    case 1:  madctl = 0xA0; break;
    case 2:  madctl = 0x00; break;
    default: madctl = 0x60; break;
  }

  // Toggle horizontal address direction for this CYD panel.
  madctl ^= 0x40;
  tft.sendCommand(0x36, &madctl, 1);
}

static int mapTouchAxis(int value, int low, int high, int extent, bool invert) {
  if (high <= low) return 0;
  long mapped = (long)(value - low) * (extent - 1) / (high - low);
  mapped = constrain(mapped, 0, extent - 1);
  return invert ? extent - 1 - (int)mapped : (int)mapped;
}

static bool readTouchPoint(TouchPoint &touchPoint) {
  if (!touch.touched()) return false;
  TS_Point point = touch.getPoint();
  touchPoint.rawX = TOUCH_SWAP_XY ? point.y : point.x;
  touchPoint.rawY = TOUCH_SWAP_XY ? point.x : point.y;
  if (!TouchCalibrationScreen::instance().mapRawPoint(
        touchPoint.rawX, touchPoint.rawY, touchPoint.x, touchPoint.y)) {
    touchPoint.x = mapTouchAxis(touchPoint.rawX, TOUCH_X_MIN, TOUCH_X_MAX,
                                tft.width(), TOUCH_INVERT_X);
    touchPoint.y = mapTouchAxis(touchPoint.rawY, TOUCH_Y_MIN, TOUCH_Y_MAX,
                                tft.height(), TOUCH_INVERT_Y);
  }
  return true;
}

static bool enterCalibrationIfHeldAtStartup() {
  if (!touch.touched()) return false;

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(22, 120);
  tft.print("Hold for touch");
  tft.setCursor(56, 148);
  tft.print("calibration");

  uint32_t start = millis();
  while (touch.touched() && millis() - start < 1500) delay(10);
  if (!touch.touched()) return false;

  TouchCalibrationScreen::instance().start();
  if (!Navigation::navigateTo(TouchCalibrationScreen::instance()))
    return false;

  while (touch.touched()) delay(10);
  return true;
}

void setup() {
  Serial.begin(115200);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tftSPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  touch.setRotation(0);

  tft.init(240, 320);
  tft.setRotation(2);
  //setRotationWithHorizontalFlip(2);
  tft.invertDisplay(false);

  gameScreen.begin(tft);
  MenuScreen::instance().begin(tft, gameScreen);
  TouchCalibrationScreen::instance().begin(tft);
  Navigation::setRoot(gameScreen);
  bool calibrationRequested = enterCalibrationIfHeldAtStartup();
  if (!calibrationRequested) {
    SplashScreen::instance().begin(tft, gameScreen);
    Navigation::setRoot(SplashScreen::instance());
  }
  UI::draw();
}

void loop() {
  TouchPoint point;
  bool isTouched = readTouchPoint(point);
  uint32_t now = millis();

  if (isTouched && !previousTouch && now - lastTouchMillis > 180) {
    lastTouchMillis = now;
    if (UI::handleTouch(point)) UI::draw();
  }
  previousTouch = isTouched;

  if (UI::update()) UI::draw();
  delay(15);
}
