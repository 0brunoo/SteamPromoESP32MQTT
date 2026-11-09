# 🎮 SteamPromo - ESP32 & MQTT
Um projeto maneiro de Internet das Coisas (IoT) desenvolvido com ESP32 que busca as melhores promoções de jogos da Steam e transmite os dados via protocolo MQTT.

O sistema utiliza a API do CheapShark para encontrar os descontos e conta com dois módulos principais: um que busca e publica as ofertas, e outro que recebe as mensagens e sinaliza visualmente se o jogo está com um preço imperdível!

## ✨ Funcionalidades
- Integração com API REST: Consulta de ofertas em tempo real utilizando a API pública do CheapShark.

- Comunicação Pub/Sub: Troca de mensagens instantânea via protocolo MQTT e JSON.

- Interface Física Interativa: Navegação entre diferentes páginas de ofertas através de um botão físico com debounce.

- Displays Informativos: Uso de telas LCD 20x4 (I2C) com animações customizadas de carregamento.

- Alerta Visual de Preço: O módulo receptor conta com um LED RGB que brilha em VERDE para jogos muito baratos (abaixo de $1.00) ou VERMELHO para os demais preços.

## 🏗️ Arquitetura do Projeto
O código deste repositório foi dividido na lógica de dois nós (nodes):

- O Buscador (Publisher):

- Conecta ao Wi-Fi e faz requisições HTTPS para a API do CheapShark.

- Um botão permite avançar a página da API para descobrir novos jogos.

- Monta um payload em formato JSON com Nome do Jogo, Preço Original, Preço Promocional e Desconto.

- Publica os dados no tópico MQTT (Senai/Mesa_do_CANO/Jean-Bruno/Status).

- O Receptor (Subscriber):

- Fica inscrito no tópico aguardando ofertas.

- Deserializa o JSON recebido.

- Exibe o nome do jogo e o preço no LCD.

- Aciona o LED RGB (Verde para < $1.00, Vermelho para o restante).

## 🛠️ Hardware Necessário
Para reproduzir o projeto completo, você vai precisar de:

- 2x Placas ESP32 (NodeMCU ou similar)

- 2x Displays LCD 20x4 I2C

- 1x Módulo LED RGB

- 1x Push Button

- Protoboard e Jumpers

## 📚 Bibliotecas Utilizadas
Certifique-se de instalar as seguintes bibliotecas na sua Arduino IDE antes de compilar:

- WiFi.h, HTTPClient.h, WiFiClientSecure.h

- PubSubClient

- ArduinoJson

- LiquidCrystal_I2C 

- Botao.h 

## 🚀 Como usar
Clone o repositório:

- Bash
git clone https://github.com/SEU-USUARIO/steam-deal-hunter-iot.git
Abra na Arduino IDE (ou VS Code com PlatformIO).

- Configure as credenciais:
Altere as variáveis de rede e MQTT no início de ambos os códigos para o seu ambiente:

<fieldset>
<p>C++</p>
<p>const char *SSID = "SUA_REDE_WIFI";</p>
<p>const char *SENHA = "SUA_SENHA";</p>
<p>const char *BROKER = "broker.emqx.io"; // Ou outro de sua preferência</p>
<p>const char *TOPICO_PUB = "Seu/Topico/Personalizado";</p>
<p>Faça o Upload do código do Receptor no primeiro ESP32 e do código do Buscador no segundo ESP32.</p>
</fieldset>

Pressione o botão no ESP32 Buscador para começar a minerar as ofertas! 💸

## 📡 Payload JSON
Exemplo do dado que trafega pelo Broker MQTT:

<fieldset>
<p>JSON</p>
<p>{</p>
  <p>"jogo": "Left 4 Dead 2",</p>
  <p>"preco_de": 9.99,</p>
  <p>"preco_por": 0.99,</p>
  <p>"desconto": 90,</p>
  <p>"posicao": 1</p>
<p>}/</p>
</fieldset>
## 👨‍💻 Autor
Desenvolvido por Jean Oliveira e Bruno Santos(como visto no tópico MQTT!) e equipe do Senai. Sinta-se à vontade para fazer um fork, abrir issues ou mandar pull requests com melhorias!

Disclaimer: Este projeto foi criado com fins educacionais para explorar os conceitos de IoT, HTTPs e MQTT.
