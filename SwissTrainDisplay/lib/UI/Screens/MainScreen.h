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

  // Multi-train display layouts
  void drawSingleTrain(const TrainConnection& conn);
  void drawTwoTrains(const std::vector<TrainConnection>& connections);
  void drawThreeTrains(const std::vector<TrainConnection>& connections);
  void drawFourTrains(const std::vector<TrainConnection>& connections);

  // Helper functions
  String getCurrentTime();
  String calculateDuration(const String& departureTime, const String& arrivalTime);

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
