
![C++](https://img.shields.io/badge/-C++-blue?style=for-the-badge&logo=cplusplus) ![IoT](https://img.shields.io/badge/IoT-43B02A?style=for-the-badge) ![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino) ![ESP8266](https://img.shields.io/badge/ESP8266-000000?style=for-the-badge) ![License](https://img.shields.io/badge/Apache_2.0-blue.svg?style=for-the-badge)

# Проект: Умная метеостанция

Этот репозиторий содержит прошивки и код аппаратной части умной метеостанции, разработанной в рамках учебного проекта IoT.  
Система предназначена для мониторинга параметров окружающей среды (температура, влажность, давление, CO2), передачи данных по радиоканалу и публикации их на сервер для последующего анализа и уведомлений

## Архитектура проекта

**Метеостанция состоит из 3 компонентов:**

1. **Outdoor станция (outdoor/)**  
    Считывает температуру, влажность и давление на улице, передаёт данные на центральную станцию по NRF24L01

2. **Central станция (central/)**  
    Принимает outdoor данные, считывает собственные локальные параметры (в помещении), формирует итоговый JSON и передаёт его по Serial на ESP

3. **ESP модуль (esp/)**  
    Принимает JSON по Serial, отправляет данные в облачный backend-сервер по Wi-Fi

**Подробные схемы подключения и инструкции** находятся в README внутри каждой папки (`outdoor/`, `central/`, `esp/`)

## Репозитории проекта

- [meteostation-firmware](https://github.com/finstape/meteostation-firmware) (данный репозиторий) — код Arduino + ESP
- [meteostation-backend](https://github.com/finstape/meteostation-backend) — backend-сервер (FastAPI + PostgreSQL)
- [meteostation-ml](https://github.com/finstape/meteostation-ml) — ML-модель прогнозирования погоды

## Описание проекта (аппаратная часть)

Система построена на микроконтроллерах Arduino + ESP8266 и включает в себя:

- **Arduino Uno (central)** → основной контроллер системы
- **Arduino Uno (outdoor)** → внешняя станция (уличный модуль)
- **ESP8266 NodeMCU V3 Lolin** → передача данных в интернет
- **Датчики:** DHT21 (влажность и температура), BMP180 (давление), SGP30 (качество воздуха), nRF24L01+ (радиосвязь)

Электропитание, общие шины 5V, 3.3V и GND, подключение датчиков и модулей подробно описаны в папках `central/` и `outdoor/`

## Лицензия

Данный проект распространяется под лицензией **Apache License 2.0**.  
Свободное использование, модификация и распространение в образовательных и исследовательских целях