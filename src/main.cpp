#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// --- Configurações de Rede e MQTT ---
const char *SSID = "SENAI EDUCACIONAL";
const char *SENHA = "info@134";

const char *BROKER = "broker.emqx.io";
const int PORTA = 1883;
const char *TOPICO_PUB = "Senai/Mesa_do_CANO/Jean-Bruno/Promocoes";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

struct JogoPromocao {
  String titulo;
  float precoNormal;
  float precoPromocao;
  int desconto;
};

JogoPromocao primeiroJogo;
bool temJogoCarregado = false;

// --- Função para buscar apenas o 1º Jogo da CheapShark ---
void buscarPrimeiroJogoSteam() {
  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sem Conexao WiFi!");
    return;
  }

  lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("#####Consultando####");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print(" ####Consultando### ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("  ###Consultando##  ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("   ##Consultando#   ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("    #Consultando    ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("     Consultando    ");
    delay(500);


    lcd.setCursor(0, 1);
    lcd.print("=====Consultado!====");
    delay(500);

  lcd.setCursor(0, 2);
  lcd.print(" CheapShark (Steam) ");
  delay(500);

  WiFiClientSecure client;
  client.setInsecure(); // Ignora certificados SSL para poupar memória RAM

  HTTPClient http;
  // pageSize=1 garante que a API responda apenas com o 1º jogo com maior desconto
  String url = "https://www.cheapshark.com/api/1.0/deals?storeID=1&sortBy=Savings&pageSize=1";

  http.begin(client, url);
  http.addHeader("User-Agent", "ESP32"); // Evita que o servidor bloqueie o ESP32

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString(); // Pega a resposta completa em texto

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // Como a API retorna um Array JSON [ {...} ], acessamos o índice [0]
      JsonObject deal = doc[0];

      if (!deal.isNull()) {
        primeiroJogo.titulo = deal["title"].as<String>();
        primeiroJogo.precoNormal = deal["normalPrice"].as<float>();
        primeiroJogo.precoPromocao = deal["salePrice"].as<float>();
        primeiroJogo.desconto = (int)deal["savings"].as<float>();

        temJogoCarregado = true;
        Serial.println("Jogo encontrado: " + primeiroJogo.titulo);
      } else {
        Serial.println("Erro: Objeto do jogo veio vazio.");
        temJogoCarregado = false;
      }
    } else {
      Serial.print("Erro ao processar JSON: ");
      Serial.println(error.c_str());
      temJogoCarregado = false;
    }
  } else {
    Serial.printf("Erro HTTP na API: %d\n", httpCode);
    temJogoCarregado = false;
  }
  http.end();
}

// --- Exibe o 1º Jogo no LCD ---
void exibirJogoNoLCD() {
  lcd.clear();

  if (!temJogoCarregado) {
    lcd.setCursor(0, 0);
    lcd.print("Nenhuma oferta");
    lcd.setCursor(0, 1);
    lcd.print("encontrada!");
    return;
  }

  // Linha 1: Cabeçalho
  lcd.setCursor(0, 0);
  lcd.print("OFERTA DO DIA STEAM");

  // Linha 2: Nome do Jogo (Ajustado para no máximo 20 caracteres)
  lcd.setCursor(0, 1);
  String nome = primeiroJogo.titulo.length() > 20 ? primeiroJogo.titulo.substring(0, 20) : primeiroJogo.titulo;
  lcd.print(nome);

  // Linha 3: Preço de / por
  lcd.setCursor(0, 2);
  lcd.print("De:$");
  lcd.print(primeiroJogo.precoNormal, 2);
  lcd.print(" Por:$");
  lcd.print(primeiroJogo.precoPromocao, 2);

  // Linha 4: Porcentagem de Desconto
  lcd.setCursor(0, 3);
  lcd.print("Desconto: -");
  lcd.print(primeiroJogo.desconto);
  lcd.print("% OFF");
}

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);


  WiFi.begin(SSID, SENHA);

  // --- Sua Animação de Conexão no LCD ---
  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("     Conectando     ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("    #Conectando#    ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("   ##Conectando##   ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("  ###Conectando###  ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print(" ####Conectando#### ");
    delay(500);

    lcd.setCursor(0, 1);
    lcd.print("#####Conectando#####");
    delay(500);

    Serial.print("#");
  }

  // Confirmação de conexão no LCD
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("==WIFI CONECTADO!===");
  delay(1500);

  // Conexão ao Broker MQTT
  mqtt.setServer(BROKER, PORTA);
  String id = "ESP32-CheapShark-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  mqtt.connect(id.c_str());

  // Executa a busca inicial e atualiza a tela
  buscarPrimeiroJogoSteam();
  exibirJogoNoLCD();
}

void loop() {
  mqtt.loop();

  // Atualiza a cada 5 minutos (300.000 ms)
  static unsigned long ultimaAtualizacao = 0;
  if (millis() - ultimaAtualizacao > 300000) {
    ultimaAtualizacao = millis();

    buscarPrimeiroJogoSteam();
    exibirJogoNoLCD();

    // Envia o status para o tópico MQTT se o jogo for carregado com sucesso
    if (temJogoCarregado) {
      JsonDocument doc;
      doc["jogo"] = primeiroJogo.titulo;
      doc["preco_de"] = primeiroJogo.precoNormal;
      doc["preco_por"] = primeiroJogo.precoPromocao;
      doc["desconto"] = primeiroJogo.desconto;

      String mensagem;
      serializeJson(doc, mensagem);
      mqtt.publish(TOPICO_PUB, mensagem.c_str());
    }
  }
}