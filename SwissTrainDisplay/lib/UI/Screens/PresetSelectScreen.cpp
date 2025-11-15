#include "PresetSelectScreen.h"

PresetSelectScreen::PresetSelectScreen(DisplayManager* disp, PresetManager* presetMgr)
  : Screen(disp), presets(presetMgr), selection(0), mode(MODE_LIST),
    actionSelection(0), typeSelection(0), deleteConfirmSelection(0),
    presetToDelete(-1), newPresetType(PRESET_TRAIN) {
}

void PresetSelectScreen::enter() {
  Serial.println("Entering PresetSelectScreen");
  selection = presets->getCurrentIndex();
  mode = MODE_LIST;
  actionSelection = 0;
  typeSelection = 0;
  deleteConfirmSelection = 0;
  menuList.setSelected(selection);
}

void PresetSelectScreen::exit() {}
void PresetSelectScreen::update() {}

int PresetSelectScreen::getTotalMenuItems() const {
  // Presets + "Add New" + "< Back"
  return presets->getCount() + 2;
}

void PresetSelectScreen::handleEncoder(int delta) {
  if (delta == 0) return;

  switch (mode) {
    case MODE_LIST:
      selection += delta;
      {
        int totalItems = getTotalMenuItems();
        if (selection < 0) selection = totalItems - 1;
        if (selection >= totalItems) selection = 0;
        menuList.setSelected(selection);
      }
      break;

    case MODE_ACTION_MENU:
      actionSelection += delta;
      if (actionSelection < 0) actionSelection = 3;  // 4 options: Edit, Delete, Toggle/Activate, Cancel
      if (actionSelection > 3) actionSelection = 0;
      break;

    case MODE_TYPE_SELECT:
      typeSelection += delta;
      if (typeSelection < 0) typeSelection = 4;  // 5 items: Train, Clock, Weather, Calendar, Cancel
      if (typeSelection > 4) typeSelection = 0;
      break;

    case MODE_DELETE_CONFIRM:
      deleteConfirmSelection += delta;
      if (deleteConfirmSelection < 0) deleteConfirmSelection = 1;  // Cancel, Confirm
      if (deleteConfirmSelection > 1) deleteConfirmSelection = 0;
      break;
  }
}

void PresetSelectScreen::handleShortPress() {
  switch (mode) {
    case MODE_LIST:
      {
        int presetCount = presets->getCount();
        if (selection < presetCount) {
          // Preset selected - show action menu
          mode = MODE_ACTION_MENU;
          actionSelection = 0;
        } else if (selection == presetCount) {
          // "Add New" selected - show type selection
          mode = MODE_TYPE_SELECT;
          typeSelection = 0;
        } else if (selection == presetCount + 1) {
          // "< Back" selected - return to menu
          requestState(STATE_MENU);
        }
      }
      break;

    case MODE_ACTION_MENU:
      handleActionSelection();
      break;

    case MODE_TYPE_SELECT:
      handleTypeSelection();
      break;

    case MODE_DELETE_CONFIRM:
      handleDeleteConfirm();
      break;
  }
}

void PresetSelectScreen::handleLongPress() {
  if (mode == MODE_LIST) {
    requestState(STATE_MENU);
  } else {
    // Cancel any modal and return to list
    mode = MODE_LIST;
  }
}

void PresetSelectScreen::draw() {
  switch (mode) {
    case MODE_LIST:
      drawList();
      break;
    case MODE_ACTION_MENU:
      drawActionMenu();
      break;
    case MODE_TYPE_SELECT:
      drawTypeSelect();
      break;
    case MODE_DELETE_CONFIRM:
      drawDeleteConfirm();
      break;
  }
}

void PresetSelectScreen::drawList() {
  display->clear();
  YellowBar::draw(*display, "Manage Presets");

  Adafruit_SSD1306& d = display->getDisplay();
  int count = presets->getCount();
  int totalItems = getTotalMenuItems();
  String items[totalItems];

  // Build preset list with checkboxes on left
  for (int i = 0; i < count; i++) {
    const Preset* p = presets->getPreset(i);
    if (p) {
      String checkbox = p->enabled ? "[x]" : "[ ]";
      String prefix = (i == presets->getCurrentIndex()) ? ">" : " ";
      // Truncate name to max 15 chars to prevent wrapping
      String name = PresetManager::getDisplayName(*p);
      if (name.length() > 15) {
        name = name.substring(0, 15);
      }
      items[i] = checkbox + prefix + name;
    }
  }

  // Add bottom menu items
  items[count] = "Add New";
  items[count + 1] = "< Back";

  menuList.draw(*display, items, totalItems, BLUE_ZONE_Y + 2);
  display->show();
}

void PresetSelectScreen::drawActionMenu() {
  display->clear();

  const Preset* p = presets->getPreset(selection);
  if (!p) {
    mode = MODE_LIST;
    return;
  }

  // Truncate preset name for title
  String title = PresetManager::getDisplayName(*p);
  if (title.length() > 13) {
    title = title.substring(0, 13);
  }
  YellowBar::draw(*display, title);

  String toggleText = p->enabled ? "Disable" : "Enable";
  String items[] = {"Edit", "Delete", toggleText, "< Cancel"};

  MenuList list;
  list.setSelected(actionSelection);
  list.draw(*display, items, 4, BLUE_ZONE_Y + 2);

  display->show();
}

void PresetSelectScreen::drawTypeSelect() {
  display->clear();
  YellowBar::draw(*display, "Add Preset");

  String items[] = {"Train Route", "Clock", "Weather", "Calendar", "< Cancel"};
  MenuList list;
  list.setSelected(typeSelection);
  list.draw(*display, items, 5, BLUE_ZONE_Y + 2);

  display->show();
}

void PresetSelectScreen::drawDeleteConfirm() {
  display->clear();

  const Preset* p = presets->getPreset(presetToDelete);
  if (!p) {
    mode = MODE_LIST;
    return;
  }

  String content = "Delete '" + PresetManager::getDisplayName(*p) + "'?";
  String buttons[] = {"Cancel", "Confirm"};

  ModalDialog::draw(*display, "Confirm Delete", content, buttons, 2, deleteConfirmSelection);
  display->show();
}

void PresetSelectScreen::handleActionSelection() {
  const Preset* p = presets->getPreset(selection);
  if (!p) {
    mode = MODE_LIST;
    return;
  }

  switch (actionSelection) {
    case 0: // Edit
      requestState(STATE_PRESET_EDIT);
      break;

    case 1: // Delete
      if (presets->getCount() <= 1) {
        Serial.println("Cannot delete last preset");
        mode = MODE_LIST;
      } else {
        presetToDelete = selection;
        mode = MODE_DELETE_CONFIRM;
        deleteConfirmSelection = 0;
      }
      break;

    case 2: // Toggle Enable/Disable
      {
        Preset updatedPreset = *p;
        updatedPreset.enabled = !updatedPreset.enabled;
        presets->updatePreset(selection, updatedPreset);
        presets->saveAll();
        mode = MODE_LIST;
      }
      break;

    case 3: // Cancel
      mode = MODE_LIST;
      break;
  }
}

void PresetSelectScreen::handleTypeSelection() {
  // Check for cancel
  if (typeSelection == 4) {
    mode = MODE_LIST;
    return;
  }

  // Map selection to PresetType
  switch (typeSelection) {
    case 0:
      newPresetType = PRESET_TRAIN;
      break;
    case 1:
      newPresetType = PRESET_CLOCK;
      break;
    case 2:
      newPresetType = PRESET_WEATHER;
      break;
    case 3:
      newPresetType = PRESET_CALENDAR;
      break;
  }

  // Go to preset creation screen (PresetEditScreen in create mode)
  requestState(STATE_PRESET_EDIT);
}

void PresetSelectScreen::handleDeleteConfirm() {
  if (deleteConfirmSelection == 1) {
    // Confirm - delete the preset
    if (presetToDelete >= 0 && presetToDelete < presets->getCount()) {
      // If deleting current preset, switch to another first
      if (presetToDelete == presets->getCurrentIndex()) {
        int newCurrent = (presetToDelete > 0) ? presetToDelete - 1 : 1;
        presets->setCurrentIndex(newCurrent);
      }

      presets->deletePreset(presetToDelete);
      presets->saveAll();

      // Adjust selection if needed
      if (selection >= presets->getCount()) {
        selection = presets->getCount() - 1;
      }
    }
  }

  // Cancel or done - return to list
  presetToDelete = -1;
  mode = MODE_LIST;
}
