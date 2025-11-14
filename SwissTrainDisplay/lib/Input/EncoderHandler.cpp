#include "EncoderHandler.h"

// Static instance for ISR
EncoderHandler* EncoderHandler::instance = nullptr;

EncoderHandler::EncoderHandler()
  : position(0), rawCount(0), lastEncoded(0),
    lastInterruptTime(0), lastReadPosition(0) {
  instance = this;
}

bool EncoderHandler::begin() {
  Serial.println("Initializing encoder...");

  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), handleInterrupt, CHANGE);

  Serial.println("Encoder initialized");
  return true;
}

void IRAM_ATTR EncoderHandler::handleInterrupt() {
  if (instance == nullptr) {
    return;
  }

  // Debouncing
  unsigned long currentTime = millis();
  if (currentTime - instance->lastInterruptTime < ENCODER_DEBOUNCE_MS) {
    return;
  }
  instance->lastInterruptTime = currentTime;

  // Read encoder pins
  int MSB = digitalRead(ENCODER_CLK);
  int LSB = digitalRead(ENCODER_DT);

  int encoded = (MSB << 1) | LSB;
  int sum = (instance->lastEncoded << 2) | encoded;

  // Determine direction based on state transition
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    instance->rawCount++;
  }
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    instance->rawCount--;
  }

  // Update position (one position per 2 raw counts = one per detent/click)
  instance->position = instance->rawCount / 2;

  instance->lastEncoded = encoded;
}

int EncoderHandler::getDelta() {
  int delta = position - lastReadPosition;
  lastReadPosition = position;
  return delta;
}

void EncoderHandler::resetPosition() {
  noInterrupts();
  position = 0;
  rawCount = 0;
  lastReadPosition = 0;
  interrupts();
}

void EncoderHandler::setPosition(int pos) {
  noInterrupts();
  position = pos;
  rawCount = pos * 2;
  lastReadPosition = pos;
  interrupts();
}
