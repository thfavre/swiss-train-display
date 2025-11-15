#include "WiFiScanScreen.h"

WiFiScanScreen::WiFiScanScreen(DisplayManager* disp, WiFiManager* wifiMgr, SettingsManager* settingsMgr)
  : Screen(disp), wifi(wifiMgr), settings(settingsMgr), selection(0), scanning(false) {
}

void WiFiScanScreen::enter() {
  Serial.println("Entering WiFiScanScreen");
  selection = 0;
  scanning = true;

  // Show "Scanning..." immediately
  draw();

  // Perform scan
  wifi->scan();
  scanning = false;

  menuList.setSelected(0);
  requestRedraw();
}

void WiFiScanScreen::exit() {}
void WiFiScanScreen::update() {}

void WiFiScanScreen::handleEncoder(int delta) {
  if (delta != 0 && !scanning) {
    selection += delta;
    int totalItems = getTotalMenuItems();
    if (selection < 0) selection = totalItems - 1;
    if (selection >= totalItems) selection = 0;
    menuList.setSelected(selection);
  }
}

int WiFiScanScreen::getTotalMenuItems() const {
  // Networks + Refresh + Back
  return wifi->getNetworkCount() + 2;
}

void WiFiScanScreen::handleShortPress() {
  if (scanning) return;

  int networkCount = wifi->getNetworkCount();

  if (selection < networkCount) {
    // Network selected - go to password entry
    if (networkCount > 0) {
      requestState(STATE_WIFI_PASSWORD);
    }
  } else if (selection == networkCount) {
    // "Refresh" selected - rescan
    performScan();
  } else if (selection == networkCount + 1) {
    // "< Back" selected - go back to settings
    requestState(STATE_SETTINGS);
  }
}

void WiFiScanScreen::performScan() {
  selection = 0;
  scanning = true;
  draw();

  wifi->scan();
  scanning = false;

  menuList.setSelected(0);
  requestRedraw();
}

void WiFiScanScreen::handleLongPress() {
  requestState(STATE_SETTINGS);
}

void WiFiScanScreen::draw() {
  display->clear();
  YellowBar::draw(*display, "WiFi Networks");

  if (scanning) {
    display->drawCenteredText("Scanning...", 30, 1);
  } else if (wifi->getNetworkCount() == 0) {
    display->drawCenteredText("No networks found", 30, 1);

    // Show bottom menu options even when no networks
    String bottomItems[] = {"Refresh", "< Back"};
    menuList.draw(*display, bottomItems, 2, BLUE_ZONE_Y + 2);
  } else {
    // Show networks + bottom menu options
    const std::vector<WiFiNetwork>& networks = wifi->getNetworks();
    int totalItems = networks.size() + 2;
    String items[totalItems];

    // Add network names
    for (size_t i = 0; i < networks.size(); i++) {
      items[i] = networks[i].ssid;
    }

    // Add bottom menu options
    items[networks.size()] = "Refresh";
    items[networks.size() + 1] = "< Back";

    menuList.draw(*display, items, totalItems, BLUE_ZONE_Y + 2);
  }

  display->show();
}
