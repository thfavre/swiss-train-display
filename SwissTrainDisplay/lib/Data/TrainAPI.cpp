#include "TrainAPI.h"

TrainAPI::TrainAPI()
  : lastFetchTime(0), cachedFrom(""), cachedTo("") {
  clearError();
}

TrainAPI::~TrainAPI() {
  http.end();
}

// ====== FETCH DATA ======

bool TrainAPI::fetchConnection(const String& from, const String& to, TrainConnection& connection) {
  clearError();

  // Build URL
  String url = String(API_BASE_URL) + "/connections?from=" + from + "&to=" + to + "&limit=1";

  Serial.println("Fetching train data: " + url);

  http.begin(url);
  http.setTimeout(5000);  // 5 second timeout

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
  if (!parseConnection(payload, connection)) {
    Serial.println("Failed to parse train data");
    return false;
  }

  // Update cache
  cachedConnection = connection;
  cachedFrom = from;
  cachedTo = to;
  lastFetchTime = millis();

  Serial.printf("Train data fetched: %s -> %s at %s\n",
                from.c_str(), to.c_str(), connection.departureTime.c_str());

  return true;
}

// ====== PARSE JSON ======

bool TrainAPI::parseConnection(const String& json, TrainConnection& connection) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    String errorMsg = "JSON parse error: " + String(error.c_str());
    Serial.println(errorMsg);

    lastError = ErrorInfo(ERROR_API_PARSE, errorMsg, json.substring(0, 100));
    return false;
  }

  // Check if we have connections
  JsonArray connections = doc["connections"];
  if (connections.isNull() || connections.size() == 0) {
    Serial.println("No connections found");
    lastError = ErrorInfo(ERROR_NO_CONNECTIONS, "No connections found", "");
    return false;
  }

  JsonObject conn = connections[0];
  if (conn.isNull()) {
    Serial.println("Invalid connection data");
    lastError = ErrorInfo(ERROR_API_PARSE, "Invalid connection data", "");
    return false;
  }

  // Parse from/to
  JsonObject from = conn["from"];
  JsonObject to = conn["to"];

  if (from.isNull() || to.isNull()) {
    Serial.println("Missing from/to data");
    lastError = ErrorInfo(ERROR_API_PARSE, "Missing from/to data", "");
    return false;
  }

  // Extract times
  const char* depTime = from["departure"];
  const char* arrTime = to["arrival"];

  if (depTime == nullptr || arrTime == nullptr) {
    Serial.println("Missing departure/arrival times");
    lastError = ErrorInfo(ERROR_API_PARSE, "Missing times", "");
    return false;
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

  return true;
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

bool TrainAPI::isCacheValid(unsigned long maxAge) const {
  if (cachedConnection.fetchTime == 0) {
    return false;
  }

  unsigned long age = millis() - cachedConnection.fetchTime;
  return age < maxAge;
}

unsigned long TrainAPI::getTimeSinceLastFetch() const {
  if (lastFetchTime == 0) {
    return 0;
  }
  return millis() - lastFetchTime;
}
