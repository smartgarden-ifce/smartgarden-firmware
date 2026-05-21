# SmartGarden Firmware

Firmware do projeto **SmartGarden**, um sistema de monitoramento ambiental para jardins inteligentes usando **ESP32** e sensor **DHT22**.

Esta versao realiza a leitura de **temperatura** e **umidade do ar**, exibe os dados no **Monitor Serial** e envia as leituras para o backend Spring Boot via HTTP.

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
- WiFi.h
- HTTPClient.h

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
const char* API_URL = "http://192.168.0.10:8080/api/readings";
const char* DEVICE_CODE = "esp32-jardim-bloco-a";
```

## Importante sobre a URL do backend

No `API_URL`, voce deve usar o IP da maquina que esta rodando o backend na sua rede local.

Exemplo:

```text
http://192.168.0.10:8080/api/readings
```

Nao use:

```text
http://localhost:8080/api/readings
```

O motivo é que, para o ESP32, `localhost` aponta para o proprio ESP32 e nao para o seu computador.

## Payload enviado para o backend

O firmware envia um `POST` para o backend com este JSON:

```json
{
  "deviceCode": "esp32-jardim-bloco-a",
  "temperatureC": 27.40,
  "humidityPercent": 63.10
}
```

Esse payload e compativel com o endpoint:

```text
POST /api/readings
```

## Como testar

1. Suba o backend e o banco.
2. Confirme que o backend responde em `http://SEU_IP:8080/swagger-ui.html`.
3. Grave o firmware no ESP32.
4. Abra o monitor serial em `115200`.
5. Verifique se aparecem os logs de leitura e o `HTTP status` do envio.

Se o envio estiver funcionando, voce deve ver respostas `201` no monitor serial e os registros aparecendo no PostgreSQL.
