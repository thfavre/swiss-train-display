#ifndef MAINSCREEN_H
#define MAINSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Data/PresetManager.h"
#include "../../Data/TrainAPI.h"
#include "../../Network/WiFiManager.h"

class MainScreen : public Screen {
private:
  PresetManager* presets;
  TrainAPI* trainAPI;
  WiFiManager* wifi;

  void drawTrainDisplay();
  void drawClockDisplay();
  void drawWeatherDisplay();
  void drawCalendarDisplay();

  String getCurrentTime();

public:
  MainScreen(DisplayManager* disp, PresetManager* presetMgr, TrainAPI* api, WiFiManager* wifiMgr);

  void enter() override;
  void exit() override;
  void update() override;

  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;

  void draw() override;
};

#endif // MAINSCREEN_H
