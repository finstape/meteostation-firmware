#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

void initSensors();
void updateSensors();

// DHT21 (температура и влажность)
float getTemperature();
float getHumidity();

// BMP180 (атмосферное давление и температура)
float getPressure();
float getBmpTemperature();

#endif
