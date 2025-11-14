#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"

class SettingsScreen : public Screen {
private:
  MenuList menuList;
  int selection;
  static const int MENU_ITEM_COUNT = 2;
  String menuItems[MENU_ITEM_COUNT];

public:
  SettingsScreen(DisplayManager* disp);

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;
};

#endif // SETTINGSSCREEN_H
