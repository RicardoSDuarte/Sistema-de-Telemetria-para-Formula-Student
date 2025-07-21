
//APP BASE

#include "esp_now.h"
#include "ESPNOW.h"

#define MESSAGE_CAN_FRAMES_TO_SEND 10


// // Definição da estrutura FILTRO
// typedef struct {
//   uint16_t id;
//   uint8_t code;
// } FILTRO;

// // Estrutura para cabeçalho do pacote Wi-Fi
// typedef struct {
//   unsigned frame_ctrl: 16;
//   unsigned duration_id: 16;
//   uint8_t addr1[6];
//   uint8_t addr2[6];
//   uint8_t addr3[6];
//   unsigned sequence_ctrl: 16;
//   uint8_t addr4[6];
// } wifi_ieee80211_mac_hdr_t;

// Variáveis globais
volatile bool newDataAvailable = false;
int rssi_display;
CAN_FRAME latestFrame;
uint8_t latestMac[6];

// Adicionar implementação da função wrapper
void sendFiltroGlobal(const FILTRO &filtro) {
    //Serial.println("sendFiltroGlobal");
    extern ESPNOW esp;  // Referência à instância global
    esp.sendFiltro(filtro);
}

ESPNOW::ESPNOW(const uint8_t *peerAddr) {
  memcpy(peerAddress, peerAddr, 6);
}

void ESPNOW::ESPNOW_Init() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao iniciar ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(ESPNOW::onDataSent);
  esp_now_register_recv_cb(onDataReceive);



  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(peerInfo.peer_addr)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Falha ao adicionar peer");
    } else {
      Serial.println("Peer adicionado com sucesso!");
    }
  }

  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_5M_L);
}

bool ESPNOW::sendFiltro(const FILTRO &filtro) {
  //Serial.println("sendFiltro");
  esp_err_t result = esp_now_send(peerAddress, (uint8_t *)&filtro, sizeof(FILTRO));
  return (result == ESP_OK);
}

void ESPNOW::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("Status de envio: ");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
  static int c = 0;  // Variável estática local
  if(status != ESP_NOW_SEND_SUCCESS){
    //c++;
    //Serial.println("Falha");
    Serial.println(c);
  }
}

void ESPNOW::setReceiveCallback(esp_now_recv_cb_t cb) {
  esp_now_register_recv_cb(cb);
}

// === Funções ESP-NOW adicionais ===
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Status de envio: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
}

int c = 0;
uint64_t error = 0;
void onDataReceive(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  // static int c = 0;
  if (len != sizeof(CAN_FRAME)*MESSAGE_CAN_FRAMES_TO_SEND) {
    //Serial.println("Tamanho de pacote inválido");
    error++;
    return;
  }
    
  // Serial.println(c);
  for (int i=0;i<MESSAGE_CAN_FRAMES_TO_SEND;i++){
    CAN_FRAME receivedFrame;
    memcpy(&receivedFrame, incomingData+sizeof(CAN_FRAME)*i, sizeof(CAN_FRAME));
    // memcpy(latestMac, info->src_addr, 6);
    receivedFrame.data.value = c+(error<<32);
    c++;
    if (xQueueSend(QueueHandle, &receivedFrame, 0) != pdTRUE) {
      //Serial.println("Falha ao enviar para fila");
          error+=1<<16;
    }
  }
}

// === Promiscuous Mode para capturar RSSI ===
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return;
  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  rssi_display = ppkt->rx_ctrl.rssi;
}

void enablePromiscuousMode() {
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);
}