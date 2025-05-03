#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ThingSpeak.h"

// Configurações Wi-Fi
const char* ssid = "ALGAR_Torres2023";
const char* password = "t0rres!001";

// Configurações canal 1 ThingSpeak
const char* writeApiKey1 = "9NG6QLIN8UXLE2AH";
const char* readApiKey1 = "5UWNQD21RD2A7QHG";
unsigned long Channel1 = 2840207;
unsigned int field3 = 3;     // field3 = variável para a bomba
unsigned int field4 = 4;     // field4 = variável para o aquecedor
unsigned int field5 = 5;     // field5 = variável para o modo de funcionamento
unsigned int field8 = 6;     // field8 = variável para o setpoint temperatura
unsigned int field6 = 7;     // field6 = variável para o tempo ciclo liga bomba
unsigned int field7 = 8;     // field7 = variável para o tempo desliga bomba

// Configurações canal 2 ThingSpeak
const char* writeApiKey2 = "BY3NQR5RTECHYXQ5";
const char* readApiKey2 = "7ORUZSCMCUEUAQ3Z";
unsigned long Channel2 = 2533413;

// Variáveis para ciclo da bomba 
unsigned long tempoanterior;  // Armazena o último tempo de mudança
  
// Pinos
#define BOMBA 5         // D1 - utilizado para acionar a bomba 
#define AQUECEDOR 4     // D2 - utilizado para acionar o aquecedor
#define BARRAMENTO 2    // D4 - utilizado como barramento do OneWire para o sensor DS18B20 
#define TRIGGER_PIN 12  // D6 - Utilizado para envio de sinal do ultrassom HC-SR04
#define ECHO_PIN 14     // D5 - Utilizado para recebimento de sinal do ultrassom HC-SR04

// Inicializa o barramento OneWire (sensor DS18B20 - D4)
OneWire barramento(BARRAMENTO);
DallasTemperature sensors(&barramento);

// Seção configuração WIFI e IP estático
WiFiClient client;

IPAddress ip(192, 168, 10, 50);             // IP definido ao NodeMCU
IPAddress gateway(192, 168, 10, 114);       // IP do roteador
IPAddress subnet(255, 255, 255, 0);         // Mascara da rede

WiFiServer server(80);                     // Cria o servidor web na porta 80

// Protótipos das funções
float lernivel();
void EscreveCanal2(int estadoBomba, int estadoAquecedor, int modoAtual, int setAtual, int ligaAtual, int desligaAtual);
void EscreveCanal1(float tempAtual, float nivelAtual);

void setup() {
  tempoanterior = millis();   // Inicia contagem de tempo para ciclo automatico bomba

  Serial.begin(115200); // velocidade do monitor serial
  delay(100);

  pinMode(BOMBA, OUTPUT);      // indica o pino 5 como saída da bomba
  pinMode(AQUECEDOR, OUTPUT);  // indica o pino 4 como saída do aquecedor
  digitalWrite(BOMBA, LOW);    // coloca o pino 5 em LOW - inicia desligado
  digitalWrite(AQUECEDOR, LOW); // coloca o pino 4 em LOW - inicia desligado

  pinMode(TRIGGER_PIN, OUTPUT); // indica o pino 12 como saida - emite sinal do sensor HC-SR04
  pinMode(ECHO_PIN, INPUT);     // indica o pino 14 como entrada - recebe sinal do sensor HC-SR04

  WiFi.begin(ssid, password);   // Conecta ao WIFI
  Serial.println("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Conectado ao Wi-Fi");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  sensors.begin();
  ThingSpeak.begin(client);
  
  // Envia estado inicial para o Canal 2
  EscreveCanal2(0, 0, 0, 0, 0, 0);
}

// Função que faz a emissão e recepção do sinal do HC-SR04
float lernivel() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  long duracao = pulseIn(ECHO_PIN, HIGH);
  float nivel = duracao * 0.034 / 2; // Distância em centímetros
  return nivel;
}

// Função para enviar informações para o Canal 2
void EscreveCanal2(int estadoBomba, int estadoAquecedor, int modoAtual, int setAtual, int ligaAtual, int desligaAtual) {
  ThingSpeak.setField(1, estadoBomba);      // Field 1 - Estado da bomba
  ThingSpeak.setField(2, estadoAquecedor);  // Field 2 - Estado do aquecedor
  ThingSpeak.setField(3, modoAtual);        // Field 3 - Modo de operação
  ThingSpeak.setField(4, setAtual);         // Field 4 - Setpoint de temperatura
  ThingSpeak.setField(5, ligaAtual);        // Field 5 - Tempo liga bomba
  ThingSpeak.setField(6, desligaAtual);     // Field 6 - Tempo desliga bomba
  
  int status = ThingSpeak.writeFields(Channel2, writeApiKey2);
  if (status == 200) {
    Serial.println("Estados enviados ao Canal 2 com sucesso!");
  } else {
    Serial.println("Erro ao enviar estados ao Canal 2. Código: " + String(status));
  }
}

// Função para enviar informações para o Canal 1
void EscreveCanal1(float tempAtual, float nivelAtual) {
  ThingSpeak.setField(1, tempAtual);    // Field 1 - Temperatura atual
  ThingSpeak.setField(2, nivelAtual);   // Field 2 - Nível atual
  
  int status = ThingSpeak.writeFields(Channel1, writeApiKey1);
  if (status == 200) {
    Serial.println("Dados enviados ao Canal 1 com sucesso!");
  } else {
    Serial.println("Erro ao enviar dados ao Canal 1. Código: " + String(status));
  }
}

void loop() {
  // Verifica se o WIFI continua conectado e reconecta se necessário
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }

  // Seção de leitura dos sensores
  sensors.requestTemperatures();
  float temperatura = sensors.getTempCByIndex(0);
  float nivel = lernivel();

  // Escreve a temperatura e o nível no Thingspeak (Canal 1)
  EscreveCanal1(temperatura, nivel);

  // Seção de comando das saídas - recebendo do Thingspeak
  int statusCode = 0;
  int est_bba = ThingSpeak.readFloatField(Channel1, field3);  // Pega ultimo estado field3 (Bomba)
  int est_aqu = ThingSpeak.readFloatField(Channel1, field4);  // Pega ultimo estado field4 (Aquecedor)
  int modofunc = ThingSpeak.readFloatField(Channel1, field5); // Pega modo de funcionamento
  int tempoliga = ThingSpeak.readFloatField(Channel1, field6); // Pega o tempo do ciclo liga bomba
  int tempodesliga = ThingSpeak.readFloatField(Channel1, field7); // Pega o tempo do ciclo desliga bomba
  int settemp = ThingSpeak.readFloatField(Channel1, field8); // Pega o setpoint de temperatura
  statusCode = ThingSpeak.getLastReadStatus();

  if(statusCode == 200) {
    Serial.print("Leitura de dados OK. Modo funcionamento (0) manual (1) automático ");
    Serial.println(modofunc);
    Serial.println("---------------------------------------------------------");

    // Se modofunc == 1, entra em modo de funcionamento automático
    if(modofunc == 1) {
      Serial.println("Entrando em modo de funcionamento automático (modofunc = 1)");
      Serial.println("---------------------------------------------------------");
      
      while(modofunc == 1) {
        modofunc = ThingSpeak.readFloatField(Channel1, field5); // Atualiza modo de funcionamento
        
        // Aquisição dados sensores
        sensors.requestTemperatures();
        temperatura = sensors.getTempCByIndex(0);
        nivel = lernivel();
        
        // Envia dados para ThingSpeak (Canal 1 e 2)
        EscreveCanal1(temperatura, nivel);
        EscreveCanal2(digitalRead(BOMBA), digitalRead(AQUECEDOR), modofunc, settemp, tempoliga, tempodesliga);

        // Ciclo automático bomba
        if(tempoliga != 0) {
          unsigned long tempoatual = millis();
          if (digitalRead(BOMBA)) {
            if (tempoatual - tempoanterior >= tempoliga) {
              digitalWrite(BOMBA, LOW);
              tempoanterior = tempoatual;
              Serial.println("Modo automático - Bomba desligada pelo ciclo");
              EscreveCanal2(0, digitalRead(AQUECEDOR), modofunc, settemp, tempoliga, tempodesliga);
            }
          } else {
            if (tempoatual - tempoanterior >= tempodesliga) {
              digitalWrite(BOMBA, HIGH);
              tempoanterior = tempoatual;
              Serial.println("Modo automático - Bomba ligada pelo ciclo");
              EscreveCanal2(1, digitalRead(AQUECEDOR), modofunc, settemp, tempoliga, tempodesliga);
            }
          }
        } else {
          digitalWrite(BOMBA, HIGH);
          Serial.println("Modo automático - Bomba ligada continuamente");
          EscreveCanal2(1, digitalRead(AQUECEDOR), modofunc, settemp, tempoliga, tempodesliga);
        }
        
        // Ciclo automático aquecedor
        if(temperatura < settemp) {
          digitalWrite(AQUECEDOR, HIGH);
          Serial.println("Modo automático - Aquecedor ligado (buscando setpoint)");
          EscreveCanal2(digitalRead(BOMBA), 1, modofunc, settemp, tempoliga, tempodesliga);
        } else {
          digitalWrite(AQUECEDOR, LOW);
          Serial.println("Modo automático - Aquecedor desligado");
          EscreveCanal2(digitalRead(BOMBA), 0, modofunc, settemp, tempoliga, tempodesliga);
        }
        
        delay(15000); // Aguarda 15 segundos 
      }
      
      Serial.println("Saindo do modo de funcionamento automático - desligando bomba e aquecedor");
      digitalWrite(BOMBA, LOW);
      digitalWrite(AQUECEDOR, LOW);
      EscreveCanal2(0, 0, modofunc, settemp, tempoliga, tempodesliga);
    }
    // Operação modo manual quando modofunc != 1
    else { 
      // Informa no modo manual os dados coletados pelos sensores 
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println(" °C");
      Serial.print("Nível: ");
      Serial.print(nivel);
      Serial.println(" cm");

      // Atua na bomba modo manual
      if(est_bba == 1) {
        digitalWrite(BOMBA, HIGH);
        Serial.println("Modo manual - Bomba ligada");
      } else {
        digitalWrite(BOMBA, LOW);
        Serial.println("Modo manual - bomba desligada");
      }

      // Atua no aquecedor modo manual
      if(est_aqu == 1) {
        digitalWrite(AQUECEDOR, HIGH);
        Serial.println("Modo manual - Aquecedor ligado");
      } else {
        digitalWrite(AQUECEDOR, LOW);
        Serial.println("Modo manual - Aquecedor desligado");
      }

      // Envia estados para os canais
      EscreveCanal1(temperatura, nivel);
      EscreveCanal2(digitalRead(BOMBA), digitalRead(AQUECEDOR), modofunc, settemp, tempoliga, tempodesliga);
      Serial.println("---------------------------------------------------------");
    }
  } else {
    Serial.println("Problemas para ler os canais do thingspeak. HTTP error code " + String(statusCode));
  }
  delay(15000); // Aguarda 15 segundos
}