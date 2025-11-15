#include "MainScreen.h"

MainScreen::MainScreen(DisplayManager* disp, PresetManager* presetMgr, TrainAPI* api, WiFiManager* wifiMgr)
  : Screen(disp), presets(presetMgr), trainAPI(api), wifi(wifiMgr) {
}

void MainScreen::enter() {
  Serial.println("Entering MainScreen");

  // Don't fetch data here - it blocks for 1-5 seconds!
  // Display will show cached data or "No data" message
  // User can refresh from menu if needed
}

void MainScreen::exit() {
  Serial.println("Exiting MainScreen");
}

void MainScreen::update() {
  const Preset* current = presets->getCurrent();

  // NOTE: Removed auto-refresh to prevent UI blocking
  // Train data is only fetched:
  // - On startup
  // - When manually refreshing from menu
  // This keeps UI responsive

  // Clock needs to update every second
  if (current && current->type == PRESET_CLOCK) {
    static unsigned long lastClockUpdate = 0;
    unsigned long now = millis();
    if (now - lastClockUpdate >= 1000) {  // Every 1 second
      lastClockUpdate = now;
      requestRedraw();
    }
  }
}

void MainScreen::handleEncoder(int delta) {
  if (delta != 0) {
    // Switch presets (skip disabled ones)
    if (delta > 0) {
      presets->nextEnabled();
    } else {
      presets->previousEnabled();
    }

    // Don't fetch data here - it blocks for 1-5 seconds!
    // User can manually refresh from menu if needed
    // This keeps preset switching instant
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
    display->drawCenteredText("No presets", 28, 1);
    display->show();
    return;
  }

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

  // Yellow zone: Route information
  String route = current->fromStation + " -> " + current->toStation;
  YellowBar::draw(*display, route, true, wifi->isConnected());

  // Check if we have cached data
  if (!trainAPI->hasCachedData()) {
    // No data yet
    if (!wifi->isConnected()) {
      display->drawCenteredText("No WiFi", 30, 1);
      display->drawCenteredText("Long press for menu", 42, 1);
    } else {
      display->drawCenteredText("Loading...", 35, 1);
    }
    return;
  }

  // Get all cached connections
  const std::vector<TrainConnection>& connections = trainAPI->getCachedConnections();

  if (connections.empty()) {
    display->drawCenteredText("No connections", 35, 1);
    return;
  }

  // Draw based on number of trains to display
  int trainsToShow = min((int)current->trainsToDisplay, (int)connections.size());

  if (trainsToShow == 1) {
    drawSingleTrain(connections[0]);
  } else if (trainsToShow == 2) {
    drawTwoTrains(connections);
  } else if (trainsToShow == 3) {
    drawThreeTrains(connections);
  } else if (trainsToShow >= 4) {
    drawFourTrains(connections);
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

// ====== MULTI-TRAIN DISPLAY LAYOUTS ======

void MainScreen::drawSingleTrain(const TrainConnection& conn) {
  Adafruit_SSD1306& d = display->getDisplay();

  if (conn.isCancelled) {
    display->drawCenteredText("CANCELLED", 32, 2);
    return;
  }

  // Large departure time
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

  // Calculate duration
  String duration = calculateDuration(conn.departureTime, conn.arrivalTime);

  // Platform and duration
  d.setTextSize(1);
  d.setCursor(2, 45);
  d.print("Pl ");
  d.print(conn.platform);

  d.setCursor(2, 55);
  d.print("Duration: ");
  d.print(duration);
}

void MainScreen::drawTwoTrains(const std::vector<TrainConnection>& connections) {
  Adafruit_SSD1306& d = display->getDisplay();
  d.setTextSize(1);
  d.setTextColor(SSD1306_WHITE);

  // Draw 2 rows
  for (int i = 0; i < min(2, (int)connections.size()); i++) {
    const TrainConnection& conn = connections[i];
    int y = BLUE_ZONE_Y + 2 + (i * 22);  // Each row is 22 pixels

    if (conn.isCancelled) {
      d.setCursor(2, y);
      d.print("CANCELLED");
    } else {
      // Departure time + delay (first line)
      d.setCursor(2, y);
      d.print(conn.departureTime);

      if (conn.delayMinutes > 0) {
        d.setCursor(40, y);
        d.print("+");
        d.print(conn.delayMinutes);
      }

      // Duration (first line, right side)
      String duration = calculateDuration(conn.departureTime, conn.arrivalTime);
      d.setCursor(60, y);
      d.print(duration);

      // Platform (second line)
      d.setCursor(2, y + 10);
      d.print("Pl ");
      d.print(conn.platform);
    }
  }
}

void MainScreen::drawThreeTrains(const std::vector<TrainConnection>& connections) {
  Adafruit_SSD1306& d = display->getDisplay();
  d.setTextColor(SSD1306_WHITE);

  // Compact 3-row list (16 pixels per row)
  for (int i = 0; i < min(3, (int)connections.size()); i++) {
    const TrainConnection& conn = connections[i];
    int y = BLUE_ZONE_Y + 2 + (i * 15);

    if (conn.isCancelled) {
      d.setTextSize(1);
      d.setCursor(2, y);
      d.print("CANCELLED");
    } else {
      // Departure time (slightly bigger)
      d.setTextSize(1);
      d.setCursor(2, y);
      d.print(conn.departureTime);

      if (conn.delayMinutes > 0) {
        d.setCursor(40, y);
        d.print("+");
        d.print(conn.delayMinutes);
      }

      // Platform
      d.setCursor(55, y);
      d.print("Pl");
      d.print(conn.platform);

      // Duration instead of train number
      String duration = calculateDuration(conn.departureTime, conn.arrivalTime);
      d.setCursor(80, y);
      d.print(duration);
    }
  }
}

void MainScreen::drawFourTrains(const std::vector<TrainConnection>& connections) {
  Adafruit_SSD1306& d = display->getDisplay();
  d.setTextSize(1);
  d.setTextColor(SSD1306_WHITE);

  // 2x2 grid layout
  for (int i = 0; i < min(4, (int)connections.size()); i++) {
    const TrainConnection& conn = connections[i];

    // Calculate position (2 columns, 2 rows)
    int col = i % 2;
    int row = i / 2;
    int x = col * 64;  // Each column is 64 pixels wide
    int y = BLUE_ZONE_Y + 2 + (row * 22);  // Each row is 22 pixels

    if (conn.isCancelled) {
      d.setCursor(x + 2, y);
      d.print("CANC");
    } else {
      // Line 1: Departure time
      d.setCursor(x + 2, y);
      d.print(conn.departureTime);

      if (conn.delayMinutes > 0) {
        d.setCursor(x + 40, y);
        d.print("+");
        d.print(conn.delayMinutes);
      }

      // Line 2: Platform
      d.setCursor(x + 2, y + 10);
      d.print("Pl");
      d.print(conn.platform);
    }
  }
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

String MainScreen::calculateDuration(const String& departureTime, const String& arrivalTime) {
  // Parse HH:MM format
  if (departureTime.length() < 5 || arrivalTime.length() < 5) {
    return "?";
  }

  int depHour = departureTime.substring(0, 2).toInt();
  int depMin = departureTime.substring(3, 5).toInt();
  int arrHour = arrivalTime.substring(0, 2).toInt();
  int arrMin = arrivalTime.substring(3, 5).toInt();

  // Convert to minutes
  int depTotalMin = depHour * 60 + depMin;
  int arrTotalMin = arrHour * 60 + arrMin;

  // Handle day wrap-around (if arrival is next day)
  if (arrTotalMin < depTotalMin) {
    arrTotalMin += 24 * 60;
  }

  int durationMin = arrTotalMin - depTotalMin;

  // Format as HHh MM or just XXmin if less than an hour
  int hours = durationMin / 60;
  int mins = durationMin % 60;

  if (hours > 0) {
    return String(hours) + "h" + (mins < 10 ? "0" : "") + String(mins);
  } else {
    return String(mins) + "m";
  }
}
