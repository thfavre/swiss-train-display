#ifndef TRAINAPI_H
#define TRAINAPI_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "../../include/Config.h"
#include "../../include/Types.h"

class TrainAPI {
private:
  HTTPClient http;
  TrainConnection cachedConnection;
  String cachedFrom;
  String cachedTo;
  unsigned long lastFetchTime;
  ErrorInfo lastError;

  // Parse JSON response into TrainConnection
  bool parseConnection(const String& json, TrainConnection& connection);

  // Extract time from ISO format
  String extractTime(const String& isoTime);

public:
  TrainAPI();
  ~TrainAPI();

  // Fetch train data
  bool fetchConnection(const String& from, const String& to, TrainConnection& connection);

  // Get cached data
  const TrainConnection& getCachedConnection() const { return cachedConnection; }
  bool hasCachedData() const { return cachedConnection.fetchTime > 0; }
  bool isCacheValid(unsigned long maxAge = TRAIN_FETCH_INTERVAL_MS) const;

  // Error handling
  const ErrorInfo& getLastError() const { return lastError; }
  bool hasError() const { return lastError.type != ERROR_NONE; }
  void clearError() { lastError = ErrorInfo(); }

  // Utility
  unsigned long getTimeSinceLastFetch() const;
};

#endif // TRAINAPI_H
