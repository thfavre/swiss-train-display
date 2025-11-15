#include "UIComponents.h"
#include <time.h>

// ====== YELLOW BAR ======

void YellowBar::draw(DisplayManager& disp, const String& title, bool showWiFi, bool wifiConnected) {
  Adafruit_SSD1306& d = disp.getDisplay();

  // Fill yellow zone with white (inverted for visibility)
  d.fillRect(0, 0, SCREEN_WIDTH, YELLOW_ZONE_HEIGHT, SSD1306_WHITE);

  // Draw title
  d.setTextSize(1);
  d.setTextColor(SSD1306_BLACK);
  d.setCursor(TITLE_BAR_PADDING, TITLE_BAR_PADDING);
  d.print(title);

  // Draw WiFi indicator if requested
  if (showWiFi) {
    Icons::drawWiFi(disp, 120, 8, wifiConnected);
  }
}

void YellowBar::drawWithTime(DisplayManager& disp, const String& title) {
  Adafruit_SSD1306& d = disp.getDisplay();

  // Fill yellow zone
  d.fillRect(0, 0, SCREEN_WIDTH, YELLOW_ZONE_HEIGHT, SSD1306_WHITE);

  // Draw title on left
  d.setTextSize(1);
  d.setTextColor(SSD1306_BLACK);
  d.setCursor(TITLE_BAR_PADDING, TITLE_BAR_PADDING);
  d.print(title);

  // Draw time on right
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 100)) {  // 100ms timeout - don't block UI
    char timeStr[6];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);

    int16_t x1, y1;
    uint16_t w, h;
    d.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
    int x = SCREEN_WIDTH - w - TITLE_BAR_PADDING;

    d.setCursor(x, TITLE_BAR_PADDING);
    d.print(timeStr);
  }
}

// ====== MENU LIST ======

MenuList::MenuList() : selectedIndex(0), scrollOffset(0) {
}

void MenuList::draw(DisplayManager& disp, const String items[], int itemCount, int yStart) {
  Adafruit_SSD1306& d = disp.getDisplay();

  updateScroll(itemCount);

  int visibleCount = min(itemCount - scrollOffset, MAX_VISIBLE_MENU_ITEMS);

  for (int i = 0; i < visibleCount; i++) {
    int itemIndex = i + scrollOffset;
    int yPos = yStart + (i * MENU_ITEM_HEIGHT);

    bool isSelected = (itemIndex == selectedIndex);

    if (isSelected) {
      // Highlight selected item
      d.fillRect(0, yPos, SCREEN_WIDTH, MENU_ITEM_HEIGHT, SSD1306_WHITE);
      d.setTextColor(SSD1306_BLACK);
      d.setCursor(4, yPos + 1);
      d.print(">");
    } else {
      d.setTextColor(SSD1306_WHITE);
    }

    d.setCursor(isSelected ? 12 : 8, yPos + 1);
    d.print(items[itemIndex]);
  }

  // Draw scroll indicator if needed
  if (itemCount > MAX_VISIBLE_MENU_ITEMS) {
    int scrollBarHeight = (MAX_VISIBLE_MENU_ITEMS * MENU_ITEM_HEIGHT * MAX_VISIBLE_MENU_ITEMS) / itemCount;
    int scrollBarY = yStart + ((selectedIndex * (MAX_VISIBLE_MENU_ITEMS * MENU_ITEM_HEIGHT - scrollBarHeight)) / (itemCount - 1));

    d.fillRect(126, scrollBarY, 2, scrollBarHeight, SSD1306_WHITE);
  }
}

void MenuList::draw(DisplayManager& disp, const MenuItem items[], int itemCount, int yStart) {
  // Convert to String array and call other draw
  String labels[itemCount];
  for (int i = 0; i < itemCount; i++) {
    labels[i] = items[i].label;
  }
  draw(disp, labels, itemCount, yStart);
}

void MenuList::updateScroll(int itemCount, int maxVisible) {
  // Keep selected item visible
  if (selectedIndex < scrollOffset) {
    scrollOffset = selectedIndex;
  }
  if (selectedIndex >= scrollOffset + maxVisible) {
    scrollOffset = selectedIndex - maxVisible + 1;
  }

  // Clamp scroll offset
  scrollOffset = max(0, min(scrollOffset, itemCount - maxVisible));
}

// ====== CHARACTER SELECTOR ======

void CharacterSelector::draw(DisplayManager& disp, const char* charset, int charsetSize,
                              int currentIndex, int yPos) {
  Adafruit_SSD1306& d = disp.getDisplay();

  d.setTextSize(2);

  int positions[] = {-2, -1, 0, 1, 2};
  int xPositions[] = {10, 35, 60, 85, 110};

  for (int i = 0; i < 5; i++) {
    int idx = (currentIndex + positions[i] + charsetSize) % charsetSize;

    if (positions[i] == 0) {
      // Current character - highlighted
      d.fillRect(xPositions[i] - 8, yPos, 20, 18, SSD1306_WHITE);
      d.setTextColor(SSD1306_BLACK);
    } else {
      d.setTextColor(SSD1306_WHITE);
    }

    d.setCursor(xPositions[i] - 4, yPos + 2);
    d.print(charset[idx]);
  }
}

// ====== MODAL DIALOG ======

void ModalDialog::draw(DisplayManager& disp, const String& title, const String& content,
                       const String buttons[], int buttonCount, int selectedButton) {
  Adafruit_SSD1306& d = disp.getDisplay();

  // Background
  d.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_BLACK);

  // Border - full screen with 1px margin
  d.drawRect(1, 1, SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, SSD1306_WHITE);

  // Title
  d.setTextSize(1);
  d.setTextColor(SSD1306_WHITE);
  d.setCursor(4, 5);
  d.print(title);

  // Content area
  d.setCursor(4, 16);
  d.print(content);

  // Separator line
  d.drawLine(3, SCREEN_HEIGHT - 18, SCREEN_WIDTH - 3, SCREEN_HEIGHT - 18, SSD1306_WHITE);

  // Buttons - fit within border
  int buttonWidth = (buttonCount <= 2) ? 40 : 24;
  int totalButtonWidth = buttonCount * buttonWidth;
  int availableSpace = SCREEN_WIDTH - 10;  // Leave margin for border
  int buttonSpacing = max(2, (availableSpace - totalButtonWidth) / (buttonCount + 1));

  for (int i = 0; i < buttonCount; i++) {
    int xPos = 5 + buttonSpacing + (i * (buttonWidth + buttonSpacing));
    int yPos = SCREEN_HEIGHT - 12;

    if (i == selectedButton) {
      d.fillRect(xPos, yPos - 2, buttonWidth, 10, SSD1306_WHITE);
      d.setTextColor(SSD1306_BLACK);
    } else {
      d.setTextColor(SSD1306_WHITE);
    }

    d.setCursor(xPos + 2, yPos);
    d.print(buttons[i]);
  }
}

void ModalDialog::drawConfirm(DisplayManager& disp, const String& title, const String& content,
                               int selectedOption) {
  const String buttons[] = {"Cancel", "OK"};
  draw(disp, title, content, buttons, 2, selectedOption);
}

// ====== PROGRESS BAR ======

void ProgressBar::draw(DisplayManager& disp, int x, int y, int width, int height,
                       int progress, int total) {
  Adafruit_SSD1306& d = disp.getDisplay();

  d.drawRect(x, y, width, height, SSD1306_WHITE);

  int fillWidth = (progress * (width - 2)) / total;
  if (fillWidth > 0) {
    d.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
  }
}

// ====== TEXT INPUT DISPLAY ======

void TextInputDisplay::draw(DisplayManager& disp, const String& label, const String& text,
                             int y, int maxChars) {
  Adafruit_SSD1306& d = disp.getDisplay();

  d.setTextSize(1);
  d.setTextColor(SSD1306_WHITE);
  d.setCursor(2, y);
  d.print(label);
  d.print(" ");

  // Show last N characters if text is too long
  String displayText = text;
  if ((int)text.length() > maxChars) {
    displayText = text.substring(text.length() - maxChars);
  }

  d.print(displayText);
  d.print("_");  // Cursor
}

// ====== ICONS ======

void Icons::drawWiFi(DisplayManager& disp, int x, int y, bool connected) {
  Adafruit_SSD1306& d = disp.getDisplay();

  if (connected) {
    // Draw WiFi waves (simple filled circle for now)
    d.fillCircle(x, y, 3, SSD1306_BLACK);
  } else {
    // Draw X or empty
    d.drawCircle(x, y, 3, SSD1306_BLACK);
  }
}

void Icons::drawError(DisplayManager& disp, int x, int y) {
  Adafruit_SSD1306& d = disp.getDisplay();

  // Draw X
  d.drawLine(x - 3, y - 3, x + 3, y + 3, SSD1306_WHITE);
  d.drawLine(x + 3, y - 3, x - 3, y + 3, SSD1306_WHITE);
}

void Icons::drawWarning(DisplayManager& disp, int x, int y) {
  Adafruit_SSD1306& d = disp.getDisplay();

  // Draw triangle
  d.drawLine(x, y - 4, x - 4, y + 4, SSD1306_WHITE);
  d.drawLine(x, y - 4, x + 4, y + 4, SSD1306_WHITE);
  d.drawLine(x - 4, y + 4, x + 4, y + 4, SSD1306_WHITE);

  // Draw exclamation mark
  d.drawPixel(x, y, SSD1306_WHITE);
  d.drawPixel(x, y - 2, SSD1306_WHITE);
}

void Icons::drawCheck(DisplayManager& disp, int x, int y) {
  Adafruit_SSD1306& d = disp.getDisplay();

  // Draw checkmark
  d.drawLine(x - 3, y, x - 1, y + 2, SSD1306_WHITE);
  d.drawLine(x - 1, y + 2, x + 3, y - 2, SSD1306_WHITE);
}
