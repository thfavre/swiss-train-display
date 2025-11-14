#include "MainScreen.h"

MainScreen::MainScreen(DisplayManager* disp, PresetManager* presetMgr, TrainAPI* api, WiFiManager* wifiMgr)
  : Screen(disp), presets(presetMgr), trainAPI(api), wifi(wifiMgr) {
}

void MainScreen::enter() {
  Serial.println("Entering MainScreen");

  // Fetch train data if we have a train preset and WiFi
  const Preset* current = presets->getCurrent();
  if (current && current->type == PRESET_TRAIN && wifi->isConnected()) {
    TrainConnection conn;
    trainAPI->fetchConnection(current->fromStation, current->toStation, conn);
  }
}

void MainScreen::exit() {
  Serial.println("Exiting MainScreen");
}

void MainScreen::update() {
  // Auto-refresh train data periodically
  const Preset* current = presets->getCurrent();
  if (current && current->type == PRESET_TRAIN && wifi->isConnected()) {
    if (!trainAPI->isCacheValid()) {
      TrainConnection conn;
      trainAPI->fetchConnection(current->fromStation, current->toStation, conn);
    }
  }
}

void MainScreen::handleEncoder(int delta) {
  if (delta != 0) {
    // Switch presets
    if (delta > 0) {
      presets->next();
    } else {
      presets->previous();
    }

    // Fetch data for new preset
    const Preset* current = presets->getCurrent();
    if (current && current->type == PRESET_TRAIN && wifi->isConnected()) {
      TrainConnection conn;
      trainAPI->fetchConnection(current->fromStation, current->toStation, conn);
    }
  }
}

void MainScreen::handleShortPress() {
  // Short press opens preset edit for current preset
  const Preset* current = presets->getCurrent();
  if (current && current->type != PRESET_CLOCK) {
    requestState(STATE_PRESET_EDIT);
  }
}

void MainScreen::handleLongPress() {
  // Long press opens main menu
  requestState(STATE_MENU);
}

void MainScreen::draw() {
  display->clear();

  const Preset* current = presets->getCurrent();
  if (!current) {
    Serial.println("MainScreen: No preset found!");
    display->drawCenteredText("No presets", 28, 1);
    display->show();
    return;
  }

  Serial.print("MainScreen: Drawing preset type ");
  Serial.println(current->type);

  // Draw based on preset type
  switch (current->type) {
    case PRESET_TRAIN:
      drawTrainDisplay();
      break;
    case PRESET_CLOCK:
      drawClockDisplay();
      break;
    case PRESET_WEATHER:
      drawWeatherDisplay();
      break;
    case PRESET_CALENDAR:
      drawCalendarDisplay();
      break;
  }

  display->show();
}

void MainScreen::drawTrainDisplay() {
  const Preset* current = presets->getCurrent();
  Adafruit_SSD1306& d = display->getDisplay();

  Serial.println("Drawing train display");

  // Yellow zone: Route information
  String route = current->fromStation + " -> " + current->toStation;
  Serial.println("Route: " + route);
  YellowBar::draw(*display, route, true, wifi->isConnected());

  // Blue zone: Train info
  const TrainConnection& conn = trainAPI->getCachedConnection();

  if (!trainAPI->hasCachedData()) {
    // No data yet
    Serial.println("No cached train data");
    if (!wifi->isConnected()) {
      Serial.println("Drawing: No WiFi");
      display->drawCenteredText("No WiFi", 30, 1);
      display->drawCenteredText("Long press for menu", 42, 1);
    } else {
      Serial.println("Drawing: Loading");
      display->drawCenteredText("Loading...", 35, 1);
    }
  } else if (conn.isCancelled) {
    // Cancelled
    display->drawCenteredText("CANCELLED", 32, 2);
  } else {
    // Show departure time (large)
    d.setTextSize(2);
    d.setTextColor(SSD1306_WHITE);
    d.setCursor(2, 20);
    d.print(conn.departureTime);

    // Show delay if any
    if (conn.delayMinutes > 0) {
      d.setTextSize(1);
      d.setCursor(85, 24);
      d.print("+");
      d.print(conn.delayMinutes);
      d.print("'");
    }

    // Platform and train number
    d.setTextSize(1);
    d.setCursor(2, 45);
    d.print("Pl ");
    d.print(conn.platform);
    d.print("  ");
    d.print(conn.trainNumber);
  }
}

void MainScreen::drawClockDisplay() {
  const Preset* current = presets->getCurrent();

  // Yellow zone: Title
  YellowBar::draw(*display, current->name, true, wifi->isConnected());

  // Blue zone: Large time display
  String timeStr = getCurrentTime();
  display->drawCenteredText(timeStr, 28, 3);
}

void MainScreen::drawWeatherDisplay() {
  const Preset* current = presets->getCurrent();

  YellowBar::draw(*display, current->name, true, wifi->isConnected());

  // Placeholder
  display->drawCenteredText("Weather Mode", 28, 1);
  display->drawCenteredText("(Coming soon)", 40, 1);
}

void MainScreen::drawCalendarDisplay() {
  const Preset* current = presets->getCurrent();

  YellowBar::draw(*display, current->name, true, wifi->isConnected());

  // Placeholder
  display->drawCenteredText("Calendar Mode", 28, 1);
  display->drawCenteredText("(Coming soon)", 40, 1);
}

String MainScreen::getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "00:00";
  }

  char timeStr[6];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  return String(timeStr);
}
