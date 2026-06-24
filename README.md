# SmartGarden Firmware

Firmware do projeto **SmartGarden**, um sistema de monitoramento ambiental para jardins inteligentes usando **ESP32** e sensor **DHT22**.

Esta versao realiza a leitura de **temperatura** e **umidade do ar**, exibe os dados no **Monitor Serial** e publica as leituras no Mosquitto via MQTT QoS 1.

---

## Objetivo

O objetivo deste firmware é coletar informações ambientais do jardim, como:

- temperatura do ar;
- umidade relativa do ar;
- status básico do ambiente.

Esses dados são consumidos pelo backend Spring Boot e exibidos no dashboard administrativo.

---

## Tecnologias e componentes utilizados

- ESP32 Dev Module
- Sensor DHT22
- Arduino IDE
- Biblioteca DHT sensor library by Adafruit
- Biblioteca Adafruit Unified Sensor
- WiFi.h
- ArduinoMqttClient

---

## Ligações do sensor DHT22

| DHT22 | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO 4 |

No codigo atual, o pino de dados esta configurado como:

```cpp
#define DHTPIN 4
```

## Configuracao antes de gravar na placa

No arquivo `esp32-dht22-serial.ino`, ajuste estes valores:

```cpp
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";
const char* MQTT_BROKER = "192.168.0.10";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "smartgarden";
const char* MQTT_PASSWORD = "smartgarden_mqtt";
const char* DEVICE_CODE = "esp32-jardim-bloco-a";
```

Instale também a biblioteca **ArduinoMqttClient** pelo gerenciador de bibliotecas da Arduino IDE.

## Importante sobre o endereço do broker

Em `MQTT_BROKER`, use o IP da máquina que está executando o Docker Compose na rede local. `localhost` apontaria para o próprio ESP32.

Exemplo:

```text
192.168.0.10
```

## Publicação MQTT

O tópico é montado automaticamente a partir de `DEVICE_CODE`: `smartgarden/devices/{deviceCode}/telemetry`.

```json
{
  "messageId": "9d892fe8-d62a-4bd9-a0cc-a4c70f78271e",
  "temperatureC": 27.40,
  "humidityPercent": 63.10,
  "recordedAt": null
}
```

Cada leitura recebe um UUID novo. O backend usa esse identificador para ignorar retransmissões duplicadas do QoS 1.

## Como testar

1. Em `smartgarden-infra`, siga o primeiro uso do README e execute `docker compose up -d` para subir PostgreSQL e Mosquitto.
2. Suba o backend com `mvn spring-boot:run`.
3. Confirme que o backend responde em `http://SEU_IP:8080/swagger-ui.html`.
4. Grave o firmware no ESP32.
5. Abra o monitor serial em `115200` e verifique a conexão MQTT e o resultado da publicação.

As leituras devem aparecer no PostgreSQL e atualizar Dashboard e Monitoramento por SSE.
