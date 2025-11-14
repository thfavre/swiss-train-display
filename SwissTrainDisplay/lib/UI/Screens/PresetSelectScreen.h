#ifndef PRESETSELECTSCREEN_H
#define PRESETSELECTSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Data/PresetManager.h"

class PresetSelectScreen : public Screen {
private:
  PresetManager* presets;
  MenuList menuList;
  int selection;

public:
  PresetSelectScreen(DisplayManager* disp, PresetManager* presetMgr);

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;
};

#endif // PRESETSELECTSCREEN_H
