#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// ===== CONFIGURAÇÃO DO SENSOR =====
#define DHTPIN 4          // GPIO conectado ao DATA do DHT22
#define DHTTYPE DHT22     // Tipo do sensor

// ===== CONFIGURAÇÃO DA REDE =====
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// IMPORTANTE:
// Use o IP da máquina que está rodando o backend -> ip addr (comando para ver IP da máquina).
const char* API_URL = "http://192.168.0.10:8080/api/readings";
const char* DEVICE_CODE = "esp32-jardim-bloco-a";

// ===== INTERVALO ENTRE LEITURAS =====
const unsigned long READ_INTERVAL_MS = 30000;
const unsigned long WIFI_RETRY_INTERVAL_MS = 10000;

// ===== OBJETO DO SENSOR =====
DHT dht(DHTPIN, DHTTYPE);

// ===== CONTROLE DE TEMPO =====
unsigned long lastReadAt = 0;
unsigned long lastWifiRetryAt = 0;

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

void sendReadingToBackend(float temperature, float humidity) {
  if (!ensureWiFiConnected()) {
    Serial.println("Wi-Fi indisponivel. Leitura nao enviada.");
    return;
  }

  HTTPClient http;
  http.begin(API_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"deviceCode\":\"" + String(DEVICE_CODE) + "\",";
  payload += "\"temperatureC\":" + String(temperature, 2) + ",";
  payload += "\"humidityPercent\":" + String(humidity, 2);
  payload += "}";

  int httpCode = http.POST(payload);

  Serial.println("===== ENVIO PARA BACKEND =====");
  Serial.print("URL: ");
  Serial.println(API_URL);
  Serial.print("Payload: ");
  Serial.println(payload);
  Serial.print("HTTP status: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.print("Resposta: ");
    Serial.println(response);
  } else {
    Serial.print("Erro HTTP: ");
    Serial.println(http.errorToString(httpCode));
  }

  Serial.println("---------------------------------");
  http.end();
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
