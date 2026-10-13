#ifndef TOUCH_CALIBRATION_SCREEN_H
#define TOUCH_CALIBRATION_SCREEN_H

#include <Adafruit_ILI9341.h>

#include "Screen.h"

class TouchCalibrationScreen : public Screen {
public:
  static TouchCalibrationScreen &instance();
  void begin(Adafruit_ILI9341 &display);
  void start();
  void draw();
  bool handleTouch(const TouchPoint &point);

  bool mapRawPoint(int rawX, int rawY, int &x, int &y) const;

private:
  TouchCalibrationScreen();
  bool saveCalibration();

  Adafruit_ILI9341 *display;
  int rawXs[4];
  int rawYs[4];
  uint8_t targetIndex;
  bool calibrationValid;
  bool storageError;
};

#endif
