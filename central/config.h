#ifndef CONFIG_H
#define CONFIG_H

// Пин подключения AM2301 (DHT21)
#define DHT_PIN 2

// Тип датчика (DHT21 = AM2301)
#define DHT_TYPE DHT21

// Интервал обновления данных с датчиков (в миллисекундах)
#define SENSOR_READ_INTERVAL 5000  // 5 секунд

// Интервал сохранения baseline SGP30 (мс)
#define BASELINE_SAVE_INTERVAL 3600000UL  // 1 час

#endif
