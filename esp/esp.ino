#include "config.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

void setup() {
  Serial.begin(9600); // связь с Arduino
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // подключаем Wi-Fi

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("\nWiFi connected");
}

void loop() {
  if (Serial.available()) {
    String json = Serial.readStringUntil('\n');  // ждём полную строку
    Serial.println("Received: " + json);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;

      HTTPClient http;
      http.begin(client, SERVER_URL);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("User-Agent", "ESP8266Client");

      int code = http.POST(json);
      Serial.print("HTTP code: ");
      Serial.println(code);

      http.end();
    } else {
      Serial.println("WiFi not connected");
    }
  }
}
