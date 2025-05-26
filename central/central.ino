#include "config.h"
#include "sensors.h"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <ArduinoJson.h>

RF24 radio(9, 10);
byte addresses[][6] = {"1Node", "2Node"};

char outdoorPayload[32];

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!radio.begin()) {
    Serial.println("NRF24 не найден!");
  } else {
    Serial.println("NRF24 модуль найден");
    radio.setAutoAck(1);
    radio.enableAckPayload();
    radio.setRetries(5, 15);
    radio.setDataRate(RF24_1MBPS);
    radio.setPALevel(RF24_PA_LOW);

    radio.openWritingPipe(addresses[0]);    // outdoor
    radio.openReadingPipe(1, addresses[1]); // central
    radio.startListening();
  }

  initSensors();
}

void loop() {
  updateSensors();

  String outdoorStr = "";
  unsigned long startWait = millis();
  while (millis() - startWait < SENSOR_READ_INTERVAL) {
    byte pipeNo;
    if (radio.available(&pipeNo)) {
      memset(outdoorPayload, 0, sizeof(outdoorPayload));
      radio.read(&outdoorPayload, sizeof(outdoorPayload));
      outdoorStr = String(outdoorPayload);

      // можно отправить ack payload (например байт подтверждения)
      byte ackResponse = 1;
      radio.writeAckPayload(pipeNo, &ackResponse, 1);

      break;
    }
  }

  if (outdoorStr.length() == 0) {
    outdoorStr = "T:null;H:null;P:null";
  }

  char buffer[32];
  memcpy(buffer, outdoorPayload, sizeof(buffer));
  buffer[31] = '\0';
  float ot = NAN, oh = NAN, op = NAN;

  char *token = strtok(buffer, ";");
  while (token != NULL) {
    if (strncmp(token, "T:", 2) == 0) ot = atof(token + 2);
    else if (strncmp(token, "H:", 2) == 0) oh = atof(token + 2);
    else if (strncmp(token, "P:", 2) == 0) op = atof(token + 2);
    token = strtok(NULL, ";");
  }


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

  serializeJson(doc, Serial);
  Serial.println();

  delay(SENSOR_READ_INTERVAL);
}
