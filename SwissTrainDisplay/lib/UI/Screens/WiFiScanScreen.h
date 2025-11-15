#ifndef WIFISCANSCREEN_H
#define WIFISCANSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Network/WiFiManager.h"
#include "../../Storage/SettingsManager.h"

class WiFiScanScreen : public Screen {
private:
  WiFiManager* wifi;
  SettingsManager* settings;
  MenuList menuList;
  int selection;
  bool scanning;

  void performScan();
  int getTotalMenuItems() const;

public:
  WiFiScanScreen(DisplayManager* disp, WiFiManager* wifiMgr, SettingsManager* settingsMgr);

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;

  // Getters
  int getSelected() const { return selection; }
};

#endif // WIFISCANSCREEN_H
