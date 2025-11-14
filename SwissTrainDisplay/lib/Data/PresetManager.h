#ifndef PRESETMANAGER_H
#define PRESETMANAGER_H

#include <Arduino.h>
#include <vector>
#include "../../include/Types.h"
#include "../Storage/SettingsManager.h"

class PresetManager {
private:
  std::vector<Preset> presets;
  int currentPresetIndex;
  SettingsManager* settings;
  bool isDirty;  // Track if presets need saving

  // Initialize default presets if none exist
  void initializeDefaults();

public:
  PresetManager(SettingsManager* settingsManager);

  // Load/Save
  bool loadAll();
  bool saveAll();
  bool save(int index);

  // Preset operations
  bool addPreset(const Preset& preset);
  bool updatePreset(int index, const Preset& preset);
  bool deletePreset(int index);
  Preset* getPreset(int index);
  const Preset* getPreset(int index) const;

  // Navigation
  int getCount() const { return presets.size(); }
  int getCurrentIndex() const { return currentPresetIndex; }
  Preset* getCurrent();
  const Preset* getCurrent() const;
  bool setCurrentIndex(int index);
  bool next();
  bool previous();

  // Validation
  bool isValidIndex(int index) const;
  bool validatePreset(const Preset& preset) const;

  // Utility
  void clear();
  bool hasDirtyFlag() const { return isDirty; }
  void clearDirtyFlag() { isDirty = false; }
};

#endif // PRESETMANAGER_H
