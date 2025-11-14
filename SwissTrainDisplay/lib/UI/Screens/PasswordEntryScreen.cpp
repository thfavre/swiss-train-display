#include "PasswordEntryScreen.h"

PasswordEntryScreen::PasswordEntryScreen(DisplayManager* disp, WiFiManager* wifiMgr, SettingsManager* settingsMgr)
  : Screen(disp), wifi(wifiMgr), settings(settingsMgr), password(""), charIndex(0), showModal(false), modalSelection(0) {
}

void PasswordEntryScreen::enter() {
  Serial.println("Entering PasswordEntryScreen");
  password = "";
  charIndex = 0;
  showModal = false;
  modalSelection = 0;
}

void PasswordEntryScreen::exit() {}
void PasswordEntryScreen::update() {}

void PasswordEntryScreen::handleEncoder(int delta) {
  if (delta == 0) return;

  if (showModal) {
    modalSelection += delta;
    if (modalSelection < 0) modalSelection = 3;
    if (modalSelection > 3) modalSelection = 0;
  } else {
    charIndex += delta;
    if (charIndex < 0) charIndex = KEYBOARD_CHARS_COUNT - 1;
    if (charIndex >= KEYBOARD_CHARS_COUNT) charIndex = 0;
  }
}

void PasswordEntryScreen::handleShortPress() {
  if (showModal) {
    switch (modalSelection) {
      case 0: // Del
        if (password.length() > 0) {
          password.remove(password.length() - 1);
        }
        break;
      case 1: // Save & Connect
        Serial.println("Connecting to: " + ssid);
        if (wifi->connect(ssid, password)) {
          settings->saveWiFiCredentials(ssid, password);
        }
        requestState(STATE_MAIN_DISPLAY);
        break;
      case 2: // Edit
        showModal = false;
        break;
      case 3: // Exit
        requestState(STATE_WIFI_SCAN);
        break;
    }
  } else {
    // Add character
    password += KEYBOARD_CHARS[charIndex];
  }
}

void PasswordEntryScreen::handleLongPress() {
  showModal = !showModal;
  if (showModal) {
    modalSelection = 0;
  }
}

void PasswordEntryScreen::draw() {
  display->clear();

  if (showModal) {
    // Show modal
    String buttons[] = {"Del", "Save", "Edit", "Exit"};
    ModalDialog::draw(*display, "Connect to:", ssid + "\nPass: " + password, buttons, 4, modalSelection);
  } else {
    // Normal entry mode
    YellowBar::draw(*display, ssid.substring(0, 15));

    // Draw password input in blue zone (safely below y=16)
    TextInputDisplay::draw(*display, "Pass:", password, 20);

    display->drawText("Select character:", 2, 32, 1);
    CharacterSelector::draw(*display, KEYBOARD_CHARS, KEYBOARD_CHARS_COUNT, charIndex, 42);
  }

  display->show();
}
