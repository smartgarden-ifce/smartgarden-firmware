#include <DHT.h>

// ===== CONFIGURAÇÃO DO SENSOR =====
#define DHTPIN 4          // GPIO conectado ao DATA do DHT22
#define DHTTYPE DHT22     // Tipo do sensor

// ===== INTERVALO ENTRE LEITURAS =====
const unsigned long READ_INTERVAL_MS = 2000;

// ===== OBJETO DO SENSOR =====
DHT dht(DHTPIN, DHTTYPE);

// ===== CONTROLE DE TEMPO =====
unsigned long lastReadAt = 0;

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
}