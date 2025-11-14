#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "../../include/Config.h"

class DisplayManager {
private:
  Adafruit_SSD1306 display;
  bool initialized;

public:
  DisplayManager();

  // Initialize display
  bool begin();

  // Get raw display object (for advanced usage)
  Adafruit_SSD1306& getDisplay() { return display; }
  const Adafruit_SSD1306& getDisplay() const { return display; }

  // Basic operations
  void clear();
  void show();
  void clearYellowZone();
  void clearBlueZone();

  // Drawing helpers
  void drawText(const String& text, int x, int y, int size = 1, bool inverted = false);
  void drawCenteredText(const String& text, int y, int size = 1, bool inverted = false);
  void drawRightAlignedText(const String& text, int y, int size = 1, bool inverted = false);

  // Zone helpers
  bool isInYellowZone(int y) const { return y < YELLOW_ZONE_HEIGHT; }
  bool isInBlueZone(int y) const { return y >= BLUE_ZONE_Y; }

  // Shape helpers
  void drawRect(int x, int y, int w, int h, bool filled = false);
  void drawCircle(int x, int y, int r, bool filled = false);
  void drawLine(int x0, int y0, int x1, int y1);

  // Status
  bool isInitialized() const { return initialized; }
};

#endif // DISPLAYMANAGER_H
