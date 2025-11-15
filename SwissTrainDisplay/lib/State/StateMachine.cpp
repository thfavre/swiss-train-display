#include "StateMachine.h"

StateMachine::StateMachine(DisplayManager* disp, EncoderHandler* enc, ButtonHandler* btn,
                           PresetManager* presetMgr, TrainAPI* api,
                           WiFiManager* wifiMgr, SettingsManager* settingsMgr)
  : display(disp), encoder(enc), button(btn), presets(presetMgr),
    trainAPI(api), wifi(wifiMgr), settings(settingsMgr),
    currentState(STATE_MAIN_DISPLAY), currentScreen(nullptr),
    selectedSSID(""), selectedNetworkIndex(0) {
}

StateMachine::~StateMachine() {
  // Clean up screens
  for (auto& pair : screens) {
    delete pair.second;
  }
  screens.clear();
}

void StateMachine::begin() {
  Serial.println("Initializing StateMachine...");

  // Create all screens
  screens[STATE_MAIN_DISPLAY] = new MainScreen(display, presets, trainAPI, wifi);
  screens[STATE_MENU] = new MenuScreen(display, wifi, presets, trainAPI);
  screens[STATE_SETTINGS] = new SettingsScreen(display);
  screens[STATE_WIFI_SCAN] = new WiFiScanScreen(display, wifi, settings);
  screens[STATE_WIFI_PASSWORD] = new PasswordEntryScreen(display, wifi, settings);
  screens[STATE_PRESET_EDIT] = new PresetEditScreen(display, presets);
  screens[STATE_PRESET_SELECT] = new PresetSelectScreen(display, presets);
  screens[STATE_ERROR] = new ErrorScreen(display);

  // Set initial state
  setState(STATE_MAIN_DISPLAY);

  Serial.println("StateMachine initialized");
}

void StateMachine::update() {
  if (!currentScreen) {
    return;
  }

  bool needsRedraw = false;

  // Update current screen (may set internal redraw flag)
  currentScreen->update();

  // Check if screen requested redraw (e.g., clock ticking)
  if (currentScreen->needsRedrawNow()) {
    needsRedraw = true;
    currentScreen->clearRedrawFlag();
  }

  // Handle input
  int encoderDelta = encoder->getDelta();
  if (encoderDelta != 0) {
    Serial.print("Encoder delta: ");
    Serial.println(encoderDelta);
    currentScreen->handleEncoder(encoderDelta);
    needsRedraw = true;  // Redraw when encoder moves
  }

  ButtonEvent buttonEvent = button->getEvent();
  if (buttonEvent == BUTTON_SHORT_PRESS) {
    currentScreen->handleShortPress();
    needsRedraw = true;  // Redraw on button press
  } else if (buttonEvent == BUTTON_LONG_PRESS) {
    currentScreen->handleLongPress();
    needsRedraw = true;  // Redraw on button press
  }

  // Check for state change request
  if (currentScreen->hasStateChangeRequest()) {
    AppState nextState = currentScreen->getNextState();
    currentScreen->clearStateChangeRequest();

    // Handle special context passing between screens
    if (currentState == STATE_WIFI_SCAN && nextState == STATE_WIFI_PASSWORD) {
      // Pass selected network to password entry screen
      WiFiScanScreen* wifiScan = (WiFiScanScreen*)currentScreen;
      int networkIndex = wifiScan->getSelected();
      const WiFiNetwork* network = wifi->getNetwork(networkIndex);
      if (network) {
        selectedSSID = network->ssid;
        PasswordEntryScreen* passwordScreen = (PasswordEntryScreen*)screens[STATE_WIFI_PASSWORD];
        passwordScreen->setSSID(selectedSSID);
      }
    }

    if (currentState == STATE_PRESET_SELECT && nextState == STATE_PRESET_EDIT) {
      // Pass preset selection or create mode to edit screen
      PresetSelectScreen* presetSelect = (PresetSelectScreen*)currentScreen;
      PresetEditScreen* presetEdit = (PresetEditScreen*)screens[STATE_PRESET_EDIT];

      if (presetSelect->isInCreateMode()) {
        // Creating new preset
        PresetType type = presetSelect->getNewPresetType();
        presetEdit->setCreateMode(type);
      } else {
        // Editing existing preset
        int presetIndex = presetSelect->getSelectedPreset();
        presetEdit->setEditingIndex(presetIndex);
      }
    }

    setState(nextState);
    needsRedraw = true;  // Redraw on state change
  }

  // Draw only when something changed
  if (needsRedraw) {
    currentScreen->draw();
  }
}

void StateMachine::setState(AppState newState) {
  Serial.print("State transition: ");
  Serial.print(currentState);
  Serial.print(" -> ");
  Serial.println(newState);

  // Exit current screen
  if (currentScreen) {
    currentScreen->exit();
  }

  // Change state
  currentState = newState;
  currentScreen = screens[currentState];

  if (!currentScreen) {
    Serial.println("ERROR: No screen registered for state!");
    return;
  }

  // Enter new screen
  currentScreen->enter();

  // Draw the new screen immediately
  currentScreen->draw();
}
