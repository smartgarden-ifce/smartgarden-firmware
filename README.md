# SmartGarden Firmware

Firmware do projeto **SmartGarden**, um sistema de monitoramento ambiental para jardins inteligentes usando **ESP32** e sensor **DHT22**.

Esta primeira versão realiza a leitura de **temperatura** e **umidade do ar** e exibe os dados no **Monitor Serial** da Arduino IDE.

---

## Objetivo

O objetivo deste firmware é coletar informações ambientais do jardim, como:

- temperatura do ar;
- umidade relativa do ar;
- status básico do ambiente.

Esses dados futuramente serão enviados para um sistema web com dashboard administrativo.

---

## Tecnologias e componentes utilizados

- ESP32 Dev Module
- Sensor DHT22
- Arduino IDE
- Biblioteca DHT sensor library by Adafruit
- Biblioteca Adafruit Unified Sensor

---

## Ligações do sensor DHT22

| DHT22 | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO 4 |

No código atual, o pino de dados está configurado como:

```cpp
#define DHTPIN 4