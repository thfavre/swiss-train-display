#include "SettingsScreen.h"

SettingsScreen::SettingsScreen(DisplayManager* disp)
  : Screen(disp), selection(0) {
  menuItems[0] = "WiFi Setup";
  menuItems[1] = "< Back";
}

void SettingsScreen::enter() {
  selection = 0;
  menuList.setSelected(0);
}

void SettingsScreen::exit() {}
void SettingsScreen::update() {}

void SettingsScreen::handleEncoder(int delta) {
  if (delta != 0) {
    selection += delta;
    if (selection < 0) selection = MENU_ITEM_COUNT - 1;
    if (selection >= MENU_ITEM_COUNT) selection = 0;
    menuList.setSelected(selection);
  }
}

void SettingsScreen::handleShortPress() {
  if (selection == SETTINGS_WIFI) {
    requestState(STATE_WIFI_SCAN);
  } else {
    requestState(STATE_MENU);
  }
}

void SettingsScreen::handleLongPress() {
  requestState(STATE_MENU);
}

void SettingsScreen::draw() {
  display->clear();
  YellowBar::drawWithTime(*display, "SETTINGS");
  menuList.draw(*display, menuItems, MENU_ITEM_COUNT, BLUE_ZONE_Y + 2);
  display->show();
}
