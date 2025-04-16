#include "sensors.h"
#include "config.h"
#include <DHT.h>

// Создаём объект датчика
DHT dht(DHT_PIN, DHT_TYPE);

// Переменные для хранения последних показаний
float temperature = NAN;
float humidity = NAN;

// Время последнего опроса
unsigned long lastSensorRead = 0;

void initSensors() {
  dht.begin();
}

void updateSensors() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (!isnan(newTemp)) {
      temperature = newTemp;
    }

    if (!isnan(newHum)) {
      humidity = newHum;
    }

    Serial.print("Температура: ");
    Serial.print(temperature);
    Serial.print(" *C | Влажность: ");
    Serial.print(humidity);
    Serial.println(" %");
  }
}

float getTemperature() {
  return temperature;
}

float getHumidity() {
  return humidity;
}
