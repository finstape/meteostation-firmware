#include "sensors.h"
#include "config.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// --- DHT21 ---
// Создаём объект для работы с датчиком температуры и влажности
DHT dht(DHT_PIN, DHT_TYPE);
bool dhtAvailable = false;  // Флаг доступности датчика
float temperature = NAN;
float humidity = NAN;

// --- SGP30 ---
// Создаём объект для работы с датчиком качества воздуха (TVOC и eCO2)
Adafruit_SGP30 sgp;
bool sgpAvailable = false;
uint16_t tvoc = 0;
uint16_t co2 = 0;
uint16_t baselineTVOC = 0;
uint16_t baselineCO2 = 0;

// --- BMP180 ---
// Создаём объект для работы с барометром (давление и температура)
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10180);  // ID произвольный
bool bmpAvailable = false;
float pressure = NAN;
float bmpTemperature = NAN;

// --- Таймеры ---
unsigned long lastSensorRead = 0;       // Время последнего обновления данных с датчиков
unsigned long lastBaselineSave = 0;     // Время последнего сохранения baseline для SGP30

// --- Инициализация всех датчиков ---
void initSensors() {
  // Инициализация DHT21
  dht.begin();
  dhtAvailable = true;  // Предполагаем, что DHT всегда запускается (нет метода проверки)

  // Инициализация I2C
  Wire.begin();

  // Инициализация SGP30
  if (sgp.begin()) {
    sgpAvailable = true;
    Serial.println("SGP30 инициализирован");

    // Восстановление baseline (заглушка)
    sgp.setIAQBaseline(0x8B60, 0x8AD3);
  } else {
    Serial.println("SGP30 не найден :(");
  }

  // Инициализация BMP180
  if (bmp.begin()) {
    bmpAvailable = true;
    Serial.println("BMP180 инициализирован");
  } else {
    Serial.println("BMP180 не найден :(");
  }
}

// --- Основной цикл обновления данных ---
void updateSensors() {
  unsigned long currentMillis = millis();

  // Проверка, пришло ли время обновить данные
  if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
    lastSensorRead = currentMillis;

    // --- Чтение DHT21 ---
    if (dhtAvailable) {
      float newTemp = dht.readTemperature();
      float newHum = dht.readHumidity();

      // Обновляем значения только если они валидны
      if (!isnan(newTemp)) temperature = newTemp;
      if (!isnan(newHum)) humidity = newHum;
    }

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

    // --- Чтение BMP180 ---
    if (bmpAvailable) {
      sensors_event_t bmpEvent;
      bmp.getEvent(&bmpEvent);  // Чтение давления

      if (bmpEvent.pressure) {
        pressure = bmpEvent.pressure;  // Па (можно поделить на 100 для hPa)
      } else {
        Serial.println("Ошибка чтения BMP180");
      }
    }

    // --- Вывод данных в Serial Monitor ---
    Serial.println("===== Данные с датчиков =====");

    if (dhtAvailable) {
      Serial.print("Температура (DHT): ");
      Serial.print(temperature);
      Serial.println(" *C");
      Serial.print("Влажность: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      Serial.println("DHT21 не отвечает.");
    }

    if (sgpAvailable) {
      Serial.print("CO2: ");
      Serial.print(co2);
      Serial.println(" ppm");
      Serial.print("TVOC: ");
      Serial.print(tvoc);
      Serial.println(" ppb");
    } else {
      Serial.println("SGP30 не отвечает.");
    }

    if (bmpAvailable) {
      Serial.print("Давление: ");
      Serial.print(pressure * 0.750062);  // Переводим из hPa в мм рт. ст.
      Serial.println(" мм рт. ст.");
    } else {
      Serial.println("BMP180 не отвечает.");
    }

    Serial.println("=============================\n");
  }
}

// --- Геттеры для получения значений в других частях программы ---
float getTemperature() { return temperature; }
float getHumidity() { return humidity; }
uint16_t getTVOC() { return tvoc; }
uint16_t getCO2() { return co2; }
float getPressure() { return pressure; }
