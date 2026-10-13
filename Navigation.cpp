#include "Navigation.h"
#include "UI.h"

Screen *Navigation::history[Navigation::MAX_HISTORY];
uint8_t Navigation::historySize = 0;

void Navigation::setRoot(Screen &screen) {
  historySize = 0;
  UI::setCurrentScreen(&screen);
}

bool Navigation::navigateTo(Screen &screen) {
  Screen *current = UI::currentScreen();
  if (current == &screen) return true;
  if (historySize >= MAX_HISTORY) return false;

  if (current) history[historySize++] = current;
  UI::setCurrentScreen(&screen);
  return true;
}

bool Navigation::goBack() {
  if (historySize == 0) return false;

  UI::setCurrentScreen(history[--historySize]);
  return true;
}
