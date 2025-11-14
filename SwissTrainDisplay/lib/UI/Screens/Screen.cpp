#include "Screen.h"

Screen::Screen(DisplayManager* disp)
  : display(disp), nextState(STATE_MAIN_DISPLAY), requestStateChange(false) {
}
