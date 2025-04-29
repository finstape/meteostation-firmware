#include "config.h"
#include "sensors.h"

void setup() {
  Serial.begin(9600); // И для Arduino, и для ESP один UART
  initSensors();
}

void loop() {
  updateSensors();

  // собираем показания
  float t = getTemperature();
  float h = getHumidity();
  long  c = getCO2();
  long  v = getTVOC();
  float p = getPressure();

  // формируем JSON
  String json = "{";
  json += "\"temperature\":"  + (isnan(t) ? String("null") : String(t,1));
  json += ",\"humidity\":"    + (isnan(h) ? String("null") : String(h,1));
  json += ",\"co2\":"         + (c==0 ? String("null") : String(c));
  json += ",\"tvoc\":"        + (v==0 ? String("null") : String(v));
  json += ",\"pressure_mmHg\":" + (isnan(p) ? String("null") : String(p,1));
  json += "}";

  // Отправляем JSON напрямую на ESP
  Serial.println(json);
  delay(SENSOR_READ_INTERVAL);
}