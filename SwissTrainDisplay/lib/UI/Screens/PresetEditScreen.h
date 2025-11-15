#ifndef PRESETEDITSCREEN_H
#define PRESETEDITSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"
#include "../../Data/PresetManager.h"

class PresetEditScreen : public Screen {
private:
  PresetManager* presets;
  int editingIndex;
  int fieldIndex;  // 0=name, 1=from, 2=to, 3=save
  bool editing;
  int charIndex;
  bool showModal;
  int modalSelection;
  Preset editBuffer;
  bool createMode;  // True when creating new preset, false when editing existing

  int getFieldCount() const;  // Get number of editable fields based on preset type

public:
  PresetEditScreen(DisplayManager* disp, PresetManager* presetMgr);

  void setEditingIndex(int index) { editingIndex = index; }
  void setCreateMode(PresetType type);  // Enter create mode for new preset

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;
};

#endif // PRESETEDITSCREEN_H
