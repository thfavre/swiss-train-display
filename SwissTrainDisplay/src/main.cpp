/*
 * Swiss Train Display for ESP32-S3 - IMPROVED VERSION
 * OLED Display + Rotary Encoder Control
 * 
 * Hardware:
 *   OLED: SDA=GPIO8, SCL=GPIO9 (128x64)
 *   Encoder: CLK=GPIO4, DT=GPIO5, SW=GPIO6
 * 
 * Controls:
 *   - Turn: Navigate menus/options
 *   - Short Press: Select/Confirm
 *   - Long Press (1s): Back/Cancel
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// ====== HARDWARE PINS ======
#define I2C_SDA 8
#define I2C_SCL 9
#define ENCODER_CLK 4
#define ENCODER_DT  5
#define ENCODER_SW  6

// ====== DISPLAY SETTINGS ======
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====== ENCODER VARIABLES ======
volatile int encoderPos = 0;
volatile int lastEncoded = 0;
int lastDisplayedPos = 0;

// Button handling
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
const unsigned long longPressTime = 1000;
bool buttonHandled = false;
bool longPressTriggered = false; // Track if long press action already executed

// ====== APPLICATION STATE ======
enum AppState {
  STATE_MAIN_DISPLAY,
  STATE_MENU,
  STATE_SETTINGS,
  STATE_WIFI_SCAN,
  STATE_WIFI_PASSWORD,
  STATE_PRESET_EDIT
};

AppState currentState = STATE_MAIN_DISPLAY;

// ====== PREFERENCES (EEPROM) ======
Preferences preferences;

// ====== PRESET CONFIGURATION ======
struct Preset {
  String name;
  String fromStation;
  String toStation;
  bool enabled;
};

Preset presets[4] = {
  {"Route 1", "Lausanne", "Geneve", true},
  {"Route 2", "Lausanne", "Bern", true},
  {"Route 3", "Lausanne", "Zurich", true},
  {"Clock", "", "", true}
};

int currentPreset = 0;
int menuSelection = 0;

// Preset editing
enum PresetEditField {
  EDIT_NAME,
  EDIT_FROM,
  EDIT_TO,
  EDIT_SAVE,
  EDIT_COUNT
};
int editingPreset = 0;
int editField = 0;
String editFieldInput = "";
int editCharIndex = 0;

// ====== TRAIN DATA ======
struct TrainConnection {
  String departureTime;
  String arrivalTime;
  String platform;
  String trainNumber;
  int delayMinutes;
  bool isCancelled;
};

TrainConnection currentConnection;
unsigned long lastFetchTime = 0;
const unsigned long fetchInterval = 60000; // 60 seconds
bool dataAvailable = false;

// ====== WIFI ======
String wifiSSID = "";
String wifiPassword = "";
String wifiNetworks[20];
int wifiNetworkCount = 0;
int selectedNetwork = 0;

// ====== SIMPLIFIED KEYBOARD ======
const char keyboardChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%&*()-_=+[]{};:,.<>?";
const int numChars = strlen(keyboardChars);
int charIndex = 0;
String passwordInput = "";
bool showPasswordModal = false;
int passwordModalSelection = 0; // 0=Delete, 1=Back, 2=Save

// Menu definitions
enum MenuOption {
  MENU_SETTINGS,
  MENU_REFRESH,
  MENU_EXIT,
  MENU_COUNT
};

enum SettingsOption {
  SETTINGS_WIFI,
  SETTINGS_BACK,
  SETTINGS_COUNT
};

// ====== FUNCTION PROTOTYPES ======
void IRAM_ATTR updateEncoder();
void handleButton();
void handleShortPress();
void handleLongPress();
void updateDisplay();
void displayMainScreen();
void displayMenu();
void displaySettings();
void displayWiFiScan();
void displayPasswordEntry();
void displayPresetEdit();
void fetchTrainData();
void parseTrainData(String json);
void scanWiFiNetworks();
void connectToWiFi();
void loadSettings();
void saveSettings();
String getTimeString(String isoTime);
void drawProgressBar(int x, int y, int width, int height, int progress, int total);
void drawCenteredText(String text, int y);

// ====== BUTTON HANDLING ======
void handleShortPress() {
  Serial.print("Short press in state: ");
  Serial.println(currentState);
  
  switch(currentState) {
    case STATE_MAIN_DISPLAY:
      // Short press on main screen opens preset editor for current preset
      if (currentPreset != 3) { // Don't edit clock preset
        editingPreset = currentPreset;
        currentState = STATE_PRESET_EDIT;
        editField = 0;
        editFieldInput = "";
        editCharIndex = 0;
        encoderPos = 0;
        lastDisplayedPos = 0;
      }
      break;
      
    case STATE_MENU:
      switch(menuSelection) {
        case MENU_SETTINGS:
          currentState = STATE_SETTINGS;
          menuSelection = 0;
          encoderPos = 0;
          lastDisplayedPos = 0;
          break;
          
        case MENU_REFRESH:
          if (WiFi.status() == WL_CONNECTED && currentPreset != 3) {
            display.clearDisplay();
            drawCenteredText("Refreshing...", 28);
            display.display();
            fetchTrainData();
            delay(500);
          }
          currentState = STATE_MAIN_DISPLAY;
          break;
          
        case MENU_EXIT:
          currentState = STATE_MAIN_DISPLAY;
          break;
      }
      break;
      
    case STATE_SETTINGS:
      switch(menuSelection) {
        case SETTINGS_WIFI:
          currentState = STATE_WIFI_SCAN;
          scanWiFiNetworks();
          break;
          
        case SETTINGS_BACK:
          currentState = STATE_MENU;
          menuSelection = 0;
          encoderPos = 0;
          lastDisplayedPos = 0;
          break;
      }
      break;
      
    case STATE_WIFI_SCAN:
      if (wifiNetworkCount > 0 && menuSelection < wifiNetworkCount) {
        selectedNetwork = menuSelection;
        wifiSSID = wifiNetworks[selectedNetwork];
        passwordInput = "";
        charIndex = 0;
        currentState = STATE_WIFI_PASSWORD;
        encoderPos = 0;
        lastDisplayedPos = 0;
      }
      break;
      
    case STATE_WIFI_PASSWORD:
      if (showPasswordModal) {
        // Handle modal selection
        switch(passwordModalSelection) {
          case 0: // Delete
            if (passwordInput.length() > 0) {
              passwordInput.remove(passwordInput.length() - 1);
              Serial.println("Deleted char, password now: " + passwordInput);
            }
            showPasswordModal = false;
            break;
          case 1: // Back
            currentState = STATE_WIFI_SCAN;
            menuSelection = selectedNetwork;
            encoderPos = selectedNetwork;
            lastDisplayedPos = selectedNetwork;
            showPasswordModal = false;
            break;
          case 2: // Save & Connect
            showPasswordModal = false;
            connectToWiFi();
            break;
        }
      } else {
        // Add character to password
        if (charIndex < numChars) {
          passwordInput += keyboardChars[charIndex];
          Serial.println("Added char, password now: " + passwordInput);
        }
      }
      break;
      
    case STATE_PRESET_EDIT:
      // Add character to current field or save
      if (editField == EDIT_SAVE) {
        // Apply all accumulated changes to the preset
        // (changes were already applied as we typed)
        saveSettings();
        if (WiFi.status() == WL_CONNECTED && currentPreset != 3) {
          fetchTrainData();
        }
        currentState = STATE_MAIN_DISPLAY;
      } else {
        // Add character to field and update preset immediately
        if (editCharIndex < numChars) {
          char newChar = keyboardChars[editCharIndex];
          
          // Update the appropriate field
          if (editField == EDIT_NAME) {
            presets[editingPreset].name += newChar;
          } else if (editField == EDIT_FROM) {
            presets[editingPreset].fromStation += newChar;
          } else if (editField == EDIT_TO) {
            presets[editingPreset].toStation += newChar;
          }
          
          editFieldInput = ""; // Clear temp input since we applied it
        }
      }
      break;
  }
}

void handleLongPress() {
  Serial.print("Long press in state: ");
  Serial.println(currentState);
  
  switch(currentState) {
    case STATE_MAIN_DISPLAY:
      // Long press opens main menu
      currentState = STATE_MENU;
      menuSelection = 0;
      encoderPos = 0;
      lastDisplayedPos = 0;
      break;
      
    case STATE_WIFI_PASSWORD:
      // In password mode, long press opens modal menu
      if (!showPasswordModal) {
        showPasswordModal = true;
        passwordModalSelection = 0;
        encoderPos = 0;
        lastDisplayedPos = 0;
        Serial.println("Opening password modal menu");
      } else {
        // If modal already open, close it
        showPasswordModal = false;
      }
      break;
      
    case STATE_PRESET_EDIT:
      // Delete last character from current field or go back
      if (editField != EDIT_SAVE) {
        String* currentField = nullptr;
        if (editField == EDIT_NAME) currentField = &presets[editingPreset].name;
        else if (editField == EDIT_FROM) currentField = &presets[editingPreset].fromStation;
        else if (editField == EDIT_TO) currentField = &presets[editingPreset].toStation;
        
        if (currentField && currentField->length() > 0) {
          currentField->remove(currentField->length() - 1);
        } else {
          // No more characters, go back
          currentState = STATE_MAIN_DISPLAY;
        }
      } else {
        // On save button, go back without saving
        currentState = STATE_MAIN_DISPLAY;
      }
      break;
      
    case STATE_MENU:
      currentState = STATE_MAIN_DISPLAY;
      break;
      
    case STATE_SETTINGS:
      currentState = STATE_MENU;
      menuSelection = 0;
      encoderPos = 0;
      lastDisplayedPos = 0;
      break;
      
    case STATE_WIFI_SCAN:
      currentState = STATE_SETTINGS;
      menuSelection = 0;
      encoderPos = 0;
      lastDisplayedPos = 0;
      break;
      
    default:
      currentState = STATE_MAIN_DISPLAY;
  }
}

// ====== DISPLAY FUNCTIONS ======
void updateDisplay() {
  // Handle encoder on main display for preset switching
  if (currentState == STATE_MAIN_DISPLAY) {
    if (encoderPos != lastDisplayedPos) {
      int diff = encoderPos - lastDisplayedPos;
      currentPreset += diff;
      
      if (currentPreset < 0) currentPreset = 3;
      if (currentPreset > 3) currentPreset = 0;
      
      lastDisplayedPos = encoderPos;
      
      // Fetch new data if switching to a train preset
      if (WiFi.status() == WL_CONNECTED && currentPreset != 3) {
        fetchTrainData();
      }
    }
  }
  
  switch(currentState) {
    case STATE_MAIN_DISPLAY:
      displayMainScreen();
      break;
    case STATE_MENU:
      displayMenu();
      break;
    case STATE_SETTINGS:
      displaySettings();
      break;
    case STATE_WIFI_SCAN:
      displayWiFiScan();
      break;
    case STATE_WIFI_PASSWORD:
      displayPasswordEntry();
      break;
    case STATE_PRESET_EDIT:
      displayPresetEdit();
      break;
  }
}

void drawCenteredText(String text, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(text);
}

void drawProgressBar(int x, int y, int width, int height, int progress, int total) {
  display.drawRect(x, y, width, height, SSD1306_WHITE);
  int fillWidth = (progress * (width - 2)) / total;
  if (fillWidth > 0) {
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
  }
}

void displayMainScreen() {
  display.clearDisplay();
  
  // Title bar with preset name
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print(presets[currentPreset].name);
  
  // WiFi indicator
  if (WiFi.status() == WL_CONNECTED) {
    display.fillCircle(120, 5, 3, SSD1306_BLACK);
  }
  
  display.setTextColor(SSD1306_WHITE);
  
  if (currentPreset == 3) {
    // Clock mode
    display.setTextSize(3);
    String timeStr = getTimeString("");
    drawCenteredText(timeStr, 25);
  } else {
    // Train display
    if (!dataAvailable) {
      display.setTextSize(1);
      if (WiFi.status() != WL_CONNECTED) {
        display.setCursor(10, 20);
        display.println("No WiFi Connection");
        display.setCursor(10, 35);
        display.println("Press button for");
        display.setCursor(10, 45);
        display.println("settings menu");
      } else {
        drawCenteredText("Loading trains...", 28);
      }
    } else {
      // Route
      display.setTextSize(1);
      display.setCursor(2, 14);
      display.print(presets[currentPreset].fromStation);
      display.print(" -> ");
      display.print(presets[currentPreset].toStation);
      
      if (currentConnection.isCancelled) {
        display.setTextSize(2);
        drawCenteredText("CANCELLED", 32);
      } else {
        // Departure time (large)
        display.setTextSize(2);
        display.setCursor(2, 28);
        display.print(currentConnection.departureTime);
        
        // Delay indicator
        if (currentConnection.delayMinutes > 0) {
          display.setTextSize(1);
          display.setCursor(85, 32);
          display.print("+");
          display.print(currentConnection.delayMinutes);
          display.print("'");
        }
        
        // Platform and train number
        display.setTextSize(1);
        display.setCursor(2, 50);
        display.print("Pl ");
        display.print(currentConnection.platform);
        display.print("  ");
        display.print(currentConnection.trainNumber);
      }
    }
  }
  
  // Footer hint
  display.setCursor(2, 56);
  display.setTextSize(1);
  if (currentPreset != 3) {
    display.print("Turn=Switch Hold=Menu");
  } else {
    display.print("Hold for Menu");
  }
  
  display.display();
}

void displayMenu() {
  // Handle encoder navigation
  if (encoderPos != lastDisplayedPos) {
    int diff = encoderPos - lastDisplayedPos;
    menuSelection += diff;
    
    if (menuSelection < 0) menuSelection = MENU_COUNT - 1;
    if (menuSelection >= MENU_COUNT) menuSelection = 0;
    
    lastDisplayedPos = encoderPos;
  }
  
  display.clearDisplay();
  
  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print("MAIN MENU");
  
  display.setTextColor(SSD1306_WHITE);
  
  // Menu items
  String menuItems[] = {
    "Settings",
    "Refresh Data",
    "< Back"
  };
  
  int itemHeight = 15;
  int startY = 16;
  
  for (int i = 0; i < MENU_COUNT; i++) {
    int yPos = startY + (i * itemHeight);
    
    if (i == menuSelection) {
      // Highlight selected item
      display.fillRect(0, yPos, SCREEN_WIDTH, itemHeight, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(4, yPos + 3);
      display.print("> ");
    } else {
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(8, yPos + 3);
    }
    
    display.print(menuItems[i]);
  }
  
  display.display();
}

void displaySettings() {
  // Handle encoder navigation
  if (encoderPos != lastDisplayedPos) {
    int diff = encoderPos - lastDisplayedPos;
    menuSelection += diff;
    
    if (menuSelection < 0) menuSelection = SETTINGS_COUNT - 1;
    if (menuSelection >= SETTINGS_COUNT) menuSelection = 0;
    
    lastDisplayedPos = encoderPos;
  }
  
  display.clearDisplay();
  
  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print("SETTINGS");
  
  display.setTextColor(SSD1306_WHITE);
  
  // Settings items
  String settingsItems[] = {
    "WiFi Setup",
    "< Back to Menu"
  };
  
  int itemHeight = 20;
  int startY = 18;
  
  for (int i = 0; i < SETTINGS_COUNT; i++) {
    int yPos = startY + (i * itemHeight);
    
    if (i == menuSelection) {
      display.fillRect(0, yPos, SCREEN_WIDTH, itemHeight, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(4, yPos + 5);
      display.print("> ");
    } else {
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(8, yPos + 5);
    }
    
    display.print(settingsItems[i]);
  }
  
  display.display();
}

void displayWiFiScan() {
  // Handle encoder navigation
  if (encoderPos != lastDisplayedPos) {
    int diff = encoderPos - lastDisplayedPos;
    menuSelection += diff;
    
    if (menuSelection < 0) menuSelection = wifiNetworkCount - 1;
    if (menuSelection >= wifiNetworkCount) menuSelection = 0;
    
    lastDisplayedPos = encoderPos;
  }
  
  display.clearDisplay();
  
  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print("WiFi Networks");
  
  display.setTextColor(SSD1306_WHITE);
  
  if (wifiNetworkCount == 0) {
    drawCenteredText("No networks found", 28);
  } else {
    // Show up to 5 networks
    int visibleStart = max(0, menuSelection - 2);
    int visibleEnd = min(wifiNetworkCount, visibleStart + 5);
    
    int itemHeight = 10;
    int startY = 14;
    
    for (int i = visibleStart; i < visibleEnd; i++) {
      int yPos = startY + ((i - visibleStart) * itemHeight);
      
      if (i == menuSelection) {
        display.fillRect(0, yPos, SCREEN_WIDTH, itemHeight, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(2, yPos + 1);
        display.print(">");
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      
      display.setCursor(10, yPos + 1);
      display.print(wifiNetworks[i]);
    }
    
    // Scroll indicator
    if (wifiNetworkCount > 5) {
      display.setTextColor(SSD1306_WHITE);
      int scrollY = 14 + ((menuSelection * 40) / wifiNetworkCount);
      display.fillRect(126, scrollY, 2, 8, SSD1306_WHITE);
    }
  }
  
  display.display();
}

void displayPasswordEntry() {
  display.clearDisplay();
  
  // If modal is open, show it
  if (showPasswordModal) {
    // Handle encoder for modal navigation
    if (encoderPos != lastDisplayedPos) {
      int diff = encoderPos - lastDisplayedPos;
      passwordModalSelection += diff;
      
      if (passwordModalSelection < 0) passwordModalSelection = 2;
      if (passwordModalSelection > 2) passwordModalSelection = 0;
      
      lastDisplayedPos = encoderPos;
    }
    
    // Draw semi-transparent background (just a border)
    display.drawRect(10, 10, 108, 44, SSD1306_WHITE);
    display.drawRect(11, 11, 106, 42, SSD1306_WHITE);
    
    // Modal title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(40, 14);
    display.print("OPTIONS");
    
    // Modal options
    String options[] = {"Delete Char", "< Cancel", "Save & Connect"};
    int startY = 24;
    
    for (int i = 0; i < 3; i++) {
      int yPos = startY + (i * 10);
      
      if (i == passwordModalSelection) {
        display.fillRect(14, yPos - 1, 100, 9, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(16, yPos);
        display.print(">");
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      
      display.setCursor(24, yPos);
      display.print(options[i]);
    }
    
  } else {
    // Normal password entry mode
    
    // Handle encoder for character selection
    if (encoderPos != lastDisplayedPos) {
      int diff = encoderPos - lastDisplayedPos;
      charIndex += diff;
      
      // Wrap around character list
      if (charIndex < 0) charIndex = numChars - 1;
      if (charIndex >= numChars) charIndex = 0;
      
      lastDisplayedPos = encoderPos;
    }
    
    // Title bar with network name
    display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
    display.setTextSize(1);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(2, 2);
    display.print(wifiSSID.substring(0, 15));
    
    // Password display area - SHOW ACTUAL TEXT
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 14);
    display.print("Pass: ");
    
    // Show actual password (last 10 chars)
    int passLen = passwordInput.length();
    String displayPass = passwordInput.substring(max(0, passLen - 10));
    display.print(displayPass);
    display.print("_"); // Cursor
    
    // Character selector area
    display.setTextSize(1);
    display.setCursor(2, 28);
    display.print("Select character:");
    
    // Show 5 characters: [prev2] [prev1] [CURRENT] [next1] [next2]
    display.setTextSize(2);
    
    int positions[] = {-2, -1, 0, 1, 2};
    int xPositions[] = {10, 35, 60, 85, 110};
    
    for (int i = 0; i < 5; i++) {
      int idx = (charIndex + positions[i] + numChars) % numChars;
      
      if (positions[i] == 0) {
        // Current character - highlighted
        display.fillRect(xPositions[i] - 8, 38, 20, 18, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      
      display.setCursor(xPositions[i] - 4, 40);
      display.print(keyboardChars[idx]);
    }
    
    // Instructions
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 56);
    display.print("Hold=Menu");
  }
  
  display.display();
}

void displayPresetEdit() {
  // Handle encoder navigation
  if (encoderPos != lastDisplayedPos) {
    int diff = encoderPos - lastDisplayedPos;
    
    // If navigating between fields
    if (diff > 5 || diff < -5 || editField == EDIT_SAVE) {
      editField += (diff > 0 ? 1 : -1);
      if (editField < 0) editField = EDIT_COUNT - 1;
      if (editField >= EDIT_COUNT) editField = 0;
      encoderPos = 0; // Reset encoder for character selection
    } else {
      // Navigate characters within current field
      if (editField != EDIT_SAVE) {
        editCharIndex += diff;
        if (editCharIndex < 0) editCharIndex = numChars - 1;
        if (editCharIndex >= numChars) editCharIndex = 0;
      }
    }
    
    lastDisplayedPos = encoderPos;
  }
  
  display.clearDisplay();
  
  // Title bar
  display.fillRect(0, 0, SCREEN_WIDTH, 11, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(2, 2);
  display.print("Edit Route ");
  display.print(editingPreset + 1);
  
  display.setTextColor(SSD1306_WHITE);
  
  // Field labels and values
  String fieldNames[] = {"Name:", "From:", "To:", "< Save"};
  int yPositions[] = {14, 26, 38, 52};
  
  for (int i = 0; i < EDIT_COUNT; i++) {
    int yPos = yPositions[i];
    
    if (i == editField) {
      display.setTextColor(SSD1306_BLACK);
      display.fillRect(0, yPos - 1, SCREEN_WIDTH, 11, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    
    display.setCursor(2, yPos);
    display.print(fieldNames[i]);
    
    if (i != EDIT_SAVE) {
      display.print(" ");
      String* currentField = nullptr;
      if (i == EDIT_NAME) currentField = &presets[editingPreset].name;
      else if (i == EDIT_FROM) currentField = &presets[editingPreset].fromStation;
      else if (i == EDIT_TO) currentField = &presets[editingPreset].toStation;
      
      if (currentField) {
        // Show current value
        String displayText = *currentField;
        if (i == editField) {
          // Show last 6 chars + character selector
          displayText = displayText.substring(max(0, (int)displayText.length() - 6));
          display.print(displayText);
          display.print("[");
          display.print(keyboardChars[editCharIndex]);
          display.print("]");
        } else {
          // Show first 10 chars
          displayText = displayText.substring(0, 10);
          display.print(displayText);
        }
      }
    }
  }
  
  display.display();
}

// ====== WIFI FUNCTIONS ======
void scanWiFiNetworks() {
  display.clearDisplay();
  display.setTextSize(1);
  drawCenteredText("Scanning WiFi...", 28);
  display.display();
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  int n = WiFi.scanNetworks();
  wifiNetworkCount = min(n, 20);
  
  for (int i = 0; i < wifiNetworkCount; i++) {
    wifiNetworks[i] = WiFi.SSID(i);
  }
  
  menuSelection = 0;
  encoderPos = 0;
  lastDisplayedPos = 0;
  
  Serial.print("Found ");
  Serial.print(wifiNetworkCount);
  Serial.println(" networks");
}

void connectToWiFi() {
  display.clearDisplay();
  display.setTextSize(1);
  drawCenteredText("Connecting...", 20);
  drawCenteredText(wifiSSID, 35);
  display.display();
  
  wifiPassword = passwordInput;
  saveSettings();
  
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    display.clearDisplay();
    drawCenteredText("Connecting...", 20);
    drawProgressBar(14, 35, 100, 8, attempts, 20);
    display.display();
    attempts++;
  }
  
  display.clearDisplay();
  if (WiFi.status() == WL_CONNECTED) {
    drawCenteredText("Connected!", 28);
    Serial.println("WiFi Connected!");
  } else {
    drawCenteredText("Failed!", 20);
    drawCenteredText("Check password", 35);
    Serial.println("WiFi Failed!");
  }
  display.display();
  delay(2000);
  
  currentState = STATE_MAIN_DISPLAY;
}

// ====== TRAIN DATA ======
void fetchTrainData() {
  if (WiFi.status() != WL_CONNECTED || currentPreset == 3) {
    return;
  }
  
  HTTPClient http;
  String url = "http://transport.opendata.ch/v1/connections?from=" + 
               presets[currentPreset].fromStation + 
               "&to=" + presets[currentPreset].toStation + 
               "&limit=1";
  
  Serial.println("Fetching: " + url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    parseTrainData(payload);
    dataAvailable = true;
    lastFetchTime = millis();
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

void parseTrainData(String json) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return;
  }
  
  JsonObject connection = doc["connections"][0];
  
  if (connection.isNull()) {
    currentConnection.isCancelled = true;
    return;
  }
  
  JsonObject from = connection["from"];
  JsonObject to = connection["to"];
  
  currentConnection.departureTime = getTimeString(from["departure"].as<String>());
  currentConnection.arrivalTime = getTimeString(to["arrival"].as<String>());
  currentConnection.platform = from["platform"].as<String>();
  
  JsonObject section = connection["sections"][0];
  JsonObject journey = section["journey"];
  
  if (!journey.isNull()) {
    currentConnection.trainNumber = String(journey["category"].as<const char*>()) + " " + 
                                   String(journey["number"].as<const char*>());
    currentConnection.isCancelled = false;
    currentConnection.delayMinutes = 0;
  } else {
    currentConnection.isCancelled = true;
  }
  
  Serial.println("Data parsed successfully");
}

String getTimeString(String isoTime) {
  if (isoTime.length() == 0) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return "00:00";
    }
    char timeStr[6];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    return String(timeStr);
  }
  
  int tPos = isoTime.indexOf('T');
  if (tPos > 0) {
    String time = isoTime.substring(tPos + 1, tPos + 6);
    return time;
  }
  
  return isoTime;
}

// ====== SETTINGS STORAGE ======
void loadSettings() {
  preferences.begin("trainDisplay", false);
  wifiSSID = preferences.getString("ssid", "");
  wifiPassword = preferences.getString("password", "");
  
  for (int i = 0; i < 3; i++) {
    String keyFrom = "from" + String(i);
    String keyTo = "to" + String(i);
    
    presets[i].fromStation = preferences.getString(keyFrom.c_str(), presets[i].fromStation);
    presets[i].toStation = preferences.getString(keyTo.c_str(), presets[i].toStation);
  }
  
  preferences.end();
  
  Serial.println("Settings loaded");
  Serial.println("WiFi SSID: " + wifiSSID);
}

void saveSettings() {
  preferences.begin("trainDisplay", false);
  preferences.putString("ssid", wifiSSID);
  preferences.putString("password", wifiPassword);
  
  for (int i = 0; i < 3; i++) {
    String keyFrom = "from" + String(i);
    String keyTo = "to" + String(i);
    
    preferences.putString(keyFrom.c_str(), presets[i].fromStation);
    preferences.putString(keyTo.c_str(), presets[i].toStation);
  }
  
  preferences.end();
  
  Serial.println("Settings saved");
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== Swiss Train Display ===");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED init failed!");
    while(1);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  drawCenteredText("Swiss", 15);
  drawCenteredText("Trains", 35);
  display.display();
  
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), updateEncoder, CHANGE);
  
  loadSettings();
  
  delay(2000);
  
  if (wifiSSID.length() > 0) {
    display.clearDisplay();
    display.setTextSize(1);
    drawCenteredText("Connecting WiFi...", 28);
    display.display();
    
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi Connected!");
      
      // Configure time with NTP
      configTime(3600, 3600, "pool.ntp.org", "time.nist.gov"); // GMT+1 with DST for Switzerland
      Serial.println("Syncing time with NTP...");
      
      display.clearDisplay();
      drawCenteredText("WiFi Connected!", 28);
      display.display();
      delay(1500);
      
      fetchTrainData();
    } else {
      Serial.println("\nWiFi Failed!");
      display.clearDisplay();
      drawCenteredText("WiFi Failed!", 20);
      drawCenteredText("Check settings", 35);
      display.display();
      delay(2000);
    }
  }
  
  Serial.println("Ready!");
}

// ====== MAIN LOOP ======
void loop() {
  handleButton();
  updateDisplay();
  
  if (WiFi.status() == WL_CONNECTED && 
      currentState == STATE_MAIN_DISPLAY &&
      currentPreset != 3 &&
      millis() - lastFetchTime > fetchInterval) {
    fetchTrainData();
  }
  
  delay(50);
}

// ====== ENCODER INTERRUPT ======
void IRAM_ATTR updateEncoder() {
  int MSB = digitalRead(ENCODER_CLK);
  int LSB = digitalRead(ENCODER_DT);
  
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;
  
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderPos++;
  }
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderPos--;
  }
  
  lastEncoded = encoded;
}

// ====== BUTTON HANDLING ======
void handleButton() {
  int reading = digitalRead(ENCODER_SW);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      if (buttonState == LOW && !buttonHandled) {
        buttonPressTime = millis();
        longPressTriggered = false;
      }
      
      if (buttonState == HIGH && !buttonHandled) {
        unsigned long pressDuration = millis() - buttonPressTime;
        
        // Only trigger short press if long press wasn't already triggered
        if (!longPressTriggered && pressDuration < longPressTime) {
          Serial.println("Short press detected");
          handleShortPress();
        }
        buttonHandled = true;
        longPressTriggered = false;
      }
    } else if (buttonState == HIGH) {
      buttonHandled = false;
    }
    
    // Check for long press WHILE button is still held
    if (buttonState == LOW && !longPressTriggered && !buttonHandled) {
      if (millis() - buttonPressTime >= longPressTime) {
        Serial.println("Long press detected (while held)");
        handleLongPress();
        longPressTriggered = true;
        buttonHandled = true;
      }
    }
  }
  
  lastButtonState = reading;
}
