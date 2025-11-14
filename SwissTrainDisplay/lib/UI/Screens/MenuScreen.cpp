#include "MenuScreen.h"

MenuScreen::MenuScreen(DisplayManager* disp, WiFiManager* wifiMgr)
  : Screen(disp), wifi(wifiMgr), selection(0) {

  menuItems[0] = "Settings";
  menuItems[1] = "Presets";
  menuItems[2] = "Refresh";
  menuItems[3] = "< Back";
}

void MenuScreen::enter() {
  Serial.println("Entering MenuScreen");
  selection = 0;
  menuList.setSelected(0);
}

void MenuScreen::exit() {
  Serial.println("Exiting MenuScreen");
}

void MenuScreen::update() {
  // Nothing to update
}

void MenuScreen::handleEncoder(int delta) {
  if (delta != 0) {
    selection += delta;

    // Wrap around
    if (selection < 0) selection = MENU_ITEM_COUNT - 1;
    if (selection >= MENU_ITEM_COUNT) selection = 0;

    menuList.setSelected(selection);
  }
}

void MenuScreen::handleShortPress() {
  switch (selection) {
    case MENU_SETTINGS:
      requestState(STATE_SETTINGS);
      break;

    case MENU_PRESETS:
      requestState(STATE_PRESET_SELECT);
      break;

    case MENU_REFRESH:
      // Just go back to main and it will auto-refresh
      requestState(STATE_MAIN_DISPLAY);
      break;

    case MENU_EXIT:
      requestState(STATE_MAIN_DISPLAY);
      break;
  }
}

void MenuScreen::handleLongPress() {
  // Long press = back
  requestState(STATE_MAIN_DISPLAY);
}

void MenuScreen::draw() {
  display->clear();

  // Yellow zone: Title
  YellowBar::drawWithTime(*display, "MAIN MENU");

  // Blue zone: Menu items
  menuList.draw(*display, menuItems, MENU_ITEM_COUNT, BLUE_ZONE_Y + 2);

  display->show();
}
