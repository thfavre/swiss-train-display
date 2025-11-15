#include "SettingsManager.h"

SettingsManager::SettingsManager() : initialized(false) {
}

SettingsManager::~SettingsManager() {
  if (initialized) {
    end();
  }
}

bool SettingsManager::begin() {
  if (initialized) {
    return true;
  }

  initialized = prefs.begin(PREFS_NAMESPACE, false);
  if (!initialized) {
    Serial.println("ERROR: Failed to initialize Preferences");
  }
  return initialized;
}

void SettingsManager::end() {
  if (initialized) {
    prefs.end();
    initialized = false;
  }
}

// ====== WIFI SETTINGS ======

bool SettingsManager::saveWiFiCredentials(const String& ssid, const String& password) {
  if (!initialized && !begin()) {
    return false;
  }

  bool success = true;
  success &= prefs.putString(PREFS_KEY_SSID, ssid) > 0;
  success &= prefs.putString(PREFS_KEY_PASSWORD, password) > 0;

  if (success) {
    Serial.println("WiFi credentials saved");
  } else {
    Serial.println("ERROR: Failed to save WiFi credentials");
  }

  return success;
}

bool SettingsManager::loadWiFiCredentials(String& ssid, String& password) {
  if (!initialized && !begin()) {
    return false;
  }

  ssid = prefs.getString(PREFS_KEY_SSID, "");
  password = prefs.getString(PREFS_KEY_PASSWORD, "");

  Serial.println("WiFi credentials loaded: " + (ssid.length() > 0 ? ssid : "(none)"));
  return ssid.length() > 0;
}

bool SettingsManager::clearWiFiCredentials() {
  if (!initialized && !begin()) {
    return false;
  }

  bool success = true;
  success &= prefs.remove(PREFS_KEY_SSID);
  success &= prefs.remove(PREFS_KEY_PASSWORD);

  return success;
}

// ====== PRESET MANAGEMENT ======

bool SettingsManager::savePreset(int index, const Preset& preset) {
  if (!initialized && !begin()) {
    return false;
  }

  // Create keys for this preset
  String keyName = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_name";
  String keyType = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_type";
  String keyFrom = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_from";
  String keyTo = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_to";
  String keyEnabled = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_enabled";
  String keyTrainsCount = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_trains";

  bool success = true;
  success &= prefs.putString(keyName.c_str(), preset.name) > 0;
  success &= prefs.putInt(keyType.c_str(), preset.type) > 0;
  success &= prefs.putString(keyFrom.c_str(), preset.fromStation) > 0;
  success &= prefs.putString(keyTo.c_str(), preset.toStation) > 0;
  success &= prefs.putBool(keyEnabled.c_str(), preset.enabled);
  success &= prefs.putUChar(keyTrainsCount.c_str(), preset.trainsToDisplay) > 0;

  if (success) {
    Serial.printf("Preset %d saved: %s\n", index, preset.name.c_str());
  } else {
    Serial.printf("ERROR: Failed to save preset %d\n", index);
  }

  return success;
}

bool SettingsManager::loadPreset(int index, Preset& preset) {
  if (!initialized && !begin()) {
    return false;
  }

  // Create keys for this preset
  String keyName = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_name";
  String keyType = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_type";
  String keyFrom = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_from";
  String keyTo = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_to";
  String keyEnabled = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_enabled";
  String keyTrainsCount = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_trains";

  preset.name = prefs.getString(keyName.c_str(), "");
  preset.type = (PresetType)prefs.getInt(keyType.c_str(), PRESET_TRAIN);
  preset.fromStation = prefs.getString(keyFrom.c_str(), "");
  preset.toStation = prefs.getString(keyTo.c_str(), "");
  preset.enabled = prefs.getBool(keyEnabled.c_str(), true);
  preset.trainsToDisplay = prefs.getUChar(keyTrainsCount.c_str(), 1); // Default to 1 for backward compatibility

  bool exists = preset.name.length() > 0;
  if (exists) {
    Serial.printf("Preset %d loaded: %s\n", index, preset.name.c_str());
  }

  return exists;
}

bool SettingsManager::deletePreset(int index) {
  if (!initialized && !begin()) {
    return false;
  }

  // Create keys for this preset
  String keyName = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_name";
  String keyType = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_type";
  String keyFrom = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_from";
  String keyTo = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_to";
  String keyEnabled = String(PREFS_KEY_PRESET_PREFIX) + String(index) + "_enabled";

  bool success = true;
  success &= prefs.remove(keyName.c_str());
  success &= prefs.remove(keyType.c_str());
  success &= prefs.remove(keyFrom.c_str());
  success &= prefs.remove(keyTo.c_str());
  success &= prefs.remove(keyEnabled.c_str());

  Serial.printf("Preset %d deleted\n", index);
  return success;
}

int SettingsManager::getPresetCount() {
  if (!initialized && !begin()) {
    return 0;
  }

  return prefs.getInt(PREFS_KEY_PRESET_COUNT, 0);
}

bool SettingsManager::setPresetCount(int count) {
  if (!initialized && !begin()) {
    return false;
  }

  return prefs.putInt(PREFS_KEY_PRESET_COUNT, count) > 0;
}

// ====== CURRENT STATE ======

bool SettingsManager::saveCurrentPreset(int index) {
  if (!initialized && !begin()) {
    return false;
  }

  return prefs.putInt(PREFS_KEY_CURRENT_PRESET, index) > 0;
}

int SettingsManager::loadCurrentPreset() {
  if (!initialized && !begin()) {
    return 0;
  }

  return prefs.getInt(PREFS_KEY_CURRENT_PRESET, 0);
}

// ====== UTILITY ======

bool SettingsManager::clearAll() {
  if (!initialized && !begin()) {
    return false;
  }

  bool success = prefs.clear();
  Serial.println(success ? "All settings cleared" : "ERROR: Failed to clear settings");
  return success;
}
