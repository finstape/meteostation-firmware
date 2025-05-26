#include "config.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

unsigned long lastConfigCheck = 0;
const unsigned long CONFIG_CHECK_INTERVAL = 60000;
static unsigned long lastUpload = 0;
static unsigned long dynamicInterval = 5000;  // default

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
    json.trim();

    if (json.length() == 0 || !json.startsWith("{")) {
      Serial.println("⚠️ Пропущен некорректный или пустой JSON");
      return;
    }

    Serial.println("Sending JSON: " + json);

    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;

      HTTPClient http;
      String postUrl = String(SERVER_URL) + "/weather/upload";
      http.begin(client, postUrl);
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

  // Обновляем интервал с сервера
  if (WiFi.status() == WL_CONNECTED && millis() - lastConfigCheck > CONFIG_CHECK_INTERVAL) {
    lastConfigCheck = millis();

    WiFiClient client;
    HTTPClient http;
    String getUrl = String(SERVER_URL) + "/weather/interval";
    http.begin(client, getUrl);

    int code = http.GET();
    if (code == HTTP_CODE_OK) {
      String response = http.getString();
      int pos = response.indexOf("sensor_poll_interval_ms");
      if (pos != -1) {
        int valStart = response.indexOf(':', pos) + 1;
        int valEnd = response.indexOf('}', valStart);
        dynamicInterval = response.substring(valStart, valEnd).toInt();
      }
    }
    http.end();
  }

  // просто ждём перед следующим `Serial.read()` -> `POST`
  delay(dynamicInterval);
}
