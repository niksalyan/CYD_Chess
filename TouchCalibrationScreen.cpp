#include "TouchCalibrationScreen.h"

#include <Preferences.h>

#include "Navigation.h"

namespace {
  const int TARGET_MARGIN = 24;
  const uint16_t SCREEN_BACKGROUND = 0x18E3;

  struct CalibrationData {
    int leftX;
    int rightX;
    int topY;
    int bottomY;
  };

  CalibrationData currentCalibration = { 200, 3900, 200, 3900 };

  int mapAxis(int raw, int start, int end, int extent) {
    if (extent <= TARGET_MARGIN * 2 || start == end) return 0;
    long mapped = TARGET_MARGIN +
      (long)(raw - start) * (extent - 1 - TARGET_MARGIN * 2) / (end - start);
    return constrain((int)mapped, 0, extent - 1);
  }
}

TouchCalibrationScreen::TouchCalibrationScreen()
  : display(0),
    rawXs{0, 0, 0, 0},
    rawYs{0, 0, 0, 0},
    targetIndex(0),
    calibrationValid(false),
    storageError(false) {
}

TouchCalibrationScreen &TouchCalibrationScreen::instance() {
  static TouchCalibrationScreen screen;
  return screen;
}

void TouchCalibrationScreen::begin(Adafruit_ST7789 &tft) {
  display = &tft;

  Preferences preferences;
  if (!preferences.begin("touch", true)) return;
  bool valid = preferences.getBool("valid", false);
  if (valid) {
    currentCalibration.leftX = preferences.getInt("leftX", 200);
    currentCalibration.rightX = preferences.getInt("rightX", 3900);
    currentCalibration.topY = preferences.getInt("topY", 200);
    currentCalibration.bottomY = preferences.getInt("bottomY", 3900);
    calibrationValid =
      abs(currentCalibration.rightX - currentCalibration.leftX) >= 300 &&
      abs(currentCalibration.bottomY - currentCalibration.topY) >= 300;
  }
  preferences.end();
}

void TouchCalibrationScreen::start() {
  targetIndex = 0;
  storageError = false;
  for (uint8_t i = 0; i < 4; ++i) {
    rawXs[i] = 0;
    rawYs[i] = 0;
  }
}

bool TouchCalibrationScreen::mapRawPoint(
  int rawX, int rawY, int &x, int &y) const {
  if (!calibrationValid) return false;
  x = mapAxis(rawX, currentCalibration.leftX,
              currentCalibration.rightX, display->width());
  y = mapAxis(rawY, currentCalibration.topY,
              currentCalibration.bottomY, display->height());
  return true;
}

bool TouchCalibrationScreen::saveCalibration() {
  CalibrationData next = {
    (rawXs[0] + rawXs[3]) / 2,
    (rawXs[1] + rawXs[2]) / 2,
    (rawYs[0] + rawYs[1]) / 2,
    (rawYs[2] + rawYs[3]) / 2
  };

  if (abs(next.rightX - next.leftX) < 300 ||
      abs(next.bottomY - next.topY) < 300)
    return false;

  Preferences preferences;
  if (!preferences.begin("touch", false)) return false;
  bool saved = preferences.putBool("valid", false);
  saved = saved &&
    preferences.putInt("leftX", next.leftX) == sizeof(int32_t) &&
    preferences.putInt("rightX", next.rightX) == sizeof(int32_t) &&
    preferences.putInt("topY", next.topY) == sizeof(int32_t) &&
    preferences.putInt("bottomY", next.bottomY) == sizeof(int32_t);
  if (saved) saved = preferences.putBool("valid", true);
  preferences.end();

  if (saved) {
    currentCalibration = next;
    calibrationValid = true;
  }
  return saved;
}

void TouchCalibrationScreen::draw() {
  if (!display) return;

  display->fillScreen(SCREEN_BACKGROUND);
  display->setTextWrap(false);
  display->setTextSize(2);
  display->setTextColor(ST77XX_WHITE);
  display->setCursor(24, 16);
  display->print("TOUCH CALIBRATION");

  display->setTextSize(1);
  display->setCursor(26, 49);
  if (storageError) {
    display->setTextColor(ST77XX_RED);
    display->print("Invalid samples or save failed. Retry.");
  } else if (targetIndex < 4) {
    display->setTextColor(ST77XX_WHITE);
    display->print("Tap the center of each target.");
  } else {
    display->setTextColor(ST77XX_GREEN);
    display->print("Calibration saved.");
  }

  if (targetIndex < 4) {
    const int targetX[4] = {
      TARGET_MARGIN, display->width() - 1 - TARGET_MARGIN,
      display->width() - 1 - TARGET_MARGIN, TARGET_MARGIN
    };
    const int targetY[4] = {
      TARGET_MARGIN, TARGET_MARGIN,
      display->height() - 1 - TARGET_MARGIN,
      display->height() - 1 - TARGET_MARGIN
    };
    int x = targetX[targetIndex];
    int y = targetY[targetIndex];
    display->drawCircle(x, y, 12, ST77XX_YELLOW);
    display->drawFastHLine(x - 18, y, 37, ST77XX_YELLOW);
    display->drawFastVLine(x, y - 18, 37, ST77XX_YELLOW);
  }
}

bool TouchCalibrationScreen::handleTouch(const TouchPoint &point) {
  if (targetIndex >= 4) return false;

  rawXs[targetIndex] = point.rawX;
  rawYs[targetIndex] = point.rawY;
  ++targetIndex;

  if (targetIndex == 4 && !saveCalibration()) {
    storageError = true;
    targetIndex = 0;
  } else if (targetIndex == 4) {
    return Navigation::goBack();
  }
  return true;
}
