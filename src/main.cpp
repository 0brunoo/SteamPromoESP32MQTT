#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

void aoReceber(char *, byte *, unsigned int);

void tratarJson(String);

const char *SSID = "SENAI EDUCACIONAL";
const char *SENHA = "info@134";

const char *BROKER = "broker.emqx.io"; //* endereço servidor mqtt
const int PORTA = 1883;
const char *TOPICO_SUB = "Senai/Mesa_do_CANO/Jean-Bruno/Comando";
const char *TOPICO_PUB = "Senai/Mesa_do_CANO/Jean-Bruno/Status";

WiFiClient wifiClient;         //* Instância um cliente para que outros objetos utilizem a conexão WiFi
PubSubClient mqtt(wifiClient); //* Instância um objeto para criar conexão com o broker

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(SSID, SENHA);

  Serial.printf("Conectado na rede %s\n\r", SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  delay(1000);

  Serial.printf("\n\rConectado a rede %s com sucesso\n\r", SSID);

  Serial.printf("Conectado ao Broker %s\n\r", BROKER);

  mqtt.setServer(BROKER, PORTA);
  mqtt.setCallback(aoReceber);

  String id = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

  bool conectadoAoBroker = false;
  conectadoAoBroker = mqtt.connect(id.c_str());

  if (conectadoAoBroker)
  {
    Serial.printf("Conectado ao broker %s com sucesso\n\r", BROKER);
    mqtt.subscribe(TOPICO_SUB);
    Serial.printf("Inscrito no tópico: %s\n\n\r", TOPICO_SUB);
  }
  else
  {
    Serial.printf("Falha ao conectar ao broker %s\n\r", BROKER);
    Serial.print("Falha: ");
    Serial.println(mqtt.state());
  }
}

void loop()
{
  mqtt.loop();

  static unsigned long ultimoEnvio = 0;
  const unsigned long tempoEntreEnvios = 5000;

  //*Dados
  float temperatura = 50.0f;
  bool estado = true;
  unsigned timestamp = 1793796662;
  String mensagemErro = "Sem erro";
  int posicaoDisplay = 3;

  if(millis() - ultimoEnvio > tempoEntreEnvios)
  {
    Serial.printf("");
    ultimoEnvio = millis();
    
    JsonDocument doc;
    doc ["temperatura"] = temperatura;
    doc ["estado"] = estado;
    doc ["timestamp"] = timestamp;
    doc ["mensagemErro"] = mensagemErro;
    doc ["posicaoDisplay"] = posicaoDisplay;

    String mensagem;
    serializeJson(doc, mensagem);

    mqtt.publish(TOPICO_PUB, mensagem.c_str());
    Serial.printf("Mensagem a ser enviada no tópico %s: \n\r%s", TOPICO_PUB, mensagem.c_str());
  }
}

void aoReceber(char *topico, byte *payload, unsigned int tamanho)
{
  String mensagem;
  mensagem.reserve(tamanho); //* evita realocações enquanto a String cresce.

  for (unsigned int i = 0; i < tamanho; i++)
  {
    mensagem += (char)payload[i];
  }

  Serial.printf("Mensagem Recebida no tópico: %s \n\r mensagem:\n\r %s\n\n\r", topico, mensagem.c_str());

  tratarJson(mensagem);
}

void tratarJson(String mensagem)
{
  JsonDocument doc;

  DeserializationError codigoErro = deserializeJson(doc, mensagem);

  bool estadoLed = doc["estadoLed"].as<bool>();

  if (estadoLed)
  {
    Serial.println("LED LIGADO\n\r");
    rgbLedWrite(48, 0, 0, 255);
  }
  else
  {
    Serial.println("LED DESLIGADO\n\r");
    rgbLedWrite(48, 0, 0, 0);
  }
}