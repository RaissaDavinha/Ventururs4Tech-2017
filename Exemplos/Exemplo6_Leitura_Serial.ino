/******************************************
  Venturus - Centro de Inovação Tecnológica

  Exemplo 6 - Leitura da Serial

  @Author: Pedro Dionísio
  @Date: Jul/2016
******************************************/

void setup() {
  pinMode(0, OUTPUT);     // led vermelho

  Serial.begin(115200);
  delay(2000);

  Serial.println("\r\nLigando Arduino...");
}

void loop() {

  // Se existe conteúdo na porta serial
  if (Serial.available()) {

    // leia o caractere
    char c = Serial.read();

    // imprime o caractere
    Serial.print(c);
  }
}

