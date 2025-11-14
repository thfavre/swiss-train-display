#include "WiFiScanScreen.h"

WiFiScanScreen::WiFiScanScreen(DisplayManager* disp, WiFiManager* wifiMgr, SettingsManager* settingsMgr)
  : Screen(disp), wifi(wifiMgr), settings(settingsMgr), selection(0), scanning(false) {
}

void WiFiScanScreen::enter() {
  Serial.println("Entering WiFiScanScreen");
  selection = 0;
  scanning = true;

  // Perform scan
  wifi->scan();
  scanning = false;

  menuList.setSelected(0);
}

void WiFiScanScreen::exit() {}
void WiFiScanScreen::update() {}

void WiFiScanScreen::handleEncoder(int delta) {
  if (delta != 0 && !scanning) {
    selection += delta;
    int count = wifi->getNetworkCount();
    if (selection < 0) selection = count - 1;
    if (selection >= count) selection = 0;
    menuList.setSelected(selection);
  }
}

void WiFiScanScreen::handleShortPress() {
  if (!scanning && wifi->getNetworkCount() > 0) {
    // Store selected network in a shared context (for now, just transition)
    requestState(STATE_WIFI_PASSWORD);
  }
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
  } else {
    // Show networks
    const std::vector<WiFiNetwork>& networks = wifi->getNetworks();
    String items[networks.size()];
    for (size_t i = 0; i < networks.size(); i++) {
      items[i] = networks[i].ssid;
    }
    menuList.draw(*display, items, networks.size(), BLUE_ZONE_Y + 2);
  }

  display->show();
}
