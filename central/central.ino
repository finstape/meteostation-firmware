#include "config.h"
#include "sensors.h"

#include <SPI.h>
#include <RF24.h>
#include <ArduinoJson.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";
char outdoorPayload[32];

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!radio.begin()) {
    Serial.println("NRF24 не найден! Проверьте подключения");
  } else if (!radio.isChipConnected()) {
    Serial.println("NRF24 не отвечает (isChipConnected = false)");
  } else {
    Serial.println("NRF24 модуль найден");
    radio.setChannel(0x7B);
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_1MBPS);
    radio.enableDynamicPayloads();
    radio.setRetries(5, 15);
    radio.openReadingPipe(1, address);
    radio.startListening();
  }

  initSensors();
}

void loop() {
  updateSensors();

  String outdoorStr = "";
  unsigned long startWait = millis();
  while (millis() - startWait < SENSOR_READ_INTERVAL) {
    if (radio.available()) {
      radio.read(&outdoorPayload, sizeof(outdoorPayload));
      outdoorStr = String(outdoorPayload);
      break;
    }
  }

  if (outdoorStr.length() == 0) {
    outdoorStr = "T:null;H:null;P:null";
  }

  float ot = NAN, oh = NAN, op = NAN;
  sscanf(outdoorStr.c_str(), "T:%f;H:%f;P:%f", &ot, &oh, &op);

  StaticJsonDocument<256> doc;

  JsonObject central = doc.createNestedObject("central");
  central["co2"] = getCO2();
  central["tvoc"] = getTVOC();
  if (isnan(getTemperature()))
    central["temperature"] = serialized("null");
  else
    central["temperature"] = getTemperature();

  if (isnan(getHumidity()))
    central["humidity"] = serialized("null");
  else
    central["humidity"] = getHumidity();

  if (isnan(getPressure()))
    central["pressure"] = serialized("null");
  else {
    char pressure_str[8];
    dtostrf(getPressure(), 0, 1, pressure_str);
    central["pressure"] = serialized(pressure_str);
  }

  JsonObject outdoor = doc.createNestedObject("outdoor");

  if (isnan(ot))
    outdoor["temperature"] = serialized("null");
  else
    outdoor["temperature"] = ot;

  if (isnan(oh))
    outdoor["humidity"] = serialized("null");
  else
    outdoor["humidity"] = oh;

  if (isnan(op))
    outdoor["pressure"] = serialized("null");
  else
    outdoor["pressure"] = op;

  // Отправка JSON в Serial (ESP читает)
  serializeJson(doc, Serial);
  Serial.println();

  delay(SENSOR_READ_INTERVAL);
}