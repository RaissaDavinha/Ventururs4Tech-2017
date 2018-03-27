/******************************************
  Venturus - Centro de Inovação Tecnológica

  Exemplo 12 - MQTT 

  @Author: Pedro Dionísio
  @Date: Jan/2017
******************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// WiFi
char ssid[] = "Vnt4Tech";         // Nome da rede WiFi
char pass[] = "Vnt@Vnt4Tech#9";   // Senha

// Configuração do MQTT
WiFiClient wifiClient;            //Cria o objeto WiFiClient
PubSubClient mqttClient(wifiClient);    //Inicia o objeto PubSubClient utilizando o wifiClient
const char topico[] = "v4tech/chat";    //Tópico para enviar/receber as mensagem
const char usuario[] = "v4tech_vntpejr";  //Identificacao da placa(usuário)

// IOT ECLIPSE SERVER
char mqttServer[] = "iot.eclipse.org";    //Endereço do broker mqtt

// funções
void ligaLed(int porta);
void desligaLed(int porta);
void inverteLed(int porta);
void setup_wifi();
void conectaMqtt();

void setup() {
  pinMode(0, OUTPUT);     // led vermelho
  pinMode(2, OUTPUT);     // led azul

  Serial.begin(115200);
  delay(2000);  // aguarde porta serial funcionar

  // inicia wifi
  setup_wifi();

  // inicia cliente mqtt
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(callback);
}

void loop() {
  // se perder conexão, tente novamente
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
  // reconecte-se ao servidor se MQTT estiver desconectado
  if (!mqttClient.connected()) {
    conectaMqtt();
  }
  // função loop deve ser chamada para receber mensagens
  mqttClient.loop();

  // envia dados da serial para mqtt
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    char buffer[100];
    msg.toCharArray(buffer, 100);
    mqttClient.publish(topico, buffer);
  }
}

// recebe mensagens do mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  
  // transforma o payload em string
  char content[200];
  int i = 0;
  for (i = 0; i < length; i++) {
    content[i] = (char) payload[i];
  }
  content[i] = '\0';
  
  Serial.print("Nova mensagem no topico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");
  Serial.println(content);
}

void setup_wifi() {
  Serial.println();
  Serial.print("Conectando ao SSID: ");
  Serial.println(ssid);

  //Garante que está desconectado, antes de iniciar a conexão
  WiFi.disconnect();
  delay(200);

  //Conecta ao Wifi
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    inverteLed(2); // pisca led azul
    Serial.print(".");
  }

  // led azul ligado quando conectar na rede
  ligaLed(2);

  Serial.println("");
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());
}

// estabelece conexão com broker e faz subscribe no topico
void conectaMqtt() {
  while (!mqttClient.connected()) {
    Serial.println("Tentando conexao com Broker MQTT...");

    // conecta e insere um id qualquer (coloque seu nome)
    if (mqttClient.connect(usuario)) {
      Serial.println("Conectado!");

      // faz subscribe no topico principal e já envia status
      mqttClient.subscribe(topico);
    } else {
      Serial.println("Ocorreu um erro na tentativa de conexão: ");
      Serial.println(mqttClient.state());
      Serial.println("Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

// função liga o led da porta
void ligaLed(int porta) {
  digitalWrite(porta, LOW);
}

// função desliga o led da porta
void desligaLed(int porta) {
  digitalWrite(porta, HIGH);
}

// inverte estado do led da porta
void inverteLed(int porta) {
  digitalWrite(porta, !digitalRead(porta));
}
