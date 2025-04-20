#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

void initSensors();
void updateSensors();

// DHT21 (температура и влажность)
float getTemperature();
float getHumidity();

// SGP30 (качество воздуха)
uint16_t getTVOC();
uint16_t getCO2();

#endif
