#include "TrainAPI.h"

TrainAPI::TrainAPI()
  : lastFetchTime(0), cachedFrom(""), cachedTo("") {
  clearError();
}

TrainAPI::~TrainAPI() {
  http.end();
}

// ====== FETCH DATA ======

bool TrainAPI::fetchConnections(const String& from, const String& to, std::vector<TrainConnection>& connections, int limit) {
  clearError();

  // Build URL with limit parameter
  String url = String(API_BASE_URL) + "/connections?from=" + from + "&to=" + to + "&limit=" + String(limit);

  Serial.printf("Fetching train data (limit=%d): %s\n", limit, url.c_str());

#ifdef ESP8266
  http.begin(wifiClient, url);
#else
  http.begin(url);
#endif
  http.setTimeout(7000);  // 7 second timeout for multiple connections

  int httpCode = http.GET();

  if (httpCode != 200) {
    String errorMsg = "HTTP Error: " + String(httpCode);
    Serial.println(errorMsg);

    lastError = ErrorInfo(ERROR_API_REQUEST, errorMsg, url);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Parse the response
  if (!parseConnections(payload, connections, limit)) {
    Serial.println("Failed to parse train data");
    return false;
  }

  // Update cache
  cachedConnections = connections;
  cachedFrom = from;
  cachedTo = to;
  lastFetchTime = millis();

  Serial.printf("Train data fetched: %d connections from %s -> %s\n",
                connections.size(), from.c_str(), to.c_str());

  return true;
}

// Backward compatibility wrapper
bool TrainAPI::fetchConnection(const String& from, const String& to, TrainConnection& connection) {
  std::vector<TrainConnection> connections;
  if (fetchConnections(from, to, connections, 1) && !connections.empty()) {
    connection = connections[0];
    return true;
  }
  return false;
}

// ====== PARSE JSON ======

bool TrainAPI::parseConnections(const String& json, std::vector<TrainConnection>& connections, int limit) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    String errorMsg = "JSON parse error: " + String(error.c_str());
    Serial.println(errorMsg);

    lastError = ErrorInfo(ERROR_API_PARSE, errorMsg, json.substring(0, 100));
    return false;
  }

  // Check if we have connections
  JsonArray connectionsArray = doc["connections"];
  if (connectionsArray.isNull() || connectionsArray.size() == 0) {
    Serial.println("No connections found");
    lastError = ErrorInfo(ERROR_NO_CONNECTIONS, "No connections found", "");
    return false;
  }

  // Clear the output vector
  connections.clear();

  // Parse up to 'limit' connections
  int count = min((int)connectionsArray.size(), limit);
  for (int i = 0; i < count; i++) {
    JsonObject conn = connectionsArray[i];
    if (conn.isNull()) {
      Serial.printf("Invalid connection data at index %d\n", i);
      continue; // Skip invalid connections
    }

    // Create new connection object
    TrainConnection connection;

    // Parse from/to
    JsonObject from = conn["from"];
    JsonObject to = conn["to"];

    if (from.isNull() || to.isNull()) {
      Serial.printf("Missing from/to data at index %d\n", i);
      continue; // Skip this connection
    }

    // Extract times
    const char* depTime = from["departure"];
    const char* arrTime = to["arrival"];

    if (depTime == nullptr || arrTime == nullptr) {
      Serial.printf("Missing departure/arrival times at index %d\n", i);
      continue; // Skip this connection
    }

    connection.departureTime = extractTime(String(depTime));
    connection.arrivalTime = extractTime(String(arrTime));

    // Extract platform
    const char* platform = from["platform"];
    connection.platform = (platform != nullptr) ? String(platform) : "?";

    // Extract train info from first section
    JsonArray sections = conn["sections"];
    if (!sections.isNull() && sections.size() > 0) {
      JsonObject section = sections[0];
      JsonObject journey = section["journey"];

      if (!journey.isNull()) {
        const char* category = journey["category"];
        const char* number = journey["number"];

        if (category != nullptr && number != nullptr) {
          connection.trainNumber = String(category) + " " + String(number);
        } else {
          connection.trainNumber = "Unknown";
        }

        connection.isCancelled = false;
      } else {
        // No journey info - might be a walking section or cancelled
        connection.isCancelled = true;
        connection.trainNumber = "";
      }
    } else {
      connection.isCancelled = true;
      connection.trainNumber = "";
    }

    // Set delay (not provided by this API directly, could be enhanced)
    connection.delayMinutes = 0;

    connection.fetchTime = millis();

    // Add to connections vector
    connections.push_back(connection);
  }

  return !connections.empty();
}

// ====== TIME EXTRACTION ======

String TrainAPI::extractTime(const String& isoTime) {
  // ISO format: 2025-01-14T15:30:00+01:00
  // Extract HH:MM

  if (isoTime.length() < 16) {
    return "??:??";
  }

  int tPos = isoTime.indexOf('T');
  if (tPos < 0) {
    return "??:??";
  }

  // Extract HH:MM starting after 'T'
  String time = isoTime.substring(tPos + 1, tPos + 6);
  return time;
}

// ====== CACHE MANAGEMENT ======

TrainConnection TrainAPI::getCachedConnection() const {
  if (cachedConnections.empty()) {
    return TrainConnection();
  }
  return cachedConnections[0];
}

bool TrainAPI::isCacheValid(unsigned long maxAge) const {
  if (cachedConnections.empty() || cachedConnections[0].fetchTime == 0) {
    return false;
  }

  unsigned long age = millis() - cachedConnections[0].fetchTime;
  return age < maxAge;
}

unsigned long TrainAPI::getTimeSinceLastFetch() const {
  if (lastFetchTime == 0) {
    return 0;
  }
  return millis() - lastFetchTime;
}
