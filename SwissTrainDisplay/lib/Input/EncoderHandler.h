#ifndef ENCODERHANDLER_H
#define ENCODERHANDLER_H

#include <Arduino.h>
#include "../../include/Config.h"
#include "../../include/Types.h"

class EncoderHandler {
private:
  static EncoderHandler* instance;  // For ISR

  volatile int position;
  volatile int rawCount;
  volatile int lastEncoded;
  volatile unsigned long lastInterruptTime;

  int lastReadPosition;

  // ISR function
  static void IRAM_ATTR handleInterrupt();

public:
  EncoderHandler();

  // Initialize pins and attach interrupts
  bool begin();

  // Read encoder
  int getPosition() const { return position; }
  int getDelta();  // Get change since last read
  bool hasChanged() const { return position != lastReadPosition; }

  // Reset position
  void resetPosition();
  void setPosition(int pos);
};

#endif // ENCODERHANDLER_H
