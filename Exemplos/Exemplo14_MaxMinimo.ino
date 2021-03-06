/******************************************
  Venturus - Centro de Inovação Tecnológica

  Exemplo 14 - Versao temperatura Máxima e Mínima

  @Author: Pedro Dionísio
  @Date: Jan/2017
******************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "Adafruit_MCP9808.h"

// WiFi
char ssid[] = "Vnt4Tech";   // Nome da rede WiFi
char pass[] = "Vnt@Vnt4Tech#9";   // Senha

//Armazena a temperatura máxima e o usuário que enviou a temperatura máxima
float max_temp = 0;
String user_maxTemp;

//Armazena a temperatura mínima e o usuário que enviou a temperatura mínima
float min_temp = 1000;
String user_minTemp;

// Sensor de temperatura
Adafruit_MCP9808 sensor = Adafruit_MCP9808();

// Configuração do MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
const char topico[] = "v4tech/chat";
const char usuario[] = "v4tech_vntpejr1";

// IOT ECLIPSE SERVER
char mqttServer[] = "iot.eclipse.org";

// funções
void analisaJson(String json);
void enviaJson();
void ligaLed(int porta);
void desligaLed(int porta);
void inverteLed(int porta);
void atualiza_led();
void atualiza_status();
void setup_wifi();
void conectaMqtt();

void enviaMaximo();
void enviaMinimo();

// led pode ser: 0 desligado, 1 ligado, 2 piscando
int ledState = 0;
int intervaloPisca = 1000;  // padrão 1 segundo
int ultimoPisca = 0;

// status da placa
int intervaloStatus = 5000; // padrão 5 segundos
int ultimoStatus = 0;

void setup() {
  pinMode(0, OUTPUT);     // led vermelho
  pinMode(2, OUTPUT);     // led azul

  Serial.begin(115200);
  delay(2000);  // aguarde porta serial funcionar

  // inicia wifi
  setup_wifi();

  // inicializa comunicação com sensor de temperatura
  if (!sensor.begin()) {
    Serial.println("Sensor de temperatura não encontrado!");
    while (1);
  }

  // inicializa sensor
  sensor.shutdown_wake(0);

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

  // atualiza estado do led
  atualiza_led();

  // atualiza status do sistema
  atualiza_status();
}

void callback(char* topic, byte* payload, unsigned int length) {

  // transforma o payload em string
  char content[200];
  int i = 0;
  for (i = 0; i < length; i++) {
    content[i] = (char) payload[i];
  }
  content[i] = '\0';
/*  
  Serial.print("Nova mensagem no topico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");
  Serial.println(content);
*/
  analisaJson(content);
}

void setup_wifi() {
  Serial.println();
  Serial.print("Conectando ao SSID: ");
  Serial.println(ssid);

  WiFi.disconnect();
  delay(200);
  
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
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void conectaMqtt() {
  while (!mqttClient.connected()) {
    Serial.println("Tentando conexão com Broker MQTT...");

    // conecta e insere um id qualquer (coloque seu nome, pois o servidor pode bloquear ids duplicados)
    if (mqttClient.connect(usuario)) {
      Serial.println("Conectado!");

      // faz subscribe no topico principal e já envia status
      mqttClient.subscribe(topico);
      
      enviaJson();
    } else {
      Serial.println("Ocorreu um erro na tentativa de conexão: ");
      Serial.println(mqttClient.state());
      Serial.println("Tentando novamente em 5 segundos...");
      delay(5000);      
    }
  }
}

// recebe um Json da serial e extrai as informações
// {"status":0} -> Deve retornar um json completo com temperatura e estado do LED
// {"seta_led":ESTADO} -> liga ou desliga o LED. Valores: ligado, desligado, piscando
// {"pisca_led":MILLIS} -> led fica piscando. Valores: delay em millis (1000 = 1 segundo)
void analisaJson(String json) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& buffer = jsonBuffer.parseObject(json);

  if (!buffer.success()) {
    Serial.println("Json apresenta erros..");
    return;
  }

  // Verifica se recebeu temperatura
  if (buffer.containsKey("id") && buffer.containsKey("temperatura")) {
    //Serial.println("Recebido temperatura");
    String nome = buffer["id"];
    float temp = buffer["temperatura"]; 

    //Verifica se temperatura recebida é maior que a armazenada
    if( temp > max_temp )
    {
        max_temp = temp;      //Atualiza temperatura máxima
        user_maxTemp = nome;  //Atualiza nome do usuário
        enviaMaximo();        //Envia pela serial as informações
    }

    //Verifica se temperatura recebida é inferior a armazenada
    if( temp < min_temp ) {
      min_temp = temp;        //Atualiza a temperatura mínima
      user_minTemp = nome;    //Atualiza nome do usuário
      enviaMinimo();          //Envia pela serial as informações
    }
  }

  // atualiza estado do led
  if (buffer.containsKey("status")) {
    // retorna status via Web (MQTT)
    enviaJson();

    //Reinicia os dados, para capturar novamente a máxima e mínima
    Serial.print("Reinicia dados");
    Serial.println();
    min_temp = 1000;
    max_temp = -1;
    user_minTemp = "";
    user_maxTemp = "";
  }
  
  // atualiza estado do led
  if (buffer.containsKey("seta_led")) {
    String lamp = buffer["seta_led"];
    if (lamp == "desligado") {
      ledState = 0;
    } else if (lamp == "ligado") {
      ledState = 1;
    } else if (lamp == "piscando") {
      ledState = 2;
    }
    
    // retorna estado do led
    enviaJson();
  }

  // pisca se for necessário
  if (buffer.containsKey("pisca_led")) {
    intervaloPisca = buffer["pisca_led"];
    ledState = 2;

    // retorna estado do led
    enviaJson();
  }
}

//Monta a String Json com os dados da temperatura máxima e envia pela porta Serial
void enviaMaximo()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& buffer = jsonBuffer.createObject();

  // monta json
  buffer["maior"] = "Maximo";
  buffer["id"] = user_maxTemp;
  buffer["temperatura"] = max_temp;  
  buffer.printTo(Serial);
  Serial.println();
}

//Monta a String Json com os dados da temperatura mínima e envia pela porta Serial
void enviaMinimo()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& buffer = jsonBuffer.createObject();

  // monta json
  buffer["menor"] = "Minimo";
  buffer["id"] = user_minTemp;
  buffer["temperatura"] = min_temp;
  buffer.printTo(Serial);
  Serial.println();
}

// envia um json completo
// {"led":ESTADO,"temperatura¨:VALOR} -> ESTADO  "ligado", "desligado", "piscando"
void enviaJson() {
  String lampada = "";

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& buffer = jsonBuffer.createObject();

  // define a string do estado do led
  switch (ledState) {
    case 0:
      lampada = "desligado";
      break;
    case 1:
      lampada = "ligado";
      break;
    case 2:
      lampada = "piscando";
      break;
  }

  // le sensor de temperatura
  sensor.shutdown_wake(0);
  delay(200);
  float temp = sensor.readTempC();
  delay(200);
  sensor.shutdown_wake(1);
  // monta json
  buffer["led"] = lampada;
  buffer["temperatura"] = temp;

  // envia para broker
  char mensagem[100];
  buffer.printTo(mensagem, sizeof(mensagem));
  mqttClient.publish(topico, mensagem, false);

  enviaMinimo();
  enviaMaximo();

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

// liga, desliga ou faz led piscar
void atualiza_led() {

  // define a string do estado do led
  switch (ledState) {
    case 0:
      desligaLed(0);
      break;
    case 1:
      ligaLed(0);
      break;
    case 2:
      if (millis() - ultimoPisca > intervaloPisca) {
        ultimoPisca = millis();
        inverteLed(0);
      }
      break;
  }
}

// envia status para Broker em intervalos de tempo
void atualiza_status() {
  if (millis() - ultimoStatus > intervaloStatus) {
    ultimoStatus = millis();
    enviaJson();
  }
}

