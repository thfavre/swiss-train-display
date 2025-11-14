#include "DisplayManager.h"

DisplayManager::DisplayManager()
  : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), initialized(false) {
}

bool DisplayManager::begin() {
  Serial.println("Initializing display...");

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("ERROR: Display init failed");
    return false;
  }

  initialized = true;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();

  Serial.println("Display initialized");
  return true;
}

// ====== BASIC OPERATIONS ======

void DisplayManager::clear() {
  display.clearDisplay();
}

void DisplayManager::show() {
  display.display();
}

void DisplayManager::clearYellowZone() {
  display.fillRect(0, 0, SCREEN_WIDTH, YELLOW_ZONE_HEIGHT, SSD1306_BLACK);
}

void DisplayManager::clearBlueZone() {
  display.fillRect(0, BLUE_ZONE_Y, SCREEN_WIDTH, BLUE_ZONE_HEIGHT, SSD1306_BLACK);
}

// ====== DRAWING HELPERS ======

void DisplayManager::drawText(const String& text, int x, int y, int size, bool inverted) {
  display.setTextSize(size);
  display.setTextColor(inverted ? SSD1306_BLACK : SSD1306_WHITE);
  display.setCursor(x, y);
  display.print(text);
}

void DisplayManager::drawCenteredText(const String& text, int y, int size, bool inverted) {
  display.setTextSize(size);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = (SCREEN_WIDTH - w) / 2;

  display.setTextColor(inverted ? SSD1306_BLACK : SSD1306_WHITE);
  display.setCursor(x, y);
  display.print(text);
}

void DisplayManager::drawRightAlignedText(const String& text, int y, int size, bool inverted) {
  display.setTextSize(size);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  int x = SCREEN_WIDTH - w - 2;  // 2px margin from right

  display.setTextColor(inverted ? SSD1306_BLACK : SSD1306_WHITE);
  display.setCursor(x, y);
  display.print(text);
}

// ====== SHAPE HELPERS ======

void DisplayManager::drawRect(int x, int y, int w, int h, bool filled) {
  if (filled) {
    display.fillRect(x, y, w, h, SSD1306_WHITE);
  } else {
    display.drawRect(x, y, w, h, SSD1306_WHITE);
  }
}

void DisplayManager::drawCircle(int x, int y, int r, bool filled) {
  if (filled) {
    display.fillCircle(x, y, r, SSD1306_WHITE);
  } else {
    display.drawCircle(x, y, r, SSD1306_WHITE);
  }
}

void DisplayManager::drawLine(int x0, int y0, int x1, int y1) {
  display.drawLine(x0, y0, x1, y1, SSD1306_WHITE);
}
