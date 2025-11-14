#ifndef APPSTATE_H
#define APPSTATE_H

// ====== APPLICATION STATES ======
// Each state corresponds to a different screen

enum AppState {
  STATE_MAIN_DISPLAY,      // Main train/clock display
  STATE_MENU,              // Main menu
  STATE_SETTINGS,          // Settings menu
  STATE_WIFI_SCAN,         // WiFi network scanning
  STATE_WIFI_PASSWORD,     // WiFi password entry
  STATE_PRESET_SELECT,     // Preset selection (NEW - for unlimited presets)
  STATE_PRESET_EDIT,       // Preset editing
  STATE_ERROR              // Error display (NEW)
};

// ====== MENU IDS ======

enum MainMenuId {
  MENU_SETTINGS = 0,
  MENU_PRESETS,            // NEW - manage presets
  MENU_REFRESH,
  MENU_EXIT
};

enum SettingsMenuId {
  SETTINGS_WIFI = 0,
  SETTINGS_DISPLAY,        // NEW - display settings (future)
  SETTINGS_BACK
};

enum PresetMenuId {
  PRESET_SELECT = 0,
  PRESET_ADD,
  PRESET_EDIT,
  PRESET_DELETE,
  PRESET_BACK
};

// ====== MODAL ACTIONS ======

enum ModalAction {
  MODAL_DELETE = 0,
  MODAL_SAVE,
  MODAL_CANCEL,
  MODAL_EXIT
};

#endif // APPSTATE_H
