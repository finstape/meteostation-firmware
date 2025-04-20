#include "sensors.h"
#include "config.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>

// --- DHT21 ---
DHT dht(DHT_PIN, DHT_TYPE);
float temperature = NAN;
float humidity = NAN;

// --- SGP30 ---
Adafruit_SGP30 sgp;
bool sgpAvailable = false;
uint16_t tvoc = 0;
uint16_t co2 = 0;
uint16_t baselineTVOC = 0;
uint16_t baselineCO2 = 0;

unsigned long lastSensorRead = 0;
unsigned long lastBaselineSave = 0;

void initSensors() {
  dht.begin();
  Wire.begin();

  if (sgp.begin()) {
    sgpAvailable = true;
    Serial.println("SGP30 инициализирован");

    // Восстановление ранее сохранённого baseline (пример — заглушка)
    sgp.setIAQBaseline(0x8B60, 0x8AD3);
  } else {
    Serial.println("SGP30 не найден :(");
  }
}

void updateSensors() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;

    // --- Чтение DHT21 ---
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (!isnan(newTemp)) temperature = newTemp;
    if (!isnan(newHum)) humidity = newHum;

    // --- Чтение SGP30 ---
    if (sgpAvailable && sgp.IAQmeasure()) {
      tvoc = sgp.TVOC;
      co2 = sgp.eCO2;
    }

    // --- Сохранение baseline SGP30 раз в час ---
    // SGP30 требует периодической рекалибровки
    // Без этого через сутки точность начнёт плыть
    if (sgpAvailable && currentMillis - lastBaselineSave >= BASELINE_SAVE_INTERVAL) {
      lastBaselineSave = currentMillis;
      if (sgp.getIAQBaseline(&baselineCO2, &baselineTVOC)) {
        Serial.print("[Baseline сохранён] eCO2: 0x");
        Serial.print(baselineCO2, HEX);
        Serial.print(", TVOC: 0x");
        Serial.println(baselineTVOC, HEX);
      }
    }

    // --- Вывод в Serial ---
    Serial.println("===== Данные с датчиков =====");
    Serial.print("Температура: "); Serial.print(temperature); Serial.println(" *C");
    Serial.print("Влажность: "); Serial.print(humidity); Serial.println(" %");

    if (sgpAvailable) {
      Serial.print("CO2: "); Serial.print(co2); Serial.println(" ppm");
      Serial.print("TVOC: "); Serial.print(tvoc); Serial.println(" ppb");
    } else {
      Serial.println("SGP30 не отвечает.");
    }

    Serial.println("=============================\n");
  }
}

float getTemperature() {
  return temperature;
}

float getHumidity() {
  return humidity;
}

uint16_t getTVOC() {
  return tvoc;
}

uint16_t getCO2() {
  return co2;
}
