#include "PresetManager.h"

PresetManager::PresetManager(SettingsManager* settingsManager)
  : currentPresetIndex(0), settings(settingsManager), isDirty(false) {
}

// ====== INITIALIZATION ======

void PresetManager::initializeDefaults() {
  Serial.println("Initializing default presets");

  presets.clear();

  // Default train routes
  presets.push_back(Preset("Lausanne-Geneva", "Lausanne", "Geneve"));
  presets.push_back(Preset("Lausanne-Bern", "Lausanne", "Bern"));
  presets.push_back(Preset("Lausanne-Zurich", "Lausanne", "Zurich"));

  // Default clock
  Preset clockPreset("Clock", PRESET_CLOCK);
  presets.push_back(clockPreset);

  currentPresetIndex = 0;
  isDirty = true;
}

// ====== LOAD/SAVE ======

bool PresetManager::loadAll() {
  if (!settings || !settings->isInitialized()) {
    Serial.println("ERROR: SettingsManager not initialized");
    initializeDefaults();
    return false;
  }

  int count = settings->getPresetCount();

  if (count == 0) {
    Serial.println("No presets found, initializing defaults");
    initializeDefaults();
    return saveAll();
  }

  Serial.printf("Loading %d presets\n", count);
  presets.clear();

  for (int i = 0; i < count; i++) {
    Preset preset;
    if (settings->loadPreset(i, preset)) {
      presets.push_back(preset);
    } else {
      Serial.printf("WARNING: Failed to load preset %d\n", i);
    }
  }

  // Load current preset index
  currentPresetIndex = settings->loadCurrentPreset();
  if (currentPresetIndex >= (int)presets.size()) {
    currentPresetIndex = 0;
  }

  isDirty = false;
  Serial.printf("Loaded %d presets, current: %d\n", presets.size(), currentPresetIndex);
  return true;
}

bool PresetManager::saveAll() {
  if (!settings || !settings->isInitialized()) {
    Serial.println("ERROR: SettingsManager not initialized");
    return false;
  }

  Serial.printf("Saving %d presets\n", presets.size());

  // Save preset count
  if (!settings->setPresetCount(presets.size())) {
    Serial.println("ERROR: Failed to save preset count");
    return false;
  }

  // Save each preset
  for (size_t i = 0; i < presets.size(); i++) {
    if (!settings->savePreset(i, presets[i])) {
      Serial.printf("ERROR: Failed to save preset %d\n", i);
      return false;
    }
  }

  // Save current preset index
  if (!settings->saveCurrentPreset(currentPresetIndex)) {
    Serial.println("ERROR: Failed to save current preset index");
    return false;
  }

  isDirty = false;
  Serial.println("All presets saved successfully");
  return true;
}

bool PresetManager::save(int index) {
  if (!isValidIndex(index)) {
    Serial.println("ERROR: Invalid preset index");
    return false;
  }

  if (!settings || !settings->isInitialized()) {
    Serial.println("ERROR: SettingsManager not initialized");
    return false;
  }

  return settings->savePreset(index, presets[index]);
}

// ====== PRESET OPERATIONS ======

bool PresetManager::addPreset(const Preset& preset) {
  if (!validatePreset(preset)) {
    Serial.println("ERROR: Invalid preset");
    return false;
  }

  presets.push_back(preset);
  isDirty = true;

  Serial.printf("Preset added: %s (total: %d)\n", preset.name.c_str(), presets.size());
  return true;
}

bool PresetManager::updatePreset(int index, const Preset& preset) {
  if (!isValidIndex(index)) {
    Serial.println("ERROR: Invalid preset index");
    return false;
  }

  if (!validatePreset(preset)) {
    Serial.println("ERROR: Invalid preset");
    return false;
  }

  presets[index] = preset;
  isDirty = true;

  Serial.printf("Preset %d updated: %s\n", index, preset.name.c_str());
  return true;
}

bool PresetManager::deletePreset(int index) {
  if (!isValidIndex(index)) {
    Serial.println("ERROR: Invalid preset index");
    return false;
  }

  // Don't allow deleting the last preset
  if (presets.size() <= 1) {
    Serial.println("ERROR: Cannot delete last preset");
    return false;
  }

  String name = presets[index].name;
  presets.erase(presets.begin() + index);
  isDirty = true;

  // Adjust current index if needed
  if (currentPresetIndex >= (int)presets.size()) {
    currentPresetIndex = presets.size() - 1;
  }

  Serial.printf("Preset deleted: %s (remaining: %d)\n", name.c_str(), presets.size());
  return true;
}

Preset* PresetManager::getPreset(int index) {
  if (!isValidIndex(index)) {
    return nullptr;
  }
  return &presets[index];
}

const Preset* PresetManager::getPreset(int index) const {
  if (!isValidIndex(index)) {
    return nullptr;
  }
  return &presets[index];
}

// ====== NAVIGATION ======

Preset* PresetManager::getCurrent() {
  return getPreset(currentPresetIndex);
}

const Preset* PresetManager::getCurrent() const {
  return getPreset(currentPresetIndex);
}

bool PresetManager::setCurrentIndex(int index) {
  if (!isValidIndex(index)) {
    return false;
  }

  currentPresetIndex = index;
  Serial.printf("Current preset: %d (%s)\n", index, presets[index].name.c_str());
  return true;
}

bool PresetManager::next() {
  if (presets.empty()) {
    return false;
  }

  currentPresetIndex = (currentPresetIndex + 1) % presets.size();
  Serial.printf("Next preset: %d (%s)\n", currentPresetIndex, presets[currentPresetIndex].name.c_str());
  return true;
}

bool PresetManager::previous() {
  if (presets.empty()) {
    return false;
  }

  currentPresetIndex--;
  if (currentPresetIndex < 0) {
    currentPresetIndex = presets.size() - 1;
  }

  Serial.printf("Previous preset: %d (%s)\n", currentPresetIndex, presets[currentPresetIndex].name.c_str());
  return true;
}

// ====== VALIDATION ======

bool PresetManager::isValidIndex(int index) const {
  return index >= 0 && index < (int)presets.size();
}

bool PresetManager::validatePreset(const Preset& preset) const {
  // Name must not be empty
  if (preset.name.length() == 0) {
    return false;
  }

  // Train presets must have from/to stations
  if (preset.type == PRESET_TRAIN) {
    if (preset.fromStation.length() == 0 || preset.toStation.length() == 0) {
      return false;
    }
  }

  return true;
}

// ====== UTILITY ======

void PresetManager::clear() {
  presets.clear();
  currentPresetIndex = 0;
  isDirty = true;
  Serial.println("All presets cleared");
}
