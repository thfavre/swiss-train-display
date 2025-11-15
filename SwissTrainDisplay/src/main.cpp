/*
 * Swiss Train Display - REFACTORED MODULAR VERSION
 *
 * Hardware:
 *   OLED (2-color): SDA=GPIO8, SCL=GPIO9 (128x64, Yellow top 16px, Blue bottom 48px)
 *   Encoder: CLK=GPIO4, DT=GPIO5, SW=GPIO6
 *
 * Controls:
 *   - Turn: Navigate menus/options
 *   - Short Press: Select/Confirm
 *   - Long Press (1s): Back/Cancel/Menu
 */

#include <Arduino.h>
#include "../include/Config.h"
#include "../include/Types.h"
#include "../include/AppState.h"

// Managers
#include "../lib/UI/DisplayManager.h"
#include "../lib/Input/EncoderHandler.h"
#include "../lib/Input/ButtonHandler.h"
#include "../lib/Storage/SettingsManager.h"
#include "../lib/Data/PresetManager.h"
#include "../lib/Data/TrainAPI.h"
#include "../lib/Network/WiFiManager.h"
#include "../lib/State/StateMachine.h"

// ====== GLOBAL OBJECTS ======
// Use pointers to avoid global constructor issues

DisplayManager* displayManager = nullptr;
EncoderHandler* encoderHandler = nullptr;
ButtonHandler* buttonHandler = nullptr;
SettingsManager* settingsManager = nullptr;
PresetManager* presetManager = nullptr;
TrainAPI* trainAPI = nullptr;
WiFiManager* wifiManager = nullptr;
StateMachine* stateMachine = nullptr;

// ====== SETUP ======

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n========================================");
  Serial.println("Swiss Train Display - Modular Version");
  Serial.println("========================================\n");

  // Create all manager objects
  Serial.println("Creating managers...");

  displayManager = new DisplayManager();
  encoderHandler = new EncoderHandler();
  buttonHandler = new ButtonHandler();
  settingsManager = new SettingsManager();
  trainAPI = new TrainAPI();
  wifiManager = new WiFiManager();

  Serial.println("Managers created");

  // Initialize display
  if (!displayManager->begin()) {
    Serial.println("FATAL: Display initialization failed!");
    while (1) {
      delay(1000);
    }
  }

  // Show splash screen
  displayManager->clear();
  displayManager->drawCenteredText("Swiss", 15, 2);
  displayManager->drawCenteredText("Train Display", 35, 1);
  displayManager->drawCenteredText("v2.0", 50, 1);
  displayManager->show();
  delay(2000);

  // Initialize input handlers
  encoderHandler->begin();
  buttonHandler->begin();

  // Initialize settings manager
  if (!settingsManager->begin()) {
    Serial.println("WARNING: Settings manager init failed");
  }

  // Create preset manager after settings manager is ready
  Serial.println("Creating preset manager...");
  presetManager = new PresetManager(settingsManager);
  Serial.println("Preset manager created");

  // Load presets
  presetManager->loadAll();
  Serial.printf("Loaded %d presets\n", presetManager->getCount());

  // Auto-connect to WiFi if credentials exist
  String ssid, password;
  if (settingsManager->loadWiFiCredentials(ssid, password)) {
    Serial.println("Attempting WiFi auto-connect...");

    displayManager->clear();
    displayManager->drawCenteredText("Connecting WiFi...", 28, 1);
    displayManager->show();

    if (wifiManager->connect(ssid, password, WIFI_CONNECT_TIMEOUT_MS)) {
      Serial.println("WiFi connected!");

      displayManager->clear();
      displayManager->drawCenteredText("WiFi Connected!", 28, 1);
      displayManager->show();
      delay(1500);

      // Fetch initial train data if current preset is a train
      const Preset* current = presetManager->getCurrent();
      if (current && current->type == PRESET_TRAIN) {
        std::vector<TrainConnection> connections;
        int limit = current->trainsToDisplay;
        trainAPI->fetchConnections(current->fromStation, current->toStation, connections, limit);
      }
    } else {
      Serial.println("WiFi connection failed");

      displayManager->clear();
      displayManager->drawCenteredText("WiFi Failed", 20, 1);
      displayManager->drawCenteredText("Check settings", 35, 1);
      displayManager->show();
      delay(2000);
    }
  } else {
    Serial.println("No WiFi credentials saved");
  }

  // Initialize state machine
  Serial.println("Creating state machine...");
  stateMachine = new StateMachine(
    displayManager,
    encoderHandler,
    buttonHandler,
    presetManager,
    trainAPI,
    wifiManager,
    settingsManager
  );
  Serial.println("State machine created");

  stateMachine->begin();

  Serial.println("\n========================================");
  Serial.println("System ready!");
  Serial.println("========================================\n");
}

// ====== MAIN LOOP ======

void loop() {
  // Update button state (must be called every loop)
  buttonHandler->update();

  // Update state machine (handles input and rendering)
  if (stateMachine) {
    stateMachine->update();
  }

  // Small delay to prevent overwhelming the system
  delay(50);
}
