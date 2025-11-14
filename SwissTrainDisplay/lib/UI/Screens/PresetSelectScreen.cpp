#include "PresetSelectScreen.h"

PresetSelectScreen::PresetSelectScreen(DisplayManager* disp, PresetManager* presetMgr)
  : Screen(disp), presets(presetMgr), selection(0) {
}

void PresetSelectScreen::enter() {
  Serial.println("Entering PresetSelectScreen");
  selection = presets->getCurrentIndex();
  menuList.setSelected(selection);
}

void PresetSelectScreen::exit() {}
void PresetSelectScreen::update() {}

void PresetSelectScreen::handleEncoder(int delta) {
  if (delta != 0) {
    selection += delta;
    int count = presets->getCount();
    if (selection < 0) selection = count - 1;
    if (selection >= count) selection = 0;
    menuList.setSelected(selection);
  }
}

void PresetSelectScreen::handleShortPress() {
  // Switch to selected preset
  presets->setCurrentIndex(selection);
  requestState(STATE_MAIN_DISPLAY);
}

void PresetSelectScreen::handleLongPress() {
  requestState(STATE_MENU);
}

void PresetSelectScreen::draw() {
  display->clear();
  YellowBar::draw(*display, "Select Preset");

  int count = presets->getCount();
  String items[count];

  for (int i = 0; i < count; i++) {
    const Preset* p = presets->getPreset(i);
    if (p) {
      items[i] = p->name;
    }
  }

  menuList.draw(*display, items, count, BLUE_ZONE_Y + 2);
  display->show();
}
