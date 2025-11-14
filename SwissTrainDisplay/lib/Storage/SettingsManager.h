#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "../../include/Config.h"
#include "../../include/Types.h"

class SettingsManager {
private:
  Preferences prefs;
  bool initialized;

public:
  SettingsManager();
  ~SettingsManager();

  // Initialize storage
  bool begin();
  void end();

  // WiFi settings
  bool saveWiFiCredentials(const String& ssid, const String& password);
  bool loadWiFiCredentials(String& ssid, String& password);
  bool clearWiFiCredentials();

  // Preset management
  bool savePreset(int index, const Preset& preset);
  bool loadPreset(int index, Preset& preset);
  bool deletePreset(int index);
  int getPresetCount();
  bool setPresetCount(int count);

  // Current state
  bool saveCurrentPreset(int index);
  int loadCurrentPreset();

  // Utility
  bool clearAll();
  bool isInitialized() const { return initialized; }
};

#endif // SETTINGSMANAGER_H
