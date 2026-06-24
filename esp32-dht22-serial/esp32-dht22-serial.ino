#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include <DHT.h>
#include <esp_system.h>

// ===== CONFIGURAÇÃO DO SENSOR =====
#define DHTPIN 4          // GPIO conectado ao DATA do DHT22
#define DHTTYPE DHT22     // Tipo do sensor

// ===== CONFIGURAÇÃO DA REDE =====
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// Use o IP da máquina que está rodando o Mosquitto.
const char* MQTT_BROKER = "192.168.0.10";
const int MQTT_PORT = 1883;
const char* MQTT_USERNAME = "smartgarden";
const char* MQTT_PASSWORD = "smartgarden_mqtt";
const char* DEVICE_CODE = "esp32-jardim-bloco-a";

// ===== INTERVALO ENTRE LEITURAS =====
const unsigned long READ_INTERVAL_MS = 30000;
const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;
const unsigned long MQTT_RETRY_INTERVAL_MS = 5000;

// ===== OBJETO DO SENSOR =====
DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// ===== CONTROLE DE TEMPO =====
unsigned long lastReadAt = 0;
unsigned long lastWifiRetryAt = 0;
unsigned long lastMqttRetryAt = 0;

void connectToWiFi() {
  Serial.println("Conectando ao Wi-Fi...");
  Serial.print("Rede: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi conectado com sucesso.");
    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha ao conectar no Wi-Fi.");
  }
}

bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  if (millis() - lastWifiRetryAt < WIFI_RETRY_INTERVAL_MS) {
    return false;
  }

  lastWifiRetryAt = millis();
  connectToWiFi();
  return WiFi.status() == WL_CONNECTED;
}

bool ensureMqttConnected() {
  if (!ensureWiFiConnected()) {
    return false;
  }

  if (mqttClient.connected()) {
    return true;
  }

  if (millis() - lastMqttRetryAt < MQTT_RETRY_INTERVAL_MS) {
    return false;
  }

  lastMqttRetryAt = millis();
  Serial.print("Conectando ao broker MQTT em ");
  Serial.print(MQTT_BROKER);
  Serial.print(":");
  Serial.println(MQTT_PORT);

  mqttClient.setId(DEVICE_CODE);
  mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
  mqttClient.setCleanSession(false);
  mqttClient.setKeepAliveInterval(30UL * 1000UL);

  if (!mqttClient.connect(MQTT_BROKER, MQTT_PORT)) {
    Serial.print("Falha MQTT. Codigo: ");
    Serial.println(mqttClient.connectError());
    return false;
  }

  Serial.println("MQTT conectado com sucesso.");
  return true;
}

String generateMessageId() {
  uint8_t bytes[16];
  for (int i = 0; i < 16; i += 4) {
    uint32_t randomValue = esp_random();
    memcpy(bytes + i, &randomValue, sizeof(randomValue));
  }
  bytes[6] = (bytes[6] & 0x0F) | 0x40;
  bytes[8] = (bytes[8] & 0x3F) | 0x80;

  char uuid[37];
  snprintf(
    uuid,
    sizeof(uuid),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    bytes[0], bytes[1], bytes[2], bytes[3],
    bytes[4], bytes[5], bytes[6], bytes[7],
    bytes[8], bytes[9], bytes[10], bytes[11],
    bytes[12], bytes[13], bytes[14], bytes[15]
  );
  return String(uuid);
}

void sendReadingToBackend(float temperature, float humidity) {
  if (!ensureMqttConnected()) {
    Serial.println("MQTT indisponivel. Leitura nao enviada.");
    return;
  }

  String payload = "{";
  payload += "\"messageId\":\"" + generateMessageId() + "\",";
  payload += "\"temperatureC\":" + String(temperature, 2) + ",";
  payload += "\"humidityPercent\":" + String(humidity, 2) + ",";
  payload += "\"recordedAt\":null";
  payload += "}";

  String topic = "smartgarden/devices/" + String(DEVICE_CODE) + "/telemetry";
  mqttClient.beginMessage(topic, payload.length(), false, 1);
  mqttClient.print(payload);
  int publishResult = mqttClient.endMessage();

  Serial.println("===== ENVIO MQTT =====");
  Serial.print("Topico: ");
  Serial.println(topic);
  Serial.print("Payload: ");
  Serial.println(payload);
  Serial.print("Resultado da publicacao: ");
  Serial.println(publishResult);

  Serial.println("---------------------------------");
}

void setup() {
  // Inicializa comunicação serial
  Serial.begin(115200);

  // Pequena espera para estabilizar
  delay(2000);

  Serial.println("=================================");
  Serial.println("INICIANDO MONITORAMENTO DO AR");
  Serial.println("ESP32 + DHT22");
  Serial.println("=================================");

  // Inicializa sensor
  dht.begin();

  // Conecta no Wi-Fi
  connectToWiFi();
}

void loop() {
  if (mqttClient.connected()) {
    mqttClient.poll();
  }

  // Verifica se já passou o intervalo
  if (millis() - lastReadAt < READ_INTERVAL_MS) {
    return;
  }

  // Atualiza tempo da última leitura
  lastReadAt = millis();

  // ===== LEITURA DO SENSOR =====
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // ===== VALIDAÇÃO =====
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("ERRO: Falha ao ler o DHT22!");
    Serial.println("Verifique:");
    Serial.println("- Alimentacao");
    Serial.println("- GPIO");
    Serial.println("- Cabo DATA");
    Serial.println("---------------------------------");
    return;
  }

  // ===== EXIBIÇÃO DOS DADOS =====
  Serial.println("===== LEITURA AMBIENTE =====");

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(humidity);
  Serial.println(" %");

  // ===== ÍNDICE DE CONFORTO =====
  if (temperature > 30) {
    Serial.println("Ambiente quente");
  }
  else if (temperature < 20) {
    Serial.println("Ambiente frio");
  }
  else {
    Serial.println("Temperatura agradavel");
  }

  if (humidity < 40) {
    Serial.println("Ar seco");
  }
  else if (humidity > 70) {
    Serial.println("Ar muito umido");
  }
  else {
    Serial.println("Umidade confortavel");
  }

  Serial.println("---------------------------------");

  // ===== ENVIO PARA O BACKEND =====
  sendReadingToBackend(temperature, humidity);
}
