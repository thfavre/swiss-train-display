#include "ErrorScreen.h"

ErrorScreen::ErrorScreen(DisplayManager* disp)
  : Screen(disp), displayTime(0) {
}

void ErrorScreen::enter() {
  Serial.println("Entering ErrorScreen");
  displayTime = millis();
}

void ErrorScreen::exit() {}

void ErrorScreen::update() {
  // Auto-dismiss after 5 seconds
  if (millis() - displayTime > 5000) {
    requestState(STATE_MAIN_DISPLAY);
  }
}

void ErrorScreen::handleEncoder(int delta) {
  // Ignore
}

void ErrorScreen::handleShortPress() {
  // Dismiss error
  requestState(STATE_MAIN_DISPLAY);
}

void ErrorScreen::handleLongPress() {
  // Dismiss error
  requestState(STATE_MAIN_DISPLAY);
}

void ErrorScreen::draw() {
  display->clear();

  // Yellow zone: ERROR title with icon
  YellowBar::draw(*display, "ERROR");
  Icons::drawError(*display, 115, 8);

  // Blue zone: Error message
  display->drawText(error.message, 4, 20, 1);

  if (error.detail.length() > 0) {
    display->drawText(error.detail.substring(0, 20), 4, 32, 1);
  }

  display->drawText("Press any button", 4, 50, 1);

  display->show();
}
