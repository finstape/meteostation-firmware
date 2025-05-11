#include "sensors.h"
#include "config.h"

#include <DHT.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

// DHT21
DHT dht(DHT_PIN, DHT_TYPE);
bool dhtAvailable = false;
float temperature = NAN, humidity = NAN;

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

  // Инициализация BMP180
  if (bmp.begin()) {
    bmpAvailable = true;
  }

  delay(15000); // Время, чтобы датчики откалибровались и стали замерять точные значения
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
float getPressure()     { return pressure; }
