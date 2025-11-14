#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Types.h"

class ButtonHandler {
private:
  int pin;
  int lastState;
  int currentState;
  unsigned long pressStartTime;
  unsigned long lastDebounceTime;
  bool isPressed;
  bool longPressTriggered;
  bool eventHandled;

public:
  ButtonHandler(int buttonPin = ENCODER_SW);

  // Initialize
  bool begin();

  // Update - call in loop()
  void update();

  // Check for events
  ButtonEvent getEvent();
  bool hasEvent() const;

  // State queries
  bool isPressedNow() const { return isPressed; }
  unsigned long getPressDuration() const;
};

#endif // BUTTONHANDLER_H
