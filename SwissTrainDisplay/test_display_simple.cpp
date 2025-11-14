/*
 * SIMPLE DISPLAY TEST
 * Use this to verify your OLED is working
 *
 * To use:
 * 1. Temporarily rename src/main.cpp to src/main.cpp.bak
 * 2. Copy this file to src/main.cpp
 * 3. Build and upload
 * 4. You should see "Hello World!" on the display
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define I2C_SDA 8
#define I2C_SCL 9
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\nSimple Display Test");

  Wire.begin(I2C_SDA, I2C_SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Display init FAILED!");
    while(1) delay(1000);
  }

  Serial.println("Display OK!");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Hello");
  display.setCursor(10, 40);
  display.println("World!");
  display.display();

  Serial.println("Text drawn - check display!");
}

void loop() {
  // Blink every second to show it's running
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    Serial.println("Still running...");
    lastBlink = millis();
  }
  delay(100);
}
