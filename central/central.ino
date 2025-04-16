#include "config.h"
#include "sensors.h"

void setup() {
  Serial.begin(9600);
  initSensors();
}

void loop() {
  updateSensors();
}
