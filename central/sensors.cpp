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
unsigned long lastBaselineSave = 0;

// BMP180
Adafruit_BMP085_Unified bmp(10180); // ID произвольный
bool bmpAvailable = false;
float pressure = NAN;

// Инициализация всех датчиков
void initSensors() {
  // Инициализация DHT21
  dht.begin();
  dhtAvailable = true;  // Предполагаем, что DHT всегда запускается (нет метода проверки)

  // Инициализация I2C
  Wire.begin();

  // Инициализация SGP30
  if (sgp.begin()) {
    sgpAvailable = true;    
    sgp.setIAQBaseline(0x8B60, 0x8AD3); // Восстановление baseline (заглушка)
    lastBaselineSave = millis();
  }

  // Инициализация BMP180
  if (bmp.begin()) {
    bmpAvailable = true;
  }

  delay(60000); // Время, чтобы датчики откалибровались и стали замерять точные значения
}

// Основной цикл обновления данных
void updateSensors() {
  // Чтение DHT21
  if (dhtAvailable) {
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    temperature = isnan(newTemp) ? NAN : newTemp;
    humidity    = isnan(newHum) ? NAN : newHum;
  } else {
    temperature = NAN;
    humidity    = NAN;
  }

  // Чтение SGP30
  if (sgpAvailable && sgp.IAQmeasure()) {
    tvoc = sgp.TVOC;
    co2 = sgp.eCO2;
  } else {
    tvoc = 0;
    co2  = 0;
  }

  // Сохранение baseline SGP30 раз в час
  // SGP30 требует периодической рекалибровки
  // Без этого через сутки точность начнёт плыть
  if (sgpAvailable && millis() - lastBaselineSave >= BASELINE_SAVE_INTERVAL) {
    lastBaselineSave = millis();
    sgp.getIAQBaseline(&baselineCO2, &baselineTVOC);
  }

  // Чтение BMP180
  if (bmpAvailable) {
    sensors_event_t bmpEvent;
    bmp.getEvent(&bmpEvent);  // Чтение давления

    if (bmpEvent.pressure) {
      pressure = bmpEvent.pressure * 0.750062;  // мм рт. ст.
    } else {
      pressure = NAN;
    }
  } else {
    pressure = NAN;
  }
}

// Геттеры для получения значений в других частях программы
float getTemperature()  { return temperature; }
float getHumidity()     { return humidity; }
uint16_t getTVOC()      { return tvoc; }
uint16_t getCO2()       { return co2; }
float getPressure()     { return pressure; }
