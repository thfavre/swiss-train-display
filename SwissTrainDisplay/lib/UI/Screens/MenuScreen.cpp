#include "MenuScreen.h"

MenuScreen::MenuScreen(DisplayManager* disp, WiFiManager* wifiMgr, PresetManager* presetMgr, TrainAPI* api)
  : Screen(disp), wifi(wifiMgr), presets(presetMgr), trainAPI(api), selection(0) {

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
      performRefresh();
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

void MenuScreen::performRefresh() {
  const Preset* current = presets->getCurrent();

  // Only refresh if current preset is a train
  if (!current || current->type != PRESET_TRAIN) {
    Serial.println("Current preset is not a train - nothing to refresh");
    return;
  }

  // Check WiFi connection
  if (!wifi->isConnected()) {
    Serial.println("No WiFi connection - cannot refresh");
    display->clear();
    display->drawCenteredText("No WiFi", 30, 1);
    display->show();
    delay(1500);
    return;
  }

  // Show loading message
  display->clear();
  display->drawCenteredText("Refreshing...", 28, 1);
  display->show();

  // Fetch train data with the correct number of trains
  std::vector<TrainConnection> connections;
  int limit = current->trainsToDisplay;

  Serial.printf("Refreshing train data: %s -> %s (limit: %d)\n",
                current->fromStation.c_str(), current->toStation.c_str(), limit);

  bool success = trainAPI->fetchConnections(current->fromStation, current->toStation, connections, limit);

  if (success) {
    Serial.printf("Refresh successful - fetched %d connections\n", connections.size());
  } else {
    Serial.println("Refresh failed");
    display->clear();
    display->drawCenteredText("Refresh failed", 28, 1);
    display->show();
    delay(1500);
  }
}
