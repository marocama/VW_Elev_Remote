/*
** VW Elev - Chamado Remoto
** Marcus Roberto
** VW Soluçoes
*/

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP_ClientMacaddress.h>

// Dispositivos cadastros 
#define NUM_DISPOSITIVOS 3

// Pino do Botão de Reset
#define BOTAO_RESET 15

// Definição dos MACs conhecidos
uint8_t macList[NUM_DISPOSITIVOS][6] = {
  {0xB4,0x63,0xC6,0x8A,0x8F,0x16},
  {0xA4,0x63,0xC6,0x8A,0x8F,0x16},
  {0xD4,0x63,0xC6,0x8A,0x8F,0x16} 
};

// Pavimentos permitidos dos dispositivos
uint8_t pavList[NUM_DISPOSITIVOS][16] = {
  {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
  {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Definição da Rede AP
const char* rede_ssid = "testee";
const char* rede_senha = "12369874";

// Índice do dispositivo conectado
int mac_indice = 0;

// Objetos de manipulação
AsyncWebServer server(80);
ClientMacaddress clientMac(macList, NUM_DISPOSITIVOS);

void setup() {

  Serial.begin(115200);

  pinMode(BOTAO_RESET, INPUT_PULLUP);
  
  // Inicializa modo AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(rede_ssid, rede_senha);
  delay(100);

  // Configura IP Fixo para o ESP32
  IPAddress Ip(192, 168, 1, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);

  Serial.println("Configuração Finalizada");
  
  // ====================================================================================
  // Inicia a rota '/verifica'
  // API: Verifica o MAC e retorna os pavimentos disponiveis. 404 caso não identifique.
  server.on("/verifica", HTTP_GET, [](AsyncWebServerRequest *request) {

    Serial.println("Nova Conexão - Rota: Verifica");

    String response = "";

    // Identifica MAC Address e confere se é conhecido
    uint8_t *m = clientMac.getAddr(request->client()->remoteIP());
    Serial.printf("MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", m[0],m[1],m[2],m[3],m[4],m[5]);
    mac_indice = clientMac.isKnown(m);
 
    // Cria String com o status dos pavimentos disponiveis
    if(mac_indice != 404) {
      for(uint8_t x = 0; x < 16; x++) {
        response += pavList[mac_indice][x];
        if(x < 15) {
          response += ".";
        }
      }
    } else {
      response = "404";
    }

    // Retorna pavimentos disponiveis
    request->send(200, "text/plain", response);
  });
  // ====================================================================================

  // ====================================================================================
  // Inicia a rota '/chamado'
  // API: Verifica o MAC e se o pavimento solicitado está liberado e faz o chamado. 202 para ok, 401 para sem permissão e 404 para não identificado
  server.on("/chamado", HTTP_GET, [](AsyncWebServerRequest *request) {

    Serial.println("Nova Conexão - Rota: Chamado");

    String response = "";
    
    // Identifica MAC Address e confere se é conhecido
    uint8_t *m = clientMac.getAddr(request->client()->remoteIP());
    Serial.printf("MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", m[0],m[1],m[2],m[3],m[4],m[5]);
    mac_indice = clientMac.isKnown(m);
    if(mac_indice == 404) {
      response = "404";
    }

    // Verifica se o pavimento solicitado está disponivel
    AsyncWebParameter* p = request->getParam(0);
    if(p->name().indexOf("pav") >= 0 && mac_indice != 404) {
      
      uint8_t pav = atoi(p->value().substring(0, 2).c_str()); 
      if(pavList[mac_indice][pav] == 1) {
        
        Serial.print("CHAMADO PARA: ");
        Serial.println(p->value());
        response = "202";
        // ACIONAR ENTRADA PARA CHAMADA DO PAVIMENTO
      }
      else {
        
        Serial.println("SEM PERMISSÃO PARA O PAVIMENTO SOLICITADO");
        response = "401";
      }
    }

    // Retorna resposta do chamado
    request->send(200, "text/plain", response);
  });
  // ====================================================================================

  // ====================================================================================
  // Inicia a rota '/add'
  server.on("/H2sKa84V/adiciona", HTTP_GET, [](AsyncWebServerRequest *request) {

    Serial.println("Nova Conexão - Rota: Adiciona");

    String response = "";

    if(!digitalRead(BOTAO_RESET)) {

      AsyncWebParameter* p = request->getParam(0);
      if(p->name().indexOf("mac") >= 0) {

        // LER MAC PELA URL
      }
    } else {

      response = "412";
    }

    // Retorna resposta da adição de MAC
    request->send(200, "text/plain", response);
  });
  // ====================================================================================

  // Inicia o servidor
  server.begin();
}

void loop() {
}
