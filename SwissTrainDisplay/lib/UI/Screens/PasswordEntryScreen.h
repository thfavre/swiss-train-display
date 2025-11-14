#ifndef PASSWORDENTRYSCREEN_H
#define PASSWORDENTRYSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Network/WiFiManager.h"
#include "../../Storage/SettingsManager.h"

class PasswordEntryScreen : public Screen {
private:
  WiFiManager* wifi;
  SettingsManager* settings;
  String password;
  int charIndex;
  bool showModal;
  int modalSelection;
  String ssid;  // Store SSID from WiFiScanScreen

public:
  PasswordEntryScreen(DisplayManager* disp, WiFiManager* wifiMgr, SettingsManager* settingsMgr);

  void setSSID(const String& networkSSID) { ssid = networkSSID; }

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;
};

#endif // PASSWORDENTRYSCREEN_H
