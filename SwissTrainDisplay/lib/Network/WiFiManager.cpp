#include "WiFiManager.h"

WiFiManager::WiFiManager()
  : currentSSID(""), currentPassword(""), isConnecting(false) {
  clearError();
  WiFi.mode(WIFI_STA);
}

// ====== SCANNING ======

bool WiFiManager::scan() {
  clearError();
  networks.clear();

  Serial.println("Scanning WiFi networks...");

  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();

  if (n < 0) {
    lastError = ErrorInfo(ERROR_WIFI_SCAN, "WiFi scan failed");
    Serial.println("WiFi scan failed");
    return false;
  }

  Serial.printf("Found %d networks\n", n);

  int maxNetworks = min(n, WIFI_SCAN_MAX_NETWORKS);
  for (int i = 0; i < maxNetworks; i++) {
    WiFiNetwork network;
    network.ssid = WiFi.SSID(i);
    network.rssi = WiFi.RSSI(i);
    network.isSecure = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);

    networks.push_back(network);
    Serial.printf("  %d: %s (%d dBm) %s\n",
                  i, network.ssid.c_str(), network.rssi,
                  network.isSecure ? "[Secure]" : "[Open]");
  }

  return true;
}

const WiFiNetwork* WiFiManager::getNetwork(int index) const {
  if (index < 0 || index >= (int)networks.size()) {
    return nullptr;
  }
  return &networks[index];
}

// ====== CONNECTION ======

bool WiFiManager::connect(const String& ssid, const String& password, unsigned long timeout) {
  clearError();
  isConnecting = true;

  Serial.println("Connecting to: " + ssid);

  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  isConnecting = false;

  if (WiFi.status() != WL_CONNECTED) {
    lastError = ErrorInfo(ERROR_WIFI_CONNECT, "Connection timeout", ssid);
    Serial.println("Connection failed: timeout");
    return false;
  }

  currentSSID = ssid;
  currentPassword = password;

  Serial.println("WiFi connected!");
  Serial.println("IP: " + WiFi.localIP().toString());

  // Configure NTP for time sync
  configTime(TIMEZONE_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);

  return true;
}

bool WiFiManager::disconnect() {
  Serial.println("Disconnecting WiFi");
  WiFi.disconnect();
  currentSSID = "";
  currentPassword = "";
  return true;
}

// ====== INFO ======

String WiFiManager::getIP() const {
  if (WiFi.status() != WL_CONNECTED) {
    return "0.0.0.0";
  }
  return WiFi.localIP().toString();
}

int WiFiManager::getRSSI() const {
  if (WiFi.status() != WL_CONNECTED) {
    return 0;
  }
  return WiFi.RSSI();
}

// ====== AUTO-RECONNECT ======

bool WiFiManager::autoReconnect() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;  // Already connected
  }

  if (currentSSID.length() == 0) {
    return false;  // No credentials to reconnect with
  }

  Serial.println("Auto-reconnecting to WiFi...");
  return connect(currentSSID, currentPassword);
}
