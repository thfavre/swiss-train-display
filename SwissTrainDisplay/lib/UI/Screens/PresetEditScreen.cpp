#include "PresetEditScreen.h"

PresetEditScreen::PresetEditScreen(DisplayManager* disp, PresetManager* presetMgr)
  : Screen(disp), presets(presetMgr), editingIndex(0), fieldIndex(0),
    editing(false), charIndex(0), showModal(false), modalSelection(0), createMode(false) {
}

void PresetEditScreen::setCreateMode(PresetType type) {
  createMode = true;
  editBuffer = Preset("", type);  // Empty preset of specified type
  editBuffer.enabled = true;
  editingIndex = -1;

  // Set default names based on type
  switch (type) {
    case PRESET_CLOCK:
      editBuffer.name = "Clock";
      break;
    case PRESET_WEATHER:
      editBuffer.name = "Weather";
      break;
    case PRESET_CALENDAR:
      editBuffer.name = "Calendar";
      break;
    case PRESET_TRAIN:
    default:
      editBuffer.name = "";
      break;
  }
}

int PresetEditScreen::getFieldCount() const {
  // Number of editable fields + "Save" button + optional "Cancel" button
  if (editBuffer.type == PRESET_TRAIN) {
    return createMode ? 6 : 5;  // Name, From, To, Trains, Save, [Cancel]
  } else {
    return createMode ? 3 : 2;  // Name, Save, [Cancel]
  }
}

void PresetEditScreen::enter() {
  Serial.println("Entering PresetEditScreen");
  if (!createMode) {
    // Edit mode - load existing preset
    editingIndex = presets->getCurrentIndex();
    const Preset* current = presets->getCurrent();
    if (current) {
      editBuffer = *current;  // Copy to edit buffer
    }
  }
  // Create mode keeps the editBuffer set by setCreateMode()

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
    bool isStationField = (editBuffer.type == PRESET_TRAIN && (fieldIndex == 1 || fieldIndex == 2));
    int maxChars = isStationField ? STATION_CHARS_COUNT : KEYBOARD_CHARS_COUNT;
    charIndex += delta;
    if (charIndex < 0) charIndex = maxChars - 1;
    if (charIndex >= maxChars) charIndex = 0;
  } else {
    // Field selection
    int maxField = getFieldCount() - 1;
    fieldIndex += delta;
    if (fieldIndex < 0) fieldIndex = maxField;
    if (fieldIndex > maxField) fieldIndex = 0;
  }
}

void PresetEditScreen::handleShortPress() {
  if (showModal) {
    switch (modalSelection) {
      case 0: // Del
        if (fieldIndex == 0 && editBuffer.name.length() > 0) {
          editBuffer.name.remove(editBuffer.name.length() - 1);
        }
        if (editBuffer.type == PRESET_TRAIN) {
          if (fieldIndex == 1 && editBuffer.fromStation.length() > 0) {
            editBuffer.fromStation.remove(editBuffer.fromStation.length() - 1);
          }
          if (fieldIndex == 2 && editBuffer.toStation.length() > 0) {
            editBuffer.toStation.remove(editBuffer.toStation.length() - 1);
          }
        }
        break;
      case 1: // Done (close modal and continue editing)
        showModal = false;
        editing = false;
        break;
      case 2: // Cancel
        showModal = false;
        break;
    }
  } else if (editing) {
    // Add character
    bool isStationField = (editBuffer.type == PRESET_TRAIN && (fieldIndex == 1 || fieldIndex == 2));
    const char* charset = isStationField ? STATION_CHARS : KEYBOARD_CHARS;
    char ch = charset[charIndex];

    // Auto-capitalize first letter for station names
    if (isStationField && ch >= 'a' && ch <= 'z') {
      String& field = (fieldIndex == 1) ? editBuffer.fromStation : editBuffer.toStation;
      if (field.length() == 0) ch = ch - 32;
    }

    if (fieldIndex == 0) {
      editBuffer.name += ch;
    } else if (editBuffer.type == PRESET_TRAIN) {
      if (fieldIndex == 1) editBuffer.fromStation += ch;
      else if (fieldIndex == 2) editBuffer.toStation += ch;
    }
  } else {
    // Enter editing, save, or cancel
    int fieldCount = getFieldCount();
    int saveFieldIndex = createMode ? fieldCount - 2 : fieldCount - 1;
    int cancelFieldIndex = createMode ? fieldCount - 1 : -1;

    if (fieldIndex == saveFieldIndex) {
      // Save and exit
      if (createMode) {
        presets->addPreset(editBuffer);
        createMode = false;
      } else {
        presets->updatePreset(editingIndex, editBuffer);
      }
      presets->saveAll();
      requestState(STATE_PRESET_SELECT);
    } else if (createMode && fieldIndex == cancelFieldIndex) {
      // Cancel creation
      createMode = false;
      requestState(STATE_PRESET_SELECT);
    } else if (editBuffer.type == PRESET_TRAIN && fieldIndex == 3) {
      // Trains count field - cycle through 1-4
      editBuffer.trainsToDisplay++;
      if (editBuffer.trainsToDisplay > 4) {
        editBuffer.trainsToDisplay = 1;
      }
    } else {
      // Enter field editing for name/from/to fields
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
    createMode = false;
    requestState(STATE_PRESET_SELECT);
  }
}

void PresetEditScreen::draw() {
  display->clear();

  if (showModal) {
    String fieldName = (fieldIndex == 0) ? "Name" :
                      (fieldIndex == 1) ? "From" : "To";
    String* field = (fieldIndex == 0) ? &editBuffer.name :
                   (fieldIndex == 1) ? &editBuffer.fromStation : &editBuffer.toStation;
    String buttons[] = {"Del", "Done", "Cancel"};
    String title = createMode ? "New Preset" : "Edit " + fieldName;
    ModalDialog::draw(*display, title, *field, buttons, 3, modalSelection);
  } else if (editing) {
    // Character selection mode
    String title = createMode ? "New Preset" : "Edit Field";
    YellowBar::draw(*display, title);

    String fieldLabel = (fieldIndex == 0) ? "Name:" :
                       (fieldIndex == 1) ? "From:" : "To:";
    String* field = (fieldIndex == 0) ? &editBuffer.name :
                   (fieldIndex == 1) ? &editBuffer.fromStation : &editBuffer.toStation;
    TextInputDisplay::draw(*display, fieldLabel, *field, 18);

    bool isStationField = (editBuffer.type == PRESET_TRAIN && (fieldIndex == 1 || fieldIndex == 2));
    const char* charset = isStationField ? STATION_CHARS : KEYBOARD_CHARS;
    int charsetSize = isStationField ? STATION_CHARS_COUNT : KEYBOARD_CHARS_COUNT;
    CharacterSelector::draw(*display, charset, charsetSize, charIndex);
  } else {
    // Field selection mode
    String title = createMode ? "New Preset" : "Edit Preset";
    YellowBar::draw(*display, title);

    int fieldCount = getFieldCount();
    String items[fieldCount];

    if (editBuffer.type == PRESET_TRAIN) {
      // Show "(optional)" for empty train preset names
      String displayName = editBuffer.name.length() > 0 ? editBuffer.name.substring(0, 8) : "(optional)";
      items[0] = "Name: " + displayName;
      items[1] = "From: " + editBuffer.fromStation.substring(0, 8);
      items[2] = "To: " + editBuffer.toStation.substring(0, 8);
      items[3] = "Trains: " + String(editBuffer.trainsToDisplay);
      items[4] = "< Save";
      if (createMode) {
        items[5] = "< Cancel";
      }
    } else {
      // Clock, Weather, Calendar - just name
      items[0] = "Name: " + editBuffer.name.substring(0, 8);
      items[1] = "< Save";
      if (createMode) {
        items[2] = "< Cancel";
      }
    }

    MenuList list;
    list.setSelected(fieldIndex);
    list.draw(*display, items, fieldCount, BLUE_ZONE_Y + 2);
  }

  display->show();
}
