#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ThingSpeak.h"

// Configurações Wi-Fi
const char* ssid = "ALGAR_Torres2023";
const char* password = "t0rres!001";

// Configurações ThingSpeak
const char* writeApiKey = "9NG6QLIN8UXLE2AH";
const char* readApiKey = "5UWNQD21RD2A7QHG";
unsigned long Channel = 2840207;
unsigned int field3 = 3;     //field3 = variável para a bomba
unsigned int field4 = 4;     //field4 = variável para o aquecedor
unsigned int field5 = 5;     //field5 = variável para o modo de funcionamento
unsigned int field8 = 6;     //field8 = variável para o setpoint temperatura
unsigned int field6 = 7;     //field6 = variável para o tempo ciclo liga bomba
unsigned int field7 = 8;     //field7 = variável para o tempo desliga bomba

//variáveis para ciclo da bomba 
unsigned long tempoanterior;  // Armazena o último tempo de mudança
//unsigned int tempoliga = 3000;     // Tempo que a saída ficará ligada (ms)
//unsigned int tempoliga = 0;     // Tempo que a saída ficará ligada (ms)
//unsigned int tempodesliga = 2000;  // Tempo que a saída ficará desligada (ms)
//unsigned int settemp = 27;   //set point de temperatura para o modo automático

  
// Pinos
#define BOMBA 5         //D1 - utilizado para acionar a bomba 
#define AQUECEDOR 4     //D2 - utilizado para acionar o aquecedor
#define BARRAMENTO 2    //D4 - utilizado como barramento do OneWire para o sensor DS18B20 
#define TRIGGER_PIN 12  //D6 - Utilizado para envio de sinal do ultrassom HC-SR04
#define ECHO_PIN 14     //D5 - Utilizado para recebimento de sinal do ultrassom HC-SR04

// Inicializa o barramento OneWire e o sensor DS18B20
OneWire barramento(BARRAMENTO);
DallasTemperature sensors(&barramento);

//Seção configuração WIFI e IP estático
WiFiClient client;

IPAddress ip(192, 168, 10, 50);             //IP definido ao NodeMCU
IPAddress gateway(192, 168, 10, 114);       //IP do roteador
IPAddress subnet(255, 255, 255, 0);         //Mascara da rede

WiFiServer server(80);                     //Cria o servidor web na porta 80

void setup() {

  tempoanterior = millis();   // Inicia contagem de tempo para ciclo automatico bomba

  Serial.begin(115200); //velocidade do monitor serial
  delay(100);

  pinMode(BOMBA, OUTPUT);                  //indica o pino 5 como saída da bomba
  pinMode(AQUECEDOR, OUTPUT);              //indica o pino 4 como saída do aquecedor
  digitalWrite(BOMBA, LOW);                //coloca o pino 5 em um - inicia desligado
  digitalWrite(AQUECEDOR, LOW);            //coloca o pino 4 em um - inicia desligado

  pinMode(TRIGGER_PIN, OUTPUT);             //indica o pino 12 como saida de sinal do sensor HC-SR04
  pinMode(ECHO_PIN, INPUT);                 //indica o pino 14 como entrada de sinal do sensor HC-SR04

  //WiFi.config(ip, gateway, subnet); depois de removido passou a comunicar com o thingspeak
  WiFi.begin(ssid, password);                 //Conecta ao WIFI
  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado ao Wi-Fi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  sensors.begin();
  ThingSpeak.begin(client);
}

//função que faz a emissão do sinal do HC-SR04
float lerDistancia() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  //função que faz a recepção do sinal do HC-SR04
  long duracao = pulseIn(ECHO_PIN, HIGH);
  float distancia = duracao * 0.034 / 2; // Distância em centímetros - distance to an object = ((speed of sound in the air)*time)/2
  return distancia;
}
//fim função emissão / recepção

//Início seção de execução em loop
void loop() {

  //Verifica se o WIFI continua conectado e reconecta se necessário
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  //------------------------------------------------------------------------------------------------------------------
  //Seção de leitura dos sensores
  //Lê a temperatua no DS18B20
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);
  //float temperatura = 30; teste para atribuir dado para temperatura
//  Serial.print("Temperatura: ");
//  Serial.print(temperatura);
//  Serial.println(" °C");

  //chama função de emissão / recepção do sensor HC-SR04
  float distancia = lerDistancia();
//  Serial.print("Distância: ");
//  Serial.print(distancia);
//  Serial.println(" cm");

  //------------------------------------------------------------------------------------------------------------------
  // Seção escreve a temperatura e o nível no Thingspeak
  ThingSpeak.setField(1, temperatura);
  ThingSpeak.setField(2, distancia);

  int status = ThingSpeak.writeFields(Channel, writeApiKey);  //--> Verifica se escrita foi bem sucedida = 200
  if (status == 200) {
    Serial.println("Dados enviados ao ThingSpeak com sucesso!");
  } else {
    Serial.println("Erro ao enviar dados ao ThingSpeak. Código: " + String(status));
  }

  //------------------------------------------------------------------------------------------------------------------
  //Seção de comando das saídas recebendo do Thingspeak
  //Lê os canais 3, 4, 5, 6, 7 e 8 do thingspeak
  int statusCode = 0;
  int est_bba = ThingSpeak.readFloatField(Channel, field3);  //--> Pega ultimo estado field3 (Bomba) 
  int est_aqu = ThingSpeak.readFloatField(Channel, field4);  //--> Pega ultimo estado field4 (Aquecedor)
//  int modofunc = ThingSpeak.readFloatField(Channel, field5); //--> Pega modo de funcionamento manual (0) ou automático (1)
  int modofunc = 0; //Sempre que liga a placa inciar no modo manual.
  int tempoliga = ThingSpeak.readFloatField(Channel, field6); //--> Pega o set de temperatura para modo automático
  int tempodesliga = ThingSpeak.readFloatField(Channel, field7); //--> Pega o tempo do ciclo liga bomba
  int settemp = ThingSpeak.readFloatField(Channel, field8); //--> Pega o tempo do ciclo desliga bomba
  statusCode = ThingSpeak.getLastReadStatus();  //--> Verifica se leitura foi bem sucedida = 200

  modofunc = ThingSpeak.readFloatField(Channel, field5); //--> Pega modo de funcionamento manual (0) ou automático (1)
  
  if(statusCode == 200){
    Serial.print("Leitura de dados OK. Modo funcionamento (0) manual (1) automático ");
    Serial.println(modofunc);
    Serial.println("---------------------------------------------------------");
 //------------------------------------------------------------------------------------------------------------------
    // Se modofunc == 1, entra em modo de funcionamento automático
    if(modofunc == 1) {
      Serial.println("Entrando em modo de funcionamento automático (modofunc = 1)");
      Serial.println("---------------------------------------------------------");
      while(modofunc == 1) {

        // Verifica novamente o modofunc a cada iteração
        modofunc = ThingSpeak.readFloatField(Channel, field5); //--> Pega modo de funcionamento field5
        ThingSpeak.getLastReadStatus(); // Atualiza statusCode
        
        // Aquisição dados sensores durante o while
        //Lê a temperatua no DS18B20
        sensors.requestTemperatures();
        temperatura = sensors.getTempCByIndex(0);
        Serial.print("Modo automático - Temperatura: ");
        Serial.print(temperatura);
        Serial.println(" °C");

        //chama função de emissão / recepção do sensor HC-SR04
        distancia = lerDistancia();
        Serial.print("Modo automático - Distância: ");
        Serial.print(distancia);
        Serial.println(" cm");
        
        // Envia dados para ThingSpeak
        ThingSpeak.setField(1, temperatura);
        ThingSpeak.setField(2, distancia);
        ThingSpeak.writeFields(Channel, writeApiKey);
              
        // Ciclo automático bomba
         if(tempoliga != 0) {
            unsigned long tempoatual = millis(); // Tempo atual em ms

            if (digitalRead(BOMBA)) { // Verifica o ESTADO ATUAL do pino (HIGH/LOW)
                // Se a bomba está LIGADA, verifica se já passou o tempo para desligar
                if (tempoatual - tempoanterior >= tempoliga) {
                    digitalWrite(BOMBA, LOW);    // Desliga a bomba
                    tempoanterior = tempoatual;  // Reinicia o contador
                    Serial.println("Modo automático - Bomba desligada pelo ciclo");
                    Serial.print("tempo informado para o ciclo desliga da bomba: ");
                    Serial.print(tempodesliga);
                    Serial.println(" ms");
                    ThingSpeak.setField(3, 0);  //atualiza thingspeak
                    ThingSpeak.writeFields(Channel, writeApiKey);
                }
            } else {
                // Se a bomba está DESLIGADA, verifica se já passou o tempo para ligar
                if (tempoatual - tempoanterior >= tempodesliga) {
                    digitalWrite(BOMBA, HIGH);   // Liga a bomba
                    tempoanterior = tempoatual;  // Reinicia o contador
                    Serial.println("Modo automático - Bomba ligada pelo ciclo");
                    Serial.print("tempo informado para o ciclo liga da bomba: ");
                    Serial.print(tempoliga);
                    Serial.println(" ms");
                    ThingSpeak.setField(3, 1); //atualiza thingspeak
                    ThingSpeak.writeFields(Channel, writeApiKey);
                }
            }
        } else {
            digitalWrite(BOMBA, HIGH); // Bomba ligada continuamente (se tempoliga == 0)
            Serial.println("Modo automático - Bomba ligada continuamente");
            ThingSpeak.setField(3, 1);  //atualiza thingspeak
            ThingSpeak.writeFields(Channel, writeApiKey);
        }
        
        // Ciclo automático aquecedor
        if(temperatura < settemp){
          digitalWrite(AQUECEDOR, HIGH);
          Serial.println("Modo automático - Aquecedor ligado (buscando setpoint)");
          Serial.print("Set de temperatura informado: ");
          Serial.print(settemp);
          Serial.println(" °C");
          Serial.println("---------------------------------------------------------");
          ThingSpeak.setField(4, 1); //atualiza thingspeak
          ThingSpeak.writeFields(Channel, writeApiKey);
        }
        else if(temperatura >= settemp){
          digitalWrite(AQUECEDOR, LOW);
          Serial.println("Modo automático - Aquecedor desligado");
          Serial.print("Set de temperatura informado: ");
          Serial.print(settemp);
          Serial.println(" °C");
          Serial.println("---------------------------------------------------------");
          ThingSpeak.setField(4, 0); //atualiza thingspeak
          ThingSpeak.writeFields(Channel, writeApiKey);
        }
        
        
        delay(15000); // Aguarda 15 segundos entre iterações
      }
      
      Serial.println("Saindo do modo de funcionamento automático - desligando bomba e aquecedor");
      Serial.println("---------------------------------------------------------");
      digitalWrite(BOMBA, LOW);    // Desliga a bomba para retornar ao modo manual
      digitalWrite(AQUECEDOR, LOW);   // Desliga o aquecedor para retornar ao modo manual
      ThingSpeak.setField(3, 0);
      ThingSpeak.setField(4, 0);
      ThingSpeak.writeFields(Channel, writeApiKey);
    }
//------------------------------------------------------------------------------------------------------------------
    else { 
      // Operação modo manual quando modofunc != 1

      // Informa no modo manual os dados coletados pelos sensores 
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println(" °C");
      Serial.print("Distância: ");
      Serial.print(distancia);
      Serial.println(" cm");

        
      //Atua na bomba modo manaul
      if(est_bba == 1){
        digitalWrite(BOMBA, HIGH);
        Serial.println("Modo manual - Bomba ligada");
      }
      else if(est_bba == 0){
        digitalWrite(BOMBA, LOW);
        Serial.println("Modo manual - bomba desligada");
      }
//      Serial.print("O último estado da bomba no API é : ");
//      Serial.println(est_bba);   

      //Atua no aquecedor modo manual
      if(est_aqu == 1){
        digitalWrite(AQUECEDOR, HIGH);
        Serial.println("Modo manual - Aquecedor ligado");
        Serial.println("---------------------------------------------------------");
      }
      else if(est_aqu == 0){
        digitalWrite(AQUECEDOR, LOW);
        Serial.println("Modo manual - Aquecedor desligado");
        Serial.println("---------------------------------------------------------");
      }
//      Serial.print("O último estado do aquecedor no API é : ");
//      Serial.println(est_aqu);    
    }
  }
  else {
    Serial.println("Problemas para ler os canais do thingspeak. HTTP error code " + String(statusCode));
  }
  delay(15000); // Aguarda 15 segundos
}
