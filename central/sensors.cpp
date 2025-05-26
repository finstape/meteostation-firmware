#include "sensors.h"
#include "config.h"

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// DHT21
DHT dht(DHT_PIN, DHT_TYPE);
bool dhtAvailable = false;
float temperature = NAN, humidity = NAN;

// SGP30
Adafruit_SGP30 sgp;
bool sgpAvailable = false;
uint16_t tvoc = 0, co2 = 0;
uint16_t baselineTVOC = 0, baselineCO2 = 0;
unsigned long lastSGPRead = 0;
unsigned long lastBaselineSave = 0;

// BMP180
Adafruit_BMP085_Unified bmp(10180); // ID произвольный
bool bmpAvailable = false;
float pressure = NAN;

void initSensors() {
  dht.begin();
  dhtAvailable = true;

  Wire.begin();

  if (sgp.begin()) {
    sgpAvailable = true;
    Serial.println("SGP30 найден. Подождите 15 секунд...");
    lastSGPRead = millis();
    lastBaselineSave = millis();
  } else {
    Serial.println("SGP30 не найден");
  }

  if (bmp.begin()) {
    bmpAvailable = true;
    Serial.println("BMP180 найден");
  } else {
    Serial.println("BMP180 не найден");
  }

  // НЕ задерживаем работу системы, просто сообщим пользователю
  delay(60000);
}

void updateSensors() {
  // Чтение DHT21
  if (dhtAvailable) {
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    temperature = isnan(newTemp) ? NAN : newTemp;
    humidity    = isnan(newHum) ? NAN : newHum;
  } else {
    temperature = NAN;
    humidity = NAN;
  }

  // Чтение SGP30 — 1 раз в секунду
  if (sgpAvailable && millis() - lastSGPRead >= 1000) {
    lastSGPRead = millis();

    if (sgp.IAQmeasure()) {
      tvoc = sgp.TVOC;
      co2  = sgp.eCO2;
      Serial.print("SGP30: TVOC = "); Serial.print(tvoc);
      Serial.print(" ppb, eCO2 = "); Serial.print(co2); Serial.println(" ppm");
    } else {
      tvoc = 0;
      co2  = 400;
    }
  }

  // Сохранение baseline раз в BASELINE_SAVE_INTERVAL
  if (sgpAvailable && millis() - lastBaselineSave >= BASELINE_SAVE_INTERVAL) {
    lastBaselineSave = millis();
    if (sgp.getIAQBaseline(&baselineCO2, &baselineTVOC)) {
      Serial.print("SGP30 Baseline: ");
      Serial.print(baselineCO2, HEX); Serial.print(" ");
      Serial.println(baselineTVOC, HEX);
    }
  }

  // Чтение BMP180
  if (bmpAvailable) {
    sensors_event_t bmpEvent;
    bmp.getEvent(&bmpEvent);

    if (bmpEvent.pressure) {
      pressure = bmpEvent.pressure * 0.750062;  // мм рт. ст.
    } else {
      pressure = NAN;
    }
  } else {
    pressure = NAN;
  }
}

// Геттеры
float getTemperature()  { return temperature; }
float getHumidity()     { return humidity; }
uint16_t getTVOC()      { return tvoc; }
uint16_t getCO2()       { return co2; }
float getPressure()     { return pressure; }
