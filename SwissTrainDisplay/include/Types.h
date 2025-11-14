#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// ====== PRESET TYPES ======

enum PresetType {
  PRESET_TRAIN,     // Train route display
  PRESET_CLOCK,     // Clock display
  PRESET_WEATHER,   // Weather display (future)
  PRESET_CALENDAR   // Calendar display (future)
};

struct Preset {
  String name;           // Display name
  PresetType type;       // Type of preset
  String fromStation;    // For train presets
  String toStation;      // For train presets
  bool enabled;          // Whether preset is active

  // Constructor for easy initialization
  Preset() : name(""), type(PRESET_TRAIN), fromStation(""), toStation(""), enabled(true) {}

  Preset(String n, String from, String to)
    : name(n), type(PRESET_TRAIN), fromStation(from), toStation(to), enabled(true) {}

  Preset(String n, PresetType t)
    : name(n), type(t), fromStation(""), toStation(""), enabled(true) {}
};

// ====== TRAIN DATA TYPES ======

struct TrainConnection {
  String departureTime;    // HH:MM format
  String arrivalTime;      // HH:MM format
  String platform;         // Platform number/letter
  String trainNumber;      // e.g., "IC 1234"
  int delayMinutes;        // Delay in minutes
  bool isCancelled;        // Whether connection is cancelled
  unsigned long fetchTime; // Timestamp when data was fetched

  TrainConnection()
    : departureTime(""), arrivalTime(""), platform(""),
      trainNumber(""), delayMinutes(0), isCancelled(false), fetchTime(0) {}

  bool isValid() const {
    return departureTime.length() > 0 && !isCancelled;
  }

  bool isStale(unsigned long maxAge) const {
    return (millis() - fetchTime) > maxAge;
  }
};

// ====== INPUT TYPES ======

enum ButtonEvent {
  BUTTON_NONE,
  BUTTON_SHORT_PRESS,
  BUTTON_LONG_PRESS
};

struct EncoderEvent {
  int delta;              // Change in position
  int position;           // Absolute position
  unsigned long timestamp; // When event occurred

  EncoderEvent() : delta(0), position(0), timestamp(0) {}
  EncoderEvent(int d, int p) : delta(d), position(p), timestamp(millis()) {}
};

// ====== ERROR TYPES ======

enum ErrorType {
  ERROR_NONE,
  ERROR_WIFI_CONNECT,
  ERROR_WIFI_SCAN,
  ERROR_API_REQUEST,
  ERROR_API_PARSE,
  ERROR_NO_CONNECTIONS,
  ERROR_STORAGE,
  ERROR_DISPLAY_INIT
};

struct ErrorInfo {
  ErrorType type;
  String message;
  String detail;
  unsigned long timestamp;

  ErrorInfo() : type(ERROR_NONE), message(""), detail(""), timestamp(0) {}
  ErrorInfo(ErrorType t, String msg, String det = "")
    : type(t), message(msg), detail(det), timestamp(millis()) {}
};

// ====== WIFI TYPES ======

struct WiFiNetwork {
  String ssid;
  int rssi;              // Signal strength
  bool isSecure;         // Whether network requires password

  WiFiNetwork() : ssid(""), rssi(0), isSecure(false) {}
  WiFiNetwork(String s, int r, bool sec) : ssid(s), rssi(r), isSecure(sec) {}
};

// ====== MENU TYPES ======

struct MenuItem {
  String label;
  int id;
  void* data;            // Optional data pointer

  MenuItem() : label(""), id(0), data(nullptr) {}
  MenuItem(String l, int i) : label(l), id(i), data(nullptr) {}
};

#endif // TYPES_H
