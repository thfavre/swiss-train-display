#ifndef MENUSCREEN_H
#define MENUSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Network/WiFiManager.h"

class MenuScreen : public Screen {
private:
  WiFiManager* wifi;
  MenuList menuList;
  int selection;

  static const int MENU_ITEM_COUNT = 4;
  String menuItems[MENU_ITEM_COUNT];

public:
  MenuScreen(DisplayManager* disp, WiFiManager* wifiMgr);

  void enter() override;
  void exit() override;
  void update() override;

  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;

  void draw() override;
};

#endif // MENUSCREEN_H
