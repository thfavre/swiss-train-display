#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include <map>
#include "../../include/AppState.h"
#include "../UI/Screens/Screen.h"
#include "../UI/Screens/MainScreen.h"
#include "../UI/Screens/MenuScreen.h"
#include "../UI/Screens/SettingsScreen.h"
#include "../UI/Screens/WiFiScanScreen.h"
#include "../UI/Screens/PasswordEntryScreen.h"
#include "../UI/Screens/PresetEditScreen.h"
#include "../UI/Screens/PresetSelectScreen.h"
#include "../UI/Screens/ErrorScreen.h"

#include "../UI/DisplayManager.h"
#include "../Input/EncoderHandler.h"
#include "../Input/ButtonHandler.h"
#include "../Data/PresetManager.h"
#include "../Data/TrainAPI.h"
#include "../Network/WiFiManager.h"
#include "../Storage/SettingsManager.h"

class StateMachine {
private:
  // Managers
  DisplayManager* display;
  EncoderHandler* encoder;
  ButtonHandler* button;
  PresetManager* presets;
  TrainAPI* trainAPI;
  WiFiManager* wifi;
  SettingsManager* settings;

  // Screens
  std::map<AppState, Screen*> screens;
  AppState currentState;
  Screen* currentScreen;

  // Context data (shared between screens)
  String selectedSSID;
  int selectedNetworkIndex;

public:
  StateMachine(DisplayManager* disp, EncoderHandler* enc, ButtonHandler* btn,
               PresetManager* presetMgr, TrainAPI* api,
               WiFiManager* wifiMgr, SettingsManager* settingsMgr);
  ~StateMachine();

  // Initialize and register all screens
  void begin();

  // Main update loop
  void update();

  // Manual state transition
  void setState(AppState newState);

  // Getters
  AppState getCurrentState() const { return currentState; }
  Screen* getCurrentScreen() const { return currentScreen; }

  // Context data access
  void setSelectedSSID(const String& ssid) { selectedSSID = ssid; }
  String getSelectedSSID() const { return selectedSSID; }
};

#endif // STATEMACHINE_H
