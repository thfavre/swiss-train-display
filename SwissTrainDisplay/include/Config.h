#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ====== HARDWARE PINS ======
#define I2C_SDA 4
#define I2C_SCL 5
#define ENCODER_CLK 14
#define ENCODER_DT 12
#define ENCODER_SW 13

// ====== DISPLAY SETTINGS ======
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// ====== DISPLAY ZONES (2-color OLED) ======
// Top 16 pixels are YELLOW
// Bottom 48 pixels are BLUE
#define YELLOW_ZONE_HEIGHT 16
#define BLUE_ZONE_Y 16
#define BLUE_ZONE_HEIGHT 48

// ====== TIMING CONSTANTS ======
#define ENCODER_DEBOUNCE_MS 5
#define BUTTON_DEBOUNCE_MS 50
#define LONG_PRESS_MS 1000
#define TRAIN_FETCH_INTERVAL_MS 60000  // 60 seconds

// ====== UI CONSTANTS ======
#define MAX_VISIBLE_MENU_ITEMS 5
#define CHAR_SELECTOR_VISIBLE_COUNT 5
#define MENU_ITEM_HEIGHT 10
#define TITLE_BAR_PADDING 2

// ====== CHARACTER SET FOR INPUT ======
const char KEYBOARD_CHARS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !@#$%&*()-_=+[]{};:,.<>?";
const int KEYBOARD_CHARS_COUNT = sizeof(KEYBOARD_CHARS) - 1; // -1 for null terminator

// Station name character set (lowercase letters only, auto-capitalize)
const char STATION_CHARS[] = "abcdefghijklmnopqrstuvwxyz ";
const int STATION_CHARS_COUNT = sizeof(STATION_CHARS) - 1;

// ====== NETWORK SETTINGS ======
#define WIFI_CONNECT_TIMEOUT_MS 10000  // 10 seconds
#define WIFI_SCAN_MAX_NETWORKS 20
#define API_BASE_URL "http://transport.opendata.ch/v1"

// ====== NTP SETTINGS ======
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"
#define TIMEZONE_OFFSET_SEC 3600   // GMT+1
#define DAYLIGHT_OFFSET_SEC 3600   // DST for Switzerland

// ====== STORAGE KEYS ======
#define PREFS_NAMESPACE "trainDisplay"
#define PREFS_KEY_SSID "ssid"
#define PREFS_KEY_PASSWORD "password"
#define PREFS_KEY_PRESET_COUNT "presetCount"
#define PREFS_KEY_CURRENT_PRESET "currentPreset"
#define PREFS_KEY_PRESET_PREFIX "preset_"

#endif // CONFIG_H
