#ifndef ERRORSCREEN_H
#define ERRORSCREEN_H

#include "Screen.h"
#include "../UIComponents.h"

class ErrorScreen : public Screen {
private:
  ErrorInfo error;
  unsigned long displayTime;

public:
  ErrorScreen(DisplayManager* disp);

  void setError(const ErrorInfo& err) { error = err; }

  void enter() override;
  void exit() override;
  void update() override;
  void handleEncoder(int delta) override;
  void handleShortPress() override;
  void handleLongPress() override;
  void draw() override;
};

#endif // ERRORSCREEN_H
