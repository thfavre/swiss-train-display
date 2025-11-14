#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include "../DisplayManager.h"
#include "../../../include/AppState.h"
#include "../../../include/Types.h"

// Forward declarations
class PresetManager;
class WiFiManager;
class TrainAPI;

// ====== BASE SCREEN CLASS ======
// All screens inherit from this

class Screen {
protected:
  DisplayManager* display;
  AppState nextState;  // State to transition to (if any)
  bool requestStateChange;
  bool needsRedraw;    // Flag to request redraw

public:
  Screen(DisplayManager* disp);
  virtual ~Screen() {}

  // Lifecycle
  virtual void enter() = 0;     // Called when entering this screen
  virtual void exit() = 0;      // Called when leaving this screen
  virtual void update() = 0;    // Called every frame

  // Input handling
  virtual void handleEncoder(int delta) = 0;
  virtual void handleShortPress() = 0;
  virtual void handleLongPress() = 0;

  // Drawing
  virtual void draw() = 0;

  // State management
  bool hasStateChangeRequest() const { return requestStateChange; }
  AppState getNextState() const { return nextState; }
  void clearStateChangeRequest() { requestStateChange = false; }

  // Redraw management
  bool needsRedrawNow() const { return needsRedraw; }
  void clearRedrawFlag() { needsRedraw = false; }

protected:
  void requestState(AppState state) {
    nextState = state;
    requestStateChange = true;
  }

  void requestRedraw() {
    needsRedraw = true;
  }
};

#endif // SCREEN_H
