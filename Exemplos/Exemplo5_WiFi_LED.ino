/******************************************
  Venturus - Centro de Inovação Tecnológica

  Exemplo 6 - Ler Porta Serial

  @Author: Pedro Dionísio
  @Date: Jul/2016
******************************************/
#include <ESP8266WiFi.h>

// WiFi
char ssid[] = "Vnt4Tech";   // Nome da rede WiFi
char pass[] = "Vnt@Vnt4Tech#9";   // Senha

// Declarando funções do código
void setup_wifi();

void setup() {
  pinMode(0, OUTPUT); // led vermelho
  pinMode(2, OUTPUT); // led azul
    
  digitalWrite(2, HIGH); // desliga led azul

  Serial.begin(115200); // Configura porta serial a 115200 baudrate

  setup_wifi();
}

void setup_wifi() {
  delay(1000);  // delay para aguardar porta serial
  
  Serial.println();
  Serial.print("Conectando ao SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    digitalWrite(2, !digitalRead(2)); // inverte led azul
    Serial.print(".");
  }

  // led azul ligado quando conectar na rede
  digitalWrite(2, LOW);

  Serial.println("");
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
}

