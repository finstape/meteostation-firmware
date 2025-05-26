#include "config.h"
#include "sensors.h"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 10);  // CE, CSN
byte addresses[][6] = {"1Node", "2Node"};

typedef enum { role_ping_out = 1, role_pong_back } role_e;
role_e role = role_ping_out;

char payload[32];

void setup() {
  Serial.begin(9600);
  delay(500);

  if (!radio.begin()) {
    Serial.println("NRF24 не найден!");
  } else {
    Serial.println("NRF24 модуль найден");
    radio.setAutoAck(1);
    radio.enableAckPayload();
    radio.setRetries(5, 15);
    radio.setDataRate(RF24_1MBPS);
    radio.setPALevel(RF24_PA_LOW);

    radio.openWritingPipe(addresses[1]);    // central
    radio.openReadingPipe(1, addresses[0]); // outdoor
    radio.stopListening();

    Serial.println("Outdoor node ready");
  }

  initSensors();
}

void loop() {
  updateSensors();

  float t = getTemperature();
  float h = getHumidity();
  float p = getPressure();

  char t_str[8], h_str[8], p_str[8];
  if (isnan(t)) strcpy(t_str, "null");
  else dtostrf(t, 0, 1, t_str);

  if (isnan(h)) strcpy(h_str, "null");
  else dtostrf(h, 0, 1, h_str);

  if (isnan(p)) strcpy(p_str, "null");
  else dtostrf(p, 0, 1, p_str);

  String msg = "T:" + String(t_str) + ";H:" + String(h_str) + ";P:" + String(p_str);
  msg.toCharArray(payload, sizeof(payload));

  Serial.print("Отправка: ");
  Serial.println(payload);

  byte response[1];
  radio.stopListening();
  if (radio.write(&payload, sizeof(payload))) {
    radio.startListening();
    if (radio.available()) {
      radio.read(&response, 1);
      Serial.print("ACK получен: ");
      Serial.println(response[0]);
    } else {
      Serial.println("ACK пустой");
    }
  } else {
    Serial.println("Отправка не удалась");
  }

  delay(SENSOR_READ_INTERVAL);
}
