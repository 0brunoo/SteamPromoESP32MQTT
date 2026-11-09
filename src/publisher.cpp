#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <Botao.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

Botao btn(17);
const char *SSID = "SENAI EDUCACIONAL";
const char *SENHA = "info@134";
const char *BROKER = "broker.emqx.io";
const int PORTA = 1883;
const char *TOPICO_PUB = "Senai/Mesa_do_CANO/Jean-Bruno/Status";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

struct JogoPromocao
{
  String titulo;
  float precoNormal;
  float precoPromocao;
  int desconto;
};

JogoPromocao jogoAtual;
bool temJogoCarregado = false;
int paginaAtual = 0;

void buscarJogoSteam(int pagina);
void exibirJogoNoLCD();

void setup()
{
  lcd.init();
  lcd.backlight();

  Serial.begin(115200);

  btn.iniciar();
  btn.modoDebounce(ModoDebounce::ESTAVEL);
  btn.intervaloDebounce(25);
  btn.habilitarCliqueDuplo(false);

  WiFi.begin(SSID, SENHA);

  // ANIMAÇÃO BOLADA CASEIRA
  while (WiFi.status() != WL_CONNECTED)
  {
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

  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("==WIFI CONECTADO!===");
  delay(1500);

  mqtt.setServer(BROKER, PORTA);

  String id = "ESP32-CheapShark-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  mqtt.connect(id.c_str());

  buscarJogoSteam(paginaAtual);

  exibirJogoNoLCD();
}

void loop()
{
  mqtt.loop();

  btn.atualizar();

  if (btn.clicou())
  {
    Serial.println("Botao pressionado! Buscando proximo jogo...");
    paginaAtual = (paginaAtual + 1) % 10;

    buscarJogoSteam(paginaAtual);

    exibirJogoNoLCD();
  }

  static unsigned long ultimaAtualizacao = 0;

  if (millis() - ultimaAtualizacao > 5000)
  {
    ultimaAtualizacao = millis();

    if (temJogoCarregado)
    {
      JsonDocument doc;

      doc["jogo"] = jogoAtual.titulo;
      doc["preco_de"] = jogoAtual.precoNormal;
      doc["preco_por"] = jogoAtual.precoPromocao;
      doc["desconto"] = jogoAtual.desconto;
      doc["posicao"] = paginaAtual + 1;

      String mensagem;

      serializeJson(doc, mensagem);

      mqtt.publish(TOPICO_PUB, mensagem.c_str());
    }
  }
}

void buscarJogoSteam(int pagina)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Sem Conexao WiFi!");

    return;
  }

  lcd.clear();

  lcd.setCursor(0, 1);
  lcd.print("#####Consultando####");
  delay(200);

  lcd.setCursor(0, 1);
  lcd.print(" ####Consultando### ");
  delay(200);

  lcd.setCursor(0, 1);
  lcd.print("  ###Consultando##  ");
  delay(200);

  lcd.setCursor(0, 1);
  lcd.print("   ##Consultando#   ");
  delay(200);

  lcd.setCursor(0, 1);
  lcd.print("    #Consultando    ");
  delay(200);

  lcd.setCursor(0, 1);
  lcd.print("     Consultando    ");
  delay(200);

  lcd.setCursor(0, 1);
  lcd.print("=====Consultado!====");
  delay(300);

  lcd.setCursor(0, 2);
  lcd.print(" CheapShark (Steam) ");
  delay(300);

  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;

  String url = "https://www.cheapshark.com/api/1.0/deals?storeID=1&sortBy=Savings&pageSize=1&pageNumber=" + String(pagina);

  http.begin(client, url);

  http.addHeader("User-Agent", "ESP32");

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK)
  {
    String payload = http.getString();

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (!error)
    {
      JsonObject deal = doc[0];

      if (!deal.isNull())
      {
        jogoAtual.titulo = deal["title"].as<String>();
        jogoAtual.precoNormal = deal["normalPrice"].as<float>();
        jogoAtual.precoPromocao = deal["salePrice"].as<float>();
        jogoAtual.desconto = (int)deal["savings"].as<float>();
        temJogoCarregado = true;

        Serial.printf("Jogo [%d] encontrado: %s\n", pagina + 1, jogoAtual.titulo.c_str());
      }
      else
      {
        Serial.println("Erro: Objeto do jogo veio vazio.");

        temJogoCarregado = false;
      }
    }
    else
    {
      Serial.print("Erro ao processar JSON: ");
      Serial.println(error.c_str());

      temJogoCarregado = false;
    }
  }
  else
  {
    Serial.printf("Erro HTTP na API: %d\n", httpCode);

    temJogoCarregado = false;
  }

  http.end();
}

void exibirJogoNoLCD()
{
  lcd.clear();

  if (!temJogoCarregado)
  {
    lcd.setCursor(0, 0);
    lcd.print("Nenhuma oferta");

    lcd.setCursor(0, 1);
    lcd.print("encontrada!");

    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("OFERTA STEAM #");
  lcd.print(paginaAtual + 1);

  lcd.setCursor(0, 1);

  String nome = jogoAtual.titulo.length() > 20 ? jogoAtual.titulo.substring(0, 20) : jogoAtual.titulo;

  lcd.print(nome);

  lcd.setCursor(0, 2);
  lcd.print("De:$");
  lcd.print(jogoAtual.precoNormal, 2);

  lcd.print(" Por:$");
  lcd.print(jogoAtual.precoPromocao, 2);

  lcd.setCursor(0, 3);
  lcd.print("Desconto: -");
  lcd.print(jogoAtual.desconto);
  lcd.print("% OFF");
}