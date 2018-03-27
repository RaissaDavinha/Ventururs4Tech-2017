/******************************************
  Venturus - Centro de Inovação Tecnológica

  Exemplo 4 - WiFi

  @Author: Pedro Dioníso
  @Date: Jul/2016
******************************************/

#include <ESP8266WiFi.h>

// WiFi
char ssid[] = "Vnt4Tech";   // Nome da rede WiFi
char pass[] = "Vnt@Vnt4Tech#9";   // Senha

// Declarando funções do código
void setup_wifi();

void setup() {
  pinMode(0, OUTPUT);
  Serial.begin(115200); // Configura porta serial a 115200 baudrate
  delay(1000);
  setup_wifi();
}


void setup_wifi() {  
  Serial.println();
  Serial.print("Conectando ao SSID: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado ao WiFi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
}

