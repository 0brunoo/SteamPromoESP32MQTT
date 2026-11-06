#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char *SSID = "SENAI EDUCACIONAL";
const char *SENHA = "info@134";

const char *BROKER = "broker.emqx.io";
const int PORTA = 1883;
const char *TOPICO_SUB = "Senai/Mesa_do_CANO/Jean-Bruno/Status";

const int PINO_RGB = 48;

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void setup_wifi()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando Wi-Fi...");

  Serial.println();
  Serial.print("Conectando-se à rede: ");
  Serial.println(SSID);

  WiFi.begin(SSID, SENHA);

  int pontos = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    lcd.setCursor(pontos % 20, 1);
    lcd.print(".");
    pontos++;
  }

  Serial.println("\nWiFi conectado!");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wi-Fi Conectado!");
  delay(1000);
}

void reconectar()
{
  while (!mqtt.connected())
  {
    Serial.print("Tentando conexão MQTT...");
    lcd.setCursor(0, 1);
    lcd.print("Conectando Broker...");

    String clientId = "ESP32-Receptor-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqtt.connect(clientId.c_str()))
    {
      Serial.println("Conectado ao Broker MQTT!");

      lcd.setCursor(0, 1);
      lcd.print("Broker Conectado!   ");
      delay(1000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Aguardando ofertas...");

      mqtt.subscribe(TOPICO_SUB);
    }
    else
    {
      Serial.print("Falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Tentando novamente...");

      lcd.setCursor(0, 1);
      lcd.print("Falha MQTT. Retentando");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("-----------------------");
  Serial.println("Nova mensagem recebida!");

  String mensagem;
  for (int i = 0; i < length; i++)
  {
    mensagem += (char)payload[i];
  }
  Serial.println("JSON: " + mensagem);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagem);

  if (erro)
  {
    Serial.print("Falha ao analisar o JSON: ");
    Serial.println(erro.c_str());
    return;
  }

  String jogo = doc["jogo"];
  float precoPor = doc["preco_por"];

  Serial.print("Jogo analisado: ");
  Serial.println(jogo);
  Serial.print("Preço Promocional: $");
  Serial.println(precoPor);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OFERTA RECEBIDA!");

  lcd.setCursor(0, 1);
  String nomeExibicao = jogo.length() > 20 ? jogo.substring(0, 20) : jogo;
  lcd.print(nomeExibicao);

  lcd.setCursor(0, 2);
  lcd.print("Preco atual: $");
  lcd.print(precoPor, 2);

  lcd.setCursor(0, 3);
  if (precoPor < 1.00)
  {
    Serial.println("LED VERDE");
    rgbLedWrite(PINO_RGB, 0, 255, 0);
    lcd.print("Status: MUITO BARATO");
  }
  else
  {
    Serial.println("LED VERMELHO");
    rgbLedWrite(PINO_RGB, 255, 0, 0);
    lcd.print("Status: PRECO NORMAL");
  }
}

void setup()
{
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  rgbLedWrite(PINO_RGB, 0, 0, 0);

  delay(1000);

  setup_wifi();

  mqtt.setServer(BROKER, PORTA);
  mqtt.setCallback(callback);
}

void loop()
{
  if (!mqtt.connected())
  {
    reconectar();
  }
  mqtt.loop();
}