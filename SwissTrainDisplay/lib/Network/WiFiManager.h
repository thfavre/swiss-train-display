#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <vector>
#include "../../include/Config.h"
#include "../../include/Types.h"

class WiFiManager {
private:
  std::vector<WiFiNetwork> networks;
  String currentSSID;
  String currentPassword;
  ErrorInfo lastError;
  bool isConnecting;

public:
  WiFiManager();

  // Scanning
  bool scan();
  int getNetworkCount() const { return networks.size(); }
  const WiFiNetwork* getNetwork(int index) const;
  const std::vector<WiFiNetwork>& getNetworks() const { return networks; }

  // Connection
  bool connect(const String& ssid, const String& password, unsigned long timeout = WIFI_CONNECT_TIMEOUT_MS);
  bool disconnect();
  bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
  bool isConnectingNow() const { return isConnecting; }

  // Info
  String getSSID() const { return currentSSID; }
  String getIP() const;
  int getRSSI() const;

  // Error handling
  const ErrorInfo& getLastError() const { return lastError; }
  bool hasError() const { return lastError.type != ERROR_NONE; }
  void clearError() { lastError = ErrorInfo(); }

  // Auto-reconnect
  bool autoReconnect();
};

#endif // WIFIMANAGER_H
