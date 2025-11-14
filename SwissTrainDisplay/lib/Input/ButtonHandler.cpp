#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(int buttonPin)
  : pin(buttonPin), lastState(HIGH), currentState(HIGH),
    pressStartTime(0), lastDebounceTime(0),
    isPressed(false), longPressTriggered(false), eventHandled(false) {
}

bool ButtonHandler::begin() {
  Serial.println("Initializing button...");

  pinMode(pin, INPUT_PULLUP);
  currentState = digitalRead(pin);
  lastState = currentState;

  Serial.println("Button initialized");
  return true;
}

void ButtonHandler::update() {
  int reading = digitalRead(pin);

  // Debouncing
  if (reading != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    if (reading != currentState) {
      currentState = reading;

      // Button pressed (LOW because of pullup)
      if (currentState == LOW) {
        if (!isPressed) {
          isPressed = true;
          pressStartTime = millis();
          longPressTriggered = false;
          eventHandled = false;
          Serial.println("Button pressed");
        }
      }

      // Button released
      if (currentState == HIGH && isPressed) {
        Serial.println("Button released");
        isPressed = false;
        // Don't reset eventHandled here - let getEvent() consume it
      }
    }

    // Check for long press while button is held
    if (isPressed && !longPressTriggered && !eventHandled) {
      if (millis() - pressStartTime >= LONG_PRESS_MS) {
        longPressTriggered = true;
        Serial.println("Long press threshold reached");
      }
    }
  }

  lastState = reading;
}

ButtonEvent ButtonHandler::getEvent() {
  ButtonEvent event = BUTTON_NONE;

  // Long press detected
  if (longPressTriggered && !eventHandled) {
    event = BUTTON_LONG_PRESS;
    eventHandled = true;
    Serial.println("Button: LONG PRESS event returned");
  }
  // Short press detected (button released without long press)
  else if (!isPressed && !longPressTriggered && !eventHandled && pressStartTime > 0) {
    unsigned long pressDuration = millis() - pressStartTime;
    if (pressDuration > BUTTON_DEBOUNCE_MS && pressDuration < LONG_PRESS_MS) {
      event = BUTTON_SHORT_PRESS;
      eventHandled = true;
      pressStartTime = 0;  // Reset to prevent re-triggering
      Serial.println("Button: SHORT PRESS event returned");
    } else if (pressDuration >= LONG_PRESS_MS) {
      // Was a long press but already handled, just clean up
      eventHandled = true;
      pressStartTime = 0;
    }
  }

  return event;
}

bool ButtonHandler::hasEvent() const {
  return (longPressTriggered && !eventHandled) ||
         (!isPressed && !longPressTriggered && !eventHandled && pressStartTime > 0);
}

unsigned long ButtonHandler::getPressDuration() const {
  if (!isPressed) {
    return 0;
  }
  return millis() - pressStartTime;
}
