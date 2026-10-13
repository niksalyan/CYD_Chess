#include "UI.h"

Screen *UI::activeScreen = 0;

Screen *UI::currentScreen() {
  return activeScreen;
}

void UI::setCurrentScreen(Screen *screen) {
  activeScreen = screen;
}

void UI::draw() {
  if (activeScreen) activeScreen->draw();
}

bool UI::handleTouch(int x, int y) {
  return activeScreen ? activeScreen->handleTouch(x, y) : false;
}

bool UI::update() {
  return activeScreen ? activeScreen->update() : false;
}
