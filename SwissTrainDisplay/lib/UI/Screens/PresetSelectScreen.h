#ifndef PRESETSELECTSCREEN_H
#define PRESETSELECTSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Data/PresetManager.h"

enum PresetScreenMode {
  MODE_LIST,           // Normal preset list with checkboxes
  MODE_ACTION_MENU,    // Action menu for selected preset
  MODE_TYPE_SELECT,    // Type selection for new preset
  MODE_DELETE_CONFIRM  // Delete confirmation dialog
};

class PresetSelectScreen : public Screen {
private:
  PresetManager* presets;
  MenuList menuList;
  int selection;
  PresetScreenMode mode;
  int actionSelection;
  int typeSelection;
  int deleteConfirmSelection;
  int presetToDelete;
  PresetType newPresetType;

  int getTotalMenuItems() const;
  void drawList();
  void drawActionMenu();
  void drawTypeSelect();
  void drawDeleteConfirm();
  void handleActionSelection();
  void handleTypeSelection();
  void handleDeleteConfirm();

public:
  PresetSelectScreen(DisplayManager* disp, PresetManager* presetMgr);

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;

  // Getters for state machine transitions
  int getSelectedPreset() const { return selection; }
  PresetType getNewPresetType() const { return newPresetType; }
  bool isInCreateMode() const { return mode == MODE_TYPE_SELECT; }
};

#endif // PRESETSELECTSCREEN_H
