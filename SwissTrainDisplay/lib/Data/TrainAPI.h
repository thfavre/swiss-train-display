#ifndef TRAINAPI_H
#define TRAINAPI_H

#include <Arduino.h>
#ifdef ESP32
  #include <HTTPClient.h>
#elif defined(ESP8266)
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif
#include <ArduinoJson.h>
#include <vector>
#include "../../include/Config.h"
#include "../../include/Types.h"

class TrainAPI {
private:
  HTTPClient http;
#ifdef ESP8266
  WiFiClient wifiClient;
#endif
  std::vector<TrainConnection> cachedConnections;
  String cachedFrom;
  String cachedTo;
  unsigned long lastFetchTime;
  ErrorInfo lastError;

  // Parse JSON response into TrainConnections
  bool parseConnections(const String& json, std::vector<TrainConnection>& connections, int limit);

  // Extract time from ISO format
  String extractTime(const String& isoTime);

public:
  TrainAPI();
  ~TrainAPI();

  // Fetch train data
  bool fetchConnections(const String& from, const String& to, std::vector<TrainConnection>& connections, int limit = 1);

  // Backward compatibility - fetch single connection
  bool fetchConnection(const String& from, const String& to, TrainConnection& connection);

  // Get cached data
  const std::vector<TrainConnection>& getCachedConnections() const { return cachedConnections; }
  TrainConnection getCachedConnection() const; // Returns first connection or empty
  bool hasCachedData() const { return !cachedConnections.empty() && cachedConnections[0].fetchTime > 0; }
  bool isCacheValid(unsigned long maxAge = TRAIN_FETCH_INTERVAL_MS) const;

  // Error handling
  const ErrorInfo& getLastError() const { return lastError; }
  bool hasError() const { return lastError.type != ERROR_NONE; }
  void clearError() { lastError = ErrorInfo(); }

  // Utility
  unsigned long getTimeSinceLastFetch() const;
};

#endif // TRAINAPI_H
