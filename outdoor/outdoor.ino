#include "config.h"
#include "sensors.h"

#include <SPI.h>
#include <RF24.h>

// Радиодатчик
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

// Буфер для передачи
char payload[32];  // NRF24L01 ограничен 32 байтами

void setup() {
  Serial.begin(9600);
  delay(500); // небольшая задержка для Serial

  if (!radio.begin()) {
    Serial.println("Ошибка: не найден модуль NRF24. Проверьте питание и подключения");
  } else if (!radio.isChipConnected()) {
    Serial.println("Ошибка: модуль NRF24 не отвечает (isChipConnected = false)");
  } else {
    Serial.println("NRF24 модуль обнаружен");

    radio.setChannel(0x7B);                   // канал 7B (123)
    radio.setPALevel(RF24_PA_LOW);            // минимальная мощность
    radio.setDataRate(RF24_1MBPS);            // стандартная скорость
    radio.enableDynamicPayloads();
    radio.setRetries(5, 15);                  // надёжность передачи
    radio.openWritingPipe(address);
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

  // Подготовка строк с 1 знаком после запятой
  char t_str[8], h_str[8], p_str[8];
  if (isnan(t)) strcpy(t_str, "null");
  else dtostrf(t, 0, 1, t_str);

  if (isnan(h)) strcpy(h_str, "null");
  else dtostrf(h, 0, 1, h_str);

  if (isnan(p)) strcpy(p_str, "null");
  else dtostrf(p, 0, 1, p_str);

  // Формируем строку для отправки
  String msg = "T:" + String(t_str) + ";H:" + String(h_str) + ";P:" + String(p_str);
  msg.toCharArray(payload, sizeof(payload));

  Serial.print("Отправка: ");
  Serial.println(payload);

  radio.write(&payload, sizeof(payload));

  delay(SENSOR_READ_INTERVAL);
}