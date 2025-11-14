#ifndef UICOMPONENTS_H
#define UICOMPONENTS_H

#include <Arduino.h>
#include "DisplayManager.h"
#include "../../include/Config.h"
#include "../../include/Types.h"

// ====== YELLOW BAR COMPONENT ======
// Draws title/status bar in yellow zone (top 16px)

class YellowBar {
public:
  static void draw(DisplayManager& disp, const String& title, bool showWiFi = false, bool wifiConnected = false);
  static void drawWithTime(DisplayManager& disp, const String& title);
};

// ====== MENU LIST COMPONENT ======
// Scrollable menu with selection highlighting

class MenuList {
private:
  int selectedIndex;
  int scrollOffset;

public:
  MenuList();

  void draw(DisplayManager& disp, const String items[], int itemCount, int yStart = BLUE_ZONE_Y);
  void draw(DisplayManager& disp, const MenuItem items[], int itemCount, int yStart = BLUE_ZONE_Y);

  void setSelected(int index) { selectedIndex = index; }
  int getSelected() const { return selectedIndex; }

  void updateScroll(int itemCount, int maxVisible = MAX_VISIBLE_MENU_ITEMS);
};

// ====== CHARACTER SELECTOR COMPONENT ======
// 5-character carousel for text input

class CharacterSelector {
public:
  static void draw(DisplayManager& disp, const char* charset, int charsetSize,
                   int currentIndex, int yPos = 38);
};

// ====== MODAL DIALOG COMPONENT ======
// Generic modal with border and button options

class ModalDialog {
public:
  static void draw(DisplayManager& disp, const String& title, const String& content,
                   const String buttons[], int buttonCount, int selectedButton);

  static void drawConfirm(DisplayManager& disp, const String& title, const String& content,
                          int selectedOption);  // 0=Cancel, 1=OK
};

// ====== PROGRESS BAR COMPONENT ======

class ProgressBar {
public:
  static void draw(DisplayManager& disp, int x, int y, int width, int height,
                   int progress, int total);
};

// ====== TEXT INPUT DISPLAY ======
// Shows current input text with cursor

class TextInputDisplay {
public:
  static void draw(DisplayManager& disp, const String& label, const String& text,
                   int y, int maxChars = 12);
};

// ====== ICON HELPERS ======

class Icons {
public:
  static void drawWiFi(DisplayManager& disp, int x, int y, bool connected);
  static void drawError(DisplayManager& disp, int x, int y);
  static void drawWarning(DisplayManager& disp, int x, int y);
  static void drawCheck(DisplayManager& disp, int x, int y);
};

#endif // UICOMPONENTS_H
