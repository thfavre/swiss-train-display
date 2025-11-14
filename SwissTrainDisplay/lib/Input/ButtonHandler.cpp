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
      if (currentState == LOW && !eventHandled) {
        isPressed = true;
        pressStartTime = millis();
        longPressTriggered = false;
      }

      // Button released
      if (currentState == HIGH && isPressed) {
        isPressed = false;
        eventHandled = false;
        longPressTriggered = false;
      }
    } else if (currentState == HIGH) {
      eventHandled = false;
    }

    // Check for long press while button is held
    if (isPressed && !longPressTriggered && !eventHandled) {
      if (millis() - pressStartTime >= LONG_PRESS_MS) {
        longPressTriggered = true;
        eventHandled = true;
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
    Serial.println("Button: LONG PRESS");
  }
  // Short press detected (button released without long press)
  else if (!isPressed && !longPressTriggered && !eventHandled) {
    unsigned long pressDuration = millis() - pressStartTime;
    if (pressDuration > BUTTON_DEBOUNCE_MS && pressDuration < LONG_PRESS_MS) {
      event = BUTTON_SHORT_PRESS;
      eventHandled = true;
      Serial.println("Button: SHORT PRESS");
    }
  }

  return event;
}

bool ButtonHandler::hasEvent() const {
  return (longPressTriggered && !eventHandled) ||
         (!isPressed && !longPressTriggered && !eventHandled);
}

unsigned long ButtonHandler::getPressDuration() const {
  if (!isPressed) {
    return 0;
  }
  return millis() - pressStartTime;
}
