#include "PresetEditScreen.h"

PresetEditScreen::PresetEditScreen(DisplayManager* disp, PresetManager* presetMgr)
  : Screen(disp), presets(presetMgr), editingIndex(0), fieldIndex(0),
    editing(false), charIndex(0), showModal(false), modalSelection(0) {
}

void PresetEditScreen::enter() {
  Serial.println("Entering PresetEditScreen");
  editingIndex = presets->getCurrentIndex();
  const Preset* current = presets->getCurrent();
  if (current) {
    editBuffer = *current;  // Copy to edit buffer
  }
  fieldIndex = 0;
  editing = false;
  showModal = false;
}

void PresetEditScreen::exit() {}
void PresetEditScreen::update() {}

void PresetEditScreen::handleEncoder(int delta) {
  if (delta == 0) return;

  if (showModal) {
    modalSelection += delta;
    if (modalSelection < 0) modalSelection = 2;
    if (modalSelection > 2) modalSelection = 0;
  } else if (editing) {
    // Character selection
    charIndex += delta;
    int maxChars = (fieldIndex == 1 || fieldIndex == 2) ? STATION_CHARS_COUNT : KEYBOARD_CHARS_COUNT;
    if (charIndex < 0) charIndex = maxChars - 1;
    if (charIndex >= maxChars) charIndex = 0;
  } else {
    // Field selection
    fieldIndex += delta;
    if (fieldIndex < 0) fieldIndex = 3;
    if (fieldIndex > 3) fieldIndex = 0;
  }
}

void PresetEditScreen::handleShortPress() {
  if (showModal) {
    switch (modalSelection) {
      case 0: // Del
        if (fieldIndex == 0 && editBuffer.name.length() > 0) editBuffer.name.remove(editBuffer.name.length() - 1);
        if (fieldIndex == 1 && editBuffer.fromStation.length() > 0) editBuffer.fromStation.remove(editBuffer.fromStation.length() - 1);
        if (fieldIndex == 2 && editBuffer.toStation.length() > 0) editBuffer.toStation.remove(editBuffer.toStation.length() - 1);
        break;
      case 1: // Save
        presets->updatePreset(editingIndex, editBuffer);
        presets->saveAll();
        requestState(STATE_MAIN_DISPLAY);
        break;
      case 2: // Cancel
        showModal = false;
        break;
    }
  } else if (editing) {
    // Add character
    const char* charset = (fieldIndex == 1 || fieldIndex == 2) ? STATION_CHARS : KEYBOARD_CHARS;
    char ch = charset[charIndex];
    // Auto-capitalize first letter for station names
    if ((fieldIndex == 1 || fieldIndex == 2) && ch >= 'a' && ch <= 'z') {
      String& field = (fieldIndex == 1) ? editBuffer.fromStation : editBuffer.toStation;
      if (field.length() == 0) ch = ch - 32;
    }

    if (fieldIndex == 0) editBuffer.name += ch;
    else if (fieldIndex == 1) editBuffer.fromStation += ch;
    else if (fieldIndex == 2) editBuffer.toStation += ch;
  } else {
    // Enter editing or save
    if (fieldIndex == 3) {
      // Save and exit
      presets->updatePreset(editingIndex, editBuffer);
      presets->saveAll();
      requestState(STATE_MAIN_DISPLAY);
    } else {
      editing = true;
      charIndex = 0;
    }
  }
}

void PresetEditScreen::handleLongPress() {
  if (editing) {
    showModal = !showModal;
    if (showModal) modalSelection = 0;
  } else {
    requestState(STATE_MAIN_DISPLAY);
  }
}

void PresetEditScreen::draw() {
  display->clear();

  if (showModal) {
    String fieldNames[] = {"Name", "From", "To"};
    String* fields[] = {&editBuffer.name, &editBuffer.fromStation, &editBuffer.toStation};
    String buttons[] = {"Del", "Save", "Cancel"};
    ModalDialog::draw(*display, "Edit " + fieldNames[fieldIndex],
                     *fields[fieldIndex], buttons, 3, modalSelection);
  } else if (editing) {
    // Character selection mode
    YellowBar::draw(*display, "Edit Field");
    String fieldNames[] = {"Name:", "From:", "To:"};
    String* fields[] = {&editBuffer.name, &editBuffer.fromStation, &editBuffer.toStation};
    TextInputDisplay::draw(*display, fieldNames[fieldIndex], *fields[fieldIndex], 18);

    const char* charset = (fieldIndex == 1 || fieldIndex == 2) ? STATION_CHARS : KEYBOARD_CHARS;
    int charsetSize = (fieldIndex == 1 || fieldIndex == 2) ? STATION_CHARS_COUNT : KEYBOARD_CHARS_COUNT;
    CharacterSelector::draw(*display, charset, charsetSize, charIndex);
  } else {
    // Field selection mode
    YellowBar::draw(*display, "Edit Preset");

    String items[] = {
      "Name: " + editBuffer.name.substring(0, 8),
      "From: " + editBuffer.fromStation.substring(0, 8),
      "To: " + editBuffer.toStation.substring(0, 8),
      "< Save"
    };
    MenuList list;
    list.setSelected(fieldIndex);
    list.draw(*display, items, 4, BLUE_ZONE_Y + 2);
  }

  display->show();
}
